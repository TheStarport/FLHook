#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define _WINSOCKAPI_
#include <Windows.h>
#include <cstdio>
#include <string>
#include <set>
#include <list>
#include <ctime>
#include <map>
#include <filesystem>
#include <variant>
#include <numbers>

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "WinSock2.h"

#include "Tools/Hk.hpp"
#include "Tools/Utils.hpp"

// Magic Enum Extensions
using namespace magic_enum::bitwise_operators; // NOLINT
using namespace magic_enum::ostream_operators; // NOLINT

DLL void ProcessEvent(std::wstring text, ...);

// Tools
class DLL Console
{
	static void Log(const std::string& str, void* addr);

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
	static void ConPrint(const std::string& str);
	static void ConErr(const std::string& str);
	static void ConWarn(const std::string& str);
	static void ConInfo(const std::string& str);
	static void ConDebug(const std::string& str);
};

class DLL DataManager : public Singleton<DataManager>
{
  private:
	std::string dataPath;
	std::map<EquipId, Light> lights;

  public:
	DataManager()
	{
		INI_Reader ini;
		ini.open("freelancer.ini", false);
		ini.find_header("Freelancer");
		while (ini.read_value())
		{
			if (ini.is_value("data path"))
			{
				dataPath = ini.get_value_string();
				break;
			}
		}
		ini.close();
	};
	DataManager(const DataManager&) = delete;

#ifdef FLHOOK
	void LoadLights();
#endif

	// Lights
	const std::map<EquipId, Light>& GetLights() const;
	cpp::result<const Light&, Error> FindLightByHash(EquipId hash) const;
};

DLL std::string IniGetS(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::string& scDefault);
DLL int IniGetI(const std::string& scFile, const std::string& scApp, const std::string& scKey, int iDefault);
DLL bool IniGetB(const std::string& scFile, const std::string& scApp, const std::string& scKey, bool bDefault);
DLL void IniWrite(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::string& scValue);
DLL void IniDelSection(const std::string& scFile, const std::string& scApp);
DLL void IniDelete(const std::string& scFile, const std::string& scApp, const std::string& scKey);
DLL void IniWriteW(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::wstring& wscValue);
DLL std::wstring IniGetWS(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::wstring& wscDefault);
DLL float IniGetF(const std::string& scFile, const std::string& scApp, const std::string& scKey, float fDefault);
DLL BOOL FileExists(LPCTSTR szPath);
DLL void ini_write_wstring(FILE* file, const std::string& parmname, const std::wstring& in);
DLL void ini_get_wstring(INI_Reader& ini, std::wstring& wscValue);
DLL std::wstring GetTimeString(bool bLocalTime);
DLL std::string GetUserFilePath(const std::variant<uint, std::wstring>& player, const std::string& scExtension);
DLL void AddLog(LogType LogType, LogLevel lvl, const std::string& str);

// variables
extern DLL HANDLE hProcFL;
extern DLL HMODULE hModServer;
extern DLL HMODULE hModCommon;
extern DLL HMODULE hModRemoteClient;
extern DLL HMODULE hModDPNet;
extern DLL HMODULE hModDaLib;
extern DLL HMODULE hModContent;
extern DLL FARPROC fpOldUpdate;

// A base class/struct used for denoting that a class can be scanned.
// Reflectable values are int, uint, bool, float, string, Reflectable, and std::vectors of the previous types.
// Reflectables are interepreted as headers of the provided name.
// Circular references are not handled and will crash.
// Marking a field as reflectable without properly initalizing it will crash upon attempted deserialization.
// Ensure that the default CTOR initalizes all fields.
struct DLL Reflectable
{
	virtual ~Reflectable() = default;
	virtual std::string File() { return {}; }
};

#include <Tools/Serialization/Serializer.hpp>

struct DLL FLHookConfig final : Reflectable, Singleton<FLHookConfig>
{
	std::string File() override;

	struct General final : Reflectable
	{
		//! Time of invulnerability upon undock in miliseconds.
		//! Protected player can also not inflict any damage.
		uint antiDockKill = 4000;
		uint antiF1 = 0;
		//! Disable the cruise disrupting effects.
		bool changeCruiseDisruptorBehaviour = false;
		//! If true, enables FLHook debug mode.
		bool debugMode = false;

