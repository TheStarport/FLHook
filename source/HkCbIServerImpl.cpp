#include "wildcards.hh"
#include "CInGame.h"
#include "hook.h"

#define ISERVER_LOG()                                                          \
    if (set_bDebug)                                                            \
        AddDebugLog(__FUNCSIG__);
#define ISERVER_LOGARG_WS(a)                                                   \
    if (set_bDebug)                                                            \
        AddDebugLog("     " #a ": %s", wstos((const wchar_t *)a).c_str());
#define ISERVER_LOGARG_S(a)                                                    \
    if (set_bDebug)                                                            \
        AddDebugLog("     " #a ": %s", (const char *)a);
#define ISERVER_LOGARG_UI(a)                                                   \
    if (set_bDebug)                                                            \
        AddDebugLog("     " #a ": %u", (uint)a);
#define ISERVER_LOGARG_I(a)                                                    \
    if (set_bDebug)                                                            \
        AddDebugLog("     " #a ": %d", (int)a);
#define ISERVER_LOGARG_H(a)                                                    \
    if (set_bDebug)                                                            \
        AddDebugLog("     " #a ": 0x%08X", (int)a);
#define ISERVER_LOGARG_F(a)                                                    \
    if (set_bDebug)                                                            \
        AddDebugLog("     " #a ": %f", (float)a);
#define ISERVER_LOGARG_V(a)                                                    \
    if (set_bDebug)                                                            \
        AddDebugLog("     " #a ": %f %f %f", (float)a.x, (float)a.y,           \
                    (float)a.z);
#define ISERVER_LOGARG_Q(a)                                                    \
    if (set_bDebug)                                                            \
        AddDebugLog("     " #a ": %f %f %f %f", (float)a.x, (float)a.y,        \
                    (float)a.z, (float)a.w);

#define EXECUTE_SERVER_CALL(args)                                              \
    {                                                                          \
        static CTimer timer(__FUNCTION__, set_iTimerThreshold);                \
        timer.start();                                                         \
        TRY_HOOK { args; }                                                     \
        CATCH_HOOK({                                                           \
            AddLog("ERROR: Exception in " __FUNCTION__ " on server call");     \
        })                                                                     \
        timer.stop();                                                          \
    }

#define CHECK_FOR_DISCONNECT                                                   \
    {                                                                          \
        if (ClientInfo[iClientID].bDisconnected) {                             \
            AddLog(                                                            \
                "ERROR: Ignoring disconnected client in " __FUNCTION__ " id=%" \
                                                                       "u",    \
                iClientID);                                                    \
            return;                                                            \
        };                                                                     \
    }

namespace HkIServerImpl {

/**************************************************************************************************************
this is our "main" loop
**************************************************************************************************************/

// add timers here
typedef void (*_TimerFunc)();

struct TIMER {
    _TimerFunc proc;
    mstime tmIntervallMS;
    mstime tmLastCall;
};

TIMER Timers[] = {
    {ProcessPendingCommands, 50, 0},
    {HkTimerCheckKick, 1000, 0},
    {HkTimerNPCAndF1Check, 50, 0},
    {HkTimerCheckResolveResults, 0, 0},
};

int __stdcall Update(void) {

    static bool bFirstTime = true;
    if (bFirstTime) {
        FLHookInit();
        bFirstTime = false;
    }

    // call timers
    for (uint i = 0; (i < sizeof(Timers) / sizeof(TIMER)); i++) {
        if ((timeInMS() - Timers[i].tmLastCall) >= Timers[i].tmIntervallMS) {
            Timers[i].tmLastCall = timeInMS();
            Timers[i].proc();
        }
    }

    char *pData;
    memcpy(&pData, g_FLServerDataPtr + 0x40, 4);
    memcpy(&g_iServerLoad, pData + 0x204, 4);
    memcpy(&g_iPlayerCount, pData + 0x208, 4);

    CALL_PLUGINS(PLUGIN_HkIServerImpl_Update, int, __stdcall, (), ());

    int result = 0;
    EXECUTE_SERVER_CALL(result = Server.Update());
    return result;
}

/**************************************************************************************************************
Chat-Messages are hooked here
<Parameters>
cId:       Sender's ClientID
lP1:       size of rdlReader (used when extracting text from that buffer)
rdlReader: RenderDisplayList which contains the chat-text
cIdTo:     recipient's clientid(0x10000 = universe chat else when (cIdTo &
0x10000) = true -> system chat) iP2:       ???
**************************************************************************************************************/

CInGame admin;
bool g_bInSubmitChat = false;
uint g_iTextLen = 0;

void __stdcall SubmitChat(struct CHAT_ID cId, unsigned long lP1,
                          void const *rdlReader, struct CHAT_ID cIdTo,
                          int iP2) {
    ISERVER_LOG();
    ISERVER_LOGARG_I(cId.iID);
    ISERVER_LOGARG_I(cIdTo.iID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SubmitChat, __stdcall,
                   (struct CHAT_ID cId, unsigned long lP1,
                    void const *rdlReader, struct CHAT_ID cIdTo, int iP2),
                   (cId, lP1, rdlReader, cIdTo, iP2));

    TRY_HOOK {

        // Group join/leave commands
        if (cIdTo.iID == 0x10004) {
            g_bInSubmitChat = true;
            EXECUTE_SERVER_CALL(
                Server.SubmitChat(cId, lP1, rdlReader, cIdTo, iP2));
            g_bInSubmitChat = false;
            return;
        }

        // extract text from rdlReader
        BinaryRDLReader rdl;
        wchar_t wszBuf[1024] = L"";
        uint iRet1;
        rdl.extract_text_from_buffer((unsigned short *)wszBuf, sizeof(wszBuf),
                                     iRet1, (const char *)rdlReader, lP1);
        std::wstring wscBuf = wszBuf;
        uint iClientID = cId.iID;

        // if this is a message in system chat then convert it to local unless
        // explicitly overriden by the player using /s.
        if (set_bDefaultLocalChat && cIdTo.iID == 0x10001) {
            cIdTo.iID = 0x10002;
        }

        // fix flserver commands and change chat to id so that event logging is
        // accurate.
        g_iTextLen = (uint)wscBuf.length();
        if (!wscBuf.find(L"/g ")) {
            cIdTo.iID = 0x10003;
            g_iTextLen -= 3;
        } else if (!wscBuf.find(L"/l ")) {
            cIdTo.iID = 0x10002;
            g_iTextLen -= 3;
        } else if (!wscBuf.find(L"/s ")) {
            cIdTo.iID = 0x10001;
            g_iTextLen -= 3;
        } else if (!wscBuf.find(L"/u ")) {
            cIdTo.iID = 0x10000;
            g_iTextLen -= 3;
        } else if (!wscBuf.find(L"/group ")) {
            cIdTo.iID = 0x10003;
            g_iTextLen -= 7;
        } else if (!wscBuf.find(L"/local ")) {
            cIdTo.iID = 0x10002;
            g_iTextLen -= 7;
        } else if (!wscBuf.find(L"/system ")) {
            cIdTo.iID = 0x10001;
            g_iTextLen -= 8;
        } else if (!wscBuf.find(L"/universe ")) {
            cIdTo.iID = 0x10000;
            g_iTextLen -= 10;
        }

        ISERVER_LOGARG_WS(wszBuf);
        ISERVER_LOGARG_I(g_iTextLen);

        // check for user cmds
        if (UserCmd_Process(iClientID, wscBuf))
            return;

        if (wszBuf[0] == '.') { // flhook admin command
            CAccount *acc = Players.FindAccountFromClientID(iClientID);
            std::wstring wscAccDirname;

            HkGetAccountDirName(acc, wscAccDirname);
            std::string scAdminFile =
                scAcctPath + wstos(wscAccDirname) + "\\flhookadmin.ini";
            WIN32_FIND_DATA fd;
            HANDLE hFind = FindFirstFile(scAdminFile.c_str(), &fd);
            if (hFind != INVALID_HANDLE_VALUE) { // is admin
                FindClose(hFind);
                admin.ReadRights(scAdminFile);
                admin.iClientID = iClientID;
                admin.wscAdminName =
                    (wchar_t *)Players.GetActiveCharacterName(iClientID);
                admin.ExecuteCommandString(wszBuf + 1);
                return;
            }
        }

        // process chat event
        std::wstring wscEvent;
        wscEvent.reserve(256);
        wscEvent = L"chat";
        wscEvent += L" from=";
        const wchar_t *wszFrom =
            (const wchar_t *)Players.GetActiveCharacterName(cId.iID);
        if (!cId.iID)
            wscEvent += L"console";
        else if (!wszFrom)
            wscEvent += L"unknown";
        else
            wscEvent += wszFrom;

        wscEvent += L" id=";
        wscEvent += std::to_wstring(cId.iID);

        wscEvent += L" type=";
        if (cIdTo.iID == 0x00010000)
            wscEvent += L"universe";
        else if (cIdTo.iID == 0x10003) {
            wscEvent += L"group";
            wscEvent += L" grpidto=";
            wscEvent += std::to_wstring(Players.GetGroupID(cId.iID));
        } else if (cIdTo.iID & 0x00010000)
            wscEvent += L"system";
        else {
            wscEvent += L"player";
            wscEvent += L" to=";

            const wchar_t *wszTo =
                (const wchar_t *)Players.GetActiveCharacterName(cIdTo.iID);
            if (!cIdTo.iID)
                wscEvent += L"console";
            else if (!wszTo)
                wscEvent += L"unknown";
            else
                wscEvent += wszTo;

            wscEvent += L" idto=";
            wscEvent += std::to_wstring(cIdTo.iID);
        }

        wscEvent += L" text=";
        wscEvent += wscBuf;
        ProcessEvent(L"%s", wscEvent.c_str());

        // check if chat should be suppressed
        for (auto &chat : set_setChatSuppress) {
            if ((ToLower(wscBuf)).find(ToLower(chat)) == 0)
                return;
        }
    }
    CATCH_HOOK({})

    // send
    g_bInSubmitChat = true;
    EXECUTE_SERVER_CALL(Server.SubmitChat(cId, lP1, rdlReader, cIdTo, iP2));
    g_bInSubmitChat = false;

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SubmitChat_AFTER, __stdcall,
                   (struct CHAT_ID cId, unsigned long lP1,
                    void const *rdlReader, struct CHAT_ID cIdTo, int iP2),
                   (cId, lP1, rdlReader, cIdTo, iP2));
}

/**************************************************************************************************************
Called when player ship was created in space (after undock or login)
**************************************************************************************************************/

void __stdcall PlayerLaunch(unsigned int iShip, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iShip);
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    TRY_HOOK {
        ClientInfo[iClientID].iShip = iShip;
        ClientInfo[iClientID].iKillsInARow = 0;
        ClientInfo[iClientID].bCruiseActivated = false;
        ClientInfo[iClientID].bThrusterActivated = false;
        ClientInfo[iClientID].bEngineKilled = false;
        ClientInfo[iClientID].bTradelane = false;

        // adjust cash, this is necessary when cash was added while use was in
        // charmenu/had other char selected
        std::wstring wscCharname =
            ToLower((wchar_t *)Players.GetActiveCharacterName(iClientID));
        for (auto &i : ClientInfo[iClientID].lstMoneyFix) {
            if (!i.wscCharname.compare(wscCharname)) {
                HkAddCash(wscCharname, i.iAmount);
                ClientInfo[iClientID].lstMoneyFix.remove(i);
                break;
            }
        }
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_PlayerLaunch, __stdcall,
                   (unsigned int iShip, unsigned int iClientID),
                   (iShip, iClientID));

    EXECUTE_SERVER_CALL(Server.PlayerLaunch(iShip, iClientID));

    TRY_HOOK {
        if (!ClientInfo[iClientID].iLastExitedBaseID) {
            ClientInfo[iClientID].iLastExitedBaseID = 1;

            // event
            ProcessEvent(L"spawn char=%s id=%d system=%s",
                         (wchar_t *)Players.GetActiveCharacterName(iClientID),
                         iClientID, HkGetPlayerSystem(iClientID).c_str());
        }
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_PlayerLaunch_AFTER, __stdcall,
                   (unsigned int iShip, unsigned int iClientID),
                   (iShip, iClientID));
}

/**************************************************************************************************************
Called when player fires a weapon
**************************************************************************************************************/

void __stdcall FireWeapon(unsigned int iClientID,
                          struct XFireWeaponInfo const &wpn) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_FireWeapon, __stdcall,
                   (unsigned int iClientID, struct XFireWeaponInfo const &wpn),
                   (iClientID, wpn));

    EXECUTE_SERVER_CALL(Server.FireWeapon(iClientID, wpn));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_FireWeapon_AFTER, __stdcall,
                   (unsigned int iClientID, struct XFireWeaponInfo const &wpn),
                   (iClientID, wpn));
}

/**************************************************************************************************************
Called when one player hits a target with a gun
<Parameters>
ci:  only figured out where dwTargetShip is ...
**************************************************************************************************************/

void __stdcall SPMunitionCollision(struct SSPMunitionCollisionInfo const &ci,
                                   unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    uint iClientIDTarget;

    CHECK_FOR_DISCONNECT

    TRY_HOOK { iClientIDTarget = HkGetClientIDByShip(ci.dwTargetShip); }
    CATCH_HOOK({})

    iDmgTo = iClientIDTarget;

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_SPMunitionCollision, __stdcall,
        (struct SSPMunitionCollisionInfo const &ci, unsigned int iClientID),
        (ci, iClientID));

    EXECUTE_SERVER_CALL(Server.SPMunitionCollision(ci, iClientID));

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_SPMunitionCollision_AFTER, __stdcall,
        (struct SSPMunitionCollisionInfo const &ci, unsigned int iClientID),
        (ci, iClientID));
}

