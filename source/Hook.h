#ifndef _HOOK_
#define _HOOK_

#include <time.h>
#if _MSC_VER == 1200
#include "xtrace.h" // __FUNCTION__ macro for vc6
#endif
#include "global.h"
#include "flcodec.h"

#include <plugin.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// defines

#define HKHKSUCCESS(a) ((a) == HKE_OK)
#define HKSUCCESS(a) ((hkLastErr = (a)) == HKE_OK)

#define SRV_ADDR(a) ((char*)hModServer + a)
#define DALIB_ADDR(a) ((char*)hModDaLib + a)
#define FLSERVER_ADDR(a) ((char*)hProcFL + a)
#define CONTENT_ADDR(a) ((char*)hModContent + a)
#define ARG_CLIENTID(a) (wstring(L"id ") + stows(itos(a)))


#define ADDR_UPDATE 0x1BAB4
#define ADDR_STARTUP 0x1BABC
#define ADDR_SHUTDOWN 0x1BAB8
#define ADDR_ANTIDIEMSG 0x39124
#define ADDR_DISCFENCR 0x6E10D
#define ADDR_DISCFENCR2 0x6BFA6
#define ADDR_CRCANTICHEAT 0x6FAF0
#define ADDR_RCSENDCHAT 0x7F30
#define ADDR_CPLIST 0x43D74
#define ADDR_CDPSERVER 0xA284 // 065CA284
#define ADDR_CREATECHAR 0x6B790 // 06D4B790
#define ADDR_FLNEW 0x80012 // 06D60012
#define ADDR_SERVERFLSERVER 0x1BC90 // 0041BC90
#define ADDR_DISABLENPCSPAWNS1 0x5987B // 06EF987B
#define ADDR_DISABLENPCSPAWNS2 0x59CD3 // 06EF9CD3
#define ADDR_DATAPTR 0x277EC // 004277EC
#define ADDR_RC_DISCONNECT 0x93E0 // 06B393E0
#define ADDR_DALIB_DISC_SUPPRESS 0x49C6 // 065C49C6
#define ADDR_SRV_GETCOMMODITIES 0x32EC2 // 06D12EC2
#define ADDR_SRV_MAXGROUPSIZE 0x3A068 // 06D1A068
#define ADDR_SRV_MAXGROUPSIZE2 0x3A46E // 06D1A46E
#define ADDR_SRV_GETINSPECT 0x206C0 // 06D006C0
#define ADDR_SRV_PLAYERDBMAXPLAYERSPATCH 0x64BC3
#define ADDR_SRV_PLAYERDBMAXPLAYERS 0xB0264
#define ADDR_SRV_REPARRAYFREE 0x7F3F0
#define ADDR_COMMON_VFTABLE_POWER 0x1398F4
#define ADDR_COMMON_VFTABLE_SCANNER 0x139920
#define ADDR_COMMON_VFTABLE_LIGHT 0x13994C
#define ADDR_COMMON_VFTABLE_TRACTOR 0x139978
#define ADDR_COMMON_VFTABLE_MINE 0x139C64
#define ADDR_COMMON_VFTABLE_CM 0x139C90
#define ADDR_COMMON_VFTABLE_GUN 0x139C38
#define ADDR_COMMON_VFTABLE_SHIELDGEN 0x139BB4
#define ADDR_COMMON_VFTABLE_THRUSTER 0x139BE0
#define ADDR_COMMON_VFTABLE_SHIELDBAT 0x1399FC
#define ADDR_COMMON_VFTABLE_NANOBOT 0x1399D0
#define ADDR_COMMON_VFTABLE_MUNITION 0x139CE8
#define ADDR_COMMON_VFTABLE_ENGINE 0x139AAC

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// exception logging

#define EXTENDED_EXCEPTION_LOGGING
#ifdef EXTENDED_EXCEPTION_LOGGING

struct SEHException
{
	SEHException(uint code, EXCEPTION_POINTERS* ep)
		: code(code), record(*ep->ExceptionRecord), context(*ep->ContextRecord)
	{ }

