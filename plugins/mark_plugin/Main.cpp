// Mark Plugin
// Originally by M0tah
// https://sourceforge.net/projects/kosacid/files/

#include "Main.h"

namespace Plugins::Mark
{
	std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();

		global->Timers = {{HkTimerMarkDelay, 50, 0}, {HkTimerSpaceObjMark, 50, 0}};
		global->config = std::make_unique<Config>(config);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void ClearClientMark(uint iClientID)
	{
		global->Mark[iClientID].MarkEverything = false;
		global->Mark[iClientID].MarkedObjects.clear();
		global->Mark[iClientID].DelayedSystemMarkedObjects.clear();
		global->Mark[iClientID].AutoMarkedObjects.clear();
		global->Mark[iClientID].DelayedAutoMarkedObjects.clear();
	}

	void JumpInComplete(uint& iSystemID, uint& iShip)
	{
		uint iClientID = HkGetClientIDByShip(iShip);
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

	void LaunchComplete(uint& iBaseID, uint& iShip)
	{
		uint iClientID = HkGetClientIDByShip(iShip);
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

	void BaseEnter(uint& iBaseID, uint& iClientID)
	{
		global->Mark[iClientID].AutoMarkedObjects.clear();
		global->Mark[iClientID].DelayedAutoMarkedObjects.clear();
	}

	int Update()
	{
		static bool bFirstTime = true;
		if (bFirstTime)
		{
			bFirstTime = false;
			// check for logged in players and reset their connection data
			struct PlayerData* pPD = 0;
			while (pPD = Players.traverse_active(pPD))
				ClearClientMark(HkGetClientIdFromPD(pPD));
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

	void DisConnect(uint& iClientID, enum EFLConnection& p2) { ClearClientMark(iClientID); }

	void LoadUserCharSettings(uint& iClientID)
	{
		CAccount* acc = Players.FindAccountFromClientID(iClientID);
		std::wstring wscDir;
		HkGetAccountDirName(acc, wscDir);
		std::string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
		std::wstring wscFilename;
		HkGetCharFileName(iClientID, wscFilename);
		std::string scFilename = wstos(wscFilename);
		std::string scSection = "general_" + scFilename;
		global->Mark[iClientID].MarkEverything = IniGetB(scUserFile, scSection, "automarkenabled", false);
		global->Mark[iClientID].IgnoreGroupMark = IniGetB(scUserFile, scSection, "ignoregroupmarkenabled", false);
		global->Mark[iClientID].AutoMarkRadius = IniGetF(scUserFile, scSection, "automarkradius", global->config->AutoMarkRadiusInM);
	}

	void UserCmd_Help(uint& iClientID, const std::wstring& wscParam)
	{
		PrintUserCmdText(iClientID, L"/mark /m ");
		PrintUserCmdText(iClientID,
		    L"Makes the selected object appear in the important section of the "
		    L"contacts and have an arrow on the side of the screen, as well as "
		    L"have "
		    L"> and < on the sides of the selection box.");
		PrintUserCmdText(iClientID, L"/unmark /um");
		PrintUserCmdText(iClientID, L"Unmarks the selected object marked with the /mark (/m) command.");
		PrintUserCmdText(iClientID, L"/groupmark /gm");
		PrintUserCmdText(iClientID, L"Marks selected object for the entire group.");
		PrintUserCmdText(iClientID, L"/groupunmark /gum");
		PrintUserCmdText(iClientID, L"Unmarks the selected object for the entire group.");
		PrintUserCmdText(iClientID, L"/ignoregroupmarks <on|off>");
		PrintUserCmdText(iClientID, L"Ignores marks from others in your group.");
		PrintUserCmdText(iClientID, L"/automark <on|off> [radius in KM]");
		PrintUserCmdText(iClientID,
		    L"Automatically marks all ships in KM radius.Bots are marked "
		    L"automatically in the\n  range specified whether on or off. If you "
		    L"want "
		    L"to completely disable automarking, set the radius to a number <= 0.");
	}

	USERCMD UserCmds[] = {
	    {L"/mark", UserCmd_MarkObj},
	    {L"/m", UserCmd_MarkObj},
	    {L"/unmark", UserCmd_UnMarkObj},
	    {L"/um", UserCmd_UnMarkObj},
	    {L"/unmarkall", UserCmd_UnMarkAllObj},
	    {L"/uma", UserCmd_UnMarkAllObj},
	    {L"/groupmark", UserCmd_MarkObjGroup},
	    {L"/gm", UserCmd_MarkObjGroup},
	    {L"/groupunmark", UserCmd_UnMarkObjGroup},
	    {L"/gum", UserCmd_UnMarkObjGroup},
	    {L"/ignoregroupmarks", UserCmd_SetIgnoreGroupMark},
	    {L"/automark", UserCmd_AutoMark},
	};

	// Process user input
	bool UserCmd_Process(uint& iClientID, const std::wstring& wscCmd) { DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, global->returncode); }
}

using namespace Plugins::Mark;
// REFL_AUTO must be global namespace
REFL_AUTO(type(Config), field(AutoMarkRadiusInM))

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		LoadSettings();
	return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Mark plugin by M0tah");
	pi->shortName("mark");
	pi->mayPause(false);
	pi->mayUnload(false);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
	pi->emplaceHook(HookedCall::IServerImpl__JumpInComplete, &JumpInComplete);
	pi->emplaceHook(HookedCall::IServerImpl__LaunchComplete, &LaunchComplete);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::IServerImpl__Update, &Update);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
	pi->emplaceHook(HookedCall::FLHook__LoadCharacterSettings, &LoadUserCharSettings);
}