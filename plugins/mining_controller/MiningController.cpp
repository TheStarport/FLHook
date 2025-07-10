#include "PCH.hpp"

#include "API/FLHook/InfocardManager.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "API/Utils/Random.hpp"
#include "MiningController.hpp"

namespace Plugins
{
    void MiningControllerPlugin::SpawnNode(MiningSpawnPointDB& ms)
    {
        int selectedPositionIndex = rand() % ms.positions.size();
        int selectedArchetypeIndex = rand() % ms.nodeArchetypes.size();

        Vector eulerRotation = { Random::UniformFloat(-180.f, 180.f), Random::UniformFloat(-180.f, 180.f), Random::UniformFloat(-180.f, 180.f) };

        ResourceManager::SolarSpawnStruct solarInfo;
        solarInfo.nickname = ms.nicknamesVector.back();
        solarInfo.solarArchetypeId = ms.nodeArchetypes[selectedArchetypeIndex].archetype;
        solarInfo.loadoutArchetypeId = ms.nodeArchetypes[selectedArchetypeIndex].loadout;
        solarInfo.systemId = ms.system;
        solarInfo.pos = ms.positions[selectedPositionIndex];
        solarInfo.ori = EulerMatrix(eulerRotation);
        solarInfo.solarIds = ms.nodeIDS;

        Id createdSolar = ResourceManager::CreateSolarSimple(solarInfo);

        if (createdSolar)
        {
            ms.nicknamesVector.pop_back();

            ms.spawnedNodesCount++;
            ms.positions.erase(ms.positions.begin() + selectedPositionIndex);

            NodeInfo node = { &ms, solarInfo.nickname };
            miningNodeMap[createdSolar] = node;
            ms.cdProgress = 0;
        }
    }

    void MiningControllerPlugin::SpawnNewNodes()
    {
        for (auto& entry : miningAreasConfig.miningAreas)
        {
            if (entry.spawnedNodesCount >= entry.maxSpawnCount)
            {
                continue;
            }

            if (++entry.cdProgress >= entry.respawnCD)
            {
                SpawnNode(entry);
            }
        }
    }

    void MiningControllerPlugin::DestroyPendingNodes()
    {
        auto currTime = TimeUtils::UnixTime<std::chrono::seconds>();
        for (auto damagedNode = pendingDestructionNodes.begin(); damagedNode != pendingDestructionNodes.end();)
        {
            if (currTime > damagedNode->second)
            {
                pub::SpaceObj::Destroy(damagedNode->first, DestroyType::Fuse);
                damagedNode = pendingDestructionNodes.erase(damagedNode);
            }
            else
            {
                damagedNode++;
            }
        }
    }

    void MiningControllerPlugin::SaveZoneStatusToDb()
    {

        uint playerModifier = 0;
        if (config.scaleFieldRechargeWithPlayerCount)
        {
            PlayerData* pd = nullptr;
            while (pd = Players.traverse_active(pd))
            {
                playerModifier++;
            }
        }
        else
        {
            playerModifier = 1;
        }

        char szDataPath[MAX_PATH];
        GetUserDataPath(szDataPath);
        std::string scStatsPath = std::string(szDataPath) + R"(\Accts\MultiPlayer\mining_stats.txt)";
        FILE* file = fopen(scStatsPath.c_str(), "w");
        if (file)
        {
            fprintf(file, "[Zones]\n");
        }

        // Recharge the fields
        for (auto i = config.mapZoneBonus.begin(); i != config.mapZoneBonus.end(); i++)
        {
            auto& zone = i->second;
            zone.currReserve = std::min(zone.currReserve + (zone.rechargeRate * playerModifier), zone.maxReserve);

            if (file && !zone.zoneName.empty() && zone.maxReserve > 0 && zone.maxReserve != zone.currReserve)
            {
                fprintf(file, "%ls, %0.0f, %0.0f\n", zone.zoneName.c_str(), zone.currReserve, zone.mined);
            }
        }

        if (file)
        {
            fclose(file);
        }
    }