		//! If true, it encodes player characters to bini (binary ini)
		bool disableCharfileEncryption = false;
		//! If a player disconnects in space, their ship will remain in game world for the time specified, in miliseconds.
		uint disconnectDelay = 0;
		//! If above zero, disables NPC spawns if "server load in ms" goes above the specified value.
		uint disableNPCSpawns = 0;

		//! If true, it uses local time when rendering current time instead of server time,
		//! in for example, "/time" function.
		bool localTime = false;
		//! Maximum amount of players in a group.
		uint maxGroupSize = 8;
		bool persistGroup = false;
		//! Number of slots reserved to specified accounts.
		uint reservedSlots = 0;
		//! Global damage multiplier to missile and torpedo type weapons.
		float torpMissileBaseDamageMultiplier = 1.0f;
		bool logPerformanceTimers = false;

		bool tempBansEnabled = true;

		std::vector<std::wstring> chatSuppressList;
		//! Vector of systems where players can't deal damage to one another.
		std::vector<std::string> noPVPSystems;

		std::vector<uint> noPVPSystemsHashed;

		//! Amount of time spent idly on a base resulting in a server kick, in seconds.
		uint antiBaseIdle = 600;
		//! Amount of time spent idly on character select screen resulting in a server kick, in seconds.
		uint antiCharMenuIdle = 600;
	};

	struct Plugins final : Reflectable
	{
		//! If true, loads all plugins on FLHook startup.
		bool loadAllPlugins = true;
		//! Contains a list of plugins to be enabled on startup if loadAllPlugins is false,
		//! or plugins to be excluded from being loaded on startup if loadAllPlugins is true.
		std::vector<std::string> plugins = {};
	};

	struct MsgStyle final : Reflectable
	{
		std::wstring msgEchoStyle = L"0x00AA0090";
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

	struct Message final : Reflectable
	{
		MsgStyle msgStyle;

		//! If true, messages will sent only to local ships
		bool defaultLocalChat = false;

		//! If true, sends a copy of submitted commands to the player's chatlog.
		bool echoCommands = true;

		//! If true, invalid commands are not echo'ed
		bool suppressInvalidCommands = true;

		//! If true, player's death renders the default message.
		bool dieMsg = true;

		//! Broadcasts a message that the player is attempting docking to all players in range
		//! currently hardcoded to 15K
		bool dockingMessages = true;
	};

	struct Socket final : Reflectable
	{
		bool activated = false;
		int port = 1919;
		int wPort = 1920;
		int ePort = 1921;
		int eWPort = 1922;
		std::string encryptionKey = "SomeRandomKey000";

		void* bfCTX = nullptr;
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
	};

	struct MultiKillMessages final : Reflectable
	{
		bool active = false;
		std::wstring multiKillMessageStyle = L"0x1919BD01";
		std::map<std::wstring, int> multiKillMessages = {{L"%player is on a rampage", 5}, {L"%player runs amok", 10}, {L"%player is godlike", 15}};
	};

	struct Bans final : Reflectable
	{
		bool banAccountOnMatch = false;
		std::vector<std::wstring> banWildcardsAndIPs;
	};

	General general;
	Plugins plugins;
	Socket socket;
	UserCommands userCommands;
	MultiKillMessages multiKillMessages;
	Bans bans;
	Message messages;
};

REFL_AUTO(type(FLHookConfig::General), field(antiDockKill), field(antiF1), field(changeCruiseDisruptorBehaviour), field(debugMode),
    field(disableCharfileEncryption), field(disconnectDelay), field(disableNPCSpawns), field(localTime), field(maxGroupSize), field(persistGroup),
    field(reservedSlots), field(torpMissileBaseDamageMultiplier), field(logPerformanceTimers), field(chatSuppressList), field(noPVPSystems), field(antiBaseIdle), field(antiCharMenuIdle))
REFL_AUTO(type(FLHookConfig::Plugins), field(loadAllPlugins), field(plugins))
REFL_AUTO(type(FLHookConfig::Socket), field(activated), field(port), field(wPort), field(ePort), field(eWPort), field(encryptionKey), field(passRightsMap))
REFL_AUTO(type(FLHookConfig::MsgStyle), field(msgEchoStyle), field(deathMsgStyle), field(deathMsgStyleSys), field(kickMsgPeriod), field(kickMsg),
    field(userCmdStyle), field(adminCmdStyle), field(deathMsgTextAdminKill), field(deathMsgTextPlayerKill), field(deathMsgTextSelfKill), field(deathMsgTextNPC),
    field(deathMsgTextSuicide))
REFL_AUTO(type(FLHookConfig::UserCommands), field(userCmdSetDieMsg), field(userCmdSetDieMsgSize), field(userCmdSetChatFont), field(userCmdIgnore),
    field(userCmdHelp), field(userCmdMaxIgnoreList))
REFL_AUTO(type(FLHookConfig::MultiKillMessages), field(active), field(multiKillMessageStyle), field(multiKillMessages))
REFL_AUTO(type(FLHookConfig::Bans), field(banAccountOnMatch), field(banWildcardsAndIPs))
REFL_AUTO(type(FLHookConfig::Message), field(msgStyle), field(defaultLocalChat), field(echoCommands), field(suppressInvalidCommands), field(dieMsg),
    field(dockingMessages))
REFL_AUTO(type(FLHookConfig), field(general), field(plugins), field(socket), field(multiKillMessages), field(userCommands), field(messages), field(bans))

// Use the class to create and send packets of inconstant size.
#pragma pack(push, 1)
class FLPACKET
{
  private:
	uint Size;

