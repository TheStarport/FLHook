#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT bool g_bMsg = false;

HK_ERROR HkMsg(uint iClientID, const std::wstring &wscMessage) {
  struct CHAT_ID ci = {0};
  struct CHAT_ID ciClient = {iClientID};

  std::wstring wscXML = L"<TRA data=\"0x19BD3A00\" mask=\"-1\"/><TEXT>" +
                        XMLText(wscMessage) + L"</TEXT>";
  uint iRet;
  char szBuf[1024];
  HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet);
  g_bMsg = true;
  HkIServerImpl::SubmitChat(ci, iRet, szBuf, ciClient, -1);
  g_bMsg = false;

  return HKE_OK;
}

HK_ERROR HkMsg(const std::wstring &wscCharname,
               const std::wstring &wscMessage) {
  HK_GET_CLIENTID(iClientID, wscCharname);

  if (iClientID == -1)
    return HKE_PLAYER_NOT_LOGGED_IN;

  return HkMsg(iClientID, wscMessage);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool g_bMsgS = false;

HK_ERROR HkMsgS(const std::wstring &wscSystemname,
                const std::wstring &wscMessage) {
  uint iSystemID = 0;
  if (!(iSystemID = ToInt(wscSystemname.c_str()))) {
    pub::GetSystemID(iSystemID, wstos(wscSystemname).c_str());
    if (!iSystemID)
      return HKE_INVALID_SYSTEM;
    ;
  }

  // prepare xml
  std::wstring wscXML = L"<TRA data=\"0xE6C68400\" mask=\"-1\"/><TEXT>" +
                        XMLText(wscMessage) + L"</TEXT>";
  uint iRet;
  char szBuf[1024];
  HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet);

  struct CHAT_ID ci = {0};

  // for all players in system...
  struct PlayerData *pPD = 0;
  while (pPD = Players.traverse_active(pPD)) {
    uint iClientID = HkGetClientIdFromPD(pPD);
    uint iClientSystemID = 0;
    pub::Player::GetSystem(iClientID, iClientSystemID);
    if (iSystemID == iClientSystemID) {
      struct CHAT_ID ciClient = {iClientID};
      g_bMsgS = true;
      HkIServerImpl::SubmitChat(ci, iRet, szBuf, ciClient, -1);
      g_bMsgS = false;
    }
  }

  return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool g_bMsgU = false;

HK_ERROR HkMsgU(const std::wstring &wscMessage) {
  struct CHAT_ID ci = {0};
  struct CHAT_ID ciClient = {0x00010000};

  std::wstring wscXML = L"<TRA font=\"1\" color=\"#FFFFFF\"/><TEXT>" +
                        XMLText(wscMessage) + L"</TEXT>";
  uint iRet;
  char szBuf[1024];
  HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet);
  g_bMsgU = true;
  HkIServerImpl::SubmitChat(ci, iRet, szBuf, ciClient, -1);
  g_bMsgU = false;

  return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkFMsgEncodeXML(const std::wstring &wscXML, char *szBuf, uint iSize,
                         uint &iRet) {
  XMLReader rdr;
  RenderDisplayList rdl;
  std::wstring wscMsg =
      L"<?xml version=\"1.0\" encoding=\"UTF-16\"?><RDL><PUSH/>";
  wscMsg += wscXML;
  wscMsg += L"<PARA/><POP/></RDL>\x000A\x000A";
  if (!rdr.read_buffer(rdl, (const char *)wscMsg.c_str(),
                       (uint)wscMsg.length() * 2))
    return HKE_WRONG_XML_SYNTAX;
  ;

  BinaryRDLWriter rdlwrite;
  rdlwrite.write_buffer(rdl, szBuf, iSize, iRet);

  return HKE_OK;
  ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

_RCSendChatMsg RCSendChatMsg;

HK_ERROR HkFMsgSendChat(uint iClientID, char *szBuf, uint iSize) {
  uint p4 = (uint)szBuf;
  uint p3 = iSize;
  uint p2 = 0x00010000;
  uint p1 = iClientID;

    __asm {
        push [p4]
        push [p3]
        push [p2]
        push [p1]
        mov ecx, [Client]
        add ecx, 4
        call [RCSendChatMsg]
    }

  return HKE_OK;
  ;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkFMsg(uint iClientID, const std::wstring &wscXML) {
  char szBuf[0xFFFF];
  uint iRet;
  if (!HKHKSUCCESS(HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet)))
    return HKE_WRONG_XML_SYNTAX;
  ;

  HkFMsgSendChat(iClientID, szBuf, iRet);
  return HKE_OK;
}

HK_ERROR HkFMsg(const std::wstring &wscCharname, const std::wstring &wscXML) {
  HK_GET_CLIENTID(iClientID, wscCharname);

  if (iClientID == -1)
    return HKE_PLAYER_NOT_LOGGED_IN;

  return HkFMsg(iClientID, wscXML);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkFMsgS(const std::wstring &wscSystemname,
                 const std::wstring &wscXML) {
  // get system id
  uint iSystemID = 0;
  if (!(iSystemID = ToInt(wscSystemname.c_str()))) {
    pub::GetSystemID(iSystemID, wstos(wscSystemname).c_str());
    if (!iSystemID)
      return HKE_INVALID_SYSTEM;
    ;
  }

  // encode xml std::string
  char szBuf[0xFFFF];
  uint iRet;
  if (!HKHKSUCCESS(HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet)))
    return HKE_WRONG_XML_SYNTAX;
  ;

  // for all players in system...
  struct PlayerData *pPD = 0;
  while (pPD = Players.traverse_active(pPD)) {
    uint iClientID = HkGetClientIdFromPD(pPD);
    uint iClientSystemID = 0;
    pub::Player::GetSystem(iClientID, iClientSystemID);
    if (iSystemID == iClientSystemID)
      HkFMsgSendChat(iClientID, szBuf, iRet);
  }

  return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkFMsgU(const std::wstring &wscXML) {
  // encode xml std::string
  char szBuf[0xFFFF];
  uint iRet;
  if (!HKHKSUCCESS(HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet)))
    return HKE_WRONG_XML_SYNTAX;
  ;

  // for all players
  struct PlayerData *pPD = 0;
  while (pPD = Players.traverse_active(pPD)) {
    uint iClientID = HkGetClientIdFromPD(pPD);
    HkFMsgSendChat(iClientID, szBuf, iRet);
  }

  return HKE_OK;
}

/** Format a chat std::string in accordance with the receiver's preferences and
send it. Will check that the receiver accepts messages from wscSender and
refuses to send if necessary. */
void FormatSendChat(uint iToClientID, const std::wstring &wscSender,
                    const std::wstring &wscText,
                    const std::wstring &wscTextColor) {
#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

  if (set_bUserCmdIgnore) {
    for (auto &ignore : ClientInfo[iToClientID].lstIgnore) {
      if (!HAS_FLAG(ignore, L"i") &&
          !(ToLower(wscSender).compare(ToLower(ignore.wscCharname))))
        return; // ignored
      else if (HAS_FLAG(ignore, L"i") &&
               (ToLower(wscSender).find(ToLower(ignore.wscCharname)) != -1))
        return; // ignored
    }
  }

  uchar cFormat;
  // adjust chatsize
  switch (ClientInfo[iToClientID].chatSize) {
  case CS_SMALL:
    cFormat = 0x90;
    break;
  case CS_BIG:
    cFormat = 0x10;
    break;
  default:
    cFormat = 0x00;
    break;
  }

  // adjust chatstyle
  switch (ClientInfo[iToClientID].chatStyle) {
  case CST_BOLD:
    cFormat += 0x01;
    break;
  case CST_ITALIC:
    cFormat += 0x02;
    break;
  case CST_UNDERLINE:
    cFormat += 0x04;
    break;
  default:
    cFormat += 0x00;
    break;
  }

  wchar_t wszFormatBuf[8];
  swprintf(wszFormatBuf, _countof(wszFormatBuf), L"%02X", (long)cFormat);
  std::wstring wscTRADataFormat = wszFormatBuf;
  const std::wstring wscTRADataSenderColor = L"FFFFFF"; // white

  std::wstring wscXML =
      L"<TRA data=\"0x" + wscTRADataSenderColor + wscTRADataFormat +
      L"\" mask=\"-1\"/><TEXT>" + XMLText(wscSender) + L": </TEXT>" +
      L"<TRA data=\"0x" + wscTextColor + wscTRADataFormat +
      L"\" mask=\"-1\"/><TEXT>" + XMLText(wscText) + L"</TEXT>";

  HkFMsg(iToClientID, wscXML);
}

/** Send a player to player message */
void SendPrivateChat(uint iFromClientID, uint iToClientID,
                     const std::wstring &wscText) {
  std::wstring wscSender =
      (const wchar_t *)Players.GetActiveCharacterName(iFromClientID);

  if (set_bUserCmdIgnore) {
    for (auto &ignore : ClientInfo[iToClientID].lstIgnore) {
      if (HAS_FLAG(ignore, L"p"))
        return;
    }
  }

  // Send the message to both the sender and receiver.
  FormatSendChat(iToClientID, wscSender, wscText, L"19BD3A");
  FormatSendChat(iFromClientID, wscSender, wscText, L"19BD3A");
}

/** Send a player to system message */
void SendSystemChat(uint iFromClientID, const std::wstring &wscText) {
  std::wstring wscSender =
      (const wchar_t *)Players.GetActiveCharacterName(iFromClientID);

  // Get the player's current system.
  uint iSystemID;
  pub::Player::GetSystem(iFromClientID, iSystemID);

  // For all players in system...
  struct PlayerData *pPD = 0;
  while (pPD = Players.traverse_active(pPD)) {
    uint iClientID = HkGetClientIdFromPD(pPD);
    uint iClientSystemID = 0;
    pub::Player::GetSystem(iClientID, iClientSystemID);
    if (iSystemID == iClientSystemID) {
      // Send the message a player in this system.
      FormatSendChat(iClientID, wscSender, wscText, L"E6C684");
    }
  }
}

/** Send a player to local system message */
void SendLocalSystemChat(uint iFromClientID, const std::wstring &wscText) {
  std::wstring wscSender =
      (const wchar_t *)Players.GetActiveCharacterName(iFromClientID);

  // Get the player's current system and location in the system.
  uint iSystemID;
  pub::Player::GetSystem(iFromClientID, iSystemID);

  uint iFromShip;
  pub::Player::GetShip(iFromClientID, iFromShip);

  Vector vFromShipLoc;
  Matrix mFromShipDir;
  pub::SpaceObj::GetLocation(iFromShip, vFromShipLoc, mFromShipDir);

  // For all players in system...
  struct PlayerData *pPD = 0;
  while (pPD = Players.traverse_active(pPD)) {
    // Get the this player's current system and location in the system.
    uint iClientID = HkGetClientIdFromPD(pPD);
    uint iClientSystemID = 0;
    pub::Player::GetSystem(iClientID, iClientSystemID);
    if (iSystemID != iClientSystemID)
      continue;

    uint iShip;
    pub::Player::GetShip(iClientID, iShip);

    Vector vShipLoc;
    Matrix mShipDir;
    pub::SpaceObj::GetLocation(iShip, vShipLoc, mShipDir);

    // Cheat in the distance calculation. Ignore the y-axis.
    float fDistance = sqrt(pow(vShipLoc.x - vFromShipLoc.x, 2) +
                           pow(vShipLoc.z - vFromShipLoc.z, 2));

    // Is player within scanner range (15K) of the sending char.
    if (fDistance > 14999)
      continue;

    // Send the message a player in this system.
    FormatSendChat(iClientID, wscSender, wscText, L"FF8F40");
  }
}

/** Send a player to group message */
void SendGroupChat(uint iFromClientID, const std::wstring &wscText) {
  const wchar_t *wscSender =
      (const wchar_t *)Players.GetActiveCharacterName(iFromClientID);
  // Format and send the message a player in this group.
  std::list<GROUP_MEMBER> lstMembers;
  HkGetGroupMembers(wscSender, lstMembers);
  for (auto &gm : lstMembers) {
    FormatSendChat(gm.iClientID, wscSender, wscText, L"FF7BFF");
  }
}

std::vector<HINSTANCE> vDLLs;

void HkUnloadStringDLLs() {
  for (uint i = 0; i < vDLLs.size(); i++)
    FreeLibrary(vDLLs[i]);
  vDLLs.clear();
}

void HkLoadStringDLLs() {
  HkUnloadStringDLLs();

  HINSTANCE hDLL =
      LoadLibraryEx("resources.dll", NULL,
                    LOAD_LIBRARY_AS_DATAFILE); // typically resources.dll
  if (hDLL)
    vDLLs.push_back(hDLL);

  INI_Reader ini;
  if (ini.open("freelancer.ini", false)) {
    while (ini.read_header()) {
      if (ini.is_header("Resources")) {
        while (ini.read_value()) {
          if (ini.is_value("DLL")) {
            hDLL = LoadLibraryEx(ini.get_value_string(0), NULL,
                                 LOAD_LIBRARY_AS_DATAFILE);
            if (hDLL)
              vDLLs.push_back(hDLL);
          }
        }
      }
    }
    ini.close();
  }
}

std::wstring HkGetWStringFromIDS(uint iIDS) {
  wchar_t wszBuf[1024];
  if (LoadStringW(vDLLs[iIDS >> 16], iIDS & 0xFFFF, wszBuf, 1024))
    return wszBuf;
  return L"";
}
