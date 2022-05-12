// Misc Commands plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include <FLHook.h>
#include <plugin.h>
#include <FLCoreCommon.h>
#include <FLCoreServer.h>

ReturnCode returncode = ReturnCode::Default;

struct INFO {
    INFO() : bLightsOn(false), bShieldsDown(false), bSelfDestruct(false) {}

    /// Lights on/off
    bool bLightsOn;

    /// Shields up/down
    bool bShieldsDown;

    /// Self destruct
    bool bSelfDestruct;
};

/** A list of clients that are being smited */
std::map<uint, INFO> mapInfo;
typedef std::map<uint, INFO, std::less<uint>>::value_type mapInfo_map_pair_t;
typedef std::map<uint, INFO, std::less<uint>>::iterator mapInfo_map_iter_t;

std::wstring set_wscStuckMsg = L"Attention! Stand clear. Towing %player";
std::wstring set_wscDiceMsg = L"%player rolled %number";
std::wstring set_wscCoinMsg = L"%player tossed %result";

/// ID of music to play when smiting players.
uint set_iSmiteMusicID = 0;

/// Cost to drop reputation changes.
int set_iRepdropCost = 0;

/// Load the configuration
void LoadSettings() {
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\misc_commands.cfg";

    // Load generic settings
    set_iRepdropCost = IniGetI(scPluginCfgFile, "General", "RepDropCost", 0);

    set_wscStuckMsg = stows(IniGetS(scPluginCfgFile, "General", "StuckMsg",
                                    "Attention! Stand clear. Towing %player"));
    set_wscDiceMsg = stows(IniGetS(scPluginCfgFile, "General", "DiceMsg",
                                   "%player rolled %number of %max"));
    set_wscCoinMsg = stows(IniGetS(scPluginCfgFile, "General", "CoinMsg",
                                   "%player tossed %result"));

    set_iSmiteMusicID = CreateID(
        IniGetS(scPluginCfgFile, "General", "SmiteMusic", "music_danger")
            .c_str());
}

/** Clean up when a client disconnects */
void ClearClientInfo(uint& iClientID) {
    if (mapInfo[iClientID].bSelfDestruct) {
        mapInfo[iClientID].bSelfDestruct = false;
        uint dummy[3] = {0};
        pub::Player::SetShipAndLoadout(iClientID, CreateID("dsy_ge_fighter"),
                                       (const EquipDescVector &)dummy);
    }
    mapInfo.erase(iClientID);
}

void CharacterInfoReq(uint& iClientID, bool& p2) {
    if (mapInfo[iClientID].bSelfDestruct) {
        mapInfo[iClientID].bSelfDestruct = false;
        uint dummy[3] = {0};
        pub::Player::SetShipAndLoadout(iClientID, CreateID("dsy_ge_fighter"),
                                       (const EquipDescVector &)dummy);
    }
}

/** One second timer */
void Timer() {
    // Drop player sheilds and keep them down.
    for (mapInfo_map_iter_t iter = mapInfo.begin(); iter != mapInfo.end();
         iter++) {
        if (iter->second.bShieldsDown) {
            HKPLAYERINFO p;
            if (HkGetPlayerInfo((const wchar_t *)Players.GetActiveCharacterName(
                                    iter->first),
                                p, false) == HKE_OK &&
                p.iShip) {
                pub::SpaceObj::DrainShields(p.iShip);
            }
        }
    }
}

