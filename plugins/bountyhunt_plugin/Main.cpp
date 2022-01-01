// BountyHunt Plugin
// Originally by ||KOS||Acid
// Modified by Raikkonen

// Includes
#include "Main.h"

// Definitions
#define PRINT_DISABLED() PrintUserCmdText(iClientID, L"Command disabled");

// Global Variables and Structures
struct BOUNTY_HUNT {
    uint uiTargetID;
    uint uiInitiatorID;
    std::wstring wscTarget;
    std::wstring wscInitiator;
    int iCash;
    mstime msEnd;
};

std::list<BOUNTY_HUNT> lstBountyHunt;
bool set_bBhEnabled;
int set_iLowLevelProtect;

PLUGIN_RETURNCODE Get_PluginReturnCode() { return returncode; }

// Functions
void RemoveBountyHunt(BOUNTY_HUNT b) {
    std::list<BOUNTY_HUNT>::iterator it = lstBountyHunt.begin();
    while (it != lstBountyHunt.end()) {
        if (it->uiTargetID == b.uiTargetID &&
            it->uiInitiatorID == b.uiInitiatorID) {
            lstBountyHunt.erase(it++);
        } else {
            ++it;
        }
    }
}

void PrintBountyHunts(uint iClientID) {
    if (lstBountyHunt.begin() != lstBountyHunt.end()) {
        PrintUserCmdText(iClientID, L"Offered Bounty Hunts:");
        for (auto &it : lstBountyHunt) {
            PrintUserCmdText(iClientID,
                             L"Kill %s and earn %u credits (%u minutes left)",
                             it.wscTarget.c_str(), it.iCash,
                             (uint)((it.msEnd - GetTimeInMS()) / 60000));
        }
    }
}

bool UserCmd_BountyHunt(uint iClientID, const std::wstring &wscCmd,
                        const std::wstring &wscParam, const wchar_t *usage) {
    if (!set_bBhEnabled) {
        PRINT_DISABLED();
        return true;
    }

    std::wstring wscTarget = GetParam(wscParam, ' ', 0);
    std::wstring wscCredits = GetParam(wscParam, ' ', 1);
    std::wstring wscTime = GetParam(wscParam, ' ', 2);
    if (!wscTarget.length() || !wscCredits.length()) {
        PrintUserCmdText(iClientID, usage);
        PrintBountyHunts(iClientID);
        return true;
    }

    int iPrize = wcstol(wscCredits.c_str(), NULL, 10);
    uint uiTime = wcstol(wscTime.c_str(), NULL, 10);
    uint uiTargetID = HkGetClientIdFromCharname(wscTarget);

    int iRankTarget;
    pub::Player::GetRank(uiTargetID, iRankTarget);

    if (uiTargetID == -1 || HkIsInCharSelectMenu(uiTargetID)) {
        PrintUserCmdText(iClientID, L"%s is not online.", wscTarget.c_str());
        return true;
    }

    if (iRankTarget < set_iLowLevelProtect) {
        PrintUserCmdText(iClientID, L"Low level players may not be hunted.");
        return true;
    }
    if (uiTime < 1 || uiTime > 240) {
        PrintUserCmdText(iClientID, L"Hunting time: 30 minutes.");
        uiTime = 30;
    }

    int iClientCash;
    pub::Player::InspectCash(iClientID, iClientCash);
    if (iClientCash < iPrize) {
        PrintUserCmdText(iClientID, L"You do not possess enough credits.");
        return true;
    }

    for (auto &bh : lstBountyHunt) {
        if (bh.uiInitiatorID == iClientID && bh.uiTargetID == uiTargetID) {
            PrintUserCmdText(iClientID,
                             L"You already have a bounty on this player.");
            return true;
        }
    }

    pub::Player::AdjustCash(iClientID, -iPrize);
    std::wstring wscInitiatior =
        (wchar_t *)Players.GetActiveCharacterName(iClientID);

    BOUNTY_HUNT bh;
    bh.uiInitiatorID = iClientID;
    bh.msEnd = GetTimeInMS() + (mstime)(uiTime * 60000);
    bh.wscInitiator = wscInitiatior;
    bh.iCash = iPrize;
    bh.wscTarget = wscTarget;
    bh.uiTargetID = uiTargetID;

    lstBountyHunt.push_back(bh);

    HkMsgU(bh.wscInitiator + L" offers " + std::to_wstring(bh.iCash) +
           L" credits for killing " + bh.wscTarget + L" in " +
           std::to_wstring(uiTime) + L" minutes.");
    return true;
}

