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

#include <io.h>
#include <shlwapi.h>

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
float HkDistance3D(Vector v1, Vector v2)
{
	float sq1 = v1.x-v2.x, sq2 = v1.y-v2.y, sq3 = v1.z-v2.z;
	return sqrt( sq1*sq1 + sq2*sq2 + sq3*sq3 );
}

/** Send a temp ban request to the tempban plugin */
void HkTempBan(uint client, uint iDuration)
{
	// call tempban plugin
	TEMPBAN_BAN_STRUCT tempban;
	tempban.iClientID = client;
	tempban.iDuration = iDuration; 
	Plugin_Communication(TEMPBAN_BAN,&tempban);
}

/** Instructs DSAce to change an IDS std::string */
void HkChangeIDSString(uint client, uint ids, const std::wstring &text)
{
	DSACE_CHANGE_INFOCARD_STRUCT info;
	info.iClientID = client;
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
void HkRelocateClient(uint client, Vector vDestination, Matrix mOrientation) 
{
	Quaternion qRotation = HkMatrixToQuaternion(mOrientation);

	FLPACKET_LAUNCH pLaunch;
	pLaunch.iShip = ClientInfo[client].iShip;
	pLaunch.iBase = 0;
	pLaunch.iState = 0xFFFFFFFF;
	pLaunch.fRotate[0] = qRotation.w;
	pLaunch.fRotate[1] = qRotation.x;
	pLaunch.fRotate[2] = qRotation.y;
	pLaunch.fRotate[3] = qRotation.z;
	pLaunch.fPos[0] = vDestination.x;
	pLaunch.fPos[1] = vDestination.y;
	pLaunch.fPos[2] = vDestination.z;

	HookClient->Send_FLPACKET_SERVER_LAUNCH(client, pLaunch);

	uint iSystem;
	pub::Player::GetSystem(client, iSystem);
	pub::SpaceObj::Relocate(ClientInfo[client].iShip, iSystem, vDestination, mOrientation);
}

/** Dock the client immediately */
HK_ERROR HkInstantDock(uint client, uint iDockObj)
{
	// check if logged in
	if(client == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	uint iShip;
	pub::Player::GetShip(client, iShip);
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

HK_ERROR HkGetRank(const std::wstring &charname, int &iRank)
{
	HK_ERROR err;
	std::wstring wscRet = L"";
	if ((err = HkFLIniGet(charname, L"rank", wscRet)) != HKE_OK)
		return err;
	if (wscRet.length())
		iRank = ToInt(wscRet);
	else
		iRank = 0;
	return HKE_OK;
}

/// Get online time.
HK_ERROR HkGetOnLineTime(const std::wstring &charname, int &iSecs)
{
	std::wstring wscDir;
	if(!HKHKSUCCESS(HkGetAccountDirName(charname, wscDir)))
		return HKE_CHAR_DOES_NOT_EXIST;

	std::wstring wscFile;
	HkGetCharFileName(charname, wscFile);

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
std::string GetUserFilePath(const std::wstring &charname)
{
	// init variables
	char datapath[MAX_PATH];
	GetUserDataPath(datapath);
	std::string scAcctPath = std::string(datapath) + "\\Accts\\MultiPlayer\\";

	std::wstring wscDir;
	std::wstring wscFile;
	if (HkGetAccountDirName(charname, wscDir)!=HKE_OK)
		return "";
	if (HkGetCharFileName(charname, wscFile)!=HKE_OK)
		return "";

	return scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + ".fl";
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
	sprintf(buf, "%08X", value);
	return buf;
}

/// Print message to all ships within the specific number of meters of the player.
void PrintLocalUserCmdText(uint client, const std::wstring &wscMsg, float fDistance)
{
	uint iShip;
	pub::Player::GetShip(client, iShip);

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(iShip, pos, rot);

	uint iSystem;
	pub::Player::GetSystem(client, iSystem);

	// For all players in system...
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		// Get the this player's current system and location in the system.
		uint client2 = HkGetClientIdFromPD(pPD);
		uint iSystem2 = 0;
		pub::Player::GetSystem(client2, iSystem2);
		if (iSystem != iSystem2)
			continue;

		uint iShip2;
		pub::Player::GetShip(client2, iShip2);

		Vector pos2;
		Matrix rot2;
		pub::SpaceObj::GetLocation(iShip2, pos2, rot2);

		// Is player within the specified range of the sending char.
		if (HkDistance3D(pos, pos2) > fDistance)
			continue;

		PrintUserCmdText(client2, L"%s", wscMsg.c_str());
	}
}


/// Return true if this player is within the specified distance of any other player.
bool IsInRange(uint client, float fDistance)
{
	std::list<GROUP_MEMBER> lstMembers;
	HkGetGroupMembers((const wchar_t*) Players.GetActiveCharacterName(client), lstMembers);

	uint iShip;
	pub::Player::GetShip(client, iShip);

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(iShip, pos, rot);

	uint iSystem;
	pub::Player::GetSystem(client, iSystem);

	// For all players in system...
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		// Get the this player's current system and location in the system.
		uint client2 = HkGetClientIdFromPD(pPD);
		uint iSystem2 = 0;
		pub::Player::GetSystem(client2, iSystem2);
		if (iSystem != iSystem2)
			continue;

		uint iShip2;
		pub::Player::GetShip(client2, iShip2);

		Vector pos2;
		Matrix rot2;
		pub::SpaceObj::GetLocation(iShip2, pos2, rot2);

		// Ignore players who are in your group.
		bool bGrouped = false;
		for(auto& gm : lstMembers)
		{
			if (gm.iClientID==client2)
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

std::wstring GetTimeString(bool bLocalTime)
{
	SYSTEMTIME st;
	if (bLocalTime)
		GetLocalTime(&st);
	else
		GetSystemTime(&st);

	wchar_t wszBuf[100];
	_snwprintf_s(wszBuf, sizeof(wszBuf), L"%04d-%02d-%02d %02d:%02d:%02d SMT ", st.wYear, st.wMonth, st.wDay, 
		st.wHour, st.wMinute, st.wSecond);
	return wszBuf;
}

std::wstring GetLocation(unsigned int client)
{
	uint iSystemID = 0;
	uint iShip = 0;
	pub::Player::GetSystem(client, iSystemID);
	pub::Player::GetShip(client, iShip);
	if (!iSystemID || !iShip)
	{
		PrintUserCmdText(client, L"ERR Not in space");
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
	_snwprintf(wszCurrentLocation, sizeof(wszCurrentLocation), L"%s-%s", wscXPos.c_str(), wscZPos.c_str());
	return wszCurrentLocation;
}

uint HkGetClientIDFromArg(const std::wstring &wscArg)
{
	uint client;

	if (HkResolveId(wscArg, client) == HKE_OK)
		return client;

	if (HkResolveShortCut(wscArg, client) == HKE_OK)
		return client;

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

CAccount* HkGetAccountByClientID(uint client)
{
	if (!HkIsValidClientID(client))
		return 0;

	return Players.FindAccountFromClientID(client);
}

std::wstring HkGetAccountIDByClientID(uint client)
{
	if (HkIsValidClientID(client))
	{
		CAccount *acc = HkGetAccountByClientID(client);
		if (acc && acc->wszAccID)
		{
			wchar_t buf[35];
			wcsncpy_s(buf, 35, acc->wszAccID, 35); 
			return buf;
		}
	}
	return L"";
}

void HkDelayedKick(uint client, uint secs)
{
	mstime kick_time = timeInMS() + (secs * 1000);
	if (!ClientInfo[client].tmKickTime || ClientInfo[client].tmKickTime > kick_time)
		ClientInfo[client].tmKickTime = kick_time;
}


#define PI 3.14159265f

// Convert radians to degrees.
float degrees( float rad )
{
  rad *= 180 / PI;

  // Prevent displaying -0 and prefer 180 to -180.
  if (rad < 0)
  {
    if (rad > -0.005f)
      rad = 0;
    else if (rad <= -179.995f)
      rad = 180;
  }

  // Round to two decimal places here, so %g can display it without decimals.
  float frac = modff( rad * 100, &rad );
  if (frac >= 0.5f)
    ++rad;
  else if (frac <= -0.5f)
    --rad;

  return rad / 100;
}

// Convert an orientation matrix to a pitch/yaw/roll vector.  Based on what
// Freelancer does for the save game.
Vector MatrixToEuler(const Matrix& mat)
{
	Vector x = { mat.data[0][0], mat.data[1][0], mat.data[2][0] };
	Vector y = { mat.data[0][1], mat.data[1][1], mat.data[2][1] };
	Vector z = { mat.data[0][2], mat.data[1][2], mat.data[2][2] };

	Vector vec;
	float h = (float)_hypot( x.x, x.y );
	if (h > 1/524288.0f)
	{
		vec.x = degrees( atan2f(  y.z, z.z ) );
		vec.y = degrees( atan2f( -x.z, h   ) );
		vec.z = degrees( atan2f(  x.y, x.x ) );
	}
	else
	{
		vec.x = degrees( atan2f( -z.y, y.y ) );
		vec.y = degrees( atan2f( -x.z, h   ) );
		vec.z = 0;
	}
	return vec;
}

void ini_write_wstring(FILE *file, const std::string &parmname, const std::wstring &in)
{
	fprintf(file, "%s=", parmname.c_str()); 
	for (int i = 0; i < (int)in.size(); i++)
	{
		UINT v1 = in[i] >> 8;
		UINT v2 = in[i] & 0xFF;
		fprintf(file, "%02x%02x", v1, v2); 
	}
	fprintf(file, "\n");
}


void ini_get_wstring(INI_Reader &ini, std::wstring &wscValue)
{
	std::string scValue = ini.get_value_string();
	wscValue = L"";
	long lHiByte;
	long lLoByte;
	while(sscanf(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
	{
		scValue = scValue.substr(4);
		wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
		wscValue.append(1, wChar);
	}
}


void Rotate180(Matrix &rot)
{
	rot.data[0][0] = -rot.data[0][0];
	rot.data[1][0] = -rot.data[1][0];
	rot.data[2][0] = -rot.data[2][0];
	rot.data[0][2] = -rot.data[0][2];
	rot.data[1][2] = -rot.data[1][2];
	rot.data[2][2] = -rot.data[2][2];
}

void TranslateY(Vector &pos, Matrix &rot, float y)
{
	pos.x += y * rot.data[0][0];
	pos.y += y * rot.data[1][0];
	pos.z += y * rot.data[2][0];
}

void TranslateX(Vector &pos, Matrix &rot, float x)
{
	pos.x += x * rot.data[0][2];
	pos.y += x * rot.data[1][2];
	pos.z += x * rot.data[2][2];
}

void TranslateZ(Vector &pos, Matrix &rot, float z)
{
	pos.x += z * rot.data[0][1];
	pos.y += z * rot.data[1][1];
	pos.z += z * rot.data[2][1];
}