// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include <errno.h>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <windows.h>

#include <FLCoreCommon.h>
#include <FLCoreServer.h>
#include <FLHook.h>
#include <plugin.h>

#include "Mail.h"

#include <list>
#include <map>
#include <math.h>
#include <plugin_comms.h>
#include <vector>

#include "Main.h"

#include "../../hookext_plugin/hookext_exports.h"

namespace Message {

/** The messaging plugin message log for offline players */
static std::string MSG_LOG = "-mail.ini";

/** the data for a single online player */
class INFO {
public:
  INFO()
      : ulastPmClientID(-1), uTargetClientID(-1), bShowChatTime(false),
        bGreetingShown(false), iSwearWordWarnings(0) {}

  static const int NUMBER_OF_SLOTS = 10;
  std::wstring slot[NUMBER_OF_SLOTS];

  // Client ID of last PM.
  uint ulastPmClientID;

  // Client ID of selected target
  uint uTargetClientID;

  // Current chat time settings
  bool bShowChatTime;

  // True if the login banner has been displayed.
  bool bGreetingShown;

  // Swear word warn level
  int iSwearWordWarnings;
};

/** cache of preset messages for the online players (by client ID) */
static std::map<uint, INFO> mapInfo;

/** help text for when user types /help */
static std::list<INISECTIONVALUE> set_lstHelpLines;

/** greetings text for when user types /help */
static std::list<std::wstring> set_lstGreetingBannerLines;

/** special banner text for when user types /help */
static std::list<std::wstring> set_lstSpecialBannerLines;

/** special banner text for when user types /help */
static std::vector<std::list<std::wstring>> set_vctStandardBannerLines;

/** Time in second to repeat display of special banner */
static int set_iSpecialBannerTimeout;

/** Time in second to repeat display of standard banner */
static int set_iStandardBannerTimeout;

/** true if we override flhook built in help */
static bool set_bCustomHelp;

/** true if we echo user and admin commands to sender */
static bool set_bCmdEcho;

/** true if we don't echo mistyped user and admin commands to other players. */
static bool set_bCmdHide;

/** if true support the /showmsg and /setmsg commands */
static bool set_bSetMsg;

/** color of echoed commands */
static std::wstring set_wscCmdEchoStyle;

static std::wstring set_wscDisconnectSwearingInSpaceMsg;

static float set_fDisconnectSwearingInSpaceRange;

/** list of swear words */
static std::list<std::wstring> set_lstSwearWords;

/** A random macro to make things easier */
#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

/** Load the msgs for specified client ID into memory. */
static void LoadMsgs(uint iClientID) {
  if (!set_bSetMsg)
    return;

  // Load from disk the messages.
  for (int iMsgSlot = 0; iMsgSlot < INFO::NUMBER_OF_SLOTS; iMsgSlot++) {
    mapInfo[iClientID].slot[iMsgSlot] =
        HookExt::IniGetWS(iClientID, "msg." + std::to_string(iMsgSlot));
  }

  // Chat time settings.
  mapInfo[iClientID].bShowChatTime =
      HookExt::IniGetB(iClientID, "msg.chat_time");
}

/** Show the greeting banner to the specified player */
static void ShowGreetingBanner(int iClientID) {
  if (!mapInfo[iClientID].bGreetingShown) {
    mapInfo[iClientID].bGreetingShown = true;
    for (auto &line : set_lstGreetingBannerLines) {
      if (line.find(L"<TRA") == 0)
        HkFMsg(iClientID, line);
      else
        PrintUserCmdText(iClientID, L"%s", line.c_str());
    }
  }
}

/** Show the special banner to all players. */
static void ShowSpecialBanner() {
  struct PlayerData *pPD = 0;
  while (pPD = Players.traverse_active(pPD)) {
    uint iClientID = HkGetClientIdFromPD(pPD);
    for (auto &line : set_lstSpecialBannerLines) {
      if (line.find(L"<TRA") == 0)
        HkFMsg(iClientID, line);
      else
        PrintUserCmdText(iClientID, L"%s", line.c_str());
    }
  }
}

/** Show the next standard banner to all players. */
static void ShowStandardBanner() {
  if (set_vctStandardBannerLines.size() == 0)
    return;

  static size_t iCurStandardBanner = 0;
  if (++iCurStandardBanner >= set_vctStandardBannerLines.size())
    iCurStandardBanner = 0;

  auto &lstStandardBannerSection =
      set_vctStandardBannerLines[iCurStandardBanner];

  struct PlayerData *pPD = 0;
  while (pPD = Players.traverse_active(pPD)) {
    uint iClientID = HkGetClientIdFromPD(pPD);

    for (auto &sec : lstStandardBannerSection) {
      if (sec.find(L"<TRA") == 0)
        HkFMsg(iClientID, sec);
      else
        PrintUserCmdText(iClientID, L"%s", sec.c_str());
    }
  }
}

static std::wstring SetSizeToSmall(const std::wstring &wscDataFormat) {
  uint iFormat = wcstoul(wscDataFormat.c_str() + 2, 0, 16);
  wchar_t wszStyleSmall[32];
  wcscpy_s(wszStyleSmall, wscDataFormat.c_str());
  swprintf(wszStyleSmall + wcslen(wszStyleSmall) - 2,
           sizeof(wszStyleSmall) / sizeof(wchar_t) + wcslen(wszStyleSmall) - 2,
           L"%02X", 0x90 | (iFormat & 7));
  return wszStyleSmall;
}

/** Replace #t and #c tags with current target name and current ship location.
Return false if tags cannot be replaced. */
static bool ReplaceMessageTags(uint iClientID, INFO &clientData,
                               std::wstring &wscMsg) {
  if (wscMsg.find(L"#t") != -1) {
    if (clientData.uTargetClientID == -1) {
      PrintUserCmdText(iClientID, L"ERR Target not available");
      return false;
    }

    std::wstring wscTargetName =
        (const wchar_t *)Players.GetActiveCharacterName(
            clientData.uTargetClientID);
    wscMsg = ReplaceStr(wscMsg, L"#t", wscTargetName);
  }

  if (wscMsg.find(L"#c") != -1) {
    std::wstring wscCurrLocation = GetLocation(iClientID);
    wscMsg = ReplaceStr(wscMsg, L"#c", wscCurrLocation.c_str());
  }

  return true;
}

/** Clean up when a client disconnects */
void Message::ClearClientInfo(uint iClientID) { mapInfo.erase(iClientID); }

/**
This function is called when the admin command rehash is called and when the
module is loaded.
*/
void Message::LoadSettings(const std::string &scPluginCfgFile) {
  set_bCustomHelp = IniGetB(scPluginCfgFile, "Message", "CustomHelp", true);
  set_bCmdEcho = IniGetB(scPluginCfgFile, "Message", "CmdEcho", true);
  set_bCmdHide = IniGetB(scPluginCfgFile, "Message", "CmdHide", true);
  set_wscCmdEchoStyle =
      stows(IniGetS(scPluginCfgFile, "Message", "CmdEchoStyle", "0x00AA0090"));
  set_iStandardBannerTimeout =
      IniGetI(scPluginCfgFile, "Message", "StandardBannerDelay", 5);
  set_iSpecialBannerTimeout =
      IniGetI(scPluginCfgFile, "Message", "SpecialBannerDelay", 60);
  set_wscDisconnectSwearingInSpaceMsg =
      stows(IniGetS(scPluginCfgFile, "Message", "DisconnectSwearingInSpaceMsg",
                    "%player has been kicked for swearing"));
  set_fDisconnectSwearingInSpaceRange = IniGetF(
      scPluginCfgFile, "Message", "DisconnectSwearingInSpaceRange", 5000.0f);
  set_bSetMsg = IniGetB(scPluginCfgFile, "Message", "SetMsg", false);

  // For every active player load their msg settings.
  std::list<HKPLAYERINFO> players = HkGetPlayers();
  for (auto &p : players)
    LoadMsgs(p.iClientID);

  // Load the help, getting and banner text
  IniGetSection(scPluginCfgFile, "Help", set_lstHelpLines);

  set_lstGreetingBannerLines.clear();
  set_lstSpecialBannerLines.clear();
  set_vctStandardBannerLines.clear();

  INI_Reader ini;
  if (ini.open(scPluginCfgFile.c_str(), false)) {
    while (ini.read_header()) {
      if (ini.is_header("GreetingBanner")) {
        while (ini.read_value()) {
          set_lstGreetingBannerLines.push_back(
              Trim(stows(ini.get_value_string())));
        }
      } else if (ini.is_header("SpecialBanner")) {
        while (ini.read_value()) {
          set_lstSpecialBannerLines.push_back(
              Trim(stows(ini.get_value_string())));
        }
      } else if (ini.is_header("StandardBanner")) {
        std::list<std::wstring> lstStandardBannerSection;
        while (ini.read_value()) {
          lstStandardBannerSection.push_back(
              Trim(stows(ini.get_value_string())));
        }
        set_vctStandardBannerLines.push_back(lstStandardBannerSection);
      } else if (ini.is_header("SwearWords")) {
        while (ini.read_value()) {
          std::wstring word = Trim(stows(ini.get_value_string()));
          word = ReplaceStr(word, L"_", L" ");
          set_lstSwearWords.push_back(word);
        }
      }
    }
    ini.close();
  }
}

/// On this timer display banners
void Message::Timer() {
  static int iSpecialBannerTimer = 0;
  static int iStandardBannerTimer = 0;

  if (++iSpecialBannerTimer > set_iSpecialBannerTimeout) {
    iSpecialBannerTimer = 0;
    ShowSpecialBanner();
  }

  if (++iStandardBannerTimer > set_iStandardBannerTimeout) {
    iStandardBannerTimer = 0;
    ShowStandardBanner();
  }
}

/// On client disconnect remove any references to this client.
void Message::DisConnect(uint iClientID, enum EFLConnection p2) {
  auto iter = mapInfo.begin();
  while (iter != mapInfo.end()) {
    if (iter->second.ulastPmClientID == iClientID)
      iter->second.ulastPmClientID = -1;
    if (iter->second.uTargetClientID == iClientID)
      iter->second.uTargetClientID = -1;
    ++iter;
  }
}

/// On client F1 or entry to char select menu.
void Message::CharacterInfoReq(unsigned int iClientID, bool p2) {
  auto iter = mapInfo.begin();
  while (iter != mapInfo.end()) {
    if (iter->second.ulastPmClientID == iClientID)
      iter->second.ulastPmClientID = -1;
    if (iter->second.uTargetClientID == iClientID)
      iter->second.uTargetClientID = -1;
    ++iter;
  }
}

/// On launch events and reload the msg cache for the client.
void Message::PlayerLaunch(uint iShip, unsigned int iClientID) {
  LoadMsgs(iClientID);
  ShowGreetingBanner(iClientID);
  Mail::MailCheckLog((const wchar_t *)Players.GetActiveCharacterName(iClientID),
                     MSG_LOG);
}

/// On base entry events and reload the msg cache for the client.
void Message::BaseEnter(uint iBaseID, uint iClientID) {
  LoadMsgs(iClientID);
  ShowGreetingBanner(iClientID);
  Mail::MailCheckLog((const wchar_t *)Players.GetActiveCharacterName(iClientID),
                     MSG_LOG);
}

/// When a char selects a target and the target is a player ship then
/// record the target's clientID. */
void Message::SetTarget(uint uClientID, struct XSetTarget const &p2) {
  // The iSpaceID *appears* to represent a player ship ID when it is
  // targeted but this might not be the case. Also note that
  // HkGetClientIDByShip returns 0 on failure not -1.
  uint uTargetClientID = HkGetClientIDByShip(p2.iSpaceID);
  if (uTargetClientID) {
    auto iter = mapInfo.find(uClientID);
    if (iter != mapInfo.end()) {
      iter->second.uTargetClientID = uTargetClientID;
    }
  }
}

bool Message::SubmitChat(CHAT_ID cId, unsigned long iSize,
                         const void *rdlReader, CHAT_ID cIdTo, int p2) {
  // Ignore group join/leave commands
  if (cIdTo.iID == 0x10004)
    return false;

  // Extract text from rdlReader
  BinaryRDLReader rdl;
  wchar_t wszBuf[1024];
  uint iRet1;
  rdl.extract_text_from_buffer((unsigned short *)wszBuf, sizeof(wszBuf), iRet1,
                               (const char *)rdlReader, iSize);

  std::wstring wscChatMsg = ToLower(wszBuf);
  uint iClientID = cId.iID;

  bool bIsGroup = (cIdTo.iID == 0x10003 || !wscChatMsg.find(L"/g ") ||
                   !wscChatMsg.find(L"/group "));
  if (!bIsGroup) {
    // If a restricted word appears in the message take appropriate action.
    for (auto &word : set_lstSwearWords) {
      if (wscChatMsg.find(word) != -1) {
        PrintUserCmdText(iClientID, L"This is an automated message.");
        PrintUserCmdText(iClientID,
                         L"Please do not swear or you may be sanctioned.");

        mapInfo[iClientID].iSwearWordWarnings++;
        if (mapInfo[iClientID].iSwearWordWarnings > 2) {
          std::wstring wscCharname =
              (const wchar_t *)Players.GetActiveCharacterName(iClientID);
          AddLog("NOTICE: Swearing tempban on %s (%s) reason='%s'",
                 wstos(wscCharname).c_str(),
                 wstos(HkGetAccountID(HkGetAccountByCharname(wscCharname)))
                     .c_str(),
                 wstos(wscChatMsg).c_str());
          HkTempBan(iClientID, 10);
          HkDelayedKick(iClientID, 1);

          if (set_fDisconnectSwearingInSpaceRange > 0.0f) {
            std::wstring wscMsg = set_wscDisconnectSwearingInSpaceMsg;
            wscMsg =
                ReplaceStr(wscMsg, L"%time", GetTimeString(set_bLocalTime));
            wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
            PrintLocalUserCmdText(iClientID, wscMsg,
                                  set_fDisconnectSwearingInSpaceRange);
          }
        }
        return true;
      }
    }
  }

  /// When a private chat message is sent from one client to another record
  /// who sent the message so that the receiver can reply using the /r command
  /// */
  if (iClientID < 0x10000 && cIdTo.iID > 0 && cIdTo.iID < 0x10000) {
    auto iter = mapInfo.find(cIdTo.iID);
    if (iter != mapInfo.end()) {
      iter->second.ulastPmClientID = iClientID;
    }
  }
  return false;
}

/** This parameter is sent when we send a chat time line so that we don't print
a time chat line recursively. */
static bool bSendingTime = false;

/** When a chat message is sent to a client and this client has showchattime on
insert the time on the line immediately before the chat message */
bool Message::HkCb_SendChat(uint iClientID, uint iTo, uint iSize,
                            void *rdlReader) {
  // Return immediately if the chat line is the time.
  if (bSendingTime)
    return false;

  // Ignore group messages (I don't know if they ever get here
  if (iTo == 0x10004)
    return false;

  if (set_bCmdHide) {
    // Extract text from rdlReader
    BinaryRDLReader rdl;
    wchar_t wszBuf[1024];
    uint iRet1;
    rdl.extract_text_from_buffer((unsigned short *)wszBuf, sizeof(wszBuf),
                                 iRet1, (const char *)rdlReader, iSize);
    std::wstring wscChatMsg = wszBuf;

    // Find the ': ' which indicates the end of the sending player name.
    size_t iTextStartPos = wscChatMsg.find(L": ");
    if (iTextStartPos != std::string::npos) {
      if ((wscChatMsg.find(L": /") == iTextStartPos &&
           wscChatMsg.find(L": //") != iTextStartPos) ||
          wscChatMsg.find(L": .") == iTextStartPos) {
        return true;
      }
    }
  }

  if (mapInfo[iClientID].bShowChatTime) {
    // Send time with gray color (BEBEBE) in small text (90) above the chat
    // line.
    bSendingTime = true;
    HkFMsg(iClientID, L"<TRA data=\"0xBEBEBE90\" mask=\"-1\"/><TEXT>" +
                          XMLText(GetTimeString(set_bLocalTime)) + L"</TEXT>");
    bSendingTime = false;
  }
  return false;
}

/** Set an preset message */
bool Message::UserCmd_SetMsg(uint iClientID, const std::wstring &wscCmd,
                             const std::wstring &wscParam,
                             const wchar_t *usage) {
  if (!set_bSetMsg)
    return false;

  int iMsgSlot = ToInt(GetParam(wscParam, ' ', 0));
  std::wstring wscMsg = GetParamToEnd(wscParam, ' ', 1);

  if (iMsgSlot < 0 || iMsgSlot > 9 || wscParam.size() == 0) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }

