#include "PCH.hpp"

#include "Cloak.hpp"
#include "../hyperjump/Hyperjump.hpp"
#include "Core/PluginManager.hpp"

namespace Plugins
{
    // Returns false if the ship has no fuel to operate its cloaking device.
    bool CloakPlugin::ConsumeFuel(ClientId client)
    {
        auto info = clientCloakData.find(client);
        if (info == clientCloakData.end())
        {
            return true;
        }

        if (info->second.admin || info->second.disableJumpFuelConsumption)
        {
            return true;
        }

        auto ship = client.GetShip();
        if (ship.HasError())
        {
            return true;
        }

        auto cship = ship.Handle();
        CEquipTraverser tr((uint)EquipmentClass::Cargo);
        CECargo* cargo;
        auto& fuelUsage = info->second.cloakInfo->fuelMap;

        while (cargo = reinterpret_cast<CECargo*>(cship.GetEquipmentManager().Handle()->Traverse(tr)))
        {
            auto fuelIter = fuelUsage.find(cargo->archetype->archId);
            if (fuelIter == fuelUsage.end())
            {
                continue;
            }

            float currFuelUsage = fuelIter->second.staticUse;
            if (fuelIter->second.linearUse != 0.0f || fuelIter->second.squaredUse != 0.0f)
            {
                auto dirAndSpeed = cship.GetVelocityAndSpeed().Handle();

                currFuelUsage += fuelIter->second.linearUse * dirAndSpeed.second;
                currFuelUsage += fuelIter->second.linearUse * dirAndSpeed.second * dirAndSpeed.second;
            }
            info->second.fuelUsageOverflow += currFuelUsage;
            uint totalFuelUsage = static_cast<uint>(std::max(info->second.fuelUsageOverflow, 0.0f));
            if (cargo->count >= totalFuelUsage)
            {
                if (totalFuelUsage >= 25) // Wait until the fuel usage reaches 25 to actually call RemoveCargo, as it's an expensive operation.
                {
                    info->second.fuelUsageOverflow -= static_cast<float>(totalFuelUsage);
                    pub::Player::RemoveCargo(client.GetValue(), cargo->SubObjId, totalFuelUsage);
                }
                return true;
            }
        }

        return false;
    }

    void CloakPlugin::ProcessFuel()
    {
        int64 now = TimeUtils::UnixTime();

        for (auto ci = clientCloakData.begin(); ci != clientCloakData.end();)
        {
            ClientId client = ci->first;

            auto ship = client.GetShip();
            if (ship.HasError())
            {
                ci = clientCloakData.erase(ci);
                continue;
            }

            // code to check if the player is disrupted. We run this separately to not cause issues with the bugfix
            // first we check if it's 0. If it is, it's useless to process the other conditions, even if it means an additional check to begin with.
            if (ci->second.disruptedUntilTime && ci->second.disruptedUntilTime < now)
            {
                client.Message(L"Cloaking Device rebooted after disruption.");
                ci->second.disruptedUntilTime = 0;
            }

            switch (ci->second.cloakState)
            {
                case CloakState::Charging:
                    if (!ConsumeFuel(client))
                    {
                        client.Message(L"Cloaking device shutdown, no fuel");
                        SetState(client, CloakState::Off);
                    }
                    else if ((ci->second.cloakTime + ci->second.cloakInfo->warmupTime) < now)
                    {
                        SetState(client, CloakState::On);
                    }
                    break;

                case CloakState::On:
                    if (!ConsumeFuel(client) && ci->second.cloakTime + ci->second.cloakInfo->activationTime < now)
                    {
                        client.Message(L"Cloaking device shutdown, no fuel");
                        SetState(client, CloakState::Off);
                    }
                    break;
            }
        }
    }

    void CloakPlugin::ProcessDisruptors()
    {
        int64 now = TimeUtils::UnixTime();

        for (auto& iter : clientDisruptorData)
        {
            if (!iter.second.cooldownUntil || iter.second.cooldownUntil > now)
            {
                continue;
            }

            iter.first.Message(L"Cloak Disruptor cooldown complete.");
            iter.first.GetShip().Handle().ExtinguishFuse(iter.second.disruptorInfo->effect);

            iter.second.cooldownUntil = 0;
        }
    }

