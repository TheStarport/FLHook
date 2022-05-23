// Restarts plugin - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

// Players with a rank above this value cannot use the restart command.
static int set_iMaxRank;

// Players with a cash above this value cannot use the restart command.
static int set_iMaxCash;

// Name of each restart and how much cash they cost
static std::map<std::wstring, int> shipPrices;

void LoadSettings()
{
	// The path to the configuration file.
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	std::string scPluginCfgFile = std::string(szCurDir) + "\\flhook_plugins\\restarts.cfg";

	set_iMaxRank = IniGetI(scPluginCfgFile, "General", "MaxRank", 5);
	set_iMaxCash = IniGetI(scPluginCfgFile, "General", "MaxCash", 1000000);
	set_bEnableRestartCost = IniGetB(scPluginCfgFile, "General", "EnableCost", false);

	INI_Reader ini;
	if (ini.open(scPluginCfgFile.c_str(), false))
	{
		while (ini.read_header())
		{
			if (ini.is_header("Restart"))
			{
				while (ini.read_value())
				{
					if (ini.is_value("restart"))
					{
						std::wstring name = stows(ini.get_value_string(0));
						int iCash = ini.get_value_int(1);
						shipPrices[name] = iCash;
					}
				}
			}
		}
	}
}

/* User Commands */

void UserCmd_ShowRestarts(uint iClientID, const std::wstring& wscParam)
{
	WIN32_FIND_DATA FileData;
	HANDLE hSearch;

	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	std::string scRestartFiles = std::string(szCurDir) + "\\flhook_plugins\\restart\\*.fl";

	// Start searching for .fl files in the current directory.
	hSearch = FindFirstFile(scRestartFiles.c_str(), &FileData);
	if (hSearch == INVALID_HANDLE_VALUE)
	{
		PrintUserCmdText(iClientID, L"Restart files not found");
		return;
	}

	PrintUserCmdText(iClientID, L"You can use these restarts:");
	do
	{
		// add filename
		std::string scFileName = FileData.cFileName;
		size_t len = scFileName.length();
		scFileName.erase(len - 3, len);
		if (scFileName[0] != '_')
		{
			if (set_bEnableRestartCost)
				PrintUserCmdText(
				    iClientID, stows(scFileName) + L" - $" + std::to_wstring(shipPrices[stows(scFileName)]));
			else
				PrintUserCmdText(iClientID, stows(scFileName));
		}
	} while (FindNextFile(hSearch, &FileData));

	FindClose(hSearch);
}

