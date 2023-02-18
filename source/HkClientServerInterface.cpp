#include "Global.hpp"
#include "Features/Mail.hpp"
#include "Features/TempBan.hpp"

void IClientImpl__Startup__Inner(uint, uint)
{
	// load the universe directly before the server becomes internet accessible
	CoreGlobals::i()->allBases.clear();
	Universe::IBase* base = Universe::GetFirstBase();
	while (base)
	{
		BaseInfo bi;
		bi.bDestroyed = false;
		bi.iObjectId = base->lSpaceObjId;
		char const* szBaseName = "";
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
		CoreGlobals::i()->allBases.push_back(bi);
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
	    {TimerTempBanCheck, 15000},
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
			if (plugin.timers)
			{
				for (auto& timer : *plugin.timers)
				{
					if ((currentTime - timer.lastTime) >= (timer.intervalInSeconds * 1000))
					{
						timer.lastTime = currentTime;
						timer.func();
					}
				}
			}
		}

		auto globals = CoreGlobals::i();
		char* data;
		memcpy(&data, g_FLServerDataPtr + 0x40, 4);
		memcpy(&globals->serverLoadInMs, data + 0x204, 4);
		memcpy(&globals->playerCount, data + 0x208, 4);
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
			if (cidTo.iId == SpecialChatIds::GROUP_EVENT)
				return true;

			// Anything outside normal bounds is aborted to prevent crashes
			if (cidTo.iId > SpecialChatIds::GROUP_EVENT || cidTo.iId > SpecialChatIds::PLAYER_MAX && cidTo.iId < SpecialChatIds::SPECIAL_BASE)
				return false;

			// extract text from rdlReader
			BinaryRDLReader rdl;
			wchar_t wszBuf[1024] = L"";
			uint iRet1;
			rdl.extract_text_from_buffer((unsigned short*)wszBuf, sizeof(wszBuf), iRet1, (const char*)rdlReader, size);
			std::wstring buffer = wszBuf;

			// if this is a message in system chat then convert it to local unless
			// explicitly overriden by the player using /s.
			if (config->messages.defaultLocalChat && cidTo.iId == SpecialChatIds::SYSTEM)
			{
				cidTo.iId = SpecialChatIds::LOCAL;
			}

			// fix flserver commands and change chat to id so that event logging is
			// accurate.
			bool inBuiltCommand = true;
			g_TextLength = buffer.length();
			if (!buffer.find(L"/g "))
			{
				cidTo.iId = SpecialChatIds::GROUP;
				g_TextLength -= 3;
			}
			else if (!buffer.find(L"/l "))
			{
				cidTo.iId = SpecialChatIds::LOCAL;
				g_TextLength -= 3;
			}
			else if (!buffer.find(L"/s "))
			{
				cidTo.iId = SpecialChatIds::SYSTEM;
				g_TextLength -= 3;
			}
			else if (!buffer.find(L"/u "))
			{
				cidTo.iId = SpecialChatIds::UNIVERSE;
				g_TextLength -= 3;
			}
			else if (!buffer.find(L"/group "))
			{
				cidTo.iId = SpecialChatIds::GROUP;
				g_TextLength -= 7;
			}
			else if (!buffer.find(L"/local "))
			{
				cidTo.iId = SpecialChatIds::LOCAL;
				g_TextLength -= 7;
			}
			else if (!buffer.find(L"/system "))
			{
				cidTo.iId = SpecialChatIds::SYSTEM;
				g_TextLength -= 8;
			}
			else if (!buffer.find(L"/universe "))
			{
				cidTo.iId = SpecialChatIds::UNIVERSE;
				g_TextLength -= 10;
			}
			else
			{
				inBuiltCommand = false;
			}

			if (UserCmd_Process(cidFrom.iId, buffer))
				return false;

			else if (buffer[0] == '.')
			{
				const CAccount* acc = Players.FindAccountFromClientID(cidFrom.iId);
				std::wstring accDirname = Hk::Client::GetAccountDirName(acc);
				std::string adminFile = CoreGlobals::c()->accPath + wstos(accDirname) + "\\flhookadmin.ini";
				WIN32_FIND_DATA fd;
				HANDLE hFind = FindFirstFile(adminFile.c_str(), &fd);
				if (hFind != INVALID_HANDLE_VALUE)
				{ // is admin
					FindClose(hFind);
					g_Admin.ReadRights(adminFile);
					g_Admin.client = cidFrom.iId;
					g_Admin.wscAdminName = ToWChar(Players.GetActiveCharacterName(cidFrom.iId));
					g_Admin.ExecuteCommandString(buffer.data() + 1);
					return false;
				}
			}

			std::wstring eventString;
			eventString.reserve(256);
			eventString = L"chat";
			eventString += L" from=";
			if (cidFrom.iId == SpecialChatIds::CONSOLE)
				eventString += L"console";
			else
			{
				const auto* fromName = ToWChar(Players.GetActiveCharacterName(cidFrom.iId));
				if (!fromName)
					eventString += L"unknown";
				else
					eventString += fromName;
			}

			eventString += L" id=";
			eventString += std::to_wstring(cidFrom.iId);

			eventString += L" type=";
			if (cidTo.iId == SpecialChatIds::UNIVERSE)
				eventString += L"universe";
			else if (cidTo.iId == SpecialChatIds::GROUP)
			{
				eventString += L"group";
				eventString += L" grpidto=";
				eventString += std::to_wstring(Players.GetGroupID(cidFrom.iId));
			}
			else if (cidTo.iId == SpecialChatIds::SYSTEM)
				eventString += L"system";
			else if (cidTo.iId == SpecialChatIds::LOCAL)
				eventString += L"local";
			else
			{
				eventString += L"player";
				eventString += L" to=";

				if (cidTo.iId == SpecialChatIds::CONSOLE)
					eventString += L"console";
				else
				{
					const auto* toName = ToWChar(Players.GetActiveCharacterName(cidTo.iId));
					if (!toName)
						eventString += L"unknown";
					else
						eventString += toName;
				}

				eventString += L" idto=";
				eventString += std::to_wstring(cidTo.iId);
			}

			eventString += L" text=";
			eventString += buffer;
			ProcessEvent(L"{}", eventString.c_str());

			// check if chat should be suppressed for in-built command prefixes
			if (config->messages.suppressInvalidCommands && !inBuiltCommand && (buffer.rfind(L'/', 0) == 0 || buffer.rfind(L'.', 0) == 0))
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

	void PlayerLaunch__Inner(uint shipId, ClientId client)
	{
		TRY_HOOK
		{
			ClientInfo[client].ship = shipId;
			ClientInfo[client].iKillsInARow = 0;
			ClientInfo[client].bCruiseActivated = false;
			ClientInfo[client].bThrusterActivated = false;
			ClientInfo[client].bEngineKilled = false;
			ClientInfo[client].bTradelane = false;

			// adjust cash, this is necessary when cash was added while use was in
			// charmenu/had other char selected
			std::wstring charName = ToLower(ToWChar(Players.GetActiveCharacterName(client)));
			for (const auto& i : ClientInfo[client].lstMoneyFix)
			{
				if (i.character == charName)
				{
					Hk::Player::AddCash(charName, i.uAmount);
					ClientInfo[client].lstMoneyFix.remove(i);
					break;
				}
			}
		}
		CATCH_HOOK({})
	}

	void PlayerLaunch__InnerAfter([[maybe_unused]] uint shipId, ClientId client)
	{
		TRY_HOOK
		{
			if (!ClientInfo[client].iLastExitedBaseId)
			{
				ClientInfo[client].iLastExitedBaseId = 1;

				// event
				ProcessEvent(L"spawn char={} id={} system={}",
				    ToWChar(Players.GetActiveCharacterName(client)),
				    client,
				    Hk::Client::GetPlayerSystem(client).value().c_str());
			}
		}
		CATCH_HOOK({})
	}

	void SPMunitionCollision__Inner(const SSPMunitionCollisionInfo& mci, uint)
	{
		TRY_HOOK
		{
			const auto isClient = Hk::Client::GetClientIdByShip(mci.dwTargetShip);
			if (isClient.has_value())
				CoreGlobals::i()->damageToClientId = isClient.value();
		}
		CATCH_HOOK({})
	}

	bool SPObjUpdate__Inner(const SSPObjUpdateInfo& ui, ClientId client)
	{
		// NAN check
		if (isnan(ui.vPos.x) || isnan(ui.vPos.y) || isnan(ui.vPos.z) || isnan(ui.vDir.w) || isnan(ui.vDir.x) || isnan(ui.vDir.y) || 
			isnan(ui.vDir.z) || isnan(ui.fThrottle))
		{
			AddLog(LogType::Normal, LogLevel::Err, std::format("NAN found in SPObjUpdate for id={}", client));
			Hk::Player::Kick(client);
			return false;
		}

		// Denormalized check
		if (float n = ui.vDir.w * ui.vDir.w + ui.vDir.x * ui.vDir.x + ui.vDir.y * ui.vDir.y + ui.vDir.z * ui.vDir.z; n > 1.21f || n < 0.81f)
		{
			AddLog(LogType::Normal, LogLevel::Err, std::format("Non-normalized quaternion found in SPObjUpdate for id={}", client));
			Hk::Player::Kick(client);
			return false;
		}

		// Far check
		if (abs(ui.vPos.x) > 1e7f || abs(ui.vPos.y) > 1e7f || abs(ui.vPos.z) > 1e7f)
		{
			AddLog(LogType::Normal, LogLevel::Err, std::format("Ship position out of bounds in SPObjUpdate for id={}", client));
			Hk::Player::Kick(client);
			return false;
		}

		return true;
	}

	void LaunchComplete__Inner(uint, uint shipId) {TRY_HOOK {ClientId client = Hk::Client::GetClientIdByShip(shipId).value();
	if (client)
	{
		ClientInfo[client].tmSpawnTime = timeInMS(); // save for anti-dockkill
		                                             // is there spawnprotection?
		if (FLHookConfig::i()->general.antiDockKill > 0)
			ClientInfo[client].bSpawnProtected = true;
		else
			ClientInfo[client].bSpawnProtected = false;
	}

	// event
	ProcessEvent(L"launch char={} id={} base={} system={}", ToWChar(Players.GetActiveCharacterName(client)), client,
	    Hk::Client::GetBaseNickByID(ClientInfo[client].iLastExitedBaseId).value().c_str(), Hk::Client::GetPlayerSystem(client).value().c_str());
} // namespace IServerImplHook
CATCH_HOOK({})
}

std::wstring g_CharBefore;
bool CharacterSelect__Inner(const CHARACTER_ID& cid, ClientId client)
{
	try
	{
		const wchar_t* charName = ToWChar(Players.GetActiveCharacterName(client));
		g_CharBefore = charName ? ToWChar(Players.GetActiveCharacterName(client)) : L"";
		ClientInfo[client].iLastExitedBaseId = 0;
		ClientInfo[client].iTradePartner = 0;
	}
	catch (...)
	{
		AddKickLog(client, "Corrupt character file?");
		Hk::Player::Kick(client);
		return false;
	}

	Hk::Ini::CharacterSelect(cid, client);
	return true;
}

void CharacterSelect__InnerAfter(const CHARACTER_ID& cId, unsigned int client)
{
	TRY_HOOK
	{
		std::wstring charName = ToWChar(Players.GetActiveCharacterName(client));

		if (g_CharBefore.compare(charName) != 0)
		{
			LoadUserCharSettings(client);

			if (FLHookConfig::i()->userCommands.userCmdHelp)
				PrintUserCmdText(client,
				    L"To get a list of available commands, type "
				    L"\"/help\" in chat.");

			int iHold;
			auto lstCargo = Hk::Player::EnumCargo(client, iHold);
			if (lstCargo.has_error())
			{
				Hk::Player::Kick(client);
				return;
			}
			for (const auto& cargo : lstCargo.value())
			{
				if (cargo.iCount < 0)
				{
					AddCheaterLog(charName, "Negative good-count, likely to have cheated in the past");

					Hk::Message::MsgU(std::format(L"Possible cheating detected ({})", charName.c_str()));
					Hk::Player::Ban(client, true);
					Hk::Player::Kick(client);
					return;
				}
			}

			// event
			CAccount* acc = Players.FindAccountFromClientID(client);
			std::wstring dir = Hk::Client::GetAccountDirName(acc);
			auto pi = Hk::Admin::GetPlayerInfo(client, false);
			ProcessEvent(L"login char={} accountdirname={} id={} ip={}", charName.c_str(), dir.c_str(), client, pi.value().wscIP.c_str());

			MailManager::i()->SendMailNotification(client);
		}
	}
	CATCH_HOOK({})
}

void BaseEnter__Inner(uint baseId, ClientId client)
{
}

void BaseEnter__InnerAfter(uint baseId, ClientId client)
{
	TRY_HOOK
	{
		// adjust cash, this is necessary when cash was added while use was in
		// charmenu/had other char selected
		std::wstring charName = ToLower(ToWChar(Players.GetActiveCharacterName(client)));
		for (const auto& i : ClientInfo[client].lstMoneyFix)
		{
			if (i.character == charName)
			{
				Hk::Player::AddCash(charName, i.uAmount);
				ClientInfo[client].lstMoneyFix.remove(i);
				break;
			}
		}

		// anti base-idle
		ClientInfo[client].iBaseEnterTime = static_cast<uint>(time(0));

		// event
		ProcessEvent(L"baseenter char={} id={} base={} system={}",
		    ToWChar(Players.GetActiveCharacterName(client)),
		    client,
		    Hk::Client::GetBaseNickByID(baseId).value().c_str(),
		    Hk::Client::GetPlayerSystem(client).value().c_str());

		// print to log if the char has too much money
		if (const auto value = Hk::Player::GetShipValue((const wchar_t*)Players.GetActiveCharacterName(client)); value.has_value()
		 && value.value() > 2100000000)
		{
			std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
			AddLog(LogType::Normal, LogLevel::Err, std::format("Possible corrupt ship charname={} asset_value={}", wstos(charname), value.value()));
		}
	}
	CATCH_HOOK({})
}

void BaseExit__Inner(uint baseId, ClientId client)
{
	TRY_HOOK
	{
		ClientInfo[client].iBaseEnterTime = 0;
		ClientInfo[client].iLastExitedBaseId = baseId;
	}
	CATCH_HOOK({})
}

void BaseExit__InnerAfter(uint baseId, ClientId client)
{
	TRY_HOOK
	{
		ProcessEvent(L"baseexit char={} id={} base={} system={}",
		    ToWChar(Players.GetActiveCharacterName(client)),
		    client,
		    Hk::Client::GetBaseNickByID(baseId).value().c_str(),
		    Hk::Client::GetPlayerSystem(client).value().c_str());
	}
	CATCH_HOOK({})
}

void TerminateTrade__InnerAfter(ClientId client, int accepted)
{
	TRY_HOOK
	{
		if (accepted)
		{ // save both chars to prevent cheating in case of server crash
			Hk::Player::SaveChar(client);
			if (ClientInfo[client].iTradePartner)
				Hk::Player::SaveChar(ClientInfo[client].iTradePartner);
		}

		if (ClientInfo[client].iTradePartner)
			ClientInfo[ClientInfo[client].iTradePartner].iTradePartner = 0;
		ClientInfo[client].iTradePartner = 0;
	}
	CATCH_HOOK({})
}

void InitiateTrade__Inner(ClientId client1, ClientId client2)
{
	if (client1 <= MaxClientId && client2 <= MaxClientId)
	{
		ClientInfo[client1].iTradePartner = client2;
		ClientInfo[client2].iTradePartner = client1;
	}
}

void ActivateEquip__Inner(ClientId client, const XActivateEquip& aq)
{
	TRY_HOOK
	{
		int _;
		const auto lstCargo = Hk::Player::EnumCargo(client, _);

		for (auto& cargo : lstCargo.value())
		{
			if (cargo.iId == aq.sId)
			{
				Archetype::Equipment* eq = Archetype::GetEquipment(cargo.iArchId);
				EquipmentType eqType = Hk::Client::GetEqType(eq);

				if (eqType == ET_ENGINE)
				{
					ClientInfo[client].bEngineKilled = !aq.bActivate;
					if (!aq.bActivate)
						ClientInfo[client].bCruiseActivated = false; // enginekill enabled
				}
			}
		}
	}
	CATCH_HOOK({})
}

void ActivateCruise__Inner(ClientId client, const XActivateCruise& ac)
{
	TRY_HOOK
	{
		ClientInfo[client].bCruiseActivated = ac.bActivate;
	}
	CATCH_HOOK({})
}

void ActivateThrusters__Inner(ClientId client, const XActivateThrusters& at)
{
	TRY_HOOK
	{
		ClientInfo[client].bThrusterActivated = at.bActivate;
	}
	CATCH_HOOK({})
}

bool GFGoodSell__Inner(const SGFGoodSellInfo& gsi, ClientId client)
{
	TRY_HOOK
	{
		// anti-cheat check

		int _;
		const auto lstCargo = Hk::Player::EnumCargo(client, _);
		bool legalSell = false;
		for (const auto& cargo : lstCargo.value())
		{
			if (cargo.iArchId == gsi.iArchId)
			{
				legalSell = true;
				if (abs(gsi.iCount) > cargo.iCount)
				{
					const auto* charName = ToWChar(Players.GetActiveCharacterName(client));
					AddCheaterLog(charName, std::format("Sold more good than possible item={} count={}", gsi.iArchId, gsi.iCount));

					Hk::Message::MsgU(std::format(L"Possible cheating detected ({})", charName));
					Hk::Player::Ban(client, true);
					Hk::Player::Kick(client);
					return false;
				}
				break;
			}
		}
		if (!legalSell)
		{
			const auto* charName = ToWChar(Players.GetActiveCharacterName(client));
			AddCheaterLog(charName, std::format("Sold good player does not have (buggy test), item={}", gsi.iArchId));

			return false;
		}
	}
	CATCH_HOOK({
		AddLog(LogType::Normal,
		    LogLevel::Err,
		    std::format("Exception in {} (client={} ({}))", __FUNCTION__, client, wstos(Hk::Client::GetCharacterNameByID(client).value())));
	})

	return true;
}

bool CharacterInfoReq__Inner(ClientId client, bool)
{
	TRY_HOOK
	{
		if (!ClientInfo[client].bCharSelected)
			ClientInfo[client].bCharSelected = true;
		else
		{ // pushed f1
			uint shipId = 0;
			pub::Player::GetShip(client, shipId);
			if (shipId)
			{ // in space
				ClientInfo[client].tmF1Time = timeInMS() + FLHookConfig::i()->general.antiF1;
				return false;
			}
		}
	}
	CATCH_HOOK({})

	return true;
}

bool CharacterInfoReq__Catch(ClientId client, bool)
{
	AddKickLog(client, "Corrupt charfile?");
	Hk::Player::Kick(client);
	return false;
}

bool OnConnect__Inner(ClientId client)
{
	TRY_HOOK
	{
		// If Id is too high due to disconnect buffer time then manually drop
		// the connection.
		if (client > MaxClientId)
		{
			AddLog(LogType::Normal, LogLevel::Warn, std::format("INFO: Blocking connect in {} due to invalid id, id={}", __FUNCTION__, client));
			CDPClientProxy* cdpClient = clientProxyArray[client - 1];
			if (!cdpClient)
				return false;
			cdpClient->Disconnect();
			return false;
		}

		// If this client is in the anti-F1 timeout then force the disconnect.
		if (ClientInfo[client].tmF1TimeDisconnect > timeInMS())
		{
			// manual disconnect
			CDPClientProxy* cdpClient = clientProxyArray[client - 1];
			if (!cdpClient)
				return false;
			cdpClient->Disconnect();
			return false;
		}

		ClientInfo[client].iConnects++;
		ClearClientInfo(client);
	}
	CATCH_HOOK({})

	return true;
}

void OnConnect__InnerAfter(ClientId client)
{
	TRY_HOOK
	{
		// event
		std::wstring ip = Hk::Admin::GetPlayerIP(client);
		ProcessEvent(L"connect id={} ip={}", client, ip.c_str());
	}
	CATCH_HOOK({})
}

void DisConnect__Inner(ClientId client, EFLConnection)
{
	if (client <= MaxClientId && client > 0 && !ClientInfo[client].bDisconnected)
	{
		ClientInfo[client].bDisconnected = true;
		ClientInfo[client].lstMoneyFix.clear();
		ClientInfo[client].iTradePartner = 0;

		const auto* charName = ToWChar(Players.GetActiveCharacterName(client));
		ProcessEvent(L"disconnect char={} id={}", charName, client);
	}
}

void JumpInComplete__InnerAfter(uint systemId, uint shipId)
{
	TRY_HOOK
	{
		const auto client = Hk::Client::GetClientIdByShip(shipId);
		if (client.has_error())
			return;

		// event
		ProcessEvent(L"jumpin char={} id={} system={}",
		    ToWChar(Players.GetActiveCharacterName(client.value())),
		    client,
		    Hk::Client::GetSystemNickByID(systemId).value().c_str());
	}
	CATCH_HOOK({})
}

void SystemSwitchOutComplete__InnerAfter(uint, ClientId client)
{
	TRY_HOOK
	{
		const auto system = Hk::Client::GetPlayerSystem(client);
		ProcessEvent(L"switchout char={} id={} system={}", ToWChar(Players.GetActiveCharacterName(client)), client, system.value().c_str());
	}
	CATCH_HOOK({})
}

bool Login__InnerBefore(const SLoginInfo& li, ClientId client)
{
	// The startup cache disables reading of the banned file. Check this manually on
	// login and boot the player if they are banned.

	CAccount* acc = Players.FindAccountFromClientID(client);
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
			st6::wstring flStr((ushort*)acc->wszAccId);
			Players.BanAccount(flStr, true);

			// Kick them
			acc->ForceLogout();

			return false;
		}
	}

	return true;
}