  HookExt::IniSetWS(iClientID, "msg." + std::to_string(iMsgSlot), wscMsg);

  // Reload the character cache
  LoadMsgs(iClientID);
  PrintUserCmdText(iClientID, L"OK");
  return true;
}

/** Show preset messages */
bool Message::UserCmd_ShowMsgs(uint iClientID, const std::wstring &wscCmd,
                               const std::wstring &wscParam,
                               const wchar_t *usage) {
  if (!set_bSetMsg)
    return false;

  auto iter = mapInfo.find(iClientID);
  if (iter == mapInfo.end()) {
    PrintUserCmdText(iClientID, L"ERR No messages");
    return true;
  }

  for (int i = 0; i < INFO::NUMBER_OF_SLOTS; i++) {
    PrintUserCmdText(iClientID, L"%d: %s", i, iter->second.slot[i].c_str());
  }
  PrintUserCmdText(iClientID, L"OK");
  return true;
}

/** Send an preset message to the system chat */
bool Message::UserCmd_SMsg(uint iClientID, const std::wstring &wscCmd,
                           const std::wstring &wscParam, const wchar_t *usage) {
  if (!set_bSetMsg)
    return false;

  int iMsgSlot = ToInt(wscCmd.substr(1, 1));
  if (iMsgSlot < 0 || iMsgSlot > 9) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }

  auto iter = mapInfo.find(iClientID);
  if (iter == mapInfo.end() || iter->second.slot[iMsgSlot].size() == 0) {
    PrintUserCmdText(iClientID, L"ERR No message defined");
    return true;
  }

  // Replace the tag #t with name of the targeted player.
  std::wstring wscMsg = iter->second.slot[iMsgSlot];
  if (!ReplaceMessageTags(iClientID, iter->second, wscMsg))
    return true;

  SendSystemChat(iClientID, wscMsg);

  return true;
}

