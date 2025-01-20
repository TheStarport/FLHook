#include "PCH.hpp"

#include "API/FLHook/Database.hpp"

#include "API/FLHook/TaskScheduler.hpp"
#include "Defs/Database/Account.hpp"

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

        if (!db.has_collection(config->database.charactersCollection))
        {
            db.create_collection(config->database.charactersCollection);
            auto characters = db[config->database.charactersCollection];
            characters.create_index(make_document(kvp("characterName", 1)), make_document(kvp("unique", 1)));
            characters.create_index(make_document(kvp("accountId", 1)));
        }

        if (!db.has_collection(config->database.accountsCollection))
        {
            db.create_collection(config->database.accountsCollection);
            auto accounts = db[config->database.accountsCollection];
            accounts.create_index(make_document(kvp("username", 1)));
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
        ERROR(StringUtils::stows(std::string(err.what())));
        MessageBoxA(nullptr, (std::string("Unable to connect to MongoDB. Cannot start FLHook.\n\n") + err.what()).c_str(), "MongoDB Connection Error", MB_OK);
        std::quick_exit(-1);
    }
}

DatabaseQuery Database::BeginDatabaseQuery() { return DatabaseQuery(pool.acquire()); }

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

DatabaseQuery::DatabaseQuery(mongocxx::pool::entry entry) : entry(std::move(entry)), session(this->entry->start_session()) { session.start_transaction(); }

std::string_view DatabaseQuery::CollectionToString(const DatabaseCollection collection)
{
    const auto& config = FLHook::GetConfig();
    switch (collection)
    {
        case DatabaseCollection::Accounts: return config->database.accountsCollection;
        case DatabaseCollection::Character: return config->database.charactersCollection;
        case DatabaseCollection::Mail: return config->database.mailCollection;
        default: throw std::invalid_argument("Invalid collection");
    }
}

std::optional<bsoncxx::document::value> DatabaseQuery::FindFromCollection(const std::string_view collectionName, const bsoncxx::document::view filter,
                                                                          const std::optional<bsoncxx::document::view>& projection) const
{
    auto collection = Database::GetCollection(entry, collectionName);

    mongocxx::options::find find;
    if (projection.has_value())
    {
        find.projection(projection.value());
    }

    auto result = collection.find_one(filter, find);
    return collection.find_one(filter);
}

std::optional<bsoncxx::document::value> DatabaseQuery::FindFromCollection(const DatabaseCollection collectionName, const bsoncxx::document::view filter,
                                                                          const std::optional<bsoncxx::document::view>& projection) const
{
    return FindFromCollection(CollectionToString(collectionName), filter, projection);
}

bsoncxx::document::value DatabaseQuery::FindAndUpdate(const std::string_view collectionName, const bsoncxx::document::view filter,
                                                      const bsoncxx::document::view update, const std::optional<bsoncxx::document::view>& projection,
                                                      const bool before, const bool replace, const bool upsert) const
{
    auto collection = Database::GetCollection(entry, collectionName);

    mongocxx::options::find_one_and_update find;
    mongocxx::options::find_one_and_replace replaceOption;
    find.return_document(static_cast<mongocxx::options::return_document>(before));
    replaceOption.return_document(static_cast<mongocxx::options::return_document>(before));

    if (projection.has_value())
    {
        find.projection(projection.value());
        replaceOption.projection(projection.value());
    }

    find.upsert(upsert);
    replaceOption.upsert(upsert);

    auto result = replace ? collection.find_one_and_replace(filter, update, replaceOption) : collection.find_one_and_update(filter, update, find);
    assert(result.has_value());
    return result.value();
}

bsoncxx::document::value DatabaseQuery::FindAndUpdate(const DatabaseCollection collectionName, const bsoncxx::document::view filter,
                                                      const bsoncxx::document::view update, const std::optional<bsoncxx::document::view>& projection,
                                                      const bool before, const bool replace, const bool upsert) const
{
    return FindAndUpdate(CollectionToString(collectionName), filter, update, projection, before, replace, upsert);
}

bsoncxx::document::value DatabaseQuery::FindAndDelete(const std::string_view collectionName, const bsoncxx::document::view filter,
                                                      const std::optional<bsoncxx::document::view>& projection) const
{
    auto collection = Database::GetCollection(entry, collectionName);

    mongocxx::options::find_one_and_delete find;
    if (projection.has_value())
    {
        find.projection(projection.value());
    }

    auto result = collection.find_one_and_delete(filter, find);
    assert(result.has_value());
    return result.value();
}

bsoncxx::document::value DatabaseQuery::FindAndDelete(const DatabaseCollection collectionName, const bsoncxx::document::view filter,
                                                      const std::optional<bsoncxx::document::view>& projection) const
{
    return FindAndDelete(CollectionToString(collectionName), filter, projection);
}

mongocxx::result::update DatabaseQuery::UpdateFromCollection(const std::string_view collectionName, const bsoncxx::document::view filter,
                                                             const bsoncxx::document::view update, const bool many) const
{
    auto collection = Database::GetCollection(entry, collectionName);

    auto result = many ? collection.update_many(filter, update) : collection.update_one(filter, update);
    assert(result.has_value());
    return result.value();
}

mongocxx::result::update DatabaseQuery::UpdateFromCollection(const DatabaseCollection collectionName, const bsoncxx::document::view filter,
                                                             const bsoncxx::document::view update, const bool many) const
{
    return UpdateFromCollection(CollectionToString(collectionName), filter, update, many);
}

mongocxx::result::delete_result DatabaseQuery::DeleteFromCollection(const std::string_view collectionName, const bsoncxx::document::view filter,
                                                                    const bool many) const
{
    auto collection = Database::GetCollection(entry, collectionName);

    auto result = many ? collection.delete_many(filter) : collection.delete_one(filter);
    assert(result.has_value());
    return result.value();
}

mongocxx::result::delete_result DatabaseQuery::DeleteFromCollection(const DatabaseCollection collectionName, const bsoncxx::document::view filter,
                                                                    const bool many)
{
    return DeleteFromCollection(CollectionToString(collectionName), filter, many);
}

std::variant<mongocxx::result::insert_one, mongocxx::result::insert_many> DatabaseQuery::InsertIntoCollection(
    const std::string_view collectionName, const std::vector<bsoncxx::document::view>& newDocs)
{
    auto collection = Database::GetCollection(entry, collectionName);

    if (newDocs.size() != 1)
    {
        auto result = collection.insert_many(newDocs);
        assert(result.has_value());

        return result.value();
    }

    auto result = collection.insert_one(newDocs.front());
    assert(result.has_value());
    return result.value();
}

std::variant<mongocxx::result::insert_one, mongocxx::result::insert_many> DatabaseQuery::InsertIntoCollection(
    const DatabaseCollection collectionName, const std::vector<bsoncxx::document::view>& newDocs)
{
    return InsertIntoCollection(CollectionToString(collectionName), newDocs);
}

void DatabaseQuery::ConcludeQuery(const bool success)
{
    if (!sessionStarted)
    {
        throw std::logic_error("A database query cannot be concluded multiple times.");
    }

    sessionStarted = false;
    if (success)
    {
        session.commit_transaction();
    }
    else
    {
        session.abort_transaction();
    }
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
