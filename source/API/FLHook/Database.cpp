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


    bsoncxx::builder::basic::array equipmentArray;

        for (const auto& equip : newPlayer->currentEquipAndCargo)
        {
            bool commodity;
            pub::IsCommodity(equip.archId, commodity);
            if (!commodity)
            {
                std::string equipStr;
                equipStr = equip.hardPoint.value;
                equipmentArray.append(make_document(
                    kvp("archId", static_cast<long long>(equip.archId)), kvp("hardPoint", equipStr), kvp("mounted", equip.mounted), kvp("health",
       equip.health)));
            }
        }

        bsoncxx::builder::basic::array cargoArray;

        for (const auto& cargo : newPlayer->currentEquipAndCargo)
        {
            bool commodity;
            pub::IsCommodity(cargo.archId, commodity);
            if (commodity)
            {
                std::string equipStr;
                equipStr = cargo.hardPoint.value;
                cargoArray.append(make_document(
                    kvp("archId", static_cast<long long>(cargo.archId)), kvp("hardPoint", equipStr), kvp("mounted", cargo.mounted), kvp("health",
       cargo.health)));
            }
        }


    bsoncxx::builder::basic::document visitArray;

    for (auto visit = newPlayer->visitLists.begin(); visit != newPlayer->visitLists.end(); ++visit)
    {
	  visitArray.append(kvp(std::to_string(visit->key),static_cast<bsoncxx::types::b_int64>(visit->value.visitValue)));
    }

    bsoncxx::builder::basic::document reputationArray;


    for ( auto& rep : newPlayer->repList)
    {
		reputationArray.append(kvp(std::to_string(rep.hash),rep.reptuation));
    }

	bsoncxx::builder::basic::document collisionGroups;

	for (const auto& col : newPlayer->currentCollisionGroups)
	{
	  collisionGroups.append(kvp(std::to_string(col.id),col.health));
	}

	//Initialize these on the database even though they'll just be empty.
	bsoncxx::builder::basic::document shipTypesKilled;
	bsoncxx::builder::basic::array basesVisited;
	bsoncxx::builder::basic::array systemsVisited;
	bsoncxx::builder::basic::array jumpHolesVisited;
	bsoncxx::builder::basic::document weaponGroups;


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
									kvp("currentRoom", static_cast<long long>(newPlayer->currentRoom)),
                                    kvp("canTradeLane", 1),
                                    kvp("killCount", 0),
                                    kvp("missionFailureCount", 0),
                                    kvp("missionSuccessCount", 0),
                                    kvp("shipHash", static_cast<long long>(newPlayer->shipHash)),
                                    kvp("system", static_cast<long long>(newPlayer->system)),
                                    kvp("totalTimePlayed", 0.0000f),
                                    kvp("baseCostume", dbBaseCostume),
                                    kvp("commCostume", dbCommCostume),
                                    kvp("reputation", reputationArray),
                                    kvp("visits", visitArray),
									kvp("cargo", cargoArray),
									kvp("baseCargo",cargoArray),
									kvp("equipment", equipmentArray),
									kvp("baseEquipment",equipmentArray),
									kvp("collisionGroups", collisionGroups),
									kvp("shipTypesKilled", shipTypesKilled),
									kvp("systemsVisited", systemsVisited),
									kvp("basesVisited",basesVisited),
									kvp("jumpHolesVisited,", jumpHolesVisited),
									kvp("weaponGroups", weaponGroups));

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
    auto charUpdateDoc = make_document(kvp("$set", make_document(kvp("characters", characterArray))));

    accounts.collection.update_one(findRes->view(), charUpdateDoc.view());
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
Account Database::GetOrCreateAccount(std::string accountId, bool deferred)
{
  Account account;

   auto accountsOpt = GetCollection("accounts");
    if (!accountsOpt.has_value())
    {
        // TODO: Log issue

    }

    auto& accountDb = accountsOpt.value();

	auto accDoc = accountDb.collection.find_one(make_document(kvp("_id", accountId)));
	if (!accDoc.has_value())
	{
	  bsoncxx::builder::basic::array characters;
	  auto newAccDoc = make_document(kvp("_id", accountId), kvp("characters", characters));
	  accountDb.collection.insert_one(newAccDoc.view());
	}

	account._id = accountId;

	bsoncxx::array::view characters = accDoc.value()["characters"].get_array();

	for (const auto& i : characters)
	{

	  std::string charId = i.get_string().value.data();
	  account.characters.emplace_back(GrabCharacterById(charId));

	}

	return account;
}

