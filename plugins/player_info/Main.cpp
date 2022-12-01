// Player Info - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include <FLHook.hpp>
#include <plugin.h>

// Global vars
ReturnCode returncode = ReturnCode::Default;

#define POPUPDIALOG_BUTTONS_LEFT_YES    1
#define POPUPDIALOG_BUTTONS_CENTER_NO   2
#define POPUPDIALOG_BUTTONS_RIGHT_LATER 4
#define POPUPDIALOG_BUTTONS_CENTER_OK   8

#define RSRCID_PLAYERINFO_TITLE 500000
#define RSRCID_PLAYERINFO_TEXT  RSRCID_PLAYERINFO_TITLE + 1
#define MAX_PARAGRAPHS          5
#define MAX_CHARACTERS          1000

static std::wstring IniGetLongWS(
    const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::wstring& wscDefault)
{
	char szRet[0x10000];
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), "", szRet, sizeof(szRet), scFile.c_str());
	std::string scValue = szRet;
	if (!scValue.length())
		return wscDefault;

	std::wstring wscValue = L"";
	long lHiByte;
	long lLoByte;
	while (sscanf_s(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
	{
		scValue = scValue.substr(4);
		wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
		wscValue.append(1, wChar);
	}

	return wscValue;
}

static int CurrLength(const std::string& scFilePath)
{
	int iCount = 0;
	for (int i = 1; i <= MAX_PARAGRAPHS; i++)
	{
		iCount += IniGetLongWS(scFilePath, "Info", std::to_string(i), L"").length();
	}
	return iCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_ShowInfo(const uint& iClientID, const std::wstring_view& wscParam)
{
	const wchar_t* wszTargetName = 0;
	const std::wstring& wscCommand = GetParam(wscParam, ' ', 0);
	if (wscCommand == L"me")
	{
		wszTargetName = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
	}
	else
	{
		uint iShip;
		pub::Player::GetShip(iClientID, iShip);

		uint iTargetShip;
		pub::SpaceObj::GetTarget(iShip, iTargetShip);

		uint iTargetClientID = GetClientIDByShip(iTargetShip);
		if (IsValidClientID(iTargetClientID))
			wszTargetName = (const wchar_t*)Players.GetActiveCharacterName(iTargetClientID);
	}

	if (!wszTargetName)
	{
		PrintUserCmdText(iClientID, L"ERR No target");
		return;
	}

	std::string scFilePath = GetUserFilePath(wszTargetName, "-info.ini");
	std::wstring wscPlayerInfo = L"<RDL><PUSH/>";
	for (int i = 1; i <= MAX_PARAGRAPHS; i++)
	{
		std::wstring wscXML = IniGetLongWS(scFilePath, "Info", std::to_string(i), L"");
		if (wscXML.length())
			wscPlayerInfo += L"<TEXT>" + wscXML + L"</TEXT><PARA/><PARA/>";
	}
	std::wstring wscXML = IniGetLongWS(scFilePath, "Info", "AdminNote", L"");
	if (wscXML.length())
		wscPlayerInfo += L"<TEXT>" + wscXML + L"</TEXT><PARA/><PARA/>";
	wscPlayerInfo += L"<POP/></RDL>";

	if (wscPlayerInfo.length() < 30)
	{
		PrintUserCmdText(iClientID, L"ERR No information available");
		return;
	}

	ChangeIDSString(iClientID, RSRCID_PLAYERINFO_TITLE, wszTargetName);
	ChangeIDSString(iClientID, RSRCID_PLAYERINFO_TEXT, wscPlayerInfo);

	FmtStr caption(0, 0);
	caption.begin_mad_lib(RSRCID_PLAYERINFO_TITLE);
	caption.end_mad_lib();

	FmtStr message(0, 0);
	message.begin_mad_lib(RSRCID_PLAYERINFO_TEXT);
	message.end_mad_lib();

	pub::Player::PopUpDialog(iClientID, caption, message, POPUPDIALOG_BUTTONS_CENTER_OK);
}

void UserCmd_SetInfo(const uint& iClientID, const std::wstring_view& wscParam)
{
	uint iPara = ToInt(GetParam(wscParam, ' ', 0));
	const std::wstring& wscCommand = GetParam(wscParam, ' ', 1);
	const std::wstring& wscMsg = GetParamToEnd(wscParam, ' ', 2);

	std::string scFilePath = GetUserFilePath((const wchar_t*)Players.GetActiveCharacterName(iClientID), "-info.ini");
	if (scFilePath.length() == 0)
		return;

	if (iPara > 0 && iPara <= MAX_PARAGRAPHS && wscCommand == L"a")
	{
		int length = CurrLength(scFilePath) + wscMsg.length();
		if (length > MAX_CHARACTERS)
		{
			PrintUserCmdText(iClientID, L"ERR Too many characters. Limit is %d", MAX_CHARACTERS);
			return;
		}

		std::wstring wscNewMsg = IniGetLongWS(scFilePath, "Info", std::to_string(iPara), L"") + XMLText(wscMsg);
		IniWriteW(scFilePath, "Info", std::to_string(iPara), wscNewMsg);
		PrintUserCmdText(iClientID, L"OK %d/%d characters used", length, MAX_CHARACTERS);
	}
	else if (iPara > 0 && iPara <= MAX_PARAGRAPHS && wscCommand == L"d")
	{
		IniWriteW(scFilePath, "Info", std::to_string(iPara), L"");
		PrintUserCmdText(iClientID, L"OK");
	}
	else
	{
		PrintUserCmdText(iClientID, L"ERR Invalid parameters");
		PrintUserCmdText(iClientID, L"/setinfo <paragraph> <command> <text>");
		PrintUserCmdText(iClientID, L"|  <paragraph> The paragraph number in the range 1-%d", MAX_PARAGRAPHS);
		PrintUserCmdText(
		    iClientID,
		    L"|  <command> The command to perform on the "
		    L"paragraph, 'a' for append, 'd' for delete");
	}
}

// Client command processing
USERCMD UserCmds[] = {
	{ L"/showinfo", UserCmd_ShowInfo },
	{ L"/si", UserCmd_ShowInfo },
	{ L"/setinfo", UserCmd_SetInfo },
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

DefaultDllMain()

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Player Info by Cannon");
	pi->shortName("player_info");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
}
