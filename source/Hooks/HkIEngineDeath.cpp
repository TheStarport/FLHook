#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "API/InternalApi.hpp"
#include "API/Utils/Random.hpp"
#include "Core/IEngineHook.hpp"

std::wstring SetSizeToSmall(const std::wstring& dataFormat) { return dataFormat.substr(0, 8) + L"90"; }

// DmgList will be garbage if isKill is not true
void __fastcall IEngineHook::ShipDestroy(Ship* ship, DamageList* dmgList, DestroyType destroyType, Id killerId)
{
    if (destroyType == DestroyType::Fuse)
    {
        auto killer = killerId.AsShip();
        CallPlugins(&Plugin::OnShipDestroy, ship, dmgList, killer);
    }
    else
    {
        CallPlugins(&Plugin::OnShipDespawn, ship);
    }

    if (ship->is_player())
    {
        ResourceManager::playerShips.erase(ship->cship()->id.GetValue());
    }
    else
    {
        ResourceManager::npcToLastAttackingPlayerMap.erase(ship->get_id());
    }

    FLHook::GetResourceManager()->OnShipDestroyed(ship); // Remove if spawned ship

    using IShipDestroyType = void(__thiscall*)(Ship*, DestroyType, uint);

    // Only proceed if a player was killed
    ClientId victimClientId;
    if (destroyType == DestroyType::Vanish || !(victimClientId = ClientId(ship->cship()->GetOwnerPlayer())).IsValidClientId())
    {
        static_cast<IShipDestroyType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::ObjectDestroyed)))(
            ship, destroyType, killerId.GetValue());
        return;
    }

    ClientId killerClientId;
    for (const ClientData& data : FLHook::Clients())
    {
        if (data.shipId == killerId)
        {
            killerClientId = data.id;
            break;
        }
    }

    auto& victimData = victimClientId.GetData();
    const auto& msgConfig = FLHook::GetConfig()->chatConfig.msgStyle;
    std::wstring deathMessage;

    std::wstring_view victimName = victimClientId.GetCharacterName().Handle();
    // If killed by another player
    if (auto cause = dmgList->damageCause; killerClientId)
    {
        std::wstring killType;
        switch (cause)
        {
            case DamageCause::Collision: killType = L"Collision"; break;
            case DamageCause::Gun: killType = L"Gun"; break;
            case DamageCause::MissileTorpedo: killType = L"Missile/Torpedo"; break;
            case DamageCause::CruiseDisrupter:
            case DamageCause::DummyDisrupter:
            case DamageCause::UnkDisrupter: killType = L"Cruise Disruptor"; break;
            case DamageCause::Mine: killType = L"Mine"; break;
            case DamageCause::Suicide: killType = L"Suicide"; break;
            default: killType = L"Somehow"; ;
        }

        if (victimClientId == killerClientId || cause == DamageCause::Suicide)
        {
            deathMessage = std::vformat(msgConfig.deathMsgTextSelfKill, std::make_wformat_args(victimName));
        }
        else if (cause == DamageCause::Admin)
        {
            deathMessage = std::vformat(msgConfig.deathMsgTextAdminKill, std::make_wformat_args(victimName));
        }
        else
        {
            std::wstring_view view = killerClientId.GetCharacterName().Unwrap();
            deathMessage = std::vformat(msgConfig.deathMsgTextPlayerKill, std::make_wformat_args(victimName, killType, view));
        }
    }
    else if (dmgList->inflictorId)
    {
        std::wstring killType;
        switch (cause)
        {
            case DamageCause::Collision: killType = L"Collision"; break;
            case DamageCause::Gun: break;
            case DamageCause::MissileTorpedo: killType = L"Missile/Torpedo"; break;
            case DamageCause::CruiseDisrupter:
            case DamageCause::DummyDisrupter:
            case DamageCause::UnkDisrupter: killType = L"Cruise Disruptor"; break;
            case DamageCause::Mine: killType = L"Mine"; break;
            default: killType = L"Gun";
        }

        deathMessage = std::vformat(msgConfig.deathMsgTextNPC, std::make_wformat_args(victimName, killType));
    }
    else if (cause == DamageCause::Suicide)
    {
        deathMessage = std::vformat(msgConfig.deathMsgTextSuicide, std::make_wformat_args(victimName));
    }
    else if (cause == DamageCause::Admin)
    {
        deathMessage = std::vformat(msgConfig.deathMsgTextAdminKill, std::make_wformat_args(victimName));
    }
    // Unknown death cause, they died I guess
    else
    {
        deathMessage = std::format(L"Death: {} has died.", victimName);
    }

    SendDeathMessage(deathMessage, SystemId(victimData.playerData->systemId), victimClientId, killerClientId);

    victimData.ship = {};
    victimData.shipId = {};

    static_cast<IShipDestroyType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::ObjectDestroyed)))(ship, destroyType, killerId.GetValue());
}