static void SetLights(uint iClientID, bool bOn) {
    uint iShip;
    pub::Player::GetShip(iClientID, iShip);
    if (!iShip) {
        PrintUserCmdText(iClientID, L"ERR Not in space");
        return;
    }

    bool bLights = false;
    st6::list<EquipDesc> &eqLst = Players[iClientID].equipDescList.equip;
    for (auto eq = eqLst.begin(); eq != eqLst.end(); eq++) {
        std::string hp = ToLower(eq->szHardPoint.value);
        if (hp.find("dock") != std::string::npos) {
            XActivateEquip ActivateEq;
            ActivateEq.bActivate = bOn;
            ActivateEq.iSpaceID = iShip;
            ActivateEq.sID = eq->sID;
            Server.ActivateEquip(iClientID, ActivateEq);
            bLights = true;
        }
    }

    if (bLights)
        PrintUserCmdText(iClientID, L" Lights %s", bOn ? L"on" : L"off");
    else
        PrintUserCmdText(iClientID, L"Light control not available");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// User Commands
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Print current position */
void UserCmd_Pos(uint iClientID, const std::wstring &wscParam) {
    HKPLAYERINFO p;
    if (HkGetPlayerInfo(
            (const wchar_t *)Players.GetActiveCharacterName(iClientID), p,
            false) != HKE_OK ||
        p.iShip == 0) {
        PrintUserCmdText(iClientID, L"ERR Not in space");
        return;
    }

    Vector pos;
    Matrix rot;
    pub::SpaceObj::GetLocation(p.iShip, pos, rot);

    Vector erot = MatrixToEuler(rot);

    wchar_t buf[100];
    _snwprintf_s(buf, sizeof(buf),
                 L"Position %0.0f %0.0f %0.0f Orient %0.0f %0.0f %0.0f", pos.x,
                 pos.y, pos.z, erot.x, erot.y, erot.z);
    PrintUserCmdText(iClientID, buf);
}

/** Move a ship a little if it is stuck in the base */
void UserCmd_Stuck(uint iClientID, const std::wstring &wscParam) {
    std::wstring wscCharname =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);

    HKPLAYERINFO p;
    if (HkGetPlayerInfo(wscCharname, p, false) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR Not in space");
        return;
    }

    Vector dir1;
    Vector dir2;
    pub::SpaceObj::GetMotion(p.iShip, dir1, dir2);
    if (dir1.x > 5 || dir1.y > 5 || dir1.z > 5) {
        PrintUserCmdText(iClientID, L"ERR Ship is moving");
        return;
    }

    Vector pos;
    Matrix rot;
    pub::SpaceObj::GetLocation(p.iShip, pos, rot);
    pos.x += 15;
    pos.y += 15;
    pos.z += 15;
    HkRelocateClient(iClientID, pos, rot);

    std::wstring wscMsg = set_wscStuckMsg;
    wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
    PrintLocalUserCmdText(iClientID, wscMsg, 6000.0f);
}

/** A command to help remove any affiliation that you might have */
void UserCmd_DropRep(uint iClientID, const std::wstring &wscParam) {
    HK_ERROR err;

    std::wstring wscCharname =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);

    std::wstring wscRepGroupNick;
    if (HkFLIniGet(wscCharname, L"rep_group", wscRepGroupNick) != HKE_OK ||
        wscRepGroupNick.length() == 0) {
        PrintUserCmdText(iClientID, L"ERR No affiliation");
        return;
    }

    // Read the current number of credits for the player
    // and check that the character has enough cash.
    int iCash = 0;
    if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR %s", HkErrGetText(err).c_str());
        return;
    }
    if (set_iRepdropCost > 0 && iCash < set_iRepdropCost) {
        PrintUserCmdText(iClientID, L"ERR Insufficient credits");
        return;
    }

    float fValue = 0.0f;
    if ((err = HkGetRep(wscCharname, wscRepGroupNick, fValue)) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR %s", HkErrGetText(err).c_str());
        return;
    }

    HkSetRep(wscCharname, wscRepGroupNick, 0.599f);
    PrintUserCmdText(
        iClientID, L"OK Reputation dropped, logout for change to take effect.");

    // Remove cash if we're charging for it.
    if (set_iRepdropCost > 0) {
        HkAddCash(wscCharname, 0 - set_iRepdropCost);
    }
}

/** Throw the dice and tell all players within 6 km */
void UserCmd_Dice(uint iClientID, const std::wstring &wscParam) {
    std::wstring wscCharname =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);

    int max = ToInt(GetParam(wscParam, ' ', 0));
    if (max <= 1)
        max = 6;

    uint number = (rand() % max) + 1;
    std::wstring wscMsg = set_wscDiceMsg;
    wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
    wscMsg = ReplaceStr(wscMsg, L"%number", std::to_wstring(number));
    wscMsg = ReplaceStr(wscMsg, L"%max", std::to_wstring(max));
    PrintLocalUserCmdText(iClientID, wscMsg, 6000.0f);
}

/** Throw the dice and tell all players within 6 km */
void UserCmd_Coin(uint iClientID, const std::wstring &wscParam) {
    std::wstring wscCharname =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);

    uint number = (rand() % 2);
    std::wstring wscMsg = set_wscCoinMsg;
    wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
    wscMsg =
        ReplaceStr(wscMsg, L"%result", (number == 1) ? L"heads" : L"tails");
    PrintLocalUserCmdText(iClientID, wscMsg, 6000.0f);
}

void UserCmd_Lights(uint iClientID, const std::wstring &wscParam) {
    mapInfo[iClientID].bLightsOn = !mapInfo[iClientID].bLightsOn;
    SetLights(iClientID, mapInfo[iClientID].bLightsOn);
}