/**************************************************************************************************************
Called when player moves his ship
**************************************************************************************************************/

void __stdcall SPObjUpdate(struct SSPObjUpdateInfo const &ui,
                           unsigned int iClientID) {
    /*ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    ISERVER_LOGARG_UI(ui.iShip);
    ISERVER_LOGARG_V(ui.vPos);
    ISERVER_LOGARG_Q(ui.vDir);
    ISERVER_LOGARG_F(ui.throttle);*/

    CHECK_FOR_DISCONNECT;

    // NAN check
    if (!(ui.vPos.x == ui.vPos.x) || !(ui.vPos.y == ui.vPos.y) ||
        !(ui.vPos.z == ui.vPos.z) || !(ui.vDir.x == ui.vDir.x) ||
        !(ui.vDir.y == ui.vDir.y) || !(ui.vDir.z == ui.vDir.z) ||
        !(ui.vDir.w == ui.vDir.w) || !(ui.fThrottle == ui.fThrottle)) {
        AddLog("ERROR: NAN found in " __FUNCTION__ " for id=%u", iClientID);
        HkKick(Players[iClientID].Account);
        return;
    };

    float n = ui.vDir.w * ui.vDir.w + ui.vDir.x * ui.vDir.x +
              ui.vDir.y * ui.vDir.y + ui.vDir.z * ui.vDir.z;
    if (n > 1.21f || n < 0.81f) {
        AddLog(
            "ERROR: Non-normalized quaternion found in " __FUNCTION__ " for "
                                                                      "id=%u",
            iClientID);
        HkKick(Players[iClientID].Account);
        return;
    }

    // Far check
    if (abs(ui.vPos.x) > 1e7f || abs(ui.vPos.y) > 1e7f ||
        abs(ui.vPos.z) > 1e7f) {
        AddLog(
            "ERROR: Ship position out of bounds in " __FUNCTION__ " for id=%u",
            iClientID);
        HkKick(Players[iClientID].Account);
        return;
    }

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SPObjUpdate, __stdcall,
                   (struct SSPObjUpdateInfo const &ui, unsigned int iClientID),
                   (ui, iClientID));

    EXECUTE_SERVER_CALL(Server.SPObjUpdate(ui, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SPObjUpdate_AFTER, __stdcall,
                   (struct SSPObjUpdateInfo const &ui, unsigned int iClientID),
                   (ui, iClientID));
}
/**************************************************************************************************************
Called when one player collides with a space object
**************************************************************************************************************/

void __stdcall SPObjCollision(struct SSPObjCollisionInfo const &ci,
                              unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_SPObjCollision, __stdcall,
        (struct SSPObjCollisionInfo const &ci, unsigned int iClientID),
        (ci, iClientID));

    EXECUTE_SERVER_CALL(Server.SPObjCollision(ci, iClientID));

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_SPObjCollision_AFTER, __stdcall,
        (struct SSPObjCollisionInfo const &ci, unsigned int iClientID),
        (ci, iClientID));
}

/**************************************************************************************************************
Called when player has undocked and is now ready to fly
**************************************************************************************************************/

void __stdcall LaunchComplete(unsigned int iBaseID, unsigned int iShip) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iBaseID);
    ISERVER_LOGARG_UI(iShip);

    TRY_HOOK {
        uint iClientID = HkGetClientIDByShip(iShip);
        if (iClientID) {
            ClientInfo[iClientID].tmSpawnTime =
                timeInMS(); // save for anti-dockkill
            // is there spawnprotection?
            if (set_iAntiDockKill > 0)
                ClientInfo[iClientID].bSpawnProtected = true;
            else
                ClientInfo[iClientID].bSpawnProtected = false;
        }

        // event
        ProcessEvent(
            L"launch char=%s id=%d base=%s system=%s",
            (wchar_t *)Players.GetActiveCharacterName(iClientID), iClientID,
            HkGetBaseNickByID(ClientInfo[iClientID].iLastExitedBaseID).c_str(),
            HkGetPlayerSystem(iClientID).c_str());
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_LaunchComplete, __stdcall,
                   (unsigned int iBaseID, unsigned int iShip),
                   (iBaseID, iShip));

    EXECUTE_SERVER_CALL(Server.LaunchComplete(iBaseID, iShip));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_LaunchComplete_AFTER, __stdcall,
                   (unsigned int iBaseID, unsigned int iShip),
                   (iBaseID, iShip));
}

/**************************************************************************************************************
Called when player selects a character
**************************************************************************************************************/

