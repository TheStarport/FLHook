// Mark Plugin
// Originally by M0tah
// https://sourceforge.net/projects/kosacid/files/

#include "Main.h"

#define PRINT_DISABLED() PrintUserCmdText(iClientID, L"Command disabled");
#define PRINT_ERROR()                                                          \
    {                                                                          \
        for (uint i = 0; (i < sizeof(wscError) / sizeof(std::wstring)); i++)   \
            PrintUserCmdText(iClientID, wscError[i]);                          \
        return;                                                                \
    }

MARK_INFO Mark[250];
float set_fAutoMarkRadius;
std::vector<uint> vMarkSpaceObjProc;
bool set_bFlakVersion = true;
std::string scMarkFile;

EXPORT void LoadSettings() {
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    scMarkFile = std::string(szCurDir) + "\\flhook_plugins\\mark.cfg";
    set_fAutoMarkRadius =
        IniGetF(scMarkFile, "General", "AutoMarkRadius", 2.0f) * 1000;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    if (fdwReason == DLL_PROCESS_ATTACH)
        LoadSettings();
    return true;
}

void UserCmd_MarkObj(uint iClientID, const std::wstring &wscParam) {
    uint iShip, iTargetShip;
    pub::Player::GetShip(iClientID, iShip);
    pub::SpaceObj::GetTarget(iShip, iTargetShip);
    char err = HkMarkObject(iClientID, iTargetShip);
    switch (err) {
    case 0:
        // PRINT_OK()
        break;
    case 1:
        PrintUserCmdText(
            iClientID, L"Error: You must have something targeted to mark it.");
        break;
    case 2:
        PrintUserCmdText(iClientID, L"Error: You cannot mark cloaked ships.");
        break;
    case 3:
        PrintUserCmdText(iClientID, L"Error: Object is already marked.");
    default:
        break;
    }
}

void UserCmd_UnMarkObj(uint iClientID, const std::wstring &wscParam) {
    uint iShip, iTargetShip;
    pub::Player::GetShip(iClientID, iShip);
    pub::SpaceObj::GetTarget(iShip, iTargetShip);
    char err = HkUnMarkObject(iClientID, iTargetShip);
    switch (err) {
    case 0:
        // PRINT_OK()
        break;
    case 1:
        PrintUserCmdText(
            iClientID,
            L"Error: You must have something targeted to unmark it.");
        break;
    case 2:
        PrintUserCmdText(iClientID, L"Error: Object is not marked.");
    default:
        break;
    }
}

void UserCmd_UnMarkAllObj(uint iClientID, const std::wstring &wscParam) {
    HkUnMarkAllObjects(iClientID);
    // PRINT_OK()
}

void UserCmd_MarkObjGroup(uint iClientID, const std::wstring &wscParam) {
    uint iShip, iTargetShip;
    pub::Player::GetShip(iClientID, iShip);
    pub::SpaceObj::GetTarget(iShip, iTargetShip);
    if (!iTargetShip) {
        PrintUserCmdText(
            iClientID, L"Error: You must have something targeted to mark it.");
        return;
    }
    std::list<GROUP_MEMBER> lstMembers;
    lstMembers.clear();
    std::wstring wsClientID =
        (wchar_t *)Players.GetActiveCharacterName(iClientID);
    HkGetGroupMembers(wsClientID, lstMembers);
    for (auto &lstG : lstMembers) {
        if (Mark[lstG.iClientID].bIgnoreGroupMark)
            continue;
        uint iClientShip;
        pub::Player::GetShip(lstG.iClientID, iClientShip);
        if (iClientShip == iTargetShip)
            continue;
        HkMarkObject(lstG.iClientID, iTargetShip);
    }
}