bool UserCmd_BountyHuntId(uint iClientID, const std::wstring &wscCmd,
                          const std::wstring &wscParam, const wchar_t *usage) {
    if (!set_bBhEnabled) {
        PRINT_DISABLED();
        return true;
    }

    std::wstring wscTarget = GetParam(wscParam, ' ', 0);
    std::wstring wscCredits = GetParam(wscParam, ' ', 1);
    std::wstring wscTime = GetParam(wscParam, ' ', 2);
    if (!wscTarget.length() || !wscCredits.length()) {
        PrintUserCmdText(iClientID, usage);
        PrintBountyHunts(iClientID);
        return true;
    }

    uint iClientIDTarget = ToInt(wscTarget);
    if (!HkIsValidClientID(iClientIDTarget) ||
        HkIsInCharSelectMenu(iClientIDTarget)) {
        PrintUserCmdText(iClientID, L"Error: Invalid client id.");
        return true;
    }

    std::wstring wscCharName =
        (wchar_t *)Players.GetActiveCharacterName(iClientIDTarget);
    std::wstring wscParamNew =
        std::wstring(wscCharName + L" " + wscCredits + L" " + wscTime);
    UserCmd_BountyHunt(iClientID, wscCmd, wscParamNew, usage);
    return true;
}

void BhTimeOutCheck() {
    for (auto &it : lstBountyHunt) {
        if (it.msEnd < timeInMS()) {
            HkAddCash(it.wscTarget, it.iCash);
            HkMsgU(it.wscTarget + L" was not hunted down and earned " +
                   std::to_wstring(it.iCash) + L" credits.");
            RemoveBountyHunt(it);
            BhTimeOutCheck();
            break;
        }
    }
}

void BhKillCheck(uint uiClientID, uint uiKillerID) {
    for (auto &it : lstBountyHunt) {
        if (it.uiTargetID == uiClientID) {
            if (uiKillerID == 0 || uiClientID == uiKillerID) {
                HkMsgU(L"The hunt for " + it.wscTarget + L" still goes on.");
            } else {
                std::wstring wscWinnerCharname =
                    (wchar_t *)Players.GetActiveCharacterName(uiKillerID);
                if (wscWinnerCharname.size() > 0) {
                    HkAddCash(wscWinnerCharname, it.iCash);
                    HkMsgU(wscWinnerCharname + L" has killed " + it.wscTarget +
                           L" and earned " + std::to_wstring(it.iCash) +
                           L" credits.");
                } else {
                    HkAddCash(it.wscInitiator, it.iCash);
                }

                RemoveBountyHunt(it);
                BhKillCheck(uiClientID, uiKillerID);
                break;
            }
        }
    }
}

// Hooks

namespace HkIServerImpl {
typedef void (*_TimerFunc)();
struct TIMER {
    _TimerFunc proc;
    mstime tmIntervallMS;
    mstime tmLastCall;
};

TIMER Timers[] = {
    {BhTimeOutCheck, 2017, 0},
};

EXPORT int __stdcall Update() {
    returncode = DEFAULT_RETURNCODE;

    for (uint i = 0; (i < sizeof(Timers) / sizeof(TIMER)); i++) {
        if ((timeInMS() - Timers[i].tmLastCall) >= Timers[i].tmIntervallMS) {
            Timers[i].tmLastCall = timeInMS();
            Timers[i].proc();
        }
    }
    return 0;
}

} // namespace HkIServerImpl

