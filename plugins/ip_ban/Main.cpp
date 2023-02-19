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
		for (const auto& ban : global->ipBans.Bans)
			if (Wildcard::Fit(ban.c_str(), scIP.c_str()))
				return true;
		// To avoid plugin comms with DSAce because I ran out of time to make this
		// work, I use something of a trick to get the login Id.
		// Read all login Id files in the account and look for the one with a
		// matching IP to this player. If we find a matching IP then we've got a
		// login Id we can check.
		if (const CAccount* acc = Players.FindAccountFromClientID(client))
		{
			bool isBannedLoginId = false;

			std::wstring dir = Hk::Client::GetAccountDirName(acc);

			WIN32_FIND_DATA findFileData;

			std::string scFileSearchPath = CoreGlobals::c()->accPath + "\\" + wstos(dir) + "\\login_*.ini"; // Looks like DSAM generates this file
			if (HANDLE hFileFind = FindFirstFile(scFileSearchPath.c_str(), &findFileData); 
				hFileFind != INVALID_HANDLE_VALUE)
			{
				do
				{
					// Read the login Id and IP from the login Id record.
					std::string loginId;
					std::string loginId2;
					std::string thisIP;
					std::string filePath = CoreGlobals::c()->accPath + wstos(dir) + "\\" + findFileData.cFileName;
					FILE* f;
					fopen_s(&f, filePath.c_str(), "r");
					if (f)
					{
						if (char szBuf[200]; fgets(szBuf, sizeof(szBuf), f) != nullptr)
						{
							std::string sz = szBuf;
							try
							{
								loginId = Trim(GetParam(sz, '\t', 1).substr(3, std::string::npos));
								thisIP = Trim(GetParam(sz, '\t', 2).substr(3, std::string::npos));
								if (GetParam(sz, '\t', 3).length() > 4)
									loginId2 = Trim(GetParam(sz, '\t', 3).substr(4, std::string::npos));
							}
							catch (...)
							{
								Console::ConErr(std::format("ERR Corrupt loginid file {}", filePath));
							}
						}
						fclose(f);
					}

					if (FLHookConfig::i()->general.debugMode)
					{
						Console::ConInfo(std::format("Checking for ban on IP {} Login Id1 {} Id2 {} Client {}\n", thisIP, loginId, loginId2, client));
					}

					// If the login Id has been read then check it to see if it has
					// been banned
					if (thisIP == scIP && loginId.length())
					{
						for (const auto& ban : global->loginIdBans.Bans)
						{
							if (ban == loginId || ban == loginId2)
							{
								Console::ConWarn(std::format("* Kicking player on Id ban: ip={} id1={} id2={}\n", thisIP, loginId, loginId2));
								isBannedLoginId = true;
								break;
							}
						}
					}
				} while (FindNextFile(hFileFind, &findFileData));
				FindClose(hFileFind);
			}

			if (isBannedLoginId)
				return true;
		}
		return false;
	}

	/** @ingroup IPBan
	 * @brief Return true if this client is in in the AuthenticatedAccounts.json file indicating that the client can connect even if they are otherwise on a restricted IP range.
	 */
	static bool IsAuthenticated(ClientId client)
	{
		const CAccount* acc = Players.FindAccountFromClientID(client);

		if (!acc)
			return false;

		std::wstring directory = Hk::Client::GetAccountDirName(acc);

		return (std::ranges::find(global->authenticatedAccounts.Accounts, directory) != global->authenticatedAccounts.Accounts.end());
	}

	/** @ingroup IPBan
	 * @brief Reload IP Bans from file.
	 */
	static void ReloadIPBans()
	{
		global->ipBans = Serializer::JsonToObject<IPBans>();

		if (FLHookConfig::i()->general.debugMode)
			Console::ConInfo(std::format("Loading IP bans from {}", global->ipBans.File()));

		Console::ConInfo(std::format("IP Bans [{}]", global->ipBans.Bans.size()));
	}

	/** @ingroup IPBan
	 * @brief Reload Login Id bans from file.
	 */
	static void ReloadLoginIdBans()
	{
		global->loginIdBans = Serializer::JsonToObject<LoginIdBans>();

		if (FLHookConfig::i()->general.debugMode)
			Console::ConInfo(std::format("Loading Login Id bans from {}", global->loginIdBans.File()));

		Console::ConInfo(std::format("Login Id Bans [{}]", global->loginIdBans.Bans.size()));
	}

	/** @ingroup IPBan
	 * @brief Reload Authenticated Accounts from file.
	 */
	static void ReloadAuthenticatedAccounts()
	{
		global->authenticatedAccounts = Serializer::JsonToObject<AuthenticatedAccounts>();

		if (FLHookConfig::i()->general.debugMode)
			Console::ConInfo(std::format("Loading Authenticated Accounts from {}", global->authenticatedAccounts.File()));

		Console::ConInfo(std::format("Authenticated Accounts [{}]", global->authenticatedAccounts.Accounts.size()));
	}

	/// Reload the ipbans file.
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

		ReloadIPBans();
		ReloadLoginIdBans();
		ReloadAuthenticatedAccounts();
	}

	/** @ingroup IPBan
	 * @brief Hook on PlayerLaunch. Checks if player is banned and kicks if so.
	 */
	void PlayerLaunch([[maybe_unused]] const uint& ship, ClientId& client)
	{
		if (!global->IPChecked[client])
		{
			global->IPChecked[client] = true;
			if (IsBanned(client) && !IsAuthenticated(client))
			{
				AddKickLog(client, "IP banned");
				Hk::Player::MsgAndKick(client, global->config->BanMessage, 15000L);
			}
		}
	}

	/** @ingroup IPBan
	 * @brief Hook on BaseEnter. Checks if player is banned and kicks if so.
	 */
	void BaseEnter([[maybe_unused]] const uint& baseId, ClientId& client)
	{
		if (!global->IPChecked[client])
		{
			global->IPChecked[client] = true;
			if (IsBanned(client) && !IsAuthenticated(client))
			{
				AddKickLog(client, "IP banned");
				Hk::Player::MsgAndKick(client, global->config->BanMessage, 7000L);
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
		cmds->Print("OK");
	}

	/** @ingroup IPBan
	 * @brief Is called when an admin types ".authchar".
	 */
	void AdminCmd_AuthenticateChar(CCmds* cmds, const std::wstring& wscCharname)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print("ERR No permission");
			return;
		}

		st6::wstring str((ushort*)wscCharname.c_str());
		const CAccount* acc = Players.FindAccountFromCharacterName(str);

		if (!acc)
			return;


		if (std::wstring directory = Hk::Client::GetAccountDirName(acc); std::ranges::find(global->authenticatedAccounts.Accounts, directory) ==
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
		classptr->Print("authchar <charname>");
		classptr->Print("reloadbans");
	}

	/** @ingroup IPBan
	 * @brief Admin command callback. Compare the chat entry to see if it match a command
	 */
	bool ExecuteCommandString_Callback(CCmds* cmds, const std::wstring& cmd)
	{
		if (cmd == L"authchar")
		{
			global->returncode = ReturnCode::SkipAll;
			AdminCmd_AuthenticateChar(cmds, cmds->ArgStr(1));
			return true;
		}
		else if (cmd == L"reloadbans")
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
REFL_AUTO(type(Config), field(BanMessage, AttrNotEmptyNotWhiteSpace<std::wstring>{}))

DefaultDllMainSettings(LoadSettings);

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
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
}