    bool MiningControllerPlugin::OnLoadSettings()
    {

        LoadJsonWithValidation(Config, config, "config/mining_controller.json");
        LoadJsonWithValidation(MiningAreasConfig, miningAreasConfig, "config/mining_controller_nodes.json");

        // TODO: Save to database instead

        /*// Read the last saved zone reserve.
        char szDataPath[MAX_PATH];
        GetUserDataPath(szDataPath);
        string scStatsPath = string(szDataPath) + R"(\Accts\MultiPlayer\mining_stats.txt)";
        if (ini.open(scStatsPath.c_str(), false))
        {
            while (ini.read_header())
            {
                if (ini.is_header("Zones"))
                {
                    while (ini.read_value())
                    {
                        string zoneName = ini.get_value_string(0);
                        if (zoneName.empty())
                        {
                            ConPrint(L"Incorrect entry in mining stats file!\n");
                            continue;
                        }
                        uint zoneID = CreateID(zoneName.c_str());
                        auto& zoneData = set_mapZoneBonus[zoneID];
                        zoneData.currReserve = ini.get_value_float(1);
                        zoneData.mined = ini.get_value_float(2);
                    }
                }
            }
            ini.close();
        }*/

        return true;
    }

    void MiningControllerPlugin::DestroyContainer(ClientId client)
    {
        const auto& iter = mapClients.find(client);
        if (iter == mapClients.end())
        {
            return;
        }

        if (!iter->second.deployedContainerId)
        {
            return;
        }

        const auto& cd = mapMiningContainers[iter->second.deployedContainerId];
        if (cd.loot1Count)
        {
            FLHook::GetResourceManager()->CreateLootSimple(cd.systemId, cd.jettisonPos, cd.loot1Id, cd.loot1Count, cd.clientId.GetShip().Unwrap(), false);
        }
        if (cd.loot2Count)
        {
            FLHook::GetResourceManager()->CreateLootSimple(cd.systemId, cd.jettisonPos, cd.loot2Id, cd.loot2Count, cd.clientId.GetShip().Unwrap(), false);
        }
        FLHook::GetResourceManager()->CreateLootSimple(
            cd.systemId, cd.jettisonPos, config.deployableContainerCommodity, 1, cd.clientId.GetShip().Unwrap(), false);

        mapMiningContainers.erase(iter->second.deployedContainerId);

        auto container = FLHook::GetObjInspect(iter->second.deployedContainerId);
        if (container)
        {
            container->ObjectDestroyed(DestroyType::Fuse, 0);
        }
    }

    void MiningControllerPlugin::OnClearClientInfo(ClientId client)
    {
        mapClients.erase(client);
        DestroyContainer(client);
    }

    void MiningControllerPlugin::OnCharacterSelect(ClientId client)
    {
        OnClearClientInfo(client); // TODO: send infocard updates
    }
    void MiningControllerPlugin::OnPlayerLaunchAfter(ClientId client, const ShipId& ship) { OnClearClientInfo(client); }

    void MiningControllerPlugin::OnDisconnect(ClientId client, EFLConnection conn) { OnClearClientInfo(client); }

    void MiningControllerPlugin::OnBaseEnter(BaseId base, ClientId client)
    {
        const auto& clientInfo = mapClients.find(client);
        if (clientInfo == mapClients.end())
        {
            return;
        }
        if (clientInfo->second.deployedContainerId && mapMiningContainers[clientInfo->second.deployedContainerId].systemId != client.GetSystemId().Unwrap())
        {
            DestroyContainer(client);
        }
    }

    void MiningControllerPlugin::OnSystemSwitchOutComplete(ClientId client, const ShipId& ship) { DestroyContainer(client); }

    void MiningControllerPlugin::CheckClientSetup(ClientId client)
    {
        const auto& equipDesc = client.GetEquipCargo().Unwrap()->equip;
        for (auto& equip : equipDesc)
        {
            if (!equip.mounted || !equip.is_internal())
            {
                continue;
            }
            const Archetype::Equipment* itemPtr = Archetype::GetEquipment(equip.archId.GetValue());
            if (itemPtr->get_class_type() == Archetype::ClassType::Tractor)
            {
                mapClients[client].equippedID = equip.archId;
                break;
            }
        }

        mapClients[client].shipClassMiningBonus = config.shipClassModifiersMap[client.GetShipArch().Value().Cast<Archetype::Ship>().Unwrap()->shipClass];
    }

