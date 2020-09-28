// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// includes
#include <FLHook.h>
#include <float.h>
#include <list>
#include <math.h>
#include <plugin.h>
#include <set>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

#include "CrashCatcher.h"
#include "Main.h"
#include "StartupCache.h"
#include "wildcards.h"

// Current configuration
int set_iPluginDebug = 0;

/// True if loot logging is enabled
bool set_bLogLooting = false;

// Disable various user features
bool set_bEnablePimpShip = false;
bool set_bEnableRenameMe = false;
bool set_bEnableMoveChar = false;
bool set_bEnableRestart = false;
bool set_bEnableGiveCash = false;
bool set_bLocalTime = false;
bool set_bEnableLoginSound = false;
bool set_bEnableMe = false;
bool set_bEnableDo = false;

float set_fSpinProtectMass;
float set_fSpinImpulseMultiplier;

// Vector to store sounds
std::vector<int> sounds;

/** A return code to indicate to FLHook if we want the hook processing to
 * continue. */
PLUGIN_RETURNCODE returncode;

// TODO: detect frequent /stuck use and blow up ship (or something).

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadSettings();

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    srand((uint)time(0));

    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (set_scCfgFile.length() > 0)
            LoadSettings();
        HkLoadStringDLLs();
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        CrashCatcher::Shutdown();
        HkUnloadStringDLLs();
    }
    return true;
}

/// Hook will call this function after calling a plugin function to see if we
/// the processing to continue
EXPORT PLUGIN_RETURNCODE Get_PluginReturnCode() { return returncode; }

/// Load the configuration
void LoadSettings() {
    returncode = DEFAULT_RETURNCODE;

    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\playercntl.cfg";

    set_iPluginDebug = IniGetI(scPluginCfgFile, "General", "Debug", 0);
    if (set_iPluginDebug)
        ConPrint(L"NOTICE: player control debug=%d\n", set_iPluginDebug);

    set_bLogLooting = IniGetB(scPluginCfgFile, "General", "LogLooting", false);
    set_bEnableMoveChar =
        IniGetB(scPluginCfgFile, "General", "EnableMoveChar", false);
    set_bEnableRenameMe =
        IniGetB(scPluginCfgFile, "General", "EnableRenameMe", false);
    set_bEnablePimpShip =
        IniGetB(scPluginCfgFile, "General", "EnablePimpShip", false);
    set_bEnableRestart =
        IniGetB(scPluginCfgFile, "General", "EnableRestart", false);
    set_bEnableGiveCash =
        IniGetB(scPluginCfgFile, "General", "EnableGiveCash", false);
    set_bEnableLoginSound =
        IniGetB(scPluginCfgFile, "General", "EnableLoginSound", false);
    set_bEnableMe = IniGetB(scPluginCfgFile, "General", "EnableMe", false);
    set_bEnableDo = IniGetB(scPluginCfgFile, "General", "EnableDo", false);

    set_fSpinProtectMass =
        IniGetF(scPluginCfgFile, "General", "SpinProtectionMass", 180.0f);
    set_fSpinImpulseMultiplier =
        IniGetF(scPluginCfgFile, "General", "SpinProtectionMultiplier", -1.0f);

    set_bLocalTime = IniGetB(scPluginCfgFile, "General", "LocalTime", false);

    ZoneUtilities::ReadUniverse(nullptr);
    EquipmentUtilities::ReadIniNicknames();
    Rename::LoadSettings(scPluginCfgFile);
    GiveCash::LoadSettings(scPluginCfgFile);
    MiscCmds::LoadSettings(scPluginCfgFile);
    PimpShip::LoadSettings(scPluginCfgFile);
    HyperJump::LoadSettings(scPluginCfgFile);
    PurchaseRestrictions::LoadSettings(scPluginCfgFile);
    IPBans::LoadSettings(scPluginCfgFile);
    CargoDrop::LoadSettings(scPluginCfgFile);
    Restart::LoadSettings(scPluginCfgFile);
    RepFixer::LoadSettings(scPluginCfgFile);
    Message::LoadSettings(scPluginCfgFile);
    SystemSensor::LoadSettings(scPluginCfgFile);
    CrashCatcher::Init();

    // Load sounds from config if enabled
    if (set_bEnableLoginSound == true) {
        INI_Reader ini;
        if (ini.open(scPluginCfgFile.c_str(), false)) {
            while (ini.read_header()) {
                if (ini.is_header("Sounds")) {
                    while (ini.read_value()) {
                        if (ini.is_value("sound")) {
                            sounds.push_back(CreateID(ini.get_value_string(0)));
                        }
                    }
                }
            }
            ini.close();
        }
    }
}

/** Clean up when a client disconnects */
void ClearClientInfo(uint iClientID) {
    returncode = DEFAULT_RETURNCODE;
    MiscCmds::ClearClientInfo(iClientID);
    HyperJump::ClearClientInfo(iClientID);
    CargoDrop::ClearClientInfo(iClientID);
    IPBans::ClearClientInfo(iClientID);
    Message::ClearClientInfo(iClientID);
    PurchaseRestrictions::ClearClientInfo(iClientID);
    AntiJumpDisconnect::ClearClientInfo(iClientID);
    SystemSensor::ClearClientInfo(iClientID);
}

/** One second timer */
void HkTimer() {
    returncode = DEFAULT_RETURNCODE;
    MiscCmds::Timer();
    HyperJump::Timer();
    CargoDrop::Timer();
    Message::Timer();
    Restart::Timer();
    Rename::Timer();
}

/// Hook for ship distruction. It's easier to hook this than the PlayerDeath
/// one. Drop a percentage of cargo + some loot representing ship bits.
void SendDeathMsg(const std::wstring &wscMsg, uint iSystem,
                  uint iClientIDVictim, uint iClientIDKiller) {
    returncode = NOFUNCTIONCALL;

    HyperJump::SendDeathMsg(wscMsg, iSystem, iClientIDVictim, iClientIDKiller);
    CargoDrop::SendDeathMsg(wscMsg, iSystem, iClientIDVictim, iClientIDKiller);
    Message::SendDeathMsg(wscMsg, iSystem, iClientIDVictim, iClientIDKiller);

    const wchar_t *victim =
        (const wchar_t *)Players.GetActiveCharacterName(iClientIDVictim);
    const wchar_t *killer =
        (const wchar_t *)Players.GetActiveCharacterName(iClientIDKiller);
    if (victim && killer) {
        AddLog("NOTICE: Death charname=%s killername=%s system=%08x",
               wstos(victim).c_str(), wstos(killer).c_str(), iSystem);
    }
}

void __stdcall HkCb_AddDmgEntry(DamageList *dmgList, unsigned short p1,
                                float p2, enum DamageEntry::SubObjFate p3) {
    returncode = DEFAULT_RETURNCODE;
}

static bool IsDockingAllowed(uint iShip, uint iDockTarget, uint iClientID) {
    // If the player's rep is less/equal -0.55 to the owner of the station
    // then refuse the docking request
    int iSolarRep;
    pub::SpaceObj::GetSolarRep(iDockTarget, iSolarRep);

    int iPlayerRep;
    pub::SpaceObj::GetRep(iShip, iPlayerRep);

    float fAttitude = 0.0f;
    pub::Reputation::GetAttitude(iSolarRep, iPlayerRep, fAttitude);
    if (fAttitude <= -0.55f) {
        pub::Player::SendNNMessage(iClientID,
                                   pub::GetNicknameId("info_access_denied"));
        std::wstring wscMsg[3] = {
            L"Access Denied! Request to dock denied. We don't want your kind "
            L"around here.",
            L"Access Denied! Docking request rejected. Your papers are no "
            L"good.",
            L"Access Denied! You can't dock here. Your reputation stinks."};
        PrintUserCmdText(iClientID, wscMsg[rand() % 3]);
        return false;
    }

    return true;
}

