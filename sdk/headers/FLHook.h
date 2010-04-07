#ifndef _FLHOOK_
#define _FLHOOK_

#pragma warning(disable: 4311 4786)

#include <windows.h>
#include <stdio.h>
#include <string>
#include <list>
#include <time.h>
using namespace std;


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// defines

#define HKHKSUCCESS(a) ((a) == HKE_OK)
#define HKFAILED(a) !HKSUCCESS(a)
#define ARG_CLIENTID(a) (wstring(L"id ") + stows(itos(a)))
#define HK_GET_CLIENTID(a, b) \
	bool bIdString = false; \
	if(b.find(L"id ") == 0) bIdString = true; \
	uint a; \
	{ \
		HK_ERROR hkErr = HkResolveId(b, a); \
		if(hkErr != HKE_OK) \
		{ \
			if(hkErr == HKE_INVALID_ID_STRING) { \
				hkErr = HkResolveShortCut(b, a); \
				if((hkErr == HKE_AMBIGUOUS_SHORTCUT) || (hkErr == HKE_NO_MATCHING_PLAYER)) \
					return hkErr; \
				else if(hkErr == HKE_INVALID_SHORTCUT_STRING) \
					a = HkGetClientIdFromCharname(b); \
			} else \
				return hkErr; \
		} \
	} \

#define IMPORT __declspec(dllimport)
#define EXPORT __declspec(dllexport)

#define foreach(lst, type, var) for(list<type>::iterator var = lst.begin(); (var != lst.end()); var++)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

#include "FLCoreServer.h"
#include "FLCoreCommon.h"
#include "FLCoreRemoteClient.h"
#include "FLCoreDALib.h"

typedef void (__stdcall *_RCSendChatMsg)(uint iId, uint iTo, uint iSize, void *pRDL);
typedef void (__stdcall *_CRCAntiCheat)();
typedef void (__stdcall *_CreateChar)(const wchar_t *wszName);
typedef int (__cdecl *_GetFLName)(char *szBuf, const wchar_t *wszStr);
typedef bool (__cdecl *_GetShipInspect)(uint &iShip, IObjInspectImpl* &inspect, uint &iDunno);

extern IMPORT _GetShipInspect GetShipInspect;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enums

enum HK_ERROR
{
	HKE_OK,
	HKE_PLAYER_NOT_LOGGED_IN,
	HKE_CHAR_DOES_NOT_EXIST,
	HKE_INVALID_CLIENT_ID,
	HKE_COULD_NOT_DECODE_CHARFILE,
	HKE_COULD_NOT_ENCODE_CHARFILE,
	HKE_INVALID_BASENAME,
	HKE_INVALID_ID_STRING,
	HKE_INVALID_SYSTEM,
	HKE_PLAYER_NOT_IN_SPACE,
	HKE_PLAYER_NO_ADMIN,
	HKE_WRONG_XML_SYNTAX,
	HKE_INVALID_GOOD,
	HKE_NO_CHAR_SELECTED,
	HKE_CHARNAME_ALREADY_EXISTS,
	HKE_CHARNAME_TOO_LONG,
	HKE_CHARNAME_TOO_SHORT,
	HKE_AMBIGUOUS_SHORTCUT,
	HKE_NO_MATCHING_PLAYER,
	HKE_INVALID_SHORTCUT_STRING,
	HKE_MPNEWCHARACTERFILE_NOT_FOUND_OR_INVALID,
	HKE_INVALID_REP_GROUP,
	HKE_PLUGIN_UNLOADABLE,
	HKE_PLUGIN_UNPAUSABLE,
	HKE_PLUGIN_NOT_FOUND,
	HKE_UNKNOWN_ERROR,
};

enum DIEMSGTYPE
{
	DIEMSG_ALL = 0,
	DIEMSG_SYSTEM = 1,
	DIEMSG_NONE = 2,
	DIEMSG_SELF = 3,
};

enum CHATSIZE
{
	CS_DEFAULT = 0,
	CS_SMALL = 1,
	CS_BIG = 2,
};

enum CHATSTYLE
{
	CST_DEFAULT = 0,
	CST_BOLD = 1,
	CST_ITALIC = 2,
	CST_UNDERLINE = 3,
};

enum ENGINE_STATE
{
	ES_CRUISE,
	ES_THRUSTER,
	ES_ENGINE,
	ES_KILLED,
	ES_TRADELANE
};

