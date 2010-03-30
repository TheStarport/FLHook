#ifndef _HOOK_
#define _HOOK_

#include <time.h>
#if _MSC_VER == 1200
#include "xtrace.h" // __FUNCTION__ macro for vc6
#endif
#include "global.h"
#include "flcodec.h"
#include "./include/FLCoreServer.h"
#include "./include/FLCoreCommon.h"
#include "./include/FLCoreRemoteClient.h"
#include "./include/FLCoreDALib.h"

#include "./include/plugin.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// defines

#define HKHKSUCCESS(a) ((a) == HKE_OK)
#define HKSUCCESS(a) ((hkLastErr = (a)) == HKE_OK)
#define HKFAILED(a) !HKSUCCESS(a)

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
#define ADDR_COMMON_VFTABLE_MINE 0x139C64
#define ADDR_COMMON_VFTABLE_CM 0x139C90
#define ADDR_COMMON_VFTABLE_GUN 0x139C38
#define ADDR_COMMON_VFTABLE_SHIELDGEN 0x139BB4
#define ADDR_COMMON_VFTABLE_THRUSTER 0x139BE0
#define ADDR_COMMON_VFTABLE_SHIELDBAT 0x1399FC
#define ADDR_COMMON_VFTABLE_NANOBOT 0x1399D0
#define ADDR_COMMON_VFTABLE_MUNITION 0x139CE8
#define ADDR_COMMON_VFTABLE_ENGINE 0x139AAC


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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// plugin functionality & hook prototypes

class CCmds;

struct PLUGIN_HOOKDATA
{
	string sName;
	HMODULE hDLL;
	int iPriority;
	bool bPaused;
};

class CTimer
{
public:
	CTimer(string sFunction, uint iWarning);
    void start();
	void stop();

private:
	mstime tmStart;
	uint iMax;
	string sFunction;
	uint iWarning;
};

#define CALL_PLUGIN_FUNCTION(hPluginDLL,sPluginName,func_proto,args) \
	void* vPluginRetTemp = NULL; \
	if(hPluginDLL) { \
		func_proto fpDLLCall = (func_proto)GetProcAddress(hPluginDLL, __FUNCDNAME__); \
		if(fpDLLCall) { \
			string sPluginFunction = sPluginName; \
			sPluginFunction += " - "; \
			sPluginFunction += __FUNCTION__; \
			static CTimer timer(sPluginFunction,set_iTimerThreshold); \
			timer.start(); \
			try { \
				fpDLLCall args; \
				__asm { mov [vPluginRetTemp], eax } \
			} catch(...) { AddLog("Exception in plugin %s", sPluginFunction.c_str()); } \
			timer.stop(); \
		} else  \
			AddLog("Error: Plugin '%s' does not export %s", sPluginName.c_str(), __FUNCTION__); \
	}\


#define CALL_PLUGINS(func_proto,args) \
	void* vPluginRet; \
	bool bPluginReturn = false; \
	try { \
		std::map<string, list<PLUGIN_HOOKDATA>*>::iterator iter; \
		iter = mpPluginHooks.find((string)__FUNCTION__); \
		if(iter != mpPluginHooks.end()) { \
			foreach((*(iter->second)),PLUGIN_HOOKDATA, itplugin) { \
				if(itplugin->bPaused) \
					continue; \
				CALL_PLUGIN_FUNCTION(itplugin->hDLL,itplugin->sName,func_proto,args); \
				PLUGIN_Get_PluginReturnCode Plugin_ReturnCode = (PLUGIN_Get_PluginReturnCode)GetProcAddress(itplugin->hDLL, "?Get_PluginReturnCode@@YA?AW4PLUGIN_RETURNCODE@@XZ"); \
				if(Plugin_ReturnCode) { \
					PLUGIN_RETURNCODE plugin_returncode = Plugin_ReturnCode(); \
					g_bPlugin_nofunctioncall = false; \
					if(plugin_returncode == SKIPPLUGINS_NOFUNCTIONCALL) { \
						bPluginReturn = true; \
						vPluginRet = vPluginRetTemp; \
						break; \
					} else if(plugin_returncode == NOFUNCTIONCALL) { \
						bPluginReturn = true; \
						g_bPlugin_nofunctioncall = true; \
						vPluginRet = vPluginRetTemp; \
					} else if (plugin_returncode == SKIPPLUGINS) \
						break; \
				} \
			} \
		}\
	} catch(...) { AddLog("Exception in PluginCalls @ %s", __FUNCTION__); } \


