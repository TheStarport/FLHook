#include "PCH.hpp"

#include "API/Utils/Random.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "Hyperjump.hpp"
#include "../cloak/Cloak.hpp"
#include "Core/PluginManager.hpp"

namespace Plugins
{
    HyperjumpPlugin::HyperjumpPlugin(const PluginInfo& info) : Plugin(info) {}

    void HyperjumpPlugin::SpawnJumpHole(ClientId client)
    {
        auto jd = clientData.find(client);
        if (jd == clientData.end())
        {
            return;
        }

        // TODO: Spawn the damn thing

        auto& data = jd->second.jumpDriveInfo;
        auto currTime = TimeUtils::UnixTime();

        std::string entryNickname = std::format("custom_jumphole_entry_{}_{}", client.GetValue(), currTime);
        std::string exitNickname = std::format("custom_jumphole_exit_{}_{}", client.GetValue(), currTime);
        auto posOri = jd->second.targetClient.GetShip().Handle().GetPositionAndOrientation().Handle();

        posOri.first.TranslateZ(posOri.second, 1300.f);

        ResourceManager::SolarSpawnStruct entryHoleSpawn = {
            .solarArchetypeId = data->entryJumpHoleArchetype,
            .solarLoadoutId = data->entryJumpHoleLoadout,
            .nickname = entryNickname,
            //.solarIds = Uhhh
            .nameOverride = std::format(L"Collapsing {} Jump Hole", jd->second.targetSystem.GetName().Handle()),
            .pos = posOri.first,
            .ori = posOri.second,
            .systemId = client.GetSystemId().Handle(),
            .destObj = Id(exitNickname),
        };

        ResourceManager::SolarSpawnStruct exitHoleSpawn = {
            .solarArchetypeId = data->exitJumpHoleArchetype,
            .solarLoadoutId = data->exitJumpHoleLoadout,
            .nickname = exitNickname,
            //.solarIds = Uhhh
            .nameOverride = L"Unstable Jumphole Exit Point",
            .pos = posOri.first,
            .ori = posOri.second,
            .systemId = client.GetSystemId().Handle(),
            .destObj = Id(entryNickname),
        };

        if (jd->second.targetClient)
        {
            entryHoleSpawn.destSystem = jd->second.targetClient.GetSystemId().Handle();
        }
        else
        {
            entryHoleSpawn.systemId = jd->second.targetSystem;
        }

        // TODO: review
        auto resManager = FLHook::GetResourceManager();
        resManager->CreateSolarSimple(entryHoleSpawn, this);
        resManager->CreateSolarSimple(exitHoleSpawn, this);
        ShutdownJumpDrive(client, true);
    }

    void HyperjumpPlugin::ProcessExpiringJumpHoles()
    {
        auto currTime = TimeUtils::UnixTime();
        for (auto jumpObj = jumpObjData.begin(); jumpObj != jumpObjData.end();)
        {
            if (currTime < jumpObj->second.lastUntil)
            {
                jumpObj++;
                continue;
            }

            if (jumpObj->second.dockingQueue.empty())
            {
                pub::SpaceObj::Destroy(jumpObj->second.pairedExit.GetId().Handle().GetValue(), DestroyType::Fuse);
                pub::SpaceObj::Destroy(jumpObj->first.GetValue(), DestroyType::Fuse);
                jumpObj = jumpObjData.erase(jumpObj);
            }
            else
            {
                for (auto ship = jumpObj->second.dockingQueue.begin(); ship != jumpObj->second.dockingQueue.end();)
                {
                    auto shipId = ShipId(ship->GetValue());
                    if (!shipId)
                    {
                        ship = jumpObj->second.dockingQueue.erase(ship);
                        continue;
                    }
                    ship++;
                }
            }
        }
    }

