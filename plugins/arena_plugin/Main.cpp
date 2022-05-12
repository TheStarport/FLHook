// Arena Plugin by MadHunter
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes

#include <FLHook.h>
#include <plugin.h>

#define CLIENT_STATE_NONE 0
#define CLIENT_STATE_TRANSFER 1
#define CLIENT_STATE_RETURN 2

int transferFlags[MAX_CLIENT_ID + 1];

const std::wstring STR_INFO1 = L"Please dock at nearest base";
const std::wstring STR_INFO2 = L"Cargo hold is not empty";

struct Config final : Reflectable {
    std::string File() override { return "flhook_plugins/arena.json"; }

    // Reflectable fields
    std::string command = "arena";
    std::string targetBase;
    std::string targetSystem;
    std::string restrictedSystem;
    std::string defaultBase;

    // Non-reflectable fields
    uint targetBaseId;
    uint targetSystemId;
    uint restrictedSystemId;
    uint defaultBaseId;
    std::wstring wscCommand;
};

REFL_AUTO(
    type(Config),
    field(command),
    field(targetBase),
    field(targetSystem),
    field(restrictedSystem),
    field(defaultBase)
)

/// A return code to indicate to FLHook if we want the hook processing to
/// continue.
ReturnCode returncode = ReturnCode::Default;

/// Clear client info when a client connects.
void ClearClientInfo(uint& iClientID) {
    transferFlags[iClientID] = CLIENT_STATE_NONE;
}

std::unique_ptr<Config> config = nullptr;

// Client command processing
void UserCmd_Conn(uint iClientID, const std::wstring &wscParam);
void UserCmd_Return(uint iClientID, const std::wstring &wscParam);
USERCMD UserCmds[] = {
    {L"/arena", UserCmd_Conn},
    {L"/return", UserCmd_Return},
};

/// Load the configuration
void LoadSettings() {
    memset(transferFlags, 0, sizeof(int) * (MAX_CLIENT_ID + 1));
    Config conf = Serializer::JsonToObject<Config>();
    conf.wscCommand = L"/" + stows(conf.command);
    conf.defaultBaseId = CreateID(conf.defaultBase.c_str());
    conf.restrictedSystemId = CreateID(conf.restrictedSystem.c_str());
    conf.targetBaseId = CreateID(conf.targetBase.c_str());
    conf.targetSystemId = CreateID(conf.targetSystem.c_str());
    config = std::make_unique<Config>(std::move(conf));

    UserCmds[0] = { config->wscCommand.data(), UserCmd_Conn };
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
    PluginCommunication(CUSTOM_BASE_IS_DOCKED, &info);
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
    PluginCommunication(CUSTOM_BASE_BEAM, &info);
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

void __stdcall CharacterSelect(std::string& szCharFilename,
                               uint& client) {
    
    transferFlags[client] = CLIENT_STATE_NONE;
}

void __stdcall PlayerLaunch_AFTER(uint& ship, uint& client) {
    

    if (transferFlags[client] == CLIENT_STATE_TRANSFER) {
        if (!ValidateCargo(client)) {
            PrintUserCmdText(client, STR_INFO2);
            return;
        }

        transferFlags[client] = CLIENT_STATE_NONE;
        MoveClient(client, config->targetBaseId);
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Conn(uint iClientID, const std::wstring &wscParam) {
    // Prohibit jump if in a restricted system or in the target system
    uint system = 0;
    pub::Player::GetSystem(iClientID, system);
    if (system == config->restrictedSystemId ||
        system == config->targetSystemId || GetCustomBaseForClient(iClientID)) {
        PrintUserCmdText(iClientID,
                         L"ERR Cannot use command in this system or base");
        return;
    }

    if (!IsDockedClient(iClientID)) {
        PrintUserCmdText(iClientID, STR_INFO1);
        return;
    }

    if (!ValidateCargo(iClientID)) {
        PrintUserCmdText(iClientID, STR_INFO2);
        return;
    }

    StoreReturnPointForClient(iClientID);
    PrintUserCmdText(iClientID, L"Redirecting undock to Arena.");
    transferFlags[iClientID] = CLIENT_STATE_TRANSFER;
}

void UserCmd_Return(uint iClientID, const std::wstring &wscParam) {
    if (!ReadReturnPointForClient(iClientID)) {
        PrintUserCmdText(iClientID, L"No return possible");
        return;
    }

    if (!IsDockedClient(iClientID)) {
        PrintUserCmdText(iClientID, STR_INFO1);
        return;
    }

    if (!CheckReturnDock(iClientID, config->targetBaseId)) {
        PrintUserCmdText(iClientID, L"Not in correct base");
        return;
    }

    if (!ValidateCargo(iClientID)) {
        PrintUserCmdText(iClientID, STR_INFO2);
        return;
    }

    PrintUserCmdText(iClientID, L"Redirecting undock to previous base");
    transferFlags[iClientID] = CLIENT_STATE_RETURN;
}

// Process user input
bool UserCmd_Process(uint& iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

// Hook on /help
EXPORT void UserCmd_Help(uint& iClientID, const std::wstring &wscParam) {
    PrintUserCmdText(iClientID, config->wscCommand);
    PrintUserCmdText(iClientID, L"Beams you to the Arena system.");
    PrintUserCmdText(iClientID, L"/return ");
    PrintUserCmdText(iClientID, L"Returns you to the previous base.");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (set_scCfgFile.length() > 0)
            LoadSettings();
    }
    return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Arena by MadHunter");
    pi->shortName("arena");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
    pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect);
    pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch_AFTER, HookStep::After);
    pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
}