// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <float.h>
#include <list>
#include <FLHook.h>
#include <plugin.h>
#include <FLCoreRemoteClient.h>
#include "PluginUtilities.h"

#include <Psapi.h>

#define ADDR_RMCLIENT_LAUNCH 0x5B40
#define ADDR_RMCLIENT_CLIENT 0x43D74

//Player in-system beaming
struct LAUNCH_PACKET
{
	uint iShip;
	uint iDunno[2];
	float fRotate[4];
	float fPos[3];
};

/** Calculate the distance between the two vectors */
float HkDistance3DByShip(uint iShip1, uint iShip2)
{
	Vector v1;
	Matrix m1;
	pub::SpaceObj::GetLocation(iShip1, v1, m1);
	Vector v2;
	Matrix m2;
	pub::SpaceObj::GetLocation(iShip2, v2, m2);
	float sq1 = v1.x-v2.x, sq2 = v1.y-v2.y, sq3 = v1.z-v2.z;
	return sqrt( sq1*sq1 + sq2*sq2 + sq3*sq3 );
}


/** Calculate the distance between the two vectors */
float HkDistance3D(Vector v1, Vector v2)
{
	float sq1 = v1.x-v2.x, sq2 = v1.y-v2.y, sq3 = v1.z-v2.z;
	return sqrt( sq1*sq1 + sq2*sq2 + sq3*sq3 );
}

/** Send a temp ban request to the tempban plugin */
void HkTempBan(uint iClientID, uint iDuration)
{
	// call tempban plugin
	TEMPBAN_BAN_STRUCT tempban;
	tempban.iClientID = iClientID;
	tempban.iDuration = iDuration; 
	Plugin_Communication(TEMPBAN_BAN,&tempban);
}

/** Instructs DSAce to change an IDS std::string */
void HkChangeIDSString(uint iClientID, uint ids, const std::wstring &text)
{
	DSACE_CHANGE_INFOCARD_STRUCT info;
	info.iClientID = iClientID;
	info.ids = ids; 
	info.text = text;
	Plugin_Communication(DSACE_CHANGE_INFOCARD, &info);
}

Quaternion HkMatrixToQuaternion(Matrix m)
{
	Quaternion quaternion;
	quaternion.w = sqrt( max( 0, 1 + m.data[0][0] + m.data[1][1] + m.data[2][2] ) ) / 2; 
	quaternion.x = sqrt( max( 0, 1 + m.data[0][0] - m.data[1][1] - m.data[2][2] ) ) / 2; 
	quaternion.y = sqrt( max( 0, 1 - m.data[0][0] + m.data[1][1] - m.data[2][2] ) ) / 2; 
	quaternion.z = sqrt( max( 0, 1 - m.data[0][0] - m.data[1][1] + m.data[2][2] ) ) / 2; 
	quaternion.x = (float)_copysign( quaternion.x, m.data[2][1] - m.data[1][2] );
	quaternion.y = (float)_copysign( quaternion.y, m.data[0][2] - m.data[2][0] );
	quaternion.z = (float)_copysign( quaternion.z, m.data[1][0] - m.data[0][1] );
	return quaternion;
}

/** Move the client to the specified location */
void HkRelocateClient(uint iClientID, Vector vDestination, Matrix mOrientation) 
{
	Quaternion qRotation = HkMatrixToQuaternion(mOrientation);

	FLPACKET_LAUNCH pLaunch;
	pLaunch.iShip = ClientInfo[iClientID].iShip;
	pLaunch.iBase = 0;
	pLaunch.iState = 0xFFFFFFFF;
	pLaunch.fRotate[0] = qRotation.w;
	pLaunch.fRotate[1] = qRotation.x;
	pLaunch.fRotate[2] = qRotation.y;
	pLaunch.fRotate[3] = qRotation.z;
	pLaunch.fPos[0] = vDestination.x;
	pLaunch.fPos[1] = vDestination.y;
	pLaunch.fPos[2] = vDestination.z;

	HookClient->Send_FLPACKET_SERVER_LAUNCH(iClientID, pLaunch);

	uint iSystem;
	pub::Player::GetSystem(iClientID, iSystem);
	pub::SpaceObj::Relocate(ClientInfo[iClientID].iShip, iSystem, vDestination, mOrientation);
}

