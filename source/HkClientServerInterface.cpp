#include "Global.hpp"

void IClientImpl__Startup__Inner(uint, uint)
{
	// load the universe directly before the server becomes internet accessible
	lstBases.clear();
	Universe::IBase* base = Universe::GetFirstBase();
	while (base)
	{
		BASE_INFO bi;
		bi.bDestroyed = false;
		bi.iObjectID = base->lSpaceObjID;
		char* szBaseName = "";
		__asm {
            pushad
            mov ecx, [base]
            mov eax, [base]
            mov eax, [eax]
            call [eax+4]
            mov [szBaseName], eax
            popad
		}

		bi.scBasename = szBaseName;
		bi.baseId = CreateID(szBaseName);
		lstBases.push_back(bi);
		pub::System::LoadSystem(base->systemId);

		base = Universe::GetNextBase();
	}
}

// Doesn't call a Client method, so we need a custom hook
bool IClientImpl::DispatchMsgs()
{
	cdpServer->DispatchMsgs(); // calls IServerImpl functions, which also call
	                           // IClientImpl functions
	return true;
}

namespace IServerImplHook
{
	Timer g_Timers[] = {
	    {ProcessPendingCommands, 50},
	    {TimerCheckKick, 1000},
	    {TimerNPCAndF1Check, 50},
	    {TimerCheckResolveResults, 0},
	};

	void Update__Inner()
	{
		static bool firstTime = true;
		if (firstTime)
		{
			FLHookInit();
			firstTime = false;
		}

		const auto currentTime = timeInMS();
		for (auto& timer : g_Timers)
		{
			// This one isn't actually in seconds, but the plugins should be
			if ((currentTime - timer.lastTime) >= timer.intervalInSeconds)
			{
				timer.lastTime = currentTime;
				timer.func();
			}
		}

		for (PluginData& plugin : PluginManager::ir())
		{
			for (auto& timer : plugin.timers)
			{
				if ((currentTime - timer.lastTime) >= (timer.intervalInSeconds * 100))
				{
					timer.lastTime = currentTime;
					timer.func();
				}
			}
		}

		char* data;
		memcpy(&data, g_FLServerDataPtr + 0x40, 4);
		memcpy(&g_iServerLoad, data + 0x204, 4);
		memcpy(&g_iPlayerCount, data + 0x208, 4);
	}

	CInGame g_Admin;
	bool g_InSubmitChat = false;
	uint g_TextLength = 0;

	bool SubmitChat__Inner(CHAT_ID cidFrom, ulong size, void const* rdlReader, CHAT_ID& cidTo, int)
	{
		TRY_HOOK
		{
			const auto* config = FLHookConfig::i();

			// Group join/leave commands are not parsed
			if (cidTo.iID == SpecialChatIDs::GROUP_EVENT)
				return true;

			// Anything outside normal bounds is aborted to prevent crashes
			if (cidTo.iID > SpecialChatIDs::GROUP_EVENT || cidTo.iID > SpecialChatIDs::PLAYER_MAX && cidTo.iID < SpecialChatIDs::SPECIAL_BASE)
				return false;

			// extract text from rdlReader
			BinaryRDLReader rdl;
			wchar_t wszBuf[1024] = L"";
			uint iRet1;
			rdl.extract_text_from_buffer((unsigned short*)wszBuf, sizeof(wszBuf), iRet1, (const char*)rdlReader, size);
			std::wstring buffer = wszBuf;

			// if this is a message in system chat then convert it to local unless
			// explicitly overriden by the player using /s.
			if (config->userCommands.defaultLocalChat && cidTo.iID == SpecialChatIDs::SYSTEM)
			{
				cidTo.iID = SpecialChatIDs::LOCAL;
			}

			// fix flserver commands and change chat to id so that event logging is
			// accurate.
			bool inBuiltCommand = true;
			g_TextLength = static_cast<uint>(buffer.length());
			if (!buffer.find(L"/g "))
			{
				cidTo.iID = SpecialChatIDs::GROUP;
				g_TextLength -= 3;
			}
			else if (!buffer.find(L"/l "))
			{
				cidTo.iID = SpecialChatIDs::LOCAL;
				g_TextLength -= 3;
			}
			else if (!buffer.find(L"/s "))
			{
				cidTo.iID = SpecialChatIDs::SYSTEM;
				g_TextLength -= 3;
			}
			else if (!buffer.find(L"/u "))
			{
				cidTo.iID = SpecialChatIDs::UNIVERSE;
				g_TextLength -= 3;
			}
			else if (!buffer.find(L"/group "))
			{
				cidTo.iID = SpecialChatIDs::GROUP;
				g_TextLength -= 7;
			}
			else if (!buffer.find(L"/local "))
			{
				cidTo.iID = SpecialChatIDs::LOCAL;
				g_TextLength -= 7;
			}
			else if (!buffer.find(L"/system "))
			{
				cidTo.iID = SpecialChatIDs::SYSTEM;
				g_TextLength -= 8;
			}
			else if (!buffer.find(L"/universe "))
			{
				cidTo.iID = SpecialChatIDs::UNIVERSE;
				g_TextLength -= 10;
			}
			else
			{
				inBuiltCommand = false;
			}

			if (UserCmd_Process(cidFrom.iID, buffer))
				return false;

			else if (buffer[0] == '.')
			{
				CAccount* acc = Players.FindAccountFromClientID(cidFrom.iID);
				std::wstring accDirname = Hk::Client::GetAccountDirName(acc);
				std::string adminFile = scAcctPath + wstos(accDirname) + "\\flhookadmin.ini";
				WIN32_FIND_DATA fd;
				HANDLE hFind = FindFirstFile(adminFile.c_str(), &fd);
				if (hFind != INVALID_HANDLE_VALUE)
				{ // is admin
					FindClose(hFind);
					g_Admin.ReadRights(adminFile);
					g_Admin.clientId = cidFrom.iID;
					g_Admin.wscAdminName = ToWChar(Players.GetActiveCharacterName(cidFrom.iID));
					g_Admin.ExecuteCommandString(buffer.data() + 1);
					return false;
				}
			}

			std::wstring eventString;
			eventString.reserve(256);
			eventString = L"chat";
			eventString += L" from=";
			if (cidFrom.iID == SpecialChatIDs::CONSOLE)
				eventString += L"console";
			else
			{
				const auto* fromName = ToWChar(Players.GetActiveCharacterName(cidFrom.iID));
				if (!fromName)
					eventString += L"unknown";
				else
					eventString += fromName;
			}

			eventString += L" id=";
			eventString += std::to_wstring(cidFrom.iID);

			eventString += L" type=";
			if (cidTo.iID == SpecialChatIDs::UNIVERSE)
				eventString += L"universe";
			else if (cidTo.iID == SpecialChatIDs::GROUP)
			{
				eventString += L"group";
				eventString += L" grpidto=";
				eventString += std::to_wstring(Players.GetGroupID(cidFrom.iID));
			}
			else if (cidTo.iID == SpecialChatIDs::SYSTEM)
				eventString += L"system";
			else if (cidTo.iID == SpecialChatIDs::LOCAL)
				eventString += L"local";
			else
			{
				eventString += L"player";
				eventString += L" to=";

				if (cidTo.iID == SpecialChatIDs::CONSOLE)
					eventString += L"console";
				else
				{
					const auto* toName = ToWChar(Players.GetActiveCharacterName(cidTo.iID));
					if (!toName)
						eventString += L"unknown";
					else
						eventString += toName;
				}

				eventString += L" idto=";
				eventString += std::to_wstring(cidTo.iID);
			}

			eventString += L" text=";
			eventString += buffer;
			ProcessEvent(L"%s", eventString.c_str());

			// check if chat should be suppressed for in-built command prefixes
			if (config->general.suppressInvalidCommands && !inBuiltCommand && (buffer.rfind(L'/', 0) == 0 || buffer.rfind(L'.', 0) == 0))
			{
				return false;
			}

			// Check if any other custom prefixes have been added
			if (!config->general.chatSuppressList.empty())
			{
				auto lcBuffer = ToLower(buffer);
				for (const auto& chat : config->general.chatSuppressList)
				{
					if (lcBuffer.rfind(chat, 0) == 0)
						return false;
				}
			}
		}
		CATCH_HOOK({})

		return true;
	}

	void PlayerLaunch__Inner(uint shipID, uint clientId)
	{
		TRY_HOOK
		{
			ClientInfo[clientId].iShip = shipID;
			ClientInfo[clientId].iKillsInARow = 0;
			ClientInfo[clientId].bCruiseActivated = false;
			ClientInfo[clientId].bThrusterActivated = false;
			ClientInfo[clientId].bEngineKilled = false;
			ClientInfo[clientId].bTradelane = false;

			// adjust cash, this is necessary when cash was added while use was in
			// charmenu/had other char selected
			std::wstring charName = ToLower(ToWChar(Players.GetActiveCharacterName(clientId)));
			for (auto& i : ClientInfo[clientId].lstMoneyFix)
			{
				if (i.character == charName)
				{
					Hk::Player::AddCash(charName, i.iAmount);
					ClientInfo[clientId].lstMoneyFix.remove(i);
					break;
				}
			}
		}
		CATCH_HOOK({})
	}

	void PlayerLaunch__InnerAfter(uint shipID, uint clientId)
	{
		TRY_HOOK
		{
			if (!ClientInfo[clientId].iLastExitedBaseID)
			{
				ClientInfo[clientId].iLastExitedBaseID = 1;

				// event
				ProcessEvent(L"spawn char=%s id=%d system=%s",
				    ToWChar(Players.GetActiveCharacterName(clientId)),
				    clientId,
				    Hk::Client::GetPlayerSystem(clientId).value().c_str());
			}
		}
		CATCH_HOOK({})
	}

	void SPMunitionCollision__Inner(const SSPMunitionCollisionInfo& mci, uint)
	{
		uint clientIdTarget;

		TRY_HOOK { clientIdTarget = Hk::Client::GetClientIDByShip(mci.dwTargetShip).value(); }
		CATCH_HOOK({})

		g_DmgTo = clientIdTarget;
	}

	bool SPObjUpdate__Inner(const SSPObjUpdateInfo& ui, uint clientId)
	{
		// NAN check
		if (!(ui.vPos.x == ui.vPos.x) || !(ui.vPos.y == ui.vPos.y) || !(ui.vPos.z == ui.vPos.z) || !(ui.vDir.x == ui.vDir.x) || !(ui.vDir.y == ui.vDir.y) ||
		    !(ui.vDir.z == ui.vDir.z) || !(ui.vDir.w == ui.vDir.w) || !(ui.fThrottle == ui.fThrottle))
		{
			AddLog(LogType::Normal, LogLevel::Info, L"ERROR: NAN found in SPObjUpdate for id=%u", clientId);
			Hk::Player::Kick(clientId);
			return false;
		}

		// Denormalized check
		float n = ui.vDir.w * ui.vDir.w + ui.vDir.x * ui.vDir.x + ui.vDir.y * ui.vDir.y + ui.vDir.z * ui.vDir.z;
		if (n > 1.21f || n < 0.81f)
		{
			AddLog(LogType::Normal, LogLevel::Info, L"ERROR: Non-normalized quaternion found in SPObjUpdate for id=%u", clientId);
			Hk::Player::Kick(clientId);
			return false;
		}

		// Far check
		if (abs(ui.vPos.x) > 1e7f || abs(ui.vPos.y) > 1e7f || abs(ui.vPos.z) > 1e7f)
		{
			AddLog(LogType::Normal, LogLevel::Info, L"ERROR: Ship position out of bounds in SPObjUpdate for id=%u", clientId);
			Hk::Player::Kick(clientId);
			return false;
		}

		return true;
	}

