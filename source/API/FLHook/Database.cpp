#include "PCH.hpp"

#include "API/FLHook/Database.hpp"

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
    }
    catch (std::exception& err)
    {
        Logger::Log(LogLevel::Err, StringUtils::stows(std::string(err.what())));
        MessageBoxA(nullptr, (std::string("Unable to connect to MongoDB. Cannot start FLHook.\n\n") + err.what()).c_str(), "MongoDB Connection Error", MB_OK);
        std::quick_exit(-1);
    }
}

void Database::CreateCharacter(std::string accountId, VanillaLoadData* newPlayer)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto accountsOpt = GetCollection("accounts");
    if (!accountsOpt.has_value())
    {
        // TODO: Log issue
        return;
    }

    auto& accounts = accountsOpt.value();

    auto dbBaseCostume = make_document(kvp("body", static_cast<long long>(newPlayer->baseCostume.body)),
                                       kvp("head", static_cast<long long>(newPlayer->baseCostume.head)),
                                       kvp("leftHand", static_cast<long long>(newPlayer->baseCostume.leftHand)),
                                       kvp("rightHand", static_cast<long long>(newPlayer->baseCostume.rightHand)));

    auto dbCommCostume = make_document(kvp("body", static_cast<long long>(newPlayer->commCostume.body)),
                                       kvp("head", static_cast<long long>(newPlayer->commCostume.head)),
                                       kvp("leftHand", static_cast<long long>(newPlayer->commCostume.leftHand)),
                                       kvp("rightHand", static_cast<long long>(newPlayer->commCostume.rightHand)));

    /*
    bsoncxx::builder::basic::array equipmentArray;

        for (const auto& equip : newCharTemplate.currentEquipAndCargo)
        {
            bool commodity;
            pub::IsCommodity(equip.archId, commodity);
            if (!commodity)
            {
                std::string equipStr;
                equipStr = equip.hardPoint.value;
                equipmentArray.append(
                    kvp("archId", static_cast<long long>(equip.archId)), kvp("hardPoint", equipStr), kvp("mounted", equip.mounted), kvp("health",
       equip.health));
            }
        }

        bsoncxx::builder::basic::array cargoArray;

        for (const auto& cargo : newCharTemplate.currentEquipAndCargo)
        {
            bool commodity;
            pub::IsCommodity(cargo.archId, commodity);
            if (commodity)
            {
                std::string equipStr;
                equipStr = cargo.hardPoint.value;
                cargoArray.append(
                    kvp("archId", static_cast<long long>(cargo.archId)), kvp("hardPoint", equipStr), kvp("mounted", cargo.mounted), kvp("health",
       cargo.health));
            }
        }
            */

    bsoncxx::builder::basic::array visitArray;

    for (auto visit = newPlayer->visitLists.begin(); visit != newPlayer->visitLists.end(); ++visit)
    {
        visitArray.append(static_cast<bsoncxx::types::b_int64>(visit->key), static_cast<bsoncxx::types::b_int64>(visit->value.visitValue)); // NOLINT
    }

    bsoncxx::builder::basic::array reputationArray;

    /*for (const auto& rep : template.reputationOverrides)
    {
        visitArray.append(kvp(rep.first, rep.second));
    }*/

    std::wstring charNameWide = reinterpret_cast<const wchar_t*>(newPlayer->name.c_str());
    std::string charName = StringUtils::wstos(charNameWide);

    // We cast to long long as mongo does not care about sign, and we want to prevent any sort of signed overflow
    auto newCharDoc = make_document(kvp("characterName", charName),
                                    kvp("money", static_cast<long long>(newPlayer->money)),
                                    kvp("rank", static_cast<long long>(newPlayer->rank)),
                                    kvp("repGroup", "toBeDetermined"),
                                    kvp("datetimeHigh", static_cast<long long>(newPlayer->datetimeHigh)),
                                    kvp("datetimeLow", static_cast<long long>(newPlayer->datetimeLow)),
                                    kvp("canDock", 1),
                                    kvp("canTradeLane", 1),
                                    kvp("numOfKills", 0),
                                    kvp("numOfFailedMissions", 0),
                                    kvp("numOfSuccessMissions", 0),
                                    kvp("shipHash", static_cast<long long>(newPlayer->shipHash)),
                                    kvp("system", static_cast<long long>(newPlayer->system)),
                                    kvp("totalTimePlayed", 0.0000f),
                                    kvp("baseCostume", dbBaseCostume),
                                    kvp("commCostume", dbCommCostume),
                                    kvp("reputation", reputationArray),
                                    kvp("visits", visitArray));

    const auto updateDoc = accounts.collection.find_one(make_document(kvp("characterName", charName)).view());
    const auto elem = updateDoc.value()["_id"];
    auto str = elem.get_oid().value.to_string();
    const auto findRes = accounts.collection.find_one(make_document(kvp("_id", accountId)));

    // Update the account's character list to include the newly created character.
    if (!findRes.has_value())
    {
        std::cout << "Account not found.";
        throw;
        // TODO: Handle correctly
    }

    auto characterArray = make_array(str);
    auto updateDoc = make_document(kvp("$set", make_document(kvp("characters", characterArray))));

    accounts.collection.update_one(findRes->view(), updateDoc.view());
}

std::optional<mongocxx::pool::entry> Database::AcquireClient() { return pool.try_acquire(); }