/** Send an preset message to the local system chat */
bool Message::UserCmd_LMsg(uint iClientID, const std::wstring &wscCmd,
                           const std::wstring &wscParam, const wchar_t *usage) {
  if (!set_bSetMsg)
    return false;

  int iMsgSlot = ToInt(wscCmd.substr(2, 1));
  if (iMsgSlot < 0 || iMsgSlot > 9) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }

  auto iter = mapInfo.find(iClientID);
  if (iter == mapInfo.end() || iter->second.slot[iMsgSlot].size() == 0) {
    PrintUserCmdText(iClientID, L"ERR No message defined");
    return true;
  }

  // Replace the tag #t with name of the targeted player.
  std::wstring wscMsg = iter->second.slot[iMsgSlot];
  if (!ReplaceMessageTags(iClientID, iter->second, wscMsg))
    return true;

  SendLocalSystemChat(iClientID, wscMsg);

  return true;
}

/** Send an preset message to the group chat */
bool Message::UserCmd_GMsg(uint iClientID, const std::wstring &wscCmd,
                           const std::wstring &wscParam, const wchar_t *usage) {
  if (!set_bSetMsg)
    return false;

  int iMsgSlot = ToInt(wscCmd.substr(2, 1));
  if (iMsgSlot < 0 || iMsgSlot > 9) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }

  auto iter = mapInfo.find(iClientID);
  if (iter == mapInfo.end() || iter->second.slot[iMsgSlot].size() == 0) {
    PrintUserCmdText(iClientID, L"ERR No message defined");
    return true;
  }

  // Replace the tag #t with name of the targeted player.
  std::wstring wscMsg = iter->second.slot[iMsgSlot];
  if (!ReplaceMessageTags(iClientID, iter->second, wscMsg))
    return true;

  SendGroupChat(iClientID, wscMsg);
  return true;
}