	void LaunchComplete__Inner(uint, uint shipID) {TRY_HOOK {uint clientId = Hk::Client::GetClientIDByShip(shipID).value();
	if (clientId)
	{
		ClientInfo[clientId].tmSpawnTime = timeInMS(); // save for anti-dockkill
		                                               // is there spawnprotection?
		if (FLHookConfig::i()->general.antiDockKill > 0)
			ClientInfo[clientId].bSpawnProtected = true;
		else
			ClientInfo[clientId].bSpawnProtected = false;
	}

	// event
	ProcessEvent(L"launch char=%s id=%d base=%s system=%s", ToWChar(Players.GetActiveCharacterName(clientId)), clientId,
	    Hk::Client::GetBaseNickByID(ClientInfo[clientId].iLastExitedBaseID).value().c_str(), Hk::Client::GetPlayerSystem(clientId).value().c_str());
} // namespace IServerImplHook
CATCH_HOOK({})
}

std::wstring g_CharBefore;
bool CharacterSelect__Inner(const CHARACTER_ID& cid, uint clientId)
{
	try
	{
		const wchar_t* charName = ToWChar(Players.GetActiveCharacterName(clientId));
		g_CharBefore = charName ? ToWChar(Players.GetActiveCharacterName(clientId)) : L"";
		ClientInfo[clientId].iLastExitedBaseID = 0;
		ClientInfo[clientId].iTradePartner = 0;
	}
	catch (...)
	{
		AddKickLog(clientId, L"Corrupt character file?");
		Hk::Player::Kick(clientId);
		return false;
	}

	Hk::Ini::CharacterSelect(cid, clientId);
	return true;
}

void CharacterSelect__InnerAfter(const CHARACTER_ID& cId, unsigned int clientId)
{
	TRY_HOOK
	{
		std::wstring charName = ToWChar(Players.GetActiveCharacterName(clientId));

		if (g_CharBefore.compare(charName) != 0)
		{
			LoadUserCharSettings(clientId);

			if (FLHookConfig::i()->userCommands.userCmdHelp)
				PrintUserCmdText(clientId,
				    L"To get a list of available commands, type "
				    L"\"/help\" in chat.");

			int iHold;
			auto lstCargo = Hk::Player::EnumCargo(clientId, iHold);
			if (lstCargo.has_error())
			{
				Hk::Player::Kick(clientId);
				return;
			}
			for (const auto& cargo : lstCargo.value())
			{
				if (cargo.iCount < 0)
				{
					AddCheaterLog(charName, L"Negative good-count, likely to have cheated in the past");

					wchar_t wszBuf[256];
					swprintf_s(wszBuf, L"Possible cheating detected (%s)", charName.c_str());
					Hk::Message::MsgU(wszBuf);
					Hk::Player::Ban(clientId, true);
					Hk::Player::Kick(clientId);
					return;
				}
			}

			// event
			CAccount* acc = Players.FindAccountFromClientID(clientId);
			std::wstring dir = Hk::Client::GetAccountDirName(acc);
			auto pi = Hk::Admin::GetPlayerInfo(clientId, false);
			ProcessEvent(L"login char=%s accountdirname=%s id=%d ip=%s", charName.c_str(), dir.c_str(), clientId, pi.value().wscIP.c_str());
		}
	}
	CATCH_HOOK({})
}

void BaseEnter__Inner(uint baseID, uint clientId)
{
	TRY_HOOK
	{
		if (FLHookConfig::i()->general.autobuy)
		{
			Hk::Player::PlayerAutoBuy(clientId, baseID);
		}
	}
	CATCH_HOOK({ AddLog(LogType::Normal, LogLevel::Info, L"Exception in BaseEnter on autobuy"); })
}

void BaseEnter__InnerAfter(uint baseID, uint clientId)
{
	TRY_HOOK
	{
		// adjust cash, this is necessary when cash was added while use was in
		// charmenu/had other char selected
		std::wstring charName = ToLower(ToWChar(Players.GetActiveCharacterName(clientId)));
		for (auto& i : ClientInfo[clientId].lstMoneyFix)
		{
			if (i.character == charName)
			{
				Hk::Player::AddCash(charName, i.iAmount);
				ClientInfo[clientId].lstMoneyFix.remove(i);
				break;
			}
		}

		// anti base-idle
		ClientInfo[clientId].iBaseEnterTime = static_cast<uint>(time(0));

		// event
		ProcessEvent(L"baseenter char=%s id=%d base=%s system=%s",
		    ToWChar(Players.GetActiveCharacterName(clientId)),
		    clientId,
		    Hk::Client::GetBaseNickByID(baseID).value().c_str(),
		    Hk::Client::GetPlayerSystem(clientId).value().c_str());

		// print to log if the char has too much money
		if (const auto value = Hk::Player::GetShipValue((const wchar_t*)Players.GetActiveCharacterName(clientId)))
		{
			if (value > 2100000000.0f)
			{
				std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(clientId);
				AddLog(LogType::Normal, LogLevel::Err, L"Possible corrupt ship charname=%s asset_value=%0.0f", charname.c_str(), value);
			}
		}
	}
	CATCH_HOOK({})
}

void BaseExit__Inner(uint baseID, uint clientId)
{
	TRY_HOOK
	{
		ClientInfo[clientId].iBaseEnterTime = 0;
		ClientInfo[clientId].iLastExitedBaseID = baseID;
	}
	CATCH_HOOK({})
}

void BaseExit__InnerAfter(uint baseID, uint clientId)
{
	TRY_HOOK
	{
		ProcessEvent(L"baseexit char=%s id=%d base=%s system=%s",
		    ToWChar(Players.GetActiveCharacterName(clientId)),
		    clientId,
		    Hk::Client::GetBaseNickByID(baseID).value().c_str(),
		    Hk::Client::GetPlayerSystem(clientId).value().c_str());
	}
	CATCH_HOOK({})
}

void TerminateTrade__InnerAfter(uint clientId, int accepted)
{
	TRY_HOOK
	{
		if (accepted)
		{ // save both chars to prevent cheating in case of
		  // server crash
			Hk::Player::SaveChar(clientId);
			if (ClientInfo[clientId].iTradePartner)
				Hk::Player::SaveChar(ClientInfo[clientId].iTradePartner);
		}

		if (ClientInfo[clientId].iTradePartner)
			ClientInfo[ClientInfo[clientId].iTradePartner].iTradePartner = 0;
		ClientInfo[clientId].iTradePartner = 0;
	}
	CATCH_HOOK({})
}

void InitiateTrade__Inner(uint clientId1, uint clientId2)
{
	if (clientId1 <= MaxClientId && clientId2 <= MaxClientId)
	{
		ClientInfo[clientId1].iTradePartner = clientId2;
		ClientInfo[clientId2].iTradePartner = clientId1;
	}
}

void ActivateEquip__Inner(uint clientId, const XActivateEquip& aq)
{
	TRY_HOOK
	{
		int _;
		const auto lstCargo = Hk::Player::EnumCargo(clientId, _);

		for (auto& cargo : lstCargo.value())
		{
			if (cargo.iID == aq.sID)
			{
				Archetype::Equipment* eq = Archetype::GetEquipment(cargo.iArchID);
				EquipmentType eqType = Hk::Client::GetEqType(eq);

				if (eqType == ET_ENGINE)
				{
					ClientInfo[clientId].bEngineKilled = !aq.bActivate;
					if (!aq.bActivate)
						ClientInfo[clientId].bCruiseActivated = false; // enginekill enabled
				}
			}
		}
	}
	CATCH_HOOK({})
}

void ActivateCruise__Inner(uint clientId, const XActivateCruise& ac)
{
	TRY_HOOK { ClientInfo[clientId].bCruiseActivated = ac.bActivate; }
	CATCH_HOOK({})
}

void ActivateThrusters__Inner(uint clientId, const XActivateThrusters& at)
{
	TRY_HOOK { ClientInfo[clientId].bThrusterActivated = at.bActivate; }
	CATCH_HOOK({})
}

bool GFGoodSell__Inner(const SGFGoodSellInfo& gsi, uint clientId)
{
	TRY_HOOK
	{
		// anti-cheat check
		 
		int _;
		const auto lstCargo = Hk::Player::EnumCargo(clientId, _);
		bool legalSell = false;
		for (const auto& cargo : lstCargo.value())
		{
			if (cargo.iArchID == gsi.iArchID)
			{
				legalSell = true;
				if (abs(gsi.iCount) > cargo.iCount)
				{
					wchar_t buf[512];
					const auto* charName = ToWChar(Players.GetActiveCharacterName(clientId));
					swprintf_s(buf, L"Sold more good than possible item=%08x count=%u", gsi.iArchID, gsi.iCount);
					AddCheaterLog(charName, buf);

					swprintf_s(buf, L"Possible cheating detected (%s)", charName);
					Hk::Message::MsgU(buf);
					Hk::Player::Ban(clientId, true);
					Hk::Player::Kick(clientId);
					return false;
				}
				break;
			}
		}
		if (!legalSell)
		{
			wchar_t buf[1000];
			const auto* charName = ToWChar(Players.GetActiveCharacterName(clientId));
			swprintf_s(buf, L"Sold good player does not have (buggy test), item=%08x", gsi.iArchID);
			AddCheaterLog(charName, buf);

			return false;
		}
	}
	CATCH_HOOK({
		AddLog(LogType::Normal,
		    LogLevel::Info,
		    L"Exception in %s (clientId=%u (%x))",
		    stows(__FUNCTION__).c_str(),
		    clientId,
		    Players.GetActiveCharacterName(clientId));
	})

	return true;
}

bool CharacterInfoReq__Inner(uint clientId, bool)
{
	TRY_HOOK
	{
		if (!ClientInfo[clientId].bCharSelected)
			ClientInfo[clientId].bCharSelected = true;
		else
		{ // pushed f1
			uint shipID = 0;
			pub::Player::GetShip(clientId, shipID);
			if (shipID)
			{ // in space
				ClientInfo[clientId].tmF1Time = timeInMS() + FLHookConfig::i()->general.antiF1;
				return false;
			}
		}
	}
	CATCH_HOOK({})

	return true;
}

bool CharacterInfoReq__Catch(uint clientId, bool)
{
	AddKickLog(clientId, L"Corrupt charfile?");
	Hk::Player::Kick(clientId);
	return false;
}

bool OnConnect__Inner(uint clientId)
{
	TRY_HOOK
	{
		// If ID is too high due to disconnect buffer time then manually drop
		// the connection.
		if (clientId > MaxClientId)
		{
			AddLog(LogType::Normal, LogLevel::Info, L"INFO: Blocking connect in " __FUNCTION__ " due to invalid id, id=%u", clientId);
			CDPClientProxy* cdpClient = g_cClientProxyArray[clientId - 1];
			if (!cdpClient)
				return false;
			cdpClient->Disconnect();
			return false;
		}

		// If this client is in the anti-F1 timeout then force the disconnect.
		if (ClientInfo[clientId].tmF1TimeDisconnect > timeInMS())
		{
			// manual disconnect
			CDPClientProxy* cdpClient = g_cClientProxyArray[clientId - 1];
			if (!cdpClient)
				return false;
			cdpClient->Disconnect();
			return false;
		}

		ClientInfo[clientId].iConnects++;
		ClearClientInfo(clientId);
	}
	CATCH_HOOK({})

	return true;
}

void OnConnect__InnerAfter(uint clientId)
{
	TRY_HOOK
	{
		// event
		std::wstring ip = Hk::Admin::GetPlayerIP(clientId);
		ProcessEvent(L"connect id=%d ip=%s", clientId, ip.c_str());
	}
	CATCH_HOOK({})
}

void DisConnect__Inner(uint clientId, EFLConnection)
{
	if (clientId <= MaxClientId && clientId > 0 && !ClientInfo[clientId].bDisconnected)
	{
		ClientInfo[clientId].bDisconnected = true;
		ClientInfo[clientId].lstMoneyFix.clear();
		ClientInfo[clientId].iTradePartner = 0;

		const auto* charName = ToWChar(Players.GetActiveCharacterName(clientId));
		ProcessEvent(L"disconnect char=%s id=%d", charName, clientId);
	}
}

void JumpInComplete__InnerAfter(uint systemID, uint shipID)
{
	TRY_HOOK
	{
		const auto clientId = Hk::Client::GetClientIDByShip(shipID);
		if (clientId.has_error())
			return;

		// event
		ProcessEvent(L"jumpin char=%s id=%d system=%s", ToWChar(Players.GetActiveCharacterName(clientId.value())), clientId, Hk::Client::GetSystemNickByID(systemID).value().c_str());
	}
	CATCH_HOOK({})
}

void SystemSwitchOutComplete__InnerAfter(uint, uint clientId)
{
	TRY_HOOK
	{
		const auto system = Hk::Client::GetPlayerSystem(clientId);
		ProcessEvent(L"switchout char=%s id=%d system=%s", ToWChar(Players.GetActiveCharacterName(clientId)), clientId, system.value().c_str());
	}
	CATCH_HOOK({})
}

bool Login__InnerBefore(const SLoginInfo& li, uint clientId)
{
	// The startup cache disables reading of the banned file. Check this manually on
	// login and boot the player if they are banned.

	CAccount* acc = Players.FindAccountFromClientID(clientId);
	if (acc)
	{
		std::wstring dir = Hk::Client::GetAccountDirName(acc);

		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);

		std::string path = std::string(szDataPath) + "\\Accts\\MultiPlayer\\" + wstos(dir) + "\\banned";

		FILE* file = fopen(path.c_str(), "r");
		if (file)
		{
			fclose(file);

			// Ban the player
			st6::wstring flStr((ushort*)acc->wszAccID);
			Players.BanAccount(flStr, true);

			// Kick them
			acc->ForceLogout();

			return false;
		}
	}

