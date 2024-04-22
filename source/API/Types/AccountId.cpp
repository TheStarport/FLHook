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

    auto accounts = db->database("FLHook")["accounts"];
    const auto findAccDoc = make_document(kvp("characterName", StringUtils::wstos(characterName)));

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

    auto accounts = db->database("FLHook")["accounts"];
    const auto findAccDoc = make_document(kvp("_id", accountIdString));

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
    if (const auto client = IsOnline(); client)
    {
        return client->account->gameRoles.has_value() && !client->account->gameRoles.value().empty();
    }

    const auto db = FLHook::GetDbClient();

    auto accounts = db->database("FLHook")["accounts"];
    auto findAccDoc = make_document(kvp("_id", accountId));

    mongocxx::pipeline pipeline{};
    pipeline.match(make_document(kvp("_id", accountId)));
    pipeline.count("gameRoles");


    for (const auto cursor = accounts.aggregate(pipeline, mongocxx::options::aggregate{}); const auto data : cursor)
    {
        return data["gameRoles"].get_int32() != 0;
    }

    return false;
}

Action<void, Error> AccountId::UnBan() const
{
    const auto db = FLHook::GetDbClient();

    auto accounts = db->database("FLHook")["accounts"];
    const auto findAccDoc = make_document(kvp("_id", accountId));
    static auto updateDoc = make_document(kvp("$set", make_document(kvp("banned", false))), kvp("$unset", make_document(kvp("scheduledUnbanDate", ""))));

    if(const auto responseDoc = accounts.update_one(findAccDoc.view(), updateDoc.view()); responseDoc->modified_count() == 0)
    {
        // TODO: Err
        return { {} };
    }

    return { {} };
}

Action<void, Error> AccountId::Ban(const uint tempBanDays) const
{
    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];
    const auto findAccDoc = make_document(kvp("_id", accountId));

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

    if (const auto responseDoc = accounts.update_one(findAccDoc.view(), banUpdateDoc.view()); responseDoc->modified_count() == 0)
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

Action<void, Error> AccountId::DeleteCharacter(const std::wstring_view name) const
{
    if (const auto client = IsOnline(); client)
    {
        (void)client->id.Kick();
    }

    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];

    const auto findCharDoc = make_document(kvp("characterName", StringUtils::wstos(name)));

    mongocxx::options::find_one_and_delete deleteOptions;
    deleteOptions.projection(make_document(kvp("_id", 1)));

    const auto foundCharDoc = accounts.find_one_and_delete(findCharDoc.view(), deleteOptions);
    if (!foundCharDoc.has_value())
    {
        return { {} };
    }

    const auto findAccDoc = make_document(kvp("_id", accountId));
    const auto updateAccDoc = make_document(kvp("$pull", make_document(kvp("characters", foundCharDoc.value()["_id"].get_oid()))));

    if (const auto updateResponse = accounts.update_one(findAccDoc.view(), updateAccDoc.view()); updateResponse->modified_count() == 0)
    {
        return { {} };
    }

    return { {} };
}

Action<void, Error> AccountId::AddRoles(const std::vector<std::wstring_view>& roles)
{
    if(roles.empty())
    {
        //TODO: report error
        return {{}};
    }

    if(const auto account = IsOnline(); account)
    {
        if(!account->account->gameRoles.has_value())
        {
            account->account->gameRoles = {};
        }
        for(auto& role : roles)
        {
            auto strRole = StringUtils::wstos(role);
            if(std::ranges::find(account->account->gameRoles.value(), strRole) != account->account->gameRoles->end())
            {
                //TODO: Role not found
                continue;
            }
            account->account->gameRoles->emplace_back(strRole);
        }
        return {{}};
    }

    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];
    const auto findAccDoc = make_document(kvp("_id", accountId));

    bsoncxx::builder::basic::array arrayBuilder;
    for (auto& role : roles)
    {
        arrayBuilder.append(StringUtils::wstos(role));
    }

    const auto roleUpdateDoc = make_document(kvp("$push", make_document(kvp("gameRoles", arrayBuilder.view()))));

    if (const auto updateResponse = accounts.update_one(findAccDoc.view(), roleUpdateDoc.view()); updateResponse->modified_count() == 0)
    {
        return { {} };
    }

    return { {} };
}

Action<void, Error> AccountId::RemoveRoles(const std::vector<std::wstring_view>& roles, bool clear)
{
    if(roles.empty())
    {
        //TODO: report error
        return {{}};
    }

    if(const auto account = IsOnline(); account)
    {
        if(!account->account->gameRoles.has_value())
        {
            return {{}};
        }
        for(auto roleIter = account->account->gameRoles.value().begin(); roleIter != account->account->gameRoles.value().end(); )
        {
            if(const auto strRole = StringUtils::stows(*roleIter); std::ranges::find(roles, strRole) == roles.end())
            {
                roleIter = account->account->gameRoles.value().erase(roleIter);
            }
            else
            {
                ++roleIter;
            }
        }
        return {{}};
    }
    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];
    const auto findAccDoc = make_document(kvp("_id", accountId));

    bsoncxx::builder::basic::array arrayBuilder;
    for (auto& role : roles)
    {
        arrayBuilder.append(StringUtils::wstos(role));
    }

    const auto roleUpdateDoc = make_document(kvp("$pull", make_document(kvp("gameRoles", arrayBuilder.view()))));

    if (const auto updateResponse = accounts.update_one(findAccDoc.view(), roleUpdateDoc.view()); updateResponse->modified_count() == 0)
    {
        return { {} };
    }

    return { {} };
}

Action<void, Error> AccountId::SetRoles(const std::vector<std::wstring_view>& roles)
{
    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];
    const auto findAccDoc = make_document(kvp("_id", accountId));

    bsoncxx::builder::basic::array arrayBuilder;
    for (auto& role : roles)
    {
        arrayBuilder.append(StringUtils::wstos(role));
    }

    const auto roleUpdateDoc = make_document(kvp("$set", make_document(kvp("gameRoles", arrayBuilder.view()))));

    if (const auto updateResponse = accounts.update_one(findAccDoc.view(), roleUpdateDoc.view()); updateResponse->modified_count() == 0)
    {
        return { {} };
    }

    return { {} };
}

Action<bool, Error> AccountId::HasRole(const std::wstring_view role) const
{
    if(role.empty())
    {
        return {false};
    }
    const auto account = IsOnline();
    if( !account)
    {
        return {false};
    }

    if(!account->account->gameRoles.has_value())
    {
        return {false};
    }

    const auto strRole = StringUtils::wstos(role);
    for(const auto& gameRole : account->account->gameRoles.value())
    {
        if(gameRole == "superadmin" || gameRole == strRole)
        {
            return {true};
        }
    }
    return {false};
}

Action<void, Error> AccountId::SetCash(std::wstring_view characterName, int64 amount) const
{
    const auto db = FLHook::GetDbClient();
    auto accounts = db->database("FLHook")["accounts"];
    const auto findAccDoc = make_document(kvp("_id", accountId));

    const auto roleUpdateDoc = make_document(kvp("$set", make_document(kvp("cash", amount))));

    if (const auto updateResponse = accounts.update_one(findAccDoc.view(), roleUpdateDoc.view()); updateResponse->modified_count() == 0)
    {
        return { {} };
    }

    return { {} };
}