void __stdcall CharacterSelect(struct CHARACTER_ID const &cId,
                               unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_S(&cId);
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_CharacterSelect, __stdcall,
                   (struct CHARACTER_ID const &cId, unsigned int iClientID),
                   (cId, iClientID));

    // Get GroupID and store against client
    if (set_bPersistGroup)
        HkGetGroupID(iClientID, ClientInfo[iClientID].iGroupID);

    std::wstring wscCharBefore;
    try {
        const wchar_t *wszCharname =
            (wchar_t *)Players.GetActiveCharacterName(iClientID);
        wscCharBefore =
            wszCharname ? (wchar_t *)Players.GetActiveCharacterName(iClientID)
                        : L"";
        ClientInfo[iClientID].iLastExitedBaseID = 0;
        ClientInfo[iClientID].iTradePartner = 0;
        Server.CharacterSelect(cId, iClientID);

        // Add player back to group
        if (set_bPersistGroup)
            HkAddToGroup(iClientID, ClientInfo[iClientID].iGroupID);
    } catch (...) {
        HkAddKickLog(iClientID, L"Corrupt charfile?");
        HkKick(ARG_CLIENTID(iClientID));
        return;
    }

    TRY_HOOK {
        std::wstring wscCharname =
            (wchar_t *)Players.GetActiveCharacterName(iClientID);

        if (wscCharBefore.compare(wscCharname) != 0) {
            LoadUserCharSettings(iClientID);

            if (set_bUserCmdHelp)
                PrintUserCmdText(iClientID,
                                 L"To get a list of available commands, type "
                                 L"\"/help\" in chat.");

            // anti-cheat check
            std::list<CARGO_INFO> lstCargo;
            int iHold;
            HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, iHold);
            for (auto &cargo : lstCargo) {
                if (cargo.iCount < 0) {
                    HkAddCheaterLog(wscCharname,
                                    L"Negative good-count, likely to have "
                                    L"cheated in the past");

                    wchar_t wszBuf[256];
                    swprintf_s(wszBuf, L"Possible cheating detected (%s)",
                               wscCharname.c_str());
                    HkMsgU(wszBuf);
                    HkBan(ARG_CLIENTID(iClientID), true);
                    HkKick(ARG_CLIENTID(iClientID));
                    return;
                }
            }

            // event
            CAccount *acc = Players.FindAccountFromClientID(iClientID);
            std::wstring wscDir;
            HkGetAccountDirName(acc, wscDir);
            HKPLAYERINFO pi;
            HkGetPlayerInfo(ARG_CLIENTID(iClientID), pi, false);
            ProcessEvent(L"login char=%s accountdirname=%s id=%d ip=%s",
                         wscCharname.c_str(), wscDir.c_str(), iClientID,
                         pi.wscIP.c_str());
        }
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_CharacterSelect_AFTER, __stdcall,
                   (struct CHARACTER_ID const &cId, unsigned int iClientID),
                   (cId, iClientID));
}

/**************************************************************************************************************
Called when player enters base
**************************************************************************************************************/

void __stdcall BaseEnter(unsigned int iBaseID, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iBaseID);
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_BaseEnter, __stdcall,
                   (unsigned int iBaseID, unsigned int iClientID),
                   (iBaseID, iClientID));

    TRY_HOOK {
        // autobuy
        if (set_bAutoBuy)
            HkPlayerAutoBuy(iClientID, iBaseID);
    }
    CATCH_HOOK({ AddLog("Exception in " __FUNCTION__ " on autobuy"); })

    EXECUTE_SERVER_CALL(Server.BaseEnter(iBaseID, iClientID));

    TRY_HOOK {
        // adjust cash, this is necessary when cash was added while use was in
        // charmenu/had other char selected
        std::wstring wscCharname =
            ToLower((wchar_t *)Players.GetActiveCharacterName(iClientID));
        for (auto &i : ClientInfo[iClientID].lstMoneyFix) {
            if (!i.wscCharname.compare(wscCharname)) {
                HkAddCash(wscCharname, i.iAmount);
                ClientInfo[iClientID].lstMoneyFix.remove(i);
                break;
            }
        }

        // anti base-idle
        ClientInfo[iClientID].iBaseEnterTime = (uint)time(0);

        // event
        ProcessEvent(L"baseenter char=%s id=%d base=%s system=%s",
                     (wchar_t *)Players.GetActiveCharacterName(iClientID),
                     iClientID, HkGetBaseNickByID(iBaseID).c_str(),
                     HkGetPlayerSystem(iClientID).c_str());
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_BaseEnter_AFTER, __stdcall,
                   (unsigned int iBaseID, unsigned int iClientID),
                   (iBaseID, iClientID));
}

/**************************************************************************************************************
Called when player exits base
**************************************************************************************************************/

void __stdcall BaseExit(unsigned int iBaseID, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iBaseID);
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    TRY_HOOK {
        ClientInfo[iClientID].iBaseEnterTime = 0;
        ClientInfo[iClientID].iLastExitedBaseID = iBaseID;
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_BaseExit, __stdcall,
                   (unsigned int iBaseID, unsigned int iClientID),
                   (iBaseID, iClientID));

    EXECUTE_SERVER_CALL(Server.BaseExit(iBaseID, iClientID));

    TRY_HOOK {
        const wchar_t *wszCharname =
            (wchar_t *)Players.GetActiveCharacterName(iClientID);

        // event
        ProcessEvent(L"baseexit char=%s id=%d base=%s system=%s",
                     (wchar_t *)Players.GetActiveCharacterName(iClientID),
                     iClientID, HkGetBaseNickByID(iBaseID).c_str(),
                     HkGetPlayerSystem(iClientID).c_str());
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_BaseExit_AFTER, __stdcall,
                   (unsigned int iBaseID, unsigned int iClientID),
                   (iBaseID, iClientID));
}
/**************************************************************************************************************
Called when player connects
**************************************************************************************************************/

void __stdcall OnConnect(unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    TRY_HOOK {
        // If ID is too high due to disconnect buffer time then manually drop
        // the connection.
        if (iClientID > MAX_CLIENT_ID) {
            AddLog("INFO: Blocking connect in " __FUNCTION__ " due to invalid "
                                                             "id, id=%u",
                   iClientID);
            CDPClientProxy *cdpClient = g_cClientProxyArray[iClientID - 1];
            if (!cdpClient)
                return;
            cdpClient->Disconnect();
            return;
        }

        // If this client is in the anti-F1 timeout then force the disconnect.
        if (ClientInfo[iClientID].tmF1TimeDisconnect > timeInMS()) {
            // manual disconnect
            CDPClientProxy *cdpClient = g_cClientProxyArray[iClientID - 1];
            if (!cdpClient)
                return;
            cdpClient->Disconnect();
            return;
        }

        ClientInfo[iClientID].iConnects++;
        ClearClientInfo(iClientID);
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_OnConnect, __stdcall,
                   (unsigned int iClientID), (iClientID));

    EXECUTE_SERVER_CALL(Server.OnConnect(iClientID));

    TRY_HOOK {
        // event
        std::wstring wscIP;
        HkGetPlayerIP(iClientID, wscIP);
        ProcessEvent(L"connect id=%d ip=%s", iClientID, wscIP.c_str());
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_OnConnect_AFTER, __stdcall,
                   (unsigned int iClientID), (iClientID));
}

/**************************************************************************************************************
Called when player disconnects
**************************************************************************************************************/

void __stdcall DisConnect(unsigned int iClientID, enum EFLConnection p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    ISERVER_LOGARG_UI(p2);

    std::wstring wscCharname;
    TRY_HOOK {
        if (!ClientInfo[iClientID].bDisconnected) {
            ClientInfo[iClientID].bDisconnected = true;
            ClientInfo[iClientID].lstMoneyFix.clear();
            ClientInfo[iClientID].iTradePartner = 0;

            // event
            const wchar_t *wszCharname =
                (const wchar_t *)Players.GetActiveCharacterName(iClientID);
            if (wszCharname)
                wscCharname = wszCharname;
            ProcessEvent(L"disconnect char=%s id=%d", wscCharname.c_str(),
                         iClientID);

            CALL_PLUGINS_V(PLUGIN_HkIServerImpl_DisConnect, __stdcall,
                           (unsigned int iClientID, enum EFLConnection p2),
                           (iClientID, p2));
            EXECUTE_SERVER_CALL(Server.DisConnect(iClientID, p2));
            CALL_PLUGINS_V(PLUGIN_HkIServerImpl_DisConnect_AFTER, __stdcall,
                           (unsigned int iClientID, enum EFLConnection p2),
                           (iClientID, p2));
        }
    }
    CATCH_HOOK({
        AddLog("ERROR: Exception in " __FUNCTION__ "@loc2 charname=%s "
                                                   "iClientID=%u",
               wstos(wscCharname).c_str(), iClientID);
    })
}

/**************************************************************************************************************
Called when trade is being terminated
**************************************************************************************************************/

void __stdcall TerminateTrade(unsigned int iClientID, int iAccepted) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    ISERVER_LOGARG_I(iAccepted);

    CHECK_FOR_DISCONNECT

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_TerminateTrade, __stdcall,
                   (unsigned int iClientID, int iAccepted),
                   (iClientID, iAccepted));

    EXECUTE_SERVER_CALL(Server.TerminateTrade(iClientID, iAccepted));

    TRY_HOOK {
        if (iAccepted) { // save both chars to prevent cheating in case of
                         // server crash
            HkSaveChar(ARG_CLIENTID(iClientID));
            if (ClientInfo[iClientID].iTradePartner)
                HkSaveChar(ARG_CLIENTID(ClientInfo[iClientID].iTradePartner));
        }

        if (ClientInfo[iClientID].iTradePartner)
            ClientInfo[ClientInfo[iClientID].iTradePartner].iTradePartner = 0;
        ClientInfo[iClientID].iTradePartner = 0;
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_TerminateTrade_AFTER, __stdcall,
                   (unsigned int iClientID, int iAccepted),
                   (iClientID, iAccepted));
}

/**************************************************************************************************************
Called when new trade request
**************************************************************************************************************/