    void HyperjumpPlugin::ProcessChargingJumpDrives()
    {
        for (auto jd = clientData.begin(); jd != clientData.end();)
        {
            if (!jd->second.isCharging || jd->second.consumeFuelTarget == jd->second.consumedFuel)
            {
                jd++;
                continue;
            }

            uint amountToConsume = std::min((uint)jd->second.fuelToConsumePerSecond, jd->second.consumeFuelTarget - jd->second.consumedFuel);

            if (!TryFuelConsume(jd->first, jd->second.selectedFuel, amountToConsume))
            {
                jd->first.MessageErr(L"Insufficient fuel, discharging jump drive.");
                ShutdownJumpDrive(jd->first);
                jd = clientData.erase(jd);
                continue;
            }

            jd->second.consumedFuel += amountToConsume;
            if (jd->second.consumedFuel != jd->second.consumeFuelTarget)
            {
                jd++;
                continue;
            }
            // TODO: add charge fuses and jump fuses

            AddOneShotTimer([this, jd] { SpawnJumpHole(jd->first); }, jd->second.jumpDriveInfo->spawnDelay);

            jd = clientData.erase(jd);
        }
    }

    bool HyperjumpPlugin::CheckFuel(ClientId client, Id fuel, ushort amount)
    {
        for (auto& equip : client.GetEquipCargo().Handle()->equip)
        {
            if (equip.mounted || equip.archId != fuel)
            {
                continue;
            }

            if (equip.count < amount)
            {
                return false;
            }

            return true;
        }

        return false;
    }

    bool HyperjumpPlugin::TryFuelConsume(ClientId client, Id fuel, ushort amount)
    {
        for (auto& equip : client.GetEquipCargo().Handle()->equip)
        {
            if (equip.mounted || equip.archId != fuel)
            {
                continue;
            }

            if (equip.count < amount)
            {
                return false;
            }

            client.RemoveCargo(equip.id, amount);
            return true;
        }

        return false;
    }

    bool HyperjumpPlugin::JumpDriveCheck(ClientId client)
    {
        if (client.InSpace())
        {
            client.MessageErr(L"This command is only usable in space");
            return false;
        }

        if (IsPlayerCloaked(client))
        {
            client.MessageErr(L"You cannot jump while cloaked!");
            return false;
        }

        auto jumpIter = clientData.find(client);
        if (jumpIter == clientData.end())
        {
            client.MessageErr(L"You do not have a Jump Drive mounted!");
            return false;
        }

        return true;
    }

    bool HyperjumpPlugin::BeaconCheck(ClientId client)
    {
        if (client.InSpace())
        {
            client.MessageErr(L"This command is only usable in space");
            return false;
        }

        if (IsPlayerCloaked(client))
        {
            client.MessageErr(L"You cannot jump while cloaked!");
            return false;
        }

        auto jumpIter = beaconData.find(client);
        if (jumpIter == beaconData.end())
        {
            client.MessageErr(L"You do not have a Jump Drive beacon mounted!");
            return false;
        }

        return true;
    }

    void HyperjumpPlugin::ListJumpableSystems(ClientId client)
    {
        auto system = client.GetSystemId().Handle();

        auto jumpDriveIter = clientData.find(client);
        if (jumpDriveIter == clientData.end())
        {
            return;
        }

        auto systemData = config.jumpSystemData.find(system);
        if (systemData == config.jumpSystemData.end())
        {
            client.MessageErr(L"Jumping from this system is not possible");
            return;
        }

        for (uint depth = 1; depth <= jumpDriveIter->second.jumpDriveInfo->jumpRange; depth++)
        {
            auto systemList = systemData->second.availableSystemsPerDepth[depth];

            client.Message(std::format(L"Systems {} jumps away:", depth));
            std::wstring systemListString;
            for (SystemId jumpableSystem : systemList)
            {
                systemListString += std::wstring(L" [") + std::wstring(jumpableSystem.GetName().Handle()) + L"]";
            }
            client.Message(systemListString);
        }
    }

    std::optional<uint> HyperjumpPlugin::GetJumpRange(ClientId client, SystemId targetSystem)
    {
        auto system = client.GetSystemId().Handle();
        auto currSystemData = config.jumpSystemData.find(system);
        if (currSystemData == config.jumpSystemData.end())
        {
            client.MessageErr(L"Jumping from this system is not possible");
            return {};
        }

        auto jumpDriveData = clientData.find(client);
        if (jumpDriveData == clientData.end())
        {
            client.MessageErr(L"Jump Drive not mounted");
            return {};
        }

        if (system == targetSystem)
        {
            return 0;
        }

        auto targetSystemData = config.jumpSystemData.find(targetSystem);
        if (targetSystemData == config.jumpSystemData.end())
        {
            client.MessageErr(std::format(L"Target system of {} has no data defined, contact staff", targetSystem.GetName().Handle()));
            return {};
        }

        for (uint i = 1; i <= jumpDriveData->second.jumpDriveInfo->jumpRange; i++)
        {
            auto systemList = currSystemData->second.availableSystemsPerDepth[i];
            auto findResult = std::ranges::find(systemList.begin(), systemList.end(), targetSystem);
            if (findResult != systemList.end())
            {
                return i;
            }
        }

        client.MessageErr(L"Selected system is not in range.");
        return {};
    }