    /* void CloakPlugin::SendCloakStatusExternal()
    {
        if (set_enableCloakSystemOverride && cloakStateChanged && timeNow % 5 == 0)
        {
            stringstream stream;
            minijson::object_writer writer(stream);
            string sPlayer = "players";
            minijson::object_writer pwc = writer.nested_object(sPlayer.c_str());

            for (auto& iter : mapClientsCloak)
            {
                if (iter.second.iState != STATE_CLOAK_ON)
                {
                    continue;
                }
                uint playerSystem = Players[iter.first].iSystemID;
                if (!mapObscuredSystems.count(playerSystem))
                {
                    continue;
                }
                string playerName = wstos((const wchar_t*)Players.GetActiveCharacterName(iter.first));
                minijson::object_writer pw = pwc.nested_object(playerName.c_str());
                pw.write("system", mapObscuredSystems.at(playerSystem).nickname.c_str());
                pw.close();
            }
            pwc.close();
            writer.close();

            FILE* file = fopen(filePath.c_str(), "w");
            if (file)
            {
                fprintf(file, "%s", stream.str().c_str());
                fclose(file);
            }
            cloakStateChanged = false;
        }

        for (auto iter = cloakSyncData.begin(); iter != cloakSyncData.end();)
        {
            ++iter->second.iterationCounter;
            if (iter->second.iterationCounter > 1)
            {
                for (uint clientId : iter->second.clientIds)
                {
                    HookClient->Send_FLPACKET_COMMON_ACTIVATEEQUIP(clientId, iter->second.eq);
                }
            }

            if (iter->second.iterationCounter > 2)
            {
                iter = cloakSyncData.erase(iter);
            }
            else
            {
                iter++;
            }
        }
    }*/

    void CloakPlugin::CloakDisruptor(ShipId ship, float disruptRange, int64 disruptTime)
    {
        auto client = ship.GetPlayer().Handle();
        auto pos = ship.GetPositionAndOrientation().Handle();

        SystemId sys = ship.GetSystem().Handle();

        // For all players in system...
        struct PlayerData* pPD = nullptr;
        while (pPD = Players.traverse_active(pPD))
        {
            if (sys != pPD->systemId || pPD->clientId)
            {
                continue;
            }

            ClientId client2 = ClientId(pPD->clientId);
            auto clientShip = client2.GetShip();
            if (clientShip.HasError())
            {
                continue;
            }
            auto clientPos = clientShip.Value().GetPositionAndOrientation().Handle();

            // Is player within the specified range of the sending char.
            if (Vector::Distance(pos.first, clientPos.first) > disruptRange)
            {
                continue;
            }

            // we check if that character has a cloaking device.
            auto cloakInfoIter = clientCloakData.find(client2);
            if (cloakInfoIter == clientCloakData.end())
            {
                continue;
            }

            auto& cloakInfo = cloakInfoIter->second;

            // if it's an admin, do nothing. Doing it this way fixes the ghost bug.
            if (cloakInfo.admin)
            {
                continue;
            }

            auto currTime = TimeUtils::UnixTime();
            // check if the cloak is charging or is already on
            if (cloakInfo.cloakState == CloakState::Charging ||
                (cloakInfo.cloakState == CloakState::On && (cloakInfo.cloakTime + cloakInfo.cloakInfo->activationTime) < currTime))
            {
                SetState(client2, CloakState::Off);
                pub::Audio::PlaySoundEffect(client2.GetValue(), CreateID("cloak_osiris"));
                cloakInfo.disruptedUntilTime = currTime + disruptTime;
                client2.Message(L"Alert: Cloaking device disruption field detected.");
                client2.Message(std::format(L"Cloak rebooting... {} seconds remaining.", (currTime - cloakInfo.disruptedUntilTime) / 1000));
            }
        }
    }

