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

template <typename T>
concept MongoSupportedType = std::is_fundamental_v<T> || std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring> ||
                             std::is_same_v<T, std::wstring_view> || std::is_same_v<T, std::string_view>;

class ClientList;
class FLHook;
class Database;

class Database
{
        friend ClientList;
        friend FLHook;
        friend AccountManager;

        Database(std::string_view uri);
        ~Database() = default;

        mongocxx::instance instance;
        mongocxx::pool pool;

        // This searches based on the objectId on the database
        std::optional<Character> GetCharacterById(bsoncxx::oid objId);

    public:
        mongocxx::pool::entry AcquireClient();
};