    float MiningControllerPlugin::GetMiningYieldBonus(Id playerId, Id lootId)
    {
        const auto& bonusForId = config.idBonusMap.find(playerId);
        if (bonusForId != config.idBonusMap.end())
        {
            const auto& bonusForLoot = bonusForId->second.find(lootId);
            if (bonusForLoot != bonusForId->second.end())
            {
                return bonusForLoot->second;
            }
        }

        return 1.0f;
    }

    uint MiningControllerPlugin::GetAsteroidMiningYield(const MiningNodeInfo& node, ClientId client, bool isNode)
    {
        CLIENT_DATA& cd = mapClients[client];
        if (!cd.initialized)
        {
            cd.initialized = true;
            CheckClientSetup(client);
        }

        float miningBonus = GetMiningYieldBonus(cd.equippedID, node.itemArchId);
        float droppedAmount = Random::UniformFloat(node.countMin, node.countMax) * miningBonus;
        if (isNode)
        {
            droppedAmount *= cd.shipClassMiningBonus.bonusAsteroidNodes;
        }
        else
        {
            droppedAmount *= cd.shipClassMiningBonus.bonusAsteroidDestruction;
        }
        return static_cast<uint>(droppedAmount);
    }

    void MiningControllerPlugin::OnSolarDestroy(Solar* solar, DestroyType& destroyType, ShipId killerId)
    {

        if (auto container = mapMiningContainers.find(Id(solar->get_id())); container != mapMiningContainers.end())
        {
            DestroyContainer(container->second.clientId);
            return;
        }

        if (auto mineable = miningNodeMap.find(Id(solar->get_id())); mineable == miningNodeMap.end())
        {
            return;
        }

        if (ClientId killerPlayer = killerId.GetPlayer().Unwrap(); killerPlayer)
        {
            CLIENT_DATA& cd = mapClients[killerPlayer];
            if (!cd.initialized)
            {
                cd.initialized = true;
                CheckClientSetup(killerPlayer);
            }

            auto& nodeArray = config.miningSolarMap.at(solar->cobj->archetype->archId);
            const MiningNodeInfo& node = nodeArray.at(1);
            if (!node.itemArchId || !node.lootArchId || !node.countMin || !node.countMax)
            {
                return;
            }
            uint minedAmount = GetAsteroidMiningYield(node, killerPlayer, false);
            if (!minedAmount)
            {
                return;
            }
            ResourceManager::CreateLootSimple(solar->cobj->system, solar->cobj->position, node.itemArchId, minedAmount, killerPlayer.GetShip().Unwrap(), false);
        }
    }

    void MiningControllerPlugin::OnMineAsteroid(ClientId client, SystemId system, const Vector& pos, Id crateId, Id lootId, uint count)
    {

        CLIENT_DATA& data = mapClients[client];
        if (!data.initialized)
        {
            data.initialized = true;
            CheckClientSetup(client);
        }
        data.itemCount = count;
        data.lootID = lootId;
        returnCode = ReturnCode::SkipFunctionCall;
    }