	uint code;
	EXCEPTION_RECORD record;
	CONTEXT context;

	static void Translator(uint code, EXCEPTION_POINTERS* ep)
	{
		throw SEHException(code, ep);
	}
};

EXPORT extern void WriteMiniDump(SEHException* ex);
EXPORT extern void AddExceptionInfoLog(SEHException* ex);
#define TRY_HOOK try { _set_se_translator(SEHException::Translator);
#define CATCH_HOOK(e) } \
catch(SEHException& ex) { e; AddBothLog("ERROR: SEH Exception in %s on line %d; minidump may contain more information.", __FUNCTION__, __LINE__); AddExceptionInfoLog(&ex); } \
catch(std::exception& ex) { e; AddBothLog("ERROR: STL Exception in %s on line %d: %s.", __FUNCTION__, __LINE__, ex.what()); AddExceptionInfoLog(0); } \
catch (...) { e; AddBothLog("ERROR: Exception in %s on line %d.", __FUNCTION__, __LINE__); AddExceptionInfoLog(0); }
#else
#define TRY_HOOK try
#define CATCH_HOOK(e) catch(...) { e; AddLog("ERROR: Exception in %s", __FUNCTION__); }
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// plugin functionality & hook prototypes

class CCmds;

class CTimer
{
public:
	EXPORT CTimer(string sFunction, uint iWarning);
    EXPORT void start();
	EXPORT uint stop();

private:
	mstime tmStart;
	uint iMax;
	string sFunction;
	uint iWarning;
};