/** Send an message to the last person that PM'd this client. */
bool Message::UserCmd_ReplyToLastPMSender(uint iClientID,
                                          const std::wstring &wscCmd,
                                          const std::wstring &wscParam,
                                          const wchar_t *usage) {
  auto iter = mapInfo.find(iClientID);
  if (iter == mapInfo.end()) {
    // There's no way for this to happen! yeah right.
    PrintUserCmdText(iClientID, L"ERR No message defined");
    return true;
  }

  std::wstring wscMsg = GetParamToEnd(wscParam, ' ', 0);

  // If this is a /rN command then setup the preset message
  if (set_bSetMsg && wscCmd.size() == 3 && wscMsg.size() == 0) {
    int iMsgSlot = ToInt(wscCmd.substr(2, 1));
    if (iMsgSlot < 0 || iMsgSlot > 9) {
      PrintUserCmdText(iClientID, L"ERR Invalid parameters");
      PrintUserCmdText(iClientID, usage);
      return true;
    }
    if (iter->second.slot[iMsgSlot].size() == 0) {
      PrintUserCmdText(iClientID, L"ERR No message defined");
      return true;
    }
    // Replace the tag #t with name of the targeted player.
    wscMsg = iter->second.slot[iMsgSlot];
    if (!ReplaceMessageTags(iClientID, iter->second, wscMsg))
      return true;
  } else if (wscMsg.size() == 0) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }

  if (iter->second.ulastPmClientID == -1) {
    PrintUserCmdText(iClientID, L"ERR PM sender not available");
    return true;
  }

  mapInfo[iter->second.ulastPmClientID].ulastPmClientID = iClientID;
  SendPrivateChat(iClientID, iter->second.ulastPmClientID, wscMsg);
  return true;
}

