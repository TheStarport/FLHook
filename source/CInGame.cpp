#include "CInGame.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInGame::DoPrint(const std::string& text)
{
	std::wstring str = stows(text);
	wchar_t wszBufSend[1024] = L"";
	for (uint i = 0; (i <= str.length()); i++)
	{
		if (str[i] == '\n' || str[i] == '\0')
		{
			if (!wcslen(wszBufSend))
				break;
			wszBufSend[wcslen(wszBufSend)] = '\0';
			std::wstring wscXML =
			    std::wstring(L"<TRA data=\"" + FLHookConfig::i()->msgStyle.adminCmdStyle + L"\" mask=\"-1\"/><TEXT>") + XMLText(wszBufSend) + L"</TEXT>";
			Hk::Message::FMsg(this->client, wscXML);
			memset(wszBufSend, 0, sizeof(wszBufSend));
		}
		else
			wszBufSend[wcslen(wszBufSend)] = str[i];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInGame::ReadRights(const std::string& scAdminFile)
{
	rights = RIGHT_NOTHING;
	std::string scRights = IniGetS(scAdminFile, "admin", "rights", "");

	SetRightsByString(scRights);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CInGame::GetAdminName()
{
	return wscAdminName;
}
