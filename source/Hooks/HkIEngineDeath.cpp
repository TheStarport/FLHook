#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "API/InternalApi.hpp"
#include "Core/IEngineHook.hpp"

std::wstring SetSizeToSmall(const std::wstring& dataFormat) { return dataFormat.substr(0, 8) + L"90"; }

// DmgList will be garbage if isKill is not true
void __fastcall IEngineHook::ShipDestroy(Ship* ship, DamageList* dmgList, bool isKill, Id killerId)
{
    if (isKill)
    {
        auto killer = killerId.AsShip();
        CallPlugins(&Plugin::OnShipDestroy, ship, dmgList, killer);
    }

    ResourceManager::playerShips.erase(ship->cship()->id);
    FLHook::GetResourceManager()->OnShipDestroyed(ship); // Remove if spawned ship

    using IShipDestroyType = void(__thiscall*)(Ship*, bool, uint);

    // Only proceed if a player was killed
    ClientId victimClientId;
    if (!isKill || !(victimClientId = ClientId(ship->cship()->GetOwnerPlayer())).IsValidClientId())
    {
        static_cast<IShipDestroyType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::ObjectDestroyed)))(ship, isKill, killerId.GetValue());
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

    static_cast<IShipDestroyType>(iShipVTable.GetOriginal(static_cast<ushort>(IShipInspectVTable::ObjectDestroyed)))(ship, isKill, killerId.GetValue());
}

void __fastcall IEngineHook::LootDestroy(Loot* loot, void* edx, bool isKill, uint killerId)
{
    CallPlugins(&Plugin::OnLootDestroy, loot, isKill, killerId);

    using ILootDestroyType = void(__thiscall*)(Loot*, bool, uint);
    static_cast<ILootDestroyType>(iLootVTable.GetOriginal(static_cast<ushort>(ILootInspectVTable::ObjectDestroyed)))(loot, isKill, killerId);
}

void __fastcall IEngineHook::SolarDestroy(Solar* solar, void* edx, bool isKill, uint killerId)
{
    CallPlugins(&Plugin::OnSolarDestroy, solar, isKill, killerId);

    FLHook::GetResourceManager()->OnSolarDestroyed(solar);

    using ISolarDestroyType = void(__thiscall*)(Solar*, bool, uint);
    static_cast<ISolarDestroyType>(iSolarVTable.GetOriginal(static_cast<ushort>(ISolarInspectVTable::ObjectDestroyed)))(solar, isKill, killerId);
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