enum EQ_TYPE
{
	ET_GUN,
	ET_TORPEDO,
	ET_CD,
	ET_MISSILE,
	ET_MINE,
	ET_CM,
	ET_SHIELDGEN,
	ET_THRUSTER,
	ET_SHIELDBAT,
	ET_NANOBOT,
	ET_MUNITION,
	ET_ENGINE,
	ET_OTHER
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// structs

struct HOOKENTRY
{
	FARPROC fpProc;
	long	dwRemoteAddress;
	FARPROC fpOldProc;
};

struct CARGO_INFO
{
	uint		iID;
	int			iCount;
	uint		iArchID;
	float		fStatus;
	bool		bMission;
	bool		bMounted;
	CacheString	hardpoint;
};


// money stuff
struct MONEY_FIX
{
	wstring		wscCharname;
	int			iAmount;

	bool operator==(MONEY_FIX mf1)
	{
		if(!wscCharname.compare(mf1.wscCharname))
			return true;

		return false;
	};
};

// ignore
struct IGNORE_INFO
{
	wstring wscCharname;
	wstring wscFlags;
};

// resolver
struct RESOLVE_IP
{
	uint iClientID;
	uint iConnects;
	wstring wscIP;
	wstring wscHostname;
};

struct CLIENT_INFO
{
// kill msgs
	uint		iShip;
	uint		iShipOld;
	mstime		tmSpawnTime;

	DamageList	dmgLast;

// money cmd
	list<MONEY_FIX> lstMoneyFix;

// anticheat
	uint		iTradePartner;

// change cruise disruptor behaviour
	bool		bCruiseActivated;
	bool		bThrusterActivated;
	bool		bEngineKilled;
	bool		bTradelane;

// idle kicks
	uint		iBaseEnterTime;
	uint		iCharMenuEnterTime;

// msg, wait and kick
	mstime		tmKickTime;

// eventmode
	uint		iLastExitedBaseID;
	bool		bDisconnected;

// f1 laming
	bool		bCharSelected;
	mstime		tmF1Time;
	mstime		tmF1TimeDisconnect;

// ignore usercommand
	list<IGNORE_INFO> lstIgnore;

// user settings
	DIEMSGTYPE	dieMsg;
	CHATSIZE	dieMsgSize;
	CHATSTYLE	dieMsgStyle;
	CHATSIZE	chatSize;
	CHATSTYLE	chatStyle;

// autobuy
	bool		bAutoBuyMissiles;
	bool		bAutoBuyMines;
	bool		bAutoBuyTorps;
	bool		bAutoBuyCD;
	bool		bAutoBuyCM;
	bool		bAutoBuyReload;

// MultiKillMessages
	uint		iKillsInARow;

// bans
	uint		iConnects; // incremented when player connects

// other
	wstring		wscHostname;
};

// extended client info
struct CLIENT_INFO_EXT01
{
	bool		bSpawnProtected;
	byte		unused_data[128];
};

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

// taken from directplay
typedef struct _DPN_CONNECTION_INFO{
    DWORD   dwSize;
    DWORD   dwRoundTripLatencyMS;
    DWORD   dwThroughputBPS;
    DWORD   dwPeakThroughputBPS;
    DWORD   dwBytesSentGuaranteed;
    DWORD   dwPacketsSentGuaranteed;
    DWORD   dwBytesSentNonGuaranteed;
    DWORD   dwPacketsSentNonGuaranteed;
    DWORD   dwBytesRetried;
    DWORD   dwPacketsRetried;
    DWORD   dwBytesDropped;   
    DWORD   dwPacketsDropped; 
    DWORD   dwMessagesTransmittedHighPriority;
    DWORD   dwMessagesTimedOutHighPriority;
    DWORD   dwMessagesTransmittedNormalPriority;
    DWORD   dwMessagesTimedOutNormalPriority;
    DWORD   dwMessagesTransmittedLowPriority;
    DWORD   dwMessagesTimedOutLowPriority;
    DWORD   dwBytesReceivedGuaranteed;
    DWORD   dwPacketsReceivedGuaranteed;
    DWORD   dwBytesReceivedNonGuaranteed;
    DWORD   dwPacketsReceivedNonGuaranteed;
    DWORD   dwMessagesReceived;
} DPN_CONNECTION_INFO, *PDPN_CONNECTION_INFO;

struct HKPLAYERINFO
{
	uint iClientID;
	wstring wscCharname;
	wstring wscBase;
	wstring wscSystem;
	uint iSystem;
	uint iShip;
	DPN_CONNECTION_INFO ci;
	wstring wscIP;
	wstring wscHostname;
};

// patch stuff
struct PATCH_INFO_ENTRY
{
	ulong pAddress;
	void *pNewValue;
	uint iSize;
	void *pOldValue;
	bool bAlloced;
};

struct PATCH_INFO
{
	char	*szBinName;
	ulong	pBaseAddress;