namespace HkIEngine {
int __cdecl Dock_Call(unsigned int const &iShip,
                      unsigned int const &iDockTarget, int iCancel,
                      enum DOCK_HOST_RESPONSE response) {
    returncode = DEFAULT_RETURNCODE;

    uint iClientID = HkGetClientIDByShip(iShip);
    if (iClientID && response == PROCEED_DOCK) {
        uint iTypeID;
        pub::SpaceObj::GetType(iDockTarget, iTypeID);
        if (iTypeID == OBJ_DOCKING_RING || iTypeID == OBJ_STATION) {
            if (!IsDockingAllowed(iShip, iDockTarget, iClientID)) {
                // AddLog("INFO: Docking suppressed docktarget=%u charname=%s",
                // iDockTarget,
                // wstos(Players.GetActiveCharacterName(iClientID)).c_str());
                returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                return 0;
            }

            // Print out a message when a player ship docks.
            std::wstring wscMsg =
                L"Traffic control alert: %player has requested to dock";
            wscMsg = ReplaceStr(
                wscMsg, L"%player",
                (const wchar_t *)Players.GetActiveCharacterName(iClientID));
            PrintLocalUserCmdText(iClientID, wscMsg, 15000);
        }
    }

    SystemSensor::Dock_Call(iShip, iDockTarget, iCancel, response);
    return 0;
}
} // namespace HkIEngine

namespace HkIServerImpl {
bool __stdcall Startup_AFTER(struct SStartupInfo const &p1) {
    returncode = DEFAULT_RETURNCODE;
    StartupCache::Done();
    return true;
}

bool __stdcall Startup(struct SStartupInfo const &p1) {
    returncode = DEFAULT_RETURNCODE;
    StartupCache::Init();
    return true;
}

// The startup cache disables reading of the banned file. Check this manually on
// login and boot the player if they are banned.
void __stdcall Login(struct SLoginInfo const &li, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;

    // Player sound when player logs in (if enabled)
    pub::Audio::PlaySoundEffect(iClientID, sounds[rand() % sounds.size()]);

    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    if (acc) {
        std::wstring wscDir;
        HkGetAccountDirName(acc, wscDir);

        char szDataPath[MAX_PATH];
        GetUserDataPath(szDataPath);

        std::string path = std::string(szDataPath) + "\\Accts\\MultiPlayer\\" +
                           wstos(wscDir) + "\\banned";

        FILE *file = fopen(path.c_str(), "r");
        if (file) {
            fclose(file);

            // Ban the player
            st6::wstring flStr((ushort *)acc->wszAccID);
            Players.BanAccount(flStr, true);

            // Kick them
            acc->ForceLogout();

            // And stop further processing.
            returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        }
    }
}

void __stdcall CreateNewCharacter(struct SCreateCharacterInfo const &si,
                                  unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    if (Rename::CreateNewCharacter(si, iClientID)) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        Server.CharacterInfoReq(iClientID, true);
    }
}

void __stdcall RequestEvent(int iIsFormationRequest, unsigned int iShip,
                            unsigned int iDockTarget, unsigned int p4,
                            unsigned long p5, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    if (iClientID) {
        if (!iIsFormationRequest) {
            uint iTargetTypeID;
            pub::SpaceObj::GetType(iDockTarget, iTargetTypeID);
            if (iTargetTypeID == OBJ_DOCKING_RING ||
                iTargetTypeID == OBJ_STATION) {
                if (!IsDockingAllowed(iShip, iDockTarget, iClientID)) {
                    // AddLog("INFO: Docking suppressed docktarget=%u
                    // charname=%s", iDockTarget,
                    // wstos(Players.GetActiveCharacterName(iClientID)).c_str());
                    returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                    return;
                }
            }
        }
    }
}

void __stdcall PlayerLaunch(unsigned int iShip, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;

    // TODO if the player is kicked abort processing.
    IPBans::PlayerLaunch(iShip, iClientID);
    RepFixer::PlayerLaunch(iShip, iClientID);
    Message::PlayerLaunch(iShip, iClientID);
    GiveCash::PlayerLaunch(iShip, iClientID);
    PurchaseRestrictions::PlayerLaunch(iShip, iClientID);
    HyperJump::PlayerLaunch(iShip, iClientID);
    SystemSensor::PlayerLaunch(iShip, iClientID);
}

void __stdcall PlayerLaunch_AFTER(unsigned int iShip, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
}

void __stdcall BaseEnter(unsigned int iBaseID, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    IPBans::BaseEnter(iBaseID, iClientID);
    RepFixer::BaseEnter(iBaseID, iClientID);
    Message::BaseEnter(iBaseID, iClientID);
    GiveCash::BaseEnter(iBaseID, iClientID);
    PurchaseRestrictions::BaseEnter(iBaseID, iClientID);
    MiscCmds::BaseEnter(iBaseID, iClientID);
}

void __stdcall BaseEnter_AFTER(unsigned int iBaseID, unsigned int iClientID) {
    float fValue;
    if (HKGetShipValue(
            (const wchar_t *)Players.GetActiveCharacterName(iClientID),
            fValue) == HKE_OK) {
        if (fValue > 2100000000.0f) {
            AddLog("ERROR: Possible corrupt ship charname=%s asset_value=%0.0f",
                   wstos((const wchar_t *)Players.GetActiveCharacterName(
                             iClientID))
                       .c_str(),
                   fValue);
        }
    }
}

void __stdcall LocationEnter(unsigned int iLocationID, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    PimpShip::LocationEnter(iLocationID, iClientID);
}

void __stdcall DisConnect(unsigned int iClientID, enum EFLConnection state) {
    returncode = DEFAULT_RETURNCODE;
    ClearClientInfo(iClientID);
}

void __stdcall CharacterSelect_AFTER(struct CHARACTER_ID const &charId,
                                     unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    ClearClientInfo(iClientID);
    Rename::CharacterSelect_AFTER(charId, iClientID);
}

void __stdcall JumpInComplete_AFTER(unsigned int iSystem, unsigned int iShip) {
    returncode = DEFAULT_RETURNCODE;
    uint iClientID = HkGetClientIDByShip(iShip);
    if (iClientID) {
        AntiJumpDisconnect::JumpInComplete(iSystem, iShip, iClientID);
        SystemSensor::JumpInComplete(iSystem, iShip, iClientID);
    }

    // Make player damageable once the ship has jumped in system.
    pub::SpaceObj::SetInvincible(iShip, false, false, 0);
}

void __stdcall SystemSwitchOutComplete(unsigned int iShip,
                                       unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    // Make player invincible to fix JHs/JGs near mine fields sometimes
    // exploding player while jumping (in jump tunnel)
    pub::SpaceObj::SetInvincible(iShip, true, true, 0);
    AntiJumpDisconnect::SystemSwitchOutComplete(iShip, iClientID);
    if (HyperJump::SystemSwitchOutComplete(iShip, iClientID))
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
}

void __stdcall SPObjCollision(struct SSPObjCollisionInfo const &ci,
                              unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;

    // If spin protection is off, do nothing.
    if (set_fSpinProtectMass == -1.0f)
        return;

    // If the target is not a player, do nothing.
    // uint iClientIDTarget = HkGetClientIDByShip(ci.dwTargetShip);
    // if (iClientIDTarget<=0)
    //	return;

    float target_mass;
    pub::SpaceObj::GetMass(ci.iColliderObjectID, target_mass);

    uint client_ship;
    pub::Player::GetShip(iClientID, client_ship);

    float client_mass;
    pub::SpaceObj::GetMass(client_ship, client_mass);

    // Don't do spin protect unless the hit ship is big
    if (target_mass < set_fSpinProtectMass)
        return;

    // Don't do spin protect unless the hit ship is 2 times larger than the
    // hitter
    if (target_mass < client_mass * 2)
        return;

    Vector V1, V2;
    pub::SpaceObj::GetMotion(ci.iColliderObjectID, V1, V2);
    V1.x *= set_fSpinImpulseMultiplier * client_mass;
    V1.y *= set_fSpinImpulseMultiplier * client_mass;
    V1.z *= set_fSpinImpulseMultiplier * client_mass;
    V2.x *= set_fSpinImpulseMultiplier * client_mass;
    V2.y *= set_fSpinImpulseMultiplier * client_mass;
    V2.z *= set_fSpinImpulseMultiplier * client_mass;
    pub::SpaceObj::AddImpulse(ci.iColliderObjectID, V1, V2);
}