    bool HyperjumpPlugin::IsPlayerJumping(ClientId client)
    {
        auto iter = clientData.find(client);
        if (iter == clientData.end() || !iter->second.isCharging)
        {
            return false;
        }

        return true;
    }

    bool HyperjumpPlugin::IsPlayerCloaked(ClientId client)
    {
        auto cloakPlugin = std::static_pointer_cast<CloakPlugin>(PluginManager::i()->GetPlugin(CloakPlugin::pluginName).lock());
        if (!cloakPlugin)
        {
            return false;
        }

        return cloakPlugin->IsClientCloaked(client);
    }

    void HyperjumpPlugin::ShutdownJumpDrive(ClientId client, bool graceful)
    {
        auto cd = clientData.find(client);

        auto clientShip = client.GetShip();
        if (cd == clientData.end() || clientShip.HasError())
        {
            return;
        }

        auto& data = cd->second;
        auto ship = clientShip.Handle();

        if (data.currentJumpFuse)
        {
            ship.ExtinguishFuse(data.currentJumpFuse);
        }

        for (auto& fuse : data.jumpDriveInfo->chargeFuses)
        {
            ship.ExtinguishFuse(fuse);
        }

        if (data.targetClient && !graceful)
        {
            data.targetClient.MessageErr(std::format(L"{} has aborted jump drive charge-up", client.GetCharacterId().Handle()));
            beaconData.find(data.targetClient)->second.incomingClients.erase(client);
        }

        data.currentJumpFuse = Id();
        data.targetClient = ClientId();
        data.targetLocation = Transform();
        data.targetSystem = SystemId();

        data.isCharging = false;
        data.isBlindJumping = false;
    }

    concurrencpp::result<void> HyperjumpPlugin::UserCmdJumpStop(ClientId client)
    {
        if (!JumpDriveCheck(client))
        {
            co_return;
        }

        auto jumpIter = clientData.find(client);
        if (!jumpIter->second.isCharging)
        {
            client.MessageErr(L"You are not currently charging the Jump Drive!");
            co_return;
        }

        ShutdownJumpDrive(client);
        client.Message(L"Jump Drive shut down.");

        co_return;
    }

    concurrencpp::result<void> HyperjumpPlugin::UserCmdJumpList(ClientId client)
    {
        if (!JumpDriveCheck(client))
        {
            co_return;
        }

        ListJumpableSystems(client);
    }

    concurrencpp::result<void> HyperjumpPlugin::UserCmdJumpSectors(ClientId client, std::optional<SystemId> system)
    {
        if (!JumpDriveCheck(client))
        {
            co_return;
        }

        auto systemData = config.jumpSystemData.find(system.value());

        if (systemData == config.jumpSystemData.end())
        {
            client.MessageErr(L"System not jumpable");
            co_return;
        }

        client.Message(std::format(L"Available jump coordinates for {}:", system.value().GetName().Handle()));
        uint count = 1;
        for (auto& coord : systemData->second.systemCoords)
        {
            client.Message(std::format(L"{}. {}", count, coord.sector));
            ++count;
        }
    }

    void HyperjumpPlugin::InitiateJump(ClientId client, HyperjumpClientData& jumpDrive, SystemId targetSystem, bool printSectors)
    {
        auto targetSystemData = config.jumpSystemData.find(targetSystem);

        size_t index = Random::Uniform(size_t(0), targetSystemData->second.systemCoords.size() - 1);
        auto& coords = targetSystemData->second.systemCoords.at(index);

        jumpDrive.targetSystem = targetSystem;
        jumpDrive.isCharging = true;
        jumpDrive.targetLocation = coords.position;

        if (printSectors)
        {
            if (targetSystemData->second.systemCoords.size() == 1)
            {
                return;
            }
            client.Message(L"Alternate jump coordinates available, use /setsector to switch");

            index++;
            uint counter = 1;
            for (auto& coord : targetSystemData->second.systemCoords)
            {
                if (index == counter)
                {
                    client.Message(std::format(L"{}. {} - selected", counter, coord.sector));
                }
                else
                {
                    client.Message(std::format(L"{}. {}", counter, coord.sector));
                }
                counter++;
            }
        }
    }

