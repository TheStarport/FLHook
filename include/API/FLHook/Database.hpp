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
    const auto db = AcquireClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        auto accounts = db->database("FLHook")["accounts"];

        const auto searchDoc = make_document(kvp("characterName", character));
        auto updateDoc = make_document(kvp("$set", make_document(kvp(val.first, val.second))));

        accounts.update_one(searchDoc.view(), updateDoc.view());

        session.commit_transaction();
    }
    catch (bsoncxx::exception& ex)
    {
        session.abort_transaction();
    }
    catch (mongocxx::exception& ex)
    {
        session.abort_transaction();
    }
}

template <typename T>
    requires MongoSupportedType<T>
void Database::AddValueToAccount(AccountId account, std::pair<std::string, T> val)
{
    const auto db = AcquireClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        auto accounts = db->database("FLHook")["accounts"];
        const auto cAcc = account.GetValue();
        auto key = StringUtils::wstos(cAcc->accId);

        using bsoncxx::builder::basic::kvp;
        using bsoncxx::builder::basic::make_document;

        const auto searchDoc = make_document(kvp("_id", key));
        auto updateDoc = make_document(kvp("$set", make_document(kvp(val.first, val.second))));

        accounts.update_one(searchDoc.view(), updateDoc.view());

        session.commit_transaction();
    }
    catch (bsoncxx::exception& ex)
    {
        session.abort_transaction();
    }
    catch (mongocxx::exception& ex)
    {
        session.abort_transaction();
    }
}