    concurrencpp::result<void> CloakPlugin::UserCmdCloak(ClientId client)
    {
        if (!client.InSpace())
        {
            client.MessageErr(L"Not in space");
            co_return;
        }

        auto infoIter = clientCloakData.find(client);

        if (infoIter == clientCloakData.end())
        {
            client.MessageErr(L"Cloaking device not available");
            co_return;
        }

        auto& info = infoIter->second;

        if (auto currTime = TimeUtils::UnixTime(); info.disruptedUntilTime && info.disruptedUntilTime < currTime)
        {
            client.MessageErr(std::format(L"Cloaking Device Disrupted. Please wait {} seconds", (info.disruptedUntilTime - currTime) / 1000));
            co_return;
        }

        if (IsClientJumping(client))
        {
            client.MessageErr(L"Unable to cloak while Jump Drive is engaged");
            co_return;
        }

        auto ship = client.GetShip().Handle();
        uint type = ship.GetArchetype().Handle()->archType;

        if (!(info.cloakInfo->usableClasses.get() & type))
        {
            client.MessageErr(L"Current cloaking device will not function on this ship type");
            info.cloakState = CloakState::Off;
            SetState(client, CloakState::Off);
            co_return;
        }

        info.admin = false;

        switch (info.cloakState)
        {
            case CloakState::Off: SetState(client, CloakState::Charging); break;
            case CloakState::On:
                {
                    auto currTime = TimeUtils::UnixTime();
                    if ((info.cloakTime + info.cloakInfo->activationTime) < currTime)
                    {
                        SetState(client, CloakState::Off);
                    }
                    else
                    {
                        client.MessageErr(L"Device must fully activate before deactivation.");
                    }
                    break;
                }
            case CloakState::Charging: SetState(client, CloakState::Off); break;
        }

        co_return;
    }
    concurrencpp::result<void> CloakPlugin::UserCmdDisruptor(ClientId client)
    {
        auto cd = clientDisruptorData.find(client);
        if (cd == clientDisruptorData.end())
        {
            client.MessageErr(L"Cloak Disruptor not found.");
            co_return;
        }

        if (auto currTime = TimeUtils::UnixTime(); cd->second.cooldownUntil && cd->second.cooldownUntil < currTime)
        {
            client.MessageErr(std::format(L"Cloak Disruptor recharging. {} seconds left.", (cd->second.cooldownUntil - currTime) / 1000));
            co_return;
        }

        bool foundammo = false;

        auto ship = client.GetShip();
        if (ship.HasError())
        {
            client.MessageErr(L"Ship not in space!");
            co_return;
        }
        CEquipTraverser tr((uint)EquipmentClass::Cargo);
        CECargo* cargo;
        while (cargo = reinterpret_cast<CECargo*>(ship.Value().GetEquipmentManager().Value()->Traverse(tr)))
        {
            if (cargo->archetype->archId != cd->second.disruptorInfo->ammoType)
            {
                continue;
            }

            if (cargo->count < cd->second.disruptorInfo->ammoAmount)
            {
                client.MessageErr(L"Not enough batteries.");
                co_return;
            }

            client.RemoveCargo(cargo->SubObjId, cd->second.disruptorInfo->ammoAmount);
            break;
        }

        ship.Value().IgniteFuse(cd->second.disruptorInfo->effect);

        static const Id disruptSound = Id("cloak_osiris");

        client.PlaySound(disruptSound);

        CloakDisruptor(ship.Value(), cd->second.disruptorInfo->range, cd->second.disruptorInfo->disruptTime);

        cd->second.cooldownUntil = TimeUtils::UnixTime() + cd->second.disruptorInfo->cooldown;
        client.Message(std::format(L"Cloak Disruptor engaged. Cooldown: {} seconds.", cd->second.disruptorInfo->cooldown / 1000));

        co_return;
    }

    concurrencpp::result<void> CloakPlugin::AdminCmdCloak(ClientId client) {
        if (!client.InSpace())
        {
            client.MessageErr(L"Not in space");
            co_return;
        }

        auto infoIter = clientCloakData.find(client);

        if (infoIter == clientCloakData.end())
        {
            client.MessageErr(L"Cloaking device not available");
            co_return;
        }

        auto& info = infoIter->second;

        auto ship = client.GetShip().Handle();
        uint type = ship.GetArchetype().Handle()->archType;

        info.admin = true;

        switch (info.cloakState)
        {
            case CloakState::Off: SetState(client, CloakState::On); break;
            case CloakState::On: SetState(client, CloakState::Off); break;
            case CloakState::Charging: SetState(client, CloakState::Off); break;
        }

        co_return;
    }

    void CloakPlugin::OnClearClientInfo(ClientId client)
    {
        clientCloakData.erase(client);
        clientDisruptorData.erase(client);
    }

    void CloakPlugin::ObscureSystemList(ClientId client)
    {
        if (!config.enableSystemSpoofing)
        {
            return;
        }
        SystemId system = client.GetSystemId().Handle();
        auto systemObscureIter = config.systemObscureMap.find(system);
        if (systemObscureIter != config.systemObscureMap.end())
        {
            Players.SendSystemID(client.GetValue(), systemObscureIter->second.GetValue());
        }
        else
        {
            static const Id fallbackSystemId = Id("fp7_system");
            Players.SendSystemID(client.GetValue(), fallbackSystemId.GetValue());
        }
    }