	PATCH_INFO_ENTRY piEntries[128];
};

struct DATA_MARKETITEM
{
	uint iArchID;
	float fRep;
};

struct BASE_INFO
{
	uint	iBaseID;
	string	scBasename;
	uint	iObjectID;
	bool	bDestroyed;
	list<DATA_MARKETITEM> lstMarketMisc;
};

struct GROUP_MEMBER
{
	uint iClientID;
	wstring wscCharname;
};

struct PLUGIN_HOOKDATA
{
	string sName;
	HMODULE hDLL;
	int iPriority;
	bool bPaused;
};

struct PLUGIN_DATA
{
	string sName;
	string sShortName;
	HMODULE hDLL;
	string sDLL;
	bool bMayPause;
	bool bMayUnload;
	bool bPaused;
};



// HkFuncTools
IMPORT uint HkGetClientIdFromAccount(CAccount *acc);
IMPORT uint HkGetClientIdFromPD(struct PlayerData *pPD);
IMPORT CAccount* HkGetAccountByCharname(wstring wscCharname);
IMPORT uint HkGetClientIdFromCharname(wstring wscCharname);
IMPORT wstring HkGetAccountID(CAccount *acc);
IMPORT bool HkIsEncoded(string scFilename);
IMPORT bool HkIsInCharSelectMenu(wstring wscCharname);
IMPORT bool HkIsInCharSelectMenu(uint iClientID);
IMPORT bool HkIsValidClientID(uint iClientID);
IMPORT HK_ERROR HkResolveId(wstring wscCharname, uint &iClientID);
IMPORT HK_ERROR HkResolveShortCut(wstring wscShortcut, uint &iClientID);
IMPORT uint HkGetClientIDByShip(uint iShip);
IMPORT HK_ERROR HkGetAccountDirName(CAccount *acc, wstring &wscDir);
IMPORT HK_ERROR HkGetAccountDirName(wstring wscCharname, wstring &wscDir);
IMPORT HK_ERROR HkGetCharFileName(wstring wscCharname, wstring &wscFilename);
IMPORT wstring HkGetBaseNickByID(uint iBaseID);
IMPORT wstring HkGetPlayerSystem(uint iClientID);
IMPORT wstring HkGetSystemNickByID(uint iSystemID);
IMPORT void HkLockAccountAccess(CAccount *acc, bool bKick);
IMPORT void HkUnlockAccountAccess(CAccount *acc);
IMPORT void HkGetItemsForSale(uint iBaseID, list<uint> &lstItems);
IMPORT IObjInspectImpl* HkGetInspect(uint iClientID);
IMPORT ENGINE_STATE HkGetEngineState(uint iClientID);
IMPORT EQ_TYPE HkGetEqType(Archetype::Equipment *eq);

// HkFuncMsg
IMPORT HK_ERROR HkMsg(int iClientID, wstring wscMessage);
IMPORT HK_ERROR HkMsg(wstring wscCharname, wstring wscMessage);
IMPORT HK_ERROR HkMsgS(wstring wscSystemname, wstring wscMessage);
IMPORT HK_ERROR HkMsgU(wstring wscMessage);
IMPORT HK_ERROR HkFMsgEncodeXML(wstring wscXML, char *szBuf, uint iSize, uint &iRet);
IMPORT HK_ERROR HkFMsgSendChat(uint iClientID, char *szBuf, uint iSize);
IMPORT HK_ERROR HkFMsg(uint iClientID, wstring wscXML);
IMPORT HK_ERROR HkFMsg(wstring wscCharname, wstring wscXML);
IMPORT HK_ERROR HkFMsgS(wstring wscSystemname, wstring wscXML);
IMPORT HK_ERROR HkFMsgU(wstring wscXML);

// HkFuncPlayers
IMPORT HK_ERROR HkGetCash(wstring wscCharname, int &iCash);
IMPORT HK_ERROR HkAddCash(wstring wscCharname, int iAmount);
IMPORT HK_ERROR HkKick(CAccount *acc);
IMPORT HK_ERROR HkKick(wstring wscCharname);
IMPORT HK_ERROR HkKickReason(wstring wscCharname, wstring wscReason);
IMPORT HK_ERROR HkBan(wstring wscCharname, bool bBan);
IMPORT HK_ERROR HkBeam(wstring wscCharname, wstring wscBasename);
IMPORT HK_ERROR HkSaveChar(wstring wscCharname);
IMPORT HK_ERROR HkEnumCargo(wstring wscCharname, list<CARGO_INFO> &lstCargo, int &iRemainingHoldSize);
IMPORT HK_ERROR HkRemoveCargo(wstring wscCharname, uint iID, int iCount);
IMPORT HK_ERROR HkAddCargo(wstring wscCharname, uint iGoodID, int iCount, bool bMission);
IMPORT HK_ERROR HkAddCargo(wstring wscCharname, wstring wscGood, int iCount, bool bMission);
IMPORT HK_ERROR HkRename(wstring wscCharname, wstring wscNewCharname, bool bOnlyDelete);
IMPORT HK_ERROR HkMsgAndKick(uint iClientID, wstring wscReason, uint iIntervall);
IMPORT HK_ERROR HkKill(wstring wscCharname);
IMPORT HK_ERROR HkGetReservedSlot(wstring wscCharname, bool &bResult);
IMPORT HK_ERROR HkSetReservedSlot(wstring wscCharname, bool bReservedSlot);
IMPORT void HkPlayerAutoBuy(uint iClientID, uint iBaseID);
IMPORT HK_ERROR HkResetRep(wstring wscCharname);
IMPORT HK_ERROR HkGetGroupMembers(wstring wscCharname, list<GROUP_MEMBER> &lstMembers);
IMPORT HK_ERROR HkSetRep(wstring wscCharname, wstring wscRepGroup, float fValue);
IMPORT HK_ERROR HkGetRep(wstring wscCharname, wstring wscRepGroup, float &fValue);
IMPORT HK_ERROR HkReadCharFile(wstring wscCharname, list<wstring> &lstOutput);
IMPORT HK_ERROR HkWriteCharFile(wstring wscCharname, wstring wscData);

// HkFuncLog
IMPORT void AddLog(FILE* fLog, const char *szString, ...);
IMPORT void AddLog(const char *szString, ...);
IMPORT void HkHandleCheater(uint iClientID, bool bBan, wstring wscReason, ...);
IMPORT bool HkAddCheaterLog(wstring wscCharname, wstring wscReason);
IMPORT bool HkAddKickLog(uint iClientID, wstring wscReason, ...);
IMPORT bool HkAddConnectLog(uint iClientID, wstring wscReason, ...);
IMPORT void HkAddAdminCmdLog(const char *szString, ...);
IMPORT void HkAddUserCmdLog(const char *szString, ...);
IMPORT void HkAddPerfTimerLog(const char *szString, ...);

// HkFuncOther
IMPORT void HkGetPlayerIP(uint iClientID, wstring &wscIP);
IMPORT HK_ERROR HkGetPlayerInfo(wstring wscCharname, HKPLAYERINFO &pi, bool bAlsoCharmenu);
IMPORT list<HKPLAYERINFO> HkGetPlayers();
IMPORT HK_ERROR HkGetConnectionStats(uint iClientID, DPN_CONNECTION_INFO &ci);
IMPORT HK_ERROR HkSetAdmin(wstring wscCharname, wstring wscRights);
IMPORT HK_ERROR HkGetAdmin(wstring wscCharname, wstring &wscRights);
IMPORT HK_ERROR HkDelAdmin(wstring wscCharname);
IMPORT HK_ERROR HkChangeNPCSpawn(bool bDisable);
IMPORT HK_ERROR HkGetBaseStatus(wstring wscBasename, float &fHealth, float &fMaxHealth);

// HkFLIni
IMPORT HK_ERROR HkFLIniGet(wstring wscCharname, wstring wscKey, wstring &wscRet);
IMPORT HK_ERROR HkFLIniWrite(wstring wscCharname, wstring wscKey, wstring wscValue);

IMPORT wstring HkErrGetText(HK_ERROR hkErr);
IMPORT void ClearClientInfo(uint iClientID);
IMPORT void LoadUserSettings(uint iClientID);

// HkCbUserCmd
IMPORT bool UserCmd_Process(uint iClientID, wstring wscCmd);
IMPORT void UserCmd_SetDieMsg(uint iClientID, wstring wscParam);
IMPORT void UserCmd_SetChatFont(uint iClientID, wstring wscParam);

IMPORT void ProcessEvent(wstring wscText, ...);

IMPORT void PrintUserCmdText(uint iClientID, wstring wscText, ...);

// tools
IMPORT wstring stows(string scText);
IMPORT string wstos(wstring wscText);
IMPORT string itos(int i);
IMPORT string IniGetS(string scFile, string scApp, string scKey, string scDefault);
IMPORT int IniGetI(string scFile, string scApp, string scKey, int iDefault);
IMPORT bool IniGetB(string scFile, string scApp, string scKey, bool bDefault);
IMPORT void IniWrite(string scFile, string scApp, string scKey, string scValue);
IMPORT void WriteProcMem(void *pAddress, void *pMem, int iSize);
IMPORT void ReadProcMem(void *pAddress, void *pMem, int iSize);
IMPORT wstring ToLower(wstring wscStr);
IMPORT string ToLower(string scStr);
IMPORT int ToInt(wstring wscStr);
IMPORT void ConPrint(wstring wscText, ...);
IMPORT wstring XMLText(wstring wscText);
IMPORT wstring GetParam(wstring wscLine, wchar_t wcSplitChar, uint iPos);
IMPORT wstring ReplaceStr(wstring wscSource, wstring wscSearchFor, wstring wscReplaceWith);
IMPORT void IniDelSection(string scFile, string scApp);
IMPORT void IniWriteW(string scFile, string scApp, string scKey, wstring wscValue);
IMPORT wstring IniGetWS(string scFile, string scApp, string scKey, wstring wscDefault);
IMPORT wstring ToMoneyStr(int iCash);
IMPORT float IniGetF(string scFile, string scApp, string scKey, float fDefault);
IMPORT void IniGetSection(string scFile, string scApp, list<INISECTIONVALUE> &lstValues);
IMPORT float ToFloat(wstring wscStr);
IMPORT mstime timeInMS();

// flcodec
IMPORT bool flc_decode(const char *ifile, const char *ofile);
IMPORT bool flc_encode(const char *ifile, const char *ofile);

// admin commands

// enums
enum CCMDS_RIGHTS
{
	RIGHT_NOTHING		= 0,
	RIGHT_SUPERADMIN	= 0xFFFFFFFF,
	RIGHT_MSG			= (1 << 0),
	RIGHT_KICKBAN		= (1 << 1),
	RIGHT_EVENTMODE		= (1 << 2),
	RIGHT_CASH			= (1 << 3),
	RIGHT_BEAMKILL		= (1 << 4),
	RIGHT_REPUTATION	= (1 << 5),
	RIGHT_CARGO			= (1 << 6),
	RIGHT_CHARACTERS	= (1 << 7),
	RIGHT_SETTINGS		= (1 << 8),
	RIGHT_PLUGINS		= (1 << 9),
	RIGHT_OTHER			= (1 << 10),
	RIGHT_SPECIAL1		= (1 << 11),
	RIGHT_SPECIAL2		= (1 << 12),
	RIGHT_SPECIAL3		= (1 << 13),
};

class CTimer
{
public:
	IMPORT CTimer(string sFunction, uint iWarning);
    IMPORT void start();
	IMPORT uint stop();

private:
	mstime tmStart;
	uint iMax;
	string sFunction;
	uint iWarning;
};

// admin stuff
class CCmds
{
	bool bID;
	bool bShortCut;

public:
	DWORD rights;
	HK_ERROR hkLastErr;