/** Dock the client immediately */
HK_ERROR HkInstantDock(uint iClientID, uint iDockObj)
{
	// check if logged in
	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	uint iShip;
	pub::Player::GetShip(iClientID, iShip);
	if(!iShip)
		return HKE_PLAYER_NOT_IN_SPACE;

	uint iSystem, iSystem2;
	pub::SpaceObj::GetSystem(iShip, iSystem);
	pub::SpaceObj::GetSystem(iDockObj, iSystem2);
	if(iSystem!=iSystem2)
	{
		return HKE_PLAYER_NOT_IN_SPACE;
	}

	try {
		pub::SpaceObj::InstantDock(iShip, iDockObj, 1);
	} catch(...) { return HKE_PLAYER_NOT_IN_SPACE; }

	return HKE_OK;
}

HK_ERROR HkGetRank(const std::wstring &wscCharname, int &iRank)
{
	HK_ERROR err;
	std::wstring wscRet = L"";
	if ((err = HkFLIniGet(wscCharname, L"rank", wscRet)) != HKE_OK)
		return err;
	if (wscRet.length())
		iRank = ToInt(wscRet);
	else
		iRank = 0;
	return HKE_OK;
}

/// Get online time.
HK_ERROR HkGetOnLineTime(const std::wstring &wscCharname, int &iSecs)
{
	std::wstring wscDir;
	if(!HKHKSUCCESS(HkGetAccountDirName(wscCharname, wscDir)))
		return HKE_CHAR_DOES_NOT_EXIST;

	std::wstring wscFile;
	HkGetCharFileName(wscCharname, wscFile);

	std::string scCharFile  = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
	if (HkIsEncoded(scCharFile))
	{
		std::string scCharFileNew = scCharFile + ".ini";
		if (!flc_decode(scCharFile.c_str(), scCharFileNew.c_str()))
			return HKE_COULD_NOT_DECODE_CHARFILE;

		iSecs = (int)IniGetF(scCharFileNew, "mPlayer", "total_time_played", 0.0f);
		DeleteFile(scCharFileNew.c_str());
	}
	else
	{
		iSecs = (int)IniGetF(scCharFile, "mPlayer", "total_time_played", 0.0f);
	}

	return HKE_OK;
}

/**
This function is similar to GetParam but instead returns everything
from the parameter specified by iPos to the end of wscLine.

wscLine - the std::string to get parameters from
wcSplitChar - the seperator character
iPos - the parameter number to start from.
*/
std::wstring GetParamToEnd(const std::wstring &wscLine, wchar_t wcSplitChar, uint iPos)
{
	for(uint i = 0, j = 0; (i <= iPos) && (j < wscLine.length()); j++)
	{
		if(wscLine[j] == wcSplitChar)
		{
			while(((j + 1) < wscLine.length()) && (wscLine[j+1] == wcSplitChar))
				j++; // skip "whitechar"
			i++;
			continue;
		}
		if	(i == iPos)
		{
			return wscLine.substr(j);
		}
	}
	return L"";
}


std::string GetParam(std::string scLine, char cSplitChar, uint iPos)
{
	uint i = 0, j = 0;

	std::string scResult = "";
	for(i = 0, j = 0; (i <= iPos) && (j < scLine.length()); j++)
	{
		if(scLine[j] == cSplitChar)
		{
			while(((j + 1) < scLine.length()) && (scLine[j+1] == cSplitChar))
				j++; // skip "whitechar"

			i++;
			continue;
		}

		if(i == iPos)
			scResult += scLine[j];
	}

	return scResult;
}


/**
Determine the path name of a file in the charname account directory with the
provided extension. The resulting path is returned in the path parameter.
*/
std::string GetUserFilePath(const std::wstring &wscCharname, const std::string &scExtension)
{
	// init variables
	char szDataPath[MAX_PATH];
	GetUserDataPath(szDataPath);
	std::string scAcctPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\";

	std::wstring wscDir;
	std::wstring wscFile;
	if (HkGetAccountDirName(wscCharname, wscDir)!=HKE_OK)
		return "";
	if (HkGetCharFileName(wscCharname, wscFile)!=HKE_OK)
		return "";

	return scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + scExtension;
}

