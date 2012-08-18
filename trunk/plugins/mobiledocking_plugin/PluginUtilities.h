// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.


#ifndef __PluginUtilities_H__
#define __PluginUtilities_H__ 1

float HkDistance3D(Vector v1, Vector v2);
Quaternion HkMatrixToQuaternion(Matrix m);
void HkTempBan(uint iClientID, uint iDuration);
void HkRelocateClient(uint iClientID, Vector vDestination, Matrix mOrientation);
HK_ERROR HkInstantDock(uint iClientID, uint iDockObj);
HK_ERROR HkNewCharacter(CAccount *acc, wstring &wscCharname);
HK_ERROR HkDeleteCharacter(CAccount *acc, wstring &wscCharname);
HK_ERROR HkFMsgEncodeMsg(const wstring &wscMessage, char *szBuf, uint iSize, uint &iRet);
HK_ERROR HkGetRank(const wstring &wscCharname, int &iRank);
HK_ERROR HkAntiCheat(UINT iClientID);
HK_ERROR HkGetOnLineTime(const wstring &wscCharname, int &iSecs);

wstring GetParamToEnd(const wstring &wscLine, wchar_t wcSplitChar, uint iPos);
string GetParam(string scLine, char cSplitChar, uint iPos);
string GetUserFilePath(const wstring &wscCharname, const string &scExtension);
bool GetUserFilePath(string &path, const wstring &wscCharname, const string &extension);
wstring Trim(wstring wscIn);
string Trim(string scIn);
int ToInt(const string &scStr);

string itohexs(uint value);

/// Print message to all ships within the specific number of meters of the player.
void PrintLocalUserCmdText(uint iClientID, const wstring &wscMsg, float fDistance);

/// Return true if this player is within the specified distance of any other player.
bool IsInRange(uint iClientID, float fDistance);

/// Format a chat string in accordance with the receiver's preferences and send it. Will
/// check that the receiver accepts messages from wscSender and refuses to send if necessary.
void FormatSendChat(uint iToClientID, const wstring &wscSender, const wstring &wscText, const wstring &wscTextColor);

/// Send a player to player message.
void SendPrivateChat(uint iFromClientID, uint iToClientID, const wstring &wscText);

/// Send a player to system message.
void SendSystemChat(uint iFromClientID, const wstring &wscText);

/// Send a player to local system message.
void SendLocalSystemChat(uint iFromClientID, const wstring &wscText);

/// Send a player to group message.
void SendGroupChat(uint iFromClientID, const wstring &wscText);

/// Return the current time as a string
wstring GetTimeString(bool bLocalTime);

/// Message to DSAce to change a client string.
void HkChangeIDSString(uint iClientID, uint ids, const wstring &text);

/// Add mounted equipment to ship.
HK_ERROR HkAddEquip(const wstring &wscCharname, uint iGoodID, const string &scHardpoint);

/// Return the ship location in nav-map coordinates
wstring GetLocation(unsigned int iClientID);

uint HkGetClientIDFromArg(const wstring &wscArg);

CEqObj * __stdcall HkGetEqObjFromObjRW(struct IObjRW *objRW);

void __stdcall HkLightFuse(IObjRW *ship, uint iFuseID, float fDelay, float fLifetime, float fSkip);
void __stdcall HkUnLightFuse(IObjRW *ship, uint iFuseID, float fDunno);

void HkLoadStringDLLs();
void HkUnloadStringDLLs();
wstring HkGetWStringFromIDS(uint iIDS);

void AddExceptionInfoLog();
#define LOG_EXCEPTION { AddLog("ERROR Exception in %s", __FUNCTION__); AddExceptionInfoLog(); }

CAccount* HkGetAccountByClientID(uint iClientID);
wstring HkGetAccountIDByClientID(uint iClientID);

void HkDelayedKick(uint iClientID, uint secs);

string HkGetPlayerSystemS(uint iClientID);
float HkDistance3DByShip(uint iShip1, uint iShip2);

#endif