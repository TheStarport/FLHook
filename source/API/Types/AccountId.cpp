#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"

#include "API/Types/AccountId.hpp"
#include "API/Utils/TempBan.hpp"

#include "API/FLHook/Database.hpp"

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
using bsoncxx::builder::basic::make_document;
using bsoncxx::builder::basic::make_array;

std::optional<AccountId> AccountId::GetAccountFromClient(ClientId client)
{
    if (!client) 
    {
        return std::nullopt;
    }

    auto& data = client.GetData();
    if (!data.account)
    {
        return std::nullopt;
    }

    AccountId acc;
    acc.accountId = data.account->_id;
    return acc;
}

std::optional<AccountId> AccountId::GetAccountFromCharacterName(std::wstring_view characterName)
{
    for (auto& client : FLHook::Clients())
    {
        if (client.characterName == characterName && client.account)
        {
            AccountId acc;
            acc.accountId = client.account->_id;
            return acc;
        }
    }


    const auto db = FLHook::GetDbClient();

    auto accounts = db->database("FLHook")["accounts"];
    auto findAccDoc = make_document(kvp("characterName", StringUtils::wstos(characterName)));

    mongocxx::options::find options;
    options.projection(make_document(kvp("_id", 1)));

    const auto databaseAccount = accounts.find_one(findAccDoc.view(), options); 
    if (!databaseAccount.has_value())
    {
        return std::nullopt;
    }

    AccountId acc;
    acc.accountId = databaseAccount.value()["_id"].get_string();
    return acc;
}

std::optional<AccountId> AccountId::GetAccountFromAccountId(std::wstring_view accountId)
{
    auto accountIdString = StringUtils::wstos(accountId);
    for (auto& client : FLHook::Clients())
    {
        if (client.account && client.account->_id == accountIdString)
        {
            AccountId acc;
            acc.accountId = client.account->_id;
            return acc;
        }
    }

    const auto db = FLHook::GetDbClient();

    auto accounts = db->database("FLHook")["accounts"];
    auto findAccDoc = make_document(kvp("_id", accountIdString));

    mongocxx::options::find options;
    options.projection(make_document(kvp("_id", 1)));

    const auto databaseAccount = accounts.find_one(findAccDoc.view(), options); 
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
    auto client = IsOnline();
    if (client) 
    {
        return client->account->gameRoles.has_value() && !client->account->gameRoles.value().empty();
    }

    const auto db = FLHook::GetDbClient();

    auto accounts = db->database("FLHook")["accounts"];
    auto findAccDoc = make_document(kvp("_id", accountId));

    mongocxx::pipeline pipeline{};
    pipeline.match(make_document(kvp("_id", accountId)));
    pipeline.count("gameRoles");

    auto cursor = accounts.aggregate(pipeline, mongocxx::options::aggregate{});

    for (auto data : cursor)
    {
        return data["gameRoles"].get_int32() != 0;
    }

    return false;
}

Action<void, Error> AccountId::UnBan() const
{
    const auto db = FLHook::GetDbClient();

    auto accounts = db->database("FLHook")["accounts"];
    auto findAccDoc = make_document(kvp("_id", accountId));
    static auto updateDoc = make_document(kvp("$set", make_document(kvp("banned", false))), kvp("$unset", make_document(kvp("scheduledUnbanDate", ""))));

    const auto responseDoc = accounts.update_one(findAccDoc.view(), updateDoc.view());
    if(responseDoc->modified_count() == 0)
    {
        // TODO: Err
        return {{}};
    }

    return {{}};
}