/** This function is not exported by FLHook so we include it here */
HK_ERROR HkFMsgEncodeMsg(const std::wstring &wscMessage, char *szBuf, uint iSize, uint &iRet)
{
	XMLReader rdr;
	RenderDisplayList rdl;
	std::wstring wscMsg = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?><RDL><PUSH/>";
	wscMsg += L"<TRA data=\"0xE6C68400\" mask=\"-1\"/><TEXT>" + XMLText(wscMessage) + L"</TEXT>";
	wscMsg += L"<PARA/><POP/></RDL>\x000A\x000A";
	if(!rdr.read_buffer(rdl, (const char*)wscMsg.c_str(), (uint)wscMsg.length() * 2))
		return HKE_WRONG_XML_SYNTAX;

	BinaryRDLWriter	rdlwrite;
	rdlwrite.write_buffer(rdl, szBuf, iSize, iRet);

	return HKE_OK;
}

int ToInt(const std::string &scStr)
{
	return atoi(scStr.c_str());
}

std::string itohexs(uint value)
{
	char buf[16];
	sprintf_s(buf, "%08X", value);
	return buf;
}

/// Print message to all ships within the specific number of meters of the player.
void PrintLocalUserCmdText(uint iClientID, const std::wstring &wscMsg, float fDistance)
{
	uint iShip;
	pub::Player::GetShip(iClientID, iShip);

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(iShip, pos, rot);

	uint iSystem;
	pub::Player::GetSystem(iClientID, iSystem);

	// For all players in system...
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		// Get the this player's current system and location in the system.
		uint iClientID2 = HkGetClientIdFromPD(pPD);
		uint iSystem2 = 0;
		pub::Player::GetSystem(iClientID2, iSystem2);
		if (iSystem != iSystem2)
			continue;

		uint iShip2;
		pub::Player::GetShip(iClientID2, iShip2);

		Vector pos2;
		Matrix rot2;
		pub::SpaceObj::GetLocation(iShip2, pos2, rot2);

		// Is player within the specified range of the sending char.
		if (HkDistance3D(pos, pos2) > fDistance)
			continue;

		PrintUserCmdText(iClientID2, wscMsg);
	}
}


/// Return true if this player is within the specified distance of any other player.
bool IsInRange(uint iClientID, float fDistance)
{
	std::list<GROUP_MEMBER> lstMembers;
	HkGetGroupMembers((const wchar_t*) Players.GetActiveCharacterName(iClientID), lstMembers);

	uint iShip;
	pub::Player::GetShip(iClientID, iShip);

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(iShip, pos, rot);

	uint iSystem;
	pub::Player::GetSystem(iClientID, iSystem);

	// For all players in system...
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		// Get the this player's current system and location in the system.
		uint iClientID2 = HkGetClientIdFromPD(pPD);
		uint iSystem2 = 0;
		pub::Player::GetSystem(iClientID2, iSystem2);
		if (iSystem != iSystem2)
			continue;

		uint iShip2;
		pub::Player::GetShip(iClientID2, iShip2);

		Vector pos2;
		Matrix rot2;
		pub::SpaceObj::GetLocation(iShip2, pos2, rot2);

		// Ignore players who are in your group.
		bool bGrouped = false;
		for(auto& gm : lstMembers)
		{
			if (gm.iClientID == iClientID2)
			{
				bGrouped = true;
				break;
			}
		}
		if (bGrouped)
			continue;

		// Is player within the specified range of the sending char.
		if (HkDistance3D(pos, pos2) < fDistance)
			return true;
	}
	return false;
}

/**
Remove leading and trailing spaces from the std::string ~FlakCommon by Motah.
*/
std::wstring Trim(std::wstring wscIn)
{
	while(wscIn.length() && (wscIn[0]==L' ' || wscIn[0]==L'	' || wscIn[0]==L'\n' || wscIn[0]==L'\r') )
	{
		wscIn = wscIn.substr(1);
	}
	while(wscIn.length() && (wscIn[wscIn.length()-1]==L' ' || wscIn[wscIn.length()-1]==L'	' || wscIn[wscIn.length()-1]==L'\n' || wscIn[wscIn.length()-1]==L'\r') )
	{
		wscIn = wscIn.substr(0, wscIn.length()-1);
	}
	return wscIn;
}

/**
Remove leading and trailing spaces from the std::string  ~FlakCommon by Motah.
*/
std::string Trim(std::string scIn)
{
	while(scIn.length() && (scIn[0]==' ' || scIn[0]=='	' || scIn[0]=='\n' || scIn[0]=='\r') )
	{
		scIn = scIn.substr(1);
	}
	while(scIn.length() && (scIn[scIn.length()-1]==L' ' || scIn[scIn.length()-1]=='	' || scIn[scIn.length()-1]=='\n' || scIn[scIn.length()-1]=='\r') )
	{
		scIn = scIn.substr(0, scIn.length()-1);
	}
	return scIn;
}