/** Send a message to the last/current target. */
bool Message::UserCmd_SendToLastTarget(uint iClientID,
                                       const std::wstring &wscCmd,
                                       const std::wstring &wscParam,
                                       const wchar_t *usage) {
  auto iter = mapInfo.find(iClientID);
  if (iter == mapInfo.end()) {
    // There's no way for this to happen! yeah right.
    PrintUserCmdText(iClientID, L"ERR No message defined");
    return true;
  }

  std::wstring wscMsg = GetParamToEnd(wscParam, ' ', 0);

  // If this is a /tN command then setup the preset message
  if (set_bSetMsg && wscCmd.size() == 3 && wscMsg.size() == 0) {
    int iMsgSlot = ToInt(wscCmd.substr(2, 1));
    if (iMsgSlot < 0 || iMsgSlot > 9) {
      PrintUserCmdText(iClientID, L"ERR Invalid parameters");
      PrintUserCmdText(iClientID, usage);
      return true;
    }
    if (iter->second.slot[iMsgSlot].size() == 0) {
      PrintUserCmdText(iClientID, L"ERR No message defined");
      return true;
    }
    // Replace the tag #t with name of the targeted player.
    wscMsg = iter->second.slot[iMsgSlot];
    if (!ReplaceMessageTags(iClientID, iter->second, wscMsg))
      return true;
  } else if (wscMsg.size() == 0) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }

  if (iter->second.uTargetClientID == -1) {
    PrintUserCmdText(iClientID, L"ERR PM target not available");
    return true;
  }

  mapInfo[iter->second.uTargetClientID].ulastPmClientID = iClientID;
  SendPrivateChat(iClientID, iter->second.uTargetClientID, wscMsg);
  return true;
}

/** Shows the sender of the last PM and the last char targetted */
bool Message::UserCmd_ShowLastPMSender(uint iClientID,
                                       const std::wstring &wscCmd,
                                       const std::wstring &wscParam,
                                       const wchar_t *usage) {
  auto iter = mapInfo.find(iClientID);
  if (iter == mapInfo.end()) {
    // There's no way for this to happen! yeah right.
    PrintUserCmdText(iClientID, L"ERR No message defined");
    return true;
  }

  std::wstring wscSenderCharname =
      L"<not available>" + std::to_wstring(iter->second.ulastPmClientID);
  if (iter->second.ulastPmClientID != -1 &&
      HkIsValidClientID(iter->second.ulastPmClientID))
    wscSenderCharname = (const wchar_t *)Players.GetActiveCharacterName(
        iter->second.ulastPmClientID);

  std::wstring wscTargetCharname =
      L"<not available>" + std::to_wstring(iter->second.uTargetClientID);
  if (iter->second.uTargetClientID != -1 &&
      HkIsValidClientID(iter->second.uTargetClientID))
    wscTargetCharname = (const wchar_t *)Players.GetActiveCharacterName(
        iter->second.uTargetClientID);

  PrintUserCmdText(iClientID, L"OK sender=" + wscSenderCharname + L" target=" +
                                  wscTargetCharname);
  return true;
}

/** Send a private message to the specified charname. If the player is offline
the message will be delivery when they next login. */
bool Message::UserCmd_PrivateMsg(uint iClientID, const std::wstring &wscCmd,
                                 const std::wstring &wscParam,
                                 const wchar_t *usage) {
  std::wstring wscCharname =
      (const wchar_t *)Players.GetActiveCharacterName(iClientID);
  const std::wstring &wscTargetCharname = GetParam(wscParam, ' ', 0);
  const std::wstring &wscMsg = GetParamToEnd(wscParam, ' ', 1);

  if (wscCharname.size() == 0 || wscMsg.size() == 0) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }

  if (!HkGetAccountByCharname(wscTargetCharname)) {
    PrintUserCmdText(iClientID, L"ERR charname does not exist");
    return true;
  }

  uint iToClientID = HkGetClientIdFromCharname(wscTargetCharname);
  if (iToClientID == -1) {
    Mail::MailSend(wscTargetCharname, MSG_LOG, wscCharname + L": " + wscMsg);
    Mail::MailCheckLog(wscTargetCharname, MSG_LOG);
    PrintUserCmdText(iClientID, L"OK message saved to mailbox");
  } else {
    mapInfo[iToClientID].ulastPmClientID = iClientID;
    SendPrivateChat(iClientID, iToClientID, wscMsg);
  }

  return true;
}

/** Send a private message to the specified clientid. */
bool Message::UserCmd_PrivateMsgID(uint iClientID, const std::wstring &wscCmd,
                                   const std::wstring &wscParam,
                                   const wchar_t *usage) {
  std::wstring wscCharname =
      (const wchar_t *)Players.GetActiveCharacterName(iClientID);
  const std::wstring &wscClientID = GetParam(wscParam, ' ', 0);
  const std::wstring &wscMsg = GetParamToEnd(wscParam, ' ', 1);

  uint iToClientID = ToInt(wscClientID);
  if (!HkIsValidClientID(iToClientID) || HkIsInCharSelectMenu(iToClientID)) {
    PrintUserCmdText(iClientID, L"ERR Invalid client-id");
    return true;
  }

  mapInfo[iToClientID].ulastPmClientID = iClientID;
  SendPrivateChat(iClientID, iToClientID, wscMsg);
  return true;
}