void __stdcall GFGoodBuy(struct SGFGoodBuyInfo const &gbi,
                         unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    if (PurchaseRestrictions::GFGoodBuy(gbi, iClientID)) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    }
}

void __stdcall ReqAddItem(unsigned int goodID, char const *hardpoint, int count,
                          float status, bool mounted, uint iClientID) {
    returncode = DEFAULT_RETURNCODE;
    if (PurchaseRestrictions::ReqAddItem(goodID, hardpoint, count, status,
                                         mounted, iClientID)) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    }
}

void __stdcall ReqChangeCash(int iMoneyDiff, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    if (PurchaseRestrictions::ReqChangeCash(iMoneyDiff, iClientID)) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    }
}

void __stdcall ReqSetCash(int iMoney, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    if (PurchaseRestrictions::ReqSetCash(iMoney, iClientID)) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    }
}

void __stdcall ReqEquipment(class EquipDescList const &eqDesc,
                            unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    if (PurchaseRestrictions::ReqEquipment(eqDesc, iClientID)) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    }
}

void __stdcall ReqHullStatus(float fStatus, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    if (PurchaseRestrictions::ReqHullStatus(fStatus, iClientID)) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    }
}

void __stdcall ReqShipArch(unsigned int iArchID, unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    if (PurchaseRestrictions::ReqShipArch(iArchID, iClientID)) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    }
}

std::map<uint, mstime> mapSaveTimes;

void Timer() {
    mstime currTime = GetTimeInMS();
    for (auto &t : mapSaveTimes) {
        uint iClientID = t.first;
        if (t.second != 0 && t.second < currTime) {
            if (HkIsValidClientID(iClientID) &&
                !HkIsInCharSelectMenu(iClientID))
                HkSaveChar(t.first);
            t.second = 0;
        }
    }
}

// Save after a tractor to prevent cargo duplication loss on crash
void __stdcall TractorObjects(unsigned int iClientID,
                              struct XTractorObjects const &objs) {
    returncode = DEFAULT_RETURNCODE;
    if (mapSaveTimes[iClientID] == 0) {
        mapSaveTimes[iClientID] = GetTimeInMS() + 60000;
    }
}

// Save after jettison to reduce chance of duplication on crash
void __stdcall JettisonCargo(unsigned int iClientID,
                             struct XJettisonCargo const &objs) {
    if (mapSaveTimes[iClientID] == 0) {
        mapSaveTimes[iClientID] = GetTimeInMS() + 60000;
    }
}

void __stdcall SetTarget(uint uClientID, struct XSetTarget const &p2) {
    returncode = DEFAULT_RETURNCODE;
    Message::SetTarget(uClientID, p2);
}

void __stdcall CharacterInfoReq(unsigned int iClientID, bool p2) {
    returncode = DEFAULT_RETURNCODE;
    Message::CharacterInfoReq(iClientID, p2);
    AntiJumpDisconnect::CharacterInfoReq(iClientID, p2);
    MiscCmds::CharacterInfoReq(iClientID, p2);
}

void __stdcall SubmitChat(struct CHAT_ID cId, unsigned long lP1,
                          void const *rdlReader, struct CHAT_ID cIdTo,
                          int iP2) {
    returncode = DEFAULT_RETURNCODE;

    // If we're in a base then reset the base kick time if the
    // player is chatting to stop the player being kicked.
    if (ClientInfo[cId.iID].iBaseEnterTime) {
        ClientInfo[cId.iID].iBaseEnterTime = (uint)time(0);
    }

    // The message subsystem may suppress some chat messages.
    if (Message::SubmitChat(cId, lP1, rdlReader, cIdTo, iP2)) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    }
}

void __stdcall GoTradelane(unsigned int iClientID,
                           struct XGoTradelane const &xgt) {
    returncode = DEFAULT_RETURNCODE;
    SystemSensor::GoTradelane(iClientID, xgt);
}

void __stdcall StopTradelane(unsigned int iClientID, unsigned int p1,
                             unsigned int p2, unsigned int p3) {
    returncode = DEFAULT_RETURNCODE;
    SystemSensor::StopTradelane(iClientID, p1, p2, p3);
}

void __stdcall SPObjUpdate(struct SSPObjUpdateInfo const &ui,
                           unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    CargoDrop::SPObjUpdate(ui, iClientID);
}
} // namespace HkIServerImpl

void __stdcall HkCb_SendChat(uint iClientID, uint iTo, uint iSize, void *pRDL) {
    returncode = DEFAULT_RETURNCODE;

    if (Message::HkCb_SendChat(iClientID, iTo, iSize, pRDL)) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    }
}

int __stdcall HkCB_MissileTorpHit(char *ECX, char *p1, DamageList *dmg) {
    returncode = DEFAULT_RETURNCODE;

    char *szP;
    memcpy(&szP, ECX + 0x10, 4);
    uint iClientID;
    memcpy(&iClientID, szP + 0xB4, 4);

    if (iClientID) {
        HyperJump::MissileTorpHit(iClientID, dmg);
    }
    return 0;
}

void __stdcall RequestBestPath(unsigned int p1, DWORD *p2, int p3) {
    returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    try {
        Server.RequestBestPath(p1, (unsigned char *)p2, p3);
    } catch (...) {
        AddLog(
            "ERROR: Exception in RequestBestPath p1=%d p2=%08x %08x %08x %08x "
            "%08x %08x %08x %08x %08x p3=%08x",
            p1, p2[0], p2[7], p2[3], p2[4], p2[5], p2[8], p2[9], p2[10],
            p2[12]);
    }
}

typedef bool (*_UserCmdProc)(uint, const std::wstring &, const std::wstring &,
                             const wchar_t *);

struct USERCMD {
    wchar_t *wszCmd;
    _UserCmdProc proc;
    wchar_t *usage;
};

