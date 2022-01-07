/**
Connecticut Plugin by MadHunter
*/

// includes

#include "../hookext_plugin/hookext_exports.h"
#include <FLHook.h>
#include <algorithm>
#include <list>
#include <map>
#include <math.h>
#include <plugin.h>
#include <plugin_comms.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

#define CLIENT_STATE_NONE 0
#define CLIENT_STATE_TRANSFER 1
#define CLIENT_STATE_RETURN 2

int transferFlags[MAX_CLIENT_ID + 1];

const std::wstring STR_INFO1 = L"Please dock at nearest base";
const std::wstring STR_INFO2 = L"Cargo hold is not empty";

int set_iPluginDebug = 0;
// Base to beam to.
uint set_iTargetBaseID = 0;

// Restricted system, cannot jump out of here.
uint set_iRestrictedSystemID = 0;

// Target system, cannot jump out of here.
uint set_iTargetSystemID = 0;

// Base to use if player is trapped in the conn system.
uint set_iDefaultBaseID = 0;

/// A return code to indicate to FLHook if we want the hook processing to
/// continue.
ReturnCode returncode;

/// Clear client info when a client connects.
void ClearClientInfo(uint iClientID) {
    transferFlags[iClientID] = CLIENT_STATE_NONE;
}

/// Load the configuration
void LoadSettings() {
    

    memset(transferFlags, 0, sizeof(int) * (MAX_CLIENT_ID + 1));

    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\conn.cfg";

    // Load generic settings
    set_iPluginDebug = IniGetI(scPluginCfgFile, "General", "Debug", 0);
    set_iTargetBaseID = CreateID(
        IniGetS(scPluginCfgFile, "General", "TargetBase", "li06_05_base")
            .c_str());
    set_iTargetSystemID = CreateID(
        IniGetS(scPluginCfgFile, "General", "TargetSystem", "li06").c_str());
    set_iRestrictedSystemID =
        CreateID(IniGetS(scPluginCfgFile, "General", "RestrictedSystem", "iw09")
                     .c_str());
    set_iDefaultBaseID = CreateID(
        IniGetS(scPluginCfgFile, "General", "DefaultBase", "li01_proxy_base")
            .c_str());
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    static bool patched = false;
    srand((uint)time(0));

    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (set_scCfgFile.length() > 0)
            LoadSettings();
    }
    return true;
}

bool IsDockedClient(unsigned int client) {
    unsigned int base = 0;
    pub::Player::GetBase(client, base);
    if (base)
        return true;

    return false;
}

bool ValidateCargo(unsigned int client) {
    std::wstring playerName =
        (const wchar_t *)Players.GetActiveCharacterName(client);
    std::list<CARGO_INFO> cargo;
    int holdSize = 0;

    HkEnumCargo(playerName, cargo, holdSize);

    for (std::list<CARGO_INFO>::const_iterator it = cargo.begin();
         it != cargo.end(); ++it) {
        const CARGO_INFO &item = *it;

        bool flag = false;
        pub::IsCommodity(item.iArchID, flag);

        // Some commodity present.
        if (flag)
            return false;
    }

    return true;
}

uint GetCustomBaseForClient(unsigned int client) {
    // Pass to plugins incase this ship is docked at a custom base.
    CUSTOM_BASE_IS_DOCKED_STRUCT info;
    info.iClientID = client;
    info.iDockedBaseID = 0;
    Plugin_Communication(CUSTOM_BASE_IS_DOCKED, &info);
    return info.iDockedBaseID;
}

void StoreReturnPointForClient(unsigned int client) {
    // It's not docked at a custom base, check for a regular base
    uint base = GetCustomBaseForClient(client);
    if (!base)
        pub::Player::GetBase(client, base);
    if (!base)
        return;

    HookExt::IniSetI(client, "conn.retbase", base);
}

unsigned int ReadReturnPointForClient(unsigned int client) {
    return HookExt::IniGetI(client, "conn.retbase");
}

void MoveClient(unsigned int client, unsigned int targetBase) {
    // Ask that another plugin handle the beam.
    CUSTOM_BASE_BEAM_STRUCT info;
    info.iClientID = client;
    info.iTargetBaseID = targetBase;
    info.bBeamed = false;
    Plugin_Communication(CUSTOM_BASE_BEAM, &info);
    if (info.bBeamed)
        return;

    // No plugin handled it, do it ourselves.
    unsigned int system;
    pub::Player::GetSystem(client, system);
    Universe::IBase *base = Universe::get_base(targetBase);

    pub::Player::ForceLand(client, targetBase); // beam

    // if not in the same system, emulate F1 charload
    if (base->iSystemID != system) {
        Server.BaseEnter(targetBase, client);
        Server.BaseExit(targetBase, client);
        std::wstring wscCharFileName;
        HkGetCharFileName(ARG_CLIENTID(client), wscCharFileName);
        wscCharFileName += L".fl";
        CHARACTER_ID cID;
        strcpy_s(cID.szCharFilename,
                 wstos(wscCharFileName.substr(0, 14)).c_str());
        Server.CharacterSelect(cID, client);
    }
}

bool CheckReturnDock(unsigned int client, unsigned int target) {
    unsigned int base = 0;
    pub::Player::GetBase(client, base);

    if (base == target)
        return true;

    return false;
}

// Process user input
bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

void __stdcall CharacterSelect(struct CHARACTER_ID const &charid,
                               unsigned int client) {
    
    transferFlags[client] = CLIENT_STATE_NONE;
}

void __stdcall PlayerLaunch_AFTER(unsigned int ship, unsigned int client) {
    

    if (transferFlags[client] == CLIENT_STATE_TRANSFER) {
        if (!ValidateCargo(client)) {
            PrintUserCmdText(client, STR_INFO2);
            return;
        }

        transferFlags[client] = CLIENT_STATE_NONE;
        MoveClient(client, set_iTargetBaseID);
        return;
    }

    if (transferFlags[client] == CLIENT_STATE_RETURN) {
        if (!ValidateCargo(client)) {
            PrintUserCmdText(client, STR_INFO2);
            return;
        }

        transferFlags[client] = CLIENT_STATE_NONE;
        unsigned int returnPoint = ReadReturnPointForClient(client);

        if (!returnPoint)
            return;

        MoveClient(client, returnPoint);
        HookExt::IniSetI(client, "conn.retbase", 0);
        return;
    }
}

/** Functions to hook */
EXPORT PLUGIN_INFO *Get_PluginInfo() {
    PLUGIN_INFO *p_PI = new PLUGIN_INFO();
    p_PI->sName = "Conn Plugin by MadHunter";
    p_PI->sShortName = "conn";
    p_PI->bMayPause = true;
    p_PI->bMayUnload = true;
    p_PI->ePluginReturnCode = &returncode;
    pi->emplaceHook(
        PLUGIN_HOOKINFO((FARPROC *)&LoadSettings, PLUGIN_LoadSettings, 0));
    pi->emplaceHook(PLUGIN_HOOKINFO((FARPROC *)&ClearClientInfo,
                                             PLUGIN_ClearClientInfo, 0));
    pi->emplaceHook(PLUGIN_HOOKINFO(
        (FARPROC *)&CharacterSelect, PLUGIN_HkIServerImpl_CharacterSelect, 0));
    pi->emplaceHook(
        PLUGIN_HOOKINFO((FARPROC *)&PlayerLaunch_AFTER,
                        PLUGIN_HkIServerImpl_PlayerLaunch_AFTER, 0));
    pi->emplaceHook(PLUGIN_HOOKINFO((FARPROC *)&UserCmd_Process,
                                             PLUGIN_UserCmd_Process, 0));
    }