	return true;
}

bool Login__InnerAfter(const SLoginInfo& li, uint clientId)
{
	TRY_HOOK
	{
		if (clientId > MaxClientId)
			return false; // DisconnectDelay bug

		if (!Hk::Client::IsValidClientID(clientId))
			return false; // player was kicked

		// Kick the player if the account ID doesn't exist. This is caused
		// by a duplicate log on.
		CAccount* acc = Players.FindAccountFromClientID(clientId);
		if (acc && !acc->wszAccID)
		{
			acc->ForceLogout();
			return false;
		}

		if (CallPluginsOther(HookedCall::IServerImpl__Login, HookStep::Mid, li, clientId))
			return false;

		// check for ip ban
		auto ip = Hk::Admin::GetPlayerIP(clientId);

		for (const auto& ban : FLHookConfig::i()->bans.banWildcardsAndIPs)
		{
			if (Wildcard::Fit(wstos(ban).c_str(), wstos(ip).c_str()))
			{
				AddKickLog(clientId, L"IP/Hostname ban(%s matches %s)", ip.c_str(), ban.c_str());
				if (FLHookConfig::i()->bans.banAccountOnMatch)
					Hk::Player::Ban(clientId, true);
				Hk::Player::Kick(clientId);
			}
		}

		// resolve
		RESOLVE_IP rip;
		rip.wscIP = ip;
		rip.wscHostname = L"";
		rip.iConnects = ClientInfo[clientId].iConnects; // security check so that wrong
		                                                // person doesnt get banned
		rip.clientId = clientId;
		EnterCriticalSection(&csIPResolve);
		g_lstResolveIPs.push_back(rip);
		LeaveCriticalSection(&csIPResolve);

		// count players
		struct PlayerData* playerData = nullptr;
		uint playerCount = 0;
		while ((playerData = Players.traverse_active(playerData)))
			playerCount++;

		if (playerCount > (Players.GetMaxPlayerCount() - FLHookConfig::i()->general.reservedSlots))
		{ // check if player has a reserved slot
			std::wstring dir = Hk::Client::GetAccountDirName(acc);
			std::string userFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";

			bool reserved = IniGetB(userFile, "Settings", "ReservedSlot", false);
			if (!reserved)
			{
				Hk::Player::Kick(clientId);
				return false;
			}
		}

		LoadUserSettings(clientId);

		AddConnectLog(clientId, ip);
	}
	CATCH_HOOK({
		CAccount* acc = Players.FindAccountFromClientID(clientId);
		if (acc)
			acc->ForceLogout();
		return false;
	})

	return true;
}

void GoTradelane__Inner(uint clientId, const XGoTradelane& gtl)
{
	if (clientId <= MaxClientId && clientId > 0)
		ClientInfo[clientId].bTradelane = true;
}

bool GoTradelane__Catch(uint clientId, const XGoTradelane& gtl)
{
	uint system;
	pub::Player::GetSystem(clientId, system);
	AddLog(LogType::Normal,
	    LogLevel::Info,
	    L"ERROR: Exception in IServerImpl::GoTradelane charname=%s "
	    "sys=%08x arch=%08x arch2=%08x",
	    wstos(ToWChar(Players.GetActiveCharacterName(clientId))).c_str(),
	    system,
	    gtl.iTradelaneSpaceObj1,
	    gtl.iTradelaneSpaceObj2);
	return true;
}

void StopTradelane__Inner(uint clientId, uint, uint, uint)
{
	if (clientId <= MaxClientId && clientId > 0)
		ClientInfo[clientId].bTradelane = false;
}

void Shutdown__InnerAfter()
{
	FLHookShutdown();
}

// The maximum number of players we can support is MaxClientId
// Add one to the maximum number to allow renames
int g_MaxPlayers = MaxClientId + 1;

void Startup__Inner(const SStartupInfo& si)
{
	FLHookInit_Pre();

	// Startup the server with this number of players.
	char* address = (reinterpret_cast<char*>(hModServer) + ADDR_SRV_PLAYERDBMAXPLAYERSPATCH);
	char nop[] = {'\x90'};
	char movECX[] = {'\xB9'};
	WriteProcMem(address, movECX, sizeof(movECX));
	WriteProcMem(address + 1, &g_MaxPlayers, sizeof(g_MaxPlayers));
	WriteProcMem(address + 5, nop, sizeof(nop));

	StartupCache::Init();
}

void Startup__InnerAfter(const SStartupInfo& si)
{
	// Patch to set maximum number of players to connect. This is normally
	// less than MaxClientId
	char* address = (reinterpret_cast<char*>(hModServer) + ADDR_SRV_PLAYERDBMAXPLAYERS);
	WriteProcMem(address, reinterpret_cast<const void*>(&si.iMaxPlayers), sizeof(g_MaxPlayers));

	// read base market data from ini
	LoadBaseMarket();

	StartupCache::Done();

	Console::ConInfo(L"FLHook Ready");

	flhookReady = true;
}
}