void __stdcall InitiateTrade(unsigned int iClientID1, unsigned int iClientID2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID1);
    ISERVER_LOGARG_UI(iClientID2);

    TRY_HOOK {
        // save traders client-ids
        ClientInfo[iClientID1].iTradePartner = iClientID2;
        ClientInfo[iClientID2].iTradePartner = iClientID1;
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_InitiateTrade, __stdcall,
                   (unsigned int iClientID1, unsigned int iClientID2),
                   (iClientID1, iClientID2));

    EXECUTE_SERVER_CALL(Server.InitiateTrade(iClientID1, iClientID2));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_InitiateTrade_AFTER, __stdcall,
                   (unsigned int iClientID1, unsigned int iClientID2),
                   (iClientID1, iClientID2));
}

/**************************************************************************************************************
Called when equipment is being activated/disabled
**************************************************************************************************************/

void __stdcall ActivateEquip(unsigned int iClientID,
                             struct XActivateEquip const &aq) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    TRY_HOOK {

        std::list<CARGO_INFO> lstCargo;
        int iRem;
        HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, iRem);

        for (auto &cargo : lstCargo) {
            if (cargo.iID == aq.sID) {
                Archetype::Equipment *eq =
                    Archetype::GetEquipment(cargo.iArchID);
                EQ_TYPE eqType = HkGetEqType(eq);

                if (eqType == ET_ENGINE) {
                    ClientInfo[iClientID].bEngineKilled = !aq.bActivate;
                    if (!aq.bActivate)
                        ClientInfo[iClientID].bCruiseActivated =
                            false; // enginekill enabled
                }
            }
        }
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ActivateEquip, __stdcall,
                   (unsigned int iClientID, struct XActivateEquip const &aq),
                   (iClientID, aq));

    EXECUTE_SERVER_CALL(Server.ActivateEquip(iClientID, aq));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ActivateEquip_AFTER, __stdcall,
                   (unsigned int iClientID, struct XActivateEquip const &aq),
                   (iClientID, aq));
}

/**************************************************************************************************************
Called when cruise engine is being activated/disabled
**************************************************************************************************************/

void __stdcall ActivateCruise(unsigned int iClientID,
                              struct XActivateCruise const &ac) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    TRY_HOOK { ClientInfo[iClientID].bCruiseActivated = ac.bActivate; }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ActivateCruise, __stdcall,
                   (unsigned int iClientID, struct XActivateCruise const &ac),
                   (iClientID, ac));

    EXECUTE_SERVER_CALL(Server.ActivateCruise(iClientID, ac));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ActivateCruise_AFTER, __stdcall,
                   (unsigned int iClientID, struct XActivateCruise const &ac),
                   (iClientID, ac));
}

/**************************************************************************************************************
Called when thruster is being activated/disabled
**************************************************************************************************************/

void __stdcall ActivateThrusters(unsigned int iClientID,
                                 struct XActivateThrusters const &at) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    TRY_HOOK { ClientInfo[iClientID].bThrusterActivated = at.bActivate; }
    CATCH_HOOK({})

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_ActivateThrusters, __stdcall,
        (unsigned int iClientID, struct XActivateThrusters const &at),
        (iClientID, at));

    EXECUTE_SERVER_CALL(Server.ActivateThrusters(iClientID, at));

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_ActivateThrusters_AFTER, __stdcall,
        (unsigned int iClientID, struct XActivateThrusters const &at),
        (iClientID, at));
}

/**************************************************************************************************************
Called when player sells good on a base
**************************************************************************************************************/

void __stdcall GFGoodSell(struct SGFGoodSellInfo const &gsi,
                          unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    TRY_HOOK {
        // anti-cheat check
        std::list<CARGO_INFO> lstCargo;
        int iHold;
        HkEnumCargo(ARG_CLIENTID(iClientID), lstCargo, iHold);
        bool bLegalSell = false;
        for (auto &cargo : lstCargo) {
            if (cargo.iArchID == gsi.iArchID) {
                bLegalSell = true;
                if (abs(gsi.iCount) > cargo.iCount) {
                    wchar_t wszBuf[1000];

                    const wchar_t *wszCharname =
                        (wchar_t *)Players.GetActiveCharacterName(iClientID);
                    swprintf_s(
                        wszBuf,
                        L"Sold more good than possible item=%08x count=%u",
                        gsi.iArchID, gsi.iCount);
                    HkAddCheaterLog(wszCharname, wszBuf);

                    swprintf_s(wszBuf, L"Possible cheating detected (%s)",
                               wszCharname);
                    HkMsgU(wszBuf);
                    HkBan(ARG_CLIENTID(iClientID), true);
                    HkKick(ARG_CLIENTID(iClientID));
                    return;
                }
                break;
            }
        }
        if (!bLegalSell) {
            wchar_t wszBuf[1000];
            const wchar_t *wszCharname =
                (wchar_t *)Players.GetActiveCharacterName(iClientID);
            swprintf_s(
                wszBuf,
                L"Sold good player does not have (buggy test), item=%08x",
                gsi.iArchID);
            HkAddCheaterLog(wszCharname, wszBuf);

            // swprintf(wszBuf, L"Possible cheating detected (%s)",
            // wszCharname); HkMsgU(wszBuf); HkBan(ARG_CLIENTID(iClientID),
            // true); HkKick(ARG_CLIENTID(iClientID));
            return;
        }
    }
    CATCH_HOOK({
        AddLog("Exception in %s (iClientID=%u (%x))", __FUNCTION__, iClientID,
               Players.GetActiveCharacterName(iClientID));
    })

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_GFGoodSell, __stdcall,
                   (struct SGFGoodSellInfo const &gsi, unsigned int iClientID),
                   (gsi, iClientID));

    EXECUTE_SERVER_CALL(Server.GFGoodSell(gsi, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_GFGoodSell_AFTER, __stdcall,
                   (struct SGFGoodSellInfo const &gsi, unsigned int iClientID),
                   (gsi, iClientID));
}

/**************************************************************************************************************
Called when player connects or pushes f1
**************************************************************************************************************/

void __stdcall CharacterInfoReq(unsigned int iClientID, bool p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    ISERVER_LOGARG_UI(p2);

    CHECK_FOR_DISCONNECT

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_CharacterInfoReq, __stdcall,
                   (unsigned int iClientID, bool p2), (iClientID, p2));

    try {
        if (!ClientInfo[iClientID].bCharSelected)
            ClientInfo[iClientID].bCharSelected = true;
        else { // pushed f1
            uint iShip = 0;
            pub::Player::GetShip(iClientID, iShip);
            if (iShip) { // in space
                ClientInfo[iClientID].tmF1Time = timeInMS() + set_iAntiF1;
                return;
            }
        }

        Server.CharacterInfoReq(iClientID, p2);
    } catch (...) { // something is wrong with charfile
        HkAddKickLog(iClientID, L"Corrupt charfile?");
        HkKick(ARG_CLIENTID(iClientID));
        return;
    }

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_CharacterInfoReq_AFTER, __stdcall,
                   (unsigned int iClientID, bool p2), (iClientID, p2));
}

/**************************************************************************************************************
Called when player jumps in system
**************************************************************************************************************/

void __stdcall JumpInComplete(unsigned int iSystemID, unsigned int iShip) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iSystemID);
    ISERVER_LOGARG_UI(iShip);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_JumpInComplete, __stdcall,
                   (unsigned int iSystemID, unsigned int iShip),
                   (iSystemID, iShip));

    EXECUTE_SERVER_CALL(Server.JumpInComplete(iSystemID, iShip));

    TRY_HOOK {
        uint iClientID = HkGetClientIDByShip(iShip);
        if (!iClientID)
            return;

        // event
        ProcessEvent(L"jumpin char=%s id=%d system=%s",
                     (wchar_t *)Players.GetActiveCharacterName(iClientID),
                     iClientID, HkGetSystemNickByID(iSystemID).c_str());
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_JumpInComplete_AFTER, __stdcall,
                   (unsigned int iSystemID, unsigned int iShip),
                   (iSystemID, iShip));
}

/**************************************************************************************************************
Called when player jumps out of system
**************************************************************************************************************/

void __stdcall SystemSwitchOutComplete(unsigned int iShip,
                                       unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iShip);
    ISERVER_LOGARG_UI(iClientID);

    CHECK_FOR_DISCONNECT

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SystemSwitchOutComplete, __stdcall,
                   (unsigned int iShip, unsigned int iClientID),
                   (iShip, iClientID));

    std::wstring wscSystem = HkGetPlayerSystem(iClientID);

    EXECUTE_SERVER_CALL(Server.SystemSwitchOutComplete(iShip, iClientID));

    TRY_HOOK {
        // event
        ProcessEvent(L"switchout char=%s id=%d system=%s",
                     (wchar_t *)Players.GetActiveCharacterName(iClientID),
                     iClientID, wscSystem.c_str());
    }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SystemSwitchOutComplete_AFTER,
                   __stdcall, (unsigned int iShip, unsigned int iClientID),
                   (iShip, iClientID));
}

/**************************************************************************************************************
Called when player logs in
**************************************************************************************************************/

