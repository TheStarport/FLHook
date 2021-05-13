// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include <float.h>
#include <list>
#include <math.h>
#include <set>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

#include <FLHook.h>
#include <plugin.h>
#include <plugin_comms.h>

#define POPUPDIALOG_BUTTONS_LEFT_YES 1
#define POPUPDIALOG_BUTTONS_CENTER_NO 2
#define POPUPDIALOG_BUTTONS_RIGHT_LATER 4
#define POPUPDIALOG_BUTTONS_CENTER_OK 8

//
#include "Main.h"

namespace PlayerInfo {
#define RSRCID_PLAYERINFO_TITLE 500000
#define RSRCID_PLAYERINFO_TEXT RSRCID_PLAYERINFO_TITLE + 1
#define MAX_PARAGRAPHS 5
#define MAX_CHARACTERS 1000

static std::wstring IniGetLongWS(const std::string &scFile,
                                 const std::string &scApp,
                                 const std::string &scKey,
                                 const std::wstring &wscDefault) {
    char szRet[0x10000];
    GetPrivateProfileString(scApp.c_str(), scKey.c_str(), "", szRet,
                            sizeof(szRet), scFile.c_str());
    std::string scValue = szRet;
    if (!scValue.length())
        return wscDefault;

    std::wstring wscValue = L"";
    long lHiByte;
    long lLoByte;
    while (sscanf_s(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2) {
        scValue = scValue.substr(4);
        wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
        wscValue.append(1, wChar);
    }

    return wscValue;
}

bool PlayerInfo::UserCmd_ShowInfo(uint iClientID, const std::wstring &wscCmd,
                                  const std::wstring &wscParam,
                                  const wchar_t *usage) {
    const wchar_t *wszTargetName = 0;
    const std::wstring &wscCommand = GetParam(wscParam, ' ', 0);
    if (wscCommand == L"me") {
        wszTargetName =
            (const wchar_t *)Players.GetActiveCharacterName(iClientID);
    } else {
        uint iShip;
        pub::Player::GetShip(iClientID, iShip);

        uint iTargetShip;
        pub::SpaceObj::GetTarget(iShip, iTargetShip);

        uint iTargetClientID = HkGetClientIDByShip(iTargetShip);
        if (HkIsValidClientID(iTargetClientID))
            wszTargetName = (const wchar_t *)Players.GetActiveCharacterName(
                iTargetClientID);
    }

    if (!wszTargetName) {
        PrintUserCmdText(iClientID, L"ERR No target");
        return true;
    }

    std::string scFilePath = GetUserFilePath(wszTargetName, "-info.ini");
    std::wstring wscPlayerInfo = L"<RDL><PUSH/>";
    for (int i = 1; i <= MAX_PARAGRAPHS; i++) {
        std::wstring wscXML =
            IniGetLongWS(scFilePath, "Info", std::to_string(i), L"");
        if (wscXML.length())
            wscPlayerInfo += L"<TEXT>" + wscXML + L"</TEXT><PARA/><PARA/>";
    }
    std::wstring wscXML = IniGetLongWS(scFilePath, "Info", "AdminNote", L"");
    if (wscXML.length())
        wscPlayerInfo += L"<TEXT>" + wscXML + L"</TEXT><PARA/><PARA/>";
    wscPlayerInfo += L"<POP/></RDL>";

    if (wscPlayerInfo.length() < 30) {
        PrintUserCmdText(iClientID, L"ERR No information available");
        return true;
    }

    HkChangeIDSString(iClientID, RSRCID_PLAYERINFO_TITLE, wszTargetName);
    HkChangeIDSString(iClientID, RSRCID_PLAYERINFO_TEXT, wscPlayerInfo);

    FmtStr caption(0, 0);
    caption.begin_mad_lib(RSRCID_PLAYERINFO_TITLE);
    caption.end_mad_lib();

    FmtStr message(0, 0);
    message.begin_mad_lib(RSRCID_PLAYERINFO_TEXT);
    message.end_mad_lib();

    pub::Player::PopUpDialog(iClientID, caption, message,
                             POPUPDIALOG_BUTTONS_CENTER_OK);
    return true;
}

static int CurrLength(const std::string &scFilePath) {
    int iCount = 0;
    for (int i = 1; i <= MAX_PARAGRAPHS; i++) {
        iCount +=
            IniGetLongWS(scFilePath, "Info", std::to_string(i), L"").length();
    }
    return iCount;
}

bool PlayerInfo::UserCmd_SetInfo(uint iClientID, const std::wstring &wscCmd,
                                 const std::wstring &wscParam,
                                 const wchar_t *usage) {
    uint iPara = ToInt(GetParam(wscParam, ' ', 0));
    const std::wstring &wscCommand = GetParam(wscParam, ' ', 1);
    const std::wstring &wscMsg = GetParamToEnd(wscParam, ' ', 2);

    std::string scFilePath = GetUserFilePath(
        (const wchar_t *)Players.GetActiveCharacterName(iClientID),
        "-info.ini");
    if (scFilePath.length() == 0)
        return false;

    if (iPara > 0 && iPara <= MAX_PARAGRAPHS && wscCommand == L"a") {
        int length = CurrLength(scFilePath) + wscMsg.length();
        if (length > MAX_CHARACTERS) {
            PrintUserCmdText(iClientID, L"ERR Too many characters. Limit is %d",
                             MAX_CHARACTERS);
            return false;
        }

        std::wstring wscNewMsg =
            IniGetLongWS(scFilePath, "Info", std::to_string(iPara), L"") +
            XMLText(wscMsg);
        IniWriteW(scFilePath, "Info", std::to_string(iPara), wscNewMsg);
        PrintUserCmdText(iClientID, L"OK %d/%d characters used", length,
                         MAX_CHARACTERS);
    } else if (iPara > 0 && iPara <= MAX_PARAGRAPHS && wscCommand == L"d") {
        IniWriteW(scFilePath, "Info", std::to_string(iPara), L"");
        PrintUserCmdText(iClientID, L"OK");
    } else {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        PrintUserCmdText(iClientID, L"/setinfo <paragraph> <command> <text>");
        PrintUserCmdText(
            iClientID, L"|  <paragraph> The paragraph number in the range 1-%d",
            MAX_PARAGRAPHS);
        PrintUserCmdText(iClientID,
                         L"|  <command> The command to perform on the "
                         L"paragraph, 'a' for append, 'd' for delete");
    }

    return true;
}
} // namespace PlayerInfo