    void CloakPlugin::SetCloak(ClientId client, bool cloakState)
    {
        XActivateEquip ActivateEq;
        ActivateEq.activate = cloakState;
        ActivateEq.spaceId = client.GetShip().Handle().GetId().Handle();
        ActivateEq.id = clientCloakData[client].cloakSlot;
        Server.ActivateEquip(client.GetValue(), ActivateEq);

        if (cloakState)
        {
            ObscureSystemList(client);
        }
    }

    void CloakPlugin::SetState(ClientId client, CloakState newState)
    {
        auto& cloakInfo = clientCloakData.at(client);
        if (cloakInfo.cloakState == newState)
        {
            return;
        }
        if (cloakInfo.cloakState == CloakState::On || newState == CloakState::On)
        {
            // cloakStateChanged = true; //Recalculate list of cloaked players for external usage
        }

        Id shipId = client.GetShip().Handle().GetId().Handle();

        cloakInfo.cloakState = newState;
        cloakInfo.cloakTime = TimeUtils::UnixTime();
        cloakInfo.fuelUsageOverflow = 0.0f;
        // CLIENT_CLOAK_STRUCT communicationInfo;
        switch (newState)
        {
            case CloakState::Charging:
                {
                    if (cloakInfo.cloakInfo->dropShieldsOnCloak)
                    {
                        shipId.AsShip().DrainShields();
                    }

                    client.Message(L"Preparing to cloak...");
                    break;
                }

            case CloakState::On:
                {
                    if (cloakInfo.cloakInfo->dropShieldsOnCloak)
                    {
                        shipId.AsShip().DrainShields();
                    }

                    client.Message(L" Cloaking device on"); // TODO: clienthook
                    client.Message(L"Cloaking device on");
                    SetCloak(client, true);

                    break;
                }
            case CloakState::Off:
                {
                }
        }
    }

    void CloakPlugin::SendUncloakPacket(ClientId packetReceivingClient, Id uncloakingShipId, ushort cloakSId)
    {
        XActivateEquip eq;
        eq.activate = false;
        eq.spaceId = uncloakingShipId;
        eq.id = cloakSId;

        FLHook::GetPacketInterface()->Send_FLPACKET_COMMON_ACTIVATEEQUIP(packetReceivingClient.GetValue(), eq);
    }

    bool CloakPlugin::IsClientJumping(ClientId client)
    {
        auto hyperjumpPlugin = std::static_pointer_cast<HyperjumpPlugin>(PluginManager::i()->GetPlugin(HyperjumpPlugin::pluginName).lock());
        if (!hyperjumpPlugin)
        {
            return false;
        }

        return hyperjumpPlugin->IsPlayerJumping(client);
    }

    bool CloakPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/cloak.json");

        for (auto& cloak : config.cloakInfoMap)
        {
            uint value = 0;
            for (auto& shipClass : cloak.second.usableClassesString)
            {
                if (shipClass.find("fighter") != std::string::npos)
                {
                    value |= (uint)ObjectType::Fighter;
                }
                if (shipClass.find("freighter") != std::string::npos)
                {
                    value |= (uint)ObjectType::Freighter;
                }
                if (shipClass.find("transport") != std::string::npos)
                {
                    value |= (uint)ObjectType::Transport;
                }
                if (shipClass.find("gunboat") != std::string::npos)
                {
                    value |= (uint)ObjectType::Gunboat;
                }
                if (shipClass.find("cruiser") != std::string::npos)
                {
                    value |= (uint)ObjectType::Cruiser;
                }
                if (shipClass.find("capital") != std::string::npos)
                {
                    value |= (uint)ObjectType::Capital;
                }
            }
            cloak.second.usableClasses.set(value);
        }
        AddTimer([this] { ProcessFuel(); }, 1000);

