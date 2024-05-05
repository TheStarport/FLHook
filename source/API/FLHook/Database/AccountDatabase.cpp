#include "API/FLHook/Database.hpp"
#include "PCH.hpp"
#include <API/FLHook/AccountManager.hpp>
#include <Defs/Database/Account.hpp>
#include <mongocxx/exception/error_code.hpp>
#include <mongocxx/exception/write_exception.hpp>

bool AccountManager::SaveCharacter(Character& newCharacter, const bool isNewCharacter)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    const auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        auto accounts = db->database("FLHook")["accounts"];
        auto findCharDoc = make_document(kvp("characterName", newCharacter.characterName));

        if (isNewCharacter)
        {
            if (const auto checkCharNameDoc = accounts.find_one(findCharDoc.view()); checkCharNameDoc.has_value())
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                                "Character already exists while trying to create new character");
            }
        }

        // Upsert Character
        auto bsonBytes = rfl::bson::write(newCharacter);
        bsoncxx::document::view savedCharDoc{ reinterpret_cast<uint8_t*>(bsonBytes.data()), bsonBytes.size() };

        // Update account character list if new character
        if (isNewCharacter)
        {
            auto insertedDoc = accounts.insert_one(savedCharDoc);

            if (!insertedDoc.has_value())
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed), "Unable to upsert a character.");
            }
            bson_oid_t charOid;
            memcpy_s(charOid.bytes, sizeof(charOid.bytes), insertedDoc->inserted_id().get_oid().value.bytes(), bsoncxx::v_noabi::oid::k_oid_length);

            newCharacter._id = charOid;
            const auto findAccDoc = make_document(kvp("_id", newCharacter.accountId));
            const auto charUpdateDoc = make_document(kvp("$push", make_document(kvp("characters", insertedDoc->inserted_id()))));
            auto updateResult = accounts.update_one(findAccDoc.view(), charUpdateDoc.view());
            if (!updateResult.has_value() || updateResult.value().modified_count() == 0)
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                                "Updating account during character creation failed.");
            }
        }
        else
        {
            auto updateDoc = make_document(kvp("$set", savedCharDoc));
            auto updateResult = accounts.update_one(findCharDoc.view(), updateDoc.view());
            if (!updateResult.has_value())
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed), "Updating character failed.");
            }
        }
        session.commit_transaction();
        return true;
    }
    catch (bsoncxx::exception& ex)
    {
        if (isNewCharacter)
        {
            Logger::Err(std::format(L"Error while creating character {}", StringUtils::stows(ex.what())));
        }
        else
        {
            Logger::Err(std::format(L"Error while updating character {}", StringUtils::stows(ex.what())));
        }
    }
    catch (mongocxx::exception& ex)
    {
        if (isNewCharacter)
        {
            Logger::Err(std::format(L"Error while creating character {}", StringUtils::stows(ex.what())));
        }
        else
        {
            Logger::Err(std::format(L"Error while updating character {}", StringUtils::stows(ex.what())));
        }
    }
    session.abort_transaction();
    return false;
}

