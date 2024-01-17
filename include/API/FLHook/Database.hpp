#pragma once

#include "API/Types/AccountId.hpp"
#include <bsoncxx/builder/basic/document.hpp>
#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>

template <typename T>
concept MongoSupportedType = std::is_fundamental_v<T> || std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring> ||
                             std::is_same_v<T, std::wstring_view> || std::is_same_v<T, std::string_view>;

class ClientList;
class FLHook;

class Database
{
        friend ClientList;
        friend FLHook;

        Database();
        ~Database() = default;
        Database(const Database&&) = delete;

        mongocxx::instance instance;
        mongocxx::collection accounts;
        mongocxx::client client;
        mongocxx::database database;

        void ResetDatabase();

    public:
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
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto searchDoc = make_document(kvp("character_name", character));
    auto updateDoc = make_document(kvp("$set", make_document(kvp(val.first, val.second))));

    accounts.update_one(searchDoc.view(), updateDoc.view());
}

template <typename T>
    requires MongoSupportedType<T>
void Database::AddValueToAccount(AccountId account, std::pair<std::string, T> val)
{
    const auto cAcc = account.GetValue();
    auto key = StringUtils::wstos(cAcc->accId);

    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto searchDoc = make_document(kvp("_id", key));
    auto updateDoc = make_document(kvp("$set", make_document(kvp(val.first, val.second))));

    accounts.update_one(searchDoc.view(), updateDoc.view());
}
