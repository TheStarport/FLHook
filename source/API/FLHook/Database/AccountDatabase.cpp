#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "Core/ClientServerInterface.hpp"
#include "PCH.hpp"
#include "Core/PluginManager.hpp"
#include "Defs/FLHookConfig.hpp"

#include <API/FLHook/AccountManager.hpp>
#include <Defs/Database/DbAccount.hpp>
#include <mongocxx/exception/error_code.hpp>
#include <mongocxx/exception/write_exception.hpp>

bool AccountManager::SaveCharacter(ClientId client, Character& newCharacter, const bool isNewCharacter)
{
    const auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        const auto config = FLHook::GetConfig();
        auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];
        auto findCharDoc = B_MDOC(B_KVP("characterName", newCharacter.characterName));

        if (isNewCharacter)
        {
            if (const auto checkCharNameDoc = charactersCollection.find_one(findCharDoc.view()); checkCharNameDoc.has_value())
            {
                throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                                "Character already exists while trying to create new character");
            }
        }

        auto wideCharacterName = StringUtils::stows(newCharacter.characterName);
        B_DOC document;
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
            const auto findAccDoc = B_MDOC(B_KVP("_id", newCharacter.accountId));
            const auto charUpdateDoc = B_MDOC(B_KVP("$push", B_MDOC(B_KVP("characters", insertedDoc->inserted_id()))));

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
            const auto updateDoc = B_MDOC(B_KVP("$set", document.view()));
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
            ERROR("Error while creating character: {{character}} {{ex}}", { "ex", ex.what() }, { "character", newCharacter.characterName });
        }
        else
        {
            ERROR("Error while updating character: {{character}} {{ex}}", { "ex", ex.what() }, { "character", newCharacter.characterName });
        }
    }
    catch (mongocxx::exception& ex)
    {
        if (isNewCharacter)
        {
            ERROR("Error while creating character: {{character}} {{ex}}", { "ex", ex.what() }, { "character", newCharacter.characterName });
        }
        else
        {
            ERROR("Error while updating character: {{character}} {{ex}}", { "ex", ex.what() }, { "character", newCharacter.characterName });
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

        mongocxx::options::find_one_and_delete deleteOptions;
        deleteOptions.projection(B_MDOC(B_KVP("accountId", 1)));

        const auto findDoc = B_MDOC(B_KVP("characterName", charName));
        const auto ret = charactersCollection.find_one_and_delete(findDoc.view(), deleteOptions);
        if (!ret.has_value())
        {
            throw std::runtime_error("Character deletion failed! Unable to find and delete character.");
        }

        auto oid = ret->view()["_id"].get_oid();
        const auto findAcc = B_MDOC(B_KVP("_id", ret->view()["accountId"].get_string()));
        const auto deleteCharacter = B_MDOC(B_KVP("$pull", B_MDOC(B_KVP("characters", oid))));
        if (auto deleteResult = accountsCollection.update_one(findAcc.view(), deleteCharacter.view());
            !deleteResult.has_value() || deleteResult.value().modified_count() == 0)
        {
            throw std::runtime_error("Removing of character id from account failed.");
        }

        session.commit_transaction();
        account.characters.erase(charCodeString);
        account.internalAccount->numberOfCharacters = account.characters.size();
        INFO("Successfully hard deleted character: {{characterName}}", { "characterName", wideCharName });

        THREAD_MAIN;

        IServerImplHook::DestroyCharacterCallback(client, cid);
    }
    catch (std::exception& ex)
    {
        session.abort_transaction();
        ERROR("Error hard deleting character {{character}} {{ex}}", { "character", wideCharName }, { "ex", ex.what() });
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

        const auto findDoc = B_MDOC(B_KVP("_id", accId));
        const auto accountBson = accountsCollection.find_one(findDoc.view());

        accounts[client.GetValue()].characters = std::unordered_map<std::string, Character>{}; // reset account characters list

        DbAccount& account = accounts[client.GetValue()].account;

        // If account does not exist
        if (!accountBson.has_value())
        {
            // Create a new account with the provided ID
            account = {};
            account._id = accId;

            B_DOC document;
            account.ToBson(document);

            accountsCollection.insert_one(document.view());
            session.commit_transaction();
            accounts[client.GetValue()].loginSuccess = true;
        }
        else
        {
            const auto accountData = accountBson.value();
            account = DbAccount{ accountData.view() };
            account.accountData = accountData;
        }

        // Convert vector to bson array
        auto idArr = B_ARR{};
        for (auto& id : account.characters)
        {
            idArr.append(id);
        }

        // Get all documents that are in the provided array
        const auto filter = B_MDOC(B_KVP("_id", B_MDOC(B_KVP("$in", idArr))));

        for (auto cursor = charactersCollection.find(filter.view()); const auto& doc : cursor)
        {
            auto character = Character{ doc };
            if (character.characterName.empty())
            {
                ERROR("Error when reading character name for account: {{accountId}}", { "accountId", wideAccountId });
                continue;
            }

            char charNameBuf[50];
            getFlName(charNameBuf, StringUtils::stows(character.characterName).c_str());
            std::string charFileName = charNameBuf;
            auto& charRef = accounts[client.GetValue()].characters[charFileName] = character;

            charRef.characterDocument = doc;
            client.GetData().characterData = &charRef; // TODO this seems questionable - can't the user select any of them?
        }
        session.commit_transaction();
        accounts[client.GetValue()].loginSuccess = true;
    }
    catch (bsoncxx::exception& ex)
    {
        ERROR("Error logging in for account {{accountId}} {{ex}} ", { "accountId", wideAccountId }, { "ex", ex.what() });

        session.abort_transaction();
    }
    catch (mongocxx::exception& ex)
    {
        ERROR("Error logging in for account {{accountId}} {{ex}} ", { "accountId", wideAccountId }, { "ex", ex.what() });
        session.abort_transaction();
    }

    THREAD_MAIN;

    IServerImplHook::DelayedLogin(li, client);
}

