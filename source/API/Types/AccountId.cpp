#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/Types/AccountId.hpp"

#include "API/FLHook/BsonHelper.hpp"
#include "Defs/FLHookConfig.hpp"

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

concurrencpp::result<std::optional<AccountId>> AccountId::GetAccountFromCharacterName(const CharacterId& characterName)
{
    const CharacterId characterId = characterName;
    THREAD_MAIN;

    for (const auto& client : FLHook::Clients())
    {
        if (client.characterId == characterId && client.account)
        {
            AccountId acc;
            acc.accountId = client.account->_id;
            co_return acc;
        }
    }

    const auto db = FLHook::GetDbClient();

    const auto config = FLHook::GetConfig();
    auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];
    const auto findCharDoc = B_MDOC(B_KVP("characterName", StringUtils::wstos(characterId.GetValue())));

    mongocxx::options::find options;
    options.projection(B_MDOC(B_KVP("accountId", 1)));

    THREAD_BACKGROUND;

    const auto character = charactersCollection.find_one(findCharDoc.view(), options);
    if (!character.has_value())
    {
        co_return std::nullopt;
    }

    AccountId acc;
    acc.accountId = character.value()["accountId"].get_string();
    co_return acc;
}

concurrencpp::result<std::optional<AccountId>> AccountId::GetAccountFromAccountId(const std::wstring_view accountId)
{
    THREAD_MAIN;

    auto accountIdString = StringUtils::wstos(accountId);
    for (const auto& client : FLHook::Clients())
    {
        if (client.account && client.account->_id == accountIdString)
        {
            AccountId acc;
            acc.accountId = client.account->_id;
            co_return acc;
        }
    }

    const auto db = FLHook::GetDbClient();

    const auto config = FLHook::GetConfig();
    auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
    const auto findAccDoc = B_MDOC(B_KVP("_id", accountIdString));

    mongocxx::options::find options;
    options.projection(B_MDOC(B_KVP("_id", 1)));

    THREAD_BACKGROUND;

    const auto databaseAccount = accountsCollection.find_one(findAccDoc.view(), options);
    if (!databaseAccount.has_value())
    {
        co_return std::nullopt;
    }

    AccountId acc;
    acc.accountId = databaseAccount.value()["_id"].get_string();
    co_return acc;
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
    auto findAccDoc = B_MDOC(B_KVP("_id", accountId));

    mongocxx::pipeline pipeline{};
    pipeline.match(B_MDOC(B_KVP("_id", accountId)));
    pipeline.count("gameRoles");

    for (auto cursor = accountsCollection.aggregate(pipeline, mongocxx::options::aggregate{}); const auto data : cursor)
    {
        return data["gameRoles"].get_int32() != 0;
    }

    return false;
}

concurrencpp::result<Action<void>> AccountId::UnBan() const
{
    const auto db = FLHook::GetDbClient();

    const auto config = FLHook::GetConfig();
    auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
    const auto findAccDoc = B_MDOC(B_KVP("_id", accountId));
    static auto updateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("banned", false))), B_KVP("$unset", B_MDOC(B_KVP("scheduledUnbanDate", ""))));

    if (const auto responseDoc = accountsCollection.update_one(findAccDoc.view(), updateDoc.view()); responseDoc->modified_count() == 0)
    {
        // TODO: Err
        return { {} };
    }

    return { {} };
}

concurrencpp::result<Action<void>> AccountId::Ban(const uint tempBanDays) const
{
    const auto db = FLHook::GetDbClient();
    const auto config = FLHook::GetConfig();
    auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
    const auto findAccDoc = B_MDOC(B_KVP("_id", accountId));

    B_VOV banUpdateDoc;
    if (tempBanDays)
    {
        auto time = TimeUtils::UnixTime<std::chrono::seconds>() + static_cast<int64>(60 * 60 * 24 * tempBanDays);
        banUpdateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("banned", true), B_KVP("scheduledUnbanDate", time))));
    }
    else
    {
        static auto permaBanDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("banned", true))), B_KVP("$unset", B_MDOC(B_KVP("scheduledUnbanDate", ""))));
        banUpdateDoc = permaBanDoc.view();
    }

    THREAD_BACKGROUND;
    if (const auto responseDoc = accountsCollection.update_one(findAccDoc.view(), banUpdateDoc.view()); responseDoc->modified_count() == 0)
    {
        // TODO: Err
        co_return Action<void>{ {} };
    }

    THREAD_MAIN;

    if (const auto client = IsOnline(); client)
    {
        co_return client->id.Kick();
    }

    co_return Action<void>{ {} };
}

concurrencpp::result<Action<void>> AccountId::DeleteCharacter(const std::wstring_view name) const
{
    auto banTarget = std::wstring{ name };

    THREAD_MAIN;
    if (const auto client = IsOnline(); client)
    {
        (void)client->id.Kick();
    }

    THREAD_BACKGROUND;

    const auto db = FLHook::GetDbClient();
    const auto config = FLHook::GetConfig();
    auto accountsCollection = db->database(config->database.dbName)[config->database.accountsCollection];
    auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];

    const auto findCharDoc = B_MDOC(B_KVP("characterName", StringUtils::wstos(banTarget)));

    mongocxx::options::find_one_and_delete deleteOptions;
    deleteOptions.projection(B_MDOC(B_KVP("_id", 1)));

    const auto foundCharDoc = charactersCollection.find_one_and_delete(findCharDoc.view(), deleteOptions);
    if (!foundCharDoc.has_value())
    {
        // TODO: err
        co_return Action<void>{ {} };
    }

    const auto findAccDoc = B_MDOC(B_KVP("_id", accountId));
    const auto updateAccDoc = B_MDOC(B_KVP("$pull", B_MDOC(B_KVP("characters", foundCharDoc.value()["_id"].get_oid()))));

    if (const auto updateResponse = accountsCollection.update_one(findAccDoc.view(), updateAccDoc.view()); updateResponse->modified_count() == 0)
    {
        // TODO: Change return type to mongo result
        co_return Action<void>{ {} };
    }

    co_return Action<void>{ {} };
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
