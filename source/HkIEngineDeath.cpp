#include "Global.hpp"

std::wstring SetSizeToSmall(const std::wstring& DataFormat)
{
	return DataFormat.substr(0, 8) + L"90";
}

/**************************************************************************************************************
Send "Death: ..." chat-message
**************************************************************************************************************/

void SendDeathMessage(const std::wstring& msg, uint systemId, ClientId clientVictim, ClientId clientKiller)
{
	CallPluginsBefore(HookedCall::IEngine__SendDeathMessage, msg, systemId, clientVictim, clientKiller);

	// encode xml std::string(default and small)
	// non-sys
	std::wstring xmlMsg = L"<TRA data=\"" + FLHookConfig::i()->messages.msgStyle.deathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
	xmlMsg += XMLText(msg);
	xmlMsg += L"</TEXT>";

	char xmlBuf[0xFFFF];
	uint ret;
	if (Hk::Message::FMsgEncodeXML(xmlMsg, xmlBuf, sizeof(xmlBuf), ret).has_error())
		return;

	std::wstring styleSmall = SetSizeToSmall(FLHookConfig::i()->messages.msgStyle.deathMsgStyle);
	std::wstring xmlMsgSmall = std::wstring(L"<TRA data=\"") + styleSmall + L"\" mask=\"-1\"/> <TEXT>";
	xmlMsgSmall += XMLText(msg);
	xmlMsgSmall += L"</TEXT>";
	char bufSmall[0xFFFF];
	uint retSmall;
	if (Hk::Message::FMsgEncodeXML(xmlMsgSmall, bufSmall, sizeof(bufSmall), retSmall).has_error())
		return;

	// sys
	std::wstring xmlMsgSys = L"<TRA data=\"" + FLHookConfig::i()->messages.msgStyle.deathMsgStyleSys + L"\" mask=\"-1\"/> <TEXT>";
	xmlMsgSys += XMLText(msg);
	xmlMsgSys += L"</TEXT>";
	char bufSys[0xFFFF];
	uint retSys;
	if (Hk::Message::FMsgEncodeXML(xmlMsgSys, bufSys, sizeof(bufSys), retSys).has_error())
		return;

	std::wstring styleSmallSys = SetSizeToSmall(FLHookConfig::i()->messages.msgStyle.deathMsgStyleSys);
	std::wstring xmlMsgSmallSys = L"<TRA data=\"" + styleSmallSys + L"\" mask=\"-1\"/> <TEXT>";
	xmlMsgSmallSys += XMLText(msg);
	xmlMsgSmallSys += L"</TEXT>";
	char BufSmallSys[0xFFFF];
	uint retSmallSys;
	if (Hk::Message::FMsgEncodeXML(xmlMsgSmallSys, BufSmallSys, sizeof(BufSmallSys), retSmallSys).has_error())
		return;

	// send
	// for all players
	PlayerData* playerData = nullptr;
	while ((playerData = Players.traverse_active(playerData)))
	{
		const auto client = playerData->iOnlineId;
		uint clientSystemId = 0;
		pub::Player::GetSystem(client, clientSystemId);

		char* sendXmlBuf;
		int sendXmlRet;
		char* sendXmlBufSys;
		int sendXmlSysRet;
		if (FLHookConfig::i()->userCommands.userCmdSetDieMsgSize && (ClientInfo[client].dieMsgSize == CS_SMALL))
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
				Hk::Message::FMsgSendChat(client, sendXmlBufSys, sendXmlSysRet);
			else
				Hk::Message::FMsgSendChat(client, sendXmlBuf, sendXmlRet);
			continue;
		}

		if (ClientInfo[client].dieMsg == DIEMSG_NONE)
			continue;
		if ((ClientInfo[client].dieMsg == DIEMSG_SYSTEM) && (systemId == clientSystemId))
			Hk::Message::FMsgSendChat(client, sendXmlBufSys, sendXmlSysRet);
		else if (
			(ClientInfo[client].dieMsg == DIEMSG_SELF) &&
			((client == clientVictim) || (client == clientKiller)))
			Hk::Message::FMsgSendChat(client, sendXmlBufSys, sendXmlSysRet);
		else if (ClientInfo[client].dieMsg == DiemsgAll)
		{
			if (systemId == clientSystemId)
				Hk::Message::FMsgSendChat(client, sendXmlBufSys, sendXmlSysRet);
			else
				Hk::Message::FMsgSendChat(client, sendXmlBuf, sendXmlRet);
		}
	}
}

/**************************************************************************************************************
Called when ship was destroyed
**************************************************************************************************************/

