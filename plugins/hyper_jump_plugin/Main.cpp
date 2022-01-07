// This is a template with the bare minimum to have a functional plugin.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h" 

// Load configuration file
void LoadSettings()
{
	returncode = DEFAULT_RETURNCODE;

	// The path to the configuration file.
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	std::string configFile = std::string(szCurDir) + "\\flhook_plugins\\Hyper_Jump_Plugin.ini";

	INI_Reader ini;
	if (ini.open(configFile.c_str(), false))
	{
		while (ini.read_header())
		{	
			if (ini.is_header("General"))
			{
				while (ini.read_value())
				{
					if (ini.is_value("debug"))
					{ 
						set_iPluginDebug = ini.get_value_int(0);
					}					
				}
			}
		}

		if (set_iPluginDebug&1)
		{
			ConPrint(L"Debug\n");
		}

		ini.close();
	}	
}

// Do something every 100 seconds
void HkTimer()
{
	if ((time(0) % 100) == 0)
	{
		// Do something here
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Demo command
void UserCmd_Template(uint iClientID, const std::wstring& wscParam)
{
	PrintUserCmdText(iClientID, L"OK");
	return true;
}

// Additional information related to the plugin when the /help command is used
void UserCmd_Help(uint iClientID, const std::wstring& wscParam)
{
	returncode = DEFAULT_RETURNCODE;
	PrintUserCmdText(iClientID, L"/template");
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMAND PROCESSING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define usable chat commands here
USERCMD UserCmds[] =
{
	{ L"/template", UserCmd_Template, L"Usage: /template" },
};

// Process user input
bool UserCmd_Process(uint iClientID, const std::wstring& wscCmd)
{
	returncode = DEFAULT_RETURNCODE;

	try
	{
		std::wstring wscCmdLineLower = ToLower(wscCmd);

		// If the chat std::string does not match the USER_CMD then we do not handle the
		// command, so let other plugins or FLHook kick in. We require an exact match
		for (uint i = 0; (i < sizeof(UserCmds) / sizeof(USERCMD)); i++)
		{
			if (wscCmdLineLower.find(UserCmds[i].wszCmd) == 0)
			{
				// Extract the parameters std::string from the chat std::string. It should
				// be immediately after the command and a space.
				std::wstring wscParam = L"";
				if (wscCmd.length() > wcslen(UserCmds[i].wszCmd))
				{
					if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
						continue;
					wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
				}

				// Dispatch the command to the appropriate processing function.
				if (UserCmds[i].proc(iClientID, wscCmd, wscParam, UserCmds[i].usage))
				{
					// We handled the command tell FL hook to stop processing this
					// chat std::string.
					returncode = SKIPPLUGINS_NOFUNCTIONCALL; // we handled the command, return immediatly
					return true;
				}
			}
		}
	}
	catch (...)
	{
		AddLog("ERROR: Exception in UserCmd_Process(iClientID=%u, wscCmd=%s)", iClientID, wstos(wscCmd).c_str());
		LOG_EXCEPTION;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADMIN COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Demo admin command
void AdminCmd_Template(CCmds* cmds, float number)
{
	if (cmds->ArgStrToEnd(1).length() == 0)
	{
		cmds->Print(L"ERR Usage: template <number>\n");
		return;
	}

	if (!(cmds->rights & RIGHT_SUPERADMIN))
	{
		cmds->Print(L"ERR No permission\n");
		return;
	}

	cmds->Print(L"Template is %0.0f\n", number);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADMIN COMMAND PROCESSING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define usable admin commands here
void CmdHelp_Callback(CCmds* classptr)
{
	returncode = DEFAULT_RETURNCODE;
	classptr->Print(L"template <number>\n");
}

// Admin command callback. Compare the chat entry to see if it match a command
bool ExecuteCommandString_Callback(CCmds* cmds, const std::wstring& wscCmd)
{
	returncode = DEFAULT_RETURNCODE;

	if (IS_CMD("template"))
	{
		returncode = SKIPPLUGINS_NOFUNCTIONCALL;
		AdminCmd_Template(cmds, cmds->ArgFloat(1));
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	srand((uint)time(0));

	// If we're being loaded from the command line while FLHook is running then
	// set_scCfgFile will not be empty so load the settings as FLHook only
	// calls load settings on FLHook startup and .rehash.
	if (fdwReason == DLL_PROCESS_ATTACH && set_scCfgFile.length() > 0)
		LoadSettings();

	return true;
}

// Functions to hook
EXPORT PLUGIN_INFO* Get_PluginInfo()
{
	PLUGIN_INFO* p_PI = new PLUGIN_INFO();
	p_PI->sName = "Hyper Jump Plugin";
	p_PI->sShortName = "Hyper_Jump_Plugin";
	p_PI->bMayPause = true;
	p_PI->bMayUnload = true;
	p_PI->ePluginReturnCode = &returncode;
	pi->emplaceHook(PLUGIN_HOOKINFO((FARPROC*)&LoadSettings, PLUGIN_LoadSettings, 0));
	pi->emplaceHook(PLUGIN_HOOKINFO((FARPROC*)&HkTimer, PLUGIN_HkTimerCheckKick, 0));
	pi->emplaceHook(PLUGIN_HOOKINFO((FARPROC*)&ExecuteCommandString_Callback, PLUGIN_ExecuteCommandString_Callback, 0));
	pi->emplaceHook(PLUGIN_HOOKINFO((FARPROC*)&CmdHelp_Callback, PLUGIN_CmdHelp_Callback, 0));
	pi->emplaceHook(PLUGIN_HOOKINFO((FARPROC*)&UserCmd_Process, PLUGIN_UserCmd_Process, 0));
	pi->emplaceHook(PLUGIN_HOOKINFO((FARPROC*)&UserCmd_Help, PLUGIN_UserCmd_Help, 0));
	}