	byte kind;
	byte type;

  public:
	// This is content of your packet. You may want to reinterpretate it as pointer to packet data struct for convenience.
	byte content[1];

	enum CommonPacket
	{
		FLPACKET_COMMON_00,
		FLPACKET_COMMON_UPDATEOBJECT,
		FLPACKET_COMMON_FIREWEAPON,
		FLPACKET_COMMON_03,
		FLPACKET_COMMON_SETTARGET,
		FLPACKET_COMMON_CHATMSG,
		FLPACKET_COMMON_06,
		FLPACKET_COMMON_07,
		FLPACKET_COMMON_ACTIVATEEQUIP,
		FLPACKET_COMMON_09,
		FLPACKET_COMMON_0A,
		FLPACKET_COMMON_0B,
		FLPACKET_COMMON_0C,
		FLPACKET_COMMON_0D,
		FLPACKET_COMMON_ACTIVATECRUISE,
		FLPACKET_COMMON_GOTRADELANE,
		FLPACKET_COMMON_STOPTRADELANE,
		FLPACKET_COMMON_SET_WEAPON_GROUP,
		FLPACKET_COMMON_PLAYER_TRADE,
		FLPACKET_COMMON_SET_VISITED_STATE,
		FLPACKET_COMMON_JETTISONCARGO,
		FLPACKET_COMMON_ACTIVATETHRUSTERS,
		FLPACKET_COMMON_REQUEST_BEST_PATH,
		FLPACKET_COMMON_REQUEST_GROUP_POSITIONS,
		FLPACKET_COMMON_REQUEST_PLAYER_STATS,
		FLPACKET_COMMON_SET_MISSION_LOG,
		FLPACKET_COMMON_REQUEST_RANK_LEVEL,
		FLPACKET_COMMON_POP_UP_DIALOG,
		FLPACKET_COMMON_SET_INTERFACE_STATE,
		FLPACKET_COMMON_TRACTOROBJECTS
	};

