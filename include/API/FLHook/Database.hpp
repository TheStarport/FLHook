#pragma once

#include "API/FLHook/AccountManager.hpp"
#include "API/Types/AccountId.hpp"
#include "Defs/Database/DbAccount.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/exception/exception.hpp>
#include <bsoncxx/json.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/exception/bulk_write_exception.hpp>
#include <mongocxx/exception/exception.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/pool.hpp>
#include <API/FLHook/BsonHelper.hpp>

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
        std::optional<B_VAL> FindFromCollection(std::string_view collectionName, B_VIEW filter, const std::optional<B_VIEW>& projection = {}) const;
        std::optional<B_VAL> FindFromCollection(DatabaseCollection collectionName, B_VIEW filter, const std::optional<B_VIEW>& projection = {}) const;

        std::optional<bsoncxx::document::value> FindAndUpdate(std::string_view collectionName, B_VIEW filter, B_VIEW update,
                                                              const std::optional<B_VIEW>& projection = {}, bool before = true, bool replace = false,
                                                              bool upsert = false) const;
        std::optional<bsoncxx::document::value> FindAndUpdate(DatabaseCollection collectionName, B_VIEW filter, B_VIEW update,
                                                              const std::optional<B_VIEW>& projection = {}, bool before = true, bool replace = false,
                                                              bool upsert = false) const;

        B_VAL FindAndDelete(DatabaseCollection collectionName, B_VIEW filter, const std::optional<B_VIEW>& projection = {}) const;
        B_VAL FindAndDelete(std::string_view collectionName, B_VIEW filter, const std::optional<B_VIEW>& projection = {}) const;

        mongocxx::result::update UpdateFromCollection(std::string_view collectionName, B_VIEW filter, B_VIEW update, bsoncxx::array::view arrayFilters = {},
                                                      bool many = false) const;
        mongocxx::result::update UpdateFromCollection(DatabaseCollection collectionName, B_VIEW filter, B_VIEW update, bsoncxx::array::view arrayFilters = {},
                                                      bool many = false) const;

        mongocxx::result::delete_result DeleteFromCollection(std::string_view collectionName, B_VIEW filter, bool many = false) const;
        mongocxx::result::delete_result DeleteFromCollection(DatabaseCollection collectionName, B_VIEW filter, bool many = false);

        std::variant<mongocxx::result::insert_one, mongocxx::result::insert_many> InsertIntoCollection(DatabaseCollection collectionName,
                                                                                                       const std::vector<B_VIEW>& newDocs);
        std::variant<mongocxx::result::insert_one, mongocxx::result::insert_many> InsertIntoCollection(std::string_view collectionName,
                                                                                                       const std::vector<B_VIEW>& newDocs);

        void ConcludeQuery(bool commitChanges);
};

enum class MongoResult
{
    Failure,
    MatchButNoChange,
    Success,
};