/** Send a message to all players with a particular prefix. */
bool Message::UserCmd_FactionMsg(uint iClientID, const std::wstring &wscCmd,
                                 const std::wstring &wscParam,
                                 const wchar_t *usage) {
  std::wstring wscSender =
      (const wchar_t *)Players.GetActiveCharacterName(iClientID);
  const std::wstring &wscCharnamePrefix = GetParam(wscParam, ' ', 0);
  const std::wstring &wscMsg = GetParamToEnd(wscParam, ' ', 1);

  if (wscCharnamePrefix.size() < 3 || wscMsg.size() == 0) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }

  bool bSenderReceived = false;
  bool bMsgSent = false;
  for (auto &player : HkGetPlayers()) {
    if (ToLower(player.wscCharname).find(ToLower(wscCharnamePrefix)) ==
        std::string::npos)
      continue;

    if (player.iClientID == iClientID)
      bSenderReceived = true;

    FormatSendChat(player.iClientID, wscSender, wscMsg, L"FF7BFF");
    bMsgSent = true;
  }
  if (!bSenderReceived)
    FormatSendChat(iClientID, wscSender, wscMsg, L"FF7BFF");

  if (bMsgSent == false)
    PrintUserCmdText(iClientID, L"ERR No chars found");
  return true;
}

/** Send a faction invite message to all players with a particular prefix. */
bool Message::UserCmd_FactionInvite(uint iClientID, const std::wstring &wscCmd,
                                    const std::wstring &wscParam,
                                    const wchar_t *usage) {
  const std::wstring &wscCharnamePrefix = GetParam(wscParam, ' ', 0);

  bool msgSent = false;

  if (wscCharnamePrefix.size() < 3) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }

  for (auto &player : HkGetPlayers()) {
    if (ToLower(player.wscCharname).find(ToLower(wscCharnamePrefix)) ==
        std::string::npos)
      continue;
    if (player.iClientID == iClientID)
      continue;

    std::wstring wscMsg = L"/i " + player.wscCharname;

    uint iRet;
    char szBuf[1024];
    HK_ERROR err;
    if ((err = HkFMsgEncodeXML(wscMsg, szBuf, sizeof(szBuf), iRet)) != HKE_OK) {
      PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
      return true;
    }

    struct CHAT_ID cId = {iClientID};
    struct CHAT_ID cIdTo = {0x10001};

    Server.SubmitChat(cId, iRet, szBuf, cIdTo, -1);
    msgSent = true;
  }

  if (msgSent == false)
    PrintUserCmdText(iClientID, L"ERR No chars found");

  return true;
}

bool Message::UserCmd_SetChatTime(uint iClientID, const std::wstring &wscCmd,
                                  const std::wstring &wscParam,
                                  const wchar_t *usage) {
  std::wstring wscParam1 = ToLower(GetParam(wscParam, ' ', 0));
  bool bShowChatTime = false;
  if (!wscParam1.compare(L"on"))
    bShowChatTime = true;
  else if (!wscParam1.compare(L"off"))
    bShowChatTime = false;
  else {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
  }

  std::wstring wscCharname =
      (const wchar_t *)Players.GetActiveCharacterName(iClientID);

  HookExt::IniSetB(iClientID, "msg.chat_time", bShowChatTime);

  // Update the client cache.
  auto iter = mapInfo.find(iClientID);
  if (iter != mapInfo.end())
    iter->second.bShowChatTime = bShowChatTime;

  // Send confirmation msg
  PrintUserCmdText(iClientID, L"OK");
  return true;
}

bool Message::UserCmd_Time(uint iClientID, const std::wstring &wscCmd,
                           const std::wstring &wscParam, const wchar_t *usage) {
  // Send time with gray color (BEBEBE) in small text (90) above the chat line.
  PrintUserCmdText(iClientID, GetTimeString(set_bLocalTime));
  return true;
}

/** Print out custom help overriding flhook built in help */
bool Message::UserCmd_CustomHelp(uint iClientID, const std::wstring &wscCmd,
                                 const std::wstring &wscParam,
                                 const wchar_t *usage) {
  if (set_bCustomHelp) {
    // Print any custom help strings
    for (auto &line : set_lstHelpLines) {
      std::string scHelp = line.scKey;
      if (line.scValue.size() > 0) {
        scHelp += "=";
        scHelp += line.scValue;
      }
      PrintUserCmdText(iClientID, stows(scHelp));
    }
    return true;
  }
  return false;
}

/** Print out help for built in flhook commands */
bool Message::UserCmd_BuiltInCmdHelp(uint iClientID, const std::wstring &wscCmd,
                                     const std::wstring &wscParam,
                                     const wchar_t *usage) {
  if (wscParam.size() == 0) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }
  return false;
}