// The user chat commands for this plugin
USERCMD UserCmds[] = {
    {L"/pos", MiscCmds::UserCmd_Pos, L"Usage: /pos"},
    {L"/stuck", MiscCmds::UserCmd_Stuck, L"Usage: /stuck"},
    {L"/droprep", MiscCmds::UserCmd_DropRep, L"Usage: /droprep"},
    {L"/dice", MiscCmds::UserCmd_Dice, L"Usage: /dice [max]"},
    {L"/coin", MiscCmds::UserCmd_Coin, L"Usage: /coin"},
    {L"/pimpship", PimpShip::UserCmd_PimpShip, L"Usage: /pimpship"},
    {L"/showsetup", PimpShip::UserCmd_ShowSetup, L"Usage: /showsetup"},
    {L"/showitems", PimpShip::UserCmd_ShowItems, L"Usage: /showitems"},
    {L"/setitem", PimpShip::UserCmd_ChangeItem, L"Usage: /setitem"},
    {L"/buynow", PimpShip::UserCmd_BuyNow, L"Usage: /buynow"},
    {L"/renameme", Rename::UserCmd_RenameMe,
     L"Usage: /renameme <charname> [password]"},
    {L"/movechar", Rename::UserCmd_MoveChar,
     L"Usage: /movechar <charname> <code>"},
    {L"/set movecharcode", Rename::UserCmd_SetMoveCharCode,
     L"Usage: /set movecharcode <code>"},
    {L"/restart", Restart::UserCmd_Restart, L"Usage: /restart <faction>"},
    {L"/showrestarts", Restart::UserCmd_ShowRestarts, L"Usage: /showrestarts"},
    {L"/givecash", GiveCash::UserCmd_GiveCash,
     L"Usage: /givecash <charname> <cash> [anon]"},
    {L"/sendcash", GiveCash::UserCmd_GiveCash,
     L"Usage: /givecash <charname> <cash> [anon]"},
    {L"/set cashcode", GiveCash::UserCmd_SetCashCode,
     L"Usage: /set cashcode <code>"},
    {L"/showcash", GiveCash::UserCmd_ShowCash,
     L"Usage: /showcash <charname> <code>"},
    {L"/drawcash", GiveCash::UserCmd_DrawCash,
     L"Usage: /drawcash <charname> <code> <cash>"},
    {L"/group", Message::UserCmd_BuiltInCmdHelp,
     L"Usage: /group <message> or /g ..."},
    {L"/local", Message::UserCmd_BuiltInCmdHelp,
     L"Usage: /local <message> or /l ...>"},
    {L"/system", Message::UserCmd_BuiltInCmdHelp,
     L"Usage: /system <message> or /s ..."},
    {L"/invite", Message::UserCmd_BuiltInCmdHelp,
     L"Usage: /invite <charname> or /i ..."},
    {L"/invite$", Message::UserCmd_BuiltInCmdHelp,
     L"Usage: /invite <clientid> or /i$ ..."},
    {L"/join", Message::UserCmd_BuiltInCmdHelp,
     L"Usage: /join <charname> or /j"},
    {L"/setmsg", Message::UserCmd_SetMsg, L"Usage: /setmsg <n> <msg text>"},
    {L"/showmsgs", Message::UserCmd_ShowMsgs, L""},
    {L"/0", Message::UserCmd_SMsg, L"Usage: /n (n=0-9)"},
    {L"/1", Message::UserCmd_SMsg, L"Usage: /n (n=0-9)"},
    {L"/2", Message::UserCmd_SMsg, L"Usage: /n (n=0-9)"},
    {L"/3", Message::UserCmd_SMsg, L"Usage: /n (n=0-9)"},
    {L"/4", Message::UserCmd_SMsg, L"Usage: /n (n=0-9)"},
    {L"/5", Message::UserCmd_SMsg, L"Usage: /n (n=0-9)"},
    {L"/6", Message::UserCmd_SMsg, L"Usage: /n (n=0-9)"},
    {L"/7", Message::UserCmd_SMsg, L"Usage: /n (n=0-9)"},
    {L"/8", Message::UserCmd_SMsg, L"Usage: /n (n=0-9)"},
    {L"/9", Message::UserCmd_SMsg, L"Usage: /n (n=0-9)"},
    {L"/l0", Message::UserCmd_LMsg, L"Usage: /ln (n=0-9)"},
    {L"/l1", Message::UserCmd_LMsg, L"Usage: /ln (n=0-9)"},
    {L"/l2", Message::UserCmd_LMsg, L"Usage: /ln (n=0-9)"},
    {L"/l3", Message::UserCmd_LMsg, L"Usage: /ln (n=0-9)"},
    {L"/l4", Message::UserCmd_LMsg, L"Usage: /ln (n=0-9)"},
    {L"/l5", Message::UserCmd_LMsg, L"Usage: /ln (n=0-9)"},
    {L"/l6", Message::UserCmd_LMsg, L"Usage: /ln (n=0-9)"},
    {L"/l7", Message::UserCmd_LMsg, L"Usage: /ln (n=0-9)"},
    {L"/l8", Message::UserCmd_LMsg, L"Usage: /ln (n=0-9)"},
    {L"/l9", Message::UserCmd_LMsg, L"Usage: /ln (n=0-9)"},
    {L"/g0", Message::UserCmd_GMsg, L"Usage: /gn (n=0-9)"},
    {L"/g1", Message::UserCmd_GMsg, L"Usage: /gn (n=0-9)"},
    {L"/g2", Message::UserCmd_GMsg, L"Usage: /gn (n=0-9)"},
    {L"/g3", Message::UserCmd_GMsg, L"Usage: /gn (n=0-9)"},
    {L"/g4", Message::UserCmd_GMsg, L"Usage: /gn (n=0-9)"},
    {L"/g5", Message::UserCmd_GMsg, L"Usage: /gn (n=0-9)"},
    {L"/g6", Message::UserCmd_GMsg, L"Usage: /gn (n=0-9)"},
    {L"/g7", Message::UserCmd_GMsg, L"Usage: /gn (n=0-9)"},
    {L"/g8", Message::UserCmd_GMsg, L"Usage: /gn (n=0-9)"},
    {L"/g9", Message::UserCmd_GMsg, L"Usage: /gn (n=0-9)"},
    {L"/t0", Message::UserCmd_SendToLastTarget, L"Usage: /tn (n=0-9)"},
    {L"/t1", Message::UserCmd_SendToLastTarget, L"Usage: /tn (n=0-9)"},
    {L"/t2", Message::UserCmd_SendToLastTarget, L"Usage: /tn (n=0-9)"},
    {L"/t3", Message::UserCmd_SendToLastTarget, L"Usage: /tn (n=0-9)"},
    {L"/t4", Message::UserCmd_SendToLastTarget, L"Usage: /tn (n=0-9)"},
    {L"/t5", Message::UserCmd_SendToLastTarget, L"Usage: /tn (n=0-9)"},
    {L"/t6", Message::UserCmd_SendToLastTarget, L"Usage: /tn (n=0-9)"},
    {L"/t7", Message::UserCmd_SendToLastTarget, L"Usage: /tn (n=0-9)"},
    {L"/t8", Message::UserCmd_SendToLastTarget, L"Usage: /tn (n=0-9)"},
    {L"/t9", Message::UserCmd_SendToLastTarget, L"Usage: /tn (n=0-9)"},
    {L"/target", Message::UserCmd_SendToLastTarget,
     L"Usage: /target <message> or /t ..."},
    {L"/t", Message::UserCmd_SendToLastTarget,
     L"Usage: /target <message> or /t ..."},
    {L"/reply", Message::UserCmd_ReplyToLastPMSender,
     L"Usage: /reply <message> or /r .."},
    {L"/r", Message::UserCmd_ReplyToLastPMSender,
     L"Usage: /reply <message> or /r ..."},
    {L"/privatemsg$", Message::UserCmd_PrivateMsgID,
     L"Usage: /privatemsg$ <clientid> <messsage> or /pm ..."},
    {L"/pm$", Message::UserCmd_PrivateMsgID,
     L"Usage: /privatemsg$ <clientid> <messsage> or /pm ..."},
    {L"/privatemsg", Message::UserCmd_PrivateMsg,
     L"Usage: /privatemsg <charname> <messsage> or /pm ..."},
    {L"/pm", Message::UserCmd_PrivateMsg,
     L"Usage: /privatemsg <charname> <messsage> or /pm ..."},
    {L"/factionmsg", Message::UserCmd_FactionMsg,
     L"Usage: /factionmsg <tag> <message> or /fm ..."},
    {L"/fm", Message::UserCmd_FactionMsg,
     L"Usage: /factionmsg <tag> <message> or /fm ..."},
    {L"/factioninvite", Message::UserCmd_FactionInvite,
     L"Usage: /factioninvite <tag> or /fi ..."},
    {L"/fi", Message::UserCmd_FactionInvite,
     L"Usage: /factioninvite <tag> or /fi ..."},
    {L"/lastpm", Message::UserCmd_ShowLastPMSender, L""},
    {L"/set chattime", Message::UserCmd_SetChatTime,
     L"Usage: /set chattime [on|off]"},
    {L"/help", Message::UserCmd_CustomHelp, L""},
    {L"/h", Message::UserCmd_CustomHelp, L""},
    {L"/?", Message::UserCmd_CustomHelp, L""},
    {L"/mail", Message::UserCmd_MailShow, L"Usage: /mail <msgnum>"},
    {L"/maildel", Message::UserCmd_MailDel, L"Usage: /maildel <msgnum>"},
    {L"/si", PlayerInfo::UserCmd_ShowInfo, L"Usage: /showinfo"},
    {L"/setinfo", PlayerInfo::UserCmd_SetInfo, L"Usage: /setinfo"},
    {L"/time", Message::UserCmd_Time, L""},
    {L"/time*", Message::UserCmd_Time, L""},
    {L"/showinfo", PlayerInfo::UserCmd_ShowInfo, L"Usage: /showinfo"},
    {L"/showinfo*", PlayerInfo::UserCmd_ShowInfo, L"Usage: /showinfo"},
    {L"/lights", MiscCmds::UserCmd_Lights, L"Usage: /lights"},
    {L"/lights*", MiscCmds::UserCmd_Lights, L"Usage: /lights"},
    //{ L"/selfdestruct",	MiscCmds::UserCmd_SelfDestruct, L"Usage:
    /// selfdestruct"}, { L"/selfdestruct*",MiscCmds::UserCmd_SelfDestruct,
    // L"Usage: /selfdestruct"},
    {L"/shields", MiscCmds::UserCmd_Shields, L"Usage: /shields"},
    {L"/shields*", MiscCmds::UserCmd_Shields, L"Usage: /shields"},
    //{ L"/ss",		    MiscCmds::UserCmd_Screenshot, L"Usage: /ss"},
    {L"/survey", HyperJump::UserCmd_Survey, L"Usage: /survey"},
    {L"/setcoords", HyperJump::UserCmd_SetCoords, L"Usage: /setcoords"},
    {L"/jump", HyperJump::UserCmd_ActivateJumpDrive, L"Usage: /jump"},
    {L"/jump*", HyperJump::UserCmd_ActivateJumpDrive, L"Usage: /jump"},
    {L"/charge", HyperJump::UserCmd_ChargeJumpDrive, L"Usage: /charge"},
    {L"/charge*", HyperJump::UserCmd_ChargeJumpDrive, L"Usage: /charge"},
    {L"/showscan", SystemSensor::UserCmd_ShowScan,
     L"Usage: /showscan <charname>"},
    {L"/showscan$", SystemSensor::UserCmd_ShowScan,
     L"Usage: /showscan$ <clientid>"},
    {L"/net", SystemSensor::UserCmd_Net, L"Usage: /net [all|jumponly|off]"},
    {L"/maketag", Rename::UserCmd_MakeTag,
     L"Usage: /maketag <tag> <master password> <description>"},
    {L"/droptag", Rename::UserCmd_DropTag,
     L"Usage: /droptag <tag> <master password>"},
    {L"/settagpass", Rename::UserCmd_SetTagPass,
     L"Usage: /settagpass <tag> <master password> <rename password>"},
    {L"/me", Message::UserCmd_Me, L"Usage: /me <message>"},
    {L"/do", Message::UserCmd_Do, L"Usage: /do <message>"}};

