// IP Ban plugin - Feb 2010 by Cannon
// Ported by Raikkonen 2022

// Includes
#include "Main.h"

namespace Plugins::IPBan
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/// Return true if this client is on a banned IP range.
	static bool IsBanned(uint iClientID)
	{
		std::wstring wscIP;
		HkGetPlayerIP(iClientID, wscIP);
		std::string scIP = wstos(wscIP);

		// Check for an IP range match.
		for (auto& ban : global->ipBans.Bans)
			if (Wildcard::Fit(ban.c_str(), scIP.c_str()))
				return true;
		// To avoid plugin comms with DSAce because I ran out of time to make this
		// work, I use something of a trick to get the login ID.
		// Read all login ID files in the account and look for the one with a
		// matching IP to this player. If we find a matching IP then we've got a
		// login ID we can check.
		CAccount* acc = Players.FindAccountFromClientID(iClientID);
		if (acc)
		{
			bool bBannedLoginID = false;

			std::wstring wscDir;
			HkGetAccountDirName(acc, wscDir);

			WIN32_FIND_DATA findFileData;

			std::string scFileSearchPath = scAcctPath + "\\" + wstos(wscDir) + "\\login_*.ini"; // Looks like DSAM generates this file
			HANDLE hFileFind = FindFirstFile(scFileSearchPath.c_str(), &findFileData);
			if (hFileFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					// Read the login ID and IP from the login ID record.
					std::string scLoginID;
					std::string scLoginID2;
					std::string scThisIP;
					std::string scFilePath = scAcctPath + wstos(wscDir) + "\\" + findFileData.cFileName;
					FILE* f;
					fopen_s(&f, scFilePath.c_str(), "r");
					if (f)
					{
						char szBuf[200];
						if (fgets(szBuf, sizeof(szBuf), f) != NULL)
						{
							std::string sz = szBuf;
							try
							{
								scLoginID = Trim(GetParam(sz, '\t', 1).substr(3, std::string::npos));
								scThisIP = Trim(GetParam(sz, '\t', 2).substr(3, std::string::npos));
								if (GetParam(sz, '\t', 3).length() > 4)
									scLoginID2 = Trim(GetParam(sz, '\t', 3).substr(4, std::string::npos));
							}
							catch (...)
							{
								Console::ConErr(L"ERR Corrupt loginid file $0", stows(scFilePath).c_str());
							}
						}
						fclose(f);
					}

					if (FLHookConfig::i()->general.debugMode)
					{
						Console::ConInfo(L"NOTICE: Checking for ban on IP %s Login ID1 %s "
						                 L"ID2 %s "
						                 L"Client %d\n",
						    stows(scThisIP).c_str(), stows(scLoginID).c_str(), stows(scLoginID2).c_str(), iClientID);
					}

					// If the login ID has been read then check it to see if it has
					// been banned
					if (scThisIP == scIP && scLoginID.length())
					{
						for (auto& ban : global->loginIDBans.Bans)
						{
							if (ban == scLoginID || ban == scLoginID2)
							{
								Console::ConWarn(L"* Kicking player on ID ban: ip=%s "
								                 L"id1=%s id2=%s\n",
								    stows(scThisIP).c_str(), stows(scLoginID).c_str(), stows(scLoginID2).c_str());
								bBannedLoginID = true;
								break;
							}
						}
					}
				} while (FindNextFile(hFileFind, &findFileData));
				FindClose(hFileFind);
			}

			if (bBannedLoginID)
				return true;
		}
		return false;
	}

	/// Return true if this client is in in the AuthenticatedAccounts.json
	/// file indicating that the client can connect even if
	/// they are otherwise on a restricted IP range.
	static bool IsAuthenticated(uint iClientID)
	{
		CAccount* acc = Players.FindAccountFromClientID(iClientID);

		if (!acc)
			return false;

		std::wstring directory;
		if (HkGetAccountDirName(acc, directory) != HKE_OK)
			return false;

		if (std::find(global->authenticatedAccounts.Accounts.begin(), global->authenticatedAccounts.Accounts.end(), directory) !=
		    global->authenticatedAccounts.Accounts.end())
			return true;
		else
			return false;
	}

	static void ReloadIPBans()
	{
		global->ipBans = Serializer::JsonToObject<IPBans>();

		if (FLHookConfig::i()->general.debugMode)
			Console::ConInfo(L"NOTICE: Loading IP bans from %s", stows(global->ipBans.File()).c_str());

		Console::ConInfo(L"IP Bans [%u]", global->ipBans.Bans.size());
	}

	static void ReloadLoginIDBans()
	{
		global->loginIDBans = Serializer::JsonToObject<LoginIDBans>();

		if (FLHookConfig::i()->general.debugMode)
			Console::ConInfo(L"NOTICE: Loading Login ID bans from %s", stows(global->loginIDBans.File()).c_str());

		Console::ConInfo(L"Login ID Bans [%u]", global->loginIDBans.Bans.size());
	}

	static void ReloadAuthenticatedAccounts()
	{
		global->authenticatedAccounts = Serializer::JsonToObject<AuthenticatedAccounts>();

		if (FLHookConfig::i()->general.debugMode)
			Console::ConInfo(L"NOTICE: Loading Authenticated Accounts from %s", stows(global->authenticatedAccounts.File()).c_str());

		Console::ConInfo(L"Authenticated Accounts [%u]", global->authenticatedAccounts.Accounts.size());
	}

	/// Reload the ipbans file.
	void LoadSettings()
	{
		ReloadIPBans();
		ReloadLoginIDBans();
		ReloadAuthenticatedAccounts();
	}

	void PlayerLaunch(uint& iShip, uint& iClientID)
	{
		if (!global->IPChecked[iClientID])
		{
			global->IPChecked[iClientID] = true;
			if (IsBanned(iClientID) && !IsAuthenticated(iClientID))
			{
				HkAddKickLog(iClientID, L"IP banned");
				HkMsgAndKick(iClientID, L"Your IP is banned, please contact an administrator", 15000L);
			}
		}
	}

	void BaseEnter(uint& iBaseID, uint& iClientID)
	{
		if (!global->IPChecked[iClientID])
		{
			global->IPChecked[iClientID] = true;
			if (IsBanned(iClientID) && !IsAuthenticated(iClientID))
			{
				HkAddKickLog(iClientID, L"IP banned");
				HkMsgAndKick(iClientID, L"Your IP is banned, please contact an administrator", 7000L);
			}
		}
	}

	void ClearClientInfo(uint iClientID) { global->IPChecked[iClientID] = false; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ADMIN COMMANDS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void AdminCmd_ReloadBans(CCmds* cmds)
	{
		ReloadLoginIDBans();
		ReloadIPBans();
		ReloadAuthenticatedAccounts();
		cmds->Print(L"OK");
	}

	/** Start automatic zone checking */
	void AdminCmd_AuthenticateChar(CCmds* cmds, const std::wstring& wscCharname)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		st6::wstring str((ushort*)wscCharname.c_str());
		CAccount* acc = Players.FindAccountFromCharacterName(str);

		if (!acc)
			return;

		std::wstring directory;
		if (HkGetAccountDirName(acc, directory) != HKE_OK)
		{
			cmds->Print(L"ERR Account not found");
			return;
		}

		if (std::find(global->authenticatedAccounts.Accounts.begin(), global->authenticatedAccounts.Accounts.end(), directory) ==
		    global->authenticatedAccounts.Accounts.end())
				global->authenticatedAccounts.Accounts.emplace_back(directory);

		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ADMIN COMMAND PROCESSING
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Define usable admin commands here
	void CmdHelp_Callback(CCmds* classptr)
	{
		classptr->Print(L"authchar <charname>");
		classptr->Print(L"reloadbans");
	}

	// Admin command callback. Compare the chat entry to see if it match a command
	bool ExecuteCommandString_Callback(CCmds* cmds, const std::wstring& wscCmd)
	{
		if (wscCmd == L"authchar")
		{
			global->returncode = ReturnCode::SkipAll;
			AdminCmd_AuthenticateChar(cmds, cmds->ArgStr(1));
			return true;
		}
		else if (wscCmd == L"reloadbans")
		{
			global->returncode = ReturnCode::SkipAll;
			AdminCmd_ReloadBans(cmds);
			return true;
		}
		return false;
	}
} // namespace Plugins::IPBan

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::IPBan;
REFL_AUTO(type(IPBans), field(Bans))
REFL_AUTO(type(LoginIDBans), field(Bans))
REFL_AUTO(type(AuthenticatedAccounts), field(Accounts))

DefaultDllMainSettings(LoadSettings)

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("IP Ban Plugin");
	pi->shortName("ip_ban");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString_Callback);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Help, &CmdHelp_Callback);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
}