void SendDeathMsg(const std::wstring &wscMsg, uint iSystemID,
                  uint iClientIDVictim, uint iClientIDKiller) {
    returncode = DEFAULT_RETURNCODE;
    if (set_bBhEnabled) {
        BhKillCheck(iClientIDVictim, iClientIDKiller);
    }
}

void __stdcall DisConnect(unsigned int iClientID, enum EFLConnection state) {
    returncode = DEFAULT_RETURNCODE;
    for (auto &it : lstBountyHunt) {
        if (it.uiTargetID == iClientID) {
            HkMsgU(L"The coward " + it.wscTarget + L" has fled. " +
                   it.wscInitiator + L" has been refunded.");
            HkAddCash(it.wscInitiator, it.iCash);
            RemoveBountyHunt(it);
            return;
        }
    }
}

// Client command processing
USERCMD UserCmds[] = {
    {L"/bountyhunt", UserCmd_BountyHunt,
     L"Usage: /bountyhunt <playername> <credits> <time>"},
    {L"/bountyhuntid", UserCmd_BountyHuntId,
     L"Usage: /bountyhuntid <id> <credits> <time>"},
};

bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    returncode = DEFAULT_RETURNCODE;

    std::wstring wscCmdLineLower = ToLower(wscCmd);

    // If the chat string does not match the USER_CMD then we do not handle the
    // command, so let other plugins or FLHook kick in. We require an exact
    // match
    for (uint i = 0; (i < sizeof(UserCmds) / sizeof(USERCMD)); i++) {

        if (wscCmdLineLower.find(UserCmds[i].wszCmd) == 0) {
            // Extract the parameters string from the chat string. It should
            // be immediately after the command and a space.
            std::wstring wscParam = L"";
            if (wscCmd.length() > wcslen(UserCmds[i].wszCmd)) {
                if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
                    continue;
                wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
            }

            // Dispatch the command to the appropriate processing function.
            if (UserCmds[i].proc(iClientID, wscCmd, wscParam,
                                 UserCmds[i].usage)) {
                // We handled the command tell FL hook to stop processing this
                // chat string.
                returncode =
                    SKIPPLUGINS_NOFUNCTIONCALL; // we handled the command,
                                                // return immediatly
                return true;
            }
        }
    }
    return false;
}

EXPORT void UserCmd_Help(uint iClientID, const std::wstring &wscParam) {
    PrintUserCmdText(iClientID,
                     L"/bountyhunt <charname> <credits> [<minutes>]");
    PrintUserCmdText(iClientID, L"/bountyhuntid <id> <credits> [<minutes>]");
}

// Load Settings
void LoadSettings() {
    std::string set_scCfgGeneralFile;
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    set_scCfgGeneralFile =
        std::string(szCurDir) + "\\flhook_plugins\\bountyhunt.cfg";
    set_bBhEnabled =
        IniGetB(set_scCfgGeneralFile, "General", "EnableBountyHunt", false);
    set_iLowLevelProtect =
        IniGetI(set_scCfgGeneralFile, "General", "LevelProtect", 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH)
        LoadSettings();
    return true;
}

EXPORT PLUGIN_INFO *Get_PluginInfo() {
    PLUGIN_INFO *p_PI = new PLUGIN_INFO();
    p_PI->sName = "Bounty Hunt";
    p_PI->sShortName = "bountyhunt";
    p_PI->bMayPause = false;
    p_PI->bMayUnload = false;
    p_PI->ePluginReturnCode = &returncode;
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::Update,
                                             PLUGIN_HkIServerImpl_Update, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&UserCmd_Process,
                                             PLUGIN_UserCmd_Process, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&UserCmd_Help, PLUGIN_UserCmd_Help, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&SendDeathMsg, PLUGIN_SendDeathMsg, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&LoadSettings, PLUGIN_LoadSettings, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&DisConnect, PLUGIN_HkIServerImpl_DisConnect, 0));
    return p_PI;
}