concurrencpp::result<bool> AccountManager::UpdateCharacter(std::wstring charName, bsoncxx::v_noabi::document::value charUpdateDoc, std::string logDescription)
{
    THREAD_BACKGROUND;

    const auto db = FLHook::GetDbClient();
    auto session = db->start_session();
    session.start_transaction();

    bool result = false;
    try
    {
        const auto config = FLHook::GetConfig();
        auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];
        const auto findCharDoc = B_MDOC(B_KVP("characterName", StringUtils::wstos(charName)));

        if (const auto checkCharNameDoc = charactersCollection.find_one(findCharDoc.view()); !checkCharNameDoc.has_value())
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                            std::format("Character doesn't exist when {}:", logDescription));
        }

        const auto updateResult = charactersCollection.update_one(findCharDoc.view(), charUpdateDoc.view());
        if (!updateResult.has_value())
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                            std::format("Updating character during {} failed due to lack of update result value:", logDescription));
        }

        if (updateResult.value().modified_count() == 0)
        {
            throw mongocxx::write_exception(make_error_code(mongocxx::error_code::k_server_response_malformed),
                                            std::format("Updating character during {} failed due to no rows being modified:", logDescription));
        }

        session.commit_transaction();
        result = true;
    }
    catch (bsoncxx::exception& ex)
    {
        ERROR("BSON error during {{logDescription}}: {{charName}} {{ex}}", { "charName", charName }, { "ex", ex.what() }, { "logDescription", logDescription });
        session.abort_transaction();
    }
    catch (mongocxx::exception& ex)
    {
        ERROR("MongoDB error during {{logDescription}}: {{charName}} {{ex}}",
              { "charName", charName },
              { "ex", ex.what() },
              { "logDescription", logDescription });
        session.abort_transaction();
    }

    THREAD_MAIN;
    co_return result;
}

concurrencpp::result<cpp::result<void, std::wstring>> AccountManager::UpdateAccount(std::wstring charName, bsoncxx::v_noabi::document::value updateAccountDoc,
                                                                                    std::string logDescription)
{
    THREAD_BACKGROUND;

    const auto dbClient = FLHook::GetDbClient();
    auto session = dbClient->start_session();
    session.start_transaction();

    try
    {
        const auto config = FLHook::GetConfig();
        auto accountCollection = dbClient->database(config->database.dbName).collection(config->database.accountsCollection);
        auto charactersCollection = dbClient->database(config->database.dbName).collection(config->database.charactersCollection);

        const auto findCharacterDoc = B_MDOC(B_KVP("characterName", StringUtils::wstos(charName)));

        const auto characterResult = charactersCollection.find_one(findCharacterDoc.view());
        if (!characterResult.has_value())
        {
            THREAD_MAIN;
            co_return cpp::fail(L"Provided character name not found");
        }

        const auto findAccountDoc = B_MDOC(B_KVP("_id", characterResult->find("accountId")->get_string()));
        const auto updateResponse = accountCollection.update_one(findAccountDoc.view(), updateAccountDoc.view());
        if (updateResponse->modified_count() == 0)
        {
            THREAD_MAIN;
            co_return cpp::fail(L"Failed to update account. Account was either invalid or already contained/did not contain role(s).");
        }
    }
    catch (bsoncxx::exception& ex)
    {
        ERROR("BSON error during {{logDescription}}: {{charName}} {{ex}}", { "charName", charName }, { "ex", ex.what() }, { "logDescription", logDescription });
        session.abort_transaction();
    }
    catch (mongocxx::exception& ex)
    {
        ERROR("MongoDB error during {{logDescription}}: {{charName}} {{ex}}",
              { "charName", charName },
              { "ex", ex.what() },
              { "logDescription", logDescription });
        session.abort_transaction();
    }

    THREAD_MAIN;
    cpp::result<void, std::wstring> ret; // success object - usually we would `return {};` for this, but concurrencpp...
    co_return ret;
}

concurrencpp::result<bool> AccountManager::SaveSavedMsgs(std::wstring charName, std::array<std::string, 10> presetMsgs)
{
    B_ARR arr;
    for (std::string msg : presetMsgs)
    {
        arr.append(msg);
    }

    const auto charUpdateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("presetMsgs", arr))));

    co_return AccountManager::UpdateCharacter(charName, charUpdateDoc, "setting preset messages list");
}
