#pragma once

#pragma warning(disable : 4311 4786)

// includes
#include "Singleton.h"
#include "blowfish.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <functional>
#include <list>
#include <map>
#include <set>
#include <sstream>
#include <stdio.h>
#include <string>
#include <variant>
#include <windows.h>

typedef void* (*st6_malloc_t)(size_t);
typedef void (*st6_free_t)(void*);

extern EXPORT st6_malloc_t st6_malloc;
extern EXPORT st6_free_t st6_free;
#define ST6_ALLOCATION_DEFINED

#include <FLCoreCommon.h>
#include <FLCoreDALib.h>
#include <FLCoreRemoteClient.h>
#include <FLCoreServer.h>

constexpr int TimeUpdate = 50;
#define IMPORT __declspec(dllimport)
#define EXPORT __declspec(dllexport)

// typedefs
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned __int64 mstime;

// structures
struct INISECTIONVALUE
{
	std::string scKey;
	std::string scValue;
};

struct MULTIKILLMESSAGE
{
	uint iKillsInARow;
	std::wstring wscMessage;
};

// functions
bool FLHookInit();
void FLHookInit_Pre();
void FLHookShutdown();
EXPORT void ProcessEvent(std::wstring wscText, ...);
void LoadSettings();
void ProcessPendingCommands();

enum class LogLevel : int
{
	Trace,
	Debug,
	Info,
	Warn,
	Err,
	Critical
};

enum LogType
{
	Normal,
	Cheater,
	Kick,
	Connects,
	AdminCmds,
	UserLogCmds,
	SocketCmds,
	PerfTimers
};

// Tools
class EXPORT Console
{
	static void Log(std::wstring& wStr, va_list args, void* addr);

	// Might use more later?
	enum class ConsoleColor
	{
		BLUE = 0x0001,
		GREEN = 0x0002,
		CYAN = BLUE | GREEN,
		RED = 0x0004,
		PURPLE = RED | BLUE,
		YELLOW = RED | GREEN,
		WHITE = RED | BLUE | GREEN,

		STRONG_BLUE = 0x0008 | BLUE,
		STRONG_GREEN = 0x0008 | GREEN,
		STRONG_CYAN = 0x0008 | CYAN,
		STRONG_RED = 0x0008 | RED,
		STRONG_PURPLE = 0x0008 | PURPLE,
		STRONG_YELLOW = 0x0008 | YELLOW,
		STRONG_WHITE = 0x0008 | WHITE,
	};

  public:
	// String
	static void ConPrint(std::string str, ...);
	static void ConErr(std::string str, ...);
	static void ConWarn(std::string str, ...);
	static void ConInfo(std::string str, ...);
	static void ConDebug(std::string str, ...);

	// Wide-string
	static void ConPrint(std::wstring wStr, ...);
	static void ConErr(std::wstring wStr, ...);
	static void ConWarn(std::wstring wStr, ...);
	static void ConInfo(std::wstring wStr, ...);
	static void ConDebug(std::wstring wStr, ...);
};

EXPORT std::wstring stows(const std::string& scText);
EXPORT std::string wstos(const std::wstring& wscText);
EXPORT std::string IniGetS(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::string& scDefault);
EXPORT int IniGetI(const std::string& scFile, const std::string& scApp, const std::string& scKey, int iDefault);
EXPORT bool IniGetB(const std::string& scFile, const std::string& scApp, const std::string& scKey, bool bDefault);
EXPORT void IniWrite(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::string& scValue);
EXPORT void WriteProcMem(void* pAddress, const void* pMem, int iSize);
EXPORT void ReadProcMem(void* pAddress, void* pMem, int iSize);
EXPORT int ToInt(const std::wstring& wscStr);
EXPORT uint ToUInt(const std::wstring& wscStr);
EXPORT std::wstring XMLText(const std::wstring& wscText);
EXPORT std::wstring GetParam(const std::wstring& wscLine, wchar_t wcSplitChar, uint iPos);
EXPORT std::string GetParam(std::string scLine, char cSplitChar, uint iPos);
EXPORT std::wstring ReplaceStr(const std::wstring& wscSource, const std::wstring& wscSearchFor, const std::wstring& wscReplaceWith);
EXPORT void IniDelSection(const std::string& scFile, const std::string& scApp);
EXPORT void IniDelete(const std::string& scFile, const std::string& scApp, const std::string& scKey);
EXPORT void IniWriteW(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::wstring& wscValue);
EXPORT std::wstring IniGetWS(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::wstring& wscDefault);
EXPORT std::wstring ToMoneyStr(int iCash);
EXPORT float IniGetF(const std::string& scFile, const std::string& scApp, const std::string& scKey, float fDefault);
EXPORT void IniGetSection(const std::string& scFile, const std::string& scApp, std::list<INISECTIONVALUE>& lstValues);
EXPORT float ToFloat(const std::wstring& wscStr);
EXPORT mstime timeInMS();
EXPORT void SwapBytes(void* ptr, uint iLen);
EXPORT FARPROC PatchCallAddr(char* hMod, DWORD dwInstallAddress, char* dwHookFunction);
template<typename Str>
Str Trim(const Str& scIn);
template EXPORT std::string Trim(const std::string& scIn);
template EXPORT std::wstring Trim(const std::wstring& scIn);
EXPORT BOOL FileExists(LPCTSTR szPath);
EXPORT std::wstring ToLower(std::wstring wscStr);
EXPORT std::string ToLower(std::string wscStr);
EXPORT std::wstring GetParamToEnd(const std::wstring& wscLine, wchar_t wcSplitChar, uint iPos);
EXPORT void ini_write_wstring(FILE* file, const std::string& parmname, const std::wstring& in);
EXPORT void ini_get_wstring(INI_Reader& ini, std::wstring& wscValue);
EXPORT std::wstring GetTimeString(bool bLocalTime);
EXPORT std::string GetUserFilePath(std::variant<uint, std::wstring> player, const std::string& scExtension);
EXPORT mstime GetTimeInMS();
EXPORT void AddLog(LogType LogType, LogLevel level, std::wstring wStr, ...);