/**
Delete a character.
*/
HK_ERROR HkDeleteCharacter(CAccount *acc, std::wstring &wscCharname)
{
	HkLockAccountAccess(acc, true);
	st6::wstring str((ushort*)wscCharname.c_str());
	Players.DeleteCharacterFromName(str);
	HkUnlockAccountAccess(acc);
	return HKE_OK; 
}

/**
Create a new character in the specified account by emulating a 
create character.
*/
HK_ERROR HkNewCharacter(CAccount *acc, std::wstring &wscCharname)
{
	HkLockAccountAccess(acc, true);
	HkUnlockAccountAccess(acc);

	INI_Reader ini;
	if (!ini.open("..\\DATA\\CHARACTERS\\newcharacter.ini", false))
		return HKE_MPNEWCHARACTERFILE_NOT_FOUND_OR_INVALID;

	// Emulate char create by logging in.
	SLoginInfo logindata;
	wcsncpy_s(logindata.wszAccount, HkGetAccountID(acc).c_str(), 36);
	Players.login(logindata, Players.GetMaxPlayerCount()+1);

	SCreateCharacterInfo newcharinfo;
	wcsncpy_s(newcharinfo.wszCharname, wscCharname.c_str(), 23);
	newcharinfo.wszCharname[23]=0;

	newcharinfo.iNickName = 0;
	newcharinfo.iBase = 0;
	newcharinfo.iPackage = 0;
	newcharinfo.iPilot = 0;

	while (ini.read_header())
	{
		if(ini.is_header("Faction"))
		{
			while(ini.read_value())
			{
				if(ini.is_value("nickname"))
					newcharinfo.iNickName = CreateID(ini.get_value_string());
				else if(ini.is_value("base"))
					newcharinfo.iBase = CreateID(ini.get_value_string());
				else if(ini.is_value("package"))
					newcharinfo.iPackage = CreateID(ini.get_value_string());
				else if(ini.is_value("pilot"))
					newcharinfo.iPilot = CreateID(ini.get_value_string());
			}
			break;
		}
	}
	ini.close();

	if(newcharinfo.iNickName == 0)
		newcharinfo.iNickName = CreateID("new_player"); 
	if(newcharinfo.iBase == 0)
		newcharinfo.iBase = CreateID("Li01_01_Base");
	if(newcharinfo.iPackage == 0)
		newcharinfo.iPackage = CreateID("ge_fighter");
	if(newcharinfo.iPilot == 0)
		newcharinfo.iPilot = CreateID("trent");

	// Fill struct with valid data (though it isnt used it is needed)
	newcharinfo.iDunno[4] = 65536;
	newcharinfo.iDunno[5] = 65538;
	newcharinfo.iDunno[6] = 0;
	newcharinfo.iDunno[7] = 1058642330;
	newcharinfo.iDunno[8] = 3206125978;
	newcharinfo.iDunno[9] = 65537;
	newcharinfo.iDunno[10] = 0;
	newcharinfo.iDunno[11] = 3206125978;
	newcharinfo.iDunno[12] = 65539;
	newcharinfo.iDunno[13] = 65540;
	newcharinfo.iDunno[14] = 65536;
	newcharinfo.iDunno[15] = 65538;
	Server.CreateNewCharacter(newcharinfo, Players.GetMaxPlayerCount()+1);
	HkSaveChar(wscCharname);
	Players.logout(Players.GetMaxPlayerCount()+1);
	return HKE_OK;
}


/**
Determine the path name of a file in the charname account directory with the
provided extension. The resulting path is returned in the path parameter.
*/
bool GetUserFilePath(std::string &path, const std::wstring &wscCharname, const std::string &extension)
{
	// init variables
	char szDataPath[MAX_PATH];
	GetUserDataPath(szDataPath);
	std::string scAcctPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\";

	std::wstring wscDir;
	std::wstring wscFile;
	if (HkGetAccountDirName(wscCharname, wscDir)!=HKE_OK)
		return false;
	if (HkGetCharFileName(wscCharname, wscFile)!=HKE_OK)
		return false;
	path = scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + extension;
	return true;
}

typedef void (__stdcall *_FLAntiCheat)();
typedef void (__stdcall *_FLPossibleCheatingDetected)(int iReason);