void __stdcall Login(struct SLoginInfo const &li, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_WS(&li);
    ISERVER_LOGARG_UI(iClientID);

    TRY_HOOK {
        CALL_PLUGINS_V(PLUGIN_HkIServerImpl_Login_BEFORE, __stdcall,
                       (struct SLoginInfo const &li, unsigned int iClientID),
                       (li, iClientID));

        Server.Login(li, iClientID);

        if (iClientID > MAX_CLIENT_ID)
            return; // lalala DisconnectDelay bug

        if (!HkIsValidClientID(iClientID))
            return; // player was kicked

        // Kick the player if the account ID doesn't exist. This is caused
        // by a duplicate log on.
        CAccount *acc = Players.FindAccountFromClientID(iClientID);
        if (acc && !acc->wszAccID) {
            acc->ForceLogout();
            return;
        }

        CALL_PLUGINS_V(PLUGIN_HkIServerImpl_Login, __stdcall,
                       (struct SLoginInfo const &li, unsigned int iClientID),
                       (li, iClientID));

        // check for ip ban
        std::wstring wscIP;
        HkGetPlayerIP(iClientID, wscIP);

        for (auto &ban : set_setBans) {
            if (Wildcard::wildcardfit(wstos(ban).c_str(),
                                      wstos(wscIP).c_str())) {
                HkAddKickLog(iClientID, L"IP/Hostname ban(%s matches %s)",
                             wscIP.c_str(), ban.c_str());
                if (set_bBanAccountOnMatch)
                    HkBan(ARG_CLIENTID(iClientID), true);
                HkKick(ARG_CLIENTID(iClientID));
            }
        }

        // resolve
        RESOLVE_IP rip;
        rip.wscIP = wscIP;
        rip.wscHostname = L"";
        rip.iConnects =
            ClientInfo[iClientID].iConnects; // security check so that wrong
                                             // person doesnt get banned
        rip.iClientID = iClientID;
        EnterCriticalSection(&csIPResolve);
        g_lstResolveIPs.push_back(rip);
        LeaveCriticalSection(&csIPResolve);

        // count players
        struct PlayerData *pPD = 0;
        uint iPlayers = 0;
        while (pPD = Players.traverse_active(pPD))
            iPlayers++;

        if (iPlayers >
            (Players.GetMaxPlayerCount() -
             set_iReservedSlots)) { // check if player has a reserved slot
            CAccount *acc = Players.FindAccountFromClientID(iClientID);
            std::wstring wscDir;
            HkGetAccountDirName(acc, wscDir);
            std::string scUserFile =
                scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

            bool bReserved =
                IniGetB(scUserFile, "Settings", "ReservedSlot", false);
            if (!bReserved) {
                HkKick(acc);
                return;
            }
        }

        LoadUserSettings(iClientID);

        // log
        if (set_bLogConnects)
            HkAddConnectLog(iClientID, wscIP);
    }
    CATCH_HOOK({
        CAccount *acc = Players.FindAccountFromClientID(iClientID);
        if (acc) {
            acc->ForceLogout();
        }
    })

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_Login_AFTER, __stdcall,
                   (struct SLoginInfo const &li, unsigned int iClientID),
                   (li, iClientID));
}

/**************************************************************************************************************
Called on item spawn
**************************************************************************************************************/

