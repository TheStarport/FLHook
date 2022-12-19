/**
 * @date Feb, 2010
 * @author Cannon (Ported by Raikkonen)
 * @defgroup IPBan IP Ban
 * @brief
 * This plugin is used to ban players based on their IP address.
 *
 * @paragraph cmds Player Commands
 * There are no player commands in this plugin.
 *
 * @paragraph adminCmds Admin Commands
 * All commands are prefixed with '.' unless explicitly specified.
 * - authchar <charname> - Allow a character to connect even if they are in a restricted IP range.
 * - reloadbans - Reload the bans from file.
 *
 * @paragraph configuration Configuration
 * No configuration file is needed.
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no dependencies.
 */
#include "Main.h"

namespace Plugins::IPBan
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup IPBan
	 * @brief Return true if this client is on a banned IP range.
	 */
	static bool IsBanned(ClientId client)
	{
		std::wstring wscIP = Hk::Admin::GetPlayerIP(client);
		std::string scIP = wstos(wscIP);

		// Check for an IP range match.
		for (auto& ban : global->ipBans.Bans)
			if (Wildcard::Fit(ban.c_str(), scIP.c_str()))
				return true;
		// To avoid plugin comms with DSAce because I ran out of time to make this
		// work, I use something of a trick to get the login Id.
		// Read all login Id files in the account and look for the one with a
		// matching IP to this player. If we find a matching IP then we've got a
		// login Id we can check.
		CAccount* acc = Players.FindAccountFromClientID(client);
		if (acc)
		{
			bool bBannedLoginId = false;

			std::wstring dir = Hk::Client::GetAccountDirName(acc);

			WIN32_FIND_DATA findFileData;

			std::string scFileSearchPath = scAcctPath + "\\" + wstos(dir) + "\\login_*.ini"; // Looks like DSAM generates this file
			HANDLE hFileFind = FindFirstFile(scFileSearchPath.c_str(), &findFileData);
			if (hFileFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					// Read the login Id and IP from the login Id record.
					std::string scLoginId;
					std::string scLoginId2;
					std::string scThisIP;
					std::string scFilePath = scAcctPath + wstos(dir) + "\\" + findFileData.cFileName;
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
								scLoginId = Trim(GetParam(sz, '\t', 1).substr(3, std::string::npos));
								scThisIP = Trim(GetParam(sz, '\t', 2).substr(3, std::string::npos));
								if (GetParam(sz, '\t', 3).length() > 4)
									scLoginId2 = Trim(GetParam(sz, '\t', 3).substr(4, std::string::npos));
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
						Console::ConInfo(L"NOTICE: Checking for ban on IP %s Login Id1 %s "
						                 L"Id2 %s "
						                 L"Client %d\n",
						    stows(scThisIP).c_str(), stows(scLoginId).c_str(), stows(scLoginId2).c_str(), client);
					}

					// If the login Id has been read then check it to see if it has
					// been banned
					if (scThisIP == scIP && scLoginId.length())
					{
						for (auto& ban : global->loginIdBans.Bans)
						{
							if (ban == scLoginId || ban == scLoginId2)
							{
								Console::ConWarn(L"* Kicking player on Id ban: ip=%s "
								                 L"id1=%s id2=%s\n",
								    stows(scThisIP).c_str(), stows(scLoginId).c_str(), stows(scLoginId2).c_str());
								bBannedLoginId = true;
								break;
							}
						}
					}
				} while (FindNextFile(hFileFind, &findFileData));
				FindClose(hFileFind);
			}

			if (bBannedLoginId)
				return true;
		}
		return false;
	}

	/** @ingroup IPBan
	 * @brief Return true if this client is in in the AuthenticatedAccounts.json file indicating that the client can connect even if they are otherwise on a restricted IP range.
	 */
	static bool IsAuthenticated(ClientId client)
	{
		CAccount* acc = Players.FindAccountFromClientID(client);

		if (!acc)
			return false;

		std::wstring directory = Hk::Client::GetAccountDirName(acc);

		if (std::find(global->authenticatedAccounts.Accounts.begin(), global->authenticatedAccounts.Accounts.end(), directory) !=
		    global->authenticatedAccounts.Accounts.end())
			return true;
		else
			return false;
	}

	/** @ingroup IPBan
	 * @brief Reload IP Bans from file.
	 */
	static void ReloadIPBans()
	{
		global->ipBans = Serializer::JsonToObject<IPBans>();

		if (FLHookConfig::i()->general.debugMode)
			Console::ConInfo(L"NOTICE: Loading IP bans from %s", stows(global->ipBans.File()).c_str());

		Console::ConInfo(L"IP Bans [%u]", global->ipBans.Bans.size());
	}

	/** @ingroup IPBan
	 * @brief Reload Login Id bans from file.
	 */
	static void ReloadLoginIdBans()
	{
		global->loginIdBans = Serializer::JsonToObject<LoginIdBans>();

		if (FLHookConfig::i()->general.debugMode)
			Console::ConInfo(L"NOTICE: Loading Login Id bans from %s", stows(global->loginIdBans.File()).c_str());

		Console::ConInfo(L"Login Id Bans [%u]", global->loginIdBans.Bans.size());
	}

	/** @ingroup IPBan
	 * @brief Reload Authenticated Accounts from file.
	 */
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
		ReloadLoginIdBans();
		ReloadAuthenticatedAccounts();
	}

	/** @ingroup IPBan
	 * @brief Hook on PlayerLaunch. Checks if player is banned and kicks if so.
	 */
	void PlayerLaunch(uint& ship, ClientId& client)
	{
		if (!global->IPChecked[client])
		{
			global->IPChecked[client] = true;
			if (IsBanned(client) && !IsAuthenticated(client))
			{
				AddKickLog(client, "IP banned");
				Hk::Player::MsgAndKick(client, L"Your IP is banned, please contact an administrator", 15000L);
			}
		}
	}

	/** @ingroup IPBan
	 * @brief Hook on BaseEnter. Checks if player is banned and kicks if so.
	 */
	void BaseEnter(uint& iBaseId, ClientId& client)
	{
		if (!global->IPChecked[client])
		{
			global->IPChecked[client] = true;
			if (IsBanned(client) && !IsAuthenticated(client))
			{
				AddKickLog(client, "IP banned");
				Hk::Player::MsgAndKick(client, L"Your IP is banned, please contact an administrator", 7000L);
			}
		}
	}

	/** @ingroup IPBan
	 * @brief Hook on ClearClientInfo. Resets the checked variable for the client Id.
	 */
	void ClearClientInfo(ClientId client) { global->IPChecked[client] = false; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ADMIN COMMANDS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** @ingroup IPBan
	 * @brief Is called when an admin types ".reloadbans".
	 */
	void AdminCmd_ReloadBans(CCmds* cmds)
	{
		ReloadLoginIdBans();
		ReloadIPBans();
		ReloadAuthenticatedAccounts();
		cmds->Print(L"OK");
	}

	/** @ingroup IPBan
	 * @brief Is called when an admin types ".authchar".
	 */
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

		std::wstring directory = Hk::Client::GetAccountDirName(acc);

		if (std::find(global->authenticatedAccounts.Accounts.begin(), global->authenticatedAccounts.Accounts.end(), directory) ==
		    global->authenticatedAccounts.Accounts.end())
				global->authenticatedAccounts.Accounts.emplace_back(directory);

		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// ADMIN COMMAND PROCESSING
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** @ingroup IPBan
	 * @brief Is called when an admin types ".help".
	 */
	void CmdHelp_Callback(CCmds* classptr)
	{
		classptr->Print(L"authchar <charname>");
		classptr->Print(L"reloadbans");
	}

	/** @ingroup IPBan
	 * @brief Admin command callback. Compare the chat entry to see if it match a command
	 */
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
REFL_AUTO(type(LoginIdBans), field(Bans))
REFL_AUTO(type(AuthenticatedAccounts), field(Accounts))

DefaultDllMainSettings(LoadSettings)

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("IP Ban Plugin");
	pi->shortName("ip_ban");
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