//TODO: Reflect-cpp claims to have serialization and deserialization support for Bson, look into.
Character Database::GrabCharacterById(std::string objId)
{
	Character character;

   auto accountsOpt = GetCollection("accounts");
    if (!accountsOpt.has_value())
    {
        // TODO: Log issue

    }
	auto& accountsDb = accountsOpt.value();

	auto charDocOpt = accountsDb.collection.find_one(make_document(kvp("_id", objId)));
	if (!charDocOpt.has_value())
	{
	  //TODO: Log Issue

	}

	auto& charDoc = charDocOpt.value();

	character._id = charDoc.view()["_id"].get_oid().value;
	character.characterName = charDoc.view()["characterName"].get_string().value;
	character.money	= charDoc.view()["money"].get_int32().value;
	character.rank = charDoc.view()["rank"].get_int32().value;
	character.repGroup = charDoc.view()["repGroup"].get_string();
	character.interfaceState = charDoc.view()["interfaceState"].get_int32().value;
	character.hullStatus = static_cast<float>(charDoc.view()["hullStatus"].get_double().value);
	character.baseHullStatus = static_cast<float>(charDoc.view()["baseHullStatus"].get_double().value);
	character.canDock = charDoc.view()["canDoc"].get_bool();
	character.canTradeLane = charDoc.view()["canTradeLane"].get_bool();
	character.lastDockedBase = charDoc.view()["lastDockedBase"].get_int64();
	character.currentRoom = charDoc.view()["currentRoom"].get_int64();
	character.killCount = charDoc.view()["killCount"].get_int32();
	character.missionFailureCount = charDoc.view()["missionFailureCount"].get_int32();
	character.missionSuccessCount = charDoc.view()["missionSuccessCount"].get_int32();
	character.shipHash = charDoc.view()["shipHash"].get_int32();
	character.system = charDoc.view()["system"].get_int32();
	character.totalTimePlayed = charDoc.view()["totalTimePlayed"].get_int32();

	bsoncxx::document::view baseCostumeDoc = charDoc.view()["baseCostume"].get_document();
	Costume baseCostume;
	baseCostume.body = baseCostumeDoc["body"].get_int32();
	baseCostume.head = baseCostumeDoc["head"].get_int32();
	baseCostume.leftHand = baseCostumeDoc["leftHand"].get_int32();
	baseCostume.rightHand = baseCostumeDoc["rightHand"].get_int32();
	character.baseCostume.value() = baseCostume;

	bsoncxx::document::view commCostumeDoc = charDoc.view()["baseCostume"].get_document();
	Costume commCostume;
	commCostume.body = commCostumeDoc["body"].get_int32();
	commCostume.head = commCostumeDoc["head"].get_int32();
	commCostume.leftHand = commCostumeDoc["leftHand"].get_int32();
	commCostume.rightHand = commCostumeDoc["rightHand"].get_int32();
	character.commCostume.value() = commCostume;

		//This is bad, but I can't find any easier way to convert a mongo array to a c-array.
	auto posArray =  charDoc.view()["pos"].get_array().value;
	int counter = 0;
	for (const auto& i : posArray)
	{
	  if (counter >= 3){

		break;
	  }
	  character.pos.value()[counter] = static_cast<float>(i.get_double());
	  counter++;
	}
	auto rotArray = charDoc.view()["rot"].get_array().value;
	counter = 0;
	for (const auto& i : rotArray){
	  	  if (counter >= 3){

		break;
	  }
	  character.rot.value()[counter] = static_cast<float>(i.get_double());
	  counter++;
	}
	bsoncxx::array::view flCargoArray = charDoc.view()["cargo"].get_array();

	FLCargo cargo = {};

	for (const auto& i : flCargoArray)
	{
	 const auto& j = i.get_document().value;

	 cargo.id =  j["id"].get_int64();
	 cargo.amount = j["amount"].get_int64();
	 cargo.health = static_cast<float>(j["health"].get_double());
	 cargo.isMissionCargo = j["isMissionCargo"].get_bool();
	 character.cargo.emplace_back(cargo);
	}

	bsoncxx::array::view flBaseCargoArray = charDoc.view()["baseCargo"].get_array();

	for (const auto& i : flBaseCargoArray)
	{
	 const auto& j = i.get_document().value;

	 cargo.id =  j["id"].get_int64();
	 cargo.amount = j["amount"].get_int64();
	 cargo.health = static_cast<float>(j["health"].get_double());
	 cargo.isMissionCargo = j["isMissionCargo"].get_bool();
	 character.baseCargo.emplace_back(cargo);
	}

	Equipment equipment = {};

	bsoncxx::array::view flEquipArray = charDoc.view()["equipment"].get_array();

	for (const auto& i : flEquipArray)
	{
	  const auto& j = i.get_document().value;

	  equipment.id = j["id"].get_int64();
	  equipment.hardPoint = j["hardPoint"].get_string().value;
	  equipment.health	= static_cast<float>(j["health"].get_double());

	  character.equipment.emplace_back(equipment);
	}

	bsoncxx::array::view flBaseEquipArray = charDoc.view()["baseEquipment"].get_array();

	for (const auto& i : flBaseEquipArray)
	{
	  const auto& j = i.get_document().value;

	  equipment.id = j["id"].get_int64();
	  equipment.hardPoint = j["hardPoint"].get_string().value;
	  equipment.health	= static_cast<float>(j["health"].get_double());

	  character.baseEquipment.emplace_back(equipment);
	}

	bsoncxx::document::view flCollisionGroups = charDoc.view()["collisionGroups"].get_document();

	for(const auto& i: flCollisionGroups)
	{
	  character.collisionGroups.emplace(std::stoi(i.key().data()),i.get_double());

	}

	bsoncxx::document::view flVisits = charDoc.view()["visits"].get_document();

	for (const auto& i : flVisits)
	{
	auto key = std::stoi(i.key().data());
	character.visits.emplace(key, i.get_int32());
	}

	bsoncxx::document::view flReputation = charDoc.view()["reputation"].get_document();

	for(const auto& i : flReputation)
	{
	  	auto key = std::stoi(i.key().data());
		character.reputation.emplace(key, i.get_int32());
	}

	bsoncxx::array::view systemsVisitedArray = charDoc.view()["systemsVisited"].get_array().value;
	for(const auto& i : systemsVisitedArray)
	{
	  character.systemsVisited.emplace_back(i.get_int64());
	}

	bsoncxx::array::view basesVisitedArray = charDoc.view()["basesVisited"].get_array().value;
	for(const auto& i : basesVisitedArray)
	{
	   character.basesVisited.emplace_back(i.get_int64());
	}
	bsoncxx::array::view holesVisitedArray = charDoc.view()["jumpHolesVisited"].get_array().value;
	for(const auto& i : holesVisitedArray)
	{
	   character.jumpHolesVisited.emplace_back(i.get_int64());
	}

	NpcVisit npcVisit = {};

	bsoncxx::array::view npcVisitsArray = charDoc.view()["npcVisits"].get_array().value;

	for(const auto& i : npcVisitsArray)
	{
	  const auto& j = i.get_document().value;

	  npcVisit.id = j["id"].get_int64();
	  npcVisit.baseId = j["baseId"].get_int64();
	  npcVisit.interactionCount = j["interactionCount"].get_int32();
	  npcVisit.missionStatus = j["missionStatus"].get_int32();

	  character.npcVisits.emplace_back(npcVisit);
	}

	bsoncxx::document::view shipTypesKilled = charDoc.view()["shipTypesKilled"].get_document();
	for(const auto& i : shipTypesKilled)
	{
	  	auto key = std::stoll(i.key().data());
		character.shipTypesKilled.emplace(key, i.get_int32());
	}

	bsoncxx::array::view weaponGroupArray = charDoc.view()["weaponGroups"].get_array();

	std::vector<std::string> weaponGroup;

	for(const auto& i : weaponGroupArray)
	{

	   for (const auto& j : i.get_document().value)
	   {
		 weaponGroup.emplace_back(j.get_string().value);
	   }
	   auto key = std::stoi(i.key().data());

	   character.weaponGroups.emplace(key, weaponGroup);
	   weaponGroup.clear();
	}


	return character;
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