void UserCmd_UnMarkObjGroup(uint iClientID, const std::wstring &wscParam) {
    uint iShip, iTargetShip;
    pub::Player::GetShip(iClientID, iShip);
    pub::SpaceObj::GetTarget(iShip, iTargetShip);
    if (!iTargetShip) {
        PrintUserCmdText(
            iClientID, L"Error: You must have something targeted to mark it.");
        return;
    }
    std::list<GROUP_MEMBER> lstMembers;
    lstMembers.clear();
    std::wstring wsClientID =
        (wchar_t *)Players.GetActiveCharacterName(iClientID);
    HkGetGroupMembers(wsClientID, lstMembers);
    for (auto &lstG : lstMembers) {
        HkUnMarkObject(lstG.iClientID, iTargetShip);
    }
}

void UserCmd_SetIgnoreGroupMark(uint iClientID, const std::wstring &wscParam) {
    std::wstring wscError[] = {
        L"Error: Invalid parameters",
        L"Usage: /ignoregroupmarks <on|off>",
    };

    if (ToLower(wscParam) == L"off") {
        Mark[iClientID].bIgnoreGroupMark = false;
        std::wstring wscDir, wscFilename;
        CAccount *acc = Players.FindAccountFromClientID(iClientID);
        if (HKHKSUCCESS(
                HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename)) &&
            HKHKSUCCESS(HkGetAccountDirName(acc, wscDir))) {
            std::string scUserFile =
                scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
            std::string scSection = "general_" + wstos(wscFilename);
            IniWrite(scUserFile, scSection, "automarkenabled", "no");
            PrintUserCmdText(iClientID, L"Accepting marks from the group");
        }
    } else if (ToLower(wscParam) == L"on") {
        Mark[iClientID].bIgnoreGroupMark = true;
        std::wstring wscDir, wscFilename;
        CAccount *acc = Players.FindAccountFromClientID(iClientID);
        if (HKHKSUCCESS(
                HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename)) &&
            HKHKSUCCESS(HkGetAccountDirName(acc, wscDir))) {
            std::string scUserFile =
                scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
            std::string scSection = "general_" + wstos(wscFilename);
            IniWrite(scUserFile, scSection, "automarkenabled", "yes");
            PrintUserCmdText(iClientID, L"Ignoring marks from the group");
        }
    } else {
        PRINT_ERROR();
    }
}

