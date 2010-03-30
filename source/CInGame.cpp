#include "CInGame.h"
#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInGame::DoPrint(wstring wscText)
{
	wchar_t wszBufSend[1024] = L"";
	for(uint i = 0; (i <= wscText.length()); i++)
	{
		if(wscText[i] == '\n' || wscText[i] == '\0') {
			if(!wcslen(wszBufSend))
				break;
			wszBufSend[wcslen(wszBufSend)] = '\0';
			wstring wscXML = wstring(L"<TRA data=\"" + set_wscAdminCmdStyle + L"\" mask=\"-1\"/><TEXT>") + XMLText(wszBufSend) + L"</TEXT>";
			HkFMsg(this->iClientID, wscXML);
			memset(wszBufSend, 0, sizeof(wszBufSend));
		} else 
			wszBufSend[wcslen(wszBufSend)] = wscText[i];
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CInGame::ReadRights(string scAdminFile)
{
	rights = RIGHT_NOTHING;
	string scRights = IniGetS(scAdminFile, "admin", "rights", "");

	SetRightsByString(scRights);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

wstring CInGame::GetAdminName()
{
	return wscAdminName;
}