	enum ServerPacket
	{
		FLPACKET_SERVER_00,
		FLPACKET_SERVER_CONNECTRESPONSE,
		FLPACKET_SERVER_LOGINRESPONSE,
		FLPACKET_SERVER_CHARACTERINFO,
		FLPACKET_SERVER_CREATESHIP,
		FLPACKET_SERVER_DAMAGEOBJECT,
		FLPACKET_SERVER_DESTROYOBJECT,
		FLPACKET_SERVER_LAUNCH,
		FLPACKET_SERVER_CHARSELECTVERIFIED,
		FLPACKET_SERVER_09,
		FLPACKET_SERVER_ACTIVATEOBJECT,
		FLPACKET_SERVER_LAND,
		FLPACKET_SERVER_0C,
		FLPACKET_SERVER_SETSTARTROOM,
		FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST,
		FLPACKET_SERVER_GFCOMPLETECHARLIST,
		FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST,
		FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST,
		FLPACKET_SERVER_12,
		FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST,
		FLPACKET_SERVER_GFDESTROYNEWSBROADCAST,
		FLPACKET_SERVER_GFDESTROYCHARACTER,
		FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER,
		FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR,
		FLPACKET_SERVER_18,
		FLPACKET_SERVER_GFDESTROYAMBIENTSCRIPT,
		FLPACKET_SERVER_GFSCRIPTBEHAVIOR,
		FLPACKET_SERVER_1B,
		FLPACKET_SERVER_1C,
		FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER,
		FLPACKET_SERVER_GFUPDATENEWSBROADCAST,
		FLPACKET_SERVER_GFUPDATEAMBIENTSCRIPT,
		FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE,
		FLPACKET_SERVER_SYSTEM_SWITCH_OUT,
		FLPACKET_SERVER_SYSTEM_SWITCH_IN,
		FLPACKET_SERVER_SETSHIPARCH,
		FLPACKET_SERVER_SETEQUIPMENT,
		FLPACKET_SERVER_SETCARGO,
		FLPACKET_SERVER_GFUPDATECHAR,
		FLPACKET_SERVER_REQUESTCREATESHIPRESP,
		FLPACKET_SERVER_CREATELOOT,
		FLPACKET_SERVER_SETREPUTATION,
		FLPACKET_SERVER_ADJUSTATTITUDE,
		FLPACKET_SERVER_SETGROUPFEELINGS,
		FLPACKET_SERVER_CREATEMINE,
		FLPACKET_SERVER_CREATECOUNTER,
		FLPACKET_SERVER_SETADDITEM,
		FLPACKET_SERVER_SETREMOVEITEM,
		FLPACKET_SERVER_SETCASH,
		FLPACKET_SERVER_EXPLODEASTEROIdMINE,
		FLPACKET_SERVER_REQUESTSPACESCRIPT,
		FLPACKET_SERVER_SETMISSIONOBJECTIVESTATE,
		FLPACKET_SERVER_REPLACEMISSIONOBJECTIVE,
		FLPACKET_SERVER_SETMISSIONOBJECTIVES,
		FLPACKET_SERVER_36,
		FLPACKET_SERVER_CREATEGUIDED,
		FLPACKET_SERVER_ITEMTRACTORED,
		FLPACKET_SERVER_SCANNOTIFY,
		FLPACKET_SERVER_3A,
		FLPACKET_SERVER_3B,
		FLPACKET_SERVER_REPAIROBJECT,
		FLPACKET_SERVER_REMOTEOBJECTCARGOUPDATE,
		FLPACKET_SERVER_SETNUMKILLS,
		FLPACKET_SERVER_SETMISSIONSUCCESSES,
		FLPACKET_SERVER_SETMISSIONFAILURES,
		FLPACKET_SERVER_BURNFUSE,
		FLPACKET_SERVER_CREATESOLAR,
		FLPACKET_SERVER_SET_STORY_CUE,
		FLPACKET_SERVER_REQUEST_RETURNED,
		FLPACKET_SERVER_SET_MISSION_MESSAGE,
		FLPACKET_SERVER_MARKOBJ,
		FLPACKET_SERVER_CFGINTERFACENOTIFICATION,
		FLPACKET_SERVER_SETCOLLISIONGROUPS,
		FLPACKET_SERVER_SETHULLSTATUS,
		FLPACKET_SERVER_SETGUIDEDTARGET,
		FLPACKET_SERVER_SET_CAMERA,
		FLPACKET_SERVER_REVERT_CAMERA,
		FLPACKET_SERVER_LOADHINT,
		FLPACKET_SERVER_SETDIRECTIVE,
		FLPACKET_SERVER_SENDCOMM,
		FLPACKET_SERVER_50,
		FLPACKET_SERVER_USE_ITEM,
		FLPACKET_SERVER_PLAYERLIST,
		FLPACKET_SERVER_FORMATION_UPDATE,
		FLPACKET_SERVER_MISCOBJUPDATE,
		FLPACKET_SERVER_OBJECTCARGOUPDATE,
		FLPACKET_SERVER_SENDNNMESSAGE,
		FLPACKET_SERVER_SET_MUSIC,
		FLPACKET_SERVER_CANCEL_MUSIC,
		FLPACKET_SERVER_PLAY_SOUND_EFFECT,
		FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY,
		FLPACKET_SERVER_MISSIONSAVEA
	};