bool Login__InnerAfter(const SLoginInfo& li, ClientId client)
{
	TRY_HOOK
	{
		if (client > MaxClientId)
			return false; // DisconnectDelay bug

		if (!Hk::Client::IsValidClientID(client))
			return false; // player was kicked

		// Kick the player if the account Id doesn't exist. This is caused
		// by a duplicate log on.
		CAccount* acc = Players.FindAccountFromClientID(client);
		if (acc && !acc->wszAccId)
		{
			acc->ForceLogout();
			return false;
		}

		// check for ip ban
		auto ip = Hk::Admin::GetPlayerIP(client);

		for (const auto& ban : FLHookConfig::i()->bans.banWildcardsAndIPs)
		{
			if (Wildcard::Fit(wstos(ban).c_str(), wstos(ip).c_str()))
			{
				AddKickLog(client, wstos(std::format(L"IP/Hostname ban({} matches {})", ip.c_str(), ban.c_str())));
				if (FLHookConfig::i()->bans.banAccountOnMatch)
					Hk::Player::Ban(client, true);
				Hk::Player::Kick(client);
			}
		}

		// resolve
		RESOLVE_IP rip = {client, ClientInfo[client].iConnects, ip};

		EnterCriticalSection(&csIPResolve);
		g_lstResolveIPs.push_back(rip);
		LeaveCriticalSection(&csIPResolve);

		// count players
		PlayerData* playerData = nullptr;
		uint playerCount = 0;
		while ((playerData = Players.traverse_active(playerData)))
			playerCount++;

		if (playerCount > (Players.GetMaxPlayerCount() - FLHookConfig::i()->general.reservedSlots))
		{ // check if player has a reserved slot
			std::wstring dir = Hk::Client::GetAccountDirName(acc);
			std::string userFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini";

			bool reserved = IniGetB(userFile, "Settings", "ReservedSlot", false);
			if (!reserved)
			{
				Hk::Player::Kick(client);
				return false;
			}
		}

		LoadUserSettings(client);

		AddConnectLog(client, wstos(ip));
	}
	CATCH_HOOK({
		CAccount* acc = Players.FindAccountFromClientID(client);
		if (acc)
			acc->ForceLogout();
		return false;
	})

	return true;
}