bool IClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(uint clientId, XFireWeaponInfo& fwi)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(\n\tuint clientId = %u\n\tXFireWeaponInfo& fwi = %s\n)",
	    clientId,
	    ToLogString(fwi));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, clientId, fwi);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_FIREWEAPON(clientId, fwi); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, clientId, fwi);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(uint clientId, XActivateEquip& aq)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(\n\tuint clientId = %u\n\tXActivateEquip& aq = %s\n)",
	    clientId,
	    ToLogString(aq));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, clientId, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_ACTIVATEEQUIP(clientId, aq); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, clientId, aq);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(uint clientId, XActivateCruise& aq)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(\n\tuint clientId = %u\n\tXActivateCruise& aq = %s\n)",
	    clientId,
	    ToLogString(aq));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, clientId, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_ACTIVATECRUISE(clientId, aq); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, clientId, aq);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(uint clientId, XActivateThrusters& aq)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(\n\tuint clientId = %u\n\tXActivateThrusters& aq = "
	    L"%s\n)",
	    clientId,
	    ToLogString(aq));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, clientId, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(clientId, aq); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, clientId, aq);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SETTARGET(uint clientId, XSetTarget& st)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_SETTARGET(\n\tuint clientId = %u\n\tXSetTarget& st = %s\n)",
	    clientId,
	    ToLogString(st));

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_SETTARGET(clientId, st); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_6(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_6(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE { unknown_6(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(uint clientId, XGoTradelane& tl)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(\n\tuint clientId = %u\n\tXGoTradelane& tl = %s\n)",
	    clientId,
	    ToLogString(tl));

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_GOTRADELANE(clientId, tl); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(uint clientId, uint shipID, uint archTradelane1, uint archTradelane2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(\n\tuint clientId = %u\n\tuint shipID = %u\n\tuint "
	    L"archTradelane1 = %u\n\tuint archTradelane2 = %u\n)",
	    clientId,
	    shipID,
	    archTradelane1,
	    archTradelane2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_STOPTRADELANE(clientId, shipID, archTradelane1, archTradelane2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(uint clientId, XJettisonCargo& jc)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(\n\tuint clientId = %u\n\tXJettisonCargo& jc = %s\n)",
	    clientId,
	    ToLogString(jc));

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_JETTISONCARGO(clientId, jc); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::SendPacket(uint clientId, void* _genArg1)
{
	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = SendPacket(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Startup(uint _genArg1, uint _genArg2)
{
	IClientImpl__Startup__Inner(_genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Startup(_genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::nullsub(uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::nullsub(\n\tuint _genArg1 = %u\n)", _genArg1);

	CALL_CLIENT_PREAMBLE { nullsub(_genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId,
	    ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_LOGINRESPONSE(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId,
	    ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CHARACTERINFO(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 "
	    L"= %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CHARSELECTVERIFIED(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::Shutdown()
{
	CALL_CLIENT_PREAMBLE { Shutdown(); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::CDPClientProxy__Disconnect(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::CDPClientProxy__Disconnect(\n\tuint clientId = %u\n)", clientId);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = CDPClientProxy__Disconnect(clientId); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

uint IClientImpl::CDPClientProxy__GetSendQSize(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::CDPClientProxy__GetSendQSize(\n\tuint clientId = %u\n)", clientId);

	uint retVal;
	CALL_CLIENT_PREAMBLE { retVal = CDPClientProxy__GetSendQSize(clientId); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

uint IClientImpl::CDPClientProxy__GetSendQBytes(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::CDPClientProxy__GetSendQBytes(\n\tuint clientId = %u\n)", clientId);

	uint retVal;
	CALL_CLIENT_PREAMBLE { retVal = CDPClientProxy__GetSendQBytes(clientId); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

double IClientImpl::CDPClientProxy__GetLinkSaturation(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::CDPClientProxy__GetLinkSaturation(\n\tuint clientId = %u\n)", clientId);

	auto [retVal, skip] = CallPluginsBefore<double>(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, clientId);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = CDPClientProxy__GetLinkSaturation(clientId); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, clientId);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(uint clientId, uint shipArch)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"IClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(\n\tuint clientId = %u\n\tuint shipArch = %u\n)", clientId, shipArch);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, clientId, shipArch);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETSHIPARCH(clientId, shipArch); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, clientId, shipArch);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(uint clientId, float status)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"IClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(\n\tuint clientId = %u\n\tfloat status = %f\n)", clientId, status);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, clientId, status);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETHULLSTATUS(clientId, status); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, clientId, status);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 "
	    L"= %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(clientId, _genArg1); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, clientId, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId,
	    ToLogString(_genArg1));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETEQUIPMENT(clientId, _genArg1); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, clientId, _genArg1);

	return retVal;
}

void IClientImpl::unknown_26(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_26(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_26(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETADDITEM(uint clientId, FLPACKET_UNKNOWN& _genArg1, FLPACKET_UNKNOWN& _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SETADDITEM(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n\tFLPACKET_UNKNOWN& _genArg2 = %s\n)",
	    clientId,
	    ToLogString(_genArg1),
	    ToLogString(_genArg2));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETADDITEM(clientId, _genArg1, _genArg2); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, clientId, _genArg1, _genArg2);

	return retVal;
}

void IClientImpl::unknown_28(uint clientId, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_28(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2,
	    _genArg3);

	CALL_CLIENT_PREAMBLE { unknown_28(clientId, _genArg1, _genArg2, _genArg3); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETSTARTROOM(clientId, _genArg1, _genArg2); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, clientId, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFDESTROYCHARACTER(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFUPDATECHAR(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)",
	    clientId,
	    _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientId,
	    _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_36(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_36(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	CALL_CLIENT_PREAMBLE { unknown_36(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_37(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_37(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	CALL_CLIENT_PREAMBLE { unknown_37(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientId,
	    _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientId,
	    _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(uint clientId, uint reason)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(\n\tuint clientId = %u\n\tuint reason = %u\n)",
	    clientId,
	    reason);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(clientId, reason); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_44(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_44(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	CALL_CLIENT_PREAMBLE { unknown_44(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientId,
	    _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(uint clientId, FLPACKET_CREATESOLAR& solar)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(\n\tuint clientId = %u\n\tFLPACKET_CREATESOLAR& solar = "
	    L"%s\n)",
	    clientId,
	    ToLogString(solar));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, clientId, solar);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATESOLAR(clientId, solar); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, clientId, solar);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATESHIP(uint clientId, FLPACKET_CREATESHIP& ship)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_CREATESHIP(\n\tuint clientId = %u\n\tFLPACKET_CREATESHIP& ship = %s\n)",
	    clientId,
	    ToLogString(ship));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, clientId, ship);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATESHIP(clientId, ship); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, clientId, ship);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATELOOT(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_CREATELOOT(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATELOOT(clientId, _genArg1); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, clientId, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATEMINE(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_CREATEMINE(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATEMINE(clientId, _genArg1); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, clientId, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId,
	    ToLogString(_genArg1));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATEGUIDED(clientId, _genArg1); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, clientId, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId,
	    ToLogString(_genArg1));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATECOUNTER(clientId, _genArg1); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, clientId, _genArg1);

	return retVal;
}

void IClientImpl::unknown_53(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_53(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE { unknown_53(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_54(uint clientId, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_54(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2,
	    _genArg3);

	CALL_CLIENT_PREAMBLE { unknown_54(clientId, _genArg1, _genArg2, _genArg3); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_UPDATEOBJECT(uint clientId, SSPObjUpdateInfo& update)
{
	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, clientId, update);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_UPDATEOBJECT(clientId, update); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, clientId, update);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(uint clientId, FLPACKET_DESTROYOBJECT& destroy)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(\n\tuint clientId = %u\n\tFLPACKET_DESTROYOBJECT& destroy "
	    L"= %s\n)",
	    clientId,
	    ToLogString(destroy));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, clientId, destroy);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_DESTROYOBJECT(clientId, destroy); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, clientId, destroy);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(uint clientId, XActivateEquip& aq)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(\n\tuint clientId = %u\n\tXActivateEquip& aq = %s\n)",
	    clientId,
	    ToLogString(aq));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, clientId, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_ACTIVATEOBJECT(clientId, aq); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, clientId, aq);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId,
	    ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId,
	    ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_LAND(uint clientId, FLPACKET_LAND& land)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_LAND(\n\tuint clientId = %u\n\tFLPACKET_LAND& land = %s\n)",
	    clientId,
	    ToLogString(land));

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_LAND(clientId, land); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_LAUNCH(uint clientId, FLPACKET_LAUNCH& launch)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_LAUNCH(\n\tuint clientId = %u\n\tFLPACKET_LAUNCH& launch = %s\n)",
	    clientId,
	    ToLogString(launch));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, clientId, launch);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_LAUNCH(clientId, launch); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, clientId, launch);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(uint clientId, bool response, uint shipID)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(\n\tuint clientId = %u\n\tbool response = "
	    L"%d\n\tuint shipID = %u\n)",
	    clientId,
	    response,
	    shipID);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, clientId, response, shipID);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(clientId, response, shipID); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, clientId, response, shipID);

	return retVal;
}

void IClientImpl::unknown_63(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_63(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE { unknown_63(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(uint clientId, uint objID, DamageList& dmgList)
{
	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_DAMAGEOBJECT(clientId, objID, dmgList); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)",
	    clientId,
	    _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_ITEMTRACTORED(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_USE_ITEM(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::Send_FLPACKET_SERVER_USE_ITEM(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_USE_ITEM(clientId, _genArg1); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, clientId, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(uint clientId, FLPACKET_SETREPUTATION& rep)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(\n\tuint clientId = %u\n\tFLPACKET_SETREPUTATION& rep = "
	    L"%s\n)",
	    clientId,
	    ToLogString(rep));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, clientId, rep);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETREPUTATION(clientId, rep); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, clientId, rep);

	return retVal;
}

void IClientImpl::unknown_68(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_68(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE { unknown_68(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SENDCOMM(uint clientId, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6,
    uint _genArg7, uint _genArg8, uint _genArg9, uint _genArg10, uint _genArg11, uint _genArg12, uint _genArg13, uint _genArg14, uint _genArg15, uint _genArg16,
    uint _genArg17, uint _genArg18, uint _genArg19, uint _genArg20, uint _genArg21, uint _genArg22)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SENDCOMM(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = "
	    L"%u\n\tuint _genArg3 = %u\n\tuint _genArg4 = %u\n\tuint _genArg5 = %u\n\tuint _genArg6 = %u\n\tuint _genArg7 "
	    L"= %u\n\tuint _genArg8 = %u\n\tuint _genArg9 = %u\n\tuint _genArg10 = %u\n\tuint _genArg11 = %u\n\tuint "
	    L"_genArg12 = %u\n\tuint _genArg13 = %u\n\tuint _genArg14 = %u\n\tuint _genArg15 = %u\n\tuint _genArg16 = "
	    L"%u\n\tuint _genArg17 = %u\n\tuint _genArg18 = %u\n\tuint _genArg19 = %u\n\tuint _genArg20 = %u\n\tuint "
	    L"_genArg21 = %u\n\tuint _genArg22 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2,
	    _genArg3,
	    _genArg4,
	    _genArg5,
	    _genArg6,
	    _genArg7,
	    _genArg8,
	    _genArg9,
	    _genArg10,
	    _genArg11,
	    _genArg12,
	    _genArg13,
	    _genArg14,
	    _genArg15,
	    _genArg16,
	    _genArg17,
	    _genArg18,
	    _genArg19,
	    _genArg20,
	    _genArg21,
	    _genArg22);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM,
	    clientId,
	    _genArg1,
	    _genArg2,
	    _genArg3,
	    _genArg4,
	    _genArg5,
	    _genArg6,
	    _genArg7,
	    _genArg8,
	    _genArg9,
	    _genArg10,
	    _genArg11,
	    _genArg12,
	    _genArg13,
	    _genArg14,
	    _genArg15,
	    _genArg16,
	    _genArg17,
	    _genArg18,
	    _genArg19,
	    _genArg20,
	    _genArg21,
	    _genArg22);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SENDCOMM(clientId,
			    _genArg1,
			    _genArg2,
			    _genArg3,
			    _genArg4,
			    _genArg5,
			    _genArg6,
			    _genArg7,
			    _genArg8,
			    _genArg9,
			    _genArg10,
			    _genArg11,
			    _genArg12,
			    _genArg13,
			    _genArg14,
			    _genArg15,
			    _genArg16,
			    _genArg17,
			    _genArg18,
			    _genArg19,
			    _genArg20,
			    _genArg21,
			    _genArg22);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM,
	    clientId,
	    _genArg1,
	    _genArg2,
	    _genArg3,
	    _genArg4,
	    _genArg5,
	    _genArg6,
	    _genArg7,
	    _genArg8,
	    _genArg9,
	    _genArg10,
	    _genArg11,
	    _genArg12,
	    _genArg13,
	    _genArg14,
	    _genArg15,
	    _genArg16,
	    _genArg17,
	    _genArg18,
	    _genArg19,
	    _genArg20,
	    _genArg21,
	    _genArg22);

	return retVal;
}

void IClientImpl::unknown_70(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_70(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_70(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 "
	    L"= %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(clientId, _genArg1); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, clientId, _genArg1);

	return retVal;
}

void IClientImpl::unknown_72(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_72(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE { unknown_72(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)",
	    clientId,
	    _genArg1);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(clientId, _genArg1); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, clientId, _genArg1);

	return retVal;
}

void IClientImpl::unknown_74(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_74(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE { unknown_74(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_75(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_75(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_75(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_MARKOBJ(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_MARKOBJ(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = "
	    L"%u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MARKOBJ(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_77(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_77(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_77(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETCASH(uint clientId, uint cash)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::Send_FLPACKET_SERVER_SETCASH(\n\tuint clientId = %u\n\tuint cash = %u\n)", clientId, cash);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, clientId, cash);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETCASH(clientId, cash); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, clientId, cash);

	return retVal;
}

void IClientImpl::unknown_79(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_79(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_79(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_80(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_80(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_80(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_81(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_81(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_81(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_82(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_82(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_82(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_83(uint clientId, char* _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_83(\n\tuint clientId = %u\n\tchar* _genArg1 = %s\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_83(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(uint clientId, uint shipID, uint flag, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(\n\tuint clientId = %u\n\tuint shipID = %u\n\tuint flag "
	    L"= %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    shipID,
	    flag,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_REQUEST_RETURNED(clientId, shipID, flag, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_85(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_85(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE { unknown_85(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_86(uint clientId, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_86(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2,
	    _genArg3);

	CALL_CLIENT_PREAMBLE { unknown_86(clientId, _genArg1, _genArg2, _genArg3); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_BURNFUSE(uint clientId, FLPACKET_BURNFUSE& burnFuse)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_BURNFUSE(\n\tuint clientId = %u\n\tFLPACKET_BURNFUSE& burnFuse = %s\n)",
	    clientId,
	    ToLogString(burnFuse));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, clientId, burnFuse);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_BURNFUSE(clientId, burnFuse); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, clientId, burnFuse);

	return retVal;
}

void IClientImpl::unknown_89(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_89(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE { unknown_89(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_90(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_90(\n\tuint clientId = %u\n)", clientId);

	CALL_CLIENT_PREAMBLE { unknown_90(clientId); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_91(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_91(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_91(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_SET_WEAPON_GROUP(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_SET_VISITED_STATE(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_REQUEST_BEST_PATH(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(\n\tuint clientId = %u\n\tuchar* _genArg1 = "
	    L"%p\n\tint _genArg2 = %d\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_96(uint clientId, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_96(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2,
	    _genArg3);

	CALL_CLIENT_PREAMBLE { unknown_96(clientId, _genArg1, _genArg2, _genArg3); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(\n\tuint clientId = %u\n\tuchar* _genArg1 = "
	    L"%p\n\tint _genArg2 = %d\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_SET_MISSION_LOG(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(\n\tuint clientId = %u\n\tuchar* _genArg1 = "
	    L"%p\n\tint _genArg2 = %d\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_SET_INTERFACE_STATE(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_100(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_100(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	CALL_CLIENT_PREAMBLE { unknown_100(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_101(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_101(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE { unknown_101(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_102(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_102(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_102(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_103(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_103(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_103(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_104(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_104(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	CALL_CLIENT_PREAMBLE { unknown_104(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_105(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_105(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	CALL_CLIENT_PREAMBLE { unknown_105(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_106(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_106(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	CALL_CLIENT_PREAMBLE { unknown_106(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_107(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_107(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	CALL_CLIENT_PREAMBLE { unknown_107(clientId, _genArg1, _genArg2); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)",
	    clientId,
	    _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_PLAYER_TRADE(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_109(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_109(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_109(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 "
	    L"= %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SCANNOTIFY(clientId, _genArg1, _genArg2); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, clientId, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(uint clientId, wchar_t* _genArg1, uint _genArg2, char _genArg3)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(\n\tuint clientId = %u\n\twchar_t* _genArg1 = %p\n\tuint "
	    L"_genArg2 = %u\n\tchar _genArg3 = %s\n)",
	    clientId,
	    _genArg1,
	    _genArg2,
	    ToLogString(_genArg3));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, clientId, _genArg1, _genArg2, _genArg3);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_PLAYERLIST(clientId, _genArg1, _genArg2, _genArg3); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, clientId, _genArg1, _genArg2, _genArg3);

	return retVal;
}

void IClientImpl::unknown_112(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_112(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_112(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(\n\tuint clientId = %u\n)", clientId);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, clientId);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_PLAYERLIST_2(clientId); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, clientId);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_6(clientId, _genArg1, _genArg2); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, clientId, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_7(clientId, _genArg1, _genArg2); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, clientId, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId,
	    ToLogString(_genArg1));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE(clientId, _genArg1); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, clientId, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_2(clientId, _genArg1, _genArg2); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, clientId, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(uint clientId, uint targetID, uint rank)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(\n\tuint clientId = %u\n\tuint targetID = %u\n\tuint "
	    L"rank = %u\n)",
	    clientId,
	    targetID,
	    rank);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, clientId, targetID, rank);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_3(clientId, targetID, rank); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, clientId, targetID, rank);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_4(clientId, _genArg1, _genArg2); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, clientId, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_5(clientId, _genArg1, _genArg2); }
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, clientId, _genArg1, _genArg2);

	return retVal;
}

void IClientImpl::unknown_121(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_121(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_121(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(uint clientId, uint shipID, Vector& formationOffset)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(\n\tuint clientId = %u\n\tuint shipID = %u\n\tVector& "
	    L"formationOffset = %s\n)",
	    clientId,
	    shipID,
	    ToLogString(formationOffset));

	bool retVal;
	CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_FORMATION_UPDATE(clientId, shipID, formationOffset); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_123(uint clientId, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    L"IClientImpl::unknown_123(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n\tuint _genArg4 = %u\n\tuint _genArg5 = %u\n\tuint _genArg6 = %u\n)",
	    clientId,
	    _genArg1,
	    _genArg2,
	    _genArg3,
	    _genArg4,
	    _genArg5,
	    _genArg6);

	CALL_CLIENT_PREAMBLE { unknown_123(clientId, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_124(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_124(\n\tuint clientId = %u\n)", clientId);

	CALL_CLIENT_PREAMBLE { unknown_124(clientId); }
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_125(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_125(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE { unknown_125(clientId, _genArg1); }
	CALL_CLIENT_POSTAMBLE;
}

int IClientImpl::unknown_126(char* _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"IClientImpl::unknown_126(\n\tchar* _genArg1 = %s\n)", _genArg1);

	int retVal;
	CALL_CLIENT_PREAMBLE { retVal = unknown_126(_genArg1); }
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

namespace IServerImplHook
{
	void __stdcall FireWeapon(uint clientId, XFireWeaponInfo const& fwi)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"FireWeapon(\n\tuint clientId = %u\n\tXFireWeaponInfo const& fwi = %s\n)", clientId, ToLogString(fwi));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__FireWeapon, clientId, fwi);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.FireWeapon(clientId, fwi); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__FireWeapon, clientId, fwi);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ActivateEquip(uint clientId, XActivateEquip const& aq)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ActivateEquip(\n\tuint clientId = %u\n\tXActivateEquip const& aq = %s\n)", clientId, ToLogString(aq));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateEquip, clientId, aq);

		CHECK_FOR_DISCONNECT;

		ActivateEquip__Inner(clientId, aq);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ActivateEquip(clientId, aq); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateEquip, clientId, aq);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ActivateCruise(uint clientId, XActivateCruise const& ac)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ActivateCruise(\n\tuint clientId = %u\n\tXActivateCruise const& ac = %s\n)", clientId, ToLogString(ac));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateCruise, clientId, ac);

		CHECK_FOR_DISCONNECT;

		ActivateCruise__Inner(clientId, ac);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ActivateCruise(clientId, ac); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateCruise, clientId, ac);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ActivateThrusters(uint clientId, XActivateThrusters const& at)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ActivateThrusters(\n\tuint clientId = %u\n\tXActivateThrusters const& at = %s\n)", clientId, ToLogString(at));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateThrusters, clientId, at);

		CHECK_FOR_DISCONNECT;

		ActivateThrusters__Inner(clientId, at);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ActivateThrusters(clientId, at); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateThrusters, clientId, at);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetTarget(uint clientId, XSetTarget const& st)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SetTarget(\n\tuint clientId = %u\n\tXSetTarget const& st = %s\n)", clientId, ToLogString(st));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTarget, clientId, st);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SetTarget(clientId, st); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetTarget, clientId, st);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall TractorObjects(uint clientId, XTractorObjects const& to)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"TractorObjects(\n\tuint clientId = %u\n\tXTractorObjects const& to = %s\n)", clientId, ToLogString(to));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TractorObjects, clientId, to);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.TractorObjects(clientId, to); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__TractorObjects, clientId, to);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall GoTradelane(uint clientId, XGoTradelane const& gt)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"GoTradelane(\n\tuint clientId = %u\n\tXGoTradelane const& gt = %s\n)", clientId, ToLogString(gt));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GoTradelane, clientId, gt);

		GoTradelane__Inner(clientId, gt);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.GoTradelane(clientId, gt); }
			CALL_SERVER_POSTAMBLE(GoTradelane__Catch(clientId, gt), );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GoTradelane, clientId, gt);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall StopTradelane(uint clientId, uint shipID, uint tradelaneRing1, uint tradelaneRing2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"StopTradelane(\n\tuint clientId = %u\n\tuint shipID = %u\n\tuint tradelaneRing1 = %u\n\tuint "
		    L"tradelaneRing2 = %u\n)",
		    clientId,
		    shipID,
		    tradelaneRing1,
		    tradelaneRing2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradelane, clientId, shipID, tradelaneRing1, tradelaneRing2);

		StopTradelane__Inner(clientId, shipID, tradelaneRing1, tradelaneRing2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.StopTradelane(clientId, shipID, tradelaneRing1, tradelaneRing2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__StopTradelane, clientId, shipID, tradelaneRing1, tradelaneRing2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall JettisonCargo(uint clientId, XJettisonCargo const& jc)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"JettisonCargo(\n\tuint clientId = %u\n\tXJettisonCargo const& jc = %s\n)", clientId, ToLogString(jc));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JettisonCargo, clientId, jc);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.JettisonCargo(clientId, jc); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__JettisonCargo, clientId, jc);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	bool __stdcall Startup(SStartupInfo const& si)
	{
		Startup__Inner(si);

		auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IServerImpl__Startup, si);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { retVal = Server.Startup(si); }
			CALL_SERVER_POSTAMBLE(true, bool());
		}
		Startup__InnerAfter(si);

		CallPluginsAfter(HookedCall::IServerImpl__Startup, si);

		return retVal;
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall Shutdown()
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"Shutdown()");

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Shutdown);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.Shutdown(); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		Shutdown__InnerAfter();
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	int __stdcall Update()
	{
		auto [retVal, skip] = CallPluginsBefore<int>(HookedCall::IServerImpl__Update);

		Update__Inner();

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { retVal = Server.Update(); }
			CALL_SERVER_POSTAMBLE(true, int());
		}

		CallPluginsAfter(HookedCall::IServerImpl__Update);

		return retVal;
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall DisConnect(uint clientId, EFLConnection conn)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"DisConnect(\n\tuint clientId = %u\n\tEFLConnection conn = %s\n)", clientId, ToLogString(conn));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DisConnect, clientId, conn);

		DisConnect__Inner(clientId, conn);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.DisConnect(clientId, conn); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DisConnect, clientId, conn);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall OnConnect(uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"OnConnect(\n\tuint clientId = %u\n)", clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__OnConnect, clientId);

		bool innerCheck = OnConnect__Inner(clientId);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.OnConnect(clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		OnConnect__InnerAfter(clientId);

		CallPluginsAfter(HookedCall::IServerImpl__OnConnect, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall Login(SLoginInfo const& li, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"Login(\n\tSLoginInfo const& li = %s\n\tuint clientId = %u\n)", ToLogString(li), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Login, li, clientId);

		if (!skip && Login__InnerBefore(li, clientId))
		{
			CALL_SERVER_PREAMBLE { Server.Login(li, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		Login__InnerAfter(li, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__Login, li, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall CharacterInfoReq(uint clientId, bool _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"CharacterInfoReq(\n\tuint clientId = %u\n\tbool _genArg1 = %d\n)", clientId, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterInfoReq, clientId, _genArg1);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = CharacterInfoReq__Inner(clientId, _genArg1);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.CharacterInfoReq(clientId, _genArg1); }
			CALL_SERVER_POSTAMBLE(CharacterInfoReq__Catch(clientId, _genArg1), );
		}

		CallPluginsAfter(HookedCall::IServerImpl__CharacterInfoReq, clientId, _genArg1);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall CharacterSelect(CHARACTER_ID const& cid, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"CharacterSelect(\n\tCHARACTER_ID const& cid = %s\n\tuint clientId = %u\n)", ToLogString(cid), clientId);

		std::string charName = cid.szCharFilename;
		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterSelect, charName, clientId);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = CharacterSelect__Inner(cid, clientId);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.CharacterSelect(cid, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		CharacterSelect__InnerAfter(cid, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__CharacterSelect, charName, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall CreateNewCharacter(SCreateCharacterInfo const& _genArg1, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"CreateNewCharacter(\n\tSCreateCharacterInfo const& _genArg1 = %s\n\tuint clientId = %u\n)",
		    ToLogString(_genArg1),
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CreateNewCharacter, _genArg1, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.CreateNewCharacter(_genArg1, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__CreateNewCharacter, _genArg1, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall DestroyCharacter(CHARACTER_ID const& _genArg1, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"DestroyCharacter(\n\tCHARACTER_ID const& _genArg1 = %s\n\tuint clientId = %u\n)",
		    ToLogString(_genArg1),
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DestroyCharacter, _genArg1, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.DestroyCharacter(_genArg1, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DestroyCharacter, _genArg1, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqShipArch(uint archID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqShipArch(\n\tuint archID = %u\n\tuint clientId = %u\n)", archID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqShipArch, archID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ReqShipArch(archID, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqShipArch, archID, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqHullStatus(float status, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqHullStatus(\n\tfloat status = %f\n\tuint clientId = %u\n)", status, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqHullStatus, status, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ReqHullStatus(status, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqHullStatus, status, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqCollisionGroups(st6::list<CollisionGroupDesc> const& collisionGroups, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"ReqCollisionGroups(\n\tst6::list<CollisionGroupDesc> const& collisionGroups = %s\n\tuint clientId = "
		    L"%u\n)",
		    ToLogString(collisionGroups),
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqCollisionGroups, collisionGroups, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ReqCollisionGroups(collisionGroups, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqCollisionGroups, collisionGroups, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqEquipment(EquipDescList const& edl, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqEquipment(\n\tEquipDescList const& edl = %s\n\tuint clientId = %u\n)", ToLogString(edl), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqEquipment, edl, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ReqEquipment(edl, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqEquipment, edl, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqAddItem(uint goodID, char const* hardpoint, int count, float status, bool mounted, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"ReqAddItem(\n\tuint goodID = %u\n\tchar const* hardpoint = %p\n\tint count = %d\n\tfloat status = "
		    L"%f\n\tbool mounted = %d\n\tuint clientId = %u\n)",
		    goodID,
		    hardpoint,
		    count,
		    status,
		    mounted,
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqAddItem, goodID, hardpoint, count, status, mounted, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ReqAddItem(goodID, hardpoint, count, status, mounted, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqAddItem, goodID, hardpoint, count, status, mounted, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqRemoveItem(ushort slotID, int count, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqRemoveItem(\n\tushort slotID = %u\n\tint count = %d\n\tuint clientId = %u\n)", slotID, count, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqRemoveItem, slotID, count, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ReqRemoveItem(slotID, count, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqRemoveItem, slotID, count, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqModifyItem(ushort slotID, char const* hardpoint, int count, float status, bool mounted, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"ReqModifyItem(\n\tushort slotID = %u\n\tchar const* hardpoint = %p\n\tint count = %d\n\tfloat status = "
		    L"%f\n\tbool mounted = %d\n\tuint clientId = %u\n)",
		    slotID,
		    hardpoint,
		    count,
		    status,
		    mounted,
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqModifyItem, slotID, hardpoint, count, status, mounted, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ReqModifyItem(slotID, hardpoint, count, status, mounted, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqModifyItem, slotID, hardpoint, count, status, mounted, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqSetCash(int cash, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqSetCash(\n\tint cash = %d\n\tuint clientId = %u\n)", cash, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqSetCash, cash, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ReqSetCash(cash, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqSetCash, cash, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqChangeCash(int cashAdd, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqChangeCash(\n\tint cashAdd = %d\n\tuint clientId = %u\n)", cashAdd, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqChangeCash, cashAdd, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.ReqChangeCash(cashAdd, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqChangeCash, cashAdd, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall BaseEnter(uint baseID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"BaseEnter(\n\tuint baseID = %u\n\tuint clientId = %u\n)", baseID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseEnter, baseID, clientId);

		CHECK_FOR_DISCONNECT;

		BaseEnter__Inner(baseID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.BaseEnter(baseID, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		BaseEnter__InnerAfter(baseID, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__BaseEnter, baseID, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall BaseExit(uint baseID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"BaseExit(\n\tuint baseID = %u\n\tuint clientId = %u\n)", baseID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseExit, baseID, clientId);

		CHECK_FOR_DISCONNECT;

		BaseExit__Inner(baseID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.BaseExit(baseID, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		BaseExit__InnerAfter(baseID, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__BaseExit, baseID, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall LocationEnter(uint locationID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"LocationEnter(\n\tuint locationID = %u\n\tuint clientId = %u\n)", locationID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationEnter, locationID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.LocationEnter(locationID, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationEnter, locationID, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall LocationExit(uint locationID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"LocationExit(\n\tuint locationID = %u\n\tuint clientId = %u\n)", locationID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationExit, locationID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.LocationExit(locationID, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationExit, locationID, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall BaseInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"BaseInfoRequest(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tbool _genArg3 = %d\n)",
		    _genArg1,
		    _genArg2,
		    _genArg3);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseInfoRequest, _genArg1, _genArg2, _genArg3);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.BaseInfoRequest(_genArg1, _genArg2, _genArg3); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__BaseInfoRequest, _genArg1, _genArg2, _genArg3);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall LocationInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"LocationInfoRequest(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tbool _genArg3 = "
		    L"%d\n)",
		    _genArg1,
		    _genArg2,
		    _genArg3);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationInfoRequest, _genArg1, _genArg2, _genArg3);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.LocationInfoRequest(_genArg1, _genArg2, _genArg3); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationInfoRequest, _genArg1, _genArg2, _genArg3);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall GFObjSelect(unsigned int _genArg1, unsigned int _genArg2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"GFObjSelect(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)", _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFObjSelect, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.GFObjSelect(_genArg1, _genArg2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFObjSelect, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall GFGoodVaporized(SGFGoodVaporizedInfo const& gvi, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GFGoodVaporized(\n\tSGFGoodVaporizedInfo const& gvi = %s\n\tuint clientId = %u\n)", ToLogString(gvi), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodVaporized, gvi, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.GFGoodVaporized(gvi, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodVaporized, gvi, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall MissionResponse(unsigned int _genArg1, unsigned long _genArg2, bool _genArg3, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"MissionResponse(\n\tunsigned int _genArg1 = %u\n\tunsigned long _genArg2 = %u\n\tbool _genArg3 = "
		    L"%d\n\tuint clientId = %u\n)",
		    _genArg1,
		    _genArg2,
		    _genArg3,
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__MissionResponse, _genArg1, _genArg2, _genArg3, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.MissionResponse(_genArg1, _genArg2, _genArg3, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__MissionResponse, _genArg1, _genArg2, _genArg3, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall TradeResponse(unsigned char const* _genArg1, int _genArg2, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"TradeResponse(\n\tunsigned char const* _genArg1 = %p\n\tint _genArg2 = %d\n\tuint clientId = %u\n)",
		    _genArg1,
		    _genArg2,
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TradeResponse, _genArg1, _genArg2, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.TradeResponse(_genArg1, _genArg2, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__TradeResponse, _genArg1, _genArg2, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall GFGoodBuy(SGFGoodBuyInfo const& _genArg1, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GFGoodBuy(\n\tSGFGoodBuyInfo const& _genArg1 = %s\n\tuint clientId = %u\n)", ToLogString(_genArg1), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodBuy, _genArg1, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.GFGoodBuy(_genArg1, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodBuy, _genArg1, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall GFGoodSell(SGFGoodSellInfo const& _genArg1, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GFGoodSell(\n\tSGFGoodSellInfo const& _genArg1 = %s\n\tuint clientId = %u\n)", ToLogString(_genArg1), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodSell, _genArg1, clientId);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = GFGoodSell__Inner(_genArg1, clientId);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.GFGoodSell(_genArg1, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodSell, _genArg1, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SystemSwitchOutComplete(uint shipID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SystemSwitchOutComplete(\n\tuint shipID = %u\n\tuint clientId = %u\n)", shipID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SystemSwitchOutComplete, shipID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SystemSwitchOutComplete(shipID, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		SystemSwitchOutComplete__InnerAfter(shipID, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__SystemSwitchOutComplete, shipID, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall PlayerLaunch(uint shipID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"PlayerLaunch(\n\tuint shipID = %u\n\tuint clientId = %u\n)", shipID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PlayerLaunch, shipID, clientId);

		CHECK_FOR_DISCONNECT;

		PlayerLaunch__Inner(shipID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.PlayerLaunch(shipID, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		PlayerLaunch__InnerAfter(shipID, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__PlayerLaunch, shipID, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall LaunchComplete(uint baseID, uint shipID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"LaunchComplete(\n\tuint baseID = %u\n\tuint shipID = %u\n)", baseID, shipID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LaunchComplete, baseID, shipID);

		LaunchComplete__Inner(baseID, shipID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.LaunchComplete(baseID, shipID); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LaunchComplete, baseID, shipID);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall JumpInComplete(uint systemID, uint shipID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"JumpInComplete(\n\tuint systemID = %u\n\tuint shipID = %u\n)", systemID, shipID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JumpInComplete, systemID, shipID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.JumpInComplete(systemID, shipID); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		JumpInComplete__InnerAfter(systemID, shipID);

		CallPluginsAfter(HookedCall::IServerImpl__JumpInComplete, systemID, shipID);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall Hail(unsigned int _genArg1, unsigned int _genArg2, unsigned int _genArg3)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"Hail(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tunsigned int _genArg3 = %u\n)",
		    _genArg1,
		    _genArg2,
		    _genArg3);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Hail, _genArg1, _genArg2, _genArg3);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.Hail(_genArg1, _genArg2, _genArg3); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__Hail, _genArg1, _genArg2, _genArg3);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPObjUpdate(SSPObjUpdateInfo const& ui, uint clientId)
	{
		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjUpdate, ui, clientId);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = SPObjUpdate__Inner(ui, clientId);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SPObjUpdate(ui, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPObjUpdate, ui, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPMunitionCollision(SSPMunitionCollisionInfo const& mci, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"SPMunitionCollision(\n\tSSPMunitionCollisionInfo const& mci = %s\n\tuint clientId = %u\n)",
		    ToLogString(mci),
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPMunitionCollision, mci, clientId);

		CHECK_FOR_DISCONNECT;

		SPMunitionCollision__Inner(mci, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SPMunitionCollision(mci, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPMunitionCollision, mci, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPObjCollision(SSPObjCollisionInfo const& oci, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SPObjCollision(\n\tSSPObjCollisionInfo const& oci = %s\n\tuint clientId = %u\n)", ToLogString(oci), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjCollision, oci, clientId);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SPObjCollision(oci, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPObjCollision, oci, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPRequestUseItem(SSPUseItem const& ui, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SPRequestUseItem(\n\tSSPUseItem const& ui = %s\n\tuint clientId = %u\n)", ToLogString(ui), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestUseItem, ui, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SPRequestUseItem(ui, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPRequestUseItem, ui, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPRequestInvincibility(uint shipID, bool enable, InvincibilityReason reason, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"SPRequestInvincibility(\n\tuint shipID = %u\n\tbool enable = %d\n\tInvincibilityReason reason = "
		    L"%s\n\tuint clientId = %u\n)",
		    shipID,
		    enable,
		    ToLogString(reason),
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestInvincibility, shipID, enable, reason, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SPRequestInvincibility(shipID, enable, reason, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPRequestInvincibility, shipID, enable, reason, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestEvent(int eventType, uint shipID, uint dockTarget, uint _genArg1, ulong _genArg2, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"RequestEvent(\n\tint eventType = %d\n\tuint shipID = %u\n\tuint dockTarget = %u\n\tuint _genArg1 = "
		    L"%u\n\tulong _genArg2 = %u\n\tuint clientId = %u\n)",
		    eventType,
		    shipID,
		    dockTarget,
		    _genArg1,
		    _genArg2,
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestEvent, eventType, shipID, dockTarget, _genArg1, _genArg2, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.RequestEvent(eventType, shipID, dockTarget, _genArg1, _genArg2, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestEvent, eventType, shipID, dockTarget, _genArg1, _genArg2, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestCancel(int eventType, uint shipID, uint _genArg1, ulong _genArg2, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"RequestCancel(\n\tint eventType = %d\n\tuint shipID = %u\n\tuint _genArg1 = %u\n\tulong _genArg2 = "
		    L"%u\n\tuint clientId = %u\n)",
		    eventType,
		    shipID,
		    _genArg1,
		    _genArg2,
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCancel, eventType, shipID, _genArg1, _genArg2, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.RequestCancel(eventType, shipID, _genArg1, _genArg2, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestCancel, eventType, shipID, _genArg1, _genArg2, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall MineAsteroid(uint systemID, Vector const& pos, uint crateID, uint lootID, uint count, uint clientId)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"MineAsteroid(\n\tuint systemID = %u\n\tVector const& pos = %s\n\tuint crateID = %u\n\tuint lootID = "
		    L"%u\n\tuint count = %u\n\tuint clientId = %u\n)",
		    systemID,
		    ToLogString(pos),
		    crateID,
		    lootID,
		    count,
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__MineAsteroid, systemID, pos, crateID, lootID, count, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.MineAsteroid(systemID, pos, crateID, lootID, count, clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__MineAsteroid, systemID, pos, crateID, lootID, count, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestCreateShip(uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"RequestCreateShip(\n\tuint clientId = %u\n)", clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCreateShip, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.RequestCreateShip(clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestCreateShip, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPScanCargo(uint const& _genArg1, uint const& _genArg2, uint _genArg3)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"SPScanCargo(\n\tuint const& _genArg1 = %s\n\tuint const& _genArg2 = %s\n\tuint _genArg3 = %u\n)",
		    ToLogString(_genArg1),
		    ToLogString(_genArg2),
		    _genArg3);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPScanCargo, _genArg1, _genArg2, _genArg3);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SPScanCargo(_genArg1, _genArg2, _genArg3); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPScanCargo, _genArg1, _genArg2, _genArg3);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetManeuver(uint clientId, XSetManeuver const& sm)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SetManeuver(\n\tuint clientId = %u\n\tXSetManeuver const& sm = %s\n)", clientId, ToLogString(sm));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetManeuver, clientId, sm);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SetManeuver(clientId, sm); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetManeuver, clientId, sm);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall InterfaceItemUsed(uint _genArg1, uint _genArg2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"InterfaceItemUsed(\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)", _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InterfaceItemUsed, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.InterfaceItemUsed(_genArg1, _genArg2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__InterfaceItemUsed, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall AbortMission(uint clientId, uint _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"AbortMission(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AbortMission, clientId, _genArg1);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.AbortMission(clientId, _genArg1); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AbortMission, clientId, _genArg1);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetWeaponGroup(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"SetWeaponGroup(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientId,
		    _genArg1,
		    _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetWeaponGroup, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SetWeaponGroup(clientId, _genArg1, _genArg2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetWeaponGroup, clientId, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetVisitedState(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"SetVisitedState(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientId,
		    _genArg1,
		    _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetVisitedState, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SetVisitedState(clientId, _genArg1, _genArg2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetVisitedState, clientId, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestBestPath(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"RequestBestPath(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientId,
		    _genArg1,
		    _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestBestPath, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.RequestBestPath(clientId, _genArg1, _genArg2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestBestPath, clientId, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestPlayerStats(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"RequestPlayerStats(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientId,
		    _genArg1,
		    _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestPlayerStats, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.RequestPlayerStats(clientId, _genArg1, _genArg2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestPlayerStats, clientId, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall PopupDialog(uint clientId, uint buttonClicked)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"PopupDialog(\n\tuint clientId = %u\n\tuint buttonClicked = %u\n)", clientId, buttonClicked);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PopupDialog, clientId, buttonClicked);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.PopUpDialog(clientId, buttonClicked); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__PopupDialog, clientId, buttonClicked);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestGroupPositions(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"RequestGroupPositions(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientId,
		    _genArg1,
		    _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestGroupPositions, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.RequestGroupPositions(clientId, _genArg1, _genArg2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestGroupPositions, clientId, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetInterfaceState(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"SetInterfaceState(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientId,
		    _genArg1,
		    _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetInterfaceState, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SetInterfaceState(clientId, _genArg1, _genArg2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetInterfaceState, clientId, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestRankLevel(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"RequestRankLevel(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientId,
		    _genArg1,
		    _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestRankLevel, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.RequestRankLevel(clientId, _genArg1, _genArg2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestRankLevel, clientId, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall InitiateTrade(uint clientId1, uint clientId2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"InitiateTrade(\n\tuint clientId1 = %u\n\tuint clientId2 = %u\n)", clientId1, clientId2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InitiateTrade, clientId1, clientId2);

		InitiateTrade__Inner(clientId1, clientId2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.InitiateTrade(clientId1, clientId2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__InitiateTrade, clientId1, clientId2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall TerminateTrade(uint clientId, int accepted)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"TerminateTrade(\n\tuint clientId = %u\n\tint accepted = %d\n)", clientId, accepted);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TerminateTrade, clientId, accepted);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.TerminateTrade(clientId, accepted); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		TerminateTrade__InnerAfter(clientId, accepted);

		CallPluginsAfter(HookedCall::IServerImpl__TerminateTrade, clientId, accepted);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall AcceptTrade(uint clientId, bool _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"AcceptTrade(\n\tuint clientId = %u\n\tbool _genArg1 = %d\n)", clientId, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AcceptTrade, clientId, _genArg1);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.AcceptTrade(clientId, _genArg1); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AcceptTrade, clientId, _genArg1);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetTradeMoney(uint clientId, ulong _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SetTradeMoney(\n\tuint clientId = %u\n\tulong _genArg1 = %u\n)", clientId, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTradeMoney, clientId, _genArg1);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SetTradeMoney(clientId, _genArg1); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetTradeMoney, clientId, _genArg1);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall AddTradeEquip(uint clientId, EquipDesc const& ed)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"AddTradeEquip(\n\tuint clientId = %u\n\tEquipDesc const& ed = %s\n)", clientId, ToLogString(ed));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AddTradeEquip, clientId, ed);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.AddTradeEquip(clientId, ed); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AddTradeEquip, clientId, ed);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall DelTradeEquip(uint clientId, EquipDesc const& ed)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"DelTradeEquip(\n\tuint clientId = %u\n\tEquipDesc const& ed = %s\n)", clientId, ToLogString(ed));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DelTradeEquip, clientId, ed);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.DelTradeEquip(clientId, ed); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DelTradeEquip, clientId, ed);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestTrade(uint _genArg1, uint _genArg2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"RequestTrade(\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)", _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestTrade, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.RequestTrade(_genArg1, _genArg2); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestTrade, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall StopTradeRequest(uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"StopTradeRequest(\n\tuint clientId = %u\n)", clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradeRequest, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.StopTradeRequest(clientId); }
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__StopTradeRequest, clientId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall Dock(uint const& _genArg1, uint const& _genArg2) {}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SubmitChat(CHAT_ID cidFrom, ulong size, void const* rdlReader, CHAT_ID cidTo, int _genArg1)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    L"SubmitChat(\n\tCHAT_ID cidFrom = %s\n\tulong size = %u\n\tvoid const* rdlReader = %p\n\tCHAT_ID cidTo = "
		    L"%s\n\tint _genArg1 = %d\n)",
		    ToLogString(cidFrom),
		    size,
		    rdlReader,
		    ToLogString(cidTo),
		    _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SubmitChat, cidFrom.iID, size, rdlReader, cidTo.iID, _genArg1);

		bool innerCheck = SubmitChat__Inner(cidFrom, size, rdlReader, cidTo, _genArg1);
		if (!innerCheck)
			return;
		g_InSubmitChat = true;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE { Server.SubmitChat(cidFrom, size, rdlReader, cidTo, _genArg1); }
			CALL_SERVER_POSTAMBLE(true, );
		}
		g_InSubmitChat = false;

		CallPluginsAfter(HookedCall::IServerImpl__SubmitChat, cidFrom.iID, size, rdlReader, cidTo.iID, _genArg1);
	}
} // namespace IServerImplHook

HookEntry IServerImplEntries[] = {
    {FARPROC(IServerImplHook::SubmitChat), -0x008, nullptr},
    {FARPROC(IServerImplHook::FireWeapon), 0x000, nullptr},
    {FARPROC(IServerImplHook::ActivateEquip), 0x004, nullptr},
    {FARPROC(IServerImplHook::ActivateCruise), 0x008, nullptr},
    {FARPROC(IServerImplHook::ActivateThrusters), 0x00C, nullptr},
    {FARPROC(IServerImplHook::SetTarget), 0x010, nullptr},
    {FARPROC(IServerImplHook::TractorObjects), 0x014, nullptr},
    {FARPROC(IServerImplHook::GoTradelane), 0x018, nullptr},
    {FARPROC(IServerImplHook::StopTradelane), 0x01C, nullptr},
    {FARPROC(IServerImplHook::JettisonCargo), 0x020, nullptr},
    {FARPROC(IServerImplHook::DisConnect), 0x040, nullptr},
    {FARPROC(IServerImplHook::OnConnect), 0x044, nullptr},
    {FARPROC(IServerImplHook::Login), 0x048, nullptr},
    {FARPROC(IServerImplHook::CharacterInfoReq), 0x04C, nullptr},
    {FARPROC(IServerImplHook::CharacterSelect), 0x050, nullptr},
    {FARPROC(IServerImplHook::CreateNewCharacter), 0x058, nullptr},
    {FARPROC(IServerImplHook::DestroyCharacter), 0x05C, nullptr},
    {FARPROC(IServerImplHook::ReqShipArch), 0x064, nullptr},
    {FARPROC(IServerImplHook::ReqHullStatus), 0x068, nullptr},
    {FARPROC(IServerImplHook::ReqCollisionGroups), 0x06C, nullptr},
    {FARPROC(IServerImplHook::ReqEquipment), 0x070, nullptr},
    {FARPROC(IServerImplHook::ReqAddItem), 0x078, nullptr},
    {FARPROC(IServerImplHook::ReqRemoveItem), 0x07C, nullptr},
    {FARPROC(IServerImplHook::ReqModifyItem), 0x080, nullptr},
    {FARPROC(IServerImplHook::ReqSetCash), 0x084, nullptr},
    {FARPROC(IServerImplHook::ReqChangeCash), 0x088, nullptr},
    {FARPROC(IServerImplHook::BaseEnter), 0x08C, nullptr},
    {FARPROC(IServerImplHook::BaseExit), 0x090, nullptr},
    {FARPROC(IServerImplHook::LocationEnter), 0x094, nullptr},
    {FARPROC(IServerImplHook::LocationExit), 0x098, nullptr},
    {FARPROC(IServerImplHook::BaseInfoRequest), 0x09C, nullptr},
    {FARPROC(IServerImplHook::LocationInfoRequest), 0x0A0, nullptr},
    {FARPROC(IServerImplHook::GFObjSelect), 0x0A4, nullptr},
    {FARPROC(IServerImplHook::GFGoodVaporized), 0x0A8, nullptr},
    {FARPROC(IServerImplHook::MissionResponse), 0x0AC, nullptr},
    {FARPROC(IServerImplHook::TradeResponse), 0x0B0, nullptr},
    {FARPROC(IServerImplHook::GFGoodBuy), 0x0B4, nullptr},
    {FARPROC(IServerImplHook::GFGoodSell), 0x0B8, nullptr},
    {FARPROC(IServerImplHook::SystemSwitchOutComplete), 0x0BC, nullptr},
    {FARPROC(IServerImplHook::PlayerLaunch), 0x0C0, nullptr},
    {FARPROC(IServerImplHook::LaunchComplete), 0x0C4, nullptr},
    {FARPROC(IServerImplHook::JumpInComplete), 0x0C8, nullptr},
    {FARPROC(IServerImplHook::Hail), 0x0CC, nullptr},
    {FARPROC(IServerImplHook::SPObjUpdate), 0x0D0, nullptr},
    {FARPROC(IServerImplHook::SPMunitionCollision), 0x0D4, nullptr},
    {FARPROC(IServerImplHook::SPObjCollision), 0x0DC, nullptr},
    {FARPROC(IServerImplHook::SPRequestUseItem), 0x0E0, nullptr},
    {FARPROC(IServerImplHook::SPRequestInvincibility), 0x0E4, nullptr},
    {FARPROC(IServerImplHook::RequestEvent), 0x0F0, nullptr},
    {FARPROC(IServerImplHook::RequestCancel), 0x0F4, nullptr},
    {FARPROC(IServerImplHook::MineAsteroid), 0x0F8, nullptr},
    {FARPROC(IServerImplHook::RequestCreateShip), 0x100, nullptr},
    {FARPROC(IServerImplHook::SPScanCargo), 0x104, nullptr},
    {FARPROC(IServerImplHook::SetManeuver), 0x108, nullptr},
    {FARPROC(IServerImplHook::InterfaceItemUsed), 0x10C, nullptr},
    {FARPROC(IServerImplHook::AbortMission), 0x110, nullptr},
    {FARPROC(IServerImplHook::SetWeaponGroup), 0x118, nullptr},
    {FARPROC(IServerImplHook::SetVisitedState), 0x11C, nullptr},
    {FARPROC(IServerImplHook::RequestBestPath), 0x120, nullptr},
    {FARPROC(IServerImplHook::RequestPlayerStats), 0x124, nullptr},
    {FARPROC(IServerImplHook::PopupDialog), 0x128, nullptr},
    {FARPROC(IServerImplHook::RequestGroupPositions), 0x12C, nullptr},
    {FARPROC(IServerImplHook::SetInterfaceState), 0x134, nullptr},
    {FARPROC(IServerImplHook::RequestRankLevel), 0x138, nullptr},
    {FARPROC(IServerImplHook::InitiateTrade), 0x13C, nullptr},
    {FARPROC(IServerImplHook::TerminateTrade), 0x140, nullptr},
    {FARPROC(IServerImplHook::AcceptTrade), 0x144, nullptr},
    {FARPROC(IServerImplHook::SetTradeMoney), 0x148, nullptr},
    {FARPROC(IServerImplHook::AddTradeEquip), 0x14C, nullptr},
    {FARPROC(IServerImplHook::DelTradeEquip), 0x150, nullptr},
    {FARPROC(IServerImplHook::RequestTrade), 0x154, nullptr},
    {FARPROC(IServerImplHook::StopTradeRequest), 0x158, nullptr},
    {FARPROC(IServerImplHook::Dock), 0x16C, nullptr},
};

void PluginManager::setupProps()
{
	setProps(HookedCall::IEngine__CShip__Init, true, false, false);
	setProps(HookedCall::IEngine__CShip__Destroy, true, false, false);
	setProps(HookedCall::IEngine__UpdateTime, true, false, true);
	setProps(HookedCall::IEngine__ElapseTime, true, false, true);
	setProps(HookedCall::IEngine__DockCall, true, false, false);
	setProps(HookedCall::IEngine__LaunchPosition, true, false, false);
	setProps(HookedCall::IEngine__ShipDestroyed, true, false, false);
	setProps(HookedCall::IEngine__BaseDestroyed, true, false, false);
	setProps(HookedCall::IEngine__GuidedHit, true, false, false);
	setProps(HookedCall::IEngine__AddDamageEntry, true, false, false);
	setProps(HookedCall::IEngine__DamageHit, true, false, false);
	setProps(HookedCall::IEngine__AllowPlayerDamage, true, false, false);
	setProps(HookedCall::IEngine__SendDeathMessage, true, false, false);
	setProps(HookedCall::FLHook__TimerCheckKick, true, false, false);
	setProps(HookedCall::FLHook__TimerNPCAndF1Check, true, false, false);
	setProps(HookedCall::FLHook__UserCommand__Process, true, false, false);
	setProps(HookedCall::FLHook__AdminCommand__Help, true, false, true);
	setProps(HookedCall::FLHook__AdminCommand__Process, true, false, false);
	setProps(HookedCall::FLHook__LoadSettings, true, false, true);
	setProps(HookedCall::FLHook__LoadCharacterSettings, true, false, true);
	setProps(HookedCall::FLHook__ClearClientInfo, true, false, true);
	setProps(HookedCall::FLHook__ProcessEvent, true, false, false);
	setProps(HookedCall::IChat__SendChat, true, false, false);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, true, false, true);
	setProps(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, true, false, true);
	setProps(HookedCall::IServerImpl__FireWeapon, true, false, true);
	setProps(HookedCall::IServerImpl__ActivateEquip, true, false, true);
	setProps(HookedCall::IServerImpl__ActivateCruise, true, false, true);
	setProps(HookedCall::IServerImpl__ActivateThrusters, true, false, true);
	setProps(HookedCall::IServerImpl__SetTarget, true, false, true);
	setProps(HookedCall::IServerImpl__TractorObjects, true, false, true);
	setProps(HookedCall::IServerImpl__GoTradelane, true, false, true);
	setProps(HookedCall::IServerImpl__StopTradelane, true, false, true);
	setProps(HookedCall::IServerImpl__JettisonCargo, true, false, true);
	setProps(HookedCall::IServerImpl__Startup, true, false, true);
	setProps(HookedCall::IServerImpl__Shutdown, true, false, false);
	setProps(HookedCall::IServerImpl__Update, true, false, true);
	setProps(HookedCall::IServerImpl__DisConnect, true, false, true);
	setProps(HookedCall::IServerImpl__OnConnect, true, false, true);
	setProps(HookedCall::IServerImpl__Login, true, false, true);
	setProps(HookedCall::IServerImpl__CharacterInfoReq, true, false, true);
	setProps(HookedCall::IServerImpl__CharacterSelect, true, false, true);
	setProps(HookedCall::IServerImpl__CreateNewCharacter, true, false, true);
	setProps(HookedCall::IServerImpl__DestroyCharacter, true, false, true);
	setProps(HookedCall::IServerImpl__ReqShipArch, true, false, true);
	setProps(HookedCall::IServerImpl__ReqHullStatus, true, false, true);
	setProps(HookedCall::IServerImpl__ReqCollisionGroups, true, false, true);
	setProps(HookedCall::IServerImpl__ReqEquipment, true, false, true);
	setProps(HookedCall::IServerImpl__ReqAddItem, true, false, true);
	setProps(HookedCall::IServerImpl__ReqRemoveItem, true, false, true);
	setProps(HookedCall::IServerImpl__ReqModifyItem, true, false, true);
	setProps(HookedCall::IServerImpl__ReqSetCash, true, false, true);
	setProps(HookedCall::IServerImpl__ReqChangeCash, true, false, true);
	setProps(HookedCall::IServerImpl__BaseEnter, true, false, true);
	setProps(HookedCall::IServerImpl__BaseExit, true, false, true);
	setProps(HookedCall::IServerImpl__LocationEnter, true, false, true);
	setProps(HookedCall::IServerImpl__LocationExit, true, false, true);
	setProps(HookedCall::IServerImpl__BaseInfoRequest, true, false, true);
	setProps(HookedCall::IServerImpl__LocationInfoRequest, true, false, true);
	setProps(HookedCall::IServerImpl__GFObjSelect, true, false, true);
	setProps(HookedCall::IServerImpl__GFGoodVaporized, true, false, true);
	setProps(HookedCall::IServerImpl__MissionResponse, true, false, true);
	setProps(HookedCall::IServerImpl__TradeResponse, true, false, true);
	setProps(HookedCall::IServerImpl__GFGoodBuy, true, false, true);
	setProps(HookedCall::IServerImpl__GFGoodSell, true, false, true);
	setProps(HookedCall::IServerImpl__SystemSwitchOutComplete, true, false, true);
	setProps(HookedCall::IServerImpl__PlayerLaunch, true, false, true);
	setProps(HookedCall::IServerImpl__LaunchComplete, true, false, true);
	setProps(HookedCall::IServerImpl__JumpInComplete, true, false, true);
	setProps(HookedCall::IServerImpl__Hail, true, false, true);
	setProps(HookedCall::IServerImpl__SPObjUpdate, true, false, true);
	setProps(HookedCall::IServerImpl__SPMunitionCollision, true, false, true);
	setProps(HookedCall::IServerImpl__SPObjCollision, true, false, true);
	setProps(HookedCall::IServerImpl__SPRequestUseItem, true, false, true);
	setProps(HookedCall::IServerImpl__SPRequestInvincibility, true, false, true);
	setProps(HookedCall::IServerImpl__RequestEvent, true, false, true);
	setProps(HookedCall::IServerImpl__RequestCancel, true, false, true);
	setProps(HookedCall::IServerImpl__MineAsteroid, true, false, true);
	setProps(HookedCall::IServerImpl__RequestCreateShip, true, false, true);
	setProps(HookedCall::IServerImpl__SPScanCargo, true, false, true);
	setProps(HookedCall::IServerImpl__SetManeuver, true, false, true);
	setProps(HookedCall::IServerImpl__InterfaceItemUsed, true, false, true);
	setProps(HookedCall::IServerImpl__AbortMission, true, false, true);
	setProps(HookedCall::IServerImpl__SetWeaponGroup, true, false, true);
	setProps(HookedCall::IServerImpl__SetVisitedState, true, false, true);
	setProps(HookedCall::IServerImpl__RequestBestPath, true, false, true);
	setProps(HookedCall::IServerImpl__RequestPlayerStats, true, false, true);
	setProps(HookedCall::IServerImpl__PopupDialog, true, false, true);
	setProps(HookedCall::IServerImpl__RequestGroupPositions, true, false, true);
	setProps(HookedCall::IServerImpl__SetInterfaceState, true, false, true);
	setProps(HookedCall::IServerImpl__RequestRankLevel, true, false, true);
	setProps(HookedCall::IServerImpl__InitiateTrade, true, false, true);
	setProps(HookedCall::IServerImpl__TerminateTrade, true, false, true);
	setProps(HookedCall::IServerImpl__AcceptTrade, true, false, true);
	setProps(HookedCall::IServerImpl__SetTradeMoney, true, false, true);
	setProps(HookedCall::IServerImpl__AddTradeEquip, true, false, true);
	setProps(HookedCall::IServerImpl__DelTradeEquip, true, false, true);
	setProps(HookedCall::IServerImpl__RequestTrade, true, false, true);
	setProps(HookedCall::IServerImpl__StopTradeRequest, true, false, true);
	setProps(HookedCall::IServerImpl__SubmitChat, true, false, true);
}