	enum ClientPacket
	{
		FLPACKET_CLIENT_00,
		FLPACKET_CLIENT_LOGIN,
		FLPACKET_CLIENT_02,
		FLPACKET_CLIENT_MUNCOLLISION,
		FLPACKET_CLIENT_REQUESTLAUNCH,
		FLPACKET_CLIENT_REQUESTCHARINFO,
		FLPACKET_CLIENT_SELECTCHARACTER,
		FLPACKET_CLIENT_ENTERBASE,
		FLPACKET_CLIENT_REQUESTBASEINFO,
		FLPACKET_CLIENT_REQUESTLOCATIONINFO,
		FLPACKET_CLIENT_GFREQUESTSHIPINFO,
		FLPACKET_CLIENT_SYSTEM_SWITCH_OUT_COMPLETE,
		FLPACKET_CLIENT_OBJCOLLISION,
		FLPACKET_CLIENT_EXITBASE,
		FLPACKET_CLIENT_ENTERLOCATION,
		FLPACKET_CLIENT_EXITLOCATION,
		FLPACKET_CLIENT_REQUESTCREATESHIP,
		FLPACKET_CLIENT_GFGOODSELL,
		FLPACKET_CLIENT_GFGOODBUY,
		FLPACKET_CLIENT_GFSELECTOBJECT,
		FLPACKET_CLIENT_MISSIONRESPONSE,
		FLPACKET_CLIENT_REQSHIPARCH,
		FLPACKET_CLIENT_REQEQUIPMENT,
		FLPACKET_CLIENT_REQCARGO,
		FLPACKET_CLIENT_REQADDITEM,
		FLPACKET_CLIENT_REQREMOVEITEM,
		FLPACKET_CLIENT_REQMODIFYITEM,
		FLPACKET_CLIENT_REQSETCASH,
		FLPACKET_CLIENT_REQCHANGECASH,
		FLPACKET_CLIENT_1D,
		FLPACKET_CLIENT_SAVEGAME,
		FLPACKET_CLIENT_1F,
		FLPACKET_CLIENT_MINEASTEROId,
		FLPACKET_CLIENT_21,
		FLPACKET_CLIENT_DBGCREATESHIP,
		FLPACKET_CLIENT_DBGLOADSYSTEM,
		FLPACKET_CLIENT_DOCK,
		FLPACKET_CLIENT_DBGDESTROYOBJECT,
		FLPACKET_CLIENT_26,
		FLPACKET_CLIENT_TRADERESPONSE,
		FLPACKET_CLIENT_28,
		FLPACKET_CLIENT_29,
		FLPACKET_CLIENT_2A,
		FLPACKET_CLIENT_CARGOSCAN,
		FLPACKET_CLIENT_2C,
		FLPACKET_CLIENT_DBGCONSOLE,
		FLPACKET_CLIENT_DBGFREESYSTEM,
		FLPACKET_CLIENT_SETMANEUVER,
		FLPACKET_CLIENT_DBGRELOCATE_SHIP,
		FLPACKET_CLIENT_REQUEST_EVENT,
		FLPACKET_CLIENT_REQUEST_CANCEL,
		FLPACKET_CLIENT_33,
		FLPACKET_CLIENT_34,
		FLPACKET_CLIENT_INTERFACEITEMUSED,
		FLPACKET_CLIENT_REQCOLLISIONGROUPS,
		FLPACKET_CLIENT_COMMCOMPLETE,
		FLPACKET_CLIENT_REQUESTNEWCHARINFO,
		FLPACKET_CLIENT_CREATENEWCHAR,
		FLPACKET_CLIENT_DESTROYCHAR,
		FLPACKET_CLIENT_REQHULLSTATUS,
		FLPACKET_CLIENT_GFGOODVAPORIZED,
		FLPACKET_CLIENT_BADLANDSOBJCOLLISION,
		FLPACKET_CLIENT_LAUNCHCOMPLETE,
		FLPACKET_CLIENT_HAIL,
		FLPACKET_CLIENT_REQUEST_USE_ITEM,
		FLPACKET_CLIENT_ABORT_MISSION,
		FLPACKET_CLIENT_SKIP_AUTOSAVE,
		FLPACKET_CLIENT_JUMPINCOMPLETE,
		FLPACKET_CLIENT_REQINVINCIBILITY,
		FLPACKET_CLIENT_MISSIONSAVEB,
		FLPACKET_CLIENT_REQDIFFICULTYSCALE,
		FLPACKET_CLIENT_RTCDONE
	};

	// Common packets are being sent from server to client and from client to server.
	DLL static FLPACKET* Create(uint size, CommonPacket kind);

	// Server packets are being sent only from server to client.
	DLL static FLPACKET* Create(uint size, ServerPacket kind);

	// Client packets are being sent only from client to server. Can't imagine why you ever need to create such a packet at side of server.
	DLL static FLPACKET* Create(uint size, ClientPacket kind);

