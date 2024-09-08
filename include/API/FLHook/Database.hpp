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

class DLL Database
{
        friend ClientList;
        friend FLHook;
        friend AccountManager;

        explicit Database(std::string_view uri);
        ~Database() = default;

        mongocxx::instance instance;
        mongocxx::pool pool;

        // This searches based on the objectId on the database
        std::optional<Character> GetCharacterById(bsoncxx::oid objId);

    public:
        mongocxx::pool::entry AcquireClient();
        static mongocxx::collection GetCollection(const mongocxx::pool::entry& dbClient, std::string_view collectionName);
        static void SaveValueOnAccount(const AccountId& accountId, std::string_view key, bsoncxx::types::bson_value::view_or_value value);
};