std::optional<Collection> Database::GetCollection(std::string_view collectionName)
{
    try
    {
        auto client = pool.acquire();
        return Collection(client, collectionName);
    }
    catch (mongocxx::exception& ex)
    {
        Logger::Log(LogLevel::Err, std::format(L"Mongocxx error. Code: {} - Err: {}", ex.code().value(), StringUtils::stows(ex.what())));
        return std::nullopt;
    }
}

std::optional<Collection> Database::CreateCollection(std::string_view collectionName)
{
    try
    {
        auto client = pool.acquire();
        auto db = client->database("FLHook");

        if (db.has_collection(collectionName))
        {
            return GetCollection(collectionName);
        }

        auto concern = mongocxx::write_concern{};
        concern.acknowledge_level(mongocxx::write_concern::level::k_acknowledged);
        auto collection = db.create_collection(collectionName, {}, concern);

        return Collection(client, collectionName);
    }
    catch (mongocxx::exception& ex)
    {
        Logger::Log(LogLevel::Err, std::format(L"Mongocxx error. Code: {} - Err: {}", ex.code().value(), StringUtils::stows(ex.what())));
        return std::nullopt;
    }
}

void Database::RemoveValueFromCharacter(std::string character, std::string value)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto accountsOpt = GetCollection("accounts");
    if (!accountsOpt.has_value())
    {
        // TODO: Log issue
        return;
    }

    auto& accounts = accountsOpt.value();

    auto searchDoc = make_document(kvp("character_name", character));
    auto updateDoc = make_document(kvp("$unset", make_array(value)));

    accounts.UpdateItemByFilter(searchDoc.view(), updateDoc.view());
}

void Database::RemoveValueFromAccount(AccountId account, std::string value)
{
    auto accountsOpt = GetCollection("accounts");
    if (!accountsOpt.has_value())
    {
        // TODO: Log issue
        return;
    }

    auto& accounts = accountsOpt.value();

    const auto cAcc = account.GetValue();
    auto key = StringUtils::wstos(cAcc->accId);

    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    auto searchDoc = make_document(kvp("_id", key));
    auto updateDoc = make_document(kvp("$unset", make_array(value)));

    accounts.UpdateItemByFilter(searchDoc.view(), updateDoc.view());
}

Collection::Collection(mongocxx::pool::entry& client, std::string_view collection) : client(client), collectionName(collection)
{
    db = client->database("flhook");
    this->collection = db.collection(collectionName);
}

bool Collection::InsertIntoCollection(std::string_view document)
{
    try
    {
        auto doc_value = bsoncxx::from_json(document);
        auto result = collection.insert_one(std::move(doc_value));
        return result.has_value();
    }
    catch(const bsoncxx::exception& e)
    {
        std::string errInfo = std::format("Error converting string to BSON. Error: {}", e.what());
    }
    catch (mongocxx::exception& e)
    {
        std::string errInfo = std::format("Error in inserting document. Error: {}", e.what());
    }

    return false;
}

bool Collection::OverwriteItem(std::string_view id, std::string_view document)
{
    try
    {
        auto filter = make_document(kvp("_id", id));
        auto doc_value = bsoncxx::from_json(document);
        auto result = collection.find_one_and_replace(filter.view(), doc_value.view());
        return result.has_value();
    }
    catch(const bsoncxx::exception& e)
    {
        //std::string errInfo = std::format("Error converting string to BSON. Error: {}", e.what());
    }
    catch (mongocxx::exception& e)
    {
        //std::string errInfo = std::format("Error in replacing document. Error: {}", e.what());
    }

    return false;
}

bool Collection::UpdateItemById(std::string_view id, bsoncxx::document::view value)
{
    try
    {
        auto filter = make_document(kvp("_id", id));
        auto result = collection.find_one_and_update(filter.view(), value);
        return result.has_value();
    }
    catch(const bsoncxx::exception& e)
    {
        //std::string errInfo = std::format("Error converting string to BSON. Error: {}", e.what());
    }
    catch (mongocxx::exception& e)
    {
        //std::string errInfo = std::format("Error in replacing document. Error: {}", e.what());
    }

    return false;
}
bool Collection::UpdateItemByFilter(bsoncxx::document::view filter, bsoncxx::document::view value)
{
    try
    {
        auto result = collection.find_one_and_update(filter, value);
        return result.has_value();
    }
    catch(const bsoncxx::exception& e)
    {
        //std::string errInfo = std::format("Error converting string to BSON. Error: {}", e.what());
    }
    catch (mongocxx::exception& e)
    {
        //std::string errInfo = std::format("Error in replacing document. Error: {}", e.what());
    }

    return false;
}

void Collection::CreateIndex(std::string_view field, bool ascending)
{
    try
    {
        auto index = make_document(kvp(field, ascending ? 1 : -1));
        collection.create_index(std::move(index));
    }
    catch (mongocxx::exception& e)
    {
        // std::string errInfo = std::format("Error in replacing document. Error: {}", e.what());
    }
}

std::optional<bsoncxx::document::value> Collection::GetItemByIdRaw(std::string_view id)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    try
    {
        auto filter = make_document(kvp("_id", id));
        std::optional<bsoncxx::document::value> mongoResult;
        mongoResult = collection.find_one(filter.view());
        if (!mongoResult.has_value())
        {
            return std::nullopt;
        }

        return mongoResult.value();
    }
    catch (mongocxx::exception& e)
    {
        //std::string errInfo = std::format("Error in replacing document. Error: {}", e.what());
    }

    return std::nullopt;
}
