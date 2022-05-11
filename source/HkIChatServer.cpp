#include "hook.h"

/**************************************************************************************************************
called when chat-text is being sent to a player, we reformat it(/set chatfont)
**************************************************************************************************************/

#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

void __stdcall SendChat(uint clientID, uint clientIDTo, uint size, void *rdl) {
    CallPluginsBefore(HookedCall::IChat__SendChat, clientID, clientIDTo, size, rdl);

    TRY_HOOK {
        if (HkIServerImpl::g_InSubmitChat && (clientIDTo != 0x10004)) {
            wchar_t wszBuf[1024] = L"";
            // extract text from rdlReader
            BinaryRDLReader r;
            uint iRet;
            r.extract_text_from_buffer((unsigned short *)wszBuf,
                                         sizeof(wszBuf), iRet,
                                       (const char *)rdl, size);

            std::wstring buffer = wszBuf;
            std::wstring sender = buffer.substr(
                0, buffer.length() - HkIServerImpl::g_TextLength - 2);
            std::wstring text =
                buffer.substr(buffer.length() - HkIServerImpl::g_TextLength);

            if (set_bUserCmdIgnore && ((clientIDTo & 0xFFFF) != 0)) { // check ignores
                for (auto& ci : ClientInfo[clientID].lstIgnore) {
                    if (HAS_FLAG(ci, L"p") && (clientIDTo & 0x10000))
                        continue; // no privchat
                    else if (!HAS_FLAG(ci, L"i") && !(ToLower(sender).compare(ToLower(ci.wscCharname))))
                        return; // ignored
                    else if (HAS_FLAG(ci, L"i") && (ToLower(sender).find(ToLower(ci.wscCharname)) != -1))
                        return; // ignored
                }
            }

            uchar format = 0x00;
            if (set_bUserCmdSetChatFont) {
                // adjust chat size
                switch (ClientInfo[clientID].chatSize) {
                case CS_SMALL:
                    format = 0x90;
                    break;

                case CS_BIG:
                    format = 0x10;
                    break;

                default:
                    format = 0x00;
                    break;
                }

                // adjust chat style
                switch (ClientInfo[clientID].chatStyle) {
                case CST_BOLD:
                    format += 0x01;
                    break;

                case CST_ITALIC:
                    format += 0x02;
                    break;

                case CST_UNDERLINE:
                    format += 0x04;
                    break;

                default:
                    format += 0x00;
                    break;
                }
            }

            wchar_t formatBuf[8];
            swprintf_s(formatBuf, L"%02X", (long)format);
            std::wstring traDataFormat = formatBuf;
            std::wstring traDataColor;
            std::wstring traDataSenderColor = L"FFFFFF";
            if (g_bMsg) {
                traDataColor = L"19BD3A"; // pm chat color
            } else if (g_bMsgS) {
                traDataSenderColor = L"00FF00";
                traDataColor = L"E6C684"; // system chat color
            } else if (g_bMsgU) {
                traDataSenderColor = L"00FF00";
                traDataColor = L"FFFFFF"; // universe chat color
            } else if (clientIDTo == 0x10000)
                traDataColor = L"FFFFFF"; // universe chat color
            else if (clientIDTo == 0)
                traDataColor = L"FFFFFF"; // console
            else if (clientIDTo == 0x10003)
                traDataColor = L"FF7BFF"; // group chat
            else if (clientIDTo == 0x10002)
                traDataColor = L"FF8F40"; // local chat color
            else if (clientIDTo & 0x10000)
                traDataColor = L"E6C684"; // system chat color
            else
                traDataColor = L"19BD3A"; // pm chat color

            std::wstringstream wos;
            wos << L"<TRA data=\"0x" << traDataSenderColor + traDataFormat
                << L"\" mask=\"-1\"/><TEXT>" << XMLText(sender) << L": </TEXT>"
                << L"<TRA data =\"0x" << traDataColor + traDataFormat << L"\" mask=\"-1\"/>"
                << "<TEXT>" << XMLText(text) + L"</TEXT>";
            
            HkFMsg(clientID, wos.str());
        } else {
            uint sz = size;
            __asm {
                pushad
                push [rdl]
                push [sz]
                push [clientIDTo]
                push [clientID]
                mov ecx, [Client]
                add ecx, 4
                call [RCSendChatMsg]
                popad
            }
        }
    }
    CATCH_HOOK({})
}
