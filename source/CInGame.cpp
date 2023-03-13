#include "CInGame.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInGame::DoPrint(const std::string& text)
{
	const std::wstring str = stows(text);
	wchar_t wBufSend[1024] = L"";
	for (uint i = 0; (i <= str.length()); i++)
	{
		if (str[i] == '\n' || str[i] == '\0')
		{
			if (!wcslen(wBufSend))
				break;
			wBufSend[wcslen(wBufSend)] = '\0';
			std::wstring XML = std::wstring(L"<TRA data=\"" + FLHookConfig::i()->messages.msgStyle.adminCmdStyle + L"\" mask=\"-1\"/><TEXT>") +
				XMLText(wBufSend) + L"</TEXT>";
			Hk::Message::FMsg(this->client, XML);
			memset(wBufSend, 0, sizeof(wBufSend));
		}
		else
			wBufSend[wcslen(wBufSend)] = str[i];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInGame::ReadRights(const std::string& adminFile)
{
	const std::string rights = IniGetS(adminFile, "admin", "rights", "");

	SetRightsByString(rights);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CInGame::GetAdminName()
{
	return AdminName;
}