void GoTradelane__Inner(ClientId client, const XGoTradelane& gtl)
{
	if (client <= MaxClientId && client > 0)
		ClientInfo[client].bTradelane = true;
}

bool GoTradelane__Catch(ClientId client, const XGoTradelane& gtl)
{
	uint system;
	pub::Player::GetSystem(client, system);
	AddLog(LogType::Normal,
	    LogLevel::Err,
	    wstos(std::format(L"Exception in IServerImpl::GoTradelane charname={} sys=0x{:08X} arch=0x{:08X} arch2=0x{:08X}",
	        Hk::Client::GetCharacterNameByID(client).value(),
	        system,
	        gtl.iTradelaneSpaceObj1,
	        gtl.iTradelaneSpaceObj2)));
	return true;
}

void StopTradelane__Inner(ClientId client, uint, uint, uint)
{
	if (client <= MaxClientId && client > 0)
		ClientInfo[client].bTradelane = false;
}

void Shutdown__InnerAfter()
{
	FLHookShutdown();
}

// The maximum number of players we can support is MaxClientId
// Add one to the maximum number to allow renames
const int g_MaxPlayers = MaxClientId + 1;

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

	// Clean up any mail to chars that no longer exist
	MailManager::i()->CleanUpOldMail();

	StartupCache::Done();

	Console::ConInfo("FLHook Ready");

	CoreGlobals::i()->flhookReady = true;
}
}

bool IClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(ClientId client, XFireWeaponInfo& fwi)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(\n\tClientId client = {}\n\tXFireWeaponInfo& fwi = {}\n)", client, ToLogString(fwi))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, client, fwi);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_FIREWEAPON(client, fwi);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, client, fwi);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(ClientId client, XActivateEquip& aq)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(\n\tClientId client = {}\n\tXActivateEquip& aq = {}\n)", client, ToLogString(aq))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, client, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_ACTIVATEEQUIP(client, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, client, aq);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(ClientId client, XActivateCruise& aq)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(
	        std::format(L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(\n\tClientId client = {}\n\tXActivateCruise& aq = {}\n)", client, ToLogString(aq))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, client, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_ACTIVATECRUISE(client, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, client, aq);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(ClientId client, XActivateThrusters& aq)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(\n\tClientId client = {}\n\tXActivateThrusters& aq = {}\n)", client, ToLogString(aq))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, client, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(client, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, client, aq);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SETTARGET(ClientId client, XSetTarget& st)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_SETTARGET(\n\tClientId client = {}\n\tXSetTarget& st = {}\n)", client, ToLogString(st))));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SETTARGET(client, st);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_6(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_6(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	CALL_CLIENT_PREAMBLE
	{
		unknown_6(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(ClientId client, XGoTradelane& tl)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(\n\tClientId client = {}\n\tXGoTradelane& tl = {}\n)", client, ToLogString(tl))));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_GOTRADELANE(client, tl);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(ClientId client, uint shipId, uint archTradelane1, uint archTradelane2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(\n\tClientId client = {}\n\tuint shipId = {}\n\tuint archTradelane1 = {}\n\tuint "
	                      L"archTradelane2 = {}\n)",
	        client,
	        shipId,
	        archTradelane1,
	        archTradelane2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_STOPTRADELANE(client, shipId, archTradelane1, archTradelane2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(ClientId client, XJettisonCargo& jc)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(\n\tClientId client = {}\n\tXJettisonCargo& jc = {}\n)", client, ToLogString(jc))));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_JETTISONCARGO(client, jc);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::SendPacket(ClientId client, void* _genArg1)
{
	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = SendPacket(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Startup(uint _genArg1, uint _genArg2)
{
	IClientImpl__Startup__Inner(_genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Startup(_genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::nullsub(uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::nullsub(\n\tuint _genArg1 = {}\n)", _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		nullsub(_genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_LOGINRESPONSE(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_CHARACTERINFO(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)",
	        client,
	        ToLogString(_genArg1))));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_CHARSELECTVERIFIED(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::Shutdown()
{
	CALL_CLIENT_PREAMBLE
	{
		Shutdown();
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::CDPClientProxy__Disconnect(ClientId client)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::CDPClientProxy__Disconnect(\n\tClientId client = {}\n)", client));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = CDPClientProxy__Disconnect(client);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

uint IClientImpl::CDPClientProxy__GetSendQSize(ClientId client)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::CDPClientProxy__GetSendQSize(\n\tClientId client = {}\n)", client));

	uint retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = CDPClientProxy__GetSendQSize(client);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

uint IClientImpl::CDPClientProxy__GetSendQBytes(ClientId client)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::CDPClientProxy__GetSendQBytes(\n\tClientId client = {}\n)", client));

	uint retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = CDPClientProxy__GetSendQBytes(client);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

double IClientImpl::CDPClientProxy__GetLinkSaturation(ClientId client)
{
	auto [retVal, skip] = CallPluginsBefore<double>(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, client);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = CDPClientProxy__GetLinkSaturation(client);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, client);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(ClientId client, uint shipArch)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(\n\tClientId client = {}\n\tuint shipArch = {}\n)", client, shipArch)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, client, shipArch);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETSHIPARCH(client, shipArch);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, client, shipArch);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(ClientId client, float status)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(\n\tClientId client = {}\n\tfloat status = {}\n)", client, status)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, client, status);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETHULLSTATUS(client, status);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, client, status);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(ClientId client, st6::list<XCollision>& collisionGroupList)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(\n\tClientId client = {}\n\tst6::list<XCollisionGroup>& collisionGroupList = {}\n)",
	        client,
	        ToLogString(collisionGroupList))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, client, collisionGroupList);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(client, collisionGroupList);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, client, collisionGroupList);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(ClientId client, st6::vector<EquipDesc>& equipmentVector)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(\n\tClientId client = {}\n\tst6::vector<EquipDesc>& equipmentVector = {}\n)",
	        client,
	        ToLogString(equipmentVector))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, client, equipmentVector);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETEQUIPMENT(client, equipmentVector);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, client, equipmentVector);

	return retVal;
}