    bool HyperjumpPlugin::SetFuelForRange(ClientId client, uint range)
    {
        auto& jumpDrive = clientData.find(client)->second;
        for (auto& equip : client.GetEquipCargo().Handle()->equip)
        {
            if (equip.mounted)
            {
                continue;
            }

            auto fuelIter = jumpDrive.jumpDriveInfo->fuelPerDistanceMap.find(equip.archId);
            if (fuelIter != jumpDrive.jumpDriveInfo->fuelPerDistanceMap.end())
            {
                if (fuelIter->second.size() <= range)
                {
                    continue;
                }

                auto fuelNeededCount = fuelIter->second[range];
                if (equip.count < fuelNeededCount)
                {
                    continue;
                }

                jumpDrive.selectedFuel = equip.archId;
                jumpDrive.consumeFuelTarget = fuelNeededCount;
                jumpDrive.fuelToConsumePerSecond =
                    ceilf(static_cast<float>(fuelNeededCount) / (static_cast<float>(jumpDrive.jumpDriveInfo->chargeTime) / 1000));
                break;
            }
        }

        if (!jumpDrive.selectedFuel)
        {
            client.MessageErr(L"Insufficient fuel to perform a blind jump!");
            return false;
        }

        return true;
    }

    concurrencpp::result<void> HyperjumpPlugin::UserCmdJumpBlind(ClientId client)
    {
        if (!JumpDriveCheck(client))
        {
            co_return;
        }

        auto currSystemData = config.jumpSystemData.find(client.GetSystemId().Handle());

        if (currSystemData == config.jumpSystemData.end())
        {
            client.MessageErr(L"Can't jump from current system!");
            co_return;
        }

        SystemId targetSystem = config.blindJumpOverrideSystem;
        auto jumpIter = clientData.find(client);

        if (!targetSystem)
        {
            auto& systemList = currSystemData->second.availableSystemsPerDepth[jumpIter->second.jumpDriveInfo->jumpRange];
            targetSystem = systemList.at(Random::Uniform(size_t(0), systemList.size() - 1));
        }

        auto targetSystemData = config.jumpSystemData.find(targetSystem);
        if (targetSystemData == config.jumpSystemData.end())
        {
            client.MessageErr(std::format(L"Can't jump to the target system: {}", targetSystemData->first.GetName().Handle()));
            co_return;
        }

        if (!SetFuelForRange(client, config.blindJumpRange))
        {
            co_return;
        }

        size_t index = Random::Uniform(size_t(0), targetSystemData->second.systemCoords.size() - 1);
        auto& coords = targetSystemData->second.systemCoords.at(index);

        jumpIter->second.targetSystem = targetSystem;
        jumpIter->second.isCharging = true;
        jumpIter->second.targetLocation = coords.position;
    }

    concurrencpp::result<void> HyperjumpPlugin::UserCmdJump(ClientId client, std::optional<SystemId> system)
    {
        if (!system.has_value())
        {
            client.MessageErr(L"Incorrect input. Command usage:");
            client.MessageErr(L"/jump stop - terminates charging of the Jump Drive");
            client.MessageErr(L"/jump list - print systems currently in range of your Jump Drive.");
            client.MessageErr(L"/jump sectors <systemName> - print possible arrival points within selected system");
            client.MessageErr(L"/jump start <systemName> - initiate jump to target system");
            co_return;
        }

        if (!JumpDriveCheck(client))
        {
            co_return;
        }

        auto jumpIter = clientData.find(client);
        if (jumpIter->second.isCharging)
        {
            client.MessageErr(L"Jump Drive already charging, to stop, type /jump stop");
            co_return;
        }

        auto range = GetJumpRange(client, system.value());
        if (!range.has_value())
        {
            co_return;
        }

        if (!SetFuelForRange(client, range.value()))
        {
            co_return;
        }

        InitiateJump(client, jumpIter->second, system.value(), true);

        co_return;
    }