	// Returns true if sent succesfully, false if not. Frees memory allocated for packet.
	DLL bool SendTo(ClientId client);
};
#pragma pack(pop)

class CCmds
{
  protected:
	~CCmds() = default;

  private:
	bool bId;
	bool bShortCut;
	bool bSelf;
	bool bTarget;

  public:
	DWORD rights;
#ifdef FLHOOK
	// commands
	void CmdGetCash(const std::variant<uint, std::wstring>& player);
	void CmdSetCash(const std::variant<uint, std::wstring>& player, uint iAmount);
	void CmdAddCash(const std::variant<uint, std::wstring>& player, uint iAmount);

	void CmdKick(const std::variant<uint, std::wstring>& player, const std::wstring& wscReason);
	void CmdBan(const std::variant<uint, std::wstring>& player);
	void CmdTempBan(const std::variant<uint, std::wstring>& player, uint duration);
	void CmdUnban(const std::variant<uint, std::wstring>& player);

	void CmdBeam(const std::variant<uint, std::wstring>& player, const std::wstring& wscBasename);
	void CmdChase(std::wstring wscAdminName, const std::variant<uint, std::wstring>& player);
	void CmdPull(std::wstring wscAdminName, const std::variant<uint, std::wstring>& player);
	void CmdMove(std::wstring wscAdminName, float x, float y, float z);
	void CmdKill(const std::variant<uint, std::wstring>& player);
	void CmdResetRep(const std::variant<uint, std::wstring>& player);
	void CmdSetRep(const std::variant<uint, std::wstring>& player, const std::wstring& wscRepGroup, float fValue);
	void CmdGetRep(const std::variant<uint, std::wstring>& player, const std::wstring& wscRepGroup);

	void CmdMsg(const std::variant<uint, std::wstring>& player, const std::wstring& text);
	void CmdMsgS(const std::wstring& wscSystemname, const std::wstring& text);
	void CmdMsgU(const std::wstring& text);
	void CmdFMsg(const std::variant<uint, std::wstring>& player, const std::wstring& wscXML);
	void CmdFMsgS(const std::wstring& wscSystemname, const std::wstring& wscXML);
	void CmdFMsgU(const std::wstring& wscXML);

	void CmdEnumCargo(const std::variant<uint, std::wstring>& player);
	void CmdRemoveCargo(const std::variant<uint, std::wstring>& player, ushort cargoId, uint count);
	void CmdAddCargo(const std::variant<uint, std::wstring>& player, const std::wstring& wscGood, uint iCount, bool iMission);

	void CmdRename(const std::variant<uint, std::wstring>& player, const std::wstring& wscNewCharname);
	void CmdDeleteChar(const std::variant<uint, std::wstring>& player);

	void CmdReadCharFile(const std::variant<uint, std::wstring>& player);
	void CmdWriteCharFile(const std::variant<uint, std::wstring>& player, const std::wstring& wscData);

	void CmdGetClientID(const std::wstring& player);
	void PrintPlayerInfo(PlayerInfo& pi);
	void CmdGetPlayerInfo(const std::variant<uint, std::wstring>& player);
	void CmdGetPlayers();
	void XPrintPlayerInfo(const PlayerInfo& pi);
	void CmdXGetPlayerInfo(const std::variant<uint, std::wstring>& player);
	void CmdXGetPlayers();
	void CmdGetPlayerIds();
	void CmdHelp();
	void CmdGetAccountDirName(const std::variant<uint, std::wstring>& player);
	void CmdGetCharFileName(const std::variant<uint, std::wstring>& player);
	void CmdIsOnServer(const std::wstring& player);
	void CmdMoneyFixList();
	void CmdServerInfo();
	void CmdGetGroupMembers(const std::variant<uint, std::wstring>& player);

	void CmdSaveChar(const std::variant<uint, std::wstring>& player);

	void CmdGetReservedSlot(const std::variant<uint, std::wstring>& player);
	void CmdSetReservedSlot(const std::variant<uint, std::wstring>& player, int iReservedSlot);
	void CmdSetAdmin(const std::variant<uint, std::wstring>& player, const std::wstring& wscRights);
	void CmdGetAdmin(const std::variant<uint, std::wstring>& player);
	void CmdDelAdmin(const std::variant<uint, std::wstring>& player);