void IClientImpl::unknown_26(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_26(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_26(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETADDITEM(ClientId client, FLPACKET_UNKNOWN& _genArg1, FLPACKET_UNKNOWN& _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_SETADDITEM(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n\tFLPACKET_UNKNOWN& _genArg2 = {}\n)",
	        client,
	        ToLogString(_genArg1),
	        ToLogString(_genArg2))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, client, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETADDITEM(client, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, client, _genArg1, _genArg2);

	return retVal;
}

void IClientImpl::unknown_28(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_28(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n\tuint _genArg3 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2,
	        _genArg3)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_28(client, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, client, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETSTARTROOM(client, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, client, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFDESTROYCHARACTER(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFUPDATECHAR(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(\n\tClientId client = {}\n\tuint _genArg1 = "
	                      L"{}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_36(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_36(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_36(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_37(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_37(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_37(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(
	        std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(\n\tClientId client = {}\n\tuint _genArg1 = "
	                      L"{}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(\n\tClientId client = {}\n\tuint _genArg1 = "
	                      L"{}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(\n\tClientId client = {}\n\tuint _genArg1 = "
	                      L"{}\n)",
	        client,
	        _genArg1)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(\n\tClientId client = {}\n\tuint _genArg1 = "
	                      L"{}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(ClientId client, uint reason)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(\n\tClientId client = {}\n\tuint reason = {}\n)", client, reason)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(client, reason);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_44(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_44(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_44(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(\n\tClientId client = {}\n\tuint _genArg1 = "
	                      L"{}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(\n\tClientId client = {}\n\tuint _genArg1 = "
	                      L"{}\n)",
	        client,
	        _genArg1)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(ClientId client, FLPACKET_CREATESOLAR& solar)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(\n\tClientId client = {}\n\tFLPACKET_CREATESOLAR& solar = "
	                      L"{}\n)",
	        client,
	        ToLogString(solar))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, client, solar);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATESOLAR(client, solar);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, client, solar);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATESHIP(ClientId client, FLPACKET_CREATESHIP& ship)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_CREATESHIP(\n\tClientId client = {}\n\tFLPACKET_CREATESHIP& ship = {}\n)", client, ToLogString(ship))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, client, ship);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATESHIP(client, ship);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, client, ship);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATELOOT(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_CREATELOOT(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, client, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATELOOT(client, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, client, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATEMINE(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_CREATEMINE(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, client, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATEMINE(client, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, client, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, client, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATEGUIDED(client, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, client, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, client, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATECOUNTER(client, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, client, _genArg1);

	return retVal;
}

void IClientImpl::unknown_53(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_53(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	CALL_CLIENT_PREAMBLE
	{
		unknown_53(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_54(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_54(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n\tuint _genArg3 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2,
	        _genArg3)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_54(client, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_UPDATEOBJECT(ClientId client, SSPObjUpdateInfo& update)
{
	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, client, update);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_UPDATEOBJECT(client, update);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, client, update);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(ClientId client, FLPACKET_DESTROYOBJECT& destroy)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(\n\tClientId client = {}\n\tFLPACKET_DESTROYOBJECT& destroy = {}\n)",
	        client,
	        ToLogString(destroy))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, client, destroy);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_DESTROYOBJECT(client, destroy);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, client, destroy);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(ClientId client, XActivateEquip& aq)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(
	        std::format(L"IClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(\n\tClientId client = {}\n\tXActivateEquip& aq = {}\n)", client, ToLogString(aq))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, client, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_ACTIVATEOBJECT(client, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, client, aq);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)",
	        client,
	        ToLogString(_genArg1))));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)",
	        client,
	        ToLogString(_genArg1))));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_LAND(ClientId client, FLPACKET_LAND& land)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_LAND(\n\tClientId client = {}\n\tFLPACKET_LAND& land = {}\n)", client, ToLogString(land))));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_LAND(client, land);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_LAUNCH(ClientId client, FLPACKET_LAUNCH& launch)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(
	        std::format(L"IClientImpl::Send_FLPACKET_SERVER_LAUNCH(\n\tClientId client = {}\n\tFLPACKET_LAUNCH& launch = {}\n)", client, ToLogString(launch))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, client, launch);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_LAUNCH(client, launch);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, client, launch);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(ClientId client, bool response, uint shipId)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(\n\tClientId client = {}\n\tbool response = {}\n\tuint shipId = {}\n)",
	        client,
	        response,
	        shipId)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, client, response, shipId);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(client, response, shipId);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, client, response, shipId);

	return retVal;
}