    concurrencpp::result<void> HyperjumpPlugin::UserCmdCanJump(ClientId client, SystemId targetSystem)
    {
        if (!JumpDriveCheck(client))
        {
            co_return;
        }

        auto jumpIter = clientData.find(client);
        auto jumpRange = GetJumpRange(client, targetSystem);

        if (!jumpRange.has_value() || jumpRange.value() > jumpIter->second.jumpDriveInfo->jumpRange)
        {
            client.MessageErr(std::format(L"You cannot jump to {}.", targetSystem.GetName().Handle()));
            co_return;
        }

        client.MessageErr(std::format(L"{} is in jump range, {} systems away.", targetSystem.GetName().Handle(), jumpRange.value()));
        co_return;
    }

    bool HyperjumpPlugin::CanBeaconJump(ClientId client, ClientId targetClient)
    {

        auto currClientGroup = client.GetGroup().Unwrap();
        if (!currClientGroup)
        {
            client.MessageErr(L"You're not in a group! You and beacon jump target must be in the same group!");
            return false;
        }
        auto targetClientGroup = targetClient.GetGroup().Unwrap();
        if (!targetClientGroup)
        {
            client.MessageErr(
                std::format(L"{} is not in a group! You and beacon jump target must be in the same group!", targetClient.GetCharacterId().Handle()));
            return false;
        }

        if (currClientGroup != targetClientGroup)
        {
            client.MessageErr(std::format(L"You and {} are not in the same group!", targetClient.GetCharacterId().Handle()));
            return false;
        }

        auto jumpIter = clientData.find(client);
        auto beaconIter = beaconData.find(targetClient);

        if (beaconIter == beaconData.end())
        {
            client.MessageErr(std::format(L"{} has no beacon installed!", targetClient.GetCharacterId().Handle()));
            return false;
        }

        auto targetClientSystem = targetClient.GetSystemId().Handle();
        auto jumpRange = GetJumpRange(client, targetClientSystem);

        if (!jumpRange.has_value() || jumpRange.value() > (jumpIter->second.jumpDriveInfo->jumpRange + beaconIter->second.beaconInfo->jumpRangeExtension))
        {
            client.MessageErr(std::format(L"You cannot jump to {}.", targetClient.GetCharacterId().Handle()));
            return false;
        }

        return true;
    }

    concurrencpp::result<void> HyperjumpPlugin::UserCmdCanBeacon(ClientId client, ClientId targetClient)
    {
        if (!JumpDriveCheck(client))
        {
            co_return;
        }

        if (CanBeaconJump(client, targetClient))
        {
            client.Message(std::format(L"You can jump to {}.", targetClient.GetCharacterId().Handle()));
        }
    }

    void HyperjumpPlugin::BeaconRequestTimeout(ClientId targetClient)
    {
        auto iter = pendingBeaconRequestMap.find(targetClient);

        if (iter != pendingBeaconRequestMap.end())
        {
            iter->second.MessageErr(std::format(L"Your beacon request to {} timed out", targetClient.GetCharacterId().Handle()));
            targetClient.MessageErr(std::format(L"Beacon request from {} timed out", iter->second.GetCharacterId().Handle()));
            pendingBeaconRequestMap.erase(iter);
        }
    }

    concurrencpp::result<void> HyperjumpPlugin::UserCmdJumpBeacon(ClientId client, ClientId targetClient)
    {
        if (!JumpDriveCheck(client))
        {
            co_return;
        }

        if (!CanBeaconJump(client, targetClient))
        {
            co_return;
        }

        if (IsPlayerCloaked(targetClient))
        {
            client.MessageErr(std::format(L"{} is cloaked, beacon is inoperable", targetClient.GetCharacterId().Handle()));
            targetClient.MessageErr(std::format(L"{} attempted to request a beacon jump, but you're cloaked!", client.GetCharacterId().Handle()));
            co_return;
        }

        auto pendingRequest = pendingBeaconRequestMap.find(targetClient);
        if (pendingRequest != pendingBeaconRequestMap.end())
        {
            if (pendingRequest->second == client)
            {
                client.MessageErr(L"You already have a pending jump request sent to this player!");
                co_return;
            }

            client.MessageErr(std::format(L"Target client already has a pending beacon request from {}!", pendingRequest->second.GetCharacterId().Handle()));
            co_return;
        }

        targetClient.Message(std::format(L"{} is attempting a beacon jump. Accept the request by typing /jump accept within the next {} seconds.",
                                         client.GetCharacterId().Handle(),
                                         config.beaconRequestTimeout / 1000));

        AddOneShotTimer([this, targetClient] { BeaconRequestTimeout(targetClient); }, config.beaconRequestTimeout);
    }