// variables
extern EXPORT HANDLE hProcFL;
extern EXPORT HMODULE hModServer;
extern EXPORT HMODULE hModCommon;
extern EXPORT HMODULE hModRemoteClient;
extern EXPORT HMODULE hModDPNet;
extern EXPORT HMODULE hModDaLib;
extern EXPORT HMODULE hModContent;
extern EXPORT FARPROC fpOldUpdate;

// A base class/struct used for denoting that a class can be scanned.
// Reflectable values are int, uint, bool, float, string, Reflectable, and std::vectors of the previous types.
// Reflectables are interepreted as headers of the provided name.
// Circular references are not handled and will crash.
// Marking a field as reflectable without properly initalizing it will crash upon attempted deserialization.
// Ensure that the default CTOR initalizes all fields.
struct EXPORT Reflectable
{
	virtual ~Reflectable() = default;
	virtual std::string File() { return {}; }
};

#include <Serialization.hpp>

struct EXPORT FLHookConfig final : Reflectable, Singleton<FLHookConfig>
{
	std::string File() override;

	struct General final : Reflectable
	{
		bool autobuy = false;
		uint antiDockKill = 4000;
		uint antiF1 = 0;
		bool changeCruiseDisruptorBehaviour = 0;
		bool debugMode = false;
		bool dieMsg = true;
		bool disableCharfileEncryption = false;
		uint disconnectDelay = 0;
		uint disableNPCSpawns = 0;
		bool dockingMessages = true;
		bool localTime = false;
		uint maxGroupSize = 8;
		bool persistGroup = false;
		uint reservedSlots = 0;
		float torpMissileBaseDamageMultiplier = 1.0f;
		bool logPerformanceTimers = false;

		std::vector<std::wstring> chatSuppressList;
		std::vector<std::string> noPVPSystems;

		std::vector<uint> noPVPSystemsHashed;

		uint antiBaseIdle = 600;
		uint antiCharMenuIdle = 600;
	};

	struct Plugins final : Reflectable
	{
		bool loadAllPlugins = true;
		std::vector<std::string> plugins = {};
	};

	struct MsgStyle final : Reflectable
	{
		std::wstring deathMsgStyle = L"0x19198C01";
		std::wstring deathMsgStyleSys = L"0x1919BD01";
		uint kickMsgPeriod = 5000;
		std::wstring kickMsg = LR"(<TRA data=" 0x0000FF10 " mask=" - 1 "/><TEXT>You will be kicked. Reason: %reason</TEXT>)";
		std::wstring userCmdStyle = L"0x00FF0090";
		std::wstring adminCmdStyle = L"0x00FF0090";
		std::wstring deathMsgTextAdminKill = L"Death: %victim was killed by an admin";
		std::wstring deathMsgTextPlayerKill = L"Death: %victim was killed by %killer (%type)";
		std::wstring deathMsgTextSelfKill = L"Death: %victim killed himself (%type)";
		std::wstring deathMsgTextNPC = L"Death: %victim was killed by an NPC";
		std::wstring deathMsgTextSuicide = L"Death: %victim committed suicide";

	};

	struct Socket final : Reflectable
	{
		bool activated = false;
		int port = 1919;
		int wPort = 1920;
		int ePort = 1921;
		int eWPort = 1922;
		std::string encryptionKey = "SomeRandomKey000";

		BLOWFISH_CTX* bfCTX = nullptr;
		std::map<std::wstring, std::string> passRightsMap = {{L"SuperSecret", "superadmin"}};
	};

