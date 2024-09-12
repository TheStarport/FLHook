#include "PCH.hpp"

#include "API/FLHook/Database.hpp"

#include "API/FLHook/TaskScheduler.hpp"
#include <mongocxx/exception/error_code.hpp>
#include <mongocxx/exception/write_exception.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

Database::Database(const std::string_view uri) : pool(mongocxx::uri(uri), mongocxx::options::pool{})
{
    try
    {
        const auto config = FLHook::GetConfig();
        const auto client = pool.acquire();
        auto db = client->database(config->database.dbName);

        const auto ping = make_document(kvp("ping", 1));
        db.run_command(ping.view());

        if (!db.has_collection(config->database.accountsCollection))
        {
            db.create_collection(config->database.accountsCollection);
            auto accounts = db[config->database.accountsCollection];
            accounts.create_index(make_document(kvp("characterName", 1)));
            accounts.create_index(make_document(kvp("accountId", 1)));
        }

        if (!db.has_collection(config->database.mailCollection))
        {
            db.create_collection(config->database.mailCollection);
            auto mail = db[config->database.mailCollection];
            mail.create_index(make_document(kvp("recipients.target", 1), kvp("sentDate", -1)));
        }
    }
    catch (std::exception& err)
    {
        Logger::Err(StringUtils::stows(std::string(err.what())));
        MessageBoxA(nullptr, (std::string("Unable to connect to MongoDB. Cannot start FLHook.\n\n") + err.what()).c_str(), "MongoDB Connection Error", MB_OK);
        std::quick_exit(-1);
    }
}

mongocxx::pool::entry Database::AcquireClient() { return pool.acquire(); }

mongocxx::collection Database::GetCollection(const mongocxx::pool::entry& dbClient, const std::string_view collectionName)
{
    return dbClient->database(FLHook::GetConfig()->database.dbName).collection(collectionName);
}

void Database::SaveValueOnAccount(const AccountId& accountId, std::string_view key, bsoncxx::types::bson_value::view_or_value value)
{
    auto findDoc = make_document(kvp("_id", accountId.GetValue()));
    auto updateDoc = make_document(kvp("$set", make_document(kvp(key, value))));

    TaskScheduler::Schedule(
        [findDoc, updateDoc]
        {
            const auto config = FLHook::GetConfig()->database;
            auto db = FLHook::GetDbClient();

            auto accountsCollection = db->database(config.dbName)[config.accountsCollection];

            accountsCollection.update_one(findDoc.view(), updateDoc.view());
        });
}

std::optional<Character> Database::GetCharacterById(bsoncxx::oid objId)
{
    const auto config = FLHook::GetConfig();
    const auto db = AcquireClient();

    auto accounts = db->database(config->database.dbName).collection(config->database.accountsCollection);
    const auto charDocOpt = accounts.find_one(make_document(kvp("_id", objId)));
    if (!charDocOpt.has_value())
    {
        return std::nullopt;
    }

    const auto& doc = charDocOpt.value();
    return Character{ doc };
}