typedef PLUGIN_RETURNCODE (*PLUGIN_Get_PluginReturnCode)();
typedef void (*PLUGIN_Plugin_Communication)(PLUGIN_MESSAGE msg, void* data);

// plugin callback hooks
typedef void (__stdcall *PLUGIN_HkIServerImpl_Shutdown)(void);
typedef void (__stdcall *PLUGIN_HkIServerImpl_Startup)(struct SStartupInfo const &p1);
typedef void (*PLUGIN_HkTimerCheckKick)();
typedef int (__stdcall *PLUGIN_HkIServerImpl_Update)();
typedef void (__stdcall *PLUGIN_HkIServerImpl_SubmitChat)(CHAT_ID, unsigned long, void const *,CHAT_ID,int);
typedef void (__stdcall *PLUGIN_HkIServerImpl_PlayerLaunch)(unsigned int iShip, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_FireWeapon)(unsigned int iClientID, struct XFireWeaponInfo const &wpn);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SPMunitionCollision)(struct SSPMunitionCollisionInfo const & ci, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SPObjUpdate)(struct SSPObjUpdateInfo const &ui, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SPObjCollision)(struct SSPObjCollisionInfo const &ci, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_LaunchComplete)(unsigned int iBaseID, unsigned int iShip);
typedef void (__stdcall *PLUGIN_HkIServerImpl_CharacterSelect)(struct CHARACTER_ID const & cId, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_BaseEnter)(unsigned int iBaseID, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_BaseExit)(unsigned int iBaseID, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_OnConnect)(unsigned int);
typedef void (__stdcall *PLUGIN_HkIServerImpl_DisConnect)(unsigned int iClientID, enum EFLConnection p2);	
typedef void (__stdcall *PLUGIN_HkIServerImpl_TerminateTrade)(unsigned int iClientID, int iAccepted);	
typedef void (__stdcall *PLUGIN_HkIServerImpl_InitiateTrade)(unsigned int iClientID1, unsigned int iClientID2);	
typedef void (__stdcall *PLUGIN_HkIServerImpl_ActivateEquip)(unsigned int iClientID, struct XActivateEquip const &aq);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ActivateCruise)(unsigned int iClientID, struct XActivateCruise const &ac);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ActivateThrusters)(unsigned int iClientID, struct XActivateThrusters const &at);
typedef void (__stdcall *PLUGIN_HkIServerImpl_GFGoodSell)(struct SGFGoodSellInfo const &gsi, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_CharacterInfoReq)(unsigned int iClientID, bool p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_JumpInComplete)(unsigned int iSystemID, unsigned int iShip);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SystemSwitchOutComplete)(unsigned int iShip, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_Login)(struct SLoginInfo const &li, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_MineAsteroid)(unsigned int p1, class Vector const &vPos, unsigned int iLookID, unsigned int iGoodID, unsigned int iCount, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_GoTradelane)(unsigned int iClientID, struct XGoTradelane const &gtl);
typedef void (__stdcall *PLUGIN_HkIServerImpl_StopTradelane)(unsigned int iClientID, unsigned int p2, unsigned int p3, unsigned int p4);
typedef void (__stdcall *PLUGIN_HkIServerImpl_AbortMission)(unsigned int p1, unsigned int p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_AcceptTrade)(unsigned int iClientID, bool p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_AddTradeEquip)(unsigned int iClientID, struct EquipDesc const &ed);
typedef void (__stdcall *PLUGIN_HkIServerImpl_BaseInfoRequest)(unsigned int p1, unsigned int p2, bool p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_CommComplete)(unsigned int p1, unsigned int p2, unsigned int p3,enum CommResult cr);
typedef void (__stdcall *PLUGIN_HkIServerImpl_CreateNewCharacter)(struct SCreateCharacterInfo const & scci, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_DelTradeEquip)(unsigned int iClientID, struct EquipDesc const &ed);
typedef void (__stdcall *PLUGIN_HkIServerImpl_DestroyCharacter)(struct CHARACTER_ID const &cId, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_Dock)(unsigned int const &p1, unsigned int const &p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_GFGoodBuy)(struct SGFGoodBuyInfo const &gbi, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_GFGoodVaporized)(struct SGFGoodVaporizedInfo const &gvi, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_GFObjSelect)(unsigned int p1, unsigned int p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_Hail)(unsigned int p1, unsigned int p2, unsigned int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_InterfaceItemUsed)(unsigned int p1, unsigned int p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_JettisonCargo)(unsigned int iClientID, struct XJettisonCargo const &jc);
typedef void (__stdcall *PLUGIN_HkIServerImpl_LocationEnter)(unsigned int p1, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_LocationExit)(unsigned int p1, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_LocationInfoRequest)(unsigned int p1,unsigned int p2, bool p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_MissionResponse)(unsigned int p1, unsigned long p2, bool p3, unsigned int p4);
typedef void (__stdcall *PLUGIN_HkIServerImpl_MissionSaveB)(unsigned int iClientID, unsigned long p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_PopUpDialog)(unsigned int p1, unsigned int p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ReqAddItem)(unsigned int p1, char const *p2, int p3, float p4, bool p5, unsigned int p6);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ReqChangeCash)(int p1, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ReqCollisionGroups)(class std::list<struct CollisionGroupDesc,class std::allocator<struct CollisionGroupDesc> > const &p1, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ReqDifficultyScale)(float p1, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ReqEquipment)(class EquipDescList const &edl, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ReqHullStatus)(float p1, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ReqModifyItem)(unsigned short p1, char const *p2, int p3, float p4, bool p5, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ReqRemoveItem)(unsigned short p1, int p2, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ReqSetCash)(int p1, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_ReqShipArch)(unsigned int p1, unsigned int p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_RequestBestPath)(unsigned int p1, unsigned char *p2, int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_RequestCancel)(int p1, unsigned int p2, unsigned int p3, unsigned long p4, unsigned int p5);
typedef void (__stdcall *PLUGIN_HkIServerImpl_RequestCreateShip)(unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_RequestEvent)(int p1, unsigned int p2, unsigned int p3, unsigned int p4, unsigned long p5, unsigned int p6);
typedef void (__stdcall *PLUGIN_HkIServerImpl_RequestGroupPositions)(unsigned int p1, unsigned char *p2, int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_RequestPlayerStats)(unsigned int p1, unsigned char *p2, int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_RequestRankLevel)(unsigned int p1, unsigned char *p2, int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_RequestTrade)(unsigned int p1, unsigned int p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SPBadLandsObjCollision)(struct SSPBadLandsObjCollisionInfo const &p1, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SPRequestInvincibility)(unsigned int p1, bool p2, enum InvincibilityReason p3, unsigned int p4);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SPRequestUseItem)(struct SSPUseItem const &p1, unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SPScanCargo)(unsigned int const &p1, unsigned int const &p2, unsigned int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SaveGame)(struct CHARACTER_ID const &cId, unsigned short const *p2, unsigned int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SetInterfaceState)(unsigned int p1, unsigned char *p2, int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SetManeuver)(unsigned int iClientID, struct XSetManeuver const &p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SetMissionLog)(unsigned int iClientID, unsigned char *p2, int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SetTarget)(unsigned int iClientID, struct XSetTarget const &p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SetTradeMoney)(unsigned int iClientID, unsigned long p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SetVisitedState)(unsigned int iClientID, unsigned char *p2, int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_SetWeaponGroup)(unsigned int iClientID, unsigned char *p2, int p3);
typedef void (__stdcall *PLUGIN_HkIServerImpl_StopTradeRequest)(unsigned int iClientID);
typedef void (__stdcall *PLUGIN_HkIServerImpl_TractorObjects)(unsigned int iClientID, struct XTractorObjects const &p2);
typedef void (__stdcall *PLUGIN_HkIServerImpl_TradeResponse)(unsigned char const *p1, int p2, unsigned int iClientID);
typedef bool (*PLUGIN_UserCmd_Process)(uint iClientID, wstring wscCmd);
typedef void (*PLUGIN_UserCmd_Help)(uint iClientID, wstring wscParam);
typedef void (__stdcall *PLUGIN_ShipDestroyed)(DamageList *_dmg, char *szECX, uint iKill);
typedef void (*PLUGIN_SendDeathMsg)(wstring wscMsg, uint iSystemID, uint iClientIDVictim, uint iClientIDKiller);
typedef int (__stdcall *PLUGIN_HkCB_MissileTorpHit)(char *ECX, char *p1, DamageList *dmg);
typedef void (__stdcall *PLUGIN_HkCb_AddDmgEntry)(DamageList *dmgList, unsigned short p1, float p2, enum DamageEntry::SubObjFate p3);
typedef void (__stdcall *PLUGIN_HkCb_GeneralDmg)(char *szECX);
typedef bool (*PLUGIN_AllowPlayerDamage)(uint iClientID, uint iClientIDTarget);
typedef void (__stdcall *PLUGIN_HkCb_SendChat)(uint iClientID, uint iTo, uint iSize, void *pRDL);
typedef void (*PLUGIN_ClearClientInfo)(uint iClientID);
typedef void (*PLUGIN_LoadSettings)();
typedef bool (*PLUGIN_ExecuteCommandString_Callback)(CCmds* classptr, wstring wscCmdStr);
typedef void (*PLUGIN_CmdHelp_Callback)(CCmds* classptr);
typedef bool (__stdcall *PLUGIN_LaunchPosHook)(uint iSpaceID, struct CEqObj &p1, Vector &p2, Matrix &p3, int iDock);
typedef int (__cdecl *PLUGIN_HkCb_Dock_Call)(unsigned int const &uShipID,unsigned int const &uSpaceID,int p3,enum DOCK_HOST_RESPONSE p4);
typedef void (__stdcall *PLUGIN_HkCb_Elapse_Time)(float p1);
typedef void (__cdecl *PLUGIN_HkCb_Update_Time)(double dInterval);
typedef void (__cdecl *PLUGIN_BaseDestroyed)(uint iObject, uint iClientIDBy);
typedef bool (__stdcall *PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_CREATESHIP)(uint iClientID, FLPACKET_CREATESHIP& pShip);
typedef void (__cdecl *PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_CREATESHIP_AFTER)(uint iClientID, FLPACKET_CREATESHIP& pShip);
typedef bool (__stdcall *PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP)(uint iClientID, bool bResponse, uint iShipID);
typedef bool (__stdcall *PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_LAUNCH)(uint iClientID, FLPACKET_LAUNCH& pLaunch);
typedef bool (__stdcall *PLUGIN_HkIEngine_CShip_init)(CShip* ship);
typedef bool (__stdcall *PLUGIN_HkIEngine_CShip_destroy)(CShip* ship);



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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prototypes