/** Anti cheat checking code by mc_horst */
HK_ERROR HkAntiCheat(const std::wstring &wscCharname)
{
#define ADDR_FL_ANTICHEAT_1 0x70120
#define ADDR_FL_ANTICHEAT_2 0x6FD20
#define ADDR_FL_ANTICHEAT_3 0x6FAF0
#define ADDR_FL_ANTICHEAT_4 0x6FAA0
#define ADDR_FL_POSSIBLE_CHEATING_DETECTED 0x6F570

	_FLAntiCheat FLAntiCheat1 = (_FLAntiCheat) ((char*)hModServer + ADDR_FL_ANTICHEAT_1);
	_FLAntiCheat FLAntiCheat2 = (_FLAntiCheat) ((char*)hModServer + ADDR_FL_ANTICHEAT_2);
	_FLAntiCheat FLAntiCheat3 = (_FLAntiCheat) ((char*)hModServer + ADDR_FL_ANTICHEAT_3);
	_FLAntiCheat FLAntiCheat4 = (_FLAntiCheat) ((char*)hModServer + ADDR_FL_ANTICHEAT_4);
	_FLPossibleCheatingDetected FLPossibleCheatingDetected = (_FLPossibleCheatingDetected) ((char*)hModServer + ADDR_FL_POSSIBLE_CHEATING_DETECTED);

	HK_GET_CLIENTID(iClientID, wscCharname);

	// check if logged in
	if(iClientID == -1)
		return HKE_OK;

	// check if ship in space
	uint iShip = 0;
	pub::Player::GetShip(iClientID, iShip);
	if(iShip)
		return HKE_OK;

	char *szObjPtr;
	memcpy(&szObjPtr, &Players, 4);
	szObjPtr += 0x418 * (iClientID - 1);

	char cRes;

	////////////////////////// 1
	__asm
	{
		mov ecx, [szObjPtr]
		call [FLAntiCheat1]
		mov [cRes], al
	}

	if(cRes != 0)
	{ // kick
		HkKick(ARG_CLIENTID(iClientID));
		return HKE_UNKNOWN_ERROR;
	}

	////////////////////////// 2
	__asm
	{
		mov ecx, [szObjPtr]
		call [FLAntiCheat2]
		mov [cRes], al
	}

	if(cRes != 0)
	{ // kick
		HkKick(ARG_CLIENTID(iClientID));
		return HKE_UNKNOWN_ERROR;
	}

	////////////////////////// 3
	ulong lRet;
	ulong lCompare;
	__asm
	{
		mov ecx, [szObjPtr]
		mov eax, [ecx+0x320]
		mov [lCompare], eax
		call [FLAntiCheat3]
		mov [lRet], eax
	}

	if(lRet > lCompare)
	{ // kick
		HkKick(ARG_CLIENTID(iClientID));
		return HKE_UNKNOWN_ERROR;
	}

	////////////////////////// 4
	__asm
	{
		mov ecx, [szObjPtr]
		call [FLAntiCheat4]
		mov [cRes], al
	}

	if(cRes != 0)
	{ // kick
		HkKick(ARG_CLIENTID(iClientID));
		return HKE_UNKNOWN_ERROR;
	}

	return HKE_OK;
}



/** Format a chat std::string in accordance with the receiver's preferences and send it. Will
check that the receiver accepts messages from wscSender and refuses to send if
necessary. */
void FormatSendChat(uint iToClientID, const std::wstring &wscSender, const std::wstring &wscText, const std::wstring &wscTextColor)
{
#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

	if (set_bUserCmdIgnore)
	{
		for(auto& ignore : ClientInfo[iToClientID].lstIgnore)
		{
			if(!HAS_FLAG(ignore, L"i") && !(ToLower(wscSender).compare(ToLower(ignore.wscCharname))))
				return; // ignored
			else if(HAS_FLAG(ignore, L"i") && (ToLower(wscSender).find(ToLower(ignore.wscCharname)) != -1))
				return; // ignored
		}
	}

	uchar cFormat;
	// adjust chatsize
	switch(ClientInfo[iToClientID].chatSize)
	{
	case CS_SMALL: cFormat = 0x90; break;
	case CS_BIG: cFormat = 0x10; break;
	default: cFormat = 0x00; break;
	}

	// adjust chatstyle
	switch(ClientInfo[iToClientID].chatStyle)
	{
	case CST_BOLD: cFormat += 0x01; break;
	case CST_ITALIC: cFormat += 0x02; break;
	case CST_UNDERLINE: cFormat += 0x04; break;
	default: cFormat += 0x00; break;
	}

	wchar_t wszFormatBuf[8];
	swprintf(wszFormatBuf, _countof(wszFormatBuf), L"%02X", (long)cFormat);
	std::wstring wscTRADataFormat = wszFormatBuf;
	const std::wstring wscTRADataSenderColor = L"FFFFFF"; // white

	std::wstring wscXML = L"<TRA data=\"0x" + wscTRADataSenderColor + wscTRADataFormat + L"\" mask=\"-1\"/><TEXT>" + XMLText(wscSender) + L": </TEXT>" + 
		L"<TRA data=\"0x" + wscTextColor + wscTRADataFormat + L"\" mask=\"-1\"/><TEXT>" + XMLText(wscText) + L"</TEXT>";

	HkFMsg(iToClientID, wscXML);
}

