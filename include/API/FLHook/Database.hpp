#pragma once

#include "API/FLHook/AccountManager.hpp"
#include "API/Types/AccountId.hpp"
#include "Defs/Database/Account.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>

class ClientList;
class FLHook;
class DatabaseQuery;

enum class DatabaseCollection
{
    Accounts,
    Character,
    Mail,
    ChatLog,
    ServerLog
};

class DLL Database
{
        friend ClientList;
        friend FLHook;
        friend AccountManager;

        mongocxx::instance instance;
        mongocxx::pool pool;

        // This searches based on the objectId on the database
        std::optional<Character> GetCharacterById(bsoncxx::oid objId);

    public:
        explicit Database(std::string_view uri);
        ~Database() = default;
        Database(const Database&) = delete;
        Database& operator=(const Database&) = delete;

        DatabaseQuery BeginDatabaseQuery();
        mongocxx::pool::entry AcquireClient();
        static mongocxx::collection GetCollection(const mongocxx::pool::entry& dbClient, std::string_view collectionName);
        static void SaveValueOnAccount(const AccountId& accountId, std::string_view key, bsoncxx::types::bson_value::view_or_value value);
};

class DLL DatabaseQuery
{
        friend Database;

        mongocxx::pool::entry entry;
        mongocxx::client_session session;
        bool sessionStarted = true;

        explicit DatabaseQuery(mongocxx::pool::entry entry);
        static std::string_view CollectionToString(DatabaseCollection collection);

    public:
        DatabaseQuery(const DatabaseQuery&) = delete;
        ~DatabaseQuery() = default;
        std::optional<bsoncxx::document::value> FindFromCollection(std::string_view collectionName, bsoncxx::document::view filter,
                                                                   const std::optional<bsoncxx::document::view>& projection = {}) const;
        std::optional<bsoncxx::document::value> FindFromCollection(DatabaseCollection collectionName, bsoncxx::document::view filter,
                                                                   const std::optional<bsoncxx::document::view>& projection = {}) const;

        bsoncxx::document::value FindAndUpdate(std::string_view collectionName, bsoncxx::document::view filter, bsoncxx::document::view update,
                                               const std::optional<bsoncxx::document::view>& projection = {}, bool before = true, bool replace = false,
                                               bool upsert = false) const;
        bsoncxx::document::value FindAndUpdate(DatabaseCollection collectionName, bsoncxx::document::view filter, bsoncxx::document::view update,
                                               const std::optional<bsoncxx::document::view>& projection = {}, bool before = true, bool replace = false,
                                               bool upsert = false) const;

        bsoncxx::document::value FindAndDelete(DatabaseCollection collectionName, bsoncxx::document::view filter,
                                               const std::optional<bsoncxx::document::view>& projection = {}) const;
        bsoncxx::document::value FindAndDelete(std::string_view collectionName, bsoncxx::document::view filter,
                                               const std::optional<bsoncxx::document::view>& projection = {}) const;

        mongocxx::result::update UpdateFromCollection(std::string_view collectionName, bsoncxx::document::view filter, bsoncxx::document::view update,
                                                      bool many = false) const;
        mongocxx::result::update UpdateFromCollection(DatabaseCollection collectionName, bsoncxx::document::view filter, bsoncxx::document::view update,
                                                      bool many = false) const;

        mongocxx::result::delete_result DeleteFromCollection(std::string_view collectionName, bsoncxx::document::view filter, bool many = false) const;
        mongocxx::result::delete_result DeleteFromCollection(DatabaseCollection collectionName, bsoncxx::document::view filter, bool many = false);

        std::variant<mongocxx::result::insert_one, mongocxx::result::insert_many> InsertIntoCollection(DatabaseCollection collectionName,
                                                                                                       const std::vector<bsoncxx::document::view>& newDocs);
        std::variant<mongocxx::result::insert_one, mongocxx::result::insert_many> InsertIntoCollection(std::string_view collectionName,
                                                                                                       const std::vector<bsoncxx::document::view>& newDocs);

        void ConcludeQuery(bool commitChanges);
};

enum class MongoResult
{
    Failure,
    MatchButNoChange,
    Success,
};