struct PLUGIN_HOOKDATA
{
	string sName;
	string sPluginFunction;
	HMODULE hDLL;
	int iPriority;
	bool bPaused;
	FARPROC* pFunc;	
	PLUGIN_RETURNCODE* ePluginReturnCode;
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

struct PLUGIN_SORTCRIT {
  bool operator()(const PLUGIN_HOOKDATA& lhs, const PLUGIN_HOOKDATA& rhs) const {
	  if(lhs.iPriority > rhs.iPriority)
		  return true;
	  else
		  return false;
  }
};



#define CALL_PLUGINS(callback_id,ret_type,calling_convention,arg_types,args) \
{ \
	ret_type vPluginRet; \
	bool bPluginReturn = false; \
	g_bPlugin_nofunctioncall = false; \
	TRY_HOOK { \
		foreach(pPluginHooks[(int)callback_id],PLUGIN_HOOKDATA, itplugin) { \
			if(itplugin->bPaused) \
				continue; \
			if(itplugin->pFunc) { \
				CTimer timer(itplugin->sPluginFunction,set_iTimerThreshold); \
				timer.start(); \
				TRY_HOOK { \
					vPluginRet = ((ret_type (calling_convention*) arg_types )itplugin->pFunc) args; \
				} CATCH_HOOK( { AddLog("ERROR: Exception in plugin '%s' in %s", itplugin->sName.c_str(), __FUNCTION__); } ) \
				timer.stop(); \
			} else  \
				AddLog("ERROR: Plugin '%s' does not export %s [%s]", itplugin->sName.c_str(), __FUNCTION__, __FUNCDNAME__); \
			if(*itplugin->ePluginReturnCode == SKIPPLUGINS_NOFUNCTIONCALL) { \
				bPluginReturn = true; \
				break; \
			} else if(*itplugin->ePluginReturnCode == NOFUNCTIONCALL) { \
				bPluginReturn = true; \
				g_bPlugin_nofunctioncall = true; \
			} else if(*itplugin->ePluginReturnCode == SKIPPLUGINS) \
				break; \
		} \
	} CATCH_HOOK( { AddLog("ERROR: Exception %s", __FUNCTION__); } ) \
	if(bPluginReturn) \
		return vPluginRet; \
} \

// same for void types, not really seeing a way to integrate it in 1st macro :(
#define CALL_PLUGINS_V(callback_id,calling_convention,arg_types,args) \
{ \
	bool bPluginReturn = false; \
	g_bPlugin_nofunctioncall = false; \
	TRY_HOOK { \
		foreach(pPluginHooks[(int)callback_id],PLUGIN_HOOKDATA, itplugin) { \
			if(itplugin->bPaused) \
				continue; \
			if(itplugin->pFunc) { \
				CTimer timer(itplugin->sPluginFunction,set_iTimerThreshold); \
				timer.start(); \
				TRY_HOOK { \
					((void (calling_convention*) arg_types )itplugin->pFunc) args; \
				} CATCH_HOOK( { AddLog("ERROR: Exception in plugin '%s' in %s", itplugin->sName.c_str(), __FUNCTION__); } ) \
				timer.stop(); \
			} else  \
				AddLog("ERROR: Plugin '%s' does not export %s [%s]", itplugin->sName.c_str(), __FUNCTION__, __FUNCDNAME__); \
			if(*itplugin->ePluginReturnCode == SKIPPLUGINS_NOFUNCTIONCALL) { \
				bPluginReturn = true; \
				break; \
			} else if(*itplugin->ePluginReturnCode == NOFUNCTIONCALL) { \
				bPluginReturn = true; \
				g_bPlugin_nofunctioncall = true; \
			} else if(*itplugin->ePluginReturnCode == SKIPPLUGINS) \
				break; \
		} \
	} CATCH_HOOK( { AddLog("ERROR: Exception %s", __FUNCTION__); } ) \
	if(bPluginReturn) \
		return; \
} \

// extra macro for plugin calls where we dont care about or dont allow returning
#define CALL_PLUGINS_NORET(callback_id,calling_convention,arg_types,args) \
{ \
	g_bPlugin_nofunctioncall = false; \
	TRY_HOOK { \
		foreach(pPluginHooks[(int)callback_id],PLUGIN_HOOKDATA, itplugin) { \
			if(itplugin->bPaused) \
				continue; \
			if(itplugin->pFunc) { \
				CTimer timer(itplugin->sPluginFunction,set_iTimerThreshold); \
				timer.start(); \
				TRY_HOOK { \
					((void (calling_convention*) arg_types )itplugin->pFunc) args; \
				} CATCH_HOOK( { AddLog("ERROR: Exception in plugin '%s' in %s", itplugin->sName.c_str(), __FUNCTION__); } ) \
				timer.stop(); \
			} else  \
				AddLog("ERROR: Plugin '%s' does not export %s [%s]", itplugin->sName.c_str(), __FUNCTION__, __FUNCDNAME__); \
			if(*itplugin->ePluginReturnCode == SKIPPLUGINS_NOFUNCTIONCALL) { \
				AddLog("ERROR: Plugin '%s' wants to suppress function call in %s [%s] - denied!", itplugin->sName.c_str(), __FUNCTION__, __FUNCDNAME__); \
				break; \
			} else if(*itplugin->ePluginReturnCode == NOFUNCTIONCALL) { \
				AddLog("ERROR: Plugin '%s' wants to suppress function call in %s [%s] - denied!", itplugin->sName.c_str(), __FUNCTION__, __FUNCDNAME__); \
				g_bPlugin_nofunctioncall = true; \
			} else if(*itplugin->ePluginReturnCode == SKIPPLUGINS) \
				break; \
		} \
	} CATCH_HOOK( { AddLog("ERROR: Exception %s", __FUNCTION__); } ) \
} \

typedef PLUGIN_RETURNCODE (*PLUGIN_Get_PluginReturnCode)();
typedef PLUGIN_INFO* (*PLUGIN_Get_PluginInfo)();


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// typedefs
typedef void (__stdcall *_RCSendChatMsg)(uint iId, uint iTo, uint iSize, void *pRDL);
typedef void (__stdcall *_CRCAntiCheat)();
typedef void (__stdcall *_CreateChar)(const wchar_t *wszName);
typedef int (__cdecl *_GetFLName)(char *szBuf, const wchar_t *wszStr);
typedef bool (__cdecl *_GetShipInspect)(uint &iShip, IObjInspectImpl* &inspect, uint &iDunno);

EXPORT extern _GetShipInspect GetShipInspect;

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
	HKE_CUSTOM_1,
	HKE_CUSTOM_2,
	HKE_CUSTOM_3,
	HKE_CUSTOM_4,
	HKE_CUSTOM_5,
	HKE_CUSTOM_6,
	HKE_CUSTOM_7,
	HKE_CUSTOM_8,
	HKE_CUSTOM_9,
	HKE_CUSTOM_10,
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
	ET_OTHER,
	ET_SCANNER,
	ET_TRACTOR,
	ET_LIGHT
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