	IMPORT void PrintError();

// commands
	void CmdGetCash(wstring wscCharname);
	void CmdSetCash(wstring wscCharname, int iAmount);
	void CmdSetCashSec(wstring wscCharname, int iAmountCheck, int iAmount);
	void CmdAddCash(wstring wscCharname, int iAmount);
	void CmdAddCashSec(wstring wscCharname, int iAmountCheck, int iAmount);

	void CmdKick(wstring wscCharname, wstring wscReason);
	void CmdBan(wstring wscCharname);
	void CmdUnban(wstring wscCharname);
	void CmdKickBan(wstring wscCharname, wstring wscReason);

	void CmdBeam(wstring wscCharname, wstring wscBasename);
	void CmdKill(wstring wscCharname);
	void CmdResetRep(wstring wscCharname);
	void CmdSetRep(wstring wscCharname, wstring wscRepGroup, float fValue);
	void CmdGetRep(wstring wscCharname, wstring wscRepGroup);

	void CmdMsg(wstring wscCharname, wstring wscText);
	void CmdMsgS(wstring wscSystemname, wstring wscText);
	void CmdMsgU(wstring wscText);
	void CmdFMsg(wstring wscCharname, wstring wscXML);
	void CmdFMsgS(wstring wscSystemname, wstring wscXML);
	void CmdFMsgU(wstring wscXML);