void IClientImpl::unknown_63(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_63(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	CALL_CLIENT_PREAMBLE
	{
		unknown_63(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(ClientId client, uint objId, DamageList& dmgList)
{
	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_DAMAGEOBJECT(client, objId, dmgList);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_ITEMTRACTORED(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_USE_ITEM(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    std::format("IClientImpl::Send_FLPACKET_SERVER_USE_ITEM(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, client, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_USE_ITEM(client, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, client, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(ClientId client, FLPACKET_SETREPUTATION& rep)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(\n\tClientId client = {}\n\tFLPACKET_SETREPUTATION& rep = {}\n)", client, ToLogString(rep))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, client, rep);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETREPUTATION(client, rep);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, client, rep);

	return retVal;
}

void IClientImpl::unknown_68(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_68(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	CALL_CLIENT_PREAMBLE
	{
		unknown_68(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SENDCOMM(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6,
    uint _genArg7, uint _genArg8, uint _genArg9, uint _genArg10, uint _genArg11, uint _genArg12, uint _genArg13, uint _genArg14, uint _genArg15, uint _genArg16,
    uint _genArg17, uint _genArg18, uint _genArg19, uint _genArg20, uint _genArg21, uint _genArg22)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SENDCOMM(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = "
	                      L"{}\n\tuint _genArg3 = {}\n\tuint _genArg4 = {}\n\tuint _genArg5 = {}\n\tuint _genArg6 = {}\n\tuint _genArg7 "
	                      L"= {}\n\tuint _genArg8 = {}\n\tuint _genArg9 = {}\n\tuint _genArg10 = {}\n\tuint _genArg11 = {}\n\tuint "
	                      L"_genArg12 = {}\n\tuint _genArg13 = {}\n\tuint _genArg14 = {}\n\tuint _genArg15 = {}\n\tuint _genArg16 = "
	                      L"{}\n\tuint _genArg17 = {}\n\tuint _genArg18 = {}\n\tuint _genArg19 = {}\n\tuint _genArg20 = {}\n\tuint "
	                      L"_genArg21 = {}\n\tuint _genArg22 = {}\n)",
	        client,
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
	        _genArg22)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM,
	    client,
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
			retVal = Send_FLPACKET_SERVER_SENDCOMM(client,
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
	    client,
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

void IClientImpl::unknown_70(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_70(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_70(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)",
	        client,
	        ToLogString(_genArg1))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, client, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(client, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, client, _genArg1);

	return retVal;
}

void IClientImpl::unknown_72(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_72(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	CALL_CLIENT_PREAMBLE
	{
		unknown_72(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, client, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(client, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, client, _genArg1);

	return retVal;
}

void IClientImpl::unknown_74(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_74(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	CALL_CLIENT_PREAMBLE
	{
		unknown_74(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_75(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_75(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_75(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_MARKOBJ(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_MARKOBJ(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_MARKOBJ(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_77(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_77(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_77(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETCASH(ClientId client, uint cash)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, std::format("IClientImpl::Send_FLPACKET_SERVER_SETCASH(\n\tClientId client = {}\n\tuint cash = {}\n)", client, cash));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, client, cash);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETCASH(client, cash);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, client, cash);

	return retVal;
}

void IClientImpl::unknown_79(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_79(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_79(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_80(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_80(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_80(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_81(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_81(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_81(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_82(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_82(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_82(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_83(ClientId client, char* _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_83(\n\tClientId client = {}\n\tchar* _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_83(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(ClientId client, uint shipId, uint flag, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(\n\tClientId client = {}\n\tuint shipId = {}\n\tuint flag = {}\n\tuint _genArg1 "
	                      L"= {}\n\tuint _genArg2 = {}\n)",
	        client,
	        shipId,
	        flag,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_REQUEST_RETURNED(client, shipId, flag, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_85(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_85(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	CALL_CLIENT_PREAMBLE
	{
		unknown_85(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_86(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_86(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n\tuint _genArg3 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2,
	        _genArg3)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_86(client, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(SObjectCargoUpdate& cargoUpdate, uint iDunno1, uint iDunno2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(\n\tSObjectCargoUpdate client = {}\n\tuint iDunno1 = {}\n\tuint iDunno2 = {}\n)",
	        cargoUpdate.client,
	        iDunno1,
	        iDunno2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(cargoUpdate, iDunno1, iDunno2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_BURNFUSE(ClientId client, FLPACKET_BURNFUSE& burnFuse)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_BURNFUSE(\n\tClientId client = {}\n\tFLPACKET_BURNFUSE& burnFuse = {}\n)", client, ToLogString(burnFuse))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, client, burnFuse);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_BURNFUSE(client, burnFuse);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, client, burnFuse);

	return retVal;
}

void IClientImpl::unknown_89(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_89(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	CALL_CLIENT_PREAMBLE
	{
		unknown_89(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_90(ClientId client)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_90(\n\tClientId client = {}\n)", client));

	CALL_CLIENT_PREAMBLE
	{
		unknown_90(client);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_91(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_91(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_91(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(ClientId client, uint _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    std::format("IClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_WEAPON_GROUP(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(ClientId client, uint objHash, int state)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    std::format(
	        "IClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(\n\tClientId client = {}\n\tuint objHash = {}\n\tint state = {}\n)", client, objHash, state));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_VISITED_STATE(client, objHash, state);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(ClientId client, uint objHash, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(\n\tClientId client = {}\n\tobjHash = {}\n\tint _genArg2 = {}\n)",
	        client,
	        objHash,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_REQUEST_BEST_PATH(client, objHash, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(ClientId client, uint _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_96(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_96(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n\tuint _genArg3 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2,
	        _genArg3)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_96(client, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(ClientId client, uint _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(ClientId client, uint _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_MISSION_LOG(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(ClientId client, uint _genArg1, int _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_INTERFACE_STATE(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_100(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_100(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_100(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_101(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_101(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	CALL_CLIENT_PREAMBLE
	{
		unknown_101(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_102(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_102(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_102(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_103(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_103(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_103(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_104(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_104(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_104(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_105(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_105(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_105(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_106(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_106(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_106(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_107(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_107(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_107(client, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1)));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_PLAYER_TRADE(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_109(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_109(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_109(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, client, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SCANNOTIFY(client, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, client, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(ClientId client, wchar_t* characterName, uint _genArg2, char _genArg3)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(\n\tClientId client = {}\n\twchar_t* characterName = \n\tuint _genArg2 = {}\n\tchar "
	                      L"_genArg3 = {}\n)",
	        client,
	        std::wstring(characterName),
	        _genArg2,
	        ToLogString(_genArg3))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, client, characterName, _genArg2, _genArg3);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_PLAYERLIST(client, characterName, _genArg2, _genArg3);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, client, characterName, _genArg2, _genArg3);

	return retVal;
}

void IClientImpl::unknown_112(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_112(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_112(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(ClientId client)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(\n\tClientId client = {}\n)", client));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, client);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_PLAYERLIST_2(client);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, client);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, client, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_6(client, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, client, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, client, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_7(client, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, client, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(\n\tClientId client = {}\n\tFLPACKET_UNKNOWN& _genArg1 = {}\n)", client, ToLogString(_genArg1))));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, client, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE(client, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, client, _genArg1);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, client, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_2(client, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, client, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(ClientId client, uint targetId, uint rank)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(
	        L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(\n\tClientId client = {}\n\tuint targetId = {}\n\tuint rank = {}\n)", client, targetId, rank)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, client, targetId, rank);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_3(client, targetId, rank);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, client, targetId, rank);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint "
	                      L"_genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, client, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_4(client, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, client, _genArg1, _genArg2);

	return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(ClientId client, uint _genArg1, uint _genArg2)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2)));

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, client, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_5(client, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, client, _genArg1, _genArg2);

	return retVal;
}

void IClientImpl::unknown_121(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_121(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_121(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(ClientId client, uint shipId, Vector& formationOffset)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(\n\tClientId client = {}\n\tuint shipId = {}\n\tVector& formationOffset = {}\n)",
	        client,
	        shipId,
	        ToLogString(formationOffset))));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_FORMATION_UPDATE(client, shipId, formationOffset);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void IClientImpl::unknown_123(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6)
{
	AddLog(LogType::Normal,
	    LogLevel::Debug,
	    wstos(std::format(L"IClientImpl::unknown_123(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n\tuint "
	                      L"_genArg3 = {}\n\tuint _genArg4 = {}\n\tuint _genArg5 = {}\n\tuint _genArg6 = {}\n)",
	        client,
	        _genArg1,
	        _genArg2,
	        _genArg3,
	        _genArg4,
	        _genArg5,
	        _genArg6)));

	CALL_CLIENT_PREAMBLE
	{
		unknown_123(client, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_124(ClientId client)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_124(\n\tClientId client = {}\n)", client));

	CALL_CLIENT_PREAMBLE
	{
		unknown_124(client);
	}
	CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_125(ClientId client, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_125(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_125(client, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

int IClientImpl::unknown_126(char* _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, std::format("IClientImpl::unknown_126(\n\tchar* _genArg1 = {}\n)", _genArg1));

	int retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = unknown_126(_genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

namespace IServerImplHook
{
	void __stdcall FireWeapon(ClientId client, XFireWeaponInfo const& fwi)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"FireWeapon(\n\tClientId client = {}\n\tXFireWeaponInfo const& fwi = {}\n)", client, ToLogString(fwi))));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__FireWeapon, client, fwi);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.FireWeapon(client, fwi);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__FireWeapon, client, fwi);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ActivateEquip(ClientId client, XActivateEquip const& aq)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"ActivateEquip(\n\tClientId client = {}\n\tXActivateEquip const& aq = {}\n)", client, ToLogString(aq))));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateEquip, client, aq);

		CHECK_FOR_DISCONNECT;

		ActivateEquip__Inner(client, aq);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ActivateEquip(client, aq);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateEquip, client, aq);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ActivateCruise(ClientId client, XActivateCruise const& ac)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"ActivateCruise(\n\tClientId client = {}\n\tXActivateCruise const& ac = {}\n)", client, ToLogString(ac))));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateCruise, client, ac);

		CHECK_FOR_DISCONNECT;

		ActivateCruise__Inner(client, ac);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ActivateCruise(client, ac);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateCruise, client, ac);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ActivateThrusters(ClientId client, XActivateThrusters const& at)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"ActivateThrusters(\n\tClientId client = {}\n\tXActivateThrusters const& at = {}\n)", client, ToLogString(at))));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateThrusters, client, at);

		CHECK_FOR_DISCONNECT;

		ActivateThrusters__Inner(client, at);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ActivateThrusters(client, at);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateThrusters, client, at);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetTarget(ClientId client, XSetTarget const& st)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"SetTarget(\n\tClientId client = {}\n\tXSetTarget const& st = {}\n)", client, ToLogString(st))));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTarget, client, st); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetTarget(client, st);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetTarget, client, st);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall TractorObjects(ClientId client, XTractorObjects const& to)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"TractorObjects(\n\tClientId client = {}\n\tXTractorObjects const& to = {}\n)", client, ToLogString(to))));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TractorObjects, client, to); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.TractorObjects(client, to);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__TractorObjects, client, to);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall GoTradelane(ClientId client, XGoTradelane const& gt)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"GoTradelane(\n\tClientId client = {}\n\tXGoTradelane const& gt = {}\n)", client, ToLogString(gt))));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GoTradelane, client, gt);

		GoTradelane__Inner(client, gt);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GoTradelane(client, gt);
			}
			CALL_SERVER_POSTAMBLE(GoTradelane__Catch(client, gt), );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GoTradelane, client, gt);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall StopTradelane(ClientId client, uint shipId, uint tradelaneRing1, uint tradelaneRing2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"StopTradelane(\n\tClientId client = {}\n\tuint shipId = {}\n\tuint tradelaneRing1 = {}\n\tuint tradelaneRing2 = {}\n)",
		        client,
		        shipId,
		        tradelaneRing1,
		        tradelaneRing2)));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradelane, client, shipId, tradelaneRing1, tradelaneRing2);

		StopTradelane__Inner(client, shipId, tradelaneRing1, tradelaneRing2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.StopTradelane(client, shipId, tradelaneRing1, tradelaneRing2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__StopTradelane, client, shipId, tradelaneRing1, tradelaneRing2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall JettisonCargo(ClientId client, XJettisonCargo const& jc)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"JettisonCargo(\n\tClientId client = {}\n\tXJettisonCargo const& jc = {}\n)", client, ToLogString(jc))));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JettisonCargo, client, jc); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.JettisonCargo(client, jc);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__JettisonCargo, client, jc);
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
			CALL_SERVER_PREAMBLE
			{
				retVal = Server.Startup(si);
			}
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
		AddLog(LogType::Normal, LogLevel::Debug, "Shutdown()");


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Shutdown); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.Shutdown();
			}
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
			CALL_SERVER_PREAMBLE
			{
				retVal = Server.Update();
			}
			CALL_SERVER_POSTAMBLE(true, int());
		}

		CallPluginsAfter(HookedCall::IServerImpl__Update);

		return retVal;
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall DisConnect(ClientId client, EFLConnection conn)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"DisConnect(\n\tClientId client = {}\n\tEFLConnection conn = {}\n)", client, ToLogString(conn))));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DisConnect, client, conn);

		DisConnect__Inner(client, conn);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.DisConnect(client, conn);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DisConnect, client, conn);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall OnConnect(ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, wstos(std::format(L"OnConnect(\n\tClientId client = {}\n)", client)));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__OnConnect, client);

		if (bool innerCheck = OnConnect__Inner(client); 
			!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.OnConnect(client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		OnConnect__InnerAfter(client);

		CallPluginsAfter(HookedCall::IServerImpl__OnConnect, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall Login(SLoginInfo const& li, ClientId client)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, wstos(std::format(L"Login(\n\tSLoginInfo const& li = {}\n\tClientId client = {}\n)", ToLogString(li), client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Login, li, client); 
			!skip && Login__InnerBefore(li, client))
		{
			CALL_SERVER_PREAMBLE
			{
				Server.Login(li, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		Login__InnerAfter(li, client);

		if (TempBanManager::i()->CheckIfTempBanned(client))
		{
			Hk::Player::Kick(client);
			return;
		}

		CallPluginsAfter(HookedCall::IServerImpl__Login, li, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall CharacterInfoReq(ClientId client, bool _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, wstos(std::format(L"CharacterInfoReq(\n\tClientId client = {}\n\tbool _genArg1 = {}\n)", client, _genArg1)));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterInfoReq, client, _genArg1);

		CHECK_FOR_DISCONNECT;

		if (bool innerCheck = CharacterInfoReq__Inner(client, _genArg1); 
			!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.CharacterInfoReq(client, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(CharacterInfoReq__Catch(client, _genArg1), );
		}

		CallPluginsAfter(HookedCall::IServerImpl__CharacterInfoReq, client, _genArg1);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall CharacterSelect(CHARACTER_ID const& cid, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"CharacterSelect(\n\tCHARACTER_ID const& cid = {}\n\tClientId client = {}\n)", ToLogString(cid), client)));

		std::string charName = cid.szCharFilename;
		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterSelect, charName, client);

		CHECK_FOR_DISCONNECT;

		if (bool innerCheck = CharacterSelect__Inner(cid, client); 
			!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.CharacterSelect(cid, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		CharacterSelect__InnerAfter(cid, client);

		CallPluginsAfter(HookedCall::IServerImpl__CharacterSelect, charName, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall CreateNewCharacter(SCreateCharacterInfo const& _genArg1, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"CreateNewCharacter(\n\tSCreateCharacterInfo const& _genArg1 = {}\n\tClientId client = {}\n)", ToLogString(_genArg1), client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CreateNewCharacter, _genArg1, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.CreateNewCharacter(_genArg1, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__CreateNewCharacter, _genArg1, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall DestroyCharacter(CHARACTER_ID const& _genArg1, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"DestroyCharacter(\n\tCHARACTER_ID const& _genArg1 = {}\n\tClientId client = {}\n)", ToLogString(_genArg1), client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DestroyCharacter, _genArg1, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.DestroyCharacter(_genArg1, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DestroyCharacter, _genArg1, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqShipArch(uint archId, ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, wstos(std::format(L"ReqShipArch(\n\tuint archId = {}\n\tClientId client = {}\n)", archId, client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqShipArch, archId, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqShipArch(archId, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqShipArch, archId, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqHullStatus(float status, ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, wstos(std::format(L"ReqHullStatus(\n\tfloat status = {}\n\tClientId client = {}\n)", status, client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqHullStatus, status, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqHullStatus(status, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqHullStatus, status, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqCollisionGroups(st6::list<CollisionGroupDesc> const& collisionGroups, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"ReqCollisionGroups(\n\tst6::list<CollisionGroupDesc> const& CollisionGroups = {}\n\tClientId client = {}\n)",
		        ToLogString(collisionGroups),
		        client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqCollisionGroups, collisionGroups, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqCollisionGroups(collisionGroups, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqCollisionGroups, collisionGroups, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqEquipment(EquipDescList const& edl, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"ReqEquipment(\n\tEquipDescList const& edl = {}\n\tClientId client = {}\n)", ToLogString(edl), client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqEquipment, edl, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqEquipment(edl, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqEquipment, edl, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqAddItem(uint goodId, char const* hardpoint, int count, float status, bool mounted, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"ReqAddItem(\n\tuint goodId = {}\n\tchar const* hardpoint = {}\n\tint count = {}\n\tfloat status = "
		                      L"{}\n\tbool mounted = {}\n\tClientId client = {}\n)",
		        goodId,
		        stows(std::string(hardpoint)),
		        count,
		        status,
		        mounted,
		        client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqAddItem, goodId, hardpoint, count, status, mounted, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqAddItem(goodId, hardpoint, count, status, mounted, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqAddItem, goodId, hardpoint, count, status, mounted, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqRemoveItem(ushort slotId, int count, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("ReqRemoveItem(\n\tushort slotId = {}\n\tint count = {}\n\tClientId client = {}\n)", slotId, count, client));

		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqRemoveItem, slotId, count, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqRemoveItem(slotId, count, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqRemoveItem, slotId, count, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqModifyItem(ushort slotId, char const* hardpoint, int count, float status, bool mounted, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("ReqModifyItem(\n\tushort slotId = {}\n\tchar const* hardpoint = {}\n\tint count = {}\n\tfloat status = "
		                "{}\n\tbool mounted = {}\n\tClientId client = {}\n)",
		        slotId,
		        std::string(hardpoint),
		        count,
		        status,
		        mounted,
		        client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqModifyItem, slotId, hardpoint, count, status, mounted, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqModifyItem(slotId, hardpoint, count, status, mounted, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqModifyItem, slotId, hardpoint, count, status, mounted, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqSetCash(int cash, ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("ReqSetCash(\n\tint cash = {}\n\tClientId client = {}\n)", cash, client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqSetCash, cash, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqSetCash(cash, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqSetCash, cash, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall ReqChangeCash(int cashAdd, ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("ReqChangeCash(\n\tint cashAdd = {}\n\tClientId client = {}\n)", cashAdd, client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqChangeCash, cashAdd, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqChangeCash(cashAdd, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqChangeCash, cashAdd, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall BaseEnter(uint baseId, ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("BaseEnter(\n\tuint baseId = {}\n\tClientId client = {}\n)", baseId, client));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseEnter, baseId, client);

		CHECK_FOR_DISCONNECT;

		BaseEnter__Inner(baseId, client);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.BaseEnter(baseId, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		BaseEnter__InnerAfter(baseId, client);

		CallPluginsAfter(HookedCall::IServerImpl__BaseEnter, baseId, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall BaseExit(uint baseId, ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("BaseExit(\n\tuint baseId = {}\n\tClientId client = {}\n)", baseId, client));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseExit, baseId, client);

		CHECK_FOR_DISCONNECT;

		BaseExit__Inner(baseId, client);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.BaseExit(baseId, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		BaseExit__InnerAfter(baseId, client);

		CallPluginsAfter(HookedCall::IServerImpl__BaseExit, baseId, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall LocationEnter(uint locationId, ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("LocationEnter(\n\tuint locationId = {}\n\tClientId client = {}\n)", locationId, client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationEnter, locationId, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.LocationEnter(locationId, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationEnter, locationId, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall LocationExit(uint locationId, ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("LocationExit(\n\tuint locationId = {}\n\tClientId client = {}\n)", locationId, client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationExit, locationId, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.LocationExit(locationId, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationExit, locationId, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall BaseInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("BaseInfoRequest(\n\tunsigned int _genArg1 = {}\n\tunsigned int _genArg2 = {}\n\tbool _genArg3 = {}\n)", _genArg1, _genArg2, _genArg3));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseInfoRequest, _genArg1, _genArg2, _genArg3); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.BaseInfoRequest(_genArg1, _genArg2, _genArg3);
			}
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
		    std::format(
		        "LocationInfoRequest(\n\tunsigned int _genArg1 = {}\n\tunsigned int _genArg2 = {}\n\tbool _genArg3 = {}\n)", _genArg1, _genArg2, _genArg3));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationInfoRequest, _genArg1, _genArg2, _genArg3); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.LocationInfoRequest(_genArg1, _genArg2, _genArg3);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationInfoRequest, _genArg1, _genArg2, _genArg3);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall GFObjSelect(unsigned int _genArg1, unsigned int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, std::format("GFObjSelect(\n\tunsigned int _genArg1 = {}\n\tunsigned int _genArg2 = {}\n)", _genArg1, _genArg2));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFObjSelect, _genArg1, _genArg2); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFObjSelect(_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFObjSelect, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall GFGoodVaporized(SGFGoodVaporizedInfo const& gvi, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"GFGoodVaporized(\n\tSGFGoodVaporizedInfo const& gvi = {}\n\tClientId client = {}\n)", ToLogString(gvi), client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodVaporized, gvi, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFGoodVaporized(gvi, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodVaporized, gvi, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall MissionResponse(unsigned int _genArg1, unsigned long _genArg2, bool _genArg3, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("MissionResponse(\n\tunsigned int _genArg1 = {}\n\tunsigned long _genArg2 = {}\n\tbool _genArg3 = "
		                "{}\n\tClientId client = {}\n)",
		        _genArg1,
		        _genArg2,
		        _genArg3,
		        client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__MissionResponse, _genArg1, _genArg2, _genArg3, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.MissionResponse(_genArg1, _genArg2, _genArg3, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__MissionResponse, _genArg1, _genArg2, _genArg3, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall TradeResponse(unsigned char const* _genArg1, int _genArg2, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("TradeResponse(\n\tunsigned char const* _genArg1 = {}\n\tint _genArg2 = {}\n\tClientId client = {}\n)",
		        std::string(reinterpret_cast<char const*>(_genArg1)),
		        _genArg2,
		        client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TradeResponse, _genArg1, _genArg2, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.TradeResponse(_genArg1, _genArg2, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__TradeResponse, _genArg1, _genArg2, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall GFGoodBuy(SGFGoodBuyInfo const& _genArg1, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"GFGoodBuy(\n\tSGFGoodBuyInfo const& _genArg1 = {}\n\tClientId client = {}\n)", ToLogString(_genArg1), client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodBuy, _genArg1, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFGoodBuy(_genArg1, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodBuy, _genArg1, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall GFGoodSell(SGFGoodSellInfo const& _genArg1, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"GFGoodSell(\n\tSGFGoodSellInfo const& _genArg1 = {}\n\tClientId client = {}\n)", ToLogString(_genArg1), client)));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodSell, _genArg1, client);

		CHECK_FOR_DISCONNECT;

		if (bool innerCheck = GFGoodSell__Inner(_genArg1, client); 
			!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFGoodSell(_genArg1, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodSell, _genArg1, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SystemSwitchOutComplete(uint shipId, ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("SystemSwitchOutComplete(\n\tuint shipId = {}\n\tClientId client = {}\n)", shipId, client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SystemSwitchOutComplete, shipId, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SystemSwitchOutComplete(shipId, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		SystemSwitchOutComplete__InnerAfter(shipId, client);

		CallPluginsAfter(HookedCall::IServerImpl__SystemSwitchOutComplete, shipId, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall PlayerLaunch(uint shipId, ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("PlayerLaunch(\n\tuint shipId = {}\n\tClientId client = {}\n)", shipId, client));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PlayerLaunch, shipId, client);

		CHECK_FOR_DISCONNECT;

		PlayerLaunch__Inner(shipId, client);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.PlayerLaunch(shipId, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		PlayerLaunch__InnerAfter(shipId, client);

		CallPluginsAfter(HookedCall::IServerImpl__PlayerLaunch, shipId, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall LaunchComplete(uint baseId, uint shipId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("LaunchComplete(\n\tuint baseId = {}\n\tuint shipId = {}\n)", baseId, shipId));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LaunchComplete, baseId, shipId);

		LaunchComplete__Inner(baseId, shipId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.LaunchComplete(baseId, shipId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LaunchComplete, baseId, shipId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall JumpInComplete(uint systemId, uint shipId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("JumpInComplete(\n\tuint systemId = {}\n\tuint shipId = {}\n)", systemId, shipId));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JumpInComplete, systemId, shipId); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.JumpInComplete(systemId, shipId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		JumpInComplete__InnerAfter(systemId, shipId);

		CallPluginsAfter(HookedCall::IServerImpl__JumpInComplete, systemId, shipId);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall Hail(unsigned int _genArg1, unsigned int _genArg2, unsigned int _genArg3)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("Hail(\n\tunsigned int _genArg1 = {}\n\tunsigned int _genArg2 = {}\n\tunsigned int _genArg3 = {}\n)", _genArg1, _genArg2, _genArg3));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Hail, _genArg1, _genArg2, _genArg3); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.Hail(_genArg1, _genArg2, _genArg3);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__Hail, _genArg1, _genArg2, _genArg3);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPObjUpdate(SSPObjUpdateInfo const& ui, ClientId client)
	{
		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjUpdate, ui, client);

		CHECK_FOR_DISCONNECT;

		if (bool innerCheck = SPObjUpdate__Inner(ui, client); 
			!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPObjUpdate(ui, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPObjUpdate, ui, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPMunitionCollision(SSPMunitionCollisionInfo const& mci, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"SPMunitionCollision(\n\tSSPMunitionCollisionInfo const& mci = {}\n\tClientId client = {}\n)", ToLogString(mci), client)));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPMunitionCollision, mci, client);

		CHECK_FOR_DISCONNECT;

		SPMunitionCollision__Inner(mci, client);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPMunitionCollision(mci, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPMunitionCollision, mci, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPObjCollision(SSPObjCollisionInfo const& oci, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"SPObjCollision(\n\tSSPObjCollisionInfo const& oci = {}\n\tClientId client = {}\n)", ToLogString(oci), client)));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjCollision, oci, client);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPObjCollision(oci, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPObjCollision, oci, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPRequestUseItem(SSPUseItem const& ui, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"SPRequestUseItem(\n\tSSPUseItem const& ui = {}\n\tClientId client = {}\n)", ToLogString(ui), client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestUseItem, ui, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPRequestUseItem(ui, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPRequestUseItem, ui, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPRequestInvincibility(uint shipId, bool enable, InvincibilityReason reason, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"SPRequestInvincibility(\n\tuint shipId = {}\n\tbool enable = {}\n\tInvincibilityReason reason = "
		                      L"{}\n\tClientId client = {}\n)",
		        shipId,
		        enable,
		        ToLogString(reason),
		        client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestInvincibility, shipId, enable, reason, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPRequestInvincibility(shipId, enable, reason, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPRequestInvincibility, shipId, enable, reason, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestEvent(int eventType, uint shipId, uint dockTarget, uint _genArg1, ulong _genArg2, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("RequestEvent(\n\tint eventType = {}\n\tuint shipId = {}\n\tuint dockTarget = {}\n\tuint _genArg1 = {}\n\tulong _genArg2 = "
		                "{}\n\tClientId client = {}\n)",
		        eventType,
		        shipId,
		        dockTarget,
		        _genArg1,
		        _genArg2,
		        client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestEvent, eventType, shipId, dockTarget, _genArg1, _genArg2, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestEvent(eventType, shipId, dockTarget, _genArg1, _genArg2, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestEvent, eventType, shipId, dockTarget, _genArg1, _genArg2, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestCancel(int eventType, uint shipId, uint _genArg1, ulong _genArg2, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("RequestCancel(\n\tint eventType = {}\n\tuint shipId = {}\n\tuint _genArg1 = {}\n\tulong _genArg2 = {}\n\tClientId client = {}\n)",
		        eventType,
		        shipId,
		        _genArg1,
		        _genArg2,
		        client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCancel, eventType, shipId, _genArg1, _genArg2, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestCancel(eventType, shipId, _genArg1, _genArg2, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestCancel, eventType, shipId, _genArg1, _genArg2, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall MineAsteroid(uint systemId, Vector const& pos, uint crateId, uint lootId, uint count, ClientId client)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"MineAsteroid(\n\tuint systemId = {}\n\tVector const& pos = {}\n\tuint crateId = {}\n\tuint lootId = {}\n\tuint count = "
		                      L"{}\n\tClientId client = {}\n)",
		        systemId,
		        ToLogString(pos),
		        crateId,
		        lootId,
		        count,
		        client)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__MineAsteroid, systemId, pos, crateId, lootId, count, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.MineAsteroid(systemId, pos, crateId, lootId, count, client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__MineAsteroid, systemId, pos, crateId, lootId, count, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestCreateShip(ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("RequestCreateShip(\n\tClientId client = {}\n)", client));

		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCreateShip, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestCreateShip(client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestCreateShip, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SPScanCargo(uint const& _genArg1, uint const& _genArg2, uint _genArg3)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"SPScanCargo(\n\tuint const& _genArg1 = {}\n\tuint const& _genArg2 = {}\n\tuint _genArg3 = {}\n)",
		        ToLogString(_genArg1),
		        ToLogString(_genArg2),
		        _genArg3)));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPScanCargo, _genArg1, _genArg2, _genArg3); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPScanCargo(_genArg1, _genArg2, _genArg3);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPScanCargo, _genArg1, _genArg2, _genArg3);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetManeuver(ClientId client, XSetManeuver const& sm)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"SetManeuver(\n\tClientId client = {}\n\tXSetManeuver const& sm = {}\n)", client, ToLogString(sm))));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetManeuver, client, sm); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetManeuver(client, sm);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetManeuver, client, sm);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall InterfaceItemUsed(uint _genArg1, uint _genArg2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("InterfaceItemUsed(\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", _genArg1, _genArg2));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InterfaceItemUsed, _genArg1, _genArg2); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.InterfaceItemUsed(_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__InterfaceItemUsed, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall AbortMission(ClientId client, uint _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("AbortMission(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AbortMission, client, _genArg1); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.AbortMission(client, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AbortMission, client, _genArg1);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetWeaponGroup(ClientId client, uint _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("SetWeaponGroup(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetWeaponGroup, client, _genArg1, _genArg2); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetWeaponGroup(client, (uchar*)_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetWeaponGroup, client, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetVisitedState(ClientId client, uint objHash, int state)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("SetVisitedState(\n\tClientId client = {}\n\tuint objHash = {}\n\tint state = {}\n)", client, objHash, state));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetVisitedState, client, objHash, state); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetVisitedState(client, (uchar*)objHash, state);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetVisitedState, client, objHash, state);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestBestPath(ClientId client, uint _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("RequestBestPath(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestBestPath, client, _genArg1, _genArg2); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestBestPath(client, (uchar*)_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestBestPath, client, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestPlayerStats(ClientId client, uint _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("RequestPlayerStats(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestPlayerStats, client, _genArg1, _genArg2); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestPlayerStats(client, (uchar*)_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestPlayerStats, client, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall PopupDialog(ClientId client, uint buttonClicked)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("PopupDialog(\n\tClientId client = {}\n\tuint buttonClicked = {}\n)", client, buttonClicked));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PopupDialog, client, buttonClicked); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.PopUpDialog(client, buttonClicked);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__PopupDialog, client, buttonClicked);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestGroupPositions(ClientId client, uint _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("RequestGroupPositions(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestGroupPositions, client, _genArg1, _genArg2); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestGroupPositions(client, (uchar*)_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestGroupPositions, client, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetInterfaceState(ClientId client, uint _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("SetInterfaceState(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetInterfaceState, client, _genArg1, _genArg2); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetInterfaceState(client, (uchar*)_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetInterfaceState, client, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestRankLevel(ClientId client, uint _genArg1, int _genArg2)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("RequestRankLevel(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestRankLevel, client, _genArg1, _genArg2); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestRankLevel(client, (uchar*)_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestRankLevel, client, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall InitiateTrade(ClientId client1, ClientId client2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("InitiateTrade(\n\tClientId client1 = {}\n\tClientId client2 = {}\n)", client1, client2));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InitiateTrade, client1, client2);

		InitiateTrade__Inner(client1, client2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.InitiateTrade(client1, client2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__InitiateTrade, client1, client2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall TerminateTrade(ClientId client, int accepted)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("TerminateTrade(\n\tClientId client = {}\n\tint accepted = {}\n)", client, accepted));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TerminateTrade, client, accepted);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.TerminateTrade(client, accepted);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		TerminateTrade__InnerAfter(client, accepted);

		CallPluginsAfter(HookedCall::IServerImpl__TerminateTrade, client, accepted);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall AcceptTrade(ClientId client, bool _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("AcceptTrade(\n\tClientId client = {}\n\tbool _genArg1 = {}\n)", client, _genArg1));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AcceptTrade, client, _genArg1); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.AcceptTrade(client, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AcceptTrade, client, _genArg1);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SetTradeMoney(ClientId client, ulong _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("SetTradeMoney(\n\tClientId client = {}\n\tulong _genArg1 = {}\n)", client, _genArg1));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTradeMoney, client, _genArg1); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetTradeMoney(client, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetTradeMoney, client, _genArg1);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall AddTradeEquip(ClientId client, EquipDesc const& ed)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"AddTradeEquip(\n\tClientId client = {}\n\tEquipDesc const& ed = {}\n)", client, ToLogString(ed))));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AddTradeEquip, client, ed); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.AddTradeEquip(client, ed);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AddTradeEquip, client, ed);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall DelTradeEquip(ClientId client, EquipDesc const& ed)
	{
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    wstos(std::format(L"DelTradeEquip(\n\tClientId client = {}\n\tEquipDesc const& ed = {}\n)", client, ToLogString(ed))));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DelTradeEquip, client, ed); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.DelTradeEquip(client, ed);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DelTradeEquip, client, ed);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall RequestTrade(uint _genArg1, uint _genArg2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("RequestTrade(\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", _genArg1, _genArg2));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestTrade, _genArg1, _genArg2); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestTrade(_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestTrade, _genArg1, _genArg2);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall StopTradeRequest(ClientId client)
	{
		AddLog(LogType::Normal, LogLevel::Debug, std::format("StopTradeRequest(\n\tClientId client = {}\n)", client));


		if (auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradeRequest, client); 
			!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.StopTradeRequest(client);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__StopTradeRequest, client);
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall Dock([[maybe_unused]] uint const& _genArg1, [[maybe_unused]] uint const& _genArg2)
	{
	}
} // namespace IServerImplHook

namespace IServerImplHook
{
	void __stdcall SubmitChat(CHAT_ID cidFrom, ulong size, void const* rdlReader, CHAT_ID cidTo, int _genArg1)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, std::format("SubmitChat(\n\tuint From = {}\n\tulong size = {}\n\tuint cidTo = {}", cidFrom.iId, size, cidTo.iId));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SubmitChat, cidFrom.iId, size, rdlReader, cidTo.iId, _genArg1);

		bool innerCheck = SubmitChat__Inner(cidFrom, size, rdlReader, cidTo, _genArg1);
		if (!innerCheck)
			return;
		g_InSubmitChat = true;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SubmitChat(cidFrom, size, rdlReader, cidTo, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		g_InSubmitChat = false;

		CallPluginsAfter(HookedCall::IServerImpl__SubmitChat, cidFrom.iId, size, rdlReader, cidTo.iId, _genArg1);
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
	setProps(HookedCall::IEngine__CShip__Init, true, false);
	setProps(HookedCall::IEngine__CShip__Destroy, true, false);
	setProps(HookedCall::IEngine__UpdateTime, true, true);
	setProps(HookedCall::IEngine__ElapseTime, true, true);
	setProps(HookedCall::IEngine__DockCall, true, false);
	setProps(HookedCall::IEngine__LaunchPosition, true, false);
	setProps(HookedCall::IEngine__ShipDestroyed, true, false);
	setProps(HookedCall::IEngine__BaseDestroyed, true, false);
	setProps(HookedCall::IEngine__GuidedHit, true, false);
	setProps(HookedCall::IEngine__AddDamageEntry, true, true);
	setProps(HookedCall::IEngine__DamageHit, true, false);
	setProps(HookedCall::IEngine__AllowPlayerDamage, true, false);
	setProps(HookedCall::IEngine__SendDeathMessage, true, false);
	setProps(HookedCall::FLHook__TimerCheckKick, true, false);
	setProps(HookedCall::FLHook__TimerNPCAndF1Check, true, false);
	setProps(HookedCall::FLHook__UserCommand__Process, true, false);
	setProps(HookedCall::FLHook__AdminCommand__Help, true, true);
	setProps(HookedCall::FLHook__AdminCommand__Process, true, false);
	setProps(HookedCall::FLHook__LoadSettings, true, true);
	setProps(HookedCall::FLHook__LoadCharacterSettings, true, true);
	setProps(HookedCall::FLHook__ClearClientInfo, true, true);
	setProps(HookedCall::FLHook__ProcessEvent, true, false);
	setProps(HookedCall::IChat__SendChat, true, false);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, true, true);
	setProps(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, true, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, true, true);
	setProps(HookedCall::IServerImpl__FireWeapon, true, true);
	setProps(HookedCall::IServerImpl__ActivateEquip, true, true);
	setProps(HookedCall::IServerImpl__ActivateCruise, true, true);
	setProps(HookedCall::IServerImpl__ActivateThrusters, true, true);
	setProps(HookedCall::IServerImpl__SetTarget, true, true);
	setProps(HookedCall::IServerImpl__TractorObjects, true, true);
	setProps(HookedCall::IServerImpl__GoTradelane, true, true);
	setProps(HookedCall::IServerImpl__StopTradelane, true, true);
	setProps(HookedCall::IServerImpl__JettisonCargo, true, true);
	setProps(HookedCall::IServerImpl__Startup, true, true);
	setProps(HookedCall::IServerImpl__Shutdown, true, false);
	setProps(HookedCall::IServerImpl__Update, true, true);
	setProps(HookedCall::IServerImpl__DisConnect, true, true);
	setProps(HookedCall::IServerImpl__OnConnect, true, true);
	setProps(HookedCall::IServerImpl__Login, true, true);
	setProps(HookedCall::IServerImpl__CharacterInfoReq, true, true);
	setProps(HookedCall::IServerImpl__CharacterSelect, true, true);
	setProps(HookedCall::IServerImpl__CreateNewCharacter, true, true);
	setProps(HookedCall::IServerImpl__DestroyCharacter, true, true);
	setProps(HookedCall::IServerImpl__ReqShipArch, true, true);
	setProps(HookedCall::IServerImpl__ReqHullStatus, true, true);
	setProps(HookedCall::IServerImpl__ReqCollisionGroups, true, true);
	setProps(HookedCall::IServerImpl__ReqEquipment, true, true);
	setProps(HookedCall::IServerImpl__ReqAddItem, true, true);
	setProps(HookedCall::IServerImpl__ReqRemoveItem, true, true);
	setProps(HookedCall::IServerImpl__ReqModifyItem, true, true);
	setProps(HookedCall::IServerImpl__ReqSetCash, true, true);
	setProps(HookedCall::IServerImpl__ReqChangeCash, true, true);
	setProps(HookedCall::IServerImpl__BaseEnter, true, true);
	setProps(HookedCall::IServerImpl__BaseExit, true, true);
	setProps(HookedCall::IServerImpl__LocationEnter, true, true);
	setProps(HookedCall::IServerImpl__LocationExit, true, true);
	setProps(HookedCall::IServerImpl__BaseInfoRequest, true, true);
	setProps(HookedCall::IServerImpl__LocationInfoRequest, true, true);
	setProps(HookedCall::IServerImpl__GFObjSelect, true, true);
	setProps(HookedCall::IServerImpl__GFGoodVaporized, true, true);
	setProps(HookedCall::IServerImpl__MissionResponse, true, true);
	setProps(HookedCall::IServerImpl__TradeResponse, true, true);
	setProps(HookedCall::IServerImpl__GFGoodBuy, true, true);
	setProps(HookedCall::IServerImpl__GFGoodSell, true, true);
	setProps(HookedCall::IServerImpl__SystemSwitchOutComplete, true, true);
	setProps(HookedCall::IServerImpl__PlayerLaunch, true, true);
	setProps(HookedCall::IServerImpl__LaunchComplete, true, true);
	setProps(HookedCall::IServerImpl__JumpInComplete, true, true);
	setProps(HookedCall::IServerImpl__Hail, true, true);
	setProps(HookedCall::IServerImpl__SPObjUpdate, true, true);
	setProps(HookedCall::IServerImpl__SPMunitionCollision, true, true);
	setProps(HookedCall::IServerImpl__SPObjCollision, true, true);
	setProps(HookedCall::IServerImpl__SPRequestUseItem, true, true);
	setProps(HookedCall::IServerImpl__SPRequestInvincibility, true, true);
	setProps(HookedCall::IServerImpl__RequestEvent, true, true);
	setProps(HookedCall::IServerImpl__RequestCancel, true, true);
	setProps(HookedCall::IServerImpl__MineAsteroid, true, true);
	setProps(HookedCall::IServerImpl__RequestCreateShip, true, true);
	setProps(HookedCall::IServerImpl__SPScanCargo, true, true);
	setProps(HookedCall::IServerImpl__SetManeuver, true, true);
	setProps(HookedCall::IServerImpl__InterfaceItemUsed, true, true);
	setProps(HookedCall::IServerImpl__AbortMission, true, true);
	setProps(HookedCall::IServerImpl__SetWeaponGroup, true, true);
	setProps(HookedCall::IServerImpl__SetVisitedState, true, true);
	setProps(HookedCall::IServerImpl__RequestBestPath, true, true);
	setProps(HookedCall::IServerImpl__RequestPlayerStats, true, true);
	setProps(HookedCall::IServerImpl__PopupDialog, true, true);
	setProps(HookedCall::IServerImpl__RequestGroupPositions, true, true);
	setProps(HookedCall::IServerImpl__SetInterfaceState, true, true);
	setProps(HookedCall::IServerImpl__RequestRankLevel, true, true);
	setProps(HookedCall::IServerImpl__InitiateTrade, true, true);
	setProps(HookedCall::IServerImpl__TerminateTrade, true, true);
	setProps(HookedCall::IServerImpl__AcceptTrade, true, true);
	setProps(HookedCall::IServerImpl__SetTradeMoney, true, true);
	setProps(HookedCall::IServerImpl__AddTradeEquip, true, true);
	setProps(HookedCall::IServerImpl__DelTradeEquip, true, true);
	setProps(HookedCall::IServerImpl__RequestTrade, true, true);
	setProps(HookedCall::IServerImpl__StopTradeRequest, true, true);
	setProps(HookedCall::IServerImpl__SubmitChat, true, true);
}