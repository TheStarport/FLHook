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
void HkTempBan(uint client, uint iDuration);
void HkRelocateClient(uint client, Vector vDestination, Matrix mOrientation);
HK_ERROR HkInstantDock(uint client, uint iDockObj);
HK_ERROR HkFMsgEncodeMsg(const std::wstring &wscMessage, char *szBuf, uint iSize, uint &iRet);
HK_ERROR HkGetRank(const std::wstring &wscCharname, int &iRank);
HK_ERROR HkGetOnLineTime(const std::wstring &wscCharname, int &iSecs);

std::wstring GetParamToEnd(const std::wstring &wscLine, wchar_t wcSplitChar, uint iPos);
std::string GetParam(std::string scLine, char cSplitChar, uint iPos);
std::string GetUserFilePath(const std::wstring &wscCharname);
std::wstring Trim(std::wstring wscIn);
std::string Trim(std::string scIn);
int ToInt(const std::string &scStr);

std::string itohexs(uint value);

/// Print message to all ships within the specific number of meters of the player.
void PrintLocalUserCmdText(uint client, const std::wstring &wscMsg, float fDistance);

/// Return true if this player is within the specified distance of any other player.
bool IsInRange(uint client, float fDistance);

/// Return the current time as a string
std::wstring GetTimeString(bool bLocalTime);

/// Message to DSAce to change a client string.
void HkChangeIDSString(uint client, uint ids, const std::wstring &text);

/// Return the ship location in nav-map coordinates
std::wstring GetLocation(unsigned int client);

uint HkGetClientIDFromArg(const std::wstring &wscArg);

CEqObj * __stdcall HkGetEqObjFromObjRW(struct IObjRW *objRW);

void __stdcall HkLightFuse(IObjRW *ship, uint iFuseID, float fDelay, float fLifetime, float fSkip);
void __stdcall HkUnLightFuse(IObjRW *ship, uint iFuseID, float fDunno);

void HkLoadStringDLLs();
void HkUnloadStringDLLs();
std::wstring HkGetWStringFromIDS(uint iIDS);

void AddExceptionInfoLog();
#define LOG_EXCEPTION { AddLog("ERROR Exception in %s", __FUNCTION__); AddExceptionInfoLog(); }

CAccount* HkGetAccountByClientID(uint client);
std::wstring HkGetAccountIDByClientID(uint client);

void HkDelayedKick(uint client, uint secs);

float degrees( float rad );

// Convert an orientation matrix to a pitch/yaw/roll vector.  Based on what
// Freelancer does for the save game.
Vector MatrixToEuler(const Matrix& mat);
void ini_write_wstring(FILE *file, const std::string &parmname, const std::wstring &in);
void ini_get_wstring(INI_Reader &ini, std::wstring &wscValue);

void Rotate180(Matrix &rot);
void TranslateX(Vector &pos, Matrix &rot, float x);
void TranslateY(Vector &pos, Matrix &rot, float y);
void TranslateZ(Vector &pos, Matrix &rot, float z);

#endif