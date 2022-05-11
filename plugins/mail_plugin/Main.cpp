// Mail Plugin - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.
//

#include <FLHook.h>
#include <plugin.h>

#include <utility>

#include "Mail.h"

ReturnCode returncode = ReturnCode::Default;

/** The messaging plugin message log for offline players */
static std::string MSG_LOG = "-mail.ini";
static const int MAX_MAIL_MSGS = 40;

MailCommunicator *communicator;

/** Return the number of messages. */
int MailCount(const std::wstring &wscCharname, const std::string &scExtension) {
    // Get the target player's message file.
    std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
    if (scFilePath.length() == 0)
        return 0;

    int iMsgs = 0;
    for (int iMsgSlot = 1; iMsgSlot < MAX_MAIL_MSGS; iMsgSlot++) {
        if (IniGetS(scFilePath, "Msgs", std::to_string(iMsgSlot), "")
                .length() == 0)
            break;
        iMsgs++;
    }
    return iMsgs;
}

/** Show five messages from the specified starting position. */
void MailShow(const std::wstring &wscCharname, const std::string &scExtension,
              int iFirstMsg) {
    // Make sure the character is logged in.
    uint iClientID = HkGetClientIdFromCharname(wscCharname);
    if (iClientID == -1)
        return;

    // Get the target player's message file.
    std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
    if (scFilePath.length() == 0)
        return;

    int iLastMsg = iFirstMsg;
    for (int iMsgSlot = iFirstMsg, iMsgCount = 0;
         iMsgSlot < MAX_MAIL_MSGS && iMsgCount < 5; iMsgSlot++, iMsgCount++) {
        std::wstring wscTmpMsg =
            IniGetWS(scFilePath, "Msgs", std::to_string(iMsgSlot), L"");
        if (wscTmpMsg.length() == 0)
            break;
        PrintUserCmdText(iClientID, L"#%02d %s", iMsgSlot, wscTmpMsg.c_str());
        IniWrite(scFilePath, "MsgsRead", std::to_string(iMsgSlot), "yes");
        iLastMsg = iMsgSlot;
    }
    PrintUserCmdText(iClientID, L"Viewing #%02d-#%02d of %02d messages",
                     iFirstMsg, iLastMsg, MailCount(wscCharname, scExtension));
}

/** Return the number of unread messages. */
int MailCountUnread(const std::wstring &wscCharname,
                    const std::string &scExtension) {
    // Get the target player's message file.
    std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
    if (scFilePath.length() == 0)
        return 0;

    int iUnreadMsgs = 0;
    for (int iMsgSlot = 1; iMsgSlot < MAX_MAIL_MSGS; iMsgSlot++) {
        if (IniGetS(scFilePath, "Msgs", std::to_string(iMsgSlot), "")
                .length() == 0)
            break;
        if (!IniGetB(scFilePath, "MsgsRead", std::to_string(iMsgSlot), false))
            iUnreadMsgs++;
    }
    return iUnreadMsgs;
}

/** Check for new or unread messages. */
void MailCheckLog(const std::wstring &wscCharname,
                  const std::string &scExtension) {
    // Make sure the character is logged in.
    uint iClientID = HkGetClientIdFromCharname(wscCharname);
    if (iClientID == -1)
        return;

    // Get the target player's message file.
    std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
    if (scFilePath.length() == 0)
        return;

    // If there are unread messaging then inform the player
    int iUnreadMsgs = MailCountUnread(wscCharname, scExtension);
    if (iUnreadMsgs > 0) {
        PrintUserCmdText(
            iClientID,
            L"You have %d unread messages. Type /mail to see your messages",
            iUnreadMsgs);
    }
}

/**
 Save a msg to disk so that we can inform the receiving character
 when they log in.
*/
bool __stdcall MailSend(const std::wstring &wscCharname, const std::string &scExtension,
              const std::wstring &wscMsg) {
    // Get the target player's message file.
    std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
    if (scFilePath.length() == 0)
        return false;

    // Move all mail up one slot starting at the end. We automatically
    // discard the oldest messages.
    for (int iMsgSlot = MAX_MAIL_MSGS - 1; iMsgSlot > 0; iMsgSlot--) {
        std::wstring wscTmpMsg =
            IniGetWS(scFilePath, "Msgs", std::to_string(iMsgSlot), L"");
        IniWriteW(scFilePath, "Msgs", std::to_string(iMsgSlot + 1), wscTmpMsg);

        bool bTmpRead =
            IniGetB(scFilePath, "MsgsRead", std::to_string(iMsgSlot), false);
        IniWrite(scFilePath, "MsgsRead", std::to_string(iMsgSlot + 1),
                 (bTmpRead ? "yes" : "no"));
    }

    // Write message into the slot
    IniWriteW(scFilePath, "Msgs", "1",
              GetTimeString(set_bLocalTime) + L" " + wscMsg);
    IniWrite(scFilePath, "MsgsRead", "1", "no");

    MailCommunicator::MailSent mail { wscCharname, wscMsg };
    communicator->Dispatch(static_cast<int>(MailCommunicator::MailEvent::MailSent), &mail);

    // Call the CheckLog function to prompt the target to check their mail if they are logged in
    MailCheckLog(wscCharname,MSG_LOG);
    return true;
}