	void CmdEnumCargo(wstring wscCharname);
	void CmdRemoveCargo(wstring wscCharname, uint iID, uint iCount);
	void CmdAddCargo(wstring wscCharname, wstring wscGood, uint iCount, uint iMission);

	void CmdRename(wstring wscCharname, wstring wscNewCharname);
	void CmdDeleteChar(wstring wscCharname);

	void CmdReadCharFile(wstring wscCharname);
	void CmdWriteCharFile(wstring wscCharname, wstring wscData);

	void CmdGetBaseStatus(wstring wscBasename);
	void CmdGetClientId(wstring wscCharname);
	void PrintPlayerInfo(HKPLAYERINFO pi);
	void CmdGetPlayerInfo(wstring wscCharname);
	void CmdGetPlayers();
	void XPrintPlayerInfo(HKPLAYERINFO pi);
	void CmdXGetPlayerInfo(wstring wscCharname);
	void CmdXGetPlayers();
	void CmdGetPlayerIDs();
	void CmdHelp();
	void CmdGetAccountDirName(wstring wscCharname);
	void CmdGetCharFileName(wstring wscCharname);
	void CmdIsOnServer(wstring wscCharname);
	void CmdIsLoggedIn(wstring wscCharname);
	void CmdMoneyFixList();
	void CmdServerInfo();
	void CmdGetGroupMembers(wstring wscCharname);