/**
This function is called by FLHook when a user types a chat std::string. We look
at the std::string they've typed and see if it starts with one of the above
commands. If it does we try to process it.
*/
bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    returncode = DEFAULT_RETURNCODE;

    try {
        std::wstring wscCmdLineLower = ToLower(wscCmd);

        Message::UserCmd_Process(iClientID, wscCmdLineLower);

        // If the chat std::string does not match the USER_CMD then we do not
        // handle the command, so let other plugins or FLHook kick in. We
        // require an exact match
        for (uint i = 0; (i < sizeof(UserCmds) / sizeof(USERCMD)); i++) {
            if (wscCmdLineLower.find(UserCmds[i].wszCmd) == 0) {
                // Extract the parameters std::string from the chat std::string.
                // It should be immediately after the command and a space.
                std::wstring wscParam = L"";
                if (wscCmd.length() > wcslen(UserCmds[i].wszCmd)) {
                    if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
                        continue;
                    wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
                }

                // Dispatch the command to the appropriate processing function.
                if (UserCmds[i].proc(iClientID, wscCmd, wscParam,
                                     UserCmds[i].usage)) {
                    // We handled the command tell FL hook to stop processing
                    // this chat std::string.
                    returncode =
                        SKIPPLUGINS_NOFUNCTIONCALL; // we handled the command,
                                                    // return immediatly
                    return true;
                }
            }
        }
    } catch (...) {
        AddLog("ERROR: Exception in PlayerCntl::UserCmd_Process(iClientID=%u, "
               "wscCmd=%s)",
               iClientID, wstos(wscCmd).c_str());
        LOG_EXCEPTION;
    }
    return false;
}

std::list<uint> npcs;

void UserCmd_Help(uint iClientID, const std::wstring &wscParam) {
    returncode = DEFAULT_RETURNCODE;
    PrintUserCmdText(iClientID, L"/pos");
    PrintUserCmdText(iClientID, L"/stuck");
    PrintUserCmdText(iClientID, L"/droprep");
    PrintUserCmdText(iClientID, L"/dice [max]");
    PrintUserCmdText(iClientID, L"/coin");

    if (set_bEnableRenameMe) {
        PrintUserCmdText(iClientID, L"/renameme <charname>");
    }

    if (set_bEnableMoveChar) {
        PrintUserCmdText(iClientID, L"/movechar <charname> <code>");
        PrintUserCmdText(iClientID, L"/set movecharcode <code>");
    }

    if (set_bEnableRestart) {
        PrintUserCmdText(iClientID, L"/restart <faction>");
        PrintUserCmdText(iClientID, L"/showrestarts");
    }

    if (set_bEnableGiveCash) {
        PrintUserCmdText(iClientID, L"/givecash <charname> <cash>");
        PrintUserCmdText(iClientID, L"/drawcash <charname> <code> <cash>");
        PrintUserCmdText(iClientID, L"/showcash <charname> <code>");
        PrintUserCmdText(iClientID, L"/set cashcode <code>");
    }

    if (set_bEnableMe) {
        PrintUserCmdText(iClientID, L"/me <message>");
    }

    if (set_bEnableDo) {
        PrintUserCmdText(iClientID, L"/do <message>");
    }

    PrintUserCmdText(iClientID, L"/showmsgs");
    PrintUserCmdText(iClientID, L"/setmsg");
    PrintUserCmdText(iClientID, L"/n  (n=0-9)");
    PrintUserCmdText(iClientID, L"/ln (n=0-9)");
    PrintUserCmdText(iClientID, L"/gn (n=0-9)");
    PrintUserCmdText(iClientID, L"/tn (n=0-9)");
    PrintUserCmdText(iClientID, L"/target or /t");
    PrintUserCmdText(iClientID, L"/reply or /r");
    PrintUserCmdText(iClientID, L"/privatemsg or /pm"),
        PrintUserCmdText(iClientID, L"/privatemsg$ or /pm$"),
        PrintUserCmdText(iClientID, L"/factionmsg or /fm"),
        PrintUserCmdText(iClientID, L"/factioninvite or /fi");
    PrintUserCmdText(iClientID, L"/set chattime");
    PrintUserCmdText(iClientID, L"/time");
    PrintUserCmdText(iClientID, L"/mail");
    PrintUserCmdText(iClientID, L"/maildel");
}