void UserCmd_AutoMark(uint iClientID, const std::wstring &wscParam) {
    if (set_fAutoMarkRadius <= 0.0f) // automarking disabled
    {
        PRINT_DISABLED();
        return;
    }

    std::wstring wscError[] = {
        L"Error: Invalid parameters",
        L"Usage: /automark <on|off> [radius in KM]",
    };

    std::wstring wscEnabled = ToLower(GetParam(wscParam, ' ', 0));

    if (!wscParam.length() || (wscEnabled != L"on" && wscEnabled != L"off")) {
        PRINT_ERROR();
        return;
    }

    std::wstring wscRadius = GetParam(wscParam, ' ', 1);
    float fRadius = 0.0f;
    if (wscRadius.length()) {
        fRadius = ToFloat(wscRadius);
    }

    // I think this section could be done better, but I can't think of it now..
    if (!Mark[iClientID].bMarkEverything) {
        if (wscRadius.length())
            Mark[iClientID].fAutoMarkRadius = fRadius * 1000;
        if (wscEnabled == L"on") // AutoMark is being enabled
        {
            Mark[iClientID].bMarkEverything = true;
            CAccount *acc = Players.FindAccountFromClientID(iClientID);
            std::wstring wscDir, wscFilename;
            if (HKHKSUCCESS(
                    HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename)) &&
                HKHKSUCCESS(HkGetAccountDirName(acc, wscDir))) {
                std::string scUserFile =
                    scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
                std::string scSection = "general_" + wstos(wscFilename);
                IniWrite(scUserFile, scSection, "automark", "yes");
                if (wscRadius.length())
                    IniWrite(scUserFile, scSection, "automarkradius",
                             ftos(Mark[iClientID].fAutoMarkRadius));
            }
            PrintUserCmdText(iClientID,
                             L"Automarking turned on within a %g KM radius",
                             Mark[iClientID].fAutoMarkRadius / 1000);
        } else if (wscRadius.length()) {
            CAccount *acc = Players.FindAccountFromClientID(iClientID);
            std::wstring wscDir, wscFilename;
            if (HKHKSUCCESS(
                    HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename)) &&
                HKHKSUCCESS(HkGetAccountDirName(acc, wscDir))) {
                std::string scUserFile =
                    scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
                std::string scSection = "general_" + wstos(wscFilename);
                IniWrite(scUserFile, scSection, "automarkradius",
                         ftos(Mark[iClientID].fAutoMarkRadius));
            }
            PrintUserCmdText(iClientID, L"Radius changed to %g KMs", fRadius);
        }
    } else {
        if (wscRadius.length())
            Mark[iClientID].fAutoMarkRadius = fRadius * 1000;
        if (wscEnabled == L"off") // AutoMark is being disabled
        {
            Mark[iClientID].bMarkEverything = false;
            CAccount *acc = Players.FindAccountFromClientID(iClientID);
            std::wstring wscDir, wscFilename;
            if (HKHKSUCCESS(
                    HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename)) &&
                HKHKSUCCESS(HkGetAccountDirName(acc, wscDir))) {
                std::string scUserFile =
                    scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
                std::string scSection = "general_" + wstos(wscFilename);
                IniWrite(scUserFile, scSection, "automark", "no");
                if (wscRadius.length())
                    IniWrite(scUserFile, scSection, "automarkradius",
                             ftos(Mark[iClientID].fAutoMarkRadius));
            }
            if (wscRadius.length())
                PrintUserCmdText(
                    iClientID,
                    L"Automarking turned off; radius changed to %g KMs",
                    Mark[iClientID].fAutoMarkRadius / 1000);
            else
                PrintUserCmdText(iClientID, L"Automarking turned off");
        } else if (wscRadius.length()) {
            CAccount *acc = Players.FindAccountFromClientID(iClientID);
            std::wstring wscDir, wscFilename;
            if (HKHKSUCCESS(
                    HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename)) &&
                HKHKSUCCESS(HkGetAccountDirName(acc, wscDir))) {
                std::string scUserFile =
                    scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
                std::string scSection = "general_" + wstos(wscFilename);
                IniWrite(scUserFile, scSection, "automarkradius",
                         ftos(Mark[iClientID].fAutoMarkRadius));
            }
            PrintUserCmdText(iClientID, L"Radius changed to %g KMs", fRadius);
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char HkMarkObject(uint iClientID, uint iObject) {
    if (!iObject)
        return 1;

    uint iClientIDMark = HkGetClientIDByShip(iObject);

    uint iSystemID, iObjectSystemID;
    pub::Player::GetSystem(iClientID, iSystemID);
    pub::SpaceObj::GetSystem(iObject, iObjectSystemID);
    if (iSystemID == iObjectSystemID) {
        for (uint i = 0; i < Mark[iClientID].vMarkedObjs.size(); i++) {
            if (Mark[iClientID].vMarkedObjs[i] == iObject)
                return 3; // already marked
        }
    } else {
        for (uint j = 0; j < Mark[iClientID].vDelayedSystemMarkedObjs.size();
             j++) {
            if (Mark[iClientID].vDelayedSystemMarkedObjs[j] == iObject)
                return 3; // already marked
        }
        Mark[iClientID].vDelayedSystemMarkedObjs.push_back(iObject);
        pub::Audio::PlaySoundEffect(iClientID,
                                    2460046221); // CreateID("ui_select_add")
        return 0;
    }

    pub::Player::MarkObj(iClientID, iObject, 1);
    for (uint i = 0; i < Mark[iClientID].vAutoMarkedObjs.size();
         i++) // remove from automarked vector
    {
        if (Mark[iClientID].vAutoMarkedObjs[i] == iObject) {
            if (i != Mark[iClientID].vAutoMarkedObjs.size() - 1) {
                Mark[iClientID].vAutoMarkedObjs[i] =
                    Mark[iClientID].vAutoMarkedObjs
                        [Mark[iClientID].vAutoMarkedObjs.size() - 1];
            }
            Mark[iClientID].vAutoMarkedObjs.pop_back();
            break;
        }
    }
    Mark[iClientID].vMarkedObjs.push_back(iObject);
    pub::Audio::PlaySoundEffect(iClientID,
                                2460046221); // CreateID("ui_select_add")
    return 0;
}

