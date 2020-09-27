#include "hook.h"

/**************************************************************************************************************
called when chat-text is being sent to a player, we reformat it(/set chatfont)
**************************************************************************************************************/

#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

void __stdcall HkCb_SendChat(uint iClientID, uint iTo, uint iSize, void *pRDL) {

  CALL_PLUGINS_V(PLUGIN_HkCb_SendChat, __stdcall, (uint, uint, uint, void *),
                 (iClientID, iTo, iSize, pRDL));

  TRY_HOOK {
    if (HkIServerImpl::g_bInSubmitChat && (iTo != 0x10004)) {
      wchar_t wszBuf[1024] = L"";
      // extract text from rdlReader
      BinaryRDLReader rdl;
      uint iRet;
      rdl.extract_text_from_buffer((unsigned short *)wszBuf, sizeof(wszBuf),
                                   iRet, (const char *)pRDL, iSize);

      std::wstring wscBuf = wszBuf;
      std::wstring wscSender =
          wscBuf.substr(0, wscBuf.length() - HkIServerImpl::g_iTextLen - 2);
      std::wstring wscText =
          wscBuf.substr(wscBuf.length() - HkIServerImpl::g_iTextLen);

      if (set_bUserCmdIgnore && ((iTo & 0xFFFF) != 0)) { // check ignores
        for (auto &ci : ClientInfo[iClientID].lstIgnore) {
          if (HAS_FLAG(ci, L"p") && (iTo & 0x10000))
            continue; // no privchat
          else if (!HAS_FLAG(ci, L"i") &&
                   !(ToLower(wscSender).compare(ToLower(ci.wscCharname))))
            return; // ignored
          else if (HAS_FLAG(ci, L"i") &&
                   (ToLower(wscSender).find(ToLower(ci.wscCharname)) != -1))
            return; // ignored
        }
      }

      uchar cFormat = 0x00;
      if (set_bUserCmdSetChatFont) {
        // adjust chatsize
        switch (ClientInfo[iClientID].chatSize) {
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
        switch (ClientInfo[iClientID].chatStyle) {
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
      }

      wchar_t wszFormatBuf[8];
      swprintf_s(wszFormatBuf, L"%02X", (long)cFormat);
      std::wstring wscTRADataFormat = wszFormatBuf;
      std::wstring wscTRADataColor;
      std::wstring wscTRADataSenderColor = L"FFFFFF";
      if (g_bMsg) {
        //				wscTRADataSenderColor = L"00FF00";
        wscTRADataColor = L"19BD3A"; // pm chatcolor
      } else if (g_bMsgS) {
        wscTRADataSenderColor = L"00FF00";
        wscTRADataColor = L"E6C684"; // system chatcolor
      } else if (g_bMsgU) {
        wscTRADataSenderColor = L"00FF00";
        wscTRADataColor = L"FFFFFF"; // universe chatcolor
      } else if (iTo == 0x10000)
        wscTRADataColor = L"FFFFFF"; // universe chatcolor
      else if (iTo == 0)
        wscTRADataColor = L"FFFFFF"; // console
      else if (iTo == 0x10003)
        wscTRADataColor = L"FF7BFF"; // group chat
      else if (iTo == 0x10002)
        wscTRADataColor = L"FF8F40"; // local chatcolor
      else if (iTo & 0x10000)
        wscTRADataColor = L"E6C684"; // system chatcolor
      else
        wscTRADataColor = L"19BD3A"; // pm chatcolor

      std::wstring wscXML =
          L"<TRA data=\"0x" + wscTRADataSenderColor + wscTRADataFormat +
          L"\" mask=\"-1\"/><TEXT>" + XMLText(wscSender) + L": </TEXT>" +
          L"<TRA data=\"0x" + wscTRADataColor + wscTRADataFormat +
          L"\" mask=\"-1\"/><TEXT>" + XMLText(wscText) + L"</TEXT>";
      HkFMsg(iClientID, wscXML);
    } else {
      __asm {
            pushad
            push [pRDL]
            push [iSize]
            push [iTo]
            push [iClientID]
            mov ecx, [Client]
            add ecx, 4
            call [RCSendChatMsg]
            popad
      }
    }
  }
  CATCH_HOOK({})
}
