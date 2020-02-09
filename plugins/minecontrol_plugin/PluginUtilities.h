// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.


#ifndef __PluginUtilities_H__
#define __PluginUtilities_H__ 1


float HkDistance3D(Vector v1, Vector v2);
float HkDistance3DByShip(uint iShip1, uint iShip2);
Quaternion HkMatrixToQuaternion(Matrix m);
void HkTempBan(uint iClientID, uint iDuration);
void HkRelocateClient(uint iClientID, Vector vDestination, Matrix mOrientation);
HK_ERROR HkInstantDock(uint iClientID, uint iDockObj);
HK_ERROR HkNewCharacter(CAccount *acc, std::wstring &wscCharname);
HK_ERROR HkDeleteCharacter(CAccount *acc, std::wstring &wscCharname);
HK_ERROR HkFMsgEncodeMsg(const std::wstring &wscMessage, char *szBuf, uint iSize, uint &iRet);
HK_ERROR HkGetRank(const std::wstring &wscCharname, int &iRank);
HK_ERROR HkAntiCheat(const std::wstring &wscCharname);
HK_ERROR HkGetOnLineTime(const std::wstring &wscCharname, int &iSecs);

std::wstring GetParamToEnd(const std::wstring &wscLine, wchar_t wcSplitChar, uint iPos);
std::string GetParam(std::string scLine, char cSplitChar, uint iPos);
std::string GetUserFilePath(const std::wstring &wscCharname, const std::string &scExtension);
bool GetUserFilePath(std::string &path, const std::wstring &wscCharname, const std::string &extension);
std::wstring Trim(std::wstring wscIn);
std::string Trim(std::string scIn);
int ToInt(const std::string &scStr);

std::string itohexs(uint value);

/// Print message to all ships within the specific number of meters of the player.
void PrintLocalUserCmdText(uint iClientID, const std::wstring &wscMsg, float fDistance);

/// Return true if this player is within the specified distance of any other player.
bool IsInRange(uint iClientID, float fDistance);

/// Format a chat string in accordance with the receiver's preferences and send it. Will
/// check that the receiver accepts messages from wscSender and refuses to send if necessary.
void FormatSendChat(uint iToClientID, const std::wstring &wscSender, const std::wstring &wscText, const std::wstring &wscTextColor);

/// Send a player to player message.
void SendPrivateChat(uint iFromClientID, uint iToClientID, const std::wstring &wscText);

/// Send a player to system message.
void SendSystemChat(uint iFromClientID, const std::wstring &wscText);

/// Send a player to local system message.
void SendLocalSystemChat(uint iFromClientID, const std::wstring &wscText);

/// Send a player to group message.
void SendGroupChat(uint iFromClientID, const std::wstring &wscText);

/// Return the current time as a string
std::wstring GetTimeString();

/// Message to DSAce to change a client string.
void HkChangeIDSString(uint iClientID, uint ids, const std::wstring &text);

/// Add mounted equipment to ship.
HK_ERROR HkAddEquip(const std::wstring &wscCharname, uint iGoodID, const std::string &scHardpoint, bool bMounted);

/// Return the ship location in nav-map coordinates
std::wstring GetLocation(unsigned int iClientID);

uint HkGetClientIDFromArg(const std::wstring &wscArg);

CEqObj * __stdcall HkGetEqObjFromObjRW(struct IObjRW *objRW);

void __stdcall HkLightFuse(IObjRW *ship, uint iFuseID, float fDelay, float fLifetime, float fSkip);
void __stdcall HkUnLightFuse(IObjRW *ship, uint iFuseID, float fDunno);

void HkLoadStringDLLs();
void HkUnloadStringDLLs();
std::wstring HkGetWStringFromIDS(uint iIDS);

void AddExceptionInfoLog();
#define LOG_EXCEPTION { AddLog("ERROR Exception in %s", __FUNCTION__); AddExceptionInfoLog(); }

CAccount* HkGetAccountByClientID(uint iClientID);
std::wstring HkGetAccountIDByClientID(uint iClientID);

void HkDelayedKick(uint iClientID, uint secs);

#endif