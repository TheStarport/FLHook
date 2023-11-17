#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/InternalApi.hpp"
#include "API/Utils/Logger.hpp"
#include "Core/ClientServerInterface.hpp"

// called when chat-text is being sent to a player, we reformat it(/set chatfont)

void __stdcall IServerImplHook::SendChat(ClientId client, ClientId clientTo, uint size, void* rdl)
{
    CallPlugins(&Plugin::OnSendChat, client, clientTo, size, rdl);

    TryHook
    {
        if (chatData->inSubmitChat && clientTo.GetValue() != 0x10004)
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

            auto& data = client.GetData();
            if (FLHookConfig::i()->userCommands.userCmdIgnore && (clientTo.GetValue() & 0xFFFF) != 0)
            {
                // check ignores
                for (const auto& ci : data.ignoreInfoList)
                {
                    if (ci.flags.find(L'p') == std::wstring::npos && clientTo.GetValue() & 0x10000)
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
                switch (data.chatSize)
                {
                    case ChatSize::Small: format = 0x90; break;
                    case ChatSize::Big: format = 0x10; break;
                    default: format = 0x00; break;
                }

                // adjust chat style
                switch (data.chatStyle)
                {
                    case ChatStyle::Bold: format += 0x01; break;
                    case ChatStyle::Italic: format += 0x02; break;
                    case ChatStyle::Underline: format += 0x04; break;
                    default: format += 0x00; break;
                }
            }

            wchar_t formatBuf[8];
            swprintf_s(formatBuf, L"%02X", format);
            std::wstring traDataFormat = formatBuf;
            std::wstring traDataColor;
            std::wstring traDataSenderColor = L"FFFFFF";
            if (FLHook::instance->messagePrivate)
            {
                traDataColor = L"19BD3A"; // pm chat color
            }
            else if (FLHook::instance->messageSystem)
            {
                traDataSenderColor = L"00FF00";
                traDataColor = L"E6C684"; // system chat color
            }
            else if (FLHook::instance->messageUniverse)
            {
                traDataSenderColor = L"00FF00";
                traDataColor = L"FFFFFF"; // universe chat color
            }
            else if (!clientTo || clientTo.GetValue() == 0x10000)
            {
                traDataColor = L"FFFFFF"; // universe chat color
            }
            else if (clientTo.GetValue() == 0x10003)
            {
                traDataColor = L"FF7BFF"; // group chat
            }
            else if (clientTo.GetValue() == 0x10002)
            {
                traDataColor = L"FF8F40"; // local chat color
            }
            else if (clientTo.GetValue() & 0x10000)
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

            InternalApi::SendMessage(client, wos.str());
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
					call [FLHook::rcSendChatMsg]
					popad
            }
        }
    }
    CatchHook({})
}