	bool operator==(MONEY_FIX mf1) const
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

	bool		bSpawnProtected;
	uchar		unused_data[128];
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prototypes

// HkPluginManager
namespace PluginManager {
	void Init();
	void Destroy();
	EXPORT void LoadPlugins(bool, CCmds*);
	EXPORT void LoadPlugin(const string &sFileName, CCmds*, bool);
	EXPORT HK_ERROR PausePlugin(const string &sShortName, bool bPause);
	EXPORT HK_ERROR UnloadPlugin(const string &sShortName);
	EXPORT void UnloadPlugins();
}

EXPORT void Plugin_Communication(PLUGIN_MESSAGE msgtype, void* msg);

// HkInit
void PatchClientImpl();
bool InitHookExports();
void UnloadHookExports();
void HookRehashed();
void LoadUserCharSettings(uint iClientID);

// HkFuncTools
EXPORT uint HkGetClientIdFromAccount(CAccount *acc);
EXPORT uint HkGetClientIdFromPD(struct PlayerData *pPD);
EXPORT CAccount* HkGetAccountByCharname(const wstring &wscCharname);
EXPORT uint HkGetClientIdFromCharname(const wstring &wscCharname);
EXPORT wstring HkGetAccountID(CAccount *acc);
EXPORT bool HkIsEncoded(const string &scFilename);
EXPORT bool HkIsInCharSelectMenu(const wstring &wscCharname);
EXPORT bool HkIsInCharSelectMenu(uint iClientID);
EXPORT bool HkIsValidClientID(uint iClientID);
EXPORT HK_ERROR HkResolveId(const wstring &wscCharname, uint &iClientID);
EXPORT HK_ERROR HkResolveShortCut(const wstring &wscShortcut, uint &iClientID);
EXPORT uint HkGetClientIDByShip(uint iShip);
EXPORT HK_ERROR HkGetAccountDirName(CAccount *acc, wstring &wscDir);
EXPORT HK_ERROR HkGetAccountDirName(const wstring &wscCharname, wstring &wscDir);
EXPORT HK_ERROR HkGetCharFileName(const wstring &wscCharname, wstring &wscFilename);
EXPORT wstring HkGetBaseNickByID(uint iBaseID);
EXPORT wstring HkGetPlayerSystem(uint iClientID);
EXPORT wstring HkGetSystemNickByID(uint iSystemID);
EXPORT void HkLockAccountAccess(CAccount *acc, bool bKick);
EXPORT void HkUnlockAccountAccess(CAccount *acc);
EXPORT void HkGetItemsForSale(uint iBaseID, list<uint> &lstItems);
EXPORT IObjInspectImpl* HkGetInspect(uint iClientID);
EXPORT ENGINE_STATE HkGetEngineState(uint iClientID);
EXPORT EQ_TYPE HkGetEqType(Archetype::Equipment *eq);

// HkFuncMsg
EXPORT HK_ERROR HkMsg(uint iClientID, const wstring &wscMessage);
EXPORT HK_ERROR HkMsg(const wstring &wscCharname, const wstring &wscMessage);
EXPORT HK_ERROR HkMsgS(const wstring &wscSystemname, const wstring &wscMessage);
EXPORT HK_ERROR HkMsgU(const wstring &wscMessage);
EXPORT HK_ERROR HkFMsgEncodeXML(const wstring &wscXML, char *szBuf, uint iSize, uint &iRet);
EXPORT HK_ERROR HkFMsgSendChat(uint iClientID, char *szBuf, uint iSize);
EXPORT HK_ERROR HkFMsg(uint iClientID, const wstring &wscXML);
EXPORT HK_ERROR HkFMsg(const wstring &wscCharname, const wstring &wscXML);
EXPORT HK_ERROR HkFMsgS(const wstring &wscSystemname, const wstring &wscXML);
EXPORT HK_ERROR HkFMsgU(const wstring &wscXML);

// HkFuncPlayers
EXPORT HK_ERROR HkGetCash(const wstring &wscCharname, int &iCash);
EXPORT HK_ERROR HkAddCash(const wstring &wscCharname, int iAmount);
EXPORT HK_ERROR HkKick(CAccount *acc);
EXPORT HK_ERROR HkKick(const wstring &wscCharname);
EXPORT HK_ERROR HkKickReason(const wstring &wscCharname, const wstring &wscReason);
EXPORT HK_ERROR HkBan(const wstring &wscCharname, bool bBan);
EXPORT HK_ERROR HkBeam(const wstring &wscCharname, const wstring &wscBasename);
EXPORT HK_ERROR HkSaveChar(const wstring &wscCharname);
EXPORT HK_ERROR HkEnumCargo(const wstring &wscCharname, list<CARGO_INFO> &lstCargo, int &iRemainingHoldSize);
EXPORT HK_ERROR HkRemoveCargo(const wstring &wscCharname, uint iID, int iCount);
EXPORT HK_ERROR HkAddCargo(const wstring &wscCharname, uint iGoodID, int iCount, bool bMission);
EXPORT HK_ERROR HkAddCargo(const wstring &wscCharname, const wstring &wscGood, int iCount, bool bMission);
EXPORT HK_ERROR HkRename(const wstring &wscCharname, const wstring &wscNewCharname, bool bOnlyDelete);
EXPORT HK_ERROR HkMsgAndKick(uint iClientID, const wstring &wscReason, uint iIntervall);
EXPORT HK_ERROR HkKill(const wstring &wscCharname);
EXPORT HK_ERROR HkGetReservedSlot(const wstring &wscCharname, bool &bResult);
EXPORT HK_ERROR HkSetReservedSlot(const wstring &wscCharname, bool bReservedSlot);
EXPORT void HkPlayerAutoBuy(uint iClientID, uint iBaseID);
EXPORT HK_ERROR HkResetRep(const wstring &wscCharname);
EXPORT HK_ERROR HkGetGroupMembers(const wstring &wscCharname, list<GROUP_MEMBER> &lstMembers);
EXPORT HK_ERROR HkSetRep(const wstring &wscCharname, const wstring &wscRepGroup, float fValue);
EXPORT HK_ERROR HkGetRep(const wstring &wscCharname, const wstring &wscRepGroup, float &fValue);
EXPORT HK_ERROR HkReadCharFile(const wstring &wscCharname, list<wstring> &lstOutput);
EXPORT HK_ERROR HkWriteCharFile(const wstring &wscCharname, wstring wscData);
EXPORT HK_ERROR HkPlayerRecalculateCRC(uint iClientID);

// HkFuncLog
#define AddBothLog(s, ...) { AddLog(s, __VA_ARGS__); AddDebugLog(s, __VA_ARGS__);  }
EXPORT void AddDebugLog(const char *szString, ...);
EXPORT void AddLog(const char *szString, ...);
EXPORT void HkHandleCheater(uint iClientID, bool bBan, wstring wscReason, ...);
EXPORT bool HkAddCheaterLog(const wstring &wscCharname, const wstring &wscReason);
EXPORT bool HkAddCheaterLog(const uint &iClientID, const wstring &wscReason);
EXPORT bool HkAddKickLog(uint iClientID, wstring wscReason, ...);
EXPORT bool HkAddConnectLog(uint iClientID, wstring wscReason, ...);
EXPORT void HkAddAdminCmdLog(const char *szString, ...);
EXPORT void HkAddSocketCmdLog(const char *szString, ...);
EXPORT void HkAddUserCmdLog(const char *szString, ...);
EXPORT void HkAddPerfTimerLog(const char *szString, ...);

// HkFuncOther
EXPORT void HkGetPlayerIP(uint iClientID, wstring &wscIP);
EXPORT HK_ERROR HkGetPlayerInfo(const wstring &wscCharname, HKPLAYERINFO &pi, bool bAlsoCharmenu);
EXPORT list<HKPLAYERINFO> HkGetPlayers();
EXPORT HK_ERROR HkGetConnectionStats(uint iClientID, DPN_CONNECTION_INFO &ci);
EXPORT HK_ERROR HkSetAdmin(const wstring &wscCharname, const wstring &wscRights);
EXPORT HK_ERROR HkGetAdmin(const wstring &wscCharname, wstring &wscRights);
EXPORT HK_ERROR HkDelAdmin(const wstring &wscCharname);
EXPORT HK_ERROR HkChangeNPCSpawn(bool bDisable);
EXPORT HK_ERROR HkGetBaseStatus(const wstring &wscBasename, float &fHealth, float &fMaxHealth);
EXPORT Fuse* HkGetFuseFromID(uint iFuseID);
EXPORT bool __stdcall HkLightFuse(IObjRW *ship, uint iFuseID, float fDelay=0, float fLifetime=0, float fSkip=-1.0f);
EXPORT bool __stdcall HkUnLightFuse(IObjRW *ship, uint iFuseID, float fDunno=0.0f);
void HkTest(int iArg, int iArg2, int iArg3);

// HkFLIni
EXPORT HK_ERROR HkFLIniGet(const wstring &wscCharname, const wstring &wscKey, wstring &wscRet);
EXPORT HK_ERROR HkFLIniWrite(const wstring &wscCharname, const wstring &wscKey, wstring wscValue);

EXPORT wstring HkErrGetText(HK_ERROR hkErr);
void ClearClientInfo(uint iClientID);
void LoadUserSettings(uint iClientID);

// HkCbUserCmd
bool UserCmd_Process(uint iClientID, const wstring &wscCmd);
EXPORT void UserCmd_SetDieMsg(uint iClientID, wstring &wscParam);
EXPORT void UserCmd_SetChatFont(uint iClientID, wstring &wscParam);
EXPORT void PrintUserCmdText(uint iClientID, wstring wscText, ...);

// HkDeath
void ShipDestroyedHook();
void BaseDestroyed(uint iObject, uint iClientIDBy);

// HkDamage
void _HookMissileTorpHit();
void _HkCb_AddDmgEntry();
void _HkCb_GeneralDmg();
void _HkCb_GeneralDmg2();
bool AllowPlayerDamage(uint iClientID, uint iClientIDTarget);
void _HkCb_NonGunWeaponHitsBase();
extern FARPROC fpOldNonGunWeaponHitsBase;
EXPORT extern bool g_gNonGunHitsBase;
EXPORT extern float g_LastHitPts;

// HkCbCallbacks
void _SendMessageHook();
void __stdcall HkCb_SendChat(uint iId, uint iTo, uint iSize, void *pRDL);

// HkCbDisconnect
void _DisconnectPacketSent();
extern FARPROC fpOldDiscPacketSent;

// HkIEngine
namespace HkIEngine
{
	int __cdecl FreeReputationVibe(int const &p1);
	void __cdecl Update_Time(double);
	void __stdcall Elapse_Time(float p1);
	int __cdecl Dock_Call(unsigned int const &,unsigned int const &,int,enum DOCK_HOST_RESPONSE);
	void _LaunchPos();
	void _CShip_init();
	void _CShip_destroy();
	void _HkLoadRepFromCharFile();

