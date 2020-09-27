#include "Main.h"

#include <plugin_comms.h>
#include <stdio.h>
#include <windows.h>

std::list<TEMPBAN_INFO> lstTempBans;

PLUGIN_RETURNCODE returncode;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT PLUGIN_RETURNCODE Get_PluginReturnCode() { return returncode; }

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

  returncode = DEFAULT_RETURNCODE;

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
  returncode = DEFAULT_RETURNCODE;

  // check for tempban
  if (HkTempBannedCheck(iClientID)) {
    returncode = SKIPPLUGINS_NOFUNCTIONCALL;
    HkKick(ARG_CLIENTID(iClientID));
  }
}

} // namespace HkIServerImpl

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT void Plugin_Communication_CallBack(PLUGIN_MESSAGE msg, void *data) {
  returncode = DEFAULT_RETURNCODE; // we do not skip other plugins even if we
                                   // handle the msg, maybe another plugin wants
                                   // to get notice of this message
  // for example a logging plugin that would log every tempban, etc..

  // this is the hooked plugin communication function

  // we now check, if the message is for us
  if (msg == TEMPBAN_BAN) {
    // the message is for us, now we know what the actual data is, so we do a
    // reinterpret cast
    TEMPBAN_BAN_STRUCT *incoming_data =
        reinterpret_cast<TEMPBAN_BAN_STRUCT *>(data);

    // do something here with the received data & instruction
    HkTempBan(ARG_CLIENTID(incoming_data->iClientID), incoming_data->iDuration);
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

#define IS_CMD(a) !wscCmd.compare(L##a)

EXPORT bool ExecuteCommandString_Callback(CCmds *classptr,
                                          const std::wstring &wscCmd) {
  returncode = NOFUNCTIONCALL; // flhook needs to care about our return code

  if (IS_CMD("tempban")) {

    returncode = SKIPPLUGINS_NOFUNCTIONCALL; // do not let other plugins kick in
                                             // since we now handle the command

    CmdTempBan(classptr, classptr->ArgCharname(1), classptr->ArgInt(2));

    return true;
  }

  return false;
}

EXPORT void CmdHelp_Callback(CCmds *classptr) {
  returncode = DEFAULT_RETURNCODE;

  classptr->Print(L"tempban <charname>\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT PLUGIN_INFO *Get_PluginInfo() {
  PLUGIN_INFO *p_PI = new PLUGIN_INFO();
  p_PI->sName = "TempBan Plugin by w0dk4";
  p_PI->sShortName = "tempban";
  p_PI->bMayPause = true;
  p_PI->bMayUnload = true;
  p_PI->ePluginReturnCode = &returncode;
  p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&HkTimerCheckKick,
                                           PLUGIN_HkTimerCheckKick, 0));
  p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&HkIServerImpl::Login,
                                           PLUGIN_HkIServerImpl_Login, 0));
  p_PI->lstHooks.push_back(
      PLUGIN_HOOKINFO((FARPROC *)&Plugin_Communication_CallBack,
                      PLUGIN_Plugin_Communication, 0));
  p_PI->lstHooks.push_back(
      PLUGIN_HOOKINFO((FARPROC *)&ExecuteCommandString_Callback,
                      PLUGIN_ExecuteCommandString_Callback, 0));
  p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&CmdHelp_Callback,
                                           PLUGIN_CmdHelp_Callback, 0));
  return p_PI;
}
