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

        mongocxx::write_concern wc;
        wc.acknowledge_level(mongocxx::write_concern::level::k_unacknowledged);
        db.write_concern(wc);

        const auto ping = make_document(kvp("ping", 1));
        db.run_command(ping.view());

        // TODO: Make DB and collection dynamic
        if (!db.has_collection("accounts"))
        {
            auto accounts = db["accounts"];

            mongocxx::options::index indexOptions{};
            indexOptions.unique(true);

            accounts.create_index(make_document(kvp("characterName", 1)), indexOptions); // Mandate character id is unique
            accounts.create_index(make_document(kvp("accountId", 1)));
        }
    }
    catch (std::exception& err)
    {
        Logger::Log(LogLevel::Err, StringUtils::stows(std::string(err.what())));
        MessageBoxA(nullptr, (std::string("Unable to connect to MongoDB. Cannot start FLHook.\n\n") + err.what()).c_str(), "MongoDB Connection Error", MB_OK);
        std::quick_exit(-1);
    }
}

mongocxx::pool::entry Database::AcquireClient() { return pool.acquire(); }

void Database::RemoveValueFromCharacter(std::string character, std::string value)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    const auto db = pool.acquire();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        auto accounts = db->database("FLHook")["accounts"];

        const auto searchDoc = make_document(kvp("character_name", character));
        const auto updateDoc = make_document(kvp("$unset", make_array(value)));
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

void Database::RemoveValueFromAccount(AccountId account, std::string value)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    const auto db = AcquireClient();
    auto session = db->start_session();
    session.start_transaction();

    try
    {
        auto accounts = db->database("FLHook")["accounts"];

        auto searchDoc = make_document(kvp("_id", StringUtils::wstos(account.GetValue()->accId)));
        auto updateDoc = make_document(kvp("$unset", make_array(value)));

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

std::optional<Character> Database::GetCharacterById(bsoncxx::oid objId)
{
    const auto db = AcquireClient();

    // TODO: Configure DB and accounts key
    auto accounts = db->database("FLHook").collection("accounts");
    const auto charDocOpt = accounts.find_one(make_document(kvp("_id", objId)));
    if (!charDocOpt.has_value())
    {
        // TODO: Log Issue
        return std::nullopt;
    }

    const auto& doc = charDocOpt.value();
    auto character = rfl::bson::read<Character>(doc.data(), doc.length());

    if (character.error().has_value())
    {
        // TODO: Log error
        return std::nullopt;
    }

    return character.value();
}