    void MiningControllerPlugin::OnSpMunitionCollision(ClientId client, const SSPMunitionCollisionInfo& info)
    {
        // If this is not a lootable rock, do no other processing.
        if (info.projectileArchId != config.miningMunition || info.targetObjId != 0)
        {
            return;
        }

        returnCode = ReturnCode::SkipAll;

        CLIENT_DATA& cd = mapClients[client];

        if (!cd.itemCount)
        {
            return;
        }

        // use floats to ensure precision when applying various minor modifiers.
        float miningYield = static_cast<float>(cd.itemCount);
        Id lootId = cd.lootID;
        cd.itemCount = 0;
        cd.lootID = Id();

        auto cship = client.GetShip().Unwrap().GetValue().lock();
        if (!cship)
        {
            return;
        }

        SystemId clientSystemID = cship->system;
        CmnAsteroid::CAsteroidSystem* csys = CmnAsteroid::Find(clientSystemID.GetValue());
        if (!csys)
        {
            return;
        }

        Vector& pos = cship->position;

        // Find asteroid field that matches the best.
        for (CmnAsteroid::CAsteroidField* cfield = csys->FindFirst(); cfield; cfield = csys->FindNext())
        {
            if (!cfield->near_field(pos))
            {
                continue;
            }
            const Universe::IZone* zone = cfield->get_lootable_zone(pos);
            if (!zone || !zone->lootableZone)
            {
                continue;
            }

            const auto& zoneBonusData = config.mapZoneBonus.find(zone->zoneId);
            ZONE_BONUS* finalZone = nullptr;
            if (zoneBonusData != config.mapZoneBonus.end())
            {
                auto& zoneData = zoneBonusData->second;
                if (zoneData.currReserve == 0.0f)
                {
                    return;
                }

                if (zoneData.replacementLootID)
                {
                    lootId = zoneData.replacementLootID;
                }
                miningYield *= zoneData.multiplier;

                miningYield = std::max(miningYield, zoneData.currReserve);
                finalZone = &zoneData; // save ZONE_BONUS ref to update AFTER all the bonuses are applied
            }

            miningYield *= GetMiningYieldBonus(cd.equippedID, lootId) * config.globalModifier * cd.shipClassMiningBonus.bonusStandard;
            miningYield += cd.overminedFraction; // add the decimal remainder from last mining event.

            if (finalZone)
            {
                finalZone->currReserve -= miningYield;
                finalZone->mined += miningYield;
            }
            // If this ship is has another ship targetted then send the ore into the cargo
            // hold of the other ship.
            ClientId sendToClientID = client;
            const Archetype::Equipment* lootInfo = Archetype::GetEquipment(lootId.GetValue());

            bool foundContainer = false;
            GameObject* target = (GameObject*)cship->get_target();

            if (target)
            {
                Id targetId = Id(target->get_id());
                ObjectType objType = target->get_object_type();
                using namespace magic_enum::bitwise_operators;
                if (!(uint)(objType & (ObjectType::Fighter | ObjectType::Freighter | ObjectType::Transport | ObjectType::Gunboat | ObjectType::Cruiser |
                                       ObjectType::Capital | ObjectType::DestructibleDepot)))
                {
                    goto targetExit;
                }
                if (Vector::Distance(pos, target->get_position()) > 1200.0f)
                {
                    goto targetExit;
                }

                if (!(uint)(objType & ObjectType::DestructibleDepot))
                {
                    ClientId targetClientID = ClientId(target->csimple()->ownerPlayer);
                    if (targetClientID)
                    {
                        sendToClientID = targetClientID;
                    }
                    goto targetExit;
                }
                const auto& containerIter = mapMiningContainers.find(targetId);
                if (containerIter == mapMiningContainers.end())
                {
                    goto targetExit;
                }
                CONTAINER_DATA& container = containerIter->second;

                uint* lootCount = nullptr;
                if (container.loot1Id == lootId)
                {
                    foundContainer = true;
                    lootCount = &container.loot1Count;
                }
                else if (container.loot2Id == lootId)
                {
                    foundContainer = true;
                    lootCount = &container.loot2Count;
                }

                if (!foundContainer)
                {
                    goto targetExit;
                }

                *lootCount += static_cast<uint>(miningYield * config.miningContainerModifier);

                uint amountToJettison = static_cast<uint>(static_cast<float>(config.containerJettisonCount) / lootInfo->volume);
                if (*lootCount >= amountToJettison)
                {
                    ResourceManager::CreateLootSimple(
                        container.systemId, container.jettisonPos, lootId, amountToJettison, container.clientId.GetShip().Unwrap(), false);
                    *lootCount -= amountToJettison;
                }
            }
        targetExit:

            int miningYieldInt = static_cast<int>(miningYield);
            cd.overminedFraction = miningYield - miningYieldInt; // save the unused decimal portion for the next mining event.

            if (cd.miningSampleStart < time(nullptr))
            {
                float average = cd.miningEvents / 30.0f;
                if (average > config.miningCheatLogThreshold)
                {
                    // TODO: log potential cheater
                    /*
                    AddLog("NOTICE: high mining rate charname=%s rate=%0.1f/sec location=%0.0f,%0.0f,%0.0f system=%08x zone=%08x",
                           wstos((const wchar_t*)Players.GetActiveCharacterName(iClientID)).c_str(),
                           average,
                           vPos.x,
                           vPos.y,
                           vPos.z,
                           zone->iSystemID,
                           zone->iZoneID);
                           */
                }

                cd.miningSampleStart = static_cast<uint>(time(nullptr)) + 30;
                cd.miningEvents = 0;
            }

            if (foundContainer)
            {
                return;
            }

            cship = sendToClientID.GetShip().Unwrap().GetValue().lock();
            if (!cship)
            {
                return;
            }

            miningYieldInt = std::min(miningYieldInt, cship->get_space_for_cargo_type(lootInfo));

            time_t currTime = time(nullptr);
            if (!miningYieldInt && (currTime - cd.LastTimeMessageAboutBeingFull) > 2)
            {
                if (client != sendToClientID)
                {
                    sendToClientID.Message(std::format(L"{} is mining into your cargo hold, but your ship is full!", client.GetCharacterName().Unwrap()));
                    client.Message(std::format(L"{}'s cargo is now full.", sendToClientID.GetCharacterName().Unwrap()));
                    pub::Player::SendNNMessage(client.GetValue(), config.insufficientCargoSound.GetValue());
                }
                else
                {
                    sendToClientID.Message(L"Your cargo is now full.");
                }
                pub::Player::SendNNMessage(sendToClientID.GetValue(), config.insufficientCargoSound.GetValue());
                cd.LastTimeMessageAboutBeingFull = currTime;
            }
            else
            {
                pub::Player::AddCargo(sendToClientID.GetValue(), lootId.GetValue(), miningYieldInt, 1.0, false);
            }
            break;
        }
    }

