/**
 * @date Unknown
 * @author M0tah (Ported by Raikkonen)
 * @defgroup Mark Mark
 * @brief
 * A plugin that allows players to mark objects for themselves or group
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - mark - Mark the selected object.
 * - unmark - Unmark the selected object.
 * - unmarkall - Unmark all objects.
 * - groupmark - Mark an object for your group.
 * - groupunmark - Unmark an object for your group.
 * - ignoregroupmarks - Ignore any marks your group make.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "AutoMarkRadiusInM": 2000.0
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no dependencies.
 */

#include "Main.h"

namespace Plugins::Mark
{
	std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();

		global->Timers = {{TimerMarkDelay, 50, 0}, {TimerSpaceObjMark, 50, 0}};
		global->config = std::make_unique<Config>(config);
	}

	/** @ingroup Mark
	 * @brief Clear the mark settings for the specified clientId
	 */
	void ClearClientMark(uint clientId)
	{
		global->Mark[clientId].MarkEverything = false;
		global->Mark[clientId].MarkedObjects.clear();
		global->Mark[clientId].DelayedSystemMarkedObjects.clear();
		global->Mark[clientId].AutoMarkedObjects.clear();
		global->Mark[clientId].DelayedAutoMarkedObjects.clear();
	}

	/** @ingroup Mark
	 * @brief Hook on JumpInComplete. This marks objects in the new system (for example, a group member might have marked in when they were out of system).
	 */
	void JumpInComplete(uint& iSystemID, uint& iShip)
	{
		uint iClientID = GetClientIDByShip(iShip);
		if (!iClientID)
			return;
		std::vector<uint> vTempMark;
		for (uint i = 0; i < global->Mark[iClientID].DelayedSystemMarkedObjects.size(); i++)
		{
			if (pub::SpaceObj::ExistsAndAlive(global->Mark[iClientID].DelayedSystemMarkedObjects[i]))
			{
				if (i != global->Mark[iClientID].DelayedSystemMarkedObjects.size() - 1)
				{
					global->Mark[iClientID].DelayedSystemMarkedObjects[i] =
					    global->Mark[iClientID].DelayedSystemMarkedObjects[global->Mark[iClientID].DelayedSystemMarkedObjects.size() - 1];
					i--;
				}
				global->Mark[iClientID].DelayedSystemMarkedObjects.pop_back();
				continue;
			}

			uint iTargetSystem;
			pub::SpaceObj::GetSystem(global->Mark[iClientID].DelayedSystemMarkedObjects[i], iTargetSystem);
			if (iTargetSystem == iSystemID)
			{
				pub::Player::MarkObj(iClientID, global->Mark[iClientID].DelayedSystemMarkedObjects[i], 1);
				vTempMark.push_back(global->Mark[iClientID].DelayedSystemMarkedObjects[i]);
				if (i != global->Mark[iClientID].DelayedSystemMarkedObjects.size() - 1)
				{
					global->Mark[iClientID].DelayedSystemMarkedObjects[i] =
					    global->Mark[iClientID].DelayedSystemMarkedObjects[global->Mark[iClientID].DelayedSystemMarkedObjects.size() - 1];
					i--;
				}
				global->Mark[iClientID].DelayedSystemMarkedObjects.pop_back();
			}
		}
		for (uint i = 0; i < global->Mark[iClientID].MarkedObjects.size(); i++)
		{
			if (!pub::SpaceObj::ExistsAndAlive(global->Mark[iClientID].MarkedObjects[i]))
				global->Mark[iClientID].DelayedSystemMarkedObjects.push_back(global->Mark[iClientID].MarkedObjects[i]);
		}
		global->Mark[iClientID].MarkedObjects = vTempMark;
	}

	/** @ingroup Mark
	 * @brief Hook on LaunchComplete. Sets all the objects in the system to be marked.
	 */
	void LaunchComplete(uint& iBaseID, uint& iShip)
	{
		uint iClientID = GetClientIDByShip(iShip);
		if (!iClientID)
			return;
		for (uint i = 0; i < global->Mark[iClientID].MarkedObjects.size(); i++)
		{
			if (pub::SpaceObj::ExistsAndAlive(global->Mark[iClientID].MarkedObjects[i]))
			{
				if (i != global->Mark[iClientID].MarkedObjects.size() - 1)
				{
					global->Mark[iClientID].MarkedObjects[i] = global->Mark[iClientID].MarkedObjects[global->Mark[iClientID].MarkedObjects.size() - 1];
					i--;
				}
				global->Mark[iClientID].MarkedObjects.pop_back();
				continue;
			}
			pub::Player::MarkObj(iClientID, global->Mark[iClientID].MarkedObjects[i], 1);
		}
	}

	/** @ingroup Mark
	 * @brief Hook on BaseEnter. Clear marked objects.
	 */
	void BaseEnter(uint& iBaseID, uint& iClientID)
	{
		global->Mark[iClientID].AutoMarkedObjects.clear();
		global->Mark[iClientID].DelayedAutoMarkedObjects.clear();
	}

	/** @ingroup Mark
	 * @brief Hook on Update. Calls timers.
	 */
	int Update()
	{
		static bool bFirstTime = true;
		if (bFirstTime)
		{
			bFirstTime = false;
			// check for logged in players and reset their connection data
			struct PlayerData* pPD = 0;
			while (pPD = Players.traverse_active(pPD))
				ClearClientMark(GetClientIdFromPD(pPD));
		}
		for (auto& timer : global->Timers)
		{
			if ((timeInMS() - timer.LastCall) >= timer.IntervalMS)
			{
				timer.LastCall = timeInMS();
				timer.proc();
			}
		}
		return 0;
	}

	/** @ingroup Mark
	 * @brief Hook on Disconnect. CallsClearClientMark.
	 */
	void DisConnect(uint& iClientID, enum EFLConnection& p2) { ClearClientMark(iClientID); }

	/** @ingroup Mark
	 * @brief Hook on LoadUserCharSettings. Loads the character's settings from the flhookuser.ini file.
	 */
	void LoadUserCharSettings(uint& iClientID)
	{
		CAccount* acc = Players.FindAccountFromClientID(iClientID);
		std::wstring dir;
		GetAccountDirName(acc, dir);
		std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
		std::wstring wscFilename;
		GetCharFileName(iClientID, wscFilename);
		std::string scFilename = wstos(wscFilename);
		std::string scSection = "general_" + scFilename;
		global->Mark[iClientID].MarkEverything = IniGetB(scUserFile, scSection, "automarkenabled", false);
		global->Mark[iClientID].IgnoreGroupMark = IniGetB(scUserFile, scSection, "ignoregroupmarkenabled", false);
		global->Mark[iClientID].AutoMarkRadius = IniGetF(scUserFile, scSection, "automarkradius", global->config->AutoMarkRadiusInM);
	}

	const std::vector commands = {{
	    CreateUserCommand(L"/mark", L"", UserCmd_MarkObj,
	        L"Makes the selected object appear in the important section of the contacts and have an arrow on the side of the screen, as well as have > and < "
	        L"on the sides of the selection box."),
	    CreateUserCommand(L"/m", L"", UserCmd_MarkObj, L"Shortcut for /mark."),
	    CreateUserCommand(L"/unmark", L"", UserCmd_UnMarkObj, L"Unmarks the selected object marked with the /mark (/m) command."),
	    CreateUserCommand(L"/um", L"", UserCmd_UnMarkObj, L"Shortcut from /unmark."),
	    CreateUserCommand(L"/unmarkall", L"", UserCmd_UnMarkAllObj, L"Unmarks all objects that have been marked."),
	    CreateUserCommand(L"/uma", L"", UserCmd_UnMarkAllObj, L"Shortcut for /unmarkall."),
	    CreateUserCommand(L"/groupmark", L"", UserCmd_MarkObjGroup, L"Marks selected object for the entire group."),
	    CreateUserCommand(L"/gm", L"", UserCmd_MarkObjGroup, L"Shortcut for /groupmark."),
	    CreateUserCommand(L"/groupunmark", L"", UserCmd_UnMarkObjGroup, L"Unmarks the selected object for the entire group."),
	    CreateUserCommand(L"/gum", L"", UserCmd_UnMarkObjGroup, L"Shortcut for /groupunmark."),
	    CreateUserCommand(L"/ignoregroupmarks", L"<on|off>", UserCmd_SetIgnoreGroupMark, L"Ignores marks from others in your group."),
	    CreateUserCommand(L"/automark", L"<on|off> [radius in KM]", UserCmd_AutoMark,
	        L"Automatically marks all ships in KM radius.Bots are marked automatically in the range specified whether on or off. If you want to completely "
	        L"disable automarking, set the radius to a number <= 0."),
	}};
} // namespace Plugins::Mark

using namespace Plugins::Mark;

// REFL_AUTO must be global namespace
REFL_AUTO(type(Config), field(AutoMarkRadiusInM))

DefaultDllMainSettings(LoadSettings)

    extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Mark plugin");
	pi->shortName("mark");
	pi->mayUnload(false);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__JumpInComplete, &JumpInComplete);
	pi->emplaceHook(HookedCall::IServerImpl__LaunchComplete, &LaunchComplete);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::IServerImpl__Update, &Update);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
	pi->emplaceHook(HookedCall::FLHook__LoadCharacterSettings, &LoadUserCharSettings);
}