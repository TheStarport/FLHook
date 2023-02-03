#include "Global.hpp"

/**************************************************************************************************************
called when chat-text is being sent to a player, we reformat it(/set chatfont)
**************************************************************************************************************/

#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

void __stdcall SendChat(ClientId client, ClientId clientTo, uint size, void* rdl)
{
	CallPluginsBefore(HookedCall::IChat__SendChat, client, clientTo, size, rdl);

	TRY_HOOK
	{
		if (IServerImplHook::g_InSubmitChat && (clientTo != 0x10004))
		{
			wchar_t wszBuf[1024] = L"";
			// extract text from rdlReader
			BinaryRDLReader r;
			uint iRet;
			r.extract_text_from_buffer((unsigned short*)wszBuf, sizeof(wszBuf), iRet, (const char*)rdl, size);

			std::wstring buffer = wszBuf;
			std::wstring sender = buffer.substr(0, buffer.length() - IServerImplHook::g_TextLength - 2);
			std::wstring text = buffer.substr(buffer.length() - IServerImplHook::g_TextLength);

			if (FLHookConfig::i()->userCommands.userCmdIgnore && ((clientTo & 0xFFFF) != 0))
			{ // check ignores
				for (auto& ci : ClientInfo[client].lstIgnore)
				{
					if (HAS_FLAG(ci, L"p") && (clientTo & 0x10000))
						continue; // no privchat
					else if (!HAS_FLAG(ci, L"i") && !(ToLower(sender).compare(ToLower(ci.character))))
						return; // ignored
					else if (HAS_FLAG(ci, L"i") && (ToLower(sender).find(ToLower(ci.character)) != -1))
						return; // ignored
				}
			}

			uchar format = 0x00;
			if (FLHookConfig::i()->userCommands.userCmdSetChatFont)
			{
				// adjust chat size
				switch (ClientInfo[client].chatSize)
				{
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
				switch (ClientInfo[client].chatStyle)
				{
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
			else if (clientTo == 0x10000)
				traDataColor = L"FFFFFF"; // universe chat color
			else if (clientTo == 0)
				traDataColor = L"FFFFFF"; // console
			else if (clientTo == 0x10003)
				traDataColor = L"FF7BFF"; // group chat
			else if (clientTo == 0x10002)
				traDataColor = L"FF8F40"; // local chat color
			else if (clientTo & 0x10000)
				traDataColor = L"E6C684"; // system chat color
			else
				traDataColor = L"19BD3A"; // pm chat color

			std::wstringstream wos;
			wos << L"<TRA data=\"0x" << traDataSenderColor + traDataFormat << L"\" mask=\"-1\"/><TEXT>" << XMLText(sender) << L": </TEXT>" << L"<TRA data =\"0x"
			    << traDataColor + traDataFormat << L"\" mask=\"-1\"/>" << "<TEXT>" << XMLText(text) + L"</TEXT>";

			Hk::Message::FMsg(client, wos.str());
		}
		else
		{
			uint sz = size;
			__asm {
                pushad
                push [rdl]
                push [sz]
                push [clientTo]
                push [client]
                mov ecx, [Client]
                add ecx, 4
                call [RCSendChatMsg]
                popad
			}
		}
	}
	CATCH_HOOK({})
}
