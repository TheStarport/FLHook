#pragma once

#include "plugin.h"

struct HookEntry
{
	FARPROC fpProc;
	long dwRemoteAddress;
	FARPROC fpOldProc;
};

struct CARGO_INFO
{
	uint iId;
	int iCount;
	uint iArchId;
	float fStatus;
	bool bMission;
	bool bMounted;
	CacheString hardpoint;
};

// money stuff
struct MONEY_FIX
{
	std::wstring character;
	uint uAmount;

	bool operator==(MONEY_FIX mf1) const
	{
		if (!character.compare(mf1.character))
			return true;

		return false;
	};
};

// ignore
struct IGNORE_INFO
{
	std::wstring character;
	std::wstring wscFlags;
};

// resolver
struct RESOLVE_IP
{
	ClientId client;
	uint iConnects;
	std::wstring wscIP;
	std::wstring wscHostname;
};

struct CLIENT_INFO
{
	// kill msgs
	uint ship;
	uint shipOld;
	mstime tmSpawnTime;

	DamageList dmgLast;

	// money cmd
	std::list<MONEY_FIX> lstMoneyFix;

	// anticheat
	uint iTradePartner;

	// change cruise disruptor behaviour
	bool bCruiseActivated;
	bool bThrusterActivated;
	bool bEngineKilled;
	bool bTradelane;

	// idle kicks
	uint iBaseEnterTime;
	uint iCharMenuEnterTime;

	// msg, wait and kick
	mstime tmKickTime;

	// eventmode
	uint iLastExitedBaseId;
	bool bDisconnected;

	// f1 laming
	bool bCharSelected;
	mstime tmF1Time;
	mstime tmF1TimeDisconnect;

	// ignore usercommand
	std::list<IGNORE_INFO> lstIgnore;

	// user settings
	DIEMSGTYPE dieMsg;
	CHATSIZE dieMsgSize;
	CHATSTYLE dieMsgStyle;
	CHATSIZE chatSize;
	CHATSTYLE chatStyle;

	// MultiKillMessages
	uint iKillsInARow;

	// bans
	uint iConnects; // incremented when player connects

	// Group
	uint iGroupId;

	// other
	std::wstring wscHostname;

	bool bSpawnProtected;
	bool bUseServersideHitDetection; // used by AC Plugin

	// Your randomly assigned formation tag, e.g. Navy Lambda 1-6
	uint formationNumber1;
	uint formationNumber2;
	uint formationTag;
};

// taken from directplay
typedef struct _DPN_CONNECTION_INFO
{
	DWORD dwSize;
	DWORD dwRoundTripLatencyMS;
	DWORD dwThroughputBPS;
	DWORD dwPeakThroughputBPS;
	DWORD dwBytesSentGuaranteed;
	DWORD dwPacketsSentGuaranteed;
	DWORD dwBytesSentNonGuaranteed;
	DWORD dwPacketsSentNonGuaranteed;
	DWORD dwBytesRetried;
	DWORD dwPacketsRetried;
	DWORD dwBytesDropped;
	DWORD dwPacketsDropped;
	DWORD dwMessagesTransmittedHighPriority;
	DWORD dwMessagesTimedOutHighPriority;
	DWORD dwMessagesTransmittedNormalPriority;
	DWORD dwMessagesTimedOutNormalPriority;
	DWORD dwMessagesTransmittedLowPriority;
	DWORD dwMessagesTimedOutLowPriority;
	DWORD dwBytesReceivedGuaranteed;
	DWORD dwPacketsReceivedGuaranteed;
	DWORD dwBytesReceivedNonGuaranteed;
	DWORD dwPacketsReceivedNonGuaranteed;
	DWORD dwMessagesReceived;
} DPN_CONNECTION_INFO, *PDPN_CONNECTION_INFO;

struct PlayerInfo
{
	uint client;
	std::wstring character;
	std::wstring wscBase;
	std::wstring wscSystem;
	uint iSystem;
	uint ship;
	DPN_CONNECTION_INFO connectionInfo;
	std::wstring wscIP;
	std::wstring wscHostname;
};

struct BaseHealth
{
	float currentHealth;
	float maxHealth;
};

struct PatchInfoEntry
{
	ulong pAddress;
	void* pNewValue;
	uint iSize;
	void* pOldValue;
	bool bAlloced;
};

struct PatchInfo
{
	const char* szBinName;
	ulong pBaseAddress;

	PatchInfoEntry piEntries[128];
};

struct DataMarketItem
{
	uint iArchId;
	float fRep;
};

struct BaseInfo
{
	uint baseId;
	std::string scBasename;
	uint iObjectId;
	bool bDestroyed;
	std::list<DataMarketItem> lstMarketMisc;
};

struct GroupMember
{
	ClientId client;
	std::wstring character;
};

struct SpecialChatIds
{
	enum : uint
	{
		CONSOLE = 0,

		PLAYER_MIN = 1,
		PLAYER_MAX = 249,

		SPECIAL_BASE = 0x10000,
		UNIVERSE = SPECIAL_BASE | 0,
		SYSTEM = SPECIAL_BASE | 1,
		LOCAL = SPECIAL_BASE | 2,
		GROUP = SPECIAL_BASE | 3,
		GROUP_EVENT = SPECIAL_BASE | 4
	};
};

struct SystemInfo
{
	/** The system nickname */
	std::string sysNick;

	/** The system id */
	uint systemId;

	/** The system scale */
	float scale;
};

struct TransformMatrix
{
	float d[4][4];
};

struct Zone
{
	/** The system nickname */
	std::string sysNick;

	/** The zone nickname */
	std::string zoneNick;

	/** The id of the system for this zone */
	uint systemId;

	/** The zone transformation matrix */
	TransformMatrix transform;

	/** The zone ellipsoid size */
	Vector size;

	/** The zone position */
	Vector pos;

	/** The damage this zone causes per second */
	int damage;

	/** Is this an encounter zone */
	bool encounter;
};

class JumpPoint
{
  public:
	/** The system nickname */
	std::string sysNick;

	/** The jump point nickname */
	std::string jumpNick;

	/** The jump point destination system nickname */
	std::string jumpDestSysNick;

	/** The id of the system for this jump point. */
	uint System;

	/** The id of the jump point. */
	uint jumpId;

	/** The jump point destination system id */
	uint jumpDestSysId;
};

struct LootableZone
{
	/** The zone nickname */
	std::string zoneNick;

	/** The id of the system for this lootable zone */
	uint systemId;

	/** The nickname and arch id of the loot dropped by the asteroids */
	std::string lootNick;
	uint iLootId;

	/** The arch id of the crate the loot is dropped in */
	uint iCrateId;

	/** The minimum number of loot items to drop */
	uint iMinLoot;

	/** The maximum number of loot items to drop */
	uint iMaxLoot;

	/** The drop difficultly */
	uint iLootDifficulty;

	/** The lootable zone ellipsoid size */
	Vector size;

	/** The lootable zone position */
	Vector pos;
};

struct Light
{
	std::string nickname;
	uint archId;
	float bulbSize;
	float glowSize;
	float intensity;
	Vector glowColor;
	Vector color;
	Vector minColor;
	bool dockingLight;
	bool alwaysOn;
	float flareConeMin;
	float flareConeMax;
	int lightSourceCone;

	bool blinks;
	float delay;
	float blinkDuration;
};