pub::AI::SetPersonalityParams HkMakePersonality() {

    pub::AI::SetPersonalityParams p;
    p.iStateGraph = pub::StateGraph::get_state_graph(
        "FIGHTER", pub::StateGraph::TYPE_STANDARD);
    p.bStateID = true;

    p.personality.EvadeDodgeUse.evade_dodge_style_weight[0] = 0.4f;
    p.personality.EvadeDodgeUse.evade_dodge_style_weight[1] = 0.0f;
    p.personality.EvadeDodgeUse.evade_dodge_style_weight[2] = 0.4f;
    p.personality.EvadeDodgeUse.evade_dodge_style_weight[3] = 0.2f;
    p.personality.EvadeDodgeUse.evade_dodge_cone_angle = 1.5708f;
    p.personality.EvadeDodgeUse.evade_dodge_interval_time = 10.0f;
    p.personality.EvadeDodgeUse.evade_dodge_time = 1.0f;
    p.personality.EvadeDodgeUse.evade_dodge_distance = 75.0f;
    p.personality.EvadeDodgeUse.evade_activate_range = 100.0f;
    p.personality.EvadeDodgeUse.evade_dodge_roll_angle = 1.5708f;
    p.personality.EvadeDodgeUse.evade_dodge_waggle_axis_cone_angle = 1.5708f;
    p.personality.EvadeDodgeUse.evade_dodge_slide_throttle = 1.0f;
    p.personality.EvadeDodgeUse.evade_dodge_turn_throttle = 1.0f;
    p.personality.EvadeDodgeUse.evade_dodge_corkscrew_roll_flip_direction =
        true;
    p.personality.EvadeDodgeUse.evade_dodge_interval_time_variance_percent =
        0.5f;
    p.personality.EvadeDodgeUse.evade_dodge_cone_angle_variance_percent = 0.5f;
    p.personality.EvadeDodgeUse.evade_dodge_direction_weight[0] = 0.25f;
    p.personality.EvadeDodgeUse.evade_dodge_direction_weight[1] = 0.25f;
    p.personality.EvadeDodgeUse.evade_dodge_direction_weight[2] = 0.25f;
    p.personality.EvadeDodgeUse.evade_dodge_direction_weight[3] = 0.25f;

    p.personality.EvadeBreakUse.evade_break_roll_throttle = 1.0f;
    p.personality.EvadeBreakUse.evade_break_time = 1.0f;
    p.personality.EvadeBreakUse.evade_break_interval_time = 10.0f;
    p.personality.EvadeBreakUse.evade_break_afterburner_delay = 0.0f;
    p.personality.EvadeBreakUse.evade_break_turn_throttle = 1.0f;
    p.personality.EvadeBreakUse.evade_break_direction_weight[0] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_direction_weight[1] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_direction_weight[2] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_direction_weight[3] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_style_weight[0] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_style_weight[1] = 1.0f;
    p.personality.EvadeBreakUse.evade_break_style_weight[2] = 1.0f;

    p.personality.BuzzHeadTowardUse.buzz_min_distance_to_head_toward = 500.0f;
    p.personality.BuzzHeadTowardUse
        .buzz_min_distance_to_head_toward_variance_percent = 0.25f;
    p.personality.BuzzHeadTowardUse.buzz_max_time_to_head_away = 1.0f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_engine_throttle = 1.0f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_turn_throttle = 1.0f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_roll_throttle = 1.0f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_turn_throttle = 1.0f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_cone_angle = 1.5708f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_cone_angle_variance_percent =
        0.5f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_waggle_axis_cone_angle = 0.3491f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_roll_angle = 1.5708f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_interval_time = 10.0f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_interval_time_variance_percent =
        0.5f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_direction_weight[0] = 0.25f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_direction_weight[1] = 0.25f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_direction_weight[2] = 0.25f;
    p.personality.BuzzHeadTowardUse.buzz_dodge_direction_weight[3] = 0.25f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_style_weight[0] = 0.33f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_style_weight[1] = 0.33f;
    p.personality.BuzzHeadTowardUse.buzz_head_toward_style_weight[2] = 0.33f;

    p.personality.BuzzPassByUse.buzz_distance_to_pass_by = 1000.0f;
    p.personality.BuzzPassByUse.buzz_pass_by_time = 1.0f;
    p.personality.BuzzPassByUse.buzz_break_direction_cone_angle = 1.5708f;
    p.personality.BuzzPassByUse.buzz_break_turn_throttle = 1.0f;
    p.personality.BuzzPassByUse.buzz_drop_bomb_on_pass_by = true;
    p.personality.BuzzPassByUse.buzz_break_direction_weight[0] = 1.0f;
    p.personality.BuzzPassByUse.buzz_break_direction_weight[1] = 1.0f;
    p.personality.BuzzPassByUse.buzz_break_direction_weight[2] = 1.0f;
    p.personality.BuzzPassByUse.buzz_break_direction_weight[3] = 1.0f;
    p.personality.BuzzPassByUse.buzz_pass_by_style_weight[2] = 1.0f;

    p.personality.TrailUse.trail_lock_cone_angle = 0.0873f;
    p.personality.TrailUse.trail_break_time = 0.5f;
    p.personality.TrailUse.trail_min_no_lock_time = 0.1f;
    p.personality.TrailUse.trail_break_roll_throttle = 1.0f;
    p.personality.TrailUse.trail_break_afterburner = true;
    p.personality.TrailUse.trail_max_turn_throttle = 1.0f;
    p.personality.TrailUse.trail_distance = 100.0f;

    p.personality.StrafeUse.strafe_run_away_distance = 100.0f;
    p.personality.StrafeUse.strafe_attack_throttle = 1.0f;

    p.personality.EngineKillUse.engine_kill_search_time = 0.0f;
    p.personality.EngineKillUse.engine_kill_face_time = 1.0f;
    p.personality.EngineKillUse.engine_kill_use_afterburner = true;
    p.personality.EngineKillUse.engine_kill_afterburner_time = 2.0f;
    p.personality.EngineKillUse.engine_kill_max_target_distance = 100.0f;

    p.personality.RepairUse.use_shield_repair_pre_delay = 0.0f;
    p.personality.RepairUse.use_shield_repair_post_delay = 1.0f;
    p.personality.RepairUse.use_shield_repair_at_damage_percent = 0.2f;
    p.personality.RepairUse.use_hull_repair_pre_delay = 0.0f;
    p.personality.RepairUse.use_hull_repair_post_delay = 1.0f;
    p.personality.RepairUse.use_hull_repair_at_damage_percent = 0.2f;

    p.personality.GunUse.gun_fire_interval_time = 0.0f;
    p.personality.GunUse.gun_fire_interval_variance_percent = 0.0f;
    p.personality.GunUse.gun_fire_burst_interval_time = 2.0f;
    p.personality.GunUse.gun_fire_burst_interval_variance_percent = 0.0f;
    p.personality.GunUse.gun_fire_no_burst_interval_time = 0.0f;
    p.personality.GunUse.gun_fire_accuracy_cone_angle = 0.00001f;
    p.personality.GunUse.gun_fire_accuracy_power = 100.0f;
    p.personality.GunUse.gun_range_threshold = 1.0f;
    p.personality.GunUse.gun_target_point_switch_time = 0.0f;
    p.personality.GunUse.fire_style = 0;
    p.personality.GunUse.auto_turret_interval_time = 2.0f;
    p.personality.GunUse.auto_turret_burst_interval_time = 2.0f;
    p.personality.GunUse.auto_turret_no_burst_interval_time = 0.0f;
    p.personality.GunUse.auto_turret_burst_interval_variance_percent = 0.0f;
    p.personality.GunUse.gun_range_threshold_variance_percent = 0.3f;
    p.personality.GunUse.gun_fire_accuracy_power_npc = 100.0f;

    p.personality.MineUse.mine_launch_interval = 8.0f;
    p.personality.MineUse.mine_launch_cone_angle = 0.7854f;
    p.personality.MineUse.mine_launch_range = 200.0f;

    p.personality.MissileUse.missile_launch_interval_time = 0.0f;
    p.personality.MissileUse.missile_launch_interval_variance_percent = 0.5f;
    p.personality.MissileUse.missile_launch_range = 800.0f;
    p.personality.MissileUse.missile_launch_cone_angle = 0.01745f;
    p.personality.MissileUse.missile_launch_allow_out_of_range = false;

    p.personality.DamageReaction.evade_break_damage_trigger_percent = 1.0f;
    p.personality.DamageReaction.evade_dodge_more_damage_trigger_percent =
        0.25f;
    p.personality.DamageReaction.engine_kill_face_damage_trigger_percent = 1.0f;
    p.personality.DamageReaction.engine_kill_face_damage_trigger_time = 0.2f;
    p.personality.DamageReaction.roll_damage_trigger_percent = 0.4f;
    p.personality.DamageReaction.roll_damage_trigger_time = 0.2f;
    p.personality.DamageReaction.afterburner_damage_trigger_percent = 0.2f;
    p.personality.DamageReaction.afterburner_damage_trigger_time = 0.5f;
    p.personality.DamageReaction.brake_reverse_damage_trigger_percent = 1.0f;
    p.personality.DamageReaction.drop_mines_damage_trigger_percent = 0.25f;
    p.personality.DamageReaction.drop_mines_damage_trigger_time = 0.1f;
    p.personality.DamageReaction.fire_guns_damage_trigger_percent = 1.0f;
    p.personality.DamageReaction.fire_guns_damage_trigger_time = 1.0f;
    p.personality.DamageReaction.fire_missiles_damage_trigger_percent = 1.0f;
    p.personality.DamageReaction.fire_missiles_damage_trigger_time = 1.0f;

    p.personality.MissileReaction.evade_missile_distance = 800.0f;
    p.personality.MissileReaction.evade_break_missile_reaction_time = 1.0f;
    p.personality.MissileReaction.evade_slide_missile_reaction_time = 1.0f;
    p.personality.MissileReaction.evade_afterburn_missile_reaction_time = 1.0f;

    p.personality.CountermeasureUse.countermeasure_active_time = 5.0f;
    p.personality.CountermeasureUse.countermeasure_unactive_time = 0.0f;

    p.personality.FormationUse.force_attack_formation_active_time = 0.0f;
    p.personality.FormationUse.force_attack_formation_unactive_time = 0.0f;
    p.personality.FormationUse.break_formation_damage_trigger_percent = 0.01f;
    p.personality.FormationUse.break_formation_damage_trigger_time = 1.0f;
    p.personality.FormationUse.break_formation_missile_reaction_time = 1.0f;
    p.personality.FormationUse.break_apart_formation_missile_reaction_time =
        1.0f;
    p.personality.FormationUse.break_apart_formation_on_evade_break = true;
    p.personality.FormationUse.break_formation_on_evade_break_time = 1.0f;
    p.personality.FormationUse.formation_exit_top_turn_break_away_throttle =
        1.0f;
    p.personality.FormationUse.formation_exit_roll_outrun_throttle = 1.0f;
    p.personality.FormationUse.formation_exit_max_time = 5.0f;
    p.personality.FormationUse.formation_exit_mode = 1;

    p.personality.Job.wait_for_leader_target = false;
    p.personality.Job.maximum_leader_target_distance = 3000;
    p.personality.Job.flee_when_leader_flees_style = false;
    p.personality.Job.scene_toughness_threshold = 4;
    p.personality.Job.flee_scene_threat_style = 4;
    p.personality.Job.flee_when_hull_damaged_percent = 0.01f;
    p.personality.Job.flee_no_weapons_style = true;
    p.personality.Job.loot_flee_threshold = 4;
    p.personality.Job.attack_subtarget_order[0] = 5;
    p.personality.Job.attack_subtarget_order[1] = 6;
    p.personality.Job.attack_subtarget_order[2] = 7;
    p.personality.Job.field_targeting = 3;
    p.personality.Job.loot_preference = 7;
    p.personality.Job.combat_drift_distance = 25000;
    p.personality.Job.attack_order[0].distance = 5000;
    p.personality.Job.attack_order[0].type = 11;
    p.personality.Job.attack_order[0].flag = 15;
    p.personality.Job.attack_order[1].type = 12;

    return p;
}