    concurrencpp::result<void> HyperjumpPlugin::UserCmdAcceptBeacon(ClientId client)
    {
        if (!BeaconCheck(client))
        {
            co_return;
        }

        auto requestIter = pendingBeaconRequestMap.find(client);
        if (requestIter == pendingBeaconRequestMap.end())
        {
            client.MessageErr(L"You have no pending beacon requests!");
            co_return;
        }

        pendingBeaconRequestMap.erase(requestIter);

        if (!CanBeaconJump(requestIter->second, client))
        {
            client.MessageErr(std::format(L"You and {} are now of range for the beacon jump, aborting!", requestIter->second.GetCharacterId().Handle()));
            requestIter->second.MessageErr(std::format(L"You and {} are now of range for the beacon jump, aborting!", client.GetCharacterId().Handle()));
            co_return;
        }

        auto range = GetJumpRange(client, requestIter->second.GetSystemId().Handle());
        if (!range.has_value())
        {
            co_return;
        }

        auto beaconIter = beaconData.find(client);
        if (!CheckFuel(client, beaconIter->second.beaconInfo->fuel, beaconIter->second.beaconInfo->fuelAmount))
        {
            client.MessageErr(L"You don't have enough fuel to accept the beacon request!");
            requestIter->second.MessageErr(std::format(L"{} doesn't have enough fuel to accept the beacon request!", client.GetCharacterId().Handle()));
            co_return;
        }

        if (!SetFuelForRange(client, range.value()))
        {
            // Check fuel on the origin, check fuel on target, then consume fuel on the beacon, then set consumption on the origin
            co_return;
        }

        auto requestingClient = requestIter->second;

        auto jumpIter = clientData.find(requestingClient);
        auto& jumpData = jumpIter->second;

        jumpData.isCharging = true;
        jumpData.targetClient = client;
    }

    concurrencpp::result<void> HyperjumpPlugin::UserCmdSetSector(ClientId client, uint index)
    {
        if (!JumpDriveCheck(client))
        {
            co_return;
        }

        auto jumpIter = clientData.find(client);
        if (!jumpIter->second.isCharging)
        {
            client.MessageErr(L"No system selected, jump drive must be charging!");
            co_return;
        }

        if (!jumpIter->second.targetSystem)
        {
            client.MessageErr(L"You're charging a jump towards a beacon, you can't set a sector.");
            co_return;
        }

        if (jumpIter->second.isBlindJumping)
        {
            client.MessageErr(L"You're charging a blind jump, can't set coordinates");
            co_return;
        }

        auto systemIter = config.jumpSystemData.find(jumpIter->second.targetSystem);
        if (!index || index > systemIter->second.systemCoords.size())
        {
            client.MessageErr(std::format(L"Only {} coordinate sets are available, invalid selection.", systemIter->second.systemCoords.size()));
            co_return;
        }

        jumpIter->second.targetLocation = systemIter->second.systemCoords.at(index - 1).position;
    }

    bool HyperjumpPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/hyperjump.json");

        AddTimer([this] { ProcessChargingJumpDrives(); }, 1000);
        AddTimer([this] { ProcessExpiringJumpHoles(); }, 1000);