// HkPluginManager
namespace PluginManager {
	EXPORT void LoadPlugins(bool, CCmds*);
	EXPORT HK_ERROR PausePlugin(string sShortName, bool bPause);
	EXPORT HK_ERROR UnloadPlugin(string sShortName);
	EXPORT void UnloadPlugins();
}

// HkInit
void PatchClientImpl();
bool InitHookExports();
void UnloadHookExports();
void HookRehashed();
void LoadUserCharSettings(uint iClientID);

// HkFuncTools
EXPORT uint HkGetClientIdFromAccount(CAccount *acc);
EXPORT uint HkGetClientIdFromPD(struct PlayerData *pPD);
EXPORT CAccount* HkGetAccountByCharname(wstring wscCharname);
EXPORT uint HkGetClientIdFromCharname(wstring wscCharname);
EXPORT wstring HkGetAccountID(CAccount *acc);
EXPORT bool HkIsEncoded(string scFilename);
EXPORT bool HkIsInCharSelectMenu(wstring wscCharname);
EXPORT bool HkIsInCharSelectMenu(uint iClientID);
EXPORT bool HkIsValidClientID(uint iClientID);
EXPORT HK_ERROR HkResolveId(wstring wscCharname, uint &iClientID);
EXPORT HK_ERROR HkResolveShortCut(wstring wscShortcut, uint &iClientID);
EXPORT uint HkGetClientIDByShip(uint iShip);
EXPORT HK_ERROR HkGetAccountDirName(CAccount *acc, wstring &wscDir);
EXPORT HK_ERROR HkGetAccountDirName(wstring wscCharname, wstring &wscDir);
EXPORT HK_ERROR HkGetCharFileName(wstring wscCharname, wstring &wscFilename);
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
EXPORT HK_ERROR HkMsg(int iClientID, wstring wscMessage);
EXPORT HK_ERROR HkMsg(wstring wscCharname, wstring wscMessage);
EXPORT HK_ERROR HkMsgS(wstring wscSystemname, wstring wscMessage);
EXPORT HK_ERROR HkMsgU(wstring wscMessage);
EXPORT HK_ERROR HkFMsgEncodeXML(wstring wscXML, char *szBuf, uint iSize, uint &iRet);
EXPORT HK_ERROR HkFMsgSendChat(uint iClientID, char *szBuf, uint iSize);
EXPORT HK_ERROR HkFMsg(uint iClientID, wstring wscXML);
EXPORT HK_ERROR HkFMsg(wstring wscCharname, wstring wscXML);
EXPORT HK_ERROR HkFMsgS(wstring wscSystemname, wstring wscXML);
EXPORT HK_ERROR HkFMsgU(wstring wscXML);

