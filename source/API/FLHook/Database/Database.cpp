#include "PCH.hpp"

#include "API/FLHook/Database.hpp"

#include "API/Utils/Reflection.hpp"
#include <mongocxx/exception/error_code.hpp>
#include <mongocxx/exception/write_exception.hpp>

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

Database::Database(const std::string_view uri) : pool(mongocxx::uri(uri), mongocxx::options::pool{})
{
    try
    {
        const auto client = pool.acquire();
        auto db = client->database("FLHook");

        const auto ping = make_document(kvp("ping", 1));
        db.run_command(ping.view());

        // TODO: Make DB and collection dynamic
        if (!db.has_collection("accounts"))
        {
            auto accounts = db["accounts"];

            accounts.create_index(make_document(kvp("characterName", 1))); // Mandate character id is unique
            accounts.create_index(make_document(kvp("accountId", 1)));
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

std::optional<Character> Database::GetCharacterById(bsoncxx::oid objId)
{
    auto& config = FLHook::GetConfig();
    const auto db = AcquireClient();

    // TODO: Configure DB and accounts key
    auto accounts = db->database(config.databaseConfig.dbName).collection("accounts");
    const auto charDocOpt = accounts.find_one(make_document(kvp("_id", objId)));
    if (!charDocOpt.has_value())
    {
        // TODO: Log Issue
        return std::nullopt;
    }

    const auto& doc = charDocOpt.value();
    return Character{ doc };
}
