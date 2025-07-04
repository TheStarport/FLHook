#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "Core/ClientServerInterface.hpp"
#include "PCH.hpp"

#include <API/FLHook/AccountManager.hpp>
#include <Defs/Database/Account.hpp>
#include <mongocxx/exception/error_code.hpp>
#include <mongocxx/exception/write_exception.hpp>

bool AccountManager::SaveCharacter(ClientId client, Character& newCharacter, const bool isNewCharacter)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    const auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        const auto config = FLHook::GetConfig();
        auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];
        auto findCharDoc = make_document(kvp("characterName", newCharacter.characterName));

        if (isNewCharacter)
        {
            if (const auto checkCharNameDoc = charactersCollection.find_one(findCharDoc.view()); checkCharNameDoc.has_value())
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
            auto insertedDoc = charactersCollection.insert_one(document.view());

            if (!insertedDoc.has_value())
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed), "Unable to upsert a character.");
            }

            newCharacter._id = insertedDoc->inserted_id().get_oid().value;
            const auto findAccDoc = make_document(kvp("_id", newCharacter.accountId));
            const auto charUpdateDoc = make_document(kvp("$push", make_document(kvp("characters", insertedDoc->inserted_id()))));

            auto accountCollection = db->database(config->database.dbName)[config->database.accountsCollection];
            if (const auto updateResult = accountCollection.update_one(findAccDoc.view(), charUpdateDoc.view());
                !updateResult.has_value() || updateResult.value().modified_count() == 0)
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                                "Updating account during character creation failed.");
            }
        }
        else
        {
            const auto updateDoc = make_document(kvp("$set", document.view()));
            if (const auto updateResult = charactersCollection.update_one(findCharDoc.view(), updateDoc.view()); !updateResult.has_value())
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed), "Updating character failed.");
            }
        }
        session.commit_transaction();

        char buf[50];
        getFlName(buf, newCharacter.wideCharacterName.c_str());

        auto& character = accounts[client.GetValue()].characters.at(buf);
        character.characterDocument = document.extract();
        client.GetData().characterData = &character;

        return true;
    }
    catch (bsoncxx::exception& ex)
    {
        if (isNewCharacter)
        {
            ERROR(L"Error while creating character: {0}", { L"character", StringUtils::stows(ex.what()) });
        }
        else
        {
            ERROR(L"Error while updating character: {0}", { L"character", StringUtils::stows(ex.what()) });
        }
    }
    catch (mongocxx::exception& ex)
    {
        if (isNewCharacter)
        {
            ERROR(L"Error while creating character: {0}", { L"character", StringUtils::stows(ex.what()) });
        }
        else
        {
            ERROR(L"Error while updating character: {0}", { L"character", StringUtils::stows(ex.what()) });
        }
    }
    session.abort_transaction();
    return false;
}

concurrencpp::result<void> AccountManager::DeleteCharacter(const ClientId client, const std::wstring characterCode, CHARACTER_ID cid)
{
    THREAD_BACKGROUND;

    auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    auto& account = accounts.at(client.GetValue());
    std::string charCodeString = StringUtils::wstos(characterCode);
    std::string charName = account.characters.at(charCodeString).characterName;
    std::wstring wideCharName = StringUtils::stows(charName);
    try
    {
        const auto config = FLHook::GetConfig();
        // TODO: Handle soft deletes
        auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
        auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];

        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        mongocxx::options::find_one_and_delete deleteOptions;
        deleteOptions.projection(make_document(kvp("accountId", 1)));

        const auto findDoc = make_document(kvp("characterName", charName));
        const auto ret = charactersCollection.find_one_and_delete(findDoc.view(), deleteOptions);
        if (!ret.has_value())
        {
            throw std::runtime_error("Character deletion failed! Unable to find and delete character.");
        }

        auto oid = ret->view()["_id"].get_oid();
        const auto findAcc = make_document(kvp("_id", ret->view()["accountId"].get_string()));
        const auto deleteCharacter = make_document(kvp("$pull", make_document(kvp("characters", oid))));
        if (auto deleteResult = accountsCollection.update_one(findAcc.view(), deleteCharacter.view());
            !deleteResult.has_value() || deleteResult.value().modified_count() == 0)
        {
            throw std::runtime_error("Removing of character id from account failed.");
        }

        session.commit_transaction();
        account.characters.erase(charCodeString);
        account.internalAccount->numberOfCharacters = account.characters.size();
        INFO(L"Successfully hard deleted character: {0}", { L"characterName", wideCharName });

        THREAD_MAIN;

        IServerImplHook::DestroyCharacterCallback(client, cid);
    }
    catch (std::exception& ex)
    {
        session.abort_transaction();
        ERROR(L"Error hard deleting character {0}{1}", { L"character", wideCharName }, { L"error", StringUtils::stows(ex.what()) });
    }
}

