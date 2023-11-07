#include "PCH.hpp"

#include "API/FLServer/Chat.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

// called when chat-text is being sent to a player, we reformat it(/set chatfont)

void __stdcall IServerImplHook::SendChat(ClientId client, ClientId clientTo, uint size, void* rdl)
{
    CallPlugins(&Plugin::OnSendChat, client, clientTo, size, rdl);

    TryHook
    {
        if (chatData->inSubmitChat && clientTo != 0x10004)
        {
            std::wstring buffer;
            buffer.resize(size);
            // extract text from rdlReader
            BinaryRDLReader r;
            uint retVal;
            r.extract_text_from_buffer((unsigned short*)buffer.data(), buffer.size(), retVal, static_cast<const char*>(rdl), size);
            std::erase(buffer, '\0');

            const std::wstring sender = chatData->characterName;
            const int spaceAfterColonOffset = buffer[sender.length() + 1] == ' ' ? sender.length() + 2 : 0;
            const std::wstring text = buffer.substr(spaceAfterColonOffset, buffer.length() - spaceAfterColonOffset);

            if (FLHookConfig::i()->userCommands.userCmdIgnore && (clientTo & 0xFFFF) != 0)
            {
                // check ignores
                for (const auto& ci : ClientInfo::At(client).ignoreInfoList)
                {
                    if (ci.flags.find(L'p') == std::wstring::npos && clientTo & 0x10000)
                    {
                        continue; // no privchat
                    }
                    else if (ci.flags.find(L'i') == std::wstring::npos && StringUtils::ToLower(sender) != (StringUtils::ToLower(ci.character)))
                    {
                        return; // ignored
                    }
                    else if (ci.flags.find(L'i') != std::wstring::npos && StringUtils::ToLower(sender).find(StringUtils::ToLower(ci.character)) != -1)
                    {
                        return; // ignored
                    }
                }
            }

            uchar format = 0x00;
            if (FLHookConfig::i()->userCommands.userCmdSetChatFont)
            {
                // adjust chat size
                switch (ClientInfo::At(client).chatSize)
                {
                    case CS_SMALL: format = 0x90; break;
                    case CS_BIG: format = 0x10; break;
                    default: format = 0x00; break;
                }

                // adjust chat style
                switch (ClientInfo::At(client).chatStyle)
                {
                    case CST_BOLD: format += 0x01; break;
                    case CST_ITALIC: format += 0x02; break;
                    case CST_UNDERLINE: format += 0x04; break;
                    default: format += 0x00; break;
                }
            }

            wchar_t formatBuf[8];
            swprintf_s(formatBuf, L"%02X", format);
            std::wstring traDataFormat = formatBuf;
            std::wstring traDataColor;
            std::wstring traDataSenderColor = L"FFFFFF";
            if (CoreGlobals::c()->messagePrivate)
            {
                traDataColor = L"19BD3A"; // pm chat color
            }
            else if (CoreGlobals::c()->messageSystem)
            {
                traDataSenderColor = L"00FF00";
                traDataColor = L"E6C684"; // system chat color
            }
            else if (CoreGlobals::c()->messageUniverse)
            {
                traDataSenderColor = L"00FF00";
                traDataColor = L"FFFFFF"; // universe chat color
            }
            else if (!clientTo || clientTo == 0x10000)
            {
                traDataColor = L"FFFFFF"; // universe chat color
            }
            else if (clientTo == 0x10003)
            {
                traDataColor = L"FF7BFF"; // group chat
            }
            else if (clientTo == 0x10002)
            {
                traDataColor = L"FF8F40"; // local chat color
            }
            else if (clientTo & 0x10000)
            {
                traDataColor = L"E6C684"; // system chat color
            }
            else
            {
                traDataColor = L"19BD3A"; // pm chat color
            }

            std::wstringstream wos;
            wos << L"<TRA data=\"0x" << traDataSenderColor + traDataFormat << L"\" mask=\"-1\"/><TEXT>" << StringUtils::XmlText(sender) << L": </TEXT>"
                << L"<TRA data =\"0x" << traDataColor + traDataFormat << L"\" mask=\"-1\"/>" << "<TEXT>" << StringUtils::XmlText(text) + L"</TEXT>";

            Hk::Chat::FMsg(client, wos.str());
        }
        else
        {
            uint packetSize = size;
            __asm {
					pushad
					push [rdl]
					push [packetSize]
					push [clientTo]
					push [client]
					mov ecx, [Client]
					add ecx, 4
					call [RCSendChatMsg]
					popad
            }
        }
    }
    CatchHook({})
}