#define IS_CMD(a) !wscCmd.compare(L##a)

bool ExecuteCommandString(CCmds *cmds, const std::wstring &wscCmd) {
    returncode = DEFAULT_RETURNCODE;

    if (IS_CMD("move")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        HyperJump::AdminCmd_Move(cmds, cmds->ArgFloat(1), cmds->ArgFloat(2),
                                 cmds->ArgFloat(3));
        return true;
    } else if (IS_CMD("smiteall")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        MiscCmds::AdminCmd_SmiteAll(cmds);
        return true;
    } else if (IS_CMD("chase")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        HyperJump::AdminCmd_Chase(cmds, cmds->ArgCharname(1));
        return true;
    } else if (IS_CMD("beam")) {
        if (HyperJump::AdminCmd_Beam(cmds, cmds->ArgCharname(1),
                                     cmds->ArgStrToEnd(2))) {
            returncode = SKIPPLUGINS_NOFUNCTIONCALL;
            return true;
        }
    } else if (IS_CMD("pull")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        HyperJump::AdminCmd_Pull(cmds, cmds->ArgCharname(1));
        return true;
    } else if (IS_CMD("testbot")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        HyperJump::AdminCmd_TestBot(cmds, cmds->ArgStr(1), cmds->ArgInt(2));
        return true;
    } else if (IS_CMD("authchar")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        IPBans::AdminCmd_AuthenticateChar(cmds, cmds->ArgStr(1));
        return true;
    } else if (IS_CMD("reloadbans")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        IPBans::AdminCmd_ReloadBans(cmds);
        return true;
    } else if (IS_CMD("setaccmovecode")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        Rename::AdminCmd_SetAccMoveCode(cmds, cmds->ArgCharname(1),
                                        cmds->ArgStr(2));
        return true;
    } else if (IS_CMD("rotatelogs")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        if (fLogDebug) {
            fclose(fLogDebug);
            ::MoveFileA(sDebugLog.c_str(),
                        std::string(sDebugLog + ".old").c_str());
            _unlink(sDebugLog.c_str());
            fLogDebug = fopen(sDebugLog.c_str(), "wt");
        }

        if (fLog) {
            fclose(fLog);
            ::MoveFileA("./flhook_logs/FLHook.log",
                        "./flhook_logs/FLHook.log.old");
            _unlink("./flhook_logs/FLHook.log");
            fLog = fopen("./flhook_logs/FLHook.log", "wt");
        }

        cmds->Print(L"OK\n");
        return true;
    } else if (IS_CMD("pm") || IS_CMD("privatemsg")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        Message::AdminCmd_SendMail(cmds, cmds->ArgCharname(1),
                                   cmds->ArgStrToEnd(2));
        return true;
    } else if (IS_CMD("pm") || IS_CMD("privatemsg")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        Message::AdminCmd_SendMail(cmds, cmds->ArgCharname(1),
                                   cmds->ArgStrToEnd(2));
        return true;
    } else if (IS_CMD("showtags")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        Rename::AdminCmd_ShowTags(cmds);
        return true;
    } else if (IS_CMD("addtag")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        Rename::AdminCmd_AddTag(cmds, cmds->ArgStr(1), cmds->ArgStr(2),
                                cmds->ArgStrToEnd(3));
        return true;
    } else if (IS_CMD("droptag")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        Rename::AdminCmd_DropTag(cmds, cmds->ArgStr(1));
        return true;
    } else if (IS_CMD("jumptest")) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        HyperJump::AdminCmd_JumpTest(cmds, cmds->ArgStr(1));
        return true;
    }

    /*
    else if (IS_CMD("aievade"))
    {
            returncode = SKIPPLUGINS_NOFUNCTIONCALL;

            uint iShip1;
            pub::Player::GetShip(HkGetClientIdFromCharname(cmds->GetAdminName()),
    iShip1); if (iShip1)
            {
                    Vector pos;
                    Matrix rot;
                    pub::SpaceObj::GetLocation(iShip1, pos, rot);

                    foreach (npcs, uint, iShipIter)
                    {
                            pub::AI::DirectiveEvadeOp testOP;
                            testOP.iShip = iShip1;
                            pub::AI::SubmitState(*iShipIter, &testOP);
                    }
            }
            cmds->Print(L"OK\n");
            return true;
    }
    else if (IS_CMD("aicome"))
    {
            returncode = SKIPPLUGINS_NOFUNCTIONCALL;

            uint iShip1;
            pub::Player::GetShip(HkGetClientIdFromCharname(cmds->GetAdminName()),
    iShip1); if (iShip1)
            {
                    Vector pos;
                    Matrix rot;
                    pub::SpaceObj::GetLocation(iShip1, pos, rot);

                    foreach (npcs, uint, iShipIter)
                    {
                            pub::AI::DirectiveGotoOp go;
                            go.iGotoType = 1;
                            go.vPos = pos;
                            go.fRange = 10;
                            pub::AI::SubmitDirective(*iShipIter, &go);
                    }
            }
            cmds->Print(L"OK\n");
            return true;
    }
    else if (IS_CMD("aifollow"))
    {
            returncode = SKIPPLUGINS_NOFUNCTIONCALL;

            uint iShip1;
            pub::Player::GetShip(HkGetClientIdFromCharname(cmds->GetAdminName()),
    iShip1); if (iShip1)
            {
                    foreach (npcs, uint, iShipIter)
                    {
                            pub::AI::DirectiveFollowOp testOP;
                            testOP.leader = iShip1;
                            pub::AI::SubmitDirective(*iShipIter, &testOP);
                    }
            }
            cmds->Print(L"OK\n");
            return true;
    } */
    return false;
}