	struct UserCommands final : Reflectable
	{
		bool userCmdSetDieMsgSize = true;
		bool userCmdSetDieMsg = true;
		bool userCmdSetChatFont = true;
		bool userCmdIgnore = true;
		bool userCmdHelp = true;
		uint userCmdMaxIgnoreList = true;
		bool defaultLocalChat = false;
	};

	struct MultiKillMessages final : Reflectable
	{
		bool active = false;
		std::wstring multiKillMessageStyle = L"0x1919BD01";
		std::map<std::wstring, int> multiKillMessages = {
			{ L"%player is on a rampage", 5},
			{ L"%player runs amok", 10 },
			{ L"%player is godlike", 15 }
		};
	};

	struct Bans final : Reflectable
	{
		bool banAccountOnMatch = false;
		std::vector<std::wstring> banWildcardsAndIPs;
	};

	General general;
	Plugins plugins;
	Socket socket;
	MsgStyle msgStyle;
	UserCommands userCommands;
	MultiKillMessages multiKillMessages;
	Bans bans;
};

REFL_AUTO(type(FLHookConfig::General), field(antiBaseIdle), field(antiCharMenuIdle), field(autobuy), field(antiDockKill), field(antiF1),
    field(changeCruiseDisruptorBehaviour), field(debugMode), field(dieMsg),
    field(disableCharfileEncryption), field(disconnectDelay), field(disableNPCSpawns), field(dockingMessages), field(localTime), field(maxGroupSize),
    field(persistGroup), field(reservedSlots), field(torpMissileBaseDamageMultiplier), field(logPerformanceTimers), field(chatSuppressList),
    field(noPVPSystems))
REFL_AUTO(type(FLHookConfig::Plugins), field(loadAllPlugins), field(plugins))
REFL_AUTO(type(FLHookConfig::Socket), field(activated), field(port), field(wPort), field(ePort), field(eWPort), field(encryptionKey), field(passRightsMap))
REFL_AUTO(type(FLHookConfig::MsgStyle), field(deathMsgStyle), field(deathMsgStyleSys), field(kickMsgPeriod), field(kickMsg), field(userCmdStyle),
    field(adminCmdStyle), field(deathMsgTextAdminKill), field(deathMsgTextPlayerKill), field(deathMsgTextSelfKill), field(deathMsgTextNPC), field(deathMsgTextSuicide))
REFL_AUTO(type(FLHookConfig::UserCommands), field(userCmdSetDieMsg), field(userCmdSetDieMsgSize), field(userCmdSetChatFont), field(userCmdIgnore), field(userCmdHelp), field(userCmdMaxIgnoreList), field(defaultLocalChat))
REFL_AUTO(type(FLHookConfig::MultiKillMessages), field(active), field(multiKillMessageStyle), field(multiKillMessages))
REFL_AUTO(type(FLHookConfig::Bans), field(banAccountOnMatch), field(banWildcardsAndIPs))
REFL_AUTO(type(FLHookConfig), field(general), field(plugins), field(socket), field(msgStyle), field(userCommands), field(multiKillMessages), field(bans))

struct SYSTEMINFO
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

struct ZONE
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

class JUMPPOINT
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
	uint jumpID;

	/** The jump point destination system id */
	uint jumpDestSysID;
};

struct LOOTABLE_ZONE
{
	/** The zone nickname */
	std::string zoneNick;

	/** The id of the system for this lootable zone */
	uint systemID;

	/** The nickname and arch id of the loot dropped by the asteroids */
	std::string lootNick;
	uint iLootID;

	/** The arch id of the crate the loot is dropped in */
	uint iCrateID;

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
typedef std::multimap<uint, LOOTABLE_ZONE, std::less<>> zone_map_t;

/** A map of system id to system info */
extern EXPORT std::map<uint, SYSTEMINFO> mapSystems;

/** A map of system id to zones */
extern EXPORT std::multimap<uint, ZONE> zones;

/** A map of system id to jumppoint info */
extern EXPORT std::multimap<uint, JUMPPOINT> jumpPoints;

namespace ZoneUtilities
{
	EXPORT void ReadUniverse(zone_map_t* set_mmapZones);
	EXPORT void ReadLootableZone(zone_map_t& set_mmapZones, const std::string& systemNick, const std::string& defaultZoneNick, const std::string& file);
	EXPORT void ReadSystemLootableZones(zone_map_t& set_mmapZones, const std::string& systemNick, const std::string& file);
	EXPORT void ReadSystemZones(zone_map_t& set_mmapZones, const std::string& systemNick, const std::string& file);
	EXPORT bool InZone(uint systemID, const Vector& pos, ZONE& rlz);
	EXPORT bool InDeathZone(uint systemID, const Vector& pos, ZONE& rlz);
	EXPORT SYSTEMINFO* GetSystemInfo(uint systemID);
	EXPORT void PrintZones();
} // namespace ZoneUtilities