        return true;
    }

    void HyperjumpPlugin::OnPlayerLaunchAfter(ClientId client, const ShipId& shipId)
    {
        clientData.erase(client);
        beaconData.erase(client);

        for (auto& equip : client.GetEquipCargo().Handle()->equip)
        {
            if (!equip.mounted)
            {
                continue;
            }

            auto jdIter = config.jumpDriveMap.find(equip.archId);
            if (jdIter != config.jumpDriveMap.end())
            {
                if (!(jdIter->second.shipClasses & client.GetShipArch().Handle().GetValue()->archType))
                {
                    continue;
                }

                HyperjumpClientData data = { .jumpDriveInfo = &jdIter->second };
                clientData[client] = data;
                continue;
            }

            auto beaconIter = config.beaconMap.find(equip.archId);
            if (beaconIter != config.beaconMap.end())
            {
                BeaconClientData data = { .beaconInfo = &beaconIter->second };
                beaconData[client] = data;
            }
        }
    }

    void HyperjumpPlugin::OnClearClientInfo(ClientId client)
    {
        clientData.erase(client);
        beaconData.erase(client);
    }

    std::optional<DOCK_HOST_RESPONSE> HyperjumpPlugin::OnDockCall(const ShipId& shipId, const ObjectId& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response)
    {
        auto jumpObj = jumpObjData.find(spaceId.GetId().Handle());
        if (jumpObj == jumpObjData.end())
        {
            return {};
        }

        auto client = shipId.GetPlayer();
        if (client.HasError())
        {
            return {};
        }

        if (jumpObj->second.jumpCapacity == 0)
        {
            client.Handle().MessageErr(L"Jumphole exceeded its capacity, too unstable to proceed!");
            return { DOCK_HOST_RESPONSE::DockDenied };
        }

        auto currTime = TimeUtils::UnixTime();
        if (currTime - 1000 > jumpObj->second.lastUntil && jumpObj->second.dockingQueue.empty())
        {
            client.Handle().MessageErr(L"Jumphole is about to collapse, too unstable to proceed!");
            return { DOCK_HOST_RESPONSE::DockDenied };
        }

        if (jumpObj->second.jumpCapacity != -1)
        {
            --jumpObj->second.jumpCapacity;
        }

        jumpObj->second.dockingQueue.insert(shipId.GetId().Handle());
        shipToJumpObjData[shipId.GetId().Handle()] = spaceId;
        return {};
    }

    void HyperjumpPlugin::OnJumpInComplete(SystemId system, const ShipId& ship)
    {
        Id shipId = ship.GetId().Handle();
        auto iter = shipToJumpObjData.find(shipId);
        if (iter == shipToJumpObjData.end())
        {
            return;
        }
        auto jumpObjIter = jumpObjData.find(iter->second.GetId().Handle());
        if (jumpObjIter == jumpObjData.end())
        {
            return;
        }
        jumpObjIter->second.dockingQueue.erase(shipId);
        shipToJumpObjData.erase(shipId);
    }

    void HyperjumpPlugin::OnRequestCancel(ClientId client, EventRequestType eventType, const ShipId& ship, const ObjectId& dockTarget, const uint unk1)
    {
        if (eventType != EventRequestType::StationJumpDock)
        {
            return;
        }

        Id shipId = ship.GetId().Handle();
        auto iter = jumpObjData.find(dockTarget.GetId().Handle());
        if (iter != jumpObjData.end())
        {
            if (iter->second.dockingQueue.count(shipId))
            {
                iter->second.dockingQueue.erase(shipId);
                iter->second.jumpCapacity++;
                shipToJumpObjData.erase(shipId);
            }
        }
    }

    void HyperjumpPlugin::OnShipExplosionHit(Ship* ship, ExplosionDamageEvent* explosion, DamageList* dmgList)
    {
        if (dmgList->damageCause != DamageCause::CruiseDisrupter)
        {
            return;
        }

        ClientId client = ClientId(ship->cship()->ownerPlayer);
        if (!client)
        {
            return;
        }

        auto jd = clientData.find(client);
        if (jd == clientData.end() || !jd->second.isCharging || !jd->second.jumpDriveInfo->cdDisruptsCharge)
        {
            return;
        }

        client.MessageErr(L"Jump drive disrupted, charging failed");
        ShutdownJumpDrive(client);
    }

    void HyperjumpPlugin::OnSystemSwitchOutPacketAfter(ClientId client, FLPACKET_SYSTEM_SWITCH_OUT& packet)
    {
        auto jd = clientData.find(client);
        if (jd == clientData.end())
        {
            return;
        }

        if (!jd->second.isCharging)
        {
            return;
        }

        client.MessageErr(L"Entering a jump tunnel, discharging Jump Drive.");
        ShutdownJumpDrive(client);
    }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Hyperjump",
	    .shortName = std::wstring(HyperjumpPlugin::pluginName),
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(HyperjumpPlugin);