Action<void, Error> AccountId::Ban(uint tempBanDays) const
{
    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];
    auto findAccDoc = make_document(kvp("_id", accountId));

    bsoncxx::document::view_or_value banUpdateDoc;
    if (tempBanDays)
    {
        auto time = static_cast<int64>(TimeUtils::UnixTime<std::chrono::seconds>() + static_cast<int64>(60 * 60 * 24 * tempBanDays));
        banUpdateDoc = make_document(kvp("$set", make_document(kvp("banned", true), kvp("scheduledUnbanDate", time))));
    }
    else 
    {
        static auto permaBanDoc = make_document(kvp("$set", make_document(kvp("banned", true))), kvp("$unset", make_document(kvp("scheduledUnbanDate", ""))));
        banUpdateDoc = permaBanDoc.view();
    }

    const auto responseDoc = accounts.update_one(findAccDoc.view(), banUpdateDoc.view());
    if(responseDoc->modified_count() == 0)
    {
        // TODO: Err
        return {{}};
    }

    auto client = IsOnline();
    if (client) 
    {
        client->id.Kick();
    }

    return { {} };
}


Action<void, Error> AccountId::DeleteCharacter(std::wstring_view name) const
{
    auto client = IsOnline();
    if (client) 
    {
        client->id.Kick();
    }

    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];

    auto findCharDoc = make_document(kvp("characterName", StringUtils::wstos(name)));

    mongocxx::options::find_one_and_delete deleteOptions;
    deleteOptions.projection(make_document(kvp("_id", 1)));

    auto foundCharDoc = accounts.find_one_and_delete(findCharDoc.view(), deleteOptions);
    if (!foundCharDoc.has_value())
    {
        return {{}};
    }

    auto findAccDoc = make_document(kvp("_id", accountId));
    auto updateAccDoc = make_document(kvp("$pull", make_document(kvp("characters", foundCharDoc.value()["_id"].get_oid()))));
    auto updateResponse = accounts.update_one(findAccDoc.view(), updateAccDoc.view());

    if (updateResponse->modified_count() == 0)
    {
        return {{}};
    }

    return { {} };
}

Action<void, Error> AccountId::AddRoles(const std::vector<std::wstring_view>& roles)
{

    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];
    auto findAccDoc = make_document(kvp("_id", accountId));

    bsoncxx::builder::basic::array arrayBuilder;
    for(auto& role : roles)
    {
        arrayBuilder.append(StringUtils::wstos(role));
    }

    auto roleUpdateDoc = make_document(kvp("$push", make_document(kvp("gameRoles", arrayBuilder.view()))));
    auto updateResponse = accounts.update_one(findAccDoc.view(), roleUpdateDoc.view());

    if (updateResponse->modified_count() == 0)
    {
        return {{}};
    }

    return { {} };
}

Action<void, Error> AccountId::RemoveRoles(const std::vector<std::wstring_view>& roles, bool clear)
{
    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];
    auto findAccDoc = make_document(kvp("_id", accountId));

    bsoncxx::builder::basic::array arrayBuilder;
    for(auto& role : roles)
    {
        arrayBuilder.append(StringUtils::wstos(role));
    }

    auto roleUpdateDoc = make_document(kvp("$pull", make_document(kvp("gameRoles", arrayBuilder.view()))));
    auto updateResponse = accounts.update_one(findAccDoc.view(), roleUpdateDoc.view());

    if (updateResponse->modified_count() == 0)
    {
        return {{}};
    }

    return { {} };
}

Action<void, Error> AccountId::SetRoles(const std::vector<std::wstring_view>& roles)
{
    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];
    auto findAccDoc = make_document(kvp("_id", accountId));

    bsoncxx::builder::basic::array arrayBuilder;
    for(auto& role : roles)
    {
        arrayBuilder.append(StringUtils::wstos(role));
    }

    auto roleUpdateDoc = make_document(kvp("$set", make_document(kvp("gameRoles", arrayBuilder.view()))));
    auto updateResponse = accounts.update_one(findAccDoc.view(), roleUpdateDoc.view());

    if (updateResponse->modified_count() == 0)
    {
        return {{}};
    }

    return { {} };
}

Action<void, Error> AccountId::SetCash(std::wstring_view characterName, int64 amount) const
{
    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];
    auto findAccDoc = make_document(kvp("_id", accountId));

    auto roleUpdateDoc = make_document(kvp("$set", make_document(kvp("cash", amount))));
    auto updateResponse = accounts.update_one(findAccDoc.view(), roleUpdateDoc.view());

    if (updateResponse->modified_count() == 0)
    {
        return {{}};
    }

    return { {} };
}
