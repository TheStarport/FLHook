#include "PCH.hpp"

#include "API/InternalApi.hpp"
#include "Core/IEngineHook.hpp"

std::wstring SetSizeToSmall(const std::wstring& dataFormat) { return dataFormat.substr(0, 8) + L"90"; }

void IEngineHook::SendDeathMessage(const std::wstring& msg, uint systemId, ClientId clientVictim, ClientId clientKiller)
{
    CallPlugins(&Plugin::OnSendDeathMessage, clientKiller, clientVictim, systemId, std::wstring_view(msg));

    // encode xml std::wstring(default and small) non-system
    const auto xmlMsg =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHookConfig::i()->chatConfig.msgStyle.deathMsgStyle, StringUtils::XmlText(msg));

    char xmlBuf[0xFFFF];
    uint ret;
    if (InternalApi::FMsgEncodeXml(xmlMsg, xmlBuf, sizeof xmlBuf, ret).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSmall = std::format(
        L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", SetSizeToSmall(FLHookConfig::i()->chatConfig.msgStyle.deathMsgStyle), StringUtils::XmlText(msg));

    char bufSmall[0xFFFF];
    uint retSmall;
    if (InternalApi::FMsgEncodeXml(xmlMsgSmall, bufSmall, sizeof bufSmall, retSmall).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSystem =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHookConfig::i()->chatConfig.msgStyle.deathMsgStyleSys, StringUtils::XmlText(msg));

    char bufSys[0xFFFF];
    uint retSys;
    if (InternalApi::FMsgEncodeXml(xmlMsgSystem, bufSys, sizeof bufSys, retSys).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSmallSys =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}", FLHookConfig::i()->chatConfig.msgStyle.deathMsgStyleSys, StringUtils::XmlText(msg));

    char BufSmallSys[0xFFFF];
    uint retSmallSys;
    if (InternalApi::FMsgEncodeXml(xmlMsgSmallSys, BufSmallSys, sizeof BufSmallSys, retSmallSys).Raw().has_error())
    {
        return;
    }

    for (auto client : FLHook::Clients())
    {
        auto system = client.id.GetSystemId().Unwrap().GetValue();

        const auto& data = client.id.GetData();

        char* sendXmlBuf;
        int sendXmlRet;
        char* sendXmlBufSys;
        int sendXmlSysRet;
        if (FLHookConfig::i()->userCommands.userCmdSetDieMsgSize && data.dieMsgSize == ChatSize::Small)
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
            if (systemId == system)
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
            }
            else
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBuf, sendXmlRet);
            }
            continue;
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
        ClientId client = ClientId(cship->GetOwnerPlayer());

        CallPlugins(&Plugin::OnShipDestroyed, client, dmgList, cship);

        if (!client)
        {
            return;
        }

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

        auto& data = client.GetData();
        auto systemId = client.GetSystemId().Unwrap();
        std::wstring_view name = systemId.GetName().Unwrap();

        if (!magic_enum::enum_integer(dmg.get_cause()))
        {
            dmg = data.dmgLast;
        }

        DamageCause cause = dmg.get_cause();
        const auto clientKiller = ShipId(dmg.get_inflictor_id()).GetPlayer();

        std::wstring_view victimName = client.GetCharacterName().Unwrap();
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
                std::wstring_view killer = client.GetCharacterName().Unwrap();

                deathMessage = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.deathMsgTextPlayerKill, L"%victim", victimName);
                deathMessage = StringUtils::ReplaceStr(deathMessage, L"%killer", killer);
            }

            deathMessage = StringUtils::ReplaceStr(deathMessage, L"%type", killType);
            if (FLHookConfig::i()->chatConfig.dieMsg && deathMessage.length())
            {
                SendDeathMessage(deathMessage, systemId.GetValue(), client, clientKiller.value());
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
                SendDeathMessage(deathMessage, systemId.GetValue(), client, ClientId());
            }
        }
        else if (cause == DamageCause::Suicide)
        {
            if (std::wstring deathMessage = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.deathMsgTextSuicide, L"%victim", victimName);
                FLHookConfig::i()->chatConfig.dieMsg && !deathMessage.empty())
            {
                SendDeathMessage(deathMessage, systemId.GetValue(), client, ClientId());
            }
        }
        else if (cause == DamageCause::Admin)
        {
            if (std::wstring deathMessage = StringUtils::ReplaceStr(FLHookConfig::i()->chatConfig.msgStyle.deathMsgTextAdminKill, L"%victim", victimName);
                FLHookConfig::i()->chatConfig.dieMsg && deathMessage.length())
            {
                SendDeathMessage(deathMessage, systemId.GetValue(), client, ClientId());
            }
        }
        else
        {
            std::wstring deathMessage = std::format(L"Death: {} has died", victimName);
            if (FLHookConfig::i()->chatConfig.dieMsg && deathMessage.length())
            {
                SendDeathMessage(deathMessage, systemId.GetValue(), client, ClientId());
            }
        }

        data.shipOld = data.ship;
        data.ship = ShipId();
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