	extern FARPROC fpOldLaunchPos;
	extern FARPROC fpOldInitCShip;
	extern FARPROC fpOldDestroyCShip;
	extern FARPROC fpOldLoadRepCharFile;
}

// HkTimers
void HkTimerCheckKick();
void HkTimerNPCAndF1Check();
void HkThreadResolver();
void HkTimerCheckResolveResults();

extern EXPORT list<BASE_INFO> lstBases;
extern CRITICAL_SECTION csIPResolve;
extern list<RESOLVE_IP> g_lstResolveIPs;
extern list<RESOLVE_IP> g_lstResolveIPsResult;
extern HANDLE hThreadResolver;

// namespaces
namespace HkIServerImpl 
{
	void __stdcall SubmitChat(struct CHAT_ID cId, unsigned long lP1, void const *rdlReader, struct CHAT_ID cIdTo, int iP2);
	int __stdcall Update(void);
	bool __stdcall Startup(struct SStartupInfo const &p1);
	void __stdcall Shutdown(void);
	EXPORT extern bool g_bInSubmitChat;
	EXPORT extern uint g_iTextLen;
	extern HOOKENTRY hookEntries[85];
}

// HkDataBaseMarket
bool HkLoadBaseMarket();


// variables

extern EXPORT list<PLUGIN_HOOKDATA>* pPluginHooks;
extern EXPORT list<PLUGIN_DATA> lstPlugins;

extern EXPORT HkIClientImpl* FakeClient;
extern EXPORT HkIClientImpl* HookClient;
extern EXPORT char* OldClient;

extern EXPORT uint iDmgTo;
extern EXPORT uint iDmgToSpaceID;

extern EXPORT bool g_bMsg;
extern EXPORT bool g_bMsgS;
extern EXPORT bool g_bMsgU;

extern FARPROC fpOldShipDestroyed;
extern FARPROC fpOldMissileTorpHit;
extern FARPROC fpOldGeneralDmg, fpOldGeneralDmg2;

extern EXPORT CDPClientProxy **g_cClientProxyArray;
extern EXPORT void *pClient;

extern EXPORT _RCSendChatMsg RCSendChatMsg;
extern EXPORT _CRCAntiCheat CRCAntiCheat;
extern EXPORT _CreateChar CreateChar;

extern EXPORT string scAcctPath;

#define MAX_CLIENT_ID 249
extern EXPORT CLIENT_INFO ClientInfo[MAX_CLIENT_ID+1];
extern EXPORT CDPServer *cdpSrv;
extern EXPORT uint g_iServerLoad;
extern EXPORT uint g_iPlayerCount;
extern EXPORT bool g_bNPCDisabled;
extern EXPORT char *g_FLServerDataPtr;

extern EXPORT bool g_bPlugin_nofunctioncall;


// help

typedef bool (*_HelpEntryDisplayed)(uint);
struct stHelpEntry {
	wstring wszCommand;
	wstring wszArguments;
	wstring wszShortHelp;
	wstring wszLongHelp;
	_HelpEntryDisplayed fnIsDisplayed;
};

extern list<stHelpEntry> lstHelpEntries;
extern EXPORT bool get_bTrue(uint iClientID);
extern EXPORT void HkAddHelpEntry(const wstring &wscCommand, const wstring &wscArguments, const wstring & wscShortHelp, const wstring &wscLongHelp, _HelpEntryDisplayed fnIsDisplayed);
extern EXPORT void HkRemoveHelpEntry(const wstring &wscCommand, const wstring &wscArguments);

inline HK_ERROR HkGetClientID(bool& bIdString, uint& iClientID, const wstring &wscCharname) {
	bIdString = wscCharname.find(L"id ") == 0;

	HK_ERROR hkErr = HkResolveId(wscCharname, iClientID);
	if(hkErr != HKE_OK) {
	    if(hkErr == HKE_INVALID_ID_STRING) {
			hkErr = HkResolveShortCut(wscCharname, iClientID);
			if((hkErr == HKE_AMBIGUOUS_SHORTCUT) || (hkErr == HKE_NO_MATCHING_PLAYER))
				return hkErr;
			if(hkErr == HKE_INVALID_SHORTCUT_STRING)
				iClientID = HkGetClientIdFromCharname(wscCharname);
		} else
			return hkErr;
	}
}

#define HK_GET_CLIENTID(a, b) \
	bool bIdString = false; uint a = uint(-1); \
    HkGetClientID(bIdString, a, b);

#endif