/** Admin help command callback */
void CmdHelp(CCmds *classptr) {
    returncode = DEFAULT_RETURNCODE;
    classptr->Print(L"move x, y, z\n");
    classptr->Print(L"pull <charname>\n");
    classptr->Print(L"chase <charname>\n");
    classptr->Print(L"smiteall [die]\n");
    classptr->Print(L"testbot <system> <testtime>\n");
    classptr->Print(L"authchar <charname>\n");
    classptr->Print(L"reloadbans\n");
    classptr->Print(L"setaccmovecode <charname> <code>\n");
    classptr->Print(L"rotatelogs\n");
    classptr->Print(L"privatemsg|pm <charname> <message>\n");

    classptr->Print(L"showtags\n");
    classptr->Print(L"addtag <tag> <password>\n");
    classptr->Print(L"droptag <tag> <password>\n");
}

void __stdcall Elapse_Time(float delta) {
    returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    if (delta < 0.0001f) {
        AddLog("ERROR: Elapse time correction delta=%f", delta);
        ConPrint(L"Elapse time correction delta=%f\n", delta);
        delta = 0.0001f;
    }

    try {
        Server.ElapseTime(delta);
    } catch (...) {
        AddLog("ERROR: Exception in Server.ElapseTime(delta=%f)", delta);
        LOG_EXCEPTION;
    }
}

/** Functions to hook */
EXPORT PLUGIN_INFO *Get_PluginInfo() {
    PLUGIN_INFO *p_PI = new PLUGIN_INFO();
    p_PI->sName = "Player Control Plugin by cannon 2.0";
    p_PI->sShortName = "playercntl";
    p_PI->bMayPause = true;
    p_PI->bMayUnload = true;
    p_PI->ePluginReturnCode = &returncode;
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&LoadSettings, PLUGIN_LoadSettings, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&ClearClientInfo,
                                             PLUGIN_ClearClientInfo, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkTimer, PLUGIN_HkTimerCheckKick, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&SendDeathMsg, PLUGIN_SendDeathMsg, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::Startup,
                                             PLUGIN_HkIServerImpl_Startup, 10));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::Startup_AFTER,
                        PLUGIN_HkIServerImpl_Startup_AFTER, 10));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::Login,
                                             PLUGIN_HkIServerImpl_Login, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::RequestEvent,
                        PLUGIN_HkIServerImpl_RequestEvent, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::PlayerLaunch,
                        PLUGIN_HkIServerImpl_PlayerLaunch, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::PlayerLaunch_AFTER,
                        PLUGIN_HkIServerImpl_PlayerLaunch_AFTER, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::BaseEnter,
                        PLUGIN_HkIServerImpl_BaseEnter, 0));
    // check causes lag:
    // p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC*)&HkIServerImpl::BaseEnter_AFTER,
    // PLUGIN_HkIServerImpl_BaseEnter_AFTER, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::LocationEnter,
                        PLUGIN_HkIServerImpl_LocationEnter, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::DisConnect,
                        PLUGIN_HkIServerImpl_DisConnect, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::CharacterSelect_AFTER,
                        PLUGIN_HkIServerImpl_CharacterSelect_AFTER, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::JumpInComplete_AFTER,
                        PLUGIN_HkIServerImpl_JumpInComplete_AFTER, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::SystemSwitchOutComplete,
                        PLUGIN_HkIServerImpl_SystemSwitchOutComplete, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::SPObjCollision,
                        PLUGIN_HkIServerImpl_SPObjCollision, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::GFGoodBuy,
                        PLUGIN_HkIServerImpl_GFGoodBuy, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::ReqAddItem,
                        PLUGIN_HkIServerImpl_ReqAddItem, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::ReqChangeCash,
                        PLUGIN_HkIServerImpl_ReqChangeCash, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::ReqSetCash,
                        PLUGIN_HkIServerImpl_ReqSetCash, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::ReqEquipment,
                        PLUGIN_HkIServerImpl_ReqEquipment, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::ReqHullStatus,
                        PLUGIN_HkIServerImpl_ReqHullStatus, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::ReqShipArch,
                        PLUGIN_HkIServerImpl_ReqShipArch, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::TractorObjects,
                        PLUGIN_HkIServerImpl_TractorObjects_AFTER, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::JettisonCargo,
                        PLUGIN_HkIServerImpl_JettisonCargo_AFTER, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::SetTarget,
                        PLUGIN_HkIServerImpl_SetTarget, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::CharacterInfoReq,
                        PLUGIN_HkIServerImpl_CharacterInfoReq, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::SubmitChat,
                        PLUGIN_HkIServerImpl_SubmitChat, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::SPObjUpdate,
                        PLUGIN_HkIServerImpl_SPObjUpdate, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::GoTradelane,
                        PLUGIN_HkIServerImpl_GoTradelane, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::StopTradelane,
                        PLUGIN_HkIServerImpl_StopTradelane, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::CreateNewCharacter,
                        PLUGIN_HkIServerImpl_CreateNewCharacter, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&HkIEngine::Dock_Call,
                                             PLUGIN_HkCb_Dock_Call, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&HkCb_SendChat, PLUGIN_HkCb_SendChat, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&UserCmd_Process,
                                             PLUGIN_UserCmd_Process, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&UserCmd_Help, PLUGIN_UserCmd_Help, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&ExecuteCommandString,
                        PLUGIN_ExecuteCommandString_Callback, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&CmdHelp,
                                             PLUGIN_CmdHelp_Callback, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&HkCB_MissileTorpHit,
                                             PLUGIN_HkCB_MissileTorpHit, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&RequestBestPath, PLUGIN_HkIServerImpl_RequestBestPath, 0));
    return p_PI;
}