char HkUnMarkObject(uint iClientID, uint iObject) {
    if (!iObject)
        return 1;

    for (uint i = 0; i < Mark[iClientID].vMarkedObjs.size(); i++) {
        if (Mark[iClientID].vMarkedObjs[i] == iObject) {
            if (i != Mark[iClientID].vMarkedObjs.size() - 1) {
                Mark[iClientID].vMarkedObjs[i] =
                    Mark[iClientID]
                        .vMarkedObjs[Mark[iClientID].vMarkedObjs.size() - 1];
            }
            Mark[iClientID].vMarkedObjs.pop_back();
            pub::Player::MarkObj(iClientID, iObject, 0);
            pub::Audio::PlaySoundEffect(
                iClientID,
                2939827141); // CreateID("ui_select_remove")
            return 0;
        }
    }

    for (uint j = 0; j < Mark[iClientID].vAutoMarkedObjs.size(); j++) {
        if (Mark[iClientID].vAutoMarkedObjs[j] == iObject) {
            if (j != Mark[iClientID].vAutoMarkedObjs.size() - 1) {
                Mark[iClientID].vAutoMarkedObjs[j] =
                    Mark[iClientID].vAutoMarkedObjs
                        [Mark[iClientID].vAutoMarkedObjs.size() - 1];
            }
            Mark[iClientID].vAutoMarkedObjs.pop_back();
            pub::Player::MarkObj(iClientID, iObject, 0);
            pub::Audio::PlaySoundEffect(
                iClientID,
                2939827141); // CreateID("ui_select_remove")
            return 0;
        }
    }

    for (uint k = 0; k < Mark[iClientID].vDelayedSystemMarkedObjs.size(); k++) {
        if (Mark[iClientID].vDelayedSystemMarkedObjs[k] == iObject) {
            if (k != Mark[iClientID].vDelayedSystemMarkedObjs.size() - 1) {
                Mark[iClientID].vDelayedSystemMarkedObjs[k] =
                    Mark[iClientID].vDelayedSystemMarkedObjs
                        [Mark[iClientID].vDelayedSystemMarkedObjs.size() - 1];
            }
            Mark[iClientID].vDelayedSystemMarkedObjs.pop_back();
            pub::Audio::PlaySoundEffect(
                iClientID,
                2939827141); // CreateID("ui_select_remove")
            return 0;
        }
    }
    return 2;
}

void HkUnMarkAllObjects(uint iClientID) {
    for (uint i = 0; i < Mark[iClientID].vMarkedObjs.size(); i++) {
        pub::Player::MarkObj(iClientID, (Mark[iClientID].vMarkedObjs[i]), 0);
    }
    Mark[iClientID].vMarkedObjs.clear();
    for (uint i = 0; i < Mark[iClientID].vAutoMarkedObjs.size(); i++) {
        pub::Player::MarkObj(iClientID, (Mark[iClientID].vAutoMarkedObjs[i]),
                             0);
    }
    Mark[iClientID].vAutoMarkedObjs.clear();
    Mark[iClientID].vDelayedSystemMarkedObjs.clear();
    pub::Audio::PlaySoundEffect(iClientID,
                                2939827141); // CreateID("ui_select_remove")
}