// ReSharper disable once CppPassValueParameterByConstReference
concurrencpp::result<void> AccountManager::Login(SLoginInfo li, const ClientId client)
{
    std::wstring wideAccountId = li.account;
    accounts[client.GetValue()].loginSuccess = false;

    const auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        const auto config = FLHook::GetConfig();
        auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
        auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];

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
            account = {};
            account._id = accId;

            bsoncxx::builder::basic::document document;
            account.ToBson(document);

            accountsCollection.insert_one(document.view());
            session.commit_transaction();
            accounts[client.GetValue()].loginSuccess = true;
        }
        else
        {
            const auto accountData = accountBson.value();
            account = Account{ accountData.view() };
            account.accountData = accountData;
        }

        // Convert vector to bson array
        auto idArr = bsoncxx::builder::basic::array{};
        for (auto& id : account.characters)
        {
            idArr.append(id);
        }

        // Get all documents that are in the provided array
        const auto filter = make_document(kvp("_id", make_document(kvp("$in", idArr))));

        for (auto cursor = charactersCollection.find(filter.view()); const auto& doc : cursor)
        {
            auto character = Character{ doc };
            if (character.characterName.empty())
            {
                ERROR(L"Error when reading character name for account: {0}", { L"accountId", wideAccountId });
                continue;
            }

            char charNameBuf[50];
            getFlName(charNameBuf, StringUtils::stows(character.characterName).c_str());
            std::string charFileName = charNameBuf;
            auto& charRef = accounts[client.GetValue()].characters[charFileName] = character;

            charRef.characterDocument = doc;
            client.GetData().characterData = &charRef;
        }
        session.commit_transaction();
        accounts[client.GetValue()].loginSuccess = true;
    }
    catch (bsoncxx::exception& ex)
    {
        ERROR(L"Error logging in for account {0} {1} ", { L"accountId", wideAccountId }, { L"error", StringUtils::stows(ex.what()) })

        session.abort_transaction();
    }
    catch (mongocxx::exception& ex)
    {
        ERROR(L"Error logging in for account {0} {1} ", { L"accountId", wideAccountId }, { L"error", wideAccountId })
        session.abort_transaction();
    }

    THREAD_MAIN;

    IServerImplHook::DelayedLogin(li, client);
}

concurrencpp::result<std::wstring> AccountManager::CheckCharnameTaken(ClientId client, const std::wstring newName)
{
    THREAD_BACKGROUND;
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    std::wstring err;
    const auto db = FLHook::GetDbClient();
    try
    {
        const auto config = FLHook::GetConfig();
        auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];
        const auto findCharDoc = make_document(kvp("characterName", StringUtils::wstos(newName)));

        if (const auto checkCharNameDoc = charactersCollection.find_one(findCharDoc.view()); checkCharNameDoc.has_value())
        {
            err = L"Name already taken!";
        }
        else if (auto character = AccountManager::GetCurrentCharacterData(client); !character)
        {
            err = L"Error fetching the character, contact staff!";
        }
        else
        {
            character->characterName = StringUtils::wstos(newName);
        }
    }
    catch (mongocxx::exception& ex)
    {
        ERROR(
            L"Error checking for taken name", { L"accountId", std::wstring(client.GetCharacterName().Handle()) }, { L"error", StringUtils::stows(ex.what()) });
        err = L"Error while contacting database, contact staff!";
    }

    THREAD_MAIN;
    co_return err;
}