/** Send a player to player message */
void SendPrivateChat(uint iFromClientID, uint iToClientID, const std::wstring &wscText)
{
	std::wstring wscSender = (const wchar_t*) Players.GetActiveCharacterName(iFromClientID);

	if (set_bUserCmdIgnore)
	{
		for(auto& ignore : ClientInfo[iToClientID].lstIgnore)
		{
			if (HAS_FLAG(ignore, L"p"))
				return;
		}
	}

	// Send the message to both the sender and receiver.
	FormatSendChat(iToClientID, wscSender, wscText, L"19BD3A");
	FormatSendChat(iFromClientID, wscSender, wscText, L"19BD3A");
}

/** Send a player to system message */
void SendSystemChat(uint iFromClientID, const std::wstring &wscText)
{
	std::wstring wscSender = (const wchar_t*) Players.GetActiveCharacterName(iFromClientID);

	// Get the player's current system.
	uint iSystemID;
	pub::Player::GetSystem(iFromClientID, iSystemID);

	// For all players in system...
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);
		uint iClientSystemID = 0;
		pub::Player::GetSystem(iClientID, iClientSystemID);
		if (iSystemID == iClientSystemID)
		{
			// Send the message a player in this system.
			FormatSendChat(iClientID, wscSender, wscText, L"E6C684");
		}
	}
}


/** Send a player to local system message */
void SendLocalSystemChat(uint iFromClientID, const std::wstring &wscText)
{
	std::wstring wscSender = (const wchar_t*) Players.GetActiveCharacterName(iFromClientID);

	// Get the player's current system and location in the system.
	uint iSystemID;
	pub::Player::GetSystem(iFromClientID, iSystemID);

	uint iFromShip;
	pub::Player::GetShip(iFromClientID, iFromShip);

	Vector vFromShipLoc;
	Matrix mFromShipDir;
	pub::SpaceObj::GetLocation(iFromShip, vFromShipLoc, mFromShipDir);

	// For all players in system...
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		// Get the this player's current system and location in the system.
		uint iClientID = HkGetClientIdFromPD(pPD);
		uint iClientSystemID = 0;
		pub::Player::GetSystem(iClientID, iClientSystemID);
		if (iSystemID != iClientSystemID)
			continue;

		uint iShip;
		pub::Player::GetShip(iClientID, iShip);

		Vector vShipLoc;
		Matrix mShipDir;
		pub::SpaceObj::GetLocation(iShip, vShipLoc, mShipDir);

		// Cheat in the distance calculation. Ignore the y-axis.
		float fDistance = sqrt(pow(vShipLoc.x - vFromShipLoc.x,2) + pow(vShipLoc.z - vFromShipLoc.z,2));

		// Is player within scanner range (15K) of the sending char.
		if (fDistance>14999)
			continue;

		// Send the message a player in this system.
		FormatSendChat(iClientID, wscSender, wscText, L"FF8F40");
	}
}

/** Send a player to group message */
void SendGroupChat(uint iFromClientID, const std::wstring &wscText)
{
	const wchar_t *wscSender = (const wchar_t*) Players.GetActiveCharacterName(iFromClientID);

	// Format and send the message a player in this group.
	std::list<GROUP_MEMBER> lstMembers;
	HkGetGroupMembers(wscSender, lstMembers);
	for(auto& gm : lstMembers)
	{
		FormatSendChat(gm.iClientID, wscSender, wscText, L"FF7BFF");
	}
}