	void CmdLoadPlugins();
	void CmdLoadPlugin(const std::wstring& wscPlugin);
	void CmdListPlugins();
	void CmdUnloadPlugin(const std::wstring& wscPlugin);
	void CmdReloadPlugin(const std::wstring& wscPlugin);
	void CmdShutdown();

	void ExecuteCommandString(const std::wstring& wscCmd);
	void SetRightsByString(const std::string& scRightStr);
	std::wstring wscCurCmdString;
#endif

  public:
	virtual void DoPrint(const std::string& text) = 0;
	DLL void PrintError(Error err);
	DLL std::wstring ArgCharname(uint iArg);
	DLL int ArgInt(uint iArg);
	DLL uint ArgUInt(uint iArg);
	DLL float ArgFloat(uint iArg);
	DLL std::wstring ArgStr(uint iArg);
	DLL std::wstring ArgStrToEnd(uint iArg);
	DLL void Print(const std::string& text);
	DLL virtual std::wstring GetAdminName() { return L""; };
	DLL virtual bool IsPlayer() { return false; }
};

class CInGame final : public CCmds
{
  public:
	uint client;
	std::wstring wscAdminName;
	DLL void DoPrint(const std::string& text) override;
	DLL void ReadRights(const std::string& scIniFile);
	DLL std::wstring GetAdminName() override;
	DLL bool IsPlayer() override { return true; }
};

class CSocket final : public CCmds
{
  public:
	SOCKET s;
	BLOWFISH_CTX* bfc;
	bool bAuthed;
	bool bEventMode;
	bool bUnicode;
	bool bEncrypted;
	std::string sIP;
	ushort iPort;

	CSocket()
	{
		bAuthed = false;
		bEventMode = false;
		bUnicode = false;
	}
	DLL void DoPrint(const std::string& text) override;
	DLL std::wstring GetAdminName() override;
};

// FuncLog

DLL bool InitLogs();

DLL void HandleCheater(ClientId client, bool bBan, const std::string& reason);
DLL bool AddCheaterLog(const std::variant<uint, std::wstring>& player, const std::string& reason);
DLL bool AddKickLog(ClientId client, const std::string& reason);
DLL bool AddConnectLog(ClientId client, const std::string& reason);

DLL void UserCmd_SetDieMsg(ClientId& client, const std::wstring& wscParam);
DLL void UserCmd_SetChatFont(ClientId& client, const std::wstring& wscParam);
DLL void PrintUserCmdText(ClientId client, const std::wstring& text);
DLL void PrintLocalUserCmdText(ClientId client, const std::wstring& wscMsg, float fDistance);

DLL extern bool g_NonGunHitsBase;
DLL extern float g_LastHitPts;

// namespaces
namespace IServerImplHook
{
	DLL extern bool g_InSubmitChat;
	DLL extern uint g_TextLength;
} // namespace IServerImplHook

struct DLL CoreGlobals : Singleton<CoreGlobals>
{
	uint damageToClientId;
	uint damageToSpaceId;

	bool messagePrivate;
	bool messageSystem;
	bool messageUniverse;

	std::string accPath;

	uint serverLoadInMs;
	uint playerCount;
	bool disableNpcs;

	std::list<BaseInfo> allBases;

	bool flhookReady;
};

extern DLL CDPServer* cdpSrv;
extern DLL _GetShipInspect GetShipInspect;
extern DLL char* g_FLServerDataPtr;
extern DLL CDPClientProxy** clientProxyArray;
extern DLL void* pClient;
extern DLL _RCSendChatMsg RCSendChatMsg;
extern DLL _CRCAntiCheat CRCAntiCheat;
extern DLL IClientImpl* FakeClient;
extern DLL IClientImpl* HookClient;
extern DLL char* OldClient;

extern DLL std::array<CLIENT_INFO, MaxClientId + 1> ClientInfo;

DLL std::string FlcDecode(std::string& input);
DLL std::string FlcEncode(std::string& input);
DLL bool EncodeDecode(const char* input, const char* output, bool encode);
DLL bool FlcDecodeFile(const char* input, const char* outputFile);
DLL bool FlcEncodeFile(const char* input, const char* outputFile);

DLL void Blowfish_Init(BLOWFISH_CTX* ctx, unsigned char* key, int keyLen);
DLL char Blowfish_Encrypt(BLOWFISH_CTX* ctx, void* ptr, unsigned long dataLen);
DLL char Blowfish_Decrypt(BLOWFISH_CTX* ctx, void* ptr, unsigned long dataLen);