	void CmdSaveChar(wstring wscCharname);

	void CmdGetReservedSlot(wstring wscCharname);
	void CmdSetReservedSlot(wstring wscCharname, int iReservedSlot);
	void CmdSetAdmin(wstring wscCharname, wstring wscRights);
	void CmdGetAdmin(wstring wscCharname);
	void CmdDelAdmin(wstring wscCharname);
	void CmdRehash();
	void CmdUnload(wstring wscParam);

	void CmdLoadPlugins();
	void CmdListPlugins();
	void CmdUnloadPlugin(wstring wscPlugin);
	void CmdPausePlugin(wstring wscPlugin);
	void CmdUnpausePlugin(wstring wscPlugin);

	void CmdTest(int iArg, int iArg2, int iArg3);
//
	IMPORT wstring ArgCharname(uint iArg);
	IMPORT int ArgInt(uint iArg);
	IMPORT float ArgFloat(uint iArg);
	IMPORT wstring ArgStr(uint iArg);
	IMPORT wstring ArgStrToEnd(uint iArg);
	void ExecuteCommandString(wstring wscCmd);

	void SetRightsByString(string scRightStr);
	IMPORT void Print(wstring wscText, ...);
	virtual void DoPrint(wstring wscText) {};
	IMPORT virtual wstring GetAdminName() { return L""; };