bool AccountManager::DeleteCharacter(const ClientId client, const std::wstring characterCode)
{
    auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    auto& account = accounts.at(client.GetValue());
    std::string charCodeString = StringUtils::wstos(characterCode);
    std::string charName = account.characters.at(charCodeString).characterName;
    std::wstring wideCharName = StringUtils::stows(charName);
    try
    {
        // TODO: Handle soft deletes
        auto accountsCollection = db->database("FLHook")["accounts"];

        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        mongocxx::options::find_one_and_delete deleteOptions;
        deleteOptions.projection(make_document(kvp("accountId", 1)));

        const auto findDoc = make_document(kvp("characterName", charName));
        const auto ret = accountsCollection.find_one_and_delete(findDoc.view(), deleteOptions);
        if (!ret.has_value())
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed), "Character deletion failed!");
        }
        auto oid = ret->view()["_id"].get_oid();
        const auto findAcc = make_document(kvp("_id", ret->view()["accountId"].get_string()));
        const auto deleteCharacter = make_document(kvp("$pull", make_document(kvp("characters", oid))));
        auto deleteResult = accountsCollection.update_one(findAcc.view(), deleteCharacter.view());
        if (!deleteResult.has_value() || deleteResult.value().modified_count() == 0)
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                            "Removing of character id from account failed.");
        }

        session.commit_transaction();
        account.characters.erase(charCodeString);
        account.internalAccount->numberOfCharacters = account.characters.size();
        Logger::Info(std::format(L"Successfully hard deleted character: {}", wideCharName));
        return true;
    }
    catch (bsoncxx::exception& ex)
    {
        Logger::Err(std::format(L"Error hard deleted character ({}): {}", wideCharName, StringUtils::stows(ex.what())));
    }
    catch (mongocxx::exception& ex)
    {
        Logger::Err(std::format(L"Error hard deleted character ({}): {}", wideCharName, StringUtils::stows(ex.what())));
    }
    session.abort_transaction();
    return false;
}

void AccountManager::Login(const std::wstring& wideAccountId, const ClientId client)
{
    accounts[client.GetValue()].loginSuccess = false;
    auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        auto accountsCollection = db->database("FLHook")["accounts"];

        std::string accId = StringUtils::wstos(wideAccountId);

        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        const auto findDoc = make_document(kvp("_id", accId));
        const auto accountBson = accountsCollection.find_one(findDoc.view());

        Account& account = accounts[client.GetValue()].account;
        // If account does not exist
        if (!accountBson.has_value())
        {
            // Create a new account with the provided ID
            account = { accId };
            auto bytes = rfl::bson::write(account);
            bsoncxx::document::view doc{ reinterpret_cast<uint8_t*>(bytes.data()), bytes.size() };

            accountsCollection.insert_one(doc);
            session.commit_transaction();
            accounts[client.GetValue()].loginSuccess = true;
        }
        else
        {
            auto& accountRaw = accountBson.value();
            auto acc = rfl::bson::read<Account>(accountRaw.view().data(), accountRaw.view().length());
            if (acc.error().has_value())
            {
                Logger::Err(std::format(L"Unable to read account: {}", wideAccountId));
                session.abort_transaction();
                return;
            }
            account = acc.value();
        }

        // Convert vector to bson array
        auto idArr = bsoncxx::builder::basic::array{};
        for (auto& [bytes] : account.characters)
        {
            idArr.append(bsoncxx::oid(reinterpret_cast<const char*>(bytes), bsoncxx::oid::size()));
        }

        bsoncxx::v_noabi::builder::basic::make_document();
        // Get all documents that are in the provided array
        auto filter = make_document(kvp("_id", make_document(kvp("$in", idArr))));

        for (auto cursor = accountsCollection.find(filter.view()); auto doc : cursor)
        {
            auto characterResult = rfl::bson::read<Character>(doc.data(), doc.length());
            if (characterResult.error().has_value())
            {
                Logger::Err(std::format(L"Error when loading a character: {}", StringUtils::stows(characterResult.error().value().what())));
                session.abort_transaction();
            }
            auto character = characterResult.value();
            if (character.characterName.empty())
            {
                Logger::Err(std::format(L"Error when reading character name for account: {}", wideAccountId));
                continue;
            }
            char charNameBuf[50];
            getFlName(charNameBuf, StringUtils::stows(character.characterName).c_str());
            std::string charFileName = charNameBuf;
            accounts[client.GetValue()].characters[charFileName] = character;
        }
        session.commit_transaction();
        accounts[client.GetValue()].loginSuccess = true;
    }
    catch (bsoncxx::exception& ex)
    {
        Logger::Err(std::format(L"Error logging in for account ({}): {}", wideAccountId, StringUtils::stows(ex.what())));
        session.abort_transaction();
    }
    catch (mongocxx::exception& ex)
    {
        Logger::Err(std::format(L"Error logging in for account ({}): {}", wideAccountId, StringUtils::stows(ex.what())));
        session.abort_transaction();
    }
}