void UserCmd_Restart(uint iClientID, const std::wstring& wscParam)
{
	std::wstring wscFaction = GetParam(wscParam, ' ', 0);
	if (!wscFaction.length())
	{
		PrintUserCmdText(iClientID, L"ERR Invalid parameters");
		PrintUserCmdText(iClientID, L"/restart <template>");
	}

	// Get the character name for this connection.
	RESTART restart;
	restart.wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

	// Searching restart
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	restart.scRestartFile = std::string(szCurDir) + "\\flhook_plugins\\restart\\" + wstos(wscFaction) + ".fl";

	if (!::PathFileExistsA(restart.scRestartFile.c_str()))
	{
		restart.scRestartFile = std::string(szCurDir) + "\\flhook_plugins\\restart\\_" + wstos(wscFaction) + ".fl";
		if (!PathFileExistsA(restart.scRestartFile.c_str()))
		{
			PrintUserCmdText(iClientID, L"ERR Template does not exist");
		}
	}

	// Saving the characters forces an anti-cheat checks and fixes
	// up a multitude of other problems.
	HkSaveChar(iClientID);
	if (!HkIsValidClientID(iClientID))
		return;

	uint iBaseID;
	pub::Player::GetBase(iClientID, iBaseID);
	if (!iBaseID)
	{
		PrintUserCmdText(iClientID, L"ERR Not in base");
		return;
	}

	if (set_iMaxRank != 0)
	{
		int iRank = 0;
		HkGetRank(restart.wscCharname, iRank);
		if (iRank == 0 || iRank > set_iMaxRank)
		{
			PrintUserCmdText(
			    iClientID,
			    L"ERR You must create a new char to "
			    L"restart. Your rank is too high");
			return;
		}
	}

	HK_ERROR err;
	int iCash = 0;
	if ((err = HkGetCash(restart.wscCharname, iCash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}

	if (set_iMaxCash != 0 && iCash > set_iMaxCash)
	{
		PrintUserCmdText(
		    iClientID,
		    L"ERR You must create a new char to "
		    L"restart. Your cash is too high");
		return;
	}

	if (set_bEnableRestartCost)
	{
		if (iCash < shipPrices[wscFaction])
		{
			PrintUserCmdText(
			    iClientID,
			    L"You need $" + std::to_wstring(shipPrices[wscFaction] - iCash) +
			        L" more credits to use this template");
			return;
		}
		restart.iCash = iCash - shipPrices[wscFaction];
	}
	else
		restart.iCash = iCash;

	CAccount* acc = Players.FindAccountFromClientID(iClientID);
	if (acc)
	{
		HkGetAccountDirName(acc, restart.wscDir);
		HkGetCharFileName(restart.wscCharname, restart.wscCharfile);
		pendingRestarts.push_back(restart);
		HkKickReason(restart.wscCharname, L"Updating character, please wait 10 seconds before reconnecting");
	}
	return;
}

/* Hooks */

void Timer()
{
	while (pendingRestarts.size())
	{
		RESTART restart = pendingRestarts.front();
		if (HkGetClientIdFromCharname(restart.wscCharname) != -1)
			return;

		pendingRestarts.pop_front();

		try
		{
			// Overwrite the existing character file
			std::string scCharFile = scAcctPath + wstos(restart.wscDir) + "\\" + wstos(restart.wscCharfile) + ".fl";
			std::string scTimeStampDesc = IniGetS(scCharFile, "Player", "description", "");
			std::string scTimeStamp = IniGetS(scCharFile, "Player", "tstamp", "0");
			if (!::CopyFileA(restart.scRestartFile.c_str(), scCharFile.c_str(), FALSE))
				throw("copy template");

			flc_decode(scCharFile.c_str(), scCharFile.c_str());
			IniWriteW(scCharFile, "Player", "name", restart.wscCharname);
			IniWrite(scCharFile, "Player", "description", scTimeStampDesc);
			IniWrite(scCharFile, "Player", "tstamp", scTimeStamp);
			IniWrite(scCharFile, "Player", "money", std::to_string(restart.iCash));

			if (!FLHookConfig::i()->general.disableCharfileEncryption)
				flc_encode(scCharFile.c_str(), scCharFile.c_str());

			AddLog(Normal, LogLevel::Info, L"NOTICE: User restart %s for %s", restart.scRestartFile.c_str(),
			    wstos(restart.wscCharname).c_str());
		}
		catch (char* err)
		{
			AddLog(Normal, LogLevel::Info, L"ERROR: User restart failed (%s) for %s", err, wstos(restart.wscCharname).c_str());
		}
		catch (...)
		{
			AddLog(Normal, LogLevel::Info, L"ERROR: User restart failed for %s", wstos(restart.wscCharname).c_str());
		}
	}
}

// Client command processing
USERCMD UserCmds[] = {
	{ L"/restart", UserCmd_Restart },
	{ L"/showrestarts", UserCmd_ShowRestarts },
};

// Process user input
bool UserCmd_Process(uint& iClientID, const std::wstring& wscCmd)
{
	DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

// Hook on /help
EXPORT void UserCmd_Help(uint& iClientID, const std::wstring& wscParam)
{
	PrintUserCmdText(iClientID, L"/restart <template>");
	PrintUserCmdText(iClientID, L"/showrestarts");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Restarts");
	pi->shortName("restarts");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &Timer);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
}
