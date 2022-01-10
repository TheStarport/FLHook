// includes
#include "FLHook.h"
#include "header.h"
#include "plugin.h"
#include <plugin_comms.h>

#define PRINT_ERROR()                                                        \
    {                                                                        \
        for (uint i = 0; (i < sizeof(wscError) / sizeof(std::wstring)); i++) \
            PrintUserCmdText(iClientID, wscError[i]);                        \
        return;                                                              \
    }
#define PRINT_OK() PrintUserCmdText(iClientID, L"OK");
#define PRINT_DISABLED() PrintUserCmdText(iClientID, L"Command disabled");

CONNECTION_DATA ConData[MAX_CLIENT_ID + 1];
bool set_bPingCmd;

ReturnCode returncode = ReturnCode::Default;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void LoadSettings() {

    set_iPingKickFrame = IniGetI(set_scCfgFile, "Kick", "PingKickFrame", 30);
    if (!set_iPingKickFrame)
        set_iPingKickFrame = 60;
    set_iPingKick = IniGetI(set_scCfgFile, "Kick", "PingKick", 0);
    set_iFluctKick = IniGetI(set_scCfgFile, "Kick", "FluctKick", 0);
    set_iLossKickFrame = IniGetI(set_scCfgFile, "Kick", "LossKickFrame", 30);
    if (!set_iLossKickFrame)
        set_iLossKickFrame = 60;
    set_iLossKick = IniGetI(set_scCfgFile, "Kick", "LossKick", 0);
    set_iLagDetectionFrame =
        IniGetI(set_scCfgFile, "Kick", "LagDetectionFrame", 50);
    set_iLagDetectionMinimum =
        IniGetI(set_scCfgFile, "Kick", "LagDetectionMinimum", 200);
    set_iLagKick = IniGetI(set_scCfgFile, "Kick", "LagKick", 0);
    set_iKickThreshold = IniGetI(set_scCfgFile, "Kick", "KickThreshold", 0);

    set_bPingCmd = IniGetB(set_scCfgFile, "UserCommands", "Ping", false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {

    if (fdwReason == DLL_PROCESS_ATTACH)
        LoadSettings();

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ClearConData(uint iClientID) {
    ConData[iClientID].iAverageLoss = 0;
    ConData[iClientID].iAveragePing = 0;
    ConData[iClientID].iLastLoss = 0;
    ConData[iClientID].iLastPacketsDropped = 0;
    ConData[iClientID].iLastPacketsReceived = 0;
    ConData[iClientID].iLastPacketsSent = 0;
    ConData[iClientID].iPingFluctuation = 0;
    ConData[iClientID].lstLoss.clear();
    ConData[iClientID].lstPing.clear();
    ConData[iClientID].lstObjUpdateIntervalls.clear();
    ConData[iClientID].iLags = 0;
    ConData[iClientID].tmLastObjUpdate = 0;
    ConData[iClientID].tmLastObjTimestamp = 0;

    ConData[iClientID].bException = false;
    ConData[iClientID].sExceptionReason = "";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void ClearClientInfo(uint iClientID) {

    ClearConData(iClientID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void UserCmd_Help(uint iClientID, const std::wstring &wscParam) {

    if (set_bPingCmd) {
        PrintUserCmdText(iClientID, L"/ping");
        PrintUserCmdText(iClientID, L"/pingtarget");
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void HkTimerCheckKick() {

    if (g_iServerLoad > set_iKickThreshold) {
        // for all players
        struct PlayerData *pPD = 0;
        while (pPD = Players.traverse_active(pPD)) {
            uint iClientID = HkGetClientIdFromPD(pPD);
            if (iClientID < 1 || iClientID > MAX_CLIENT_ID)
                continue;

            if (set_iLossKick) { // check if loss is too high
                if (ConData[iClientID].iAverageLoss > (set_iLossKick)) {
                    ConData[iClientID].lstLoss.clear();
                    HkAddKickLog(iClientID, L"High loss");
                    HkMsgAndKick(iClientID, L"High loss", set_iKickMsgPeriod);
                    // call tempban plugin
                    TEMPBAN_BAN_STRUCT tempban;
                    tempban.iClientID = iClientID;
                    tempban.iDuration = 1; // 1 minute
                    PluginCommunication(TEMPBAN_BAN, &tempban);
                }
            }

            if (set_iPingKick) { // check if ping is too high
                if (ConData[iClientID].iAveragePing > (set_iPingKick)) {
                    ConData[iClientID].lstPing.clear();
                    HkAddKickLog(iClientID, L"High ping");
                    HkMsgAndKick(iClientID, L"High ping", set_iKickMsgPeriod);
                    // call tempban plugin
                    TEMPBAN_BAN_STRUCT tempban;
                    tempban.iClientID = iClientID;
                    tempban.iDuration = 1; // 1 minute
                    PluginCommunication(TEMPBAN_BAN, &tempban);
                }
            }

            if (set_iFluctKick) { // check if ping fluct is too high
                if (ConData[iClientID].iPingFluctuation > (set_iFluctKick)) {
                    ConData[iClientID].lstPing.clear();
                    HkAddKickLog(iClientID, L"High fluct");
                    HkMsgAndKick(iClientID, L"High ping fluctuation",
                                 set_iKickMsgPeriod);
                    // call tempban plugin
                    TEMPBAN_BAN_STRUCT tempban;
                    tempban.iClientID = iClientID;
                    tempban.iDuration = 1; // 1 minute
                    PluginCommunication(TEMPBAN_BAN, &tempban);
                }
            }

            if (set_iLagKick) { // check if lag is too high
                if (ConData[iClientID].iLags > (set_iLagKick)) {
                    ConData[iClientID].lstObjUpdateIntervalls.clear();

                    HkAddKickLog(iClientID, L"High Lag");
                    HkMsgAndKick(iClientID, L"High Lag", set_iKickMsgPeriod);
                    // call tempban plugin
                    TEMPBAN_BAN_STRUCT tempban;
                    tempban.iClientID = iClientID;
                    tempban.iDuration = 1; // 1 minute
                    PluginCommunication(TEMPBAN_BAN, &tempban);
                }
            }
        }
    }

    // Are there accounts connected with client IDs greater than max player
    // count? If so, kick them as FLServer is buggy and will use high client IDs
    // but will not allow character selection on them.
    for (int iClientID = Players.GetMaxPlayerCount() + 1;
         iClientID <= MAX_CLIENT_ID; iClientID++) {
        if (Players[iClientID].iOnlineID) {
            CAccount *acc = Players.FindAccountFromClientID(iClientID);
            if (acc) {
                // Console::ConPrint(L"Kicking lag bug account iClientID=%u %u",
                // iClientID,Players[iClientID].iOnlineID);
                acc->ForceLogout();
                Players.logout(iClientID);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**************************************************************************************************************
Update average ping data
**************************************************************************************************************/

void TimerUpdatePingData() {

    // for all players
    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
        uint iClientID = HkGetClientIdFromPD(pPD);
        if (iClientID < 1 || iClientID > MAX_CLIENT_ID)
            continue;

        if (ClientInfo[iClientID].tmF1TimeDisconnect)
            continue;

        DPN_CONNECTION_INFO ci;
        if (HkGetConnectionStats(iClientID, ci) != HKE_OK)
            continue;

        ///////////////////////////////////////////////////////////////
        // update ping data
        if (ConData[iClientID].lstPing.size() >= set_iPingKickFrame) {
            // calculate average ping and ping fluctuation
            unsigned int iLastPing = 0;
            ConData[iClientID].iAveragePing = 0;
            ConData[iClientID].iPingFluctuation = 0;
            for (auto &ping : ConData[iClientID].lstPing) {
                ConData[iClientID].iAveragePing += ping;
                if (iLastPing != 0) {
                    ConData[iClientID].iPingFluctuation += (uint)sqrt(
                        (double)pow(((float)ping - (float)iLastPing), 2));
                }
                iLastPing = ping;
            }

            ConData[iClientID].iPingFluctuation /=
                (uint)ConData[iClientID].lstPing.size();
            ConData[iClientID].iAveragePing /=
                (uint)ConData[iClientID].lstPing.size();
        }

        // remove old pingdata
        while (ConData[iClientID].lstPing.size() >= set_iPingKickFrame)
            ConData[iClientID].lstPing.pop_back();

        ConData[iClientID].lstPing.push_front(ci.dwRoundTripLatencyMS);
    }
}

/**************************************************************************************************************
Update average loss data
**************************************************************************************************************/

void TimerUpdateLossData() {

    // for all players
    float fLossPercentage;
    uint iNewDrops;
    uint iNewSent;
    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
        uint iClientID = HkGetClientIdFromPD(pPD);
        if (iClientID < 1 || iClientID > MAX_CLIENT_ID)
            continue;

        if (ClientInfo[iClientID].tmF1TimeDisconnect)
            continue;

        DPN_CONNECTION_INFO ci;
        if (HkGetConnectionStats(iClientID, ci) != HKE_OK)
            continue;

        ///////////////////////////////////////////////////////////////
        // update loss data
        if (ConData[iClientID].lstLoss.size() >=
            (set_iLossKickFrame / (LOSS_INTERVALL / 1000))) {
            // calculate average loss
            ConData[iClientID].iAverageLoss = 0;
            for (auto &loss : ConData[iClientID].lstLoss)
                ConData[iClientID].iAverageLoss += loss;

            ConData[iClientID].iAverageLoss /=
                (uint)ConData[iClientID].lstLoss.size();
        }

        // remove old lossdata
        while (ConData[iClientID].lstLoss.size() >=
               (set_iLossKickFrame / (LOSS_INTERVALL / 1000)))
            ConData[iClientID].lstLoss.pop_back();

        // sum of Drops = Drops guaranteed + drops non-guaranteed
        iNewDrops = (ci.dwPacketsRetried + ci.dwPacketsDropped) -
                    ConData[iClientID].iLastPacketsDropped;

        iNewSent =
            (ci.dwPacketsSentGuaranteed + ci.dwPacketsSentNonGuaranteed) -
            ConData[iClientID].iLastPacketsSent;

        // % of Packets Lost = Drops / (sent+received) * 100
        if (iNewSent > 0) // division by zero check
            fLossPercentage = (float)((float)iNewDrops / (float)iNewSent) * 100;
        else
            fLossPercentage = 0.0;

        if (fLossPercentage > 100)
            fLossPercentage = 100;

        // add last loss to List lstLoss and put current value into iLastLoss
        ConData[iClientID].lstLoss.push_front(ConData[iClientID].iLastLoss);
        ConData[iClientID].iLastLoss = (uint)fLossPercentage;

        // Fill new ClientInfo-variables with current values
        ConData[iClientID].iLastPacketsSent =
            ci.dwPacketsSentGuaranteed + ci.dwPacketsSentNonGuaranteed;
        ConData[iClientID].iLastPacketsDropped =
            ci.dwPacketsRetried + ci.dwPacketsDropped;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace HkIServerImpl {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// add timers here
typedef void (*_TimerFunc)();

struct TIMER {
    _TimerFunc proc;
    mstime tmIntervallMS;
    mstime tmLastCall;
};

TIMER Timers[] = {
    {TimerUpdatePingData, 1000, 0},
    {TimerUpdateLossData, LOSS_INTERVALL, 0},
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT int __stdcall Update() {

    static bool bFirstTime = true;
    if (bFirstTime) {
        bFirstTime = false;
        // check for logged in players and reset their connection data
        struct PlayerData *pPD = 0;
        while (pPD = Players.traverse_active(pPD)) {
            uint iClientID = pPD->iOnlineID;
            if (iClientID < 1 || iClientID > MAX_CLIENT_ID)
                continue;

            ClearConData(HkGetClientIdFromPD(pPD));
        }
    }

    // call timers
    for (uint i = 0; (i < sizeof(Timers) / sizeof(TIMER)); i++) {
        if ((timeInMS() - Timers[i].tmLastCall) >= Timers[i].tmIntervallMS) {
            Timers[i].tmLastCall = timeInMS();
            Timers[i].proc();
        }
    }

    return 0; // it doesnt matter what we return here since we have set the
              // return code to "DEFAULT_RETURNCODE", so FLHook will just ignore
              // it
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void __stdcall PlayerLaunch(unsigned int iShip, unsigned int iClientID) {

    ConData[iClientID].tmLastObjUpdate = 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void __stdcall SPObjUpdate(struct SSPObjUpdateInfo const &ui,
                                  unsigned int iClientID) {
    // lag detection
    IObjInspectImpl *ins = HkGetInspect(iClientID);
    if (!ins)
        return; // ??? 8[

    mstime tmNow = timeInMS();
    mstime tmTimestamp = (mstime)(ui.fTimestamp * 1000);

    if (set_iLagDetectionFrame && ConData[iClientID].tmLastObjUpdate &&
        (HkGetEngineState(iClientID) != ES_TRADELANE) && (ui.cState != 7)) {
        uint iTimeDiff = (uint)(tmNow - ConData[iClientID].tmLastObjUpdate);
        uint iTimestampDiff =
            (uint)(tmTimestamp - ConData[iClientID].tmLastObjTimestamp);
        int iDiff = (int)sqrt(
            pow((long double)((int)iTimeDiff - (int)iTimestampDiff), 2));
        iDiff -= g_iServerLoad;
        if (iDiff < 0)
            iDiff = 0;

        uint iPerc;
        if (iTimestampDiff != 0)
            iPerc =
                (uint)((float)((float)iDiff / (float)iTimestampDiff) * 100.0);
        else
            iPerc = 0;

        if (ConData[iClientID].lstObjUpdateIntervalls.size() >=
            set_iLagDetectionFrame) {
            uint iLags = 0;
            for (auto &iv : ConData[iClientID].lstObjUpdateIntervalls) {
                if (iv > set_iLagDetectionMinimum)
                    iLags++;
            }

            ConData[iClientID].iLags = (iLags * 100) / set_iLagDetectionFrame;
            while (ConData[iClientID].lstObjUpdateIntervalls.size() >=
                   set_iLagDetectionFrame)
                ConData[iClientID].lstObjUpdateIntervalls.pop_front();
        }

        ConData[iClientID].lstObjUpdateIntervalls.push_back(iPerc);
    }

    ConData[iClientID].tmLastObjUpdate = tmNow;
    ConData[iClientID].tmLastObjTimestamp = tmTimestamp;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace HkIServerImpl

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Ping(uint iClientID, const std::wstring &wscParam) {
    if (!set_bPingCmd) {
        PRINT_DISABLED();
        return;
    }

    std::wstring wscTargetPlayer = GetParam(wscParam, ' ', 0);

    uint iClientIDTarget;
    iClientIDTarget = iClientID;

    std::wstring Response;

    Response += L"Ping: ";
    if (ConData[iClientIDTarget].lstPing.size() < set_iPingKickFrame)
        Response += L"n/a Fluct: n/a ";
    else {
        Response +=
            std::to_wstring(ConData[iClientIDTarget].iAveragePing).c_str();
        Response += L"ms ";
        if (set_iPingKick > 0) {
            Response += L"(Max: ";
            Response += std::to_wstring(set_iPingKick).c_str();
            Response += L"ms) ";
        }
        Response += L"Fluct: ";
        Response +=
            std::to_wstring(ConData[iClientIDTarget].iPingFluctuation).c_str();
        Response += L"ms ";
        if (set_iFluctKick > 0) {
            Response += L"(Max: ";
            Response += std::to_wstring(set_iFluctKick).c_str();
            Response += L"ms) ";
        }
    }

    Response += L"Loss: ";
    if (ConData[iClientIDTarget].lstLoss.size() <
        (set_iLossKickFrame / (LOSS_INTERVALL / 1000)))
        Response += L"n/a ";
    else {
        Response +=
            std::to_wstring(ConData[iClientIDTarget].iAverageLoss).c_str();
        Response += L"%% ";
        if (set_iLossKick > 0) {
            Response += L"(Max: ";
            Response += std::to_wstring(set_iLossKick).c_str();
            Response += L"%%) ";
        }
    }

    Response += L"Lag: ";
    if (ConData[iClientIDTarget].lstObjUpdateIntervalls.size() <
        set_iLagDetectionFrame)
        Response += L"n/a";
    else {
        Response += std::to_wstring(ConData[iClientIDTarget].iLags).c_str();
        Response += L"%% ";
        if (set_iLagKick > 0) {
            Response += L"(Max: ";
            Response += std::to_wstring(set_iLagKick).c_str();
            Response += L"%%)";
        }
    }

    // Send the message to the user
    PrintUserCmdText(iClientID, Response);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_PingTarget(uint iClientID, const std::wstring &wscParam) {
    if (!set_bPingCmd) {
        PRINT_DISABLED();
        return;
    }

    uint iShip = 0;
    pub::Player::GetShip(iClientID, iShip);
    if (!iShip) {
        PrintUserCmdText(iClientID, L"Error: You are docked");
        return;
    }

    uint iTarget = 0;
    pub::SpaceObj::GetTarget(iShip, iTarget);

    if (!iTarget) {
        PrintUserCmdText(iClientID, L"Error: No target");
        return;
    }

    uint iClientIDTarget = HkGetClientIDByShip(iTarget);
    if (!HkIsValidClientID(iClientIDTarget)) {
        PrintUserCmdText(iClientID, L"Error: Target is no player");
        return;
    }

    std::wstring Response;

    if (iClientIDTarget != iClientID) {
        std::wstring wscCharname =
            (const wchar_t *)Players.GetActiveCharacterName(iClientIDTarget);
        Response += wscCharname.c_str();
        Response += L" - ";
    }

    Response += L"Ping: ";
    if (ConData[iClientIDTarget].lstPing.size() < set_iPingKickFrame)
        Response += L"n/a Fluct: n/a ";
    else {
        Response +=
            std::to_wstring(ConData[iClientIDTarget].iAveragePing).c_str();
        Response += L"ms ";
        if (set_iPingKick > 0) {
            Response += L"(Max: ";
            Response += std::to_wstring(set_iPingKick).c_str();
            Response += L"ms) ";
        }
        Response += L"Fluct: ";
        Response +=
            std::to_wstring(ConData[iClientIDTarget].iPingFluctuation).c_str();
        Response += L"ms ";
        if (set_iFluctKick > 0) {
            Response += L"(Max: ";
            Response += std::to_wstring(set_iFluctKick).c_str();
            Response += L"ms) ";
        }
    }

    Response += L"Loss: ";
    if (ConData[iClientIDTarget].lstLoss.size() <
        (set_iLossKickFrame / (LOSS_INTERVALL / 1000)))
        Response += L"n/a ";
    else {
        Response +=
            std::to_wstring(ConData[iClientIDTarget].iAverageLoss).c_str();
        Response += L"%% ";
        if (set_iLossKick > 0) {
            Response += L"(Max: ";
            Response += std::to_wstring(set_iLossKick).c_str();
            Response += L"%%) ";
        }
    }

    Response += L"Lag: ";
    if (ConData[iClientIDTarget].lstObjUpdateIntervalls.size() <
        set_iLagDetectionFrame)
        Response += L"n/a";
    else {
        Response += std::to_wstring(ConData[iClientIDTarget].iLags).c_str();
        Response += L"%% ";
        if (set_iLagKick > 0) {
            Response += L"(Max: ";
            Response += std::to_wstring(set_iLagKick).c_str();
            Response += L"%%)";
        }
    }

    // Send the message to the user
    PrintUserCmdText(iClientID, Response);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

USERCMD UserCmds[] = {
    {L"/ping", UserCmd_Ping},
    {L"/pingtarget", UserCmd_PingTarget},
};

// Process user input
bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void PluginCommunicationCallBack(PLUGIN_MESSAGE msg, void *data) {

    // this is the hooked plugin communication function

    // we now check, if the message is for us
    if (msg == CONDATA_EXCEPTION) {
        // the message is for us, now we know what the actual data is, so we do
        // a reinterpret cast
        CONDATA_EXCEPTION_STRUCT *incoming_data =
            reinterpret_cast<CONDATA_EXCEPTION_STRUCT *>(data);

        ConData[incoming_data->iClientID].bException =
            incoming_data->bException;
        ConData[incoming_data->iClientID].sExceptionReason =
            incoming_data->sReason;
        if (!ConData[incoming_data->iClientID].bException)
            ClearConData(incoming_data->iClientID);

    } else if (msg == CONDATA_DATA) {
        CONDATA_DATA_STRUCT *incoming_data =
            reinterpret_cast<CONDATA_DATA_STRUCT *>(data);
        incoming_data->iAverageLoss =
            ConData[incoming_data->iClientID].iAverageLoss;
        incoming_data->iAveragePing =
            ConData[incoming_data->iClientID].iAveragePing;
        incoming_data->iLags = ConData[incoming_data->iClientID].iLags;
        incoming_data->iPingFluctuation =
            ConData[incoming_data->iClientID].iPingFluctuation;
    }
    return;
}

EXPORT bool ExecuteCommandString(CCmds *classptr, const std::wstring &wscCmd) {

    if (IS_CMD("getstats")) {
        struct PlayerData *pPD = 0;
        while (pPD = Players.traverse_active(pPD)) {
            uint iClientID = HkGetClientIdFromPD(pPD);
            if (HkIsInCharSelectMenu(iClientID))
                continue;

            CDPClientProxy *cdpClient = g_cClientProxyArray[iClientID - 1];
            if (!cdpClient)
                continue;

            int saturation = (int)(cdpClient->GetLinkSaturation() * 100);
            int txqueue = cdpClient->GetSendQSize();
            classptr->Print(
                L"charname=%s clientid=%u loss=%u lag=%u pingfluct=%u "
                L"saturation=%u txqueue=%u\n",
                Players.GetActiveCharacterName(iClientID), iClientID,
                ConData[iClientID].iAverageLoss, ConData[iClientID].iLags,
                ConData[iClientID].iPingFluctuation, saturation, txqueue);
        }
        classptr->Print(L"OK");
        returncode = ReturnCode::SkipAll;
        return true;
    } else if (IS_CMD("kick")) {
        // Find by charname. If this fails, fall through to default behaviour.
        CAccount *acc = HkGetAccountByCharname(classptr->ArgCharname(1));
        if (!acc)
            return false;

        // Logout.
        returncode = ReturnCode::SkipAll;
        acc->ForceLogout();
        classptr->Print(L"OK");

        // If the client is still active then force the disconnect.
        uint iClientID = HkGetClientIdFromAccount(acc);
        if (iClientID != -1) {
            classptr->Print(L"Forcing logout on iClientID=%d", iClientID);
            Players.logout(iClientID);
        }
        return true;
    }

    return false;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Advanced Connection Data Plugin by w0dk4");
    pi->shortName("condata");
    pi->mayPause(false);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
    pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &HkTimerCheckKick);
    pi->emplaceHook(HookedCall::IServerImpl__Update, &HkIServerImpl::Update);
    pi->emplaceHook(HookedCall::IServerImpl__SPObjUpdate, &HkIServerImpl::SPObjUpdate);
    pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &HkIServerImpl::PlayerLaunch);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
    pi->emplaceHook(HookedCall::FLHook__PluginCommunication, &PluginCommunicationCallBack);
    pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
}