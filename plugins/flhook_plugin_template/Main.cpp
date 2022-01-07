// This is a template with the bare minimum to have a functional plugin.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

// Load configuration file
void LoadSettings() {
    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string configFile =
        std::string(szCurDir) + "\\flhook_plugins\\$safeprojectname$.ini";

    INI_Reader ini;
    if (ini.open(configFile.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("General")) {
                while (ini.read_value()) {
                    if (ini.is_value("debug")) {
                        set_iPluginDebug = ini.get_value_int(0);
                    }
                }
            }
        }

        if (set_iPluginDebug & 1) {
            ConPrint(L"Debug\n");
        }

        ini.close();
    }
}

// Do something every 100 seconds
void HkTimer() {
    if ((time(0) % 100) == 0) {
        // Do something here
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Demo command
bool UserCmd_Template(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage) {
    PrintUserCmdText(iClientID, L"OK");
    return true;
}

// Additional information related to the plugin when the /help command is used
void UserCmd_Help(uint iClientID, const std::wstring &wscParam) {
    PrintUserCmdText(iClientID, L"/template");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMAND PROCESSING
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Define usable chat commands here
USERCMD UserCmds[] = {
    {L"/template", UserCmd_Template, L"Usage: /template"},
};

// Process user input
bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADMIN COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Demo admin command
void AdminCmd_Template(CCmds *cmds, float number) {
    if (cmds->ArgStrToEnd(1).length() == 0) {
        cmds->Print(L"ERR Usage: template <number>\n");
        return;
    }

    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
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
void CmdHelp(CCmds *classptr) {
    classptr->Print(L"template <number>\n");
}

// Admin command callback. Compare the chat entry to see if it match a command
bool ExecuteCommandString(CCmds *cmds, const std::wstring &wscCmd) {
    if (IS_CMD("template")) {
        g_ReturnCode = ReturnCode::SkipAll;
        AdminCmd_Template(cmds, cmds->ArgFloat(1));
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    srand((uint)time(0));

    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH && set_scCfgFile.length() > 0)
        LoadSettings();

    return true;
}

// Functions to hook
EXPORT void ExportPluginInfo(PluginInfo* pi) {
    pi->version();
    pi->name("$projectname$");
    pi->shortName("$safeprojectname$");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&g_ReturnCode);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
    pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &HkTimer);
    pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
    pi->emplaceHook(HookedCall::FLHook__AdminCommand__Help, &CmdHelp);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
}
