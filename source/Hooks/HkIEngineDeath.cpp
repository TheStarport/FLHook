#include "PCH.hpp"

#include "API/API.hpp"
#include "Core/IEngineHook.hpp"
#include "Global.hpp"

std::wstring SetSizeToSmall(const std::wstring& dataFormat) { return dataFormat.substr(0, 8) + L"90"; }

void IEngineHook::SendDeathMessage(const std::wstring& msg, uint systemId, ClientId clientVictim, ClientId clientKiller)
{
    CallPlugins(&Plugin::OnSendDeathMessage, clientKiller, clientVictim, systemId, std::wstring_view(msg));

    // encode xml std::wstring(default and small) non-system
    const auto xmlMsg =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHookConfig::i()->chatConfig.msgStyle.deathMsgStyle, StringUtils::XmlText(msg));

    char xmlBuf[0xFFFF];
    uint ret;
    if (Hk::Chat::FMsgEncodeXml(xmlMsg, xmlBuf, sizeof xmlBuf, ret).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSmall = std::format(
        L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", SetSizeToSmall(FLHookConfig::i()->chatConfig.msgStyle.deathMsgStyle), StringUtils::XmlText(msg));

    char bufSmall[0xFFFF];
    uint retSmall;
    if (Hk::Chat::FMsgEncodeXml(xmlMsgSmall, bufSmall, sizeof bufSmall, retSmall).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSystem =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHookConfig::i()->chatConfig.msgStyle.deathMsgStyleSys, StringUtils::XmlText(msg));

    char bufSys[0xFFFF];
    uint retSys;
    if (Hk::Chat::FMsgEncodeXml(xmlMsgSystem, bufSys, sizeof bufSys, retSys).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSmallSys =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}", FLHookConfig::i()->chatConfig.msgStyle.deathMsgStyleSys, StringUtils::XmlText(msg));

    char BufSmallSys[0xFFFF];
    uint retSmallSys;
    if (Hk::Chat::FMsgEncodeXml(xmlMsgSmallSys, BufSmallSys, sizeof BufSmallSys, retSmallSys).Raw().has_error())
    {
        return;
    }

    // send for all players
    PlayerData* playerData = nullptr;
    while ((playerData = Players.traverse_active(playerData)))
    {
        const auto client = playerData->onlineId;
        uint clientSystemId = 0;
        pub::Player::GetSystem(client, clientSystemId);

        char* sendXmlBuf;
        int sendXmlRet;
        char* sendXmlBufSys;
        int sendXmlSysRet;
        if (FLHookConfig::i()->userCommands.userCmdSetDieMsgSize && ClientInfo::At(client).dieMsgSize == CS_SMALL)
        {
            sendXmlBuf = bufSmall;
            sendXmlRet = retSmall;
            sendXmlBufSys = BufSmallSys;
            sendXmlSysRet = retSmallSys;
        }
        else
        {
            sendXmlBuf = xmlBuf;
            sendXmlRet = ret;
            sendXmlBufSys = bufSys;
            sendXmlSysRet = retSys;
        }

        if (!FLHookConfig::i()->userCommands.userCmdSetDieMsg)
        {
            // /set diemsg disabled, thus send to all
            if (systemId == clientSystemId)
            {
                Hk::Chat::FMsgSendChat(client, sendXmlBufSys, sendXmlSysRet);
            }
            else
            {
                Hk::Chat::FMsgSendChat(client, sendXmlBuf, sendXmlRet);
            }
            continue;
        }

        if (ClientInfo::At(client).dieMsg == DIEMSG_NONE)
        {
            continue;
        }
        if (ClientInfo::At(client).dieMsg == DIEMSG_SYSTEM && systemId == clientSystemId)
        {
            Hk::Chat::FMsgSendChat(client, sendXmlBufSys, sendXmlSysRet);
        }
        else if (ClientInfo::At(client).dieMsg == DIEMSG_SELF && (client == clientVictim || client == clientKiller))
        {
            Hk::Chat::FMsgSendChat(client, sendXmlBufSys, sendXmlSysRet);
        }
        else if (ClientInfo::At(client).dieMsg == All)
        {
            if (systemId == clientSystemId)
            {
                Hk::Chat::FMsgSendChat(client, sendXmlBufSys, sendXmlSysRet);
            }
            else
            {
                Hk::Chat::FMsgSendChat(client, sendXmlBuf, sendXmlRet);
            }
        }
    }

    const std::wstring formattedMsg = StringUtils::stows(BufSmallSys);
    CallPlugins(&Plugin::OnSendDeathMessageAfter, clientKiller, clientVictim, systemId, std::wstring_view(formattedMsg));
}

void __stdcall IEngineHook::ShipDestroyed(DamageList* dmgList, DWORD* ecx, uint kill)
{
    if (!kill)
    {
        return;
    }

    TryHook
    {
        auto cship = (CShip*)ecx[4];
        ClientId client = cship->GetOwnerPlayer();

        CallPlugins(&Plugin::OnShipDestroyed, client, dmgList, cship);

        if (client)
        {
            // a player was killed
            DamageList dmg;
            try
            {
                dmg = *dmgList;
            }
            catch (...)
            {
                return;
            }

            uint systemId;
            pub::Player::GetSystem(client, systemId);
            wchar_t systemName[64];
            swprintf_s(systemName, L"%u", systemId);

            if (!magic_enum::enum_integer(dmg.get_cause()))
            {
                dmg = ClientInfo::At(client).dmgLast;
            }

            DamageCause cause = dmg.get_cause();
            const auto clientKiller = Hk::Client::GetClientIdByShip(dmg.get_inflictor_id()).Raw();

            std::wstring victimName = client.GetCharacterName().Handle();
            if (clientKiller.has_value())
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
                    default: killType = L"Somehow";
                }

                std::wstring deathMessage;
                if (client == clientKiller.value() || cause == DamageCause::Suicide)
                {
                    deathMessage = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.deathMsgTextSelfKill, L"%victim", victimName);
                }
                else if (cause == DamageCause::Admin)
                {
                    deathMessage = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.deathMsgTextAdminKill, L"%victim", victimName);
                }
                else
                {
                    std::wstring killer = client.GetCharacterName().Handle();

                    deathMessage = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.deathMsgTextPlayerKill, L"%victim", victimName);
                    deathMessage = StringUtils::ReplaceStr(deathMessage, L"%killer", killer);
                }

                deathMessage = StringUtils::ReplaceStr(deathMessage, L"%type", killType);
                if (FLHookConfig::i()->chatConfig.dieMsg && deathMessage.length())
                {
                    SendDeathMessage(deathMessage, systemId, client, clientKiller.value());
                }
            }
            else if (dmg.get_inflictor_id())
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

                std::wstring deathMessage = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.deathMsgTextNPC, L"%victim", victimName);
                deathMessage = StringUtils::ReplaceStr(deathMessage, L"%type", killType);

                if (FLHookConfig::i()->chatConfig.dieMsg && deathMessage.length())
                {
                    SendDeathMessage(deathMessage, systemId, client, 0);
                }
            }
            else if (cause == DamageCause::Suicide)
            {
                if (std::wstring deathMessage = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.deathMsgTextSuicide, L"%victim", victimName);
                    FLHookConfig::i()->chatConfig.dieMsg && !deathMessage.empty())
                {
                    SendDeathMessage(deathMessage, systemId, client, 0);
                }
            }
            else if (cause == DamageCause::Admin)
            {
                if (std::wstring deathMessage = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.deathMsgTextAdminKill, L"%victim", victimName);
                    FLHookConfig::i()->chatConfig.dieMsg && deathMessage.length())
                {
                    SendDeathMessage(deathMessage, systemId, client, 0);
                }
            }
            else
            {
                std::wstring deathMessage = L"Death: " + victimName + L" has died";
                if (FLHookConfig::i()->chatConfig.dieMsg && deathMessage.length())
                {
                    SendDeathMessage(deathMessage, systemId, client, 0);
                }
            }
        }

        ClientInfo::At(client).shipOld = ClientInfo::At(client).ship;
        ClientInfo::At(client).ship = 0;
    }
    CatchHook({})
}

__declspec(naked) void IEngineHook::NakedShipDestroyed()
{
    __asm {
		mov eax, [esp+0Ch] ; +4
		mov edx, [esp+4]
		push ecx
		push edx
		push ecx
		push eax
		call IEngineHook::ShipDestroyed
		pop ecx
		mov eax, [IEngineHook::oldShipDestroyed]
		jmp eax
    }
}

void IEngineHook::BaseDestroyed(uint objectId, ClientId clientBy)
{
    CallPlugins(&Plugin::OnBaseDestroyed, clientBy, objectId);

    uint baseId;
    pub::SpaceObj::GetDockingTarget(objectId, baseId);
    Universe::IBase* base = Universe::get_base(baseId);

    auto baseName = "";
    if (base)
    {
        __asm {
			pushad
			mov ecx, [base]
			mov eax, [base]
			mov eax, [eax]
			call [eax+4]
			mov [baseName], eax
			popad
        }
    }
}