concurrencpp::result<void> AccountManager::Rename(std::wstring currName, std::wstring newName)
{
    THREAD_BACKGROUND;

    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_array;
    using bsoncxx::builder::basic::make_document;

    const auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        const auto config = FLHook::GetConfig();
        auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];
        const auto findCharDoc = make_document(kvp("characterName", StringUtils::wstos(currName)));

        if (const auto checkCharNameDoc = charactersCollection.find_one(findCharDoc.view()); !checkCharNameDoc.has_value())
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed), "Character doesn't exist when renaming!");
        }

        const auto charUpdateDoc = make_document(
            kvp("$set",
                make_document(kvp("characterName", StringUtils::wstos(newName)),
                              kvp("lastRenameTimestamp",
                                  bsoncxx::types::b_date{ static_cast<std::chrono::milliseconds>(TimeUtils::UnixTime<std::chrono::milliseconds>()) }))));
        if (const auto updateResult = charactersCollection.update_one(findCharDoc.view(), charUpdateDoc.view());
            !updateResult.has_value() || updateResult.value().modified_count() == 0)
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed), "Updating character during rename failed.");
        }

        session.commit_transaction();
    }
    catch (bsoncxx::exception& ex)
    {
        ERROR(L"BSON error renaming {0}{1}", { L"currentName", currName }, { L"error", StringUtils::stows(ex.what()) })
        session.abort_transaction();
    }
    catch (mongocxx::exception& ex)
    {
        ERROR(L"BSON error renaming {0}{1}", { L"currentName", currName }, { L"error", StringUtils::stows(ex.what()) })
        session.abort_transaction();
    }

    THREAD_MAIN;
}

concurrencpp::result<bool> AccountManager::ClearCharacterTransferCode(std::wstring charName)
{
    THREAD_BACKGROUND;

    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    const auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    bool result = false;
    try
    {
        const auto config = FLHook::GetConfig();
        auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];
        const auto findCharDoc = make_document(kvp("characterName", StringUtils::wstos(charName)));

        if (const auto checkCharNameDoc = charactersCollection.find_one(findCharDoc.view()); !checkCharNameDoc.has_value())
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                            "Character doesn't exist when clearing character transfer code!");
        }

        const auto charUpdateDoc = make_document(kvp("$unset", make_document(kvp("characterTransferCode", ""))));
        if (const auto updateResult = charactersCollection.update_one(findCharDoc.view(), charUpdateDoc.view());
            !updateResult.has_value() || updateResult.value().modified_count() == 0)
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                            "Updating character during clearing character transfer code failed.");
        }

        session.commit_transaction();
        result = true;
    }
    catch (bsoncxx::exception& ex)
    {

        ERROR(L"BSON Error clearing character transfer code {0}{1}", { L"characterName", charName }, { L"error", StringUtils::stows(ex.what()) })

        session.abort_transaction();
        result = false;
    }
    catch (mongocxx::exception& ex)
    {
        ERROR(L"BSON Error clearing character transfer code {0}{1}", { L"characterName", charName }, { L"error", StringUtils::stows(ex.what()) })
        session.abort_transaction();
        result = false;
    }

    THREAD_MAIN;
    co_return result;
}

concurrencpp::result<bool> AccountManager::SetCharacterTransferCode(std::wstring charName, std::wstring transferCode)
{
    THREAD_BACKGROUND;

    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    const auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    bool result;
    try
    {
        const auto config = FLHook::GetConfig();
        auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];
        const auto findCharDoc = make_document(kvp("characterName", StringUtils::wstos(charName)));

        if (const auto checkCharNameDoc = charactersCollection.find_one(findCharDoc.view()); !checkCharNameDoc.has_value())
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                            "Character doesn't exist when setting character transfer code!");
        }

        const auto charUpdateDoc = make_document(
            kvp("$set", make_document(kvp("characterName", StringUtils::wstos(charName)), kvp("characterTransferCode", StringUtils::wstos(transferCode)))));
        if (const auto updateResult = charactersCollection.update_one(findCharDoc.view(), charUpdateDoc.view());
            !updateResult.has_value() || updateResult.value().modified_count() == 0)
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                            "Updating character during setting character transfer code failed.");
        }

        session.commit_transaction();
        result = true;
    }
    catch (bsoncxx::exception& ex)
    {
        ERROR(L"MongoDB Error setting character transfer code {0}{1}", { L"characterName", charName }, { L"error", StringUtils::stows(ex.what()) })
        session.abort_transaction();
        result = false;
    }
    catch (mongocxx::exception& ex)
    {
        ERROR(L"MongoDB Error setting character transfer code {0}{1}", { L"characterName", charName }, { L"error", StringUtils::stows(ex.what()) })
        session.abort_transaction();
        result = false;
    }

    THREAD_MAIN;
    co_return result;
}

