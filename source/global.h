#ifndef _GLOBAL_
#define _GLOBAL_
#pragma warning(disable: 4311 4786)

// includes 
#include "blowfish.h"
#include <windows.h>
#include <stdio.h>
#include <string>
#include <list>
using namespace std;

// defines
#define VERSION L"1.6.1-RC1 plugin"

#define TIME_UPDATE 50
#define IMPORT __declspec(dllimport)
#define EXPORT __declspec(dllexport)
#define foreach(lst, type, var) for(list<type>::iterator var = lst.begin(); (var != lst.end()); var++)
#define foreachreverse(lst, type, var) for(list<type>::reverse_iterator var = lst.rbegin(); (var != lst.rend()); var++)

#define IMPORT __declspec(dllimport)
#define EXPORT __declspec(dllexport)

// typedefs
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned __int64 mstime;

// custom fl wstring (vc6 strings)
typedef class std::basic_string<unsigned short,struct ci_wchar_traits> flstr;
typedef class std::basic_string<char,struct ci_char_traits> flstrs;

typedef flstr* (*_CreateWString)(const wchar_t *wszStr);
typedef void (*_FreeWString)(flstr *wscStr);
typedef flstrs* (*_CreateString)(const char *szStr);
typedef void (*_FreeString)(flstrs *scStr);
typedef char* (*_GetCString)(flstrs *scStr);
typedef wchar_t* (*_GetWCString)(flstr *wscStr);
typedef wchar_t* (*_WStringAssign)(flstr *wscStr, const wchar_t *wszStr);
typedef wchar_t* (*_WStringAppend)(flstr *wscStr, const wchar_t *wszStr);

// structures
struct INISECTIONVALUE
{
	string scKey;
	string scValue;
};

struct MULTIKILLMESSAGE
{
	uint iKillsInARow;
	wstring wscMessage;
};

// functions
bool FLHookInit();
void FLHookInit_Pre();
void FLHookShutdown();
EXPORT void ProcessEvent(wstring wscText, ...);
void LoadSettings();
void ProcessPendingCommands();

// tools
EXPORT wstring stows(const string &scText);
EXPORT string wstos(const wstring &wscText);
EXPORT string itos(int i);
EXPORT string IniGetS(const string &scFile, const string &scApp, const string &scKey, const string &scDefault);
EXPORT int IniGetI(const string &scFile, const string &scApp, const string &scKey, int iDefault);
EXPORT bool IniGetB(const string &scFile, const string &scApp, const string &scKey, bool bDefault);
EXPORT void IniWrite(const string &scFile, const string &scApp, const string &scKey, const string &scValue);
EXPORT void WriteProcMem(void *pAddress, void *pMem, int iSize);
EXPORT void ReadProcMem(void *pAddress, void *pMem, int iSize);
EXPORT wstring ToLower(const wstring &wscStr);
EXPORT string ToLower(const string &scStr);
EXPORT int ToInt(const wstring &wscStr);
EXPORT void ConPrint(wstring wscText, ...);
EXPORT wstring XMLText(const wstring &wscText);
EXPORT wstring GetParam(const wstring &wscLine, wchar_t wcSplitChar, uint iPos);
EXPORT wstring ReplaceStr(const wstring &wscSource, const wstring &wscSearchFor, const wstring &wscReplaceWith);
EXPORT void IniDelSection(const string &scFile, const string &scApp);
EXPORT void IniDelete(const string &scFile, const string &scApp, const string &scKey);
EXPORT void IniWriteW(const string &scFile, const string &scApp, const string &scKey, const wstring &wscValue);
EXPORT wstring IniGetWS(const string &scFile, const string &scApp, const string &scKey, const wstring &wscDefault);
EXPORT wstring ToMoneyStr(int iCash);
EXPORT float IniGetF(const string &scFile, const string &scApp, const string &scKey, float fDefault);
EXPORT void IniGetSection(const string &scFile, const string &scApp, list<INISECTIONVALUE> &lstValues);
EXPORT float ToFloat(const wstring &wscStr);
EXPORT mstime timeInMS();
EXPORT void SwapBytes(void *ptr, uint iLen);
EXPORT FARPROC PatchCallAddr(char *hMod, DWORD dwInstallAddress, char *dwHookFunction);