        return true;
    }

    void CloakPlugin::OnCreateShipPacketAfter(ClientId client, FLPACKET_CREATESHIP& ship)
    {
        auto cloakIter = clientCloakData.find(client);
        if (cloakIter == clientCloakData.end() || cloakIter->second.cloakState != CloakState::On)
        {
            return;
        }

        AddOneShotTimer([this, client, ship, cloakIter] { SendUncloakPacket(client, ship.spaceId, cloakIter->second.cloakSlot); }, 100);
    }

    void CloakPlugin::OnSpRequestUseItem(ClientId client, const SSPUseItem& item)
    {
        const static Id BATTERY_ARCH_ID = Id("ge_s_battery_01");

        auto cloakInfo = clientCloakData.find(client);
        if (cloakInfo == clientCloakData.end() || cloakInfo->second.cloakState != CloakState::On || cloakInfo->second.cloakInfo->dropShieldsOnCloak == false ||
            cloakInfo->second.admin)
        {
            return;
        }

        auto cship = client.GetShip().Unwrap();
        if (!cship)
        {
            return;
        }

        auto cequip = cship.GetEquipmentManager().Unwrap()->FindByID(item.itemId);
        if (!cequip || cequip->archetype->archId != BATTERY_ARCH_ID)
        {
            return;
        }

        const static Id errorSound = Id("ui_select_reject");
        client.PlaySound(errorSound);
        returnCode = ReturnCode::SkipAll;
    }

    void CloakPlugin::EnableFuelConsumption(ClientId client)
    {
        auto clientInfo = clientCloakData.find(client);
        if (clientInfo != clientCloakData.end())
        {
            clientInfo->second.disableJumpFuelConsumption = false;
        }
    }

    void CloakPlugin::OnDockCallAfter(const ShipId& shipId, const ObjectId& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response)
    {
        if (response != DOCK_HOST_RESPONSE::Dock && response != DOCK_HOST_RESPONSE::ProceedDock && dockPortIndex != -1)
        {
            return;
        }

        auto type = (ObjectType)spaceId.GetArchetype().Value()->archType;
        if (!(type == ObjectType::JumpGate || type == ObjectType::JumpHole))
        {
            return;
        }

        auto player = shipId.GetPlayer();
        if (player.HasError())
        {
            return;
        }

        auto clientInfo = clientCloakData.find(player.Value());
        if (clientInfo == clientCloakData.end() || clientInfo->second.admin || clientInfo->second.cloakState == CloakState::Off)
        {
            return;
        }

        // disable fuel consumption and disable proximity detection
        clientInfo->second.disableJumpFuelConsumption = true;
        ClientId client = player.Value();

        AddOneShotTimer([this, client] { EnableFuelConsumption(client); }, 30000);
    }

    void CloakPlugin::OnShipExplosionHit(Ship* ship, ExplosionDamageEvent* explosion, DamageList* dmgList)
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

        auto cloakData = clientCloakData.find(client);
        if (cloakData == clientCloakData.end())
        {
            return;
        }

        if (!cloakData->second.admin && cloakData->second.cloakState == CloakState::Charging)
        {
            SetState(client, CloakState::Off);
        }
    }

    void CloakPlugin::InitCloak(ClientId client, float range)
    {
        // TODO: Send a message to clienthook
    }

    void CloakPlugin::OnPlayerLaunch(ClientId client, const ShipId& ship)
    {

        CEquipTraverser disruptorTraverser((uint)EquipmentClass::CM);

        CEquip* disruptorEq;
        while (disruptorEq = ship.GetEquipmentManager().Handle()->Traverse(disruptorTraverser))
        {
            auto disrIter = config.disruptorInfoMap.find(disruptorEq->archetype->archId);
            if (disrIter == config.disruptorInfoMap.end())
            {
                continue;
            }
            PlayerDisruptorData disrData;
            disrData.disruptorInfo = &disrIter->second;

            clientDisruptorData[client] = disrData;
            break;
        }

        // Legacy code for the cloaks, needs to be rewritten at some point. - Alley

        CEquip* cloakEq = ship.GetEquipmentManager().Handle()->FindFirst((uint)EquipmentClass::CloakingDevice);
        if (!cloakEq)
        {
            return;
        }
        auto cloakIter = config.cloakInfoMap.find(cloakEq->archetype->archId);
        if (cloakIter == config.cloakInfoMap.end())
        {
            return;
        }

        auto& cloakInfo = clientCloakData[client];
        cloakInfo.cloakSlot = cloakEq->GetID();
        cloakInfo.cloakInfo = &cloakIter->second;
        SetState(client, CloakState::Off);
        if (cloakInfo.cloakInfo->detectionRange && !cloakInfo.admin)
        {
            InitCloak(client, cloakInfo.cloakInfo->detectionRange);
        }
    }

    CloakPlugin::CloakPlugin(const PluginInfo& info) : Plugin(info) {}

    bool CloakPlugin::IsClientCloaked(ClientId client)
    {
        auto cloakIter = clientCloakData.find(client);
        if (cloakIter == clientCloakData.end() || cloakIter->second.cloakState == CloakState::Off)
        {
            return false;
        }

        return true;
    }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Cloak",
	    .shortName = std::wstring(CloakPlugin::pluginName),
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(CloakPlugin);
