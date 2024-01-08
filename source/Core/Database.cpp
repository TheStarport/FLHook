#include "PCH.hpp"

#include "Core/Database.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

//TODO: MongoDB/Mongocxx and wstring support. Mainly it doesn't support wstring as values and look into it.
Database::Database()
{
    try
    {
        const auto mongoURI = mongocxx::uri{ FLHookConfig::i()->databaseConfig.uri };
        mongocxx::options::client clientOptions;
        const auto api = mongocxx::options::server_api{ mongocxx::options::server_api::version::k_version_1 };

        client = { mongoURI, clientOptions };
        database = client["flhook"];
        accounts = database["accounts"];
    }

    catch (std::exception& err)
    {
        Logger::Log(LogLevel::Err, StringUtils::stows(std::string(err.what())));

    }

}

void Database::ResetDatabase()
{
    accounts.drop();
}

void Database::RemoveValueFromCharacter(std::string character, std::string value)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto searchDoc = make_document(kvp("character_name", character));
    auto updateDoc = make_document(kvp("$unset", make_document(kvp(value, 0))));

    accounts.update_one(searchDoc.view(), updateDoc.view());

}

void Database::RemoveValueFromAccount(AccountId account, std::string value)
{
    const auto cAcc = account.GetValue();
    auto key = StringUtils::wstos(cAcc->accId);

    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto searchDoc = make_document(kvp("_id", key));
    auto updateDoc = make_document(kvp("$unset", make_document(kvp(value, 0))));

    accounts.update_one(searchDoc.view(), updateDoc.view());


}