std::string ftos(float f) {
    char szBuf[16];
    sprintf_s(szBuf, "%f", f);
    return szBuf;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClearClientMark(uint iClientID) {
    Mark[iClientID].bMarkEverything = false;
    Mark[iClientID].vMarkedObjs.clear();
    Mark[iClientID].vDelayedSystemMarkedObjs.clear();
    Mark[iClientID].vAutoMarkedObjs.clear();
    Mark[iClientID].vDelayedAutoMarkedObjs.clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Timers
void HkTimerSpaceObjMark() {
    try {
        if (set_fAutoMarkRadius <= 0.0f) // automarking disabled
            return;

        struct PlayerData *pPD = 0;
        while (pPD = Players.traverse_active(pPD)) {
            uint iShip, iClientID = HkGetClientIdFromPD(pPD);
            pub::Player::GetShip(iClientID, iShip);
            if (!iShip || Mark[iClientID].fAutoMarkRadius <=
                              0.0f) // docked or does not want any marking
                continue;
            uint iSystem;
            pub::Player::GetSystem(iClientID, iSystem);
            Vector VClientPos;
            Matrix MClientOri;
            pub::SpaceObj::GetLocation(iShip, VClientPos, MClientOri);

            for (uint i = 0; i < Mark[iClientID].vAutoMarkedObjs.size(); i++) {
                Vector VTargetPos;
                Matrix MTargetOri;
                pub::SpaceObj::GetLocation(Mark[iClientID].vAutoMarkedObjs[i],
                                           VTargetPos, MTargetOri);
                if (HkDistance3D(VTargetPos, VClientPos) >
                    Mark[iClientID].fAutoMarkRadius) {
                    pub::Player::MarkObj(iClientID,
                                         Mark[iClientID].vAutoMarkedObjs[i], 0);
                    Mark[iClientID].vDelayedAutoMarkedObjs.push_back(
                        Mark[iClientID].vAutoMarkedObjs[i]);
                    if (i != Mark[iClientID].vAutoMarkedObjs.size() - 1) {
                        Mark[iClientID].vAutoMarkedObjs[i] =
                            Mark[iClientID].vAutoMarkedObjs
                                [Mark[iClientID].vAutoMarkedObjs.size() - 1];
                        i--;
                    }
                    Mark[iClientID].vAutoMarkedObjs.pop_back();
                }
            }

            for (uint i = 0; i < Mark[iClientID].vDelayedAutoMarkedObjs.size();
                 i++) {
                if (pub::SpaceObj::ExistsAndAlive(
                        Mark[iClientID].vDelayedAutoMarkedObjs[i])) {
                    if (i !=
                        Mark[iClientID].vDelayedAutoMarkedObjs.size() - 1) {
                        Mark[iClientID].vDelayedAutoMarkedObjs[i] =
                            Mark[iClientID].vDelayedAutoMarkedObjs
                                [Mark[iClientID].vDelayedAutoMarkedObjs.size() -
                                 1];
                        i--;
                    }
                    Mark[iClientID].vDelayedAutoMarkedObjs.pop_back();
                    continue;
                }
                Vector VTargetPos;
                Matrix MTargetOri;
                pub::SpaceObj::GetLocation(
                    Mark[iClientID].vDelayedAutoMarkedObjs[i], VTargetPos,
                    MTargetOri);
                if (!(HkDistance3D(VTargetPos, VClientPos) >
                      Mark[iClientID].fAutoMarkRadius)) {
                    pub::Player::MarkObj(
                        iClientID, Mark[iClientID].vDelayedAutoMarkedObjs[i],
                        1);
                    Mark[iClientID].vAutoMarkedObjs.push_back(
                        Mark[iClientID].vDelayedAutoMarkedObjs[i]);
                    if (i !=
                        Mark[iClientID].vDelayedAutoMarkedObjs.size() - 1) {
                        Mark[iClientID].vDelayedAutoMarkedObjs[i] =
                            Mark[iClientID].vDelayedAutoMarkedObjs
                                [Mark[iClientID].vDelayedAutoMarkedObjs.size() -
                                 1];
                        i--;
                    }
                    Mark[iClientID].vDelayedAutoMarkedObjs.pop_back();
                }
            }

            for (uint i = 0; i < vMarkSpaceObjProc.size(); i++) {
                uint iMarkSpaceObjProcShip = vMarkSpaceObjProc[i];
                if (set_bFlakVersion) {
                    uint iType;
                    pub::SpaceObj::GetType(iMarkSpaceObjProcShip, iType);
                    if (iType != OBJ_CAPITAL &&
                        ((Mark[iClientID].bMarkEverything &&
                          iType == OBJ_FIGHTER) /* || iType==OBJ_FREIGHTER*/)) {
                        uint iSpaceObjSystem;
                        pub::SpaceObj::GetSystem(iMarkSpaceObjProcShip,
                                                 iSpaceObjSystem);
                        Vector VTargetPos;
                        Matrix MTargetOri;
                        pub::SpaceObj::GetLocation(iMarkSpaceObjProcShip,
                                                   VTargetPos, MTargetOri);
                        if (iSpaceObjSystem != iSystem ||
                            HkDistance3D(VTargetPos, VClientPos) >
                                Mark[iClientID].fAutoMarkRadius) {
                            Mark[iClientID].vDelayedAutoMarkedObjs.push_back(
                                iMarkSpaceObjProcShip);
                        } else {
                            pub::Player::MarkObj(iClientID,
                                                 iMarkSpaceObjProcShip, 1);
                            Mark[iClientID].vAutoMarkedObjs.push_back(
                                iMarkSpaceObjProcShip);
                        }
                    }
                } else // just mark everything
                {
                    uint iSpaceObjSystem;
                    pub::SpaceObj::GetSystem(iMarkSpaceObjProcShip,
                                             iSpaceObjSystem);
                    Vector VTargetPos;
                    Matrix MTargetOri;
                    pub::SpaceObj::GetLocation(iMarkSpaceObjProcShip,
                                               VTargetPos, MTargetOri);
                    if (iSpaceObjSystem != iSystem ||
                        HkDistance3D(VTargetPos, VClientPos) >
                            Mark[iClientID].fAutoMarkRadius) {
                        Mark[iClientID].vDelayedAutoMarkedObjs.push_back(
                            iMarkSpaceObjProcShip);
                    } else {
                        pub::Player::MarkObj(iClientID, iMarkSpaceObjProcShip,
                                             1);
                        Mark[iClientID].vAutoMarkedObjs.push_back(
                            iMarkSpaceObjProcShip);
                    }
                }
            }
            vMarkSpaceObjProc.clear();
        }
    } catch (...) {
    }
}

std::list<DELAY_MARK> g_lstDelayedMarks;
void HkTimerMarkDelay() {
    if (!g_lstDelayedMarks.size())
        return;

    mstime tmTimeNow = timeInMS();
    for (std::list<DELAY_MARK>::iterator mark = g_lstDelayedMarks.begin();
         mark != g_lstDelayedMarks.end();) {
        if (tmTimeNow - mark->time > 50) {
            Matrix mTemp;
            Vector vItem, vPlayer;
            pub::SpaceObj::GetLocation(mark->iObj, vItem, mTemp);
            uint iItemSystem;
            pub::SpaceObj::GetSystem(mark->iObj, iItemSystem);
            // for all players
            struct PlayerData *pPD = 0;
            while (pPD = Players.traverse_active(pPD)) {
                uint iClientID = HkGetClientIdFromPD(pPD);
                if (Players[iClientID].iSystemID == iItemSystem) {
                    pub::SpaceObj::GetLocation(Players[iClientID].iShipID,
                                               vPlayer, mTemp);
                    if (HkDistance3D(vPlayer, vItem) <= LOOT_UNSEEN_RADIUS) {
                        HkMarkObject(iClientID, mark->iObj);
                    }
                }
            }
            mark = g_lstDelayedMarks.erase(mark);
        } else {
            mark++;
        }
    }
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace HkIServerImpl {
EXPORT void __stdcall JumpInComplete(unsigned int iSystemID,
                                     unsigned int iShip) {
    uint iClientID = HkGetClientIDByShip(iShip);
    if (!iClientID)
        return;
    std::vector<uint> vTempMark;
    for (uint i = 0; i < Mark[iClientID].vDelayedSystemMarkedObjs.size(); i++) {
        if (pub::SpaceObj::ExistsAndAlive(
                Mark[iClientID].vDelayedSystemMarkedObjs[i])) {
            if (i != Mark[iClientID].vDelayedSystemMarkedObjs.size() - 1) {
                Mark[iClientID].vDelayedSystemMarkedObjs[i] =
                    Mark[iClientID].vDelayedSystemMarkedObjs
                        [Mark[iClientID].vDelayedSystemMarkedObjs.size() - 1];
                i--;
            }
            Mark[iClientID].vDelayedSystemMarkedObjs.pop_back();
            continue;
        }

        uint iTargetSystem;
        pub::SpaceObj::GetSystem(Mark[iClientID].vDelayedSystemMarkedObjs[i],
                                 iTargetSystem);
        if (iTargetSystem == iSystemID) {
            pub::Player::MarkObj(
                iClientID, Mark[iClientID].vDelayedSystemMarkedObjs[i], 1);
            vTempMark.push_back(Mark[iClientID].vDelayedSystemMarkedObjs[i]);
            if (i != Mark[iClientID].vDelayedSystemMarkedObjs.size() - 1) {
                Mark[iClientID].vDelayedSystemMarkedObjs[i] =
                    Mark[iClientID].vDelayedSystemMarkedObjs
                        [Mark[iClientID].vDelayedSystemMarkedObjs.size() - 1];
                i--;
            }
            Mark[iClientID].vDelayedSystemMarkedObjs.pop_back();
        }
    }
    for (uint i = 0; i < Mark[iClientID].vMarkedObjs.size(); i++) {
        if (!pub::SpaceObj::ExistsAndAlive(Mark[iClientID].vMarkedObjs[i]))
            Mark[iClientID].vDelayedSystemMarkedObjs.push_back(
                Mark[iClientID].vMarkedObjs[i]);
    }
    Mark[iClientID].vMarkedObjs = vTempMark;
}

EXPORT void __stdcall LaunchComplete(unsigned int iBaseID, unsigned int iShip) {
    uint iClientID = HkGetClientIDByShip(iShip);
    if (!iClientID)
        return;
    for (uint i = 0; i < Mark[iClientID].vMarkedObjs.size(); i++) {
        if (pub::SpaceObj::ExistsAndAlive(Mark[iClientID].vMarkedObjs[i])) {
            if (i != Mark[iClientID].vMarkedObjs.size() - 1) {
                Mark[iClientID].vMarkedObjs[i] =
                    Mark[iClientID]
                        .vMarkedObjs[Mark[iClientID].vMarkedObjs.size() - 1];
                i--;
            }
            Mark[iClientID].vMarkedObjs.pop_back();
            continue;
        }
        pub::Player::MarkObj(iClientID, Mark[iClientID].vMarkedObjs[i], 1);
    }
}

EXPORT void __stdcall BaseEnter(unsigned int iBaseID, unsigned int iClientID) {
    Mark[iClientID].vAutoMarkedObjs.clear();
    Mark[iClientID].vDelayedAutoMarkedObjs.clear();
}

typedef void (*_TimerFunc)();
struct TIMER {
    _TimerFunc proc;
    mstime tmIntervallMS;
    mstime tmLastCall;
};

TIMER Timers[] = {
    {HkTimerMarkDelay, 50, 0},
    {HkTimerSpaceObjMark, 50, 0},
};

EXPORT int __stdcall Update() {
    static bool bFirstTime = true;
    if (bFirstTime) {
        bFirstTime = false;
        // check for logged in players and reset their connection data
        struct PlayerData *pPD = 0;
        while (pPD = Players.traverse_active(pPD))
            ClearClientMark(HkGetClientIdFromPD(pPD));
    }
    for (uint i = 0; (i < sizeof(Timers) / sizeof(TIMER)); i++) {
        if ((timeInMS() - Timers[i].tmLastCall) >= Timers[i].tmIntervallMS) {
            Timers[i].tmLastCall = timeInMS();
            Timers[i].proc();
        }
    }
    return 0;
}

EXPORT void __stdcall DisConnect(unsigned int iClientID,
                                 enum EFLConnection p2) {
    ClearClientMark(iClientID);
}
} // namespace HkIServerImpl

EXPORT void LoadUserCharSettings(uint iClientID) {
    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    std::wstring wscDir;
    HkGetAccountDirName(acc, wscDir);
    std::string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";
    std::wstring wscFilename;
    HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename);
    std::string scFilename = wstos(wscFilename);
    std::string scSection = "general_" + scFilename;
    Mark[iClientID].bMarkEverything =
        IniGetB(scUserFile, scSection, "automarkenabled", false);
    Mark[iClientID].bIgnoreGroupMark =
        IniGetB(scUserFile, scSection, "ignoregroupmarkenabled", false);
    Mark[iClientID].fAutoMarkRadius =
        IniGetF(scUserFile, scSection, "automarkradius", set_fAutoMarkRadius);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EXPORT void UserCmd_Help(uint iClientID, const std::wstring &wscParam) {
    PrintUserCmdText(iClientID, L"/mark /m ");
    PrintUserCmdText(
        iClientID,
        L"Makes the selected object appear in the important section of the "
        L"contacts and have an arrow on the side of the screen, as well as "
        L"have "
        L"> and < on the sides of the selection box.");
    PrintUserCmdText(iClientID, L"/unmark /um");
    PrintUserCmdText(
        iClientID,
        L"Unmarks the selected object marked with the /mark (/m) command.");
    PrintUserCmdText(iClientID, L"/groupmark /gm");
    PrintUserCmdText(iClientID, L"Marks selected object for the entire group.");
    PrintUserCmdText(iClientID, L"/groupunmark /gum");
    PrintUserCmdText(iClientID,
                     L"Unmarks the selected object for the entire group.");
    PrintUserCmdText(iClientID, L"/ignoregroupmarks <on|off>");
    PrintUserCmdText(iClientID, L"Ignores marks from others in your group.");
    PrintUserCmdText(iClientID, L"/automark <on|off> [radius in KM]");
    PrintUserCmdText(
        iClientID,
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

EXPORT bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    std::wstring wscCmdLower = ToLower(wscCmd);
    for (uint i = 0; (i < sizeof(UserCmds) / sizeof(USERCMD)); i++) {
        if (wscCmdLower.find(ToLower(UserCmds[i].wszCmd)) == 0) {
            std::wstring wscParam = L"";
            if (wscCmd.length() > wcslen(UserCmds[i].wszCmd)) {
                if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
                    continue;
                wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
            }
            UserCmds[i].proc(iClientID, wscParam);
            returncode = ReturnCode::SkipAll;
            return true;
        }
    }
    return false;
}

EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Mark plugin by M0tah");
    pi->shortName("mark");
    pi->mayPause(false);
    pi->mayUnload(false);
    pi->returnCode(&returncode);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
    pi->emplaceHook(HookedCall::IServerImpl__JumpInComplete,
                    &HkIServerImpl::JumpInComplete);
    pi->emplaceHook(HookedCall::IServerImpl__LaunchComplete,
                    &HkIServerImpl::LaunchComplete);
    pi->emplaceHook(HookedCall::IServerImpl__BaseEnter,
                    &HkIServerImpl::BaseEnter);
    pi->emplaceHook(HookedCall::IServerImpl__Update, &HkIServerImpl::Update);
    pi->emplaceHook(HookedCall::IServerImpl__DisConnect,
                    &HkIServerImpl::DisConnect);
    pi->emplaceHook(HookedCall::FLHook__LoadCharacterSettings,
                    &LoadUserCharSettings);
}