void __fastcall IEngineHook::LootDestroy(Loot* loot, void* edx, DestroyType destroyType, Id killerId)
{
    CallPlugins(&Plugin::OnLootDestroy, loot, destroyType, killerId.AsShip());

    using ILootDestroyType = void(__thiscall*)(Loot*, DestroyType, uint);
    static_cast<ILootDestroyType>(iLootVTable.GetOriginal(static_cast<ushort>(ILootInspectVTable::ObjectDestroyed)))(loot, destroyType, killerId.GetValue());
}

void __fastcall IEngineHook::SolarDestroy(Solar* solar, void* edx, DestroyType destroyType, Id killerId)
{
    CallPlugins(&Plugin::OnSolarDestroy, solar, destroyType, killerId.AsShip());

    FLHook::GetResourceManager()->OnSolarDestroyed(solar);

    using ISolarDestroyType = void(__thiscall*)(Solar*, DestroyType, uint);
    static_cast<ISolarDestroyType>(iSolarVTable.GetOriginal(static_cast<ushort>(ISolarInspectVTable::ObjectDestroyed)))(solar, destroyType, killerId.GetValue());
}

void __fastcall IEngineHook::MineDestroy(Mine* mine, void* edx, DestroyType destroyType, Id killerId)
{
    CallPlugins(&Plugin::OnMineDestroy, mine, destroyType, killerId.AsShip());

    using IMineDestroyType = void(__thiscall*)(Mine*, DestroyType, uint);
    static_cast<IMineDestroyType>(iMineVTable.GetOriginal(static_cast<ushort>(IMineInspectVTable::ObjectDestroyed)))(mine, destroyType, killerId.GetValue());
}

void __fastcall IEngineHook::GuidedDestroy(Guided* guided, void* edx, DestroyType destroyType, Id killerId)
{
    CallPlugins(&Plugin::OnGuidedDestroy, guided, destroyType, killerId.AsShip());

    using IGuidedDestroyType = void(__thiscall*)(Guided*, DestroyType, uint);
    static_cast<IGuidedDestroyType>(iGuidedVTable.GetOriginal(static_cast<ushort>(IGuidedInspectVTable::ObjectDestroyed)))(
        guided, destroyType, killerId.GetValue());
}