void __stdcall MineAsteroid(unsigned int p1, class Vector const &vPos,
                            unsigned int iLookID, unsigned int iGoodID,
                            unsigned int iCount, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    //	ISERVER_LOGARG_UI(vPos);
    ISERVER_LOGARG_UI(iLookID);
    ISERVER_LOGARG_UI(iGoodID);
    ISERVER_LOGARG_UI(iCount);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_MineAsteroid, __stdcall,
                   (unsigned int p1, class Vector const &vPos,
                    unsigned int iLookID, unsigned int iGoodID,
                    unsigned int iCount, unsigned int iClientID),
                   (p1, vPos, iLookID, iGoodID, iCount, iClientID));

    EXECUTE_SERVER_CALL(
        Server.MineAsteroid(p1, vPos, iLookID, iGoodID, iCount, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_MineAsteroid_AFTER, __stdcall,
                   (unsigned int p1, class Vector const &vPos,
                    unsigned int iLookID, unsigned int iGoodID,
                    unsigned int iCount, unsigned int iClientID),
                   (p1, vPos, iLookID, iGoodID, iCount, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GoTradelane(unsigned int iClientID,
                           struct XGoTradelane const &gtl) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    TRY_HOOK { ClientInfo[iClientID].bTradelane = true; }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_GoTradelane, __stdcall,
                   (unsigned int iClientID, struct XGoTradelane const &gtl),
                   (iClientID, gtl));

    TRY_HOOK { Server.GoTradelane(iClientID, gtl); }
    CATCH_HOOK({
        uint iSystem;
        pub::Player::GetSystem(iClientID, iSystem);
        AddLog("ERROR: Exception in HkIServerImpl::GoTradelane charname=%s "
               "sys=%08x arch=%08x arch2=%08x",
               wstos((const wchar_t *)Players.GetActiveCharacterName(iClientID))
                   .c_str(),
               iSystem, gtl.iTradelaneSpaceObj1, gtl.iTradelaneSpaceObj2);
    })

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_GoTradelane_AFTER, __stdcall,
                   (unsigned int iClientID, struct XGoTradelane const &gtl),
                   (iClientID, gtl));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall StopTradelane(unsigned int iClientID, unsigned int p2,
                             unsigned int p3, unsigned int p4) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    ISERVER_LOGARG_UI(p2);
    ISERVER_LOGARG_UI(p3);
    ISERVER_LOGARG_UI(p4);

    TRY_HOOK { ClientInfo[iClientID].bTradelane = false; }
    CATCH_HOOK({})

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_StopTradelane, __stdcall,
                   (unsigned int iClientID, unsigned int p2, unsigned int p3,
                    unsigned int p4),
                   (iClientID, p2, p3, p4));

    EXECUTE_SERVER_CALL(Server.StopTradelane(iClientID, p2, p3, p4));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_StopTradelane_AFTER, __stdcall,
                   (unsigned int iClientID, unsigned int p2, unsigned int p3,
                    unsigned int p4),
                   (iClientID, p2, p3, p4));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall AbortMission(unsigned int iClientID, unsigned int p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    ISERVER_LOGARG_UI(p2);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_AbortMission, __stdcall,
                   (unsigned int iClientID, unsigned int p2), (iClientID, p2));

    EXECUTE_SERVER_CALL(Server.AbortMission(iClientID, p2));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_AbortMission_AFTER, __stdcall,
                   (unsigned int iClientID, unsigned int p2), (iClientID, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall AcceptTrade(unsigned int iClientID, bool p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    ISERVER_LOGARG_UI(p2);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_AcceptTrade, __stdcall,
                   (unsigned int iClientID, bool p2), (iClientID, p2));

    EXECUTE_SERVER_CALL(Server.AcceptTrade(iClientID, p2));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_AcceptTrade_AFTER, __stdcall,
                   (unsigned int iClientID, bool p2), (iClientID, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall AddTradeEquip(unsigned int iClientID,
                             struct EquipDesc const &ed) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_AddTradeEquip, __stdcall,
                   (unsigned int iClientID, struct EquipDesc const &ed),
                   (iClientID, ed));

    EXECUTE_SERVER_CALL(Server.AddTradeEquip(iClientID, ed));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_AddTradeEquip_AFTER, __stdcall,
                   (unsigned int iClientID, struct EquipDesc const &ed),
                   (iClientID, ed));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall BaseInfoRequest(unsigned int p1, unsigned int p2, bool p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(p2);
    ISERVER_LOGARG_UI(p3);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_BaseInfoRequest, __stdcall,
                   (unsigned int p1, unsigned int p2, bool p3), (p1, p2, p3));

    EXECUTE_SERVER_CALL(Server.BaseInfoRequest(p1, p2, p3));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_BaseInfoRequest_AFTER, __stdcall,
                   (unsigned int p1, unsigned int p2, bool p3), (p1, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall CharacterSkipAutosave(unsigned int iClientID) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall CommComplete(unsigned int p1, unsigned int p2, unsigned int p3,
                            enum CommResult cr) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall CreateNewCharacter(struct SCreateCharacterInfo const &scci,
                                  unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_CreateNewCharacter, __stdcall,
        (struct SCreateCharacterInfo const &scci, unsigned int iClientID),
        (scci, iClientID));

    EXECUTE_SERVER_CALL(Server.CreateNewCharacter(scci, iClientID));

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_CreateNewCharacter_AFTER, __stdcall,
        (struct SCreateCharacterInfo const &scci, unsigned int iClientID),
        (scci, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall DelTradeEquip(unsigned int iClientID,
                             struct EquipDesc const &ed) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_DelTradeEquip, __stdcall,
                   (unsigned int iClientID, struct EquipDesc const &ed),
                   (iClientID, ed));

    EXECUTE_SERVER_CALL(Server.DelTradeEquip(iClientID, ed));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_DelTradeEquip_AFTER, __stdcall,
                   (unsigned int iClientID, struct EquipDesc const &ed),
                   (iClientID, ed));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall DestroyCharacter(struct CHARACTER_ID const &cId,
                                unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    ISERVER_LOGARG_S(&cId);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_DestroyCharacter, __stdcall,
                   (struct CHARACTER_ID const &cId, unsigned int iClientID),
                   (cId, iClientID));

    EXECUTE_SERVER_CALL(Server.DestroyCharacter(cId, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_DestroyCharacter_AFTER, __stdcall,
                   (struct CHARACTER_ID const &cId, unsigned int iClientID),
                   (cId, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall Dock(unsigned int const &p1, unsigned int const &p2) {
    // anticheat - never let the client manually dock somewhere
    return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall DumpPacketStats(char const *p1) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ElapseTime(float p1) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GFGoodBuy(struct SGFGoodBuyInfo const &gbi,
                         unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_GFGoodBuy, __stdcall,
                   (struct SGFGoodBuyInfo const &gbi, unsigned int iClientID),
                   (gbi, iClientID));

    EXECUTE_SERVER_CALL(Server.GFGoodBuy(gbi, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_GFGoodBuy_AFTER, __stdcall,
                   (struct SGFGoodBuyInfo const &gbi, unsigned int iClientID),
                   (gbi, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GFGoodVaporized(struct SGFGoodVaporizedInfo const &gvi,
                               unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_GFGoodVaporized, __stdcall,
        (struct SGFGoodVaporizedInfo const &gvi, unsigned int iClientID),
        (gvi, iClientID));

    EXECUTE_SERVER_CALL(Server.GFGoodVaporized(gvi, iClientID));

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_GFGoodVaporized_AFTER, __stdcall,
        (struct SGFGoodVaporizedInfo const &gvi, unsigned int iClientID),
        (gvi, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GFObjSelect(unsigned int p1, unsigned int p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(p2);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_GFObjSelect, __stdcall,
                   (unsigned int p1, unsigned int p2), (p1, p2));

    EXECUTE_SERVER_CALL(Server.GFObjSelect(p1, p2));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_GFObjSelect_AFTER, __stdcall,
                   (unsigned int p1, unsigned int p2), (p1, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

unsigned int __stdcall GetServerID(void) {
    ISERVER_LOG();

    return Server.GetServerID();
}

/**************************************************************************************************************
**************************************************************************************************************/

char const *__stdcall GetServerSig(void) {
    ISERVER_LOG();

    return Server.GetServerSig();
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall GetServerStats(struct ServerStats &ss) {
    ISERVER_LOG();

    Server.GetServerStats(ss);
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall Hail(unsigned int p1, unsigned int p2, unsigned int p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(p2);
    ISERVER_LOGARG_UI(p3);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_Hail, __stdcall,
                   (unsigned int p1, unsigned int p2, unsigned int p3),
                   (p1, p2, p3));

    EXECUTE_SERVER_CALL(Server.Hail(p1, p2, p3));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_Hail_AFTER, __stdcall,
                   (unsigned int p1, unsigned int p2, unsigned int p3),
                   (p1, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall InterfaceItemUsed(unsigned int p1, unsigned int p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(p2);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_InterfaceItemUsed, __stdcall,
                   (unsigned int p1, unsigned int p2), (p1, p2));

    EXECUTE_SERVER_CALL(Server.InterfaceItemUsed(p1, p2));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_InterfaceItemUsed_AFTER, __stdcall,
                   (unsigned int p1, unsigned int p2), (p1, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall JettisonCargo(unsigned int iClientID,
                             struct XJettisonCargo const &jc) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_JettisonCargo, __stdcall,
                   (unsigned int iClientID, struct XJettisonCargo const &jc),
                   (iClientID, jc));

    EXECUTE_SERVER_CALL(Server.JettisonCargo(iClientID, jc));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_JettisonCargo_AFTER, __stdcall,
                   (unsigned int iClientID, struct XJettisonCargo const &jc),
                   (iClientID, jc));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall LocationEnter(unsigned int p1, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_LocationEnter, __stdcall,
                   (unsigned int p1, unsigned int iClientID), (p1, iClientID));

    EXECUTE_SERVER_CALL(Server.LocationEnter(p1, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_LocationEnter_AFTER, __stdcall,
                   (unsigned int p1, unsigned int iClientID), (p1, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall LocationExit(unsigned int p1, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_LocationExit, __stdcall,
                   (unsigned int p1, unsigned int iClientID), (p1, iClientID));

    EXECUTE_SERVER_CALL(Server.LocationExit(p1, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_LocationExit_AFTER, __stdcall,
                   (unsigned int p1, unsigned int iClientID), (p1, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall LocationInfoRequest(unsigned int p1, unsigned int p2, bool p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(p2);
    ISERVER_LOGARG_UI(p3);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_LocationInfoRequest, __stdcall,
                   (unsigned int p1, unsigned int p2, bool p3), (p1, p2, p3));

    EXECUTE_SERVER_CALL(Server.LocationInfoRequest(p1, p2, p3));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_LocationInfoRequest_AFTER, __stdcall,
                   (unsigned int p1, unsigned int p2, bool p3), (p1, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall MissionResponse(unsigned int p1, unsigned long p2, bool p3,
                               unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(p2);
    ISERVER_LOGARG_UI(p3);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_MissionResponse, __stdcall,
        (unsigned int p1, unsigned long p2, bool p3, unsigned int iClientID),
        (p1, p2, p3, iClientID));

    EXECUTE_SERVER_CALL(Server.MissionResponse(p1, p2, p3, iClientID));

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_MissionResponse_AFTER, __stdcall,
        (unsigned int p1, unsigned long p2, bool p3, unsigned int iClientID),
        (p1, p2, p3, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall MissionSaveB(unsigned int iClientID, unsigned long p2) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall PopUpDialog(unsigned int p1, unsigned int p2) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RTCDone(unsigned int p1, unsigned int p2) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqAddItem(unsigned int p1, char const *p2, int p3, float p4,
                          bool p5, unsigned int p6) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    //	ISERVER_LOGARG_S(p2);
    ISERVER_LOGARG_UI(p3);
    ISERVER_LOGARG_F(p4);
    ISERVER_LOGARG_UI(p5);
    ISERVER_LOGARG_UI(p6);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqAddItem, __stdcall,
                   (unsigned int p1, char const *p2, int p3, float p4, bool p5,
                    unsigned int p6),
                   (p1, p2, p3, p4, p5, p6));

    EXECUTE_SERVER_CALL(Server.ReqAddItem(p1, p2, p3, p4, p5, p6));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqAddItem_AFTER, __stdcall,
                   (unsigned int p1, char const *p2, int p3, float p4, bool p5,
                    unsigned int p6),
                   (p1, p2, p3, p4, p5, p6));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqChangeCash(int p1, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqChangeCash, __stdcall,
                   (int p1, unsigned int iClientID), (p1, iClientID));

    EXECUTE_SERVER_CALL(Server.ReqChangeCash(p1, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqChangeCash_AFTER, __stdcall,
                   (int p1, unsigned int iClientID), (p1, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqCollisionGroups(
    class st6::list<struct CollisionGroupDesc,
                    class st6::allocator<struct CollisionGroupDesc>> const &p1,
    unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_ReqCollisionGroups, __stdcall,
        (class st6::list<struct CollisionGroupDesc,
                         class st6::allocator<struct CollisionGroupDesc>> const
             &p1,
         unsigned int iClientID),
        (p1, iClientID));

    EXECUTE_SERVER_CALL(Server.ReqCollisionGroups(p1, iClientID));

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_ReqCollisionGroups_AFTER, __stdcall,
        (class st6::list<struct CollisionGroupDesc,
                         class st6::allocator<struct CollisionGroupDesc>> const
             &p1,
         unsigned int iClientID),
        (p1, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqDifficultyScale(float p1, unsigned int iClientID) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqEquipment(class EquipDescList const &edl,
                            unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqEquipment, __stdcall,
                   (class EquipDescList const &edl, unsigned int iClientID),
                   (edl, iClientID));

    EXECUTE_SERVER_CALL(Server.ReqEquipment(edl, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqEquipment_AFTER, __stdcall,
                   (class EquipDescList const &edl, unsigned int iClientID),
                   (edl, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqHullStatus(float p1, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_F(p1);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqHullStatus, __stdcall,
                   (float p1, unsigned int iClientID), (p1, iClientID));

    EXECUTE_SERVER_CALL(Server.ReqHullStatus(p1, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqHullStatus_AFTER, __stdcall,
                   (float p1, unsigned int iClientID), (p1, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqModifyItem(unsigned short p1, char const *p2, int p3,
                             float p4, bool p5, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    //	ISERVER_LOGARG_S(p2);
    ISERVER_LOGARG_I(p3);
    ISERVER_LOGARG_F(p4);
    ISERVER_LOGARG_UI(p5);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqModifyItem, __stdcall,
                   (unsigned short p1, char const *p2, int p3, float p4,
                    bool p5, unsigned int iClientID),
                   (p1, p2, p3, p4, p5, iClientID));

    EXECUTE_SERVER_CALL(Server.ReqModifyItem(p1, p2, p3, p4, p5, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqModifyItem_AFTER, __stdcall,
                   (unsigned short p1, char const *p2, int p3, float p4,
                    bool p5, unsigned int iClientID),
                   (p1, p2, p3, p4, p5, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqRemoveItem(unsigned short p1, int p2,
                             unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_I(p2);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqRemoveItem, __stdcall,
                   (unsigned short p1, int p2, unsigned int iClientID),
                   (p1, p2, iClientID));

    EXECUTE_SERVER_CALL(Server.ReqRemoveItem(p1, p2, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqRemoveItem_AFTER, __stdcall,
                   (unsigned short p1, int p2, unsigned int iClientID),
                   (p1, p2, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqSetCash(int p1, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_I(p1);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqSetCash, __stdcall,
                   (int p1, unsigned int iClientID), (p1, iClientID));

    EXECUTE_SERVER_CALL(Server.ReqSetCash(p1, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqSetCash_AFTER, __stdcall,
                   (int p1, unsigned int iClientID), (p1, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall ReqShipArch(unsigned int p1, unsigned int p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(p2);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqShipArch, __stdcall,
                   (unsigned int p1, unsigned int p2), (p1, p2));

    EXECUTE_SERVER_CALL(Server.ReqShipArch(p1, p2));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_ReqShipArch_AFTER, __stdcall,
                   (unsigned int p1, unsigned int p2), (p1, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestBestPath(unsigned int p1, unsigned char *p2, int p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    //	ISERVER_LOGARG_S(p2);
    ISERVER_LOGARG_I(p3);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestBestPath, __stdcall,
                   (unsigned int p1, unsigned char *p2, int p3), (p1, p2, p3));

    EXECUTE_SERVER_CALL(Server.RequestBestPath(p1, p2, p3));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestBestPath_AFTER, __stdcall,
                   (unsigned int p1, unsigned char *p2, int p3), (p1, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

// Cancel a ship maneuver (goto, dock, formation).
// p1 = iType? ==0 if docking, ==1 if formation
void __stdcall RequestCancel(int iType, unsigned int iShip, unsigned int p3,
                             unsigned long p4, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_I(iType);
    ISERVER_LOGARG_UI(iShip);
    ISERVER_LOGARG_UI(p3);
    ISERVER_LOGARG_UI(p4);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestCancel, __stdcall,
                   (int iType, unsigned int iShip, unsigned int p3,
                    unsigned long p4, unsigned int iClientID),
                   (iType, iShip, p3, p4, iClientID));

    EXECUTE_SERVER_CALL(Server.RequestCancel(iType, iShip, p3, p4, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestCancel_AFTER, __stdcall,
                   (int iType, unsigned int iShip, unsigned int p3,
                    unsigned long p4, unsigned int iClientID),
                   (iType, iShip, p3, p4, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestCreateShip(unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestCreateShip, __stdcall,
                   (unsigned int iClientID), (iClientID));

    EXECUTE_SERVER_CALL(Server.RequestCreateShip(iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestCreateShip_AFTER, __stdcall,
                   (unsigned int iClientID), (iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

/// Called upon flight maneuver (goto, dock, formation).
/// p1 = iType? ==0 if docking, ==1 if formation
/// p2 = iShip of person docking
/// p3 = iShip of dock/formation target
/// p4 seems to be 0 all the time
/// p5 seems to be 0 all the time
/// p6 = iClientID
void __stdcall RequestEvent(int iType, unsigned int iShip,
                            unsigned int iShipTarget, unsigned int p4,
                            unsigned long p5, unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_I(iType);
    ISERVER_LOGARG_UI(iShip);
    ISERVER_LOGARG_UI(iShipTarget);
    ISERVER_LOGARG_UI(p4);
    ISERVER_LOGARG_UI(p5);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestEvent, __stdcall,
                   (int iType, unsigned int iShip, unsigned int iShipTarget,
                    unsigned int p4, unsigned long p5, unsigned int iClientID),
                   (iType, iShip, iShipTarget, p4, p5, iClientID));

    EXECUTE_SERVER_CALL(
        Server.RequestEvent(iType, iShip, iShipTarget, p4, p5, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestEvent_AFTER, __stdcall,
                   (int iType, unsigned int iShip, unsigned int iShipTarget,
                    unsigned int p4, unsigned long p5, unsigned int iClientID),
                   (iType, iShip, iShipTarget, p4, p5, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestGroupPositions(unsigned int p1, unsigned char *p2,
                                     int p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    //	ISERVER_LOGARG_S(p2);
    ISERVER_LOGARG_I(p3);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestGroupPositions, __stdcall,
                   (unsigned int p1, unsigned char *p2, int p3), (p1, p2, p3));

    EXECUTE_SERVER_CALL(Server.RequestGroupPositions(p1, p2, p3));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestGroupPositions_AFTER, __stdcall,
                   (unsigned int p1, unsigned char *p2, int p3), (p1, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestPlayerStats(unsigned int p1, unsigned char *p2, int p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    //	ISERVER_LOGARG_S(p2);
    ISERVER_LOGARG_I(p3);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestPlayerStats, __stdcall,
                   (unsigned int p1, unsigned char *p2, int p3), (p1, p2, p3));

    EXECUTE_SERVER_CALL(Server.RequestPlayerStats(p1, p2, p3));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestPlayerStats_AFTER, __stdcall,
                   (unsigned int p1, unsigned char *p2, int p3), (p1, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestRankLevel(unsigned int p1, unsigned char *p2, int p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    //	ISERVER_LOGARG_S(p2);
    ISERVER_LOGARG_I(p3);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestRankLevel, __stdcall,
                   (unsigned int p1, unsigned char *p2, int p3), (p1, p2, p3));

    EXECUTE_SERVER_CALL(Server.RequestRankLevel(p1, p2, p3));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestRankLevel_AFTER, __stdcall,
                   (unsigned int p1, unsigned char *p2, int p3), (p1, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall RequestTrade(unsigned int p1, unsigned int p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(p2);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestTrade, __stdcall,
                   (unsigned int p1, unsigned int p2), (p1, p2));

    EXECUTE_SERVER_CALL(Server.RequestTrade(p1, p2));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_RequestTrade_AFTER, __stdcall,
                   (unsigned int p1, unsigned int p2), (p1, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SPBadLandsObjCollision(
    struct SSPBadLandsObjCollisionInfo const &p1, unsigned int iClientID) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

/// Called when ship starts jump gate/hole acceleration but before system switch
/// out.
void __stdcall SPRequestInvincibility(unsigned int iShip, bool p2,
                                      enum InvincibilityReason p3,
                                      unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iShip);
    ISERVER_LOGARG_UI(p2);
    ISERVER_LOGARG_UI(p3);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SPRequestInvincibility, __stdcall,
                   (unsigned int iShip, bool p2, enum InvincibilityReason p3,
                    unsigned int iClientID),
                   (iShip, p2, p3, iClientID));

    EXECUTE_SERVER_CALL(
        Server.SPRequestInvincibility(iShip, p2, p3, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SPRequestInvincibility_AFTER, __stdcall,
                   (unsigned int iShip, bool p2, enum InvincibilityReason p3,
                    unsigned int iClientID),
                   (iShip, p2, p3, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SPRequestUseItem(struct SSPUseItem const &p1,
                                unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SPRequestUseItem, __stdcall,
                   (struct SSPUseItem const &p1, unsigned int iClientID),
                   (p1, iClientID));

    EXECUTE_SERVER_CALL(Server.SPRequestUseItem(p1, iClientID));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SPRequestUseItem_AFTER, __stdcall,
                   (struct SSPUseItem const &p1, unsigned int iClientID),
                   (p1, iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SPScanCargo(unsigned int const &p1, unsigned int const &p2,
                           unsigned int p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    ISERVER_LOGARG_UI(p2);
    ISERVER_LOGARG_UI(p3);

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_SPScanCargo, __stdcall,
        (unsigned int const &p1, unsigned int const &p2, unsigned int p3),
        (p1, p2, p3));

    EXECUTE_SERVER_CALL(Server.SPScanCargo(p1, p2, p3));

    CALL_PLUGINS_V(
        PLUGIN_HkIServerImpl_SPScanCargo_AFTER, __stdcall,
        (unsigned int const &p1, unsigned int const &p2, unsigned int p3),
        (p1, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SaveGame(struct CHARACTER_ID const &cId,
                        unsigned short const *p2, unsigned int p3) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetInterfaceState(unsigned int p1, unsigned char *p2, int p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(p1);
    //	ISERVER_LOGARG_S(p2);
    ISERVER_LOGARG_I(p3);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetInterfaceState, __stdcall,
                   (unsigned int p1, unsigned char *p2, int p3), (p1, p2, p3));

    EXECUTE_SERVER_CALL(Server.SetInterfaceState(p1, p2, p3));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetInterfaceState_AFTER, __stdcall,
                   (unsigned int p1, unsigned char *p2, int p3), (p1, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetManeuver(unsigned int iClientID,
                           struct XSetManeuver const &p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetManeuver, __stdcall,
                   (unsigned int iClientID, struct XSetManeuver const &p2),
                   (iClientID, p2));

    EXECUTE_SERVER_CALL(Server.SetManeuver(iClientID, p2));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetManeuver_AFTER, __stdcall,
                   (unsigned int iClientID, struct XSetManeuver const &p2),
                   (iClientID, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetMissionLog(unsigned int iClientID, unsigned char *p2,
                             int p3) {
    return; // not used
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetTarget(unsigned int iClientID, struct XSetTarget const &p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetTarget, __stdcall,
                   (unsigned int iClientID, struct XSetTarget const &p2),
                   (iClientID, p2));

    EXECUTE_SERVER_CALL(Server.SetTarget(iClientID, p2));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetTarget_AFTER, __stdcall,
                   (unsigned int iClientID, struct XSetTarget const &p2),
                   (iClientID, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetTradeMoney(unsigned int iClientID, unsigned long p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    ISERVER_LOGARG_UI(p2);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetTradeMoney, __stdcall,
                   (unsigned int iClientID, unsigned long p2), (iClientID, p2));

    EXECUTE_SERVER_CALL(Server.SetTradeMoney(iClientID, p2));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetTradeMoney_AFTER, __stdcall,
                   (unsigned int iClientID, unsigned long p2), (iClientID, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetVisitedState(unsigned int iClientID, unsigned char *p2,
                               int p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    //	ISERVER_LOGARG_S(p2);
    ISERVER_LOGARG_I(p3);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetVisitedState, __stdcall,
                   (unsigned int iClientID, unsigned char *p2, int p3),
                   (iClientID, p2, p3));

    EXECUTE_SERVER_CALL(Server.SetVisitedState(iClientID, p2, p3));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetVisitedState_AFTER, __stdcall,
                   (unsigned int iClientID, unsigned char *p2, int p3),
                   (iClientID, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall SetWeaponGroup(unsigned int iClientID, unsigned char *p2,
                              int p3) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);
    //	ISERVER_LOGARG_S(p2);
    ISERVER_LOGARG_I(p3);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetWeaponGroup, __stdcall,
                   (unsigned int iClientID, unsigned char *p2, int p3),
                   (iClientID, p2, p3));

    EXECUTE_SERVER_CALL(Server.SetWeaponGroup(iClientID, p2, p3));

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_SetWeaponGroup_AFTER, __stdcall,
                   (unsigned int iClientID, unsigned char *p2, int p3),
                   (iClientID, p2, p3));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall Shutdown(void) {
    ISERVER_LOG();

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_Shutdown, __stdcall, (), ());

    Server.Shutdown();

    FLHookShutdown();
}

/**************************************************************************************************************
**************************************************************************************************************/

bool __stdcall Startup(struct SStartupInfo const &p1) {
    FLHookInit_Pre();

    // The maximum number of players we can support is MAX_CLIENT_ID
    // Add one to the maximum number to allow renames
    int iMaxPlayers = MAX_CLIENT_ID + 1;

    // Startup the server with this number of players.
    char *pAddress = ((char *)hModServer + ADDR_SRV_PLAYERDBMAXPLAYERSPATCH);
    char szNOP[] = {'\x90'};
    char szMOVECX[] = {'\xB9'};
    WriteProcMem(pAddress, szMOVECX, sizeof(szMOVECX));
    WriteProcMem(pAddress + 1, &iMaxPlayers, sizeof(iMaxPlayers));
    WriteProcMem(pAddress + 5, szNOP, sizeof(szNOP));

    CALL_PLUGINS_NORET(PLUGIN_HkIServerImpl_Startup, __stdcall,
                       (struct SStartupInfo const &p1), (p1));

    bool bRet = Server.Startup(p1);

    // Patch to set maximum number of players to connect. This is normally
    // less than MAX_CLIENT_ID
    pAddress = ((char *)hModServer + ADDR_SRV_PLAYERDBMAXPLAYERS);
    WriteProcMem(pAddress, (void *)&p1.iMaxPlayers, sizeof(iMaxPlayers));

    // read base market data from ini
    HkLoadBaseMarket();

    ISERVER_LOG();

    CALL_PLUGINS_NORET(PLUGIN_HkIServerImpl_Startup_AFTER, __stdcall,
                       (struct SStartupInfo const &p1), (p1));

    return bRet;
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall StopTradeRequest(unsigned int iClientID) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_StopTradeRequest, __stdcall,
                   (unsigned int iClientID), (iClientID));

    Server.StopTradeRequest(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_StopTradeRequest_AFTER, __stdcall,
                   (unsigned int iClientID), (iClientID));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall TractorObjects(unsigned int iClientID,
                              struct XTractorObjects const &p2) {
    ISERVER_LOG();
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_TractorObjects, __stdcall,
                   (unsigned int iClientID, struct XTractorObjects const &p2),
                   (iClientID, p2));

    Server.TractorObjects(iClientID, p2);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_TractorObjects_AFTER, __stdcall,
                   (unsigned int iClientID, struct XTractorObjects const &p2),
                   (iClientID, p2));
}

/**************************************************************************************************************
**************************************************************************************************************/

void __stdcall TradeResponse(unsigned char const *p1, int p2,
                             unsigned int iClientID) {
    ISERVER_LOG();
    ///	ISERVER_LOGARG_S(p1);
    ISERVER_LOGARG_I(p2);
    ISERVER_LOGARG_UI(iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_TradeResponse, __stdcall,
                   (unsigned char const *p1, int p2, unsigned int iClientID),
                   (p1, p2, iClientID));

    Server.TradeResponse(p1, p2, iClientID);

    CALL_PLUGINS_V(PLUGIN_HkIServerImpl_TradeResponse_AFTER, __stdcall,
                   (unsigned char const *p1, int p2, unsigned int iClientID),
                   (p1, p2, iClientID));
}

/**************************************************************************************************************
IServImpl hook entries
**************************************************************************************************************/

HOOKENTRY hookEntries[85] = {
    {(FARPROC)SubmitChat, -0x08, 0},
    {(FARPROC)FireWeapon, 0x000, 0},
    {(FARPROC)ActivateEquip, 0x004, 0},
    {(FARPROC)ActivateCruise, 0x008, 0},
    {(FARPROC)ActivateThrusters, 0x00C, 0},
    {(FARPROC)SetTarget, 0x010, 0},
    {(FARPROC)TractorObjects, 0x014, 0},
    {(FARPROC)GoTradelane, 0x018, 0},
    {(FARPROC)StopTradelane, 0x01C, 0},
    {(FARPROC)JettisonCargo, 0x020, 0},
    {(FARPROC)ElapseTime, 0x030, 0},
    {(FARPROC)DisConnect, 0x040, 0},
    {(FARPROC)OnConnect, 0x044, 0},
    {(FARPROC)Login, 0x048, 0},
    {(FARPROC)CharacterInfoReq, 0x04C, 0},
    {(FARPROC)CharacterSelect, 0x050, 0},
    {(FARPROC)CreateNewCharacter, 0x058, 0},
    {(FARPROC)DestroyCharacter, 0x05C, 0},
    {(FARPROC)CharacterSkipAutosave, 0x060, 0},
    {(FARPROC)ReqShipArch, 0x064, 0},
    {(FARPROC)ReqHullStatus, 0x068, 0},
    {(FARPROC)ReqCollisionGroups, 0x06C, 0},
    {(FARPROC)ReqEquipment, 0x070, 0},
    {(FARPROC)ReqAddItem, 0x078, 0},
    {(FARPROC)ReqRemoveItem, 0x07C, 0},
    {(FARPROC)ReqModifyItem, 0x080, 0},
    {(FARPROC)ReqSetCash, 0x084, 0},
    {(FARPROC)ReqChangeCash, 0x088, 0},
    {(FARPROC)BaseEnter, 0x08C, 0},
    {(FARPROC)BaseExit, 0x090, 0},
    {(FARPROC)LocationEnter, 0x094, 0},
    {(FARPROC)LocationExit, 0x098, 0},
    {(FARPROC)BaseInfoRequest, 0x09C, 0},
    {(FARPROC)LocationInfoRequest, 0x0A0, 0},
    {(FARPROC)GFObjSelect, 0x0A4, 0},
    {(FARPROC)GFGoodVaporized, 0x0A8, 0},
    {(FARPROC)MissionResponse, 0x0AC, 0},
    {(FARPROC)TradeResponse, 0x0B0, 0},
    {(FARPROC)GFGoodBuy, 0x0B4, 0},
    {(FARPROC)GFGoodSell, 0x0B8, 0},
    {(FARPROC)SystemSwitchOutComplete, 0x0BC, 0},
    {(FARPROC)PlayerLaunch, 0x0C0, 0},
    {(FARPROC)LaunchComplete, 0x0C4, 0},
    {(FARPROC)JumpInComplete, 0x0C8, 0},
    {(FARPROC)Hail, 0x0CC, 0},
    {(FARPROC)SPObjUpdate, 0x0D0, 0},
    {(FARPROC)SPMunitionCollision, 0x0D4, 0},
    {(FARPROC)SPBadLandsObjCollision, 0x0D8, 0},
    {(FARPROC)SPObjCollision, 0x0DC, 0},
    {(FARPROC)SPRequestUseItem, 0x0E0, 0},
    {(FARPROC)SPRequestInvincibility, 0x0E4, 0},
    {(FARPROC)SaveGame, 0x0E8, 0},
    {(FARPROC)MissionSaveB, 0x0EC, 0},
    {(FARPROC)RequestEvent, 0x0F0, 0},
    {(FARPROC)RequestCancel, 0x0F4, 0},
    {(FARPROC)MineAsteroid, 0x0F8, 0},
    {(FARPROC)CommComplete, 0x0FC, 0},
    {(FARPROC)RequestCreateShip, 0x100, 0},
    {(FARPROC)SPScanCargo, 0x104, 0},
    {(FARPROC)SetManeuver, 0x108, 0},
    {(FARPROC)InterfaceItemUsed, 0x10C, 0},
    {(FARPROC)AbortMission, 0x110, 0},
    {(FARPROC)RTCDone, 0x114, 0},
    {(FARPROC)SetWeaponGroup, 0x118, 0},
    {(FARPROC)SetVisitedState, 0x11C, 0},
    {(FARPROC)RequestBestPath, 0x120, 0},
    {(FARPROC)RequestPlayerStats, 0x124, 0},
    {(FARPROC)PopUpDialog, 0x128, 0},
    {(FARPROC)RequestGroupPositions, 0x12C, 0},
    {(FARPROC)SetMissionLog, 0x130, 0},
    {(FARPROC)SetInterfaceState, 0x134, 0},
    {(FARPROC)RequestRankLevel, 0x138, 0},
    {(FARPROC)InitiateTrade, 0x13C, 0},
    {(FARPROC)TerminateTrade, 0x140, 0},
    {(FARPROC)AcceptTrade, 0x144, 0},
    {(FARPROC)SetTradeMoney, 0x148, 0},
    {(FARPROC)AddTradeEquip, 0x14C, 0},
    {(FARPROC)DelTradeEquip, 0x150, 0},
    {(FARPROC)RequestTrade, 0x154, 0},
    {(FARPROC)StopTradeRequest, 0x158, 0},
    {(FARPROC)ReqDifficultyScale, 0x15C, 0},
    {(FARPROC)GetServerID, 0x160, 0},
    {(FARPROC)GetServerSig, 0x164, 0},
    {(FARPROC)DumpPacketStats, 0x168, 0},
    {(FARPROC)Dock, 0x16C, 0},
};

} // namespace HkIServerImpl