/** Show Mail */
bool Message::UserCmd_MailShow(uint iClientID, const std::wstring &wscCmd,
                               const std::wstring &wscParam,
                               const wchar_t *usage) {
  int iNumberUnreadMsgs = Mail::MailCountUnread(
      (const wchar_t *)Players.GetActiveCharacterName(iClientID), MSG_LOG);
  int iNumberMsgs = Mail::MailCount(
      (const wchar_t *)Players.GetActiveCharacterName(iClientID), MSG_LOG);
  if (iNumberMsgs == 0) {
    PrintUserCmdText(iClientID, L"OK You have no messages");
    return true;
  }

  int iFirstMsg = ToInt(ToLower(GetParam(wscParam, ' ', 0)));
  if (iFirstMsg == 0) {
    if (iNumberUnreadMsgs > 0)
      PrintUserCmdText(iClientID, L"OK You have %d unread messages",
                       iNumberUnreadMsgs);
    else
      PrintUserCmdText(iClientID, L"OK You have %d messages", iNumberMsgs);
    PrintUserCmdText(iClientID, L"Type /mail 1 to see first message or /mail "
                                L"<num> to see specified message");
    return true;
  }

  if (iFirstMsg > iNumberMsgs) {
    PrintUserCmdText(iClientID, L"ERR Message does not exist");
    return true;
  }

  Mail::MailShow((const wchar_t *)Players.GetActiveCharacterName(iClientID),
                 MSG_LOG, iFirstMsg);
  return true;
}

/** Delete Mail */
bool Message::UserCmd_MailDel(uint iClientID, const std::wstring &wscCmd,
                              const std::wstring &wscParam,
                              const wchar_t *usage) {
  if (wscParam.size() == 0) {
    PrintUserCmdText(iClientID, L"ERR Invalid parameters");
    PrintUserCmdText(iClientID, usage);
    return true;
  }

  int iNumberMsgs = Mail::MailCount(
      (const wchar_t *)Players.GetActiveCharacterName(iClientID), MSG_LOG);
  int iMsg = ToInt(ToLower(GetParam(wscParam, ' ', 0)));
  if (iMsg == 0 || iMsg > iNumberMsgs) {
    PrintUserCmdText(iClientID, L"ERR Message does not exist");
    return true;
  }

  if (Mail::MailDel((const wchar_t *)Players.GetActiveCharacterName(iClientID),
                    MSG_LOG, iMsg))
    PrintUserCmdText(iClientID, L"OK");
  else
    PrintUserCmdText(iClientID, L"ERR");
  return true;
}

void Message::UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
  std::wstring wscCmdLineLower = ToLower(wscCmd);

  // Echo the command back to the sender's console but only if it starts with /
  // or .
  if (set_bCmdEcho) {
    std::wstring wscCmd = GetParam(wscCmdLineLower, ' ', 0);
    if (wscCmd.find(L"/") == 0 || wscCmd.find(L".") == 0) {
      if (!(wscCmd == L"/l" || wscCmd == L"/local" || wscCmd == L"/s" ||
            wscCmd == L"/system" || wscCmd == L"/g" || wscCmd == L"/group" ||
            wscCmd == L"/t" || wscCmd == L"/target" || wscCmd == L"/r" ||
            wscCmd == L"/reply" || wscCmd.find(L"//") == 0 ||
            wscCmd.find(L"*") == (wscCmd.length() - 1))) {
        std::wstring wscXML = L"<TRA data=\"" + set_wscCmdEchoStyle +
                              L"\" mask=\"-1\"/><TEXT>" +
                              XMLText(wscCmdLineLower) + L"</TEXT>";
        HkFMsg(iClientID, wscXML);
      }
    }
  }
}

void Message::AdminCmd_SendMail(CCmds *cmds, const std::wstring &wscCharname,
                                const std::wstring &wscMsg) {
  Mail::MailSend(wscCharname, MSG_LOG, cmds->GetAdminName() + L": " + wscMsg);
  cmds->Print(L"OK message saved to mailbox\n");
}

/** Me command allow players to type "/me powers his engines" which would print:
 * "Trent powers his engines" */
bool Message::UserCmd_Me(uint iClientID, const std::wstring &wscCmd,
                         const std::wstring &wscParam, const wchar_t *usage) {
  if (set_bEnableMe) {
    std::wstring charname =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);
    uint iSystemID;
    pub::Player::GetSystem(iClientID, iSystemID);

    // Encode message using the death message style (red text).
    std::wstring wscXMLMsg =
        L"<TRA data=\"" + set_wscDeathMsgStyleSys + L"\" mask=\"-1\"/> <TEXT>";
    wscXMLMsg += charname + L" ";
    wscXMLMsg += XMLText(GetParamToEnd(wscParam, ' ', 0));
    wscXMLMsg += L"</TEXT>";

    return RedText(wscXMLMsg, iSystemID);
  } else {
    PrintUserCmdText(iClientID, L"Command not enabled.");
    return true;
  }
}

/** Do command allow players to type "/do Nomad fighters detected" which would
 * print: "Nomad fighters detected" in the standard red text */
bool Message::UserCmd_Do(uint iClientID, const std::wstring &wscCmd,
                         const std::wstring &wscParam, const wchar_t *usage) {
  if (set_bEnableDo) {
    uint iSystemID;
    pub::Player::GetSystem(iClientID, iSystemID);

    // Encode message using the death message style (red text).
    std::wstring wscXMLMsg =
        L"<TRA data=\"" + set_wscDeathMsgStyleSys + L"\" mask=\"-1\"/> <TEXT>";
    wscXMLMsg += XMLText(GetParamToEnd(wscParam, ' ', 0));
    wscXMLMsg += L"</TEXT>";

    return RedText(wscXMLMsg, iSystemID);
  } else {
    PrintUserCmdText(iClientID, L"Command not enabled.");
    return true;
  }
}

