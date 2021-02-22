#include "Main.h"

std::list<TEMPBAN_INFO> lstTempBans;

ReturnCode returncode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    // clear tempban list
    lstTempBans.clear();

    return true;
}

/**************************************************************************************************************
Check if TempBans exceeded
**************************************************************************************************************/

EXPORT void HkTimerCheckKick() {
    // timed out tempbans get deleted here

    returncode = ReturnCode::Default;

    for (auto it = lstTempBans.begin(); it != lstTempBans.end(); ++it) {
        if (((*it).banstart + (*it).banduration) < timeInMS()) {
            lstTempBans.erase(it);
            break; // fix to not overflow the list
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkTempBan(const std::wstring &wscCharname, uint _duration) {

    HK_GET_CLIENTID(iClientID, wscCharname);

    mstime duration = 1000 * _duration * 60;
    TEMPBAN_INFO tempban;
    tempban.banstart = timeInMS();
    tempban.banduration = duration;

    CAccount *acc;
    if (iClientID != -1)
        acc = Players.FindAccountFromClientID(iClientID);
    else {
        if (!(acc = HkGetAccountByCharname(wscCharname)))
            return HKE_CHAR_DOES_NOT_EXIST;
    }
    std::wstring wscID = HkGetAccountID(acc);

    tempban.wscID = wscID;
    lstTempBans.push_back(tempban);

    return HKE_OK;
}

bool HkTempBannedCheck(uint iClientID) {
    CAccount *acc;
    acc = Players.FindAccountFromClientID(iClientID);

    std::wstring wscID = HkGetAccountID(acc);

    for (auto &ban : lstTempBans) {
        if (ban.wscID == wscID)
            return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace HkIServerImpl {

EXPORT void __stdcall Login(struct SLoginInfo const &li,
                            unsigned int iClientID) {
    returncode = ReturnCode::Default;

    // check for tempban
    if (HkTempBannedCheck(iClientID)) {
        returncode = ReturnCode::SkipAll;
        HkKick(ARG_CLIENTID(iClientID));
    }
}

} // namespace HkIServerImpl

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void PluginCommunication(PLUGIN_MESSAGE msg, void *data) {
    if (msg == TEMPBAN_BAN) {
        auto *incoming_data = static_cast<TEMPBAN_BAN_STRUCT *>(data);

        // do something here with the received data & instruction
        HkTempBan(ARG_CLIENTID(incoming_data->iClientID),
                  incoming_data->iDuration);
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CmdTempBan(CCmds *classptr, const std::wstring &wscCharname,
                uint iDuration) {

    // right check
    if (!(classptr->rights & RIGHT_KICKBAN)) {
        classptr->Print(L"ERR No permission\n");
        return;
    }

    if (((classptr->hkLastErr = HkTempBan(wscCharname, iDuration)) ==
         HKE_OK)) // hksuccess
        classptr->Print(L"OK\n");
    else
        classptr->PrintError();
}

EXPORT bool ExecuteCommandString(CCmds *classptr,
                                          const std::wstring &wscCmd) {
    returncode = ReturnCode::SkipFunctionCall;

    if (IS_CMD("tempban")) {

        returncode = ReturnCode::SkipAll; // do not let other plugins kick in
                                          // since we now handle the command

        CmdTempBan(classptr, classptr->ArgCharname(1), classptr->ArgInt(2));

        return true;
    }

    return false;
}

EXPORT void CmdHelp(CCmds *classptr) {
    classptr->Print(L"tempban <charname>\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi) {
    pi->name("TempBan Plugin by w0dk4");
    pi->shortName("tempban");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
    pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &HkTimerCheckKick);
    pi->emplaceHook(HookedCall::IServerImpl__Login, &HkIServerImpl::Login,
                 HookStep::After);
    pi->emplaceHook(HookedCall::FLHook__PluginCommunication, &PluginCommunication);
    pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
    pi->emplaceHook(HookedCall::FLHook__AdminCommand__Help, &CmdHelp);
}