void IEngineHook::SendDeathMessage(const std::wstring& msg, SystemId systemId, ClientId clientVictim, ClientId clientKiller)
{
    CallPlugins(&Plugin::OnSendDeathMessage, clientKiller, clientVictim, systemId, std::wstring_view(msg));

    // encode xml std::wstring(default and small) non-system
    const auto xmlMsg =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHook::GetConfig()->chatConfig.msgStyle.deathMsgStyle, StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> xmlBuf;
    uint ret;
    if (InternalApi::FMsgEncodeXml(xmlMsg, xmlBuf.data(), xmlBuf.size(), ret).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSmall = std::format(
        L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", SetSizeToSmall(FLHook::GetConfig()->chatConfig.msgStyle.deathMsgStyle), StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> bufSmall;
    uint retSmall;
    if (InternalApi::FMsgEncodeXml(xmlMsgSmall, bufSmall.data(), bufSmall.size(), retSmall).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSystem =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHook::GetConfig()->chatConfig.msgStyle.deathMsgStyleSys, StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> bufSys;
    uint retSys;
    if (InternalApi::FMsgEncodeXml(xmlMsgSystem, bufSys.data(), bufSys.size(), retSys).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSmallSys =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHook::GetConfig()->chatConfig.msgStyle.deathMsgStyleSys, StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> BufSmallSys;
    uint retSmallSys;
    if (InternalApi::FMsgEncodeXml(xmlMsgSmallSys, BufSmallSys.data(), BufSmallSys.size(), retSmallSys).Raw().has_error())
    {
        return;
    }

    for (auto& client : FLHook::Clients())
    {
        const auto system = client.id.GetSystemId().Unwrap();

        const auto& data = client.id.GetData();

        char* sendXmlBuf;
        uint sendXmlRet;
        char* sendXmlBufSys;
        uint sendXmlSysRet;
        if (data.dieMsgSize == ChatSize::Small)
        {
            sendXmlBuf = bufSmall.data();
            sendXmlRet = retSmall;
            sendXmlBufSys = BufSmallSys.data();
            sendXmlSysRet = retSmallSys;
        }
        else
        {
            sendXmlBuf = xmlBuf.data();
            sendXmlRet = ret;
            sendXmlBufSys = bufSys.data();
            sendXmlSysRet = retSys;
        }

        if (data.dieMsg == DieMsgType::None)
        {
            continue;
        }

        if (data.dieMsg == DieMsgType::System && systemId == system)
        {
            InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
        }
        else if (data.dieMsg == DieMsgType::Self && (client.id == clientVictim || client.id == clientKiller))
        {
            InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
        }
        else if (data.dieMsg == DieMsgType::All)
        {
            if (systemId == system)
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
            }
            else
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBuf, sendXmlRet);
            }
        }
    }

    std::ranges::fill(xmlBuf, 0);
    std::ranges::fill(bufSmall, 0);
    std::ranges::fill(bufSys, 0);
    std::ranges::fill(BufSmallSys, 0);

    const std::wstring formattedMsg = StringUtils::stows(BufSmallSys.data());
    CallPlugins(&Plugin::OnSendDeathMessageAfter, clientKiller, clientVictim, systemId, std::wstring_view(formattedMsg));
}

void IEngineHook::OnPlayerLaunch(ClientId client)
{
    uint affiliation;
    Reputation::Vibe::GetAffiliation(client.GetData().playerData->reputation, affiliation, false);

    if (!affiliation)
    {
        affiliation = SendCommData::Callsign::FreelancerAffiliation;
    }

    const auto fd = sendCommData.factions.find(affiliation);
    if (fd == sendCommData.factions.end())
    {
        return;
    }

    if (auto& [lastFactionAff, factionLine, formationLine, number1, number2] = sendCommData.callsigns[client.GetValue()]; lastFactionAff != affiliation)
    {
        lastFactionAff = affiliation;
        factionLine = fd->second.msgId;
        formationLine = Random::Item(fd->second.formationHashes);
        const int randNum1 = Random::Uniform(0u, sendCommData.numberHashes.size() - 1) + 1; // +1 because map starts at 1
        number1 = sendCommData.numberHashes.at(randNum1).first;
        const int randNum2 = Random::Uniform(0u, sendCommData.numberHashes.size() - 1) + 1;
        number2 = sendCommData.numberHashes.at(randNum2).second;
    }
}

void IEngineHook::OnCharacterSelectAfter(ClientId client) { sendCommData.callsigns.erase(client.GetValue()); }