    void MiningControllerPlugin::OnCargoJettison(ClientId client, const XJettisonCargo& cargo)
    {
        if (cargo.count != 1 || cargo.slot == 0)
        {
            return;
        }

        CEquipTraverser tr((uint)EquipmentClass::Cargo);
        auto cship = client.GetShip().Unwrap().GetValue().lock();
        if (!cship)
        {
            return;
        }

        CEquip* equip;
        equip = cship->equipManager.FindByID(cargo.slot);

        if (!equip)
        {

            // PrintUserCmdText(iClientID, L"ERR Issue when handling jettison event, contact developers. Error code %u", jc.iSlot);
            WARN(L"Error: jettisoned item not found! {0} {1} {2}",
                 { L"slot", std::to_wstring(cargo.slot) },
                 { L"ship", std::to_wstring(cargo.ship) },
                 { L"count", std::to_wstring(cargo.count) });

            return;
        }

        if (equip->archetype->archId != config.deployableContainerCommodity)
        {
            return;
        }
        returnCode = ReturnCode::SkipAll;

        const auto& cd = mapClients.find(client);
        if (cd != mapClients.end() && cd->second.deployedContainerId)
        {
            client.MessageErr(L"ERR A mining container is already deployed");
            return;
        }

        CONTAINER_DATA container;

        container.systemId = cship->system;

        Vector pos = cship->position;
        Matrix ori = cship->orientation;
        std::wstring commodityName1;
        std::wstring commodityName2;
        Id loot1Id;
        Id loot2Id;

        pos.TranslateZ(ori, -400);

        CmnAsteroid::CAsteroidSystem* csys = CmnAsteroid::Find(cship->system.GetValue());
        if (!csys)
        {
            client.MessageErr(L"ERR Not in a mineable field!");
            return;
        }

        bool alreadyFoundFirstMineable = false;
        // Find asteroid field that matches the best.
        for (CmnAsteroid::CAsteroidField* cfield = csys->FindFirst(); cfield; cfield = csys->FindNext())
        {
            Id tempLootId;
            if (!cfield->near_field(pos))
            {
                continue;
            }
            const Universe::IZone* zone = cfield->get_lootable_zone(pos);
            if (!zone || !zone->lootableZone)
            {
                continue;
            }
            const auto& zoneBonusData = config.mapZoneBonus.find(zone->zoneId);
            if (zoneBonusData != config.mapZoneBonus.end() && zoneBonusData->second.replacementLootID)
            {
                tempLootId = zoneBonusData->second.replacementLootID;
            }
            else
            {
                tempLootId = zone->lootableZone->dynamicLootCommodity;
            }

            if (loot1Id && tempLootId == loot1Id)
            {
                continue;
            }

            const GoodInfo* gi = GoodList::find_by_id(tempLootId.GetValue());
            if (!alreadyFoundFirstMineable)
            {
                loot1Id = tempLootId;
                alreadyFoundFirstMineable = true;
                commodityName1 = FLHook::GetInfocardManager()->GetInfoName(gi->idsName);
            }
            else
            {
                loot2Id = tempLootId;
                commodityName2 = FLHook::GetInfocardManager()->GetInfoName(gi->idsName);
                break;
            }
        }

        if (!loot1Id)
        {
            client.MessageErr(L"ERR Not in a mineable field!");
            return;
        }

        std::wstring fullContainerName;
        if (loot2Id)
        {
            fullContainerName = commodityName1 + L"/" + commodityName2 + L" Container";
        }
        else
        {
            fullContainerName = commodityName1 + L" Container";
        }

        ResourceManager::SolarSpawnStruct solarInfo;
        solarInfo.nickname = "player_mining_container_" + std::to_string(client.GetValue());
        solarInfo.solarArchetypeId = config.deployableContainerArchetype;
        solarInfo.loadoutArchetypeId = config.deployableContainerLoadout;
        solarInfo.nameOverride = fullContainerName;
        solarInfo.systemId = cship->system;
        solarInfo.pos = pos;
        solarInfo.ori = ori;
        solarInfo.solarIds = 540999 + client.GetValue();
        solarInfo.percentageHp = 1.0f;

        Id createdSolar = ResourceManager::CreateSolarSimple(solarInfo);
        if (createdSolar)
        {
            CONTAINER_DATA cd;
            cd.systemId = cship->system;
            pos.y -= 30;
            cd.jettisonPos = pos;
            cd.loot1Id = loot1Id;
            cd.loot1Name = commodityName1;
            if (loot2Id)
            {
                cd.loot2Id = loot2Id;
                cd.loot2Name = commodityName2;
            }
            cd.nameIDS = solarInfo.solarIds;
            cd.solarName = fullContainerName;
            cd.clientId = client;
            mapMiningContainers[createdSolar] = cd;
            mapClients[client].deployedContainerId = createdSolar;
            pub::Player::RemoveCargo(client.GetValue(), equip->SubObjId, 1);
        }
    }