void UserCmd_SelfDestruct(uint iClientID, const std::wstring &wscParam) {
    IObjInspectImpl *obj = HkGetInspect(iClientID);
    if (!obj) {
        PrintUserCmdText(iClientID, L"Self destruct prohibited. Not in space.");
        return;
    }

    if (wscParam == L"0000") {
        PrintUserCmdText(iClientID, L"Self destruct enabled. Standby.");
        PrintUserCmdText(iClientID, L"Ejecting pod...");
        HkLightFuse((IObjRW *)obj, CreateID("death_comm"), 0.0f, 0.0f, 0.0f);
        mapInfo[iClientID].bSelfDestruct = true;
    } else {
        PrintUserCmdText(iClientID,
                         L"WARNING! SELF DESTRUCT WILL COMPLETELY AND "
                         L"PERMANENTLY DESTROY SHIP.");
        PrintUserCmdText(
            iClientID,
            L"WARNING! WARNING! SECURITY CONFIRMATION REQUIRED. TYPE");
        PrintUserCmdText(iClientID, L"/selfdestruct 0000");
    }
}

void UserCmd_Shields(uint iClientID, const std::wstring &wscParam) {
    mapInfo[iClientID].bShieldsDown = !mapInfo[iClientID].bShieldsDown;
    PrintUserCmdText(iClientID, L"Shields %s",
                     mapInfo[iClientID].bShieldsDown ? L"Disabled"
                                                     : L"Enabled");
}

// Client command processing
USERCMD UserCmds[] = {
    {L"/lights", UserCmd_Lights},
    {L"/shields", UserCmd_Shields},
    {L"/pos", UserCmd_Pos},
    {L"/stuck", UserCmd_Stuck},
    {L"/droprep", UserCmd_DropRep},
    {L"/dice", UserCmd_Dice},
    {L"/coin", UserCmd_Coin},
};

// Process user input
bool UserCmd_Process(uint& iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

// Hook on /help
EXPORT void UserCmd_Help(uint& iClientID, const std::wstring &wscParam) {
    for (auto& uc : UserCmds) {
        PrintUserCmdText(iClientID, uc.wszCmd);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Admin Commands
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Smite all players in radar range */
void AdminCmd_SmiteAll(CCmds *cmds) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission");
        return;
    }

    HKPLAYERINFO adminPlyr;
    if (HkGetPlayerInfo(cmds->GetAdminName(), adminPlyr, false) != HKE_OK ||
        adminPlyr.iShip == 0) {
        cmds->Print(L"ERR Not in space");
        return;
    }

    bool bKillAll = cmds->ArgStr(1) == L"die";

    Vector vFromShipLoc;
    Matrix mFromShipDir;
    pub::SpaceObj::GetLocation(adminPlyr.iShip, vFromShipLoc, mFromShipDir);

    pub::Audio::Tryptich music;
    music.iDunno = 0;
    music.iDunno2 = 0;
    music.iDunno3 = 0;
    music.iMusicID = set_iSmiteMusicID;
    pub::Audio::SetMusic(adminPlyr.iClientID, music);

    // For all players in system...
    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
        // Get the this player's current system and location in the system.
        uint iClientID = HkGetClientIdFromPD(pPD);
        if (iClientID == adminPlyr.iClientID)
            continue;

        uint iClientSystem = 0;
        pub::Player::GetSystem(iClientID, iClientSystem);
        if (adminPlyr.iSystem != iClientSystem)
            continue;

        uint iShip;
        pub::Player::GetShip(iClientID, iShip);

        Vector vShipLoc;
        Matrix mShipDir;
        pub::SpaceObj::GetLocation(iShip, vShipLoc, mShipDir);

        // Is player within scanner range (15K) of the sending char.
        float fDistance = HkDistance3D(vShipLoc, vFromShipLoc);
        if (fDistance > 14999)
            continue;

        pub::Audio::Tryptich music;
        music.iDunno = 0;
        music.iDunno2 = 0;
        music.iDunno3 = 0;
        music.iMusicID = set_iSmiteMusicID;
        pub::Audio::SetMusic(iClientID, music);

        mapInfo[iClientID].bShieldsDown = true;

        if (bKillAll) {
            IObjInspectImpl *obj = HkGetInspect(iClientID);
            if (obj) {
                HkLightFuse((IObjRW *)obj, CreateID("death_comm"), 0.0f, 0.0f,
                            0.0f);
            }
        }
    }

    cmds->Print(L"OK");
    return;
}

bool ExecuteCommandString(CCmds *cmds, const std::wstring &wscCmd) {
    if (IS_CMD("smiteall")) {
        returncode = ReturnCode::SkipAll;
        AdminCmd_SmiteAll(cmds);
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Misc Commands");
    pi->shortName("MiscCommands");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
    pi->versionMajor(PluginMajorVersion::VERSION_04);
    pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
    pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
    pi->emplaceHook(HookedCall::FLHook__TimerNPCAndF1Check, &Timer);
    pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
    pi->emplaceHook(HookedCall::IServerImpl__CharacterInfoReq, &CharacterInfoReq);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}