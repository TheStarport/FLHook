#include "CInGame.h"

#include "Hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInGame::DoPrint(const std::wstring& wscText)
{
	wchar_t wszBufSend[1024] = L"";
	for (uint i = 0; (i <= wscText.length()); i++)
	{
		if (wscText[i] == '\n' || wscText[i] == '\0')
		{
			if (!wcslen(wszBufSend))
				break;
			wszBufSend[wcslen(wszBufSend)] = '\0';
			std::wstring wscXML = std::wstring(L"<TRA data=\"" + FLHookConfig::i()->msgStyle.adminCmdStyle + L"\" mask=\"-1\"/><TEXT>") +
			    XMLText(wszBufSend) + L"</TEXT>";
			HkFMsg(this->iClientID, wscXML);
			memset(wszBufSend, 0, sizeof(wszBufSend));
		}
		else
			wszBufSend[wcslen(wszBufSend)] = wscText[i];
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