    void MiningControllerPlugin::OnSolarColGrpDestroy(Solar* solar, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmgList)
    {
        if (!dmgList->inflictorPlayerId)
        {
            return;
        }

        auto miningSolarIter = config.miningSolarMap.find(solar->cobj->archetype->archId);
        if (miningSolarIter == config.miningSolarMap.end())
        {
            return;
        }

        if (auto destructionNodes = pendingDestructionNodes.find(solar->get_id()); destructionNodes != pendingDestructionNodes.end())
        {
            destructionNodes->second = TimeUtils::UnixTime<std::chrono::seconds>() + config.miningAsteroidDestructionTimer;
        }

        auto& nodeArray = miningSolarIter->second;
        const MiningNodeInfo& node = nodeArray.at(colGrp->colGrp->id);
        if (!node.itemArchId)
        {
            return;
        }
        Vector colGrpCenter;
        colGrp->GetCenterOfMass(colGrpCenter);

        uint minedAmount = GetAsteroidMiningYield(node, ClientId(dmgList->inflictorPlayerId), true);

        ResourceManager::CreateLootSimple(solar->cobj->system, colGrpCenter, node.itemArchId, minedAmount, ShipId(dmgList->inflictorId.GetValue()), false);
    }

    MiningControllerPlugin::MiningControllerPlugin(const PluginInfo& info) : Plugin(info)
    {
        AddTimer([this] { SaveZoneStatusToDb(); }, 120000);
        AddTimer([this] { DestroyPendingNodes(); }, 30000);
        AddTimer([this] { SpawnNewNodes(); }, 1000);
    }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
    return PluginInfo{
        .name = L"Mining Controller",
        .shortName = L"mining_controller",
        .versionMajor = PluginMajorVersion::V05,
        .versionMinor = PluginMinorVersion::V00
    };
};

SetupPlugin(MiningControllerPlugin);