// HkFuncPlayers
EXPORT HK_ERROR HkGetCash(wstring wscCharname, int &iCash);
EXPORT HK_ERROR HkAddCash(wstring wscCharname, int iAmount);
EXPORT HK_ERROR HkKick(CAccount *acc);
EXPORT HK_ERROR HkKick(wstring wscCharname);
EXPORT HK_ERROR HkKickReason(wstring wscCharname, wstring wscReason);
EXPORT HK_ERROR HkBan(wstring wscCharname, bool bBan);
EXPORT HK_ERROR HkBeam(wstring wscCharname, wstring wscBasename);
EXPORT HK_ERROR HkSaveChar(wstring wscCharname);
EXPORT HK_ERROR HkEnumCargo(wstring wscCharname, list<CARGO_INFO> &lstCargo, int &iRemainingHoldSize);
EXPORT HK_ERROR HkRemoveCargo(wstring wscCharname, uint iID, int iCount);
EXPORT HK_ERROR HkAddCargo(wstring wscCharname, uint iGoodID, int iCount, bool bMission);
EXPORT HK_ERROR HkAddCargo(wstring wscCharname, wstring wscGood, int iCount, bool bMission);
EXPORT HK_ERROR HkRename(wstring wscCharname, wstring wscNewCharname, bool bOnlyDelete);
EXPORT HK_ERROR HkMsgAndKick(uint iClientID, wstring wscReason, uint iIntervall);
EXPORT HK_ERROR HkKill(wstring wscCharname);
EXPORT HK_ERROR HkGetReservedSlot(wstring wscCharname, bool &bResult);
EXPORT HK_ERROR HkSetReservedSlot(wstring wscCharname, bool bReservedSlot);
EXPORT void HkPlayerAutoBuy(uint iClientID, uint iBaseID);
EXPORT HK_ERROR HkResetRep(wstring wscCharname);
EXPORT HK_ERROR HkGetGroupMembers(wstring wscCharname, list<GROUP_MEMBER> &lstMembers);
EXPORT HK_ERROR HkSetRep(wstring wscCharname, wstring wscRepGroup, float fValue);
EXPORT HK_ERROR HkGetRep(wstring wscCharname, wstring wscRepGroup, float &fValue);
EXPORT HK_ERROR HkReadCharFile(wstring wscCharname, list<wstring> &lstOutput);
EXPORT HK_ERROR HkWriteCharFile(wstring wscCharname, wstring wscData);