concurrencpp::result<std::wstring> AccountManager::TransferCharacter(const AccountId account, const std::wstring charName, const std::wstring characterCode)
{
    THREAD_BACKGROUND;

    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_array;
    using bsoncxx::builder::basic::make_document;

    const auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    std::wstring err;
    try
    {
        // check if character exists
        // check if character code matches
        // check if current account has capacity for another character

        // clang-format off
        const auto config = FLHook::GetConfig();
        auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
        auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];
            const auto findTransferCharacterDoc = make_document(
                kvp("$and",
                    make_array(
                        make_document(kvp("characterName", StringUtils::wstos(charName))),
                        make_document(kvp("characterTransferCode", StringUtils::wstos(characterCode)))
                    )
                )
            );

        const auto transferredCharacterDoc = charactersCollection.find_one(findTransferCharacterDoc.view());
        if (!transferredCharacterDoc.has_value())
        {
            session.abort_transaction();
            err = L"Character or transfer code was incorrect";
        }

        const auto transferCharacterOid = transferredCharacterDoc->find("_id")->get_oid();

        const auto findNewAccountDoc = make_document(kvp("_id", account.GetValue()));

        const auto updateNewAccountDoc = make_document(kvp("$push",
            make_document(kvp("characters", transferCharacterOid))));

        const auto updateOldAccountDoc = make_document(kvp("$pull",
             make_document(kvp("characters", transferCharacterOid))));

        const auto oldAccountId = transferredCharacterDoc->find("accountId")->get_string().value;
        const auto findOldAccountDoc = make_document(kvp("_id", oldAccountId));

        const auto findTransferredCharacterDoc = make_document(kvp("_id", transferCharacterOid));
        const auto clearCharacterTransferCodeDoc = make_document(kvp("$unset", make_document(kvp("characterTransferCode", ""))));

        // clang-format on
        if (auto updatedDocs = accountsCollection.update_one(findNewAccountDoc.view(), updateNewAccountDoc.view()); !updatedDocs->modified_count())
        {
            session.abort_transaction();
            err = L"Character transfer failed on updating of target account!";
        }
        else if (updatedDocs = charactersCollection.update_one(findTransferredCharacterDoc.view(), clearCharacterTransferCodeDoc.view());
                 !updatedDocs->modified_count())
        {
            session.abort_transaction();
            err = L"Character transfer failed on updating of the character!";
        }
        else if (updatedDocs = accountsCollection.update_one(findOldAccountDoc.view(), updateOldAccountDoc.view()); !updatedDocs->modified_count())
        {
            session.abort_transaction();
            err = L"Character transfer failed on updating of target account!";
        }
        else
        {
            session.commit_transaction();
        }
    }
    catch (bsoncxx::exception& ex)
    {
        ERROR(L"BSON Error setting character transfer code {0}{1}", { L"characterName", charName }, { L"error", StringUtils::stows(ex.what()) })

        err = L"Database error, report to server staff";
        session.abort_transaction();
    }
    catch (mongocxx::exception& ex)
    {
        ERROR(L"MongoDB Error setting character transfer code {0}{1}", { L"characterName", charName }, { L"error", StringUtils::stows(ex.what()) })

        err = L"Database error, report to server staff";
        session.abort_transaction();
    }

    THREAD_MAIN;
    co_return err;
}
