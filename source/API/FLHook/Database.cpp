#include "PCH.hpp"

#include "API/FLHook/Database.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

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

void Database::CreateCharacter(std::string accountId, VanillaLoadData* newPlayer)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;


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


    for (auto visit = newPlayer->visitLists.begin(); visit != newPlayer->visitLists.end(); ++visit )
    {
     visitArray.append((static_cast<bsoncxx::types::b_int64>(*visit.key()), static_cast<bsoncxx::types::b_int64>(*visit.value())));
    }

    bsoncxx::builder::basic::array reputationArray;

   /* for (const auto& rep : newCharTemplate.reputationOverrides)
    {
        visitArray.append(kvp(rep.first, rep.second));
    }*/

   std::wstring charNameWide =  reinterpret_cast<const wchar_t*>(newPlayer->name.c_str());
   std::string charName = StringUtils::wstos(charNameWide);


    // We cast to long long as mongo does not care about sign and we want to prevent any sort of signed overflow
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

    const auto updateDoc = accounts.find_one(make_document(kvp("characterName", charName)).view());
    const auto elem = updateDoc.value()["_id"];
    auto str = elem.get_oid().value.to_string();
    const auto findRes = accounts.find_one(make_document(kvp("_id", accountId)));

    // Update the account's character list to include the newly created character.
    if (!findRes.has_value())
    {
        std::cout << "Account not found.";
        throw;
    }

    bsoncxx::builder::basic::document updateBuilder;
    bsoncxx::builder::basic::array characterArray;

    characterArray.append(str);
    updateBuilder.append(kvp("$set", make_document(kvp("characters", characterArray))));
    accounts.update_one(findRes->view(), updateBuilder.view());
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