bool Message::RedText(std::wstring wscXMLMsg, uint iSystemID) {
  char szBuf[0x1000];
  uint iRet;
  if (!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsg, szBuf, sizeof(szBuf), iRet)))
    return false;

  // Send to all players in system
  struct PlayerData *pPD = 0;
  while (pPD = Players.traverse_active(pPD)) {
    uint iClientID = HkGetClientIdFromPD(pPD);
    uint iClientSystemID = 0;
    pub::Player::GetSystem(iClientID, iClientSystemID);

    if (iSystemID == iClientSystemID)
      HkFMsgSendChat(iClientID, szBuf, iRet);
  }
  return true;
}

/// Hook for ship distruction. It's easier to hook this than the PlayerDeath
/// one. Drop a percentage of cargo + some loot representing ship bits.
void Message::SendDeathMsg(const std::wstring &wscMsg, uint iSystemID,
                           uint iClientIDVictim, uint iClientIDKiller) {
  // encode xml std::string(default and small)
  // non-sys
  std::wstring wscXMLMsg =
      L"<TRA data=\"" + set_wscDeathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
  wscXMLMsg += XMLText(wscMsg);
  wscXMLMsg += L"</TEXT>";

  char szBuf[0xFFFF];
  uint iRet;
  if (!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsg, szBuf, sizeof(szBuf), iRet)))
    return;

  std::wstring wscStyleSmall = SetSizeToSmall(set_wscDeathMsgStyle);
  std::wstring wscXMLMsgSmall = std::wstring(L"<TRA data=\"") + wscStyleSmall +
                                L"\" mask=\"-1\"/> <TEXT>";
  wscXMLMsgSmall += XMLText(wscMsg);
  wscXMLMsgSmall += L"</TEXT>";
  char szBufSmall[0xFFFF];
  uint iRetSmall;
  if (!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsgSmall, szBufSmall,
                                   sizeof(szBufSmall), iRetSmall)))
    return;

  // sys
  std::wstring wscXMLMsgSys =
      L"<TRA data=\"" + set_wscDeathMsgStyleSys + L"\" mask=\"-1\"/> <TEXT>";
  wscXMLMsgSys += XMLText(wscMsg);
  wscXMLMsgSys += L"</TEXT>";
  char szBufSys[0xFFFF];
  uint iRetSys;
  if (!HKHKSUCCESS(
          HkFMsgEncodeXML(wscXMLMsgSys, szBufSys, sizeof(szBufSys), iRetSys)))
    return;

  std::wstring wscStyleSmallSys = SetSizeToSmall(set_wscDeathMsgStyleSys);
  std::wstring wscXMLMsgSmallSys =
      L"<TRA data=\"" + wscStyleSmallSys + L"\" mask=\"-1\"/> <TEXT>";
  wscXMLMsgSmallSys += XMLText(wscMsg);
  wscXMLMsgSmallSys += L"</TEXT>";
  char szBufSmallSys[0xFFFF];
  uint iRetSmallSys;
  if (!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsgSmallSys, szBufSmallSys,
                                   sizeof(szBufSmallSys), iRetSmallSys)))
    return;

  // send
  // for all players
  struct PlayerData *pPD = 0;
  while (pPD = Players.traverse_active(pPD)) {
    uint iClientID = HkGetClientIdFromPD(pPD);
    uint iClientSystemID = 0;
    pub::Player::GetSystem(iClientID, iClientSystemID);

    if (mapInfo[iClientID].bShowChatTime) {
      // Send time with gray color (BEBEBE) in small text (90) above the chat
      // line.
      bSendingTime = true;
      HkFMsg(iClientID, L"<TRA data=\"0xBEBEBE90\" mask=\"-1\"/><TEXT>" +
                            XMLText(GetTimeString(set_bLocalTime)) +
                            L"</TEXT>");
      bSendingTime = false;
    }

    char *szXMLBuf;
    int iXMLBufRet;
    char *szXMLBufSys;
    int iXMLBufRetSys;
    if (set_bUserCmdSetDieMsgSize &&
        (ClientInfo[iClientID].dieMsgSize == CS_SMALL)) {
      szXMLBuf = szBufSmall;
      iXMLBufRet = iRetSmall;
      szXMLBufSys = szBufSmallSys;
      iXMLBufRetSys = iRetSmallSys;
    } else {
      szXMLBuf = szBuf;
      iXMLBufRet = iRet;
      szXMLBufSys = szBufSys;
      iXMLBufRetSys = iRetSys;
    }

    if (!set_bUserCmdSetDieMsg) { // /set diemsg disabled, thus send to all
      if (iSystemID == iClientSystemID)
        HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
      else
        HkFMsgSendChat(iClientID, szXMLBuf, iXMLBufRet);
      continue;
    }

    if (ClientInfo[iClientID].dieMsg == DIEMSG_NONE)
      continue;
    else if ((ClientInfo[iClientID].dieMsg == DIEMSG_SYSTEM) &&
             (iSystemID == iClientSystemID))
      HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
    else if ((ClientInfo[iClientID].dieMsg == DIEMSG_SELF) &&
             ((iClientID == iClientIDVictim) || (iClientID == iClientIDKiller)))
      HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
    else if (ClientInfo[iClientID].dieMsg == DIEMSG_ALL) {
      if (iSystemID == iClientSystemID)
        HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
      else
        HkFMsgSendChat(iClientID, szXMLBuf, iXMLBufRet);
    }
  }
}
} // namespace Message