// variables
extern EXPORT HANDLE hProcFL;
extern EXPORT HMODULE hModServer;
extern EXPORT HMODULE hModCommon;
extern EXPORT HMODULE hModRemoteClient;
extern EXPORT HMODULE hModDPNet;
extern EXPORT HMODULE hModDaLib;
extern EXPORT HMODULE hModContent;
extern EXPORT _CreateWString CreateWString;
extern EXPORT _FreeWString FreeWString;
extern EXPORT _CreateString CreateString;
extern EXPORT _FreeString FreeString;
extern EXPORT _GetCString GetCString;
extern EXPORT _GetWCString GetWCString;
extern EXPORT _WStringAssign WStringAssign;
extern EXPORT _WStringAppend WStringAppend;
extern EXPORT FILE *fLog;
extern EXPORT FILE *fLogDebug;
extern EXPORT FARPROC fpOldUpdate;
extern EXPORT string sDebugLog;

// setting variables
extern EXPORT string set_scCfgFile;
extern EXPORT uint set_iAntiDockKill;
extern EXPORT list<uint> set_lstNoPVPSystems;
extern EXPORT list<wstring> set_lstChatSuppress;
extern EXPORT bool set_bSocketActivated;
extern EXPORT bool set_bDebug;
extern EXPORT bool set_bLogConnects;
extern EXPORT wstring set_wscDeathMsgStyle;
extern EXPORT wstring set_wscDeathMsgStyleSys;
extern EXPORT bool	set_bDieMsg;
extern EXPORT bool	set_bDisableCharfileEncryption;
extern EXPORT uint	set_iAntiBaseIdle;
extern EXPORT uint	set_iAntiCharMenuIdle;
extern EXPORT bool set_bChangeCruiseDisruptorBehaviour;
extern EXPORT bool	set_bUserCmdSetDieMsg;
extern EXPORT bool	set_bUserCmdSetChatFont;
extern EXPORT bool	set_bUserCmdIgnore;
extern EXPORT bool	set_bUserCmdHelp;
extern EXPORT bool  set_bDefaultLocalChat;
extern EXPORT uint	set_iDisableNPCSpawns;
extern EXPORT int	set_iPort;
extern EXPORT int	set_iWPort;
extern EXPORT int	set_iEPort;
extern EXPORT int	set_iEWPort;
extern EXPORT BLOWFISH_CTX	*set_BF_CTX;
extern EXPORT wstring set_wscKickMsg;
extern EXPORT wstring set_wscUserCmdStyle;
extern EXPORT wstring set_wscAdminCmdStyle;
extern EXPORT uint	set_iKickMsgPeriod;
extern EXPORT wstring set_wscDeathMsgTextPlayerKill;
extern EXPORT wstring set_wscDeathMsgTextSelfKill;
extern EXPORT wstring set_wscDeathMsgTextNPC;
extern EXPORT wstring set_wscDeathMsgTextSuicide;
extern EXPORT uint	set_iAntiF1;
extern EXPORT wstring set_wscDeathMsgTextAdminKill;
extern EXPORT uint	set_iUserCmdMaxIgnoreList;
extern EXPORT uint	set_iReservedSlots;
extern EXPORT uint	set_iDisconnectDelay;
extern EXPORT bool	set_bAutoBuy;
extern EXPORT float set_fTorpMissileBaseDamageMultiplier;
extern EXPORT bool set_MKM_bActivated;
extern EXPORT wstring set_MKM_wscStyle;
extern EXPORT list<MULTIKILLMESSAGE> set_MKM_lstMessages;
extern EXPORT bool	set_bUserCmdSetDieMsgSize;
extern EXPORT uint	set_iMaxGroupSize;
extern EXPORT list<wstring> set_lstBans;
extern EXPORT bool	set_bBanAccountOnMatch;
extern EXPORT uint set_iTimerThreshold;
extern EXPORT uint set_iTimerDebugThreshold;
extern EXPORT uint set_iDebugMaxSize;
extern EXPORT bool	set_bLogAdminCmds;
extern EXPORT bool	set_bLogSocketCmds;
extern EXPORT bool	set_bLogLocalSocketCmds;
extern EXPORT bool	set_bLogUserCmds;
extern EXPORT bool	set_bPerfTimer;

#endif