std::wstring GetTimeString()
{
	SYSTEMTIME st;
	GetSystemTime(&st);

	wchar_t wszBuf[100];
	_snwprintf_s(wszBuf, sizeof(wszBuf), L"%04d-%02d-%02d %02d:%02d:%02d SMT ", st.wYear, st.wMonth, st.wDay, 
		st.wHour, st.wMinute, st.wSecond);
	return wszBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////


HK_ERROR HkAddEquip(const std::wstring &wscCharname, uint iGoodID, const std::string &scHardpoint, bool bMounted)
{
	typedef bool (__stdcall *_AddCargoDocked)(uint iGoodID, CacheString *&hardpoint, int iNumItems, float fHealth, int bMounted, int bMission, uint iOne);
	static _AddCargoDocked AddCargoDocked = 0;
	if (!AddCargoDocked)
		AddCargoDocked = (_AddCargoDocked)((char*)hModServer +  0x6EFC0);

	HK_GET_CLIENTID(iClientID, wscCharname);
	if (iClientID == -1 || HkIsInCharSelectMenu(iClientID))
		return HKE_PLAYER_NOT_LOGGED_IN;

	uint iBase = 0;
	pub::Player::GetBase(iClientID, iBase);
	uint iLocation = 0;
	pub::Player::GetLocation(iClientID, iLocation);

	if (iLocation)
		Server.LocationExit(iLocation,iClientID);
	if (iBase)
		Server.BaseExit(iBase,iClientID);
	if (!HkIsValidClientID(iClientID))
		return HKE_PLAYER_NOT_LOGGED_IN;

	PlayerData *pd = &Players[iClientID];
	const char *p = scHardpoint.c_str();
	CacheString hardpoint;
	hardpoint.value = StringAlloc(p, false);

	int iOne = 1;
	int iMounted = bMounted;
	float fHealth = 1;
	CacheString *pHP = &hardpoint;
	__asm
	{
		push iOne
		push iMounted
		push iOne
		push fHealth
		push iOne
		push pHP
		push iGoodID
		mov ecx, pd
		call AddCargoDocked
	}

	if(iBase)
		Server.BaseEnter(iBase, iClientID);
	if(iLocation)
		Server.LocationEnter(iLocation, iClientID);

	return HKE_OK;
}

std::wstring GetLocation(unsigned int iClientID)
{
	uint iSystemID = 0;
	uint iShip = 0;
	pub::Player::GetSystem(iClientID, iSystemID);
	pub::Player::GetShip(iClientID, iShip);
	if (!iSystemID || !iShip)
	{
		PrintUserCmdText(iClientID, L"ERR Not in space");
		return false;
	}

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(iShip, pos, rot);

	float scale = 1.0;
	const Universe::ISystem *iSystem = Universe::get_system(iSystemID);
	if (iSystem)
		scale = iSystem->NavMapScale;

	float fGridsize = 34000.0f / scale;
	int gridRefX = (int)((pos.x + (fGridsize * 5)) / fGridsize) - 1;
	int gridRefZ = (int)((pos.z + (fGridsize * 5)) / fGridsize) - 1;

	std::wstring wscXPos = L"X";
	if (gridRefX >= 0 && gridRefX < 8)
	{
		wchar_t* gridXLabel[] = {L"A", L"B", L"C", L"D", L"E", L"F", L"G", L"H"};
		wscXPos = gridXLabel[gridRefX];
	}

	std::wstring wscZPos = L"X";
	if (gridRefZ >= 0 && gridRefZ < 8)
	{
		wchar_t* gridZLabel[] = {L"1", L"2", L"3", L"4", L"5", L"6", L"7", L"8"};
		wscZPos = gridZLabel[gridRefZ];
	}

	wchar_t wszCurrentLocation[100];
	_snwprintf_s(wszCurrentLocation, sizeof(wszCurrentLocation), L"%s-%s", wscXPos.c_str(), wscZPos.c_str());
	return wszCurrentLocation;
}

uint HkGetClientIDFromArg(const std::wstring &wscArg)
{
	uint iClientID;

	if (HkResolveId(wscArg, iClientID) == HKE_OK)
		return iClientID;

	if (HkResolveShortCut(wscArg, iClientID) == HKE_OK)
		return iClientID;

	return HkGetClientIdFromCharname(wscArg);
}


/// Return the CEqObj from the IObjRW
__declspec(naked) CEqObj * __stdcall HkGetEqObjFromObjRW(struct IObjRW *objRW)
{
   __asm
   {
      push ecx
      push edx
      mov ecx, [esp+12]
      mov edx, [ecx]
      call dword ptr[edx+0x150]
      pop edx
      pop ecx
      ret 4
   }
}

__declspec(naked) void __stdcall HkLightFuse(IObjRW *ship, uint iFuseID, float fDelay, float fLifetime, float fSkip)
{
	__asm
	{
		lea eax, [esp+8] //iFuseID
		push [esp+20] //fSkip
		push [esp+16] //fDelay
		push 0 //SUBOBJ_ID_NONE
		push eax
		push [esp+32] //fLifetime
		mov ecx, [esp+24]
		mov eax, [ecx]
		call [eax+0x1E4]
		ret 20
	}
}

__declspec(naked) void __stdcall HkUnLightFuse(IObjRW *ship, uint iFuseID, float fDunno)
{
	__asm
	{
		mov ecx, [esp+4]
		lea eax, [esp+8] //iFuseID
		push [esp+12] //fDunno
		push 0 //SUBOBJ_ID_NONE
		push eax //iFuseID
		mov eax, [ecx]
		call [eax+0x1E8]
		ret 12
	}
}


std::vector<HINSTANCE> vDLLs;

void HkLoadStringDLLs()
{
	HkUnloadStringDLLs();

	HINSTANCE hDLL = LoadLibraryEx("resources.dll", NULL, LOAD_LIBRARY_AS_DATAFILE); //typically resources.dll
	if(hDLL)
		vDLLs.push_back(hDLL);
	
	INI_Reader ini;
	if (ini.open("freelancer.ini", false))
	{
		while (ini.read_header())
		{
			if (ini.is_header("Resources"))
			{
				while (ini.read_value())
				{
					if (ini.is_value("DLL"))
					{
						hDLL = LoadLibraryEx(ini.get_value_string(0), NULL, LOAD_LIBRARY_AS_DATAFILE);
						if (hDLL)
							vDLLs.push_back(hDLL);
					}
				}
			}
		}
		ini.close();
	}
}

void HkUnloadStringDLLs()
{
	for (uint i=0; i<vDLLs.size(); i++)
		FreeLibrary(vDLLs[i]);
	vDLLs.clear();
}

std::wstring HkGetWStringFromIDS(uint iIDS)
{
	wchar_t wszBuf[1024];
	if (LoadStringW(vDLLs[iIDS >> 16], iIDS & 0xFFFF, wszBuf, 1024))
		return wszBuf;
	return L"";
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HMODULE GetModuleAddr(uint iAddr)
{
	HMODULE hModArr[1024];
	DWORD iArrSizeNeeded;
	HANDLE hProcess = GetCurrentProcess();
	if(EnumProcessModules(hProcess, hModArr, sizeof(hModArr), &iArrSizeNeeded))
	{
		if(iArrSizeNeeded > sizeof(hModArr))
			iArrSizeNeeded = sizeof(hModArr);
		iArrSizeNeeded /= sizeof(HMODULE);
		for(uint i = 0; i < iArrSizeNeeded; i++)
		{
			MODULEINFO mi;
			if(GetModuleInformation(hProcess, hModArr[i], &mi, sizeof(mi)))
			{
				if(((uint)mi.lpBaseOfDll) < iAddr && (uint)mi.lpBaseOfDll + (uint)mi.SizeOfImage > iAddr)
				{
					return hModArr[i];
				}
			}
		}
	}
	return 0;
}


CAccount* HkGetAccountByClientID(uint iClientID)
{
	if (!HkIsValidClientID(iClientID))
		return 0;

	return Players.FindAccountFromClientID(iClientID);
}

std::wstring HkGetAccountIDByClientID(uint iClientID)
{
	if (HkIsValidClientID(iClientID))
	{
		CAccount *acc = HkGetAccountByClientID(iClientID);
		if (acc && acc->wszAccID)
		{
			wchar_t buf[35];
			wcsncpy_s(buf, 35, acc->wszAccID, 35); 
			return buf;
		}
	}
	return L"";
}

void HkDelayedKick(uint iClientID, uint secs)
{
	mstime kick_time = timeInMS() + (secs * 1000);
	if (!ClientInfo[iClientID].tmKickTime || ClientInfo[iClientID].tmKickTime > kick_time)
		ClientInfo[iClientID].tmKickTime = kick_time;
}