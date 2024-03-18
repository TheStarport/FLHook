#pragma once

#include "API/FLHook/AccountManager.hpp"
#include "API/Types/AccountId.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <bsoncxx/json.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include "Defs/Database/Account.hpp"

template <typename T>
concept MongoSupportedType = std::is_fundamental_v<T> || std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring> ||
                             std::is_same_v<T, std::wstring_view> || std::is_same_v<T, std::string_view>;

class ClientList;
class FLHook;
class Database;
class Collection
{
        friend Database;
        std::string_view collectionName;
        mongocxx::pool::entry& client;
        mongocxx::database db;
        mongocxx::collection collection;

    public:
        explicit Collection(mongocxx::pool::entry& client, std::string_view collection);

        mongocxx::collection& GetRaw() { return collection; }
        void CreateIndex(std::string_view field, bool ascending = true);
        bool InsertIntoCollection(bsoncxx::document::view document);
        bool InsertIntoCollection(std::string_view document);
        bool OverwriteItem(std::string_view id, std::string_view document);
        bool UpdateItemById(std::string_view id, bsoncxx::document::view value);
        bool UpdateItemByFilter(bsoncxx::document::view filter, bsoncxx::document::view value);
        std::optional<bsoncxx::document::value> GetItemByIdRaw(std::string_view id);

        template<typename T>
        std::optional<T> GetItemById(std::string_view id)
        {
            auto item = GetItemByIdRaw(id);
            if (item.has_value())
            {
                auto result = rfl::bson::read<T>(item.value());
                if (result.has_error())
                {
                    return std::nullopt;
                }

                return result.value();
            }

            return std::nullopt;
        }

        std::stop_token GetItemByIdRawDeferred(std::string id);
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

		//Will return true if a character is created, false if a character is not successfully created in the database.
        bool CreateCharacter(std::string accountId,  Character&  newPlayer);

		//This searches based on the objectId on the database
		Character GrabCharacterById(bsoncxx::oid objId);
		Account GetOrCreateAccount(std::string accountId, bool deferred = true);

    public:
        std::optional<mongocxx::pool::entry> AcquireClient();
        std::optional<Collection> GetCollection(std::string_view collectionName);
        std::optional<Collection> CreateCollection(std::string_view collectionName);
		bool SaveCharacter(const Character& character);


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