void __stdcall ShipDestroyed(DamageList* dmgList, DWORD* ecx, uint kill)
{
	CallPluginsBefore(HookedCall::IEngine__ShipDestroyed, dmgList, ecx, kill);

	TRY_HOOK
		{
			if (kill == 1)
			{
				auto cship = (CShip*)ecx[4];
				ClientId client = cship->GetOwnerPlayer();

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
						dmg = ClientInfo[client].dmgLast;

					DamageCause cause = dmg.get_cause();
					const auto clientKiller = Hk::Client::GetClientIdByShip(dmg.get_inflictor_id());

					std::wstring victimName = ToWChar(Players.GetActiveCharacterName(client));
					if (clientKiller.has_value())
					{
						std::wstring killType;
						switch (cause)
						{
							case DamageCause::Collision:
								killType = L"Collision";
								break;
							case DamageCause::Gun:
								killType = L"Gun";
								break;
							case DamageCause::MissileTorpedo:
								killType = L"Missile/Torpedo";
								break;
							case DamageCause::CruiseDisrupter:
							case DamageCause::DummyDisrupter:
							case DamageCause::UnkDisrupter:
								killType = L"Cruise Disruptor";
								break;
							case DamageCause::Mine:
								killType = L"Mine";
								break;
							case DamageCause::Suicide:
								killType = L"Suicide";
								break;
							default:
								killType = L"Somehow";
						}

						std::wstring deathMessage;
						if (client == clientKiller.value() || cause == DamageCause::Suicide)
						{
							deathMessage = ReplaceStr(FLHookConfig::i()->messages.msgStyle.deathMsgTextSelfKill, L"%victim", victimName);
						}
						else if (cause == DamageCause::Admin)
						{
							deathMessage = ReplaceStr(FLHookConfig::i()->messages.msgStyle.deathMsgTextAdminKill, L"%victim", victimName);
						}
						else
						{
							std::wstring Killer = ToWChar(Players.GetActiveCharacterName(clientKiller.value()));

							deathMessage = ReplaceStr(FLHookConfig::i()->messages.msgStyle.deathMsgTextPlayerKill, L"%victim", victimName);
							deathMessage = ReplaceStr(deathMessage, L"%killer", Killer);
						}

						deathMessage = ReplaceStr(deathMessage, L"%type", killType);
						if (FLHookConfig::i()->messages.dieMsg && deathMessage.length())
							SendDeathMessage(deathMessage, systemId, client, clientKiller.value());
					}
					else if (dmg.get_inflictor_id())
					{
						std::wstring killType;
						switch (cause)
						{
							case DamageCause::Collision:
								killType = L"Collision";
								break;
							case DamageCause::Gun:
								break;
							case DamageCause::MissileTorpedo:
								killType = L"Missile/Torpedo";
								break;
							case DamageCause::CruiseDisrupter:
							case DamageCause::DummyDisrupter:
							case DamageCause::UnkDisrupter:
								killType = L"Cruise Disruptor";
								break;
							case DamageCause::Mine:
								killType = L"Mine";
								break;
							default:
								killType = L"Gun";
						}

						std::wstring deathMessage = ReplaceStr(FLHookConfig::i()->messages.msgStyle.deathMsgTextNPC, L"%victim", victimName);
						deathMessage = ReplaceStr(deathMessage, L"%type", killType);

						if (FLHookConfig::i()->messages.dieMsg && deathMessage.length())
							SendDeathMessage(deathMessage, systemId, client, 0);
					}
					else if (cause == DamageCause::Suicide)
					{
						if (std::wstring deathMessage = ReplaceStr(FLHookConfig::i()->messages.msgStyle.deathMsgTextSuicide, L"%victim", victimName);
							FLHookConfig::i()->messages.dieMsg && !deathMessage.empty())
							SendDeathMessage(deathMessage, systemId, client, 0);
					}
					else if (cause == DamageCause::Admin)
					{
						if (std::wstring deathMessage = ReplaceStr(FLHookConfig::i()->messages.msgStyle.deathMsgTextAdminKill, L"%victim", victimName);
							FLHookConfig::i()->messages.dieMsg && deathMessage.length())
							SendDeathMessage(deathMessage, systemId, client, 0);
					}
					else
					{
						std::wstring deathMessage = L"Death: " + victimName + L" has died";
						if (FLHookConfig::i()->messages.dieMsg && deathMessage.length())
							SendDeathMessage(deathMessage, systemId, client, 0);
					}
				}

				ClientInfo[client].shipOld = ClientInfo[client].ship;
				ClientInfo[client].ship = 0;
			}
		}
	CATCH_HOOK({})
}

FARPROC g_OldShipDestroyed;

__declspec(naked) void Naked__ShipDestroyed()
{
	__asm {
		mov eax, [esp+0Ch] ; +4
		mov edx, [esp+4]
		push ecx
		push edx
		push ecx
		push eax
		call ShipDestroyed
		pop ecx
		mov eax, [g_OldShipDestroyed]
		jmp eax
		}
}

/**************************************************************************************************************
Called when base was destroyed
**************************************************************************************************************/

void BaseDestroyed(uint objectId, ClientId clientBy)
{
	CallPluginsBefore(HookedCall::IEngine__BaseDestroyed, objectId, clientBy);

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
