#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "PCH.hpp"

#include <API/FLHook/AccountManager.hpp>
#include <Defs/Database/Account.hpp>
#include <mongocxx/exception/error_code.hpp>
#include <mongocxx/exception/write_exception.hpp>

bool AccountManager::SaveCharacter(ClientId client, Character& newCharacter, const bool isNewCharacter)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_array;
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

        auto wideCharacterName = StringUtils::stows(newCharacter.characterName);
        bsoncxx::builder::basic::document document;
        newCharacter.ToBson(document);

        CallPlugins(&Plugin::OnCharacterSave, client, wideCharacterName, document);

        // Update account character list if new character
        if (isNewCharacter)
        {
            auto insertedDoc = accounts.insert_one(document.view());

            if (!insertedDoc.has_value())
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed), "Unable to upsert a character.");
            }

            newCharacter._id = insertedDoc->inserted_id().get_oid().value;
            const auto findAccDoc = make_document(kvp("_id", newCharacter.accountId));
            const auto charUpdateDoc = make_document(kvp("$push", make_document(kvp("characters", insertedDoc->inserted_id()))));
            if (const auto updateResult = accounts.update_one(findAccDoc.view(), charUpdateDoc.view());
                !updateResult.has_value() || updateResult.value().modified_count() == 0)
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                                "Updating account during character creation failed.");
            }
        }
        else
        {
            const auto updateDoc = make_document(kvp("$set", document.view()));
            if (const auto updateResult = accounts.update_one(findCharDoc.view(), updateDoc.view()); !updateResult.has_value())
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed), "Updating character failed.");
            }
        }
        session.commit_transaction();

        client.GetData().characterData = document.extract();

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
        if (auto deleteResult = accountsCollection.update_one(findAcc.view(), deleteCharacter.view());
            !deleteResult.has_value() || deleteResult.value().modified_count() == 0)
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
            account = { };
            account._id = accId;

            bsoncxx::builder::basic::document document;
            account.ToBson(document);

            accountsCollection.insert_one(document.view());
            session.commit_transaction();
            accounts[client.GetValue()].loginSuccess = true;
        }
        else
        {
            auto& accountRaw = accountBson.value();
            account = Account{ accountRaw.view() };
        }

        // Convert vector to bson array
        auto idArr = bsoncxx::builder::basic::array{};
        for (auto& id : account.characters)
        {
            idArr.append(id);
        }

        // Get all documents that are in the provided array
        const auto filter = make_document(kvp("_id", make_document(kvp("$in", idArr))));

        for (auto cursor = accountsCollection.find(filter.view()); const auto doc : cursor)
        {
            auto character = Character{doc};
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