	wstring wscCurCmdString;
};


// namespaces
namespace HkIServerImpl 
{
	IMPORT extern bool g_bInSubmitChat;
	IMPORT extern uint g_iTextLen;
}
namespace PluginManager {
	IMPORT void LoadPlugins(bool, CCmds*);
	IMPORT HK_ERROR PausePlugin(string sShortName, bool bPause);
	IMPORT HK_ERROR UnloadPlugin(string sShortName);
	IMPORT void UnloadPlugins();
}

// variables

extern IMPORT std::map<string, list<PLUGIN_HOOKDATA>*> mpPluginHooks;
extern IMPORT list<PLUGIN_DATA> lstPlugins;

extern IMPORT HkIClientImpl* FakeClient;
extern IMPORT HkIClientImpl* HookClient;
extern IMPORT char* OldClient;

extern IMPORT uint	iDmgTo;
extern IMPORT uint iDmgToSpaceID;

extern IMPORT bool g_bMsg;
extern IMPORT bool g_bMsgS;
extern IMPORT bool g_bMsgU;

extern IMPORT HANDLE hProcFL;
extern IMPORT HMODULE hModServer;
extern IMPORT HMODULE hModCommon;
extern IMPORT HMODULE hModRemoteClient;
extern IMPORT HMODULE hModDPNet;
extern IMPORT HMODULE hModDaLib;
extern IMPORT HMODULE hModContent;
extern IMPORT _CreateWString CreateWString;
extern IMPORT _FreeWString FreeWString;
extern IMPORT _CreateString CreateString;
extern IMPORT _FreeString FreeString;
extern IMPORT _GetCString GetCString;
extern IMPORT _GetWCString GetWCString;
extern IMPORT FILE *fLog;
extern IMPORT FILE *fLogDebug;
extern IMPORT FARPROC fpOldUpdate;
extern IMPORT string sDebugLog;

// setting variables
extern IMPORT string set_scCfgFile;
extern IMPORT uint set_iAntiDockKill;
extern IMPORT list<uint> set_lstNoPVPSystems;
extern IMPORT list<wstring> set_lstChatSuppress;
extern IMPORT bool set_bSocketActivated;
extern IMPORT bool set_bDebug;
extern IMPORT bool set_bLogConnects;
extern IMPORT wstring set_wscDeathMsgStyle;
extern IMPORT wstring set_wscDeathMsgStyleSys;
extern IMPORT bool	set_bDieMsg;
extern IMPORT bool	set_bDisableCharfileEncryption;
extern IMPORT uint	set_iAntiBaseIdle;
extern IMPORT uint	set_iAntiCharMenuIdle;
extern IMPORT bool set_bChangeCruiseDisruptorBehaviour;
extern IMPORT bool	set_bUserCmdSetDieMsg;
extern IMPORT bool	set_bUserCmdSetChatFont;
extern IMPORT bool	set_bUserCmdIgnore;
extern IMPORT bool	set_bUserCmdHelp;
extern IMPORT uint	set_iDisableNPCSpawns;
extern IMPORT int	set_iPort;
extern IMPORT int	set_iWPort;
extern IMPORT wstring set_wscKickMsg;
extern IMPORT wstring set_wscUserCmdStyle;
extern IMPORT wstring set_wscAdminCmdStyle;
extern IMPORT uint	set_iKickMsgPeriod;
extern IMPORT wstring set_wscDeathMsgTextPlayerKill;
extern IMPORT wstring set_wscDeathMsgTextSelfKill;
extern IMPORT wstring set_wscDeathMsgTextNPC;
extern IMPORT wstring set_wscDeathMsgTextSuicide;
extern IMPORT uint	set_iAntiF1;
extern IMPORT wstring set_wscDeathMsgTextAdminKill;
extern IMPORT uint	set_iUserCmdMaxIgnoreList;
extern IMPORT uint	set_iReservedSlots;
extern IMPORT uint	set_iDisconnectDelay;
extern IMPORT bool	set_bAutoBuy;
extern IMPORT float set_fTorpMissileBaseDamageMultiplier;
extern IMPORT bool set_MKM_bActivated;
extern IMPORT wstring set_MKM_wscStyle;
extern IMPORT list<MULTIKILLMESSAGE> set_MKM_lstMessages;
extern IMPORT bool	set_bUserCmdSetDieMsgSize;
extern IMPORT uint	set_iMaxGroupSize;
extern IMPORT list<wstring> set_lstBans;
extern IMPORT bool	set_bBanAccountOnMatch;
extern IMPORT uint set_iTimerThreshold;
extern IMPORT uint set_iDebugMaxSize;
extern IMPORT bool	set_bLogAdminCmds;
extern IMPORT bool	set_bLogUserCmds;
extern IMPORT bool	set_bPerfTimer;


extern IMPORT list<BASE_INFO> lstBases;

extern IMPORT CDPClientProxy **g_cClientProxyArray;
extern IMPORT void *pClient;

extern IMPORT _RCSendChatMsg RCSendChatMsg;
extern IMPORT _CRCAntiCheat CRCAntiCheat;
extern IMPORT _CreateChar CreateChar;

extern IMPORT string scAcctPath;

extern IMPORT CLIENT_INFO ClientInfo[201];
extern IMPORT CLIENT_INFO_EXT01 ClientInfoExt01[201];
extern IMPORT CDPServer *cdpSrv;
extern IMPORT uint g_iServerLoad;
extern IMPORT bool g_bNPCDisabled;
extern IMPORT char *g_FLServerDataPtr;

#endif