/**
Delete a message
*/
bool MailDel(const std::wstring &wscCharname, const std::string &scExtension,
             int iMsg) {
    // Get the target player's message file.
    std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
    if (scFilePath.length() == 0)
        return false;

    // Move all mail down one slot starting at the deleted message to overwrite
    // it
    for (int iMsgSlot = iMsg; iMsgSlot < MAX_MAIL_MSGS; iMsgSlot++) {
        std::wstring wscTmpMsg =
            IniGetWS(scFilePath, "Msgs", std::to_string(iMsgSlot + 1), L"");
        IniWriteW(scFilePath, "Msgs", std::to_string(iMsgSlot), wscTmpMsg);

        bool bTmpRead = IniGetB(scFilePath, "MsgsRead",
                                std::to_string(iMsgSlot + 1), false);
        IniWrite(scFilePath, "MsgsRead", std::to_string(iMsgSlot),
                 (bTmpRead ? "yes" : "no"));
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hooks
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PlayerLaunch(uint& iShip, uint& iClientID) {
    MailCheckLog((const wchar_t *)Players.GetActiveCharacterName(iClientID), MSG_LOG);
}
    
/// On base entry events and reload the msg cache for the client.
void BaseEnter(uint& iBaseID, uint& iClientID) {
    MailCheckLog((const wchar_t *)Players.GetActiveCharacterName(iClientID), MSG_LOG);
}

MailCommunicator::MailCommunicator(const std::string &plugin) : PluginCommunicator(plugin) {  // NOLINT(clang-diagnostic-shadow-field)
    this->SendMail = [](std::wstring character, std::wstring msg) { MailSend(character, MSG_LOG, msg); };
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Show Mail */
void UserCmd_MailShow(uint iClientID, const std::wstring &wscParam) {
    int iNumberUnreadMsgs = MailCountUnread(
        (const wchar_t *)Players.GetActiveCharacterName(iClientID), MSG_LOG);
    int iNumberMsgs = MailCount(
        (const wchar_t *)Players.GetActiveCharacterName(iClientID), MSG_LOG);
    if (iNumberMsgs == 0) {
        PrintUserCmdText(iClientID, L"OK You have no messages");
        return;
    }

    int iFirstMsg = ToInt(ToLower(GetParam(wscParam, ' ', 0)));
    if (iFirstMsg == 0) {
        if (iNumberUnreadMsgs > 0)
            PrintUserCmdText(iClientID, L"OK You have %d unread messages",
                             iNumberUnreadMsgs);
        else
            PrintUserCmdText(iClientID, L"OK You have %d messages",
                             iNumberMsgs);
        PrintUserCmdText(iClientID,
                         L"Type /mail 1 to see first message or /mail "
                         L"<num> to see specified message");
        return;
    }

    if (iFirstMsg > iNumberMsgs) {
        PrintUserCmdText(iClientID, L"ERR Message does not exist");
        return;
    }

    MailShow((const wchar_t *)Players.GetActiveCharacterName(iClientID),
                   MSG_LOG, iFirstMsg);
}

/** Delete Mail */
void UserCmd_MailDel(uint iClientID, const std::wstring &wscParam) {
    if (wscParam.size() == 0) {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        PrintUserCmdText(iClientID, L"Usage: /maildel <msgnum>");
        return;
    }

    int iNumberMsgs = MailCount(
        (const wchar_t *)Players.GetActiveCharacterName(iClientID), MSG_LOG);
    int iMsg = ToInt(ToLower(GetParam(wscParam, ' ', 0)));
    if (iMsg == 0 || iMsg > iNumberMsgs) {
        PrintUserCmdText(iClientID, L"ERR Message does not exist");
        return;
    }

    if (MailDel(
            (const wchar_t *)Players.GetActiveCharacterName(iClientID), MSG_LOG,
            iMsg))
        PrintUserCmdText(iClientID, L"OK");
    else
        PrintUserCmdText(iClientID, L"ERR");
}

// Client command processing
USERCMD UserCmds[] = {
    {L"/mail", UserCmd_MailShow},
    {L"/maildel", UserCmd_MailDel},
};

// Process user input
bool UserCmd_Process(uint& iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

// Hook on /help
EXPORT void UserCmd_Help(uint& iClientID, const std::wstring &wscParam) {
    PrintUserCmdText(iClientID, L"/mail");
    PrintUserCmdText(iClientID, L"/maildel");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Admin commands
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AdminCmd_SendMail(CCmds *cmds, const std::wstring &wscCharname,
                       const std::wstring &wscMsg) {
    MailSend(wscCharname, MSG_LOG, cmds->GetAdminName() + L": " + wscMsg);
    cmds->Print(L"OK message saved to mailbox");
}

bool ExecuteCommandString(CCmds *cmds, const std::wstring &wscCmd) {

    if (IS_CMD("move")) {
        returncode = ReturnCode::SkipAll;
        AdminCmd_SendMail(cmds, cmds->ArgStr(0), cmds->ArgStrToEnd(1));
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
    pi->name(MailCommunicator::pluginName);
    pi->shortName("mail");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
    pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
    pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
    pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);

    // Register plugin for IPC
    communicator = new MailCommunicator(MailCommunicator::pluginName);
    PluginCommunicator::ExportPluginCommunicator(communicator);
}