// HkFuncLog
EXPORT void AddLog(FILE* fLog, const char *szString, ...);
EXPORT void AddLog(const char *szString, ...);
EXPORT void HkHandleCheater(uint iClientID, bool bBan, wstring wscReason, ...);
EXPORT bool HkAddCheaterLog(wstring wscCharname, wstring wscReason);
EXPORT bool HkAddKickLog(uint iClientID, wstring wscReason, ...);
EXPORT bool HkAddConnectLog(uint iClientID, wstring wscReason, ...);
EXPORT void HkAddAdminCmdLog(const char *szString, ...);
EXPORT void HkAddUserCmdLog(const char *szString, ...);
EXPORT void HkAddPerfTimerLog(const char *szString, ...);

// HkFuncOther
EXPORT void HkGetPlayerIP(uint iClientID, wstring &wscIP);
EXPORT HK_ERROR HkGetPlayerInfo(wstring wscCharname, HKPLAYERINFO &pi, bool bAlsoCharmenu);
EXPORT list<HKPLAYERINFO> HkGetPlayers();
EXPORT HK_ERROR HkGetConnectionStats(uint iClientID, DPN_CONNECTION_INFO &ci);
EXPORT HK_ERROR HkSetAdmin(wstring wscCharname, wstring wscRights);
EXPORT HK_ERROR HkGetAdmin(wstring wscCharname, wstring &wscRights);
EXPORT HK_ERROR HkDelAdmin(wstring wscCharname);
EXPORT HK_ERROR HkChangeNPCSpawn(bool bDisable);
EXPORT HK_ERROR HkGetBaseStatus(wstring wscBasename, float &fHealth, float &fMaxHealth);
void HkTest(int iArg, int iArg2, int iArg3);

// HkFLIni
EXPORT HK_ERROR HkFLIniGet(wstring wscCharname, wstring wscKey, wstring &wscRet);
EXPORT HK_ERROR HkFLIniWrite(wstring wscCharname, wstring wscKey, wstring wscValue);

EXPORT wstring HkErrGetText(HK_ERROR hkErr);
EXPORT void ClearClientInfo(uint iClientID);
EXPORT void LoadUserSettings(uint iClientID);

// HkCbUserCmd
EXPORT bool UserCmd_Process(uint iClientID, wstring wscCmd);
EXPORT void UserCmd_SetDieMsg(uint iClientID, wstring wscParam);
EXPORT void UserCmd_SetChatFont(uint iClientID, wstring wscParam);
EXPORT void PrintUserCmdText(uint iClientID, wstring wscText, ...);

// HkDeath
void ShipDestroyedHook();
void BaseDestroyed(uint iObject, uint iClientIDBy);

// HkDamage
void _HookMissileTorpHit();
void _HkCb_AddDmgEntry();
void _HkCb_GeneralDmg();
bool AllowPlayerDamage(uint iClientID, uint iClientIDTarget);
void _HkCb_NonGunWeaponHitsBase();
extern FARPROC fpOldNonGunWeaponHitsBase;
extern bool g_gNonGunHitsBase;

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

	extern FARPROC fpOldLaunchPos;
	extern FARPROC fpOldInitCShip;
	extern FARPROC fpOldDestroyCShip;
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

extern EXPORT std::map<string, list<PLUGIN_HOOKDATA>*> mpPluginHooks;
extern EXPORT list<PLUGIN_DATA> lstPlugins;

extern EXPORT HkIClientImpl* FakeClient;
extern EXPORT HkIClientImpl* HookClient;
extern EXPORT char* OldClient;

extern EXPORT uint	iDmgTo;
extern EXPORT uint iDmgToSpaceID;

extern EXPORT bool g_bMsg;
extern EXPORT bool g_bMsgS;
extern EXPORT bool g_bMsgU;

extern FARPROC fpOldShipDestroyed;
extern FARPROC fpOldMissileTorpHit;
extern FARPROC fpOldGeneralDmg;

extern EXPORT CDPClientProxy **g_cClientProxyArray;
extern EXPORT void *pClient;

extern EXPORT _RCSendChatMsg RCSendChatMsg;
extern EXPORT _CRCAntiCheat CRCAntiCheat;
extern EXPORT _CreateChar CreateChar;

extern EXPORT string scAcctPath;

extern EXPORT CLIENT_INFO ClientInfo[201];
extern EXPORT CLIENT_INFO_EXT01 ClientInfoExt01[201];
extern EXPORT CDPServer *cdpSrv;
extern EXPORT uint g_iServerLoad;
extern EXPORT bool g_bNPCDisabled;
extern EXPORT char *g_FLServerDataPtr;

extern EXPORT bool g_bPlugin_nofunctioncall;

#endif