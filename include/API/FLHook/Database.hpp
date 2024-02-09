#pragma once

#include "API/FLHook/AccountManager.hpp"
#include "API/Types/AccountId.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>

template <typename T>
concept MongoSupportedType = std::is_fundamental_v<T> || std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring> ||
                             std::is_same_v<T, std::wstring_view> || std::is_same_v<T, std::string_view>;

class ClientList;
class FLHook;

struct Collection
{
        mongocxx::pool::entry client;
        mongocxx::database database;
        mongocxx::collection collection;

        Collection() = delete;
        Collection(mongocxx::pool::entry cl, mongocxx::database db, mongocxx::collection col)
            : client(std::move(cl)), database(std::move(db)), collection(std::move(col))
        {}
};

class Database
{
        friend ClientList;
        friend FLHook;
        friend AccountManager;

        Database(std::string_view uri);
        ~Database() = default;

        mongocxx::instance instance;
        mongocxx::pool pool;

        void ResetDatabase();
        void CreateCharacter(std::string accountId, VanillaLoadData* newPlayer);

    public:
        std::optional<mongocxx::pool::entry> AcquireClient();
        std::optional<Collection> GetCollection(std::string_view collectionName);
        std::optional<Collection> CreateCollection(std::string_view collectionName);

        template <typename T>
            requires MongoSupportedType<T>
        void AddValueToCharacter(std::string character, std::pair<std::string, T> val);

        template <typename T>
            requires MongoSupportedType<T>
        void AddValueToAccount(AccountId account, std::pair<std::string, T> val);

        void RemoveValueFromCharacter(std::string character, std::string value);
        void RemoveValueFromAccount(AccountId account, std::string value);
};

template <typename T>
    requires MongoSupportedType<T>
void Database::AddValueToCharacter(std::string character, std::pair<std::string, T> val)
{
    auto accountsOpt = GetCollection("accounts");
    if (!accountsOpt.has_value())
    {
        // TODO: Log issue
        return;
    }

    auto& accounts = accountsOpt.value();

    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto searchDoc = make_document(kvp("characterName", character));
    auto updateDoc = make_document(kvp("$set", make_document(kvp(val.first, val.second))));

    accounts.collection.update_one(searchDoc.view(), updateDoc.view());
}

template <typename T>
    requires MongoSupportedType<T>
void Database::AddValueToAccount(AccountId account, std::pair<std::string, T> val)
{
    auto accountsOpt = GetCollection("accounts");
    if (!accountsOpt.has_value())
    {
        // TODO: Log issue
        return;
    }

    auto& accounts = accountsOpt.value();

    const auto cAcc = account.GetValue();
    auto key = StringUtils::wstos(cAcc->accId);

    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto searchDoc = make_document(kvp("_id", key));
    auto updateDoc = make_document(kvp("$set", make_document(kvp(val.first, val.second))));

    accounts.collection.update_one(searchDoc.view(), updateDoc.view());
}
