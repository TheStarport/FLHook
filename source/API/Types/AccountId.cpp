#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/Types/AccountId.hpp"

AccountId::operator bool() const { return accountId.empty(); }

ClientData* AccountId::IsOnline() const
{
    for (auto& client : FLHook::Clients())
    {
        if (client.account && client.account->_id == accountId)
        {
            return &client;
        }
    }

    return nullptr;
}

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

std::optional<AccountId> AccountId::GetAccountFromClient(const ClientId client)
{
    if (!client)
    {
        return std::nullopt;
    }

    const auto& data = client.GetData();
    if (!data.account)
    {
        return std::nullopt;
    }

    AccountId acc;
    acc.accountId = data.account->_id;
    return acc;
}

std::optional<AccountId> AccountId::GetAccountFromCharacterName(const std::wstring_view characterName)
{
    for (const auto& client : FLHook::Clients())
    {
        if (client.characterName == characterName && client.account)
        {
            AccountId acc;
            acc.accountId = client.account->_id;
            return acc;
        }
    }

    const auto db = FLHook::GetDbClient();

    const auto config = FLHook::GetConfig();
    auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];
    const auto findCharDoc = make_document(kvp("characterName", StringUtils::wstos(characterName)));

    mongocxx::options::find options;
    options.projection(make_document(kvp("_id", 1)));

    const auto character = charactersCollection.find_one(findCharDoc.view(), options);
    if (!character.has_value())
    {
        return std::nullopt;
    }

    AccountId acc;
    acc.accountId = character.value()["accountId"].get_string();
    return acc;
}

std::optional<AccountId> AccountId::GetAccountFromAccountId(const std::wstring_view accountId)
{
    auto accountIdString = StringUtils::wstos(accountId);
    for (const auto& client : FLHook::Clients())
    {
        if (client.account && client.account->_id == accountIdString)
        {
            AccountId acc;
            acc.accountId = client.account->_id;
            return acc;
        }
    }

    const auto db = FLHook::GetDbClient();

    const auto config = FLHook::GetConfig();
    auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
    const auto findAccDoc = make_document(kvp("_id", accountIdString));

    mongocxx::options::find options;
    options.projection(make_document(kvp("_id", 1)));

    const auto databaseAccount = accountsCollection.find_one(findAccDoc.view(), options);
    if (!databaseAccount.has_value())
    {
        return std::nullopt;
    }

    AccountId acc;
    acc.accountId = databaseAccount.value()["_id"].get_string();
    return acc;
}

std::string_view AccountId::GetValue() const { return accountId; }

bool AccountId::IsAdmin() const
{
    if (const auto client = IsOnline(); client)
    {
        return client->account->gameRoles.has_value() && !client->account->gameRoles.value().empty();
    }

    const auto db = FLHook::GetDbClient();

    const auto config = FLHook::GetConfig();
    auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
    auto findAccDoc = make_document(kvp("_id", accountId));

    mongocxx::pipeline pipeline{};
    pipeline.match(make_document(kvp("_id", accountId)));
    pipeline.count("gameRoles");

    for (auto cursor = accountsCollection.aggregate(pipeline, mongocxx::options::aggregate{}); const auto data : cursor)
    {
        return data["gameRoles"].get_int32() != 0;
    }

    return false;
}

Action<void> AccountId::UnBan() const
{
    const auto db = FLHook::GetDbClient();

    const auto config = FLHook::GetConfig();
    auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
    const auto findAccDoc = make_document(kvp("_id", accountId));
    static auto updateDoc = make_document(kvp("$set", make_document(kvp("banned", false))), kvp("$unset", make_document(kvp("scheduledUnbanDate", ""))));

    if (const auto responseDoc = accountsCollection.update_one(findAccDoc.view(), updateDoc.view()); responseDoc->modified_count() == 0)
    {
        // TODO: Err
        return { {} };
    }

    return { {} };
}

Action<void> AccountId::Ban(const uint tempBanDays) const
{
    const auto db = FLHook::GetDbClient();
    const auto config = FLHook::GetConfig();
    auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
    const auto findAccDoc = make_document(kvp("_id", accountId));

    bsoncxx::document::view_or_value banUpdateDoc;
    if (tempBanDays)
    {
        auto time = TimeUtils::UnixTime<std::chrono::seconds>() + static_cast<int64>(60 * 60 * 24 * tempBanDays);
        banUpdateDoc = make_document(kvp("$set", make_document(kvp("banned", true), kvp("scheduledUnbanDate", time))));
    }
    else
    {
        static auto permaBanDoc = make_document(kvp("$set", make_document(kvp("banned", true))), kvp("$unset", make_document(kvp("scheduledUnbanDate", ""))));
        banUpdateDoc = permaBanDoc.view();
    }

    if (const auto responseDoc = accountsCollection.update_one(findAccDoc.view(), banUpdateDoc.view()); responseDoc->modified_count() == 0)
    {
        // TODO: Err
        return { {} };
    }

    if (const auto client = IsOnline(); client)
    {
        (void)client->id.Kick();
    }

    return { {} };
}

Action<void> AccountId::DeleteCharacter(const std::wstring_view name) const
{
    if (const auto client = IsOnline(); client)
    {
        (void)client->id.Kick();
    }

    const auto db = FLHook::GetDbClient();
    const auto config = FLHook::GetConfig();
    auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
    auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];

    const auto findCharDoc = make_document(kvp("characterName", StringUtils::wstos(name)));

    mongocxx::options::find_one_and_delete deleteOptions;
    deleteOptions.projection(make_document(kvp("_id", 1)));

    const auto foundCharDoc = charactersCollection.find_one_and_delete(findCharDoc.view(), deleteOptions);
    if (!foundCharDoc.has_value())
    {
        return { {} };
    }

    const auto findAccDoc = make_document(kvp("_id", accountId));
    const auto updateAccDoc = make_document(kvp("$pull", make_document(kvp("characters", foundCharDoc.value()["_id"].get_oid()))));

    if (const auto updateResponse = accountsCollection.update_one(findAccDoc.view(), updateAccDoc.view()); updateResponse->modified_count() == 0)
    {
        return { {} };
    }

    return { {} };
}

Action<bool> AccountId::HasRole(const std::wstring_view role) const
{
    if (role.empty())
    {
        return { false };
    }
    const auto account = IsOnline();
    if (!account)
    {
        return { false };
    }

    if (!account->account->gameRoles.has_value())
    {
        return { false };
    }

    const auto strRole = StringUtils::wstos(role);
    for (const std::string_view gameRole : account->account->gameRoles.value())
    {
        if (gameRole == "superadmin" || StringUtils::CompareCaseInsensitive(gameRole, std::string_view(strRole)))
        {
            return { true };
        }
    }
    return { false };
}
