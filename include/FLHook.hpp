#pragma once

// Magic Enum Extensions
using namespace magic_enum::bitwise_operators; // NOLINT
using namespace magic_enum::ostream_operators; // NOLINT

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

DLL std::string IniGetS(const std::string& file, const std::string& app, const std::string& key, const std::string& def);
DLL int IniGetI(const std::string& file, const std::string& app, const std::string& key, int def);
DLL bool IniGetB(const std::string& file, const std::string& app, const std::string& key, bool def);
DLL void IniWrite(const std::string& file, const std::string& app, const std::string& key, const std::string& value);
DLL void IniDelSection(const std::string& file, const std::string& app);
DLL void IniDelete(const std::string& file, const std::string& app, const std::string& key);
DLL void IniWriteW(const std::string& file, const std::string& app, const std::string& key, const std::wstring& value);
DLL std::wstring IniGetWS(const std::string& file, const std::string& app, const std::string& key, const std::wstring& def);
DLL float IniGetF(const std::string& file, const std::string& app, const std::string& key, float def);
DLL std::wstring GetTimeString(bool localTime);
DLL std::string GetUserFilePath(const std::variant<uint, std::wstring>& player, const std::string& extension);

// variables
extern DLL HANDLE hProcFL;
extern DLL HMODULE server;
extern DLL HMODULE common;
extern DLL HMODULE remoteClient;
extern DLL HMODULE hModDPNet;
extern DLL HMODULE hModDaLib;
extern DLL HMODULE content;
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
		//! Number of miliseconds the player character remains in space after disconnecting.
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
		//! NOT IMPLEMENTED YET: if true, keeps the player in the group if they switch characters within one account.
		bool persistGroup = false;
		//! Number of slots reserved to specified accounts.
		uint reservedSlots = 0;
		//! Global damage multiplier to missile and torpedo type weapons.
		float torpMissileBaseDamageMultiplier = 1.0f;
		//! If true, it logs performance of functions if they take too long to execute.
		bool logPerformanceTimers = false;

		bool tempBansEnabled = true;

		//! A vector of forbidden words/phrases, which will not be processed and sent to other players
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
		//! Time in ms between kick message rendering and actual server kick occuring.
		uint kickMsgPeriod = 5000;
		//! Kick message content.
		std::wstring kickMsg = LR"(<TRA data=" 0x0000FF10 " mask=" - 1 "/><TEXT>You will be kicked. Reason: %reason</TEXT>)";
		std::wstring userCmdStyle = L"0x00FF0090";
		std::wstring adminCmdStyle = L"0x00FF0090";
		//! Death message for admin kills.
		std::wstring deathMsgTextAdminKill = L"Death: %victim was killed by an admin";
		//! Default player to player kill message.
		std::wstring deathMsgTextPlayerKill = L"Death: %victim was killed by %killer (%type)";
		//! Death message for weapon self-kills.
		std::wstring deathMsgTextSelfKill = L"Death: %victim killed himself (%type)";
		//! Death message for player deaths to NPCs.
		std::wstring deathMsgTextNPC = L"Death: %victim was killed by an NPC";
		//! Death message for environmental deaths.
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

	struct UserCommands final : Reflectable
	{
		//! Can users use SetDieMsgSize command
		bool userCmdSetDieMsgSize = true;
		//! Can users use SetDieMsg command
		bool userCmdSetDieMsg = true;
		//! Can users use SetChatFont command
		bool userCmdSetChatFont = true;
		//! Can users use Ignore command
		bool userCmdIgnore = true;
		//! Can users use Help command
		bool userCmdHelp = true;
		//! Maximum size of users added via /ignore command 
		uint userCmdMaxIgnoreList = 0;
		//! If true, the default player chat will be local, not system.
		bool defaultLocalChat = false;
	};

	struct Bans final : Reflectable
	{
		//! If true, apply a vanilla FLServer ban in case of a wildcard/IP match on top of kicking them.
		bool banAccountOnMatch = false;
		//! Instantly kicks any player with a matching IP or matching IP range.
		std::vector<std::wstring> banWildcardsAndIPs;
	};

	struct Callsign final : Reflectable
	{
		//! The mapping of numbers to formations. 1, min = Alpha. 29, max = Yanagi
		std::vector<int> allowedFormations = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

		//! If true, formations and numbers will not be assigned to ships. All ships will be alpha 1-1.
		bool disableRandomisedFormations = false;

		//! If true, NPCs will refer to all players as freelancer
		bool disableUsingAffiliationForCallsign = false;
	};

	General general;
	Plugins plugins;
	UserCommands userCommands;
	Bans bans;
	Message messages;
	Callsign callsign;
};

// Use the class to create and send packets of inconstant size.
#pragma pack(push, 1)
class FlPacket
{
private:
	uint size = 0;

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
		FLPACKET_SERVER_SETHULATUS,
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
		FLPACKET_CLIENT_REQHULATUS,
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
	DLL static FlPacket* Create(uint size, CommonPacket kind);

	// Server packets are being sent only from server to client.
	DLL static FlPacket* Create(uint size, ServerPacket kind);

	// Client packets are being sent only from client to server. Can't imagine why you ever need to create such a packet at side of server.
	DLL static FlPacket* Create(uint size, ClientPacket kind);

	// Returns true if sent succesfully, false if not. Frees memory allocated for packet.
	DLL bool SendTo(ClientId client);
};
#pragma pack(pop)

class CCmds
{
protected:
	~CCmds() = default;

private:
	bool id;
	bool shortCut;
	bool self;
	bool target;

public:
	ulong rights;
	#ifdef FLHOOK
	// commands
	void CmdGetCash(const std::variant<uint, std::wstring>& player);
	void CmdSetCash(const std::variant<uint, std::wstring>& player, uint amount);
	void CmdAddCash(const std::variant<uint, std::wstring>& player, uint amount);

	void CmdKick(const std::variant<uint, std::wstring>& player, const std::wstring& reason);
	void CmdBan(const std::variant<uint, std::wstring>& player);
	void CmdTempBan(const std::variant<uint, std::wstring>& player, uint duration);
	void CmdUnban(const std::variant<uint, std::wstring>& player);

	void CmdBeam(const std::variant<uint, std::wstring>& player, const std::wstring& basename);
	void CmdChase(std::wstring adminName, const std::variant<uint, std::wstring>& player);
	void CmdPull(std::wstring adminName, const std::variant<uint, std::wstring>& player);
	void CmdMove(std::wstring adminName, float x, float y, float z);
	void CmdKill(const std::variant<uint, std::wstring>& player);
	void CmdResetRep(const std::variant<uint, std::wstring>& player);
	void CmdSetRep(const std::variant<uint, std::wstring>& player, const std::wstring& repGroup, float value);
	void CmdGetRep(const std::variant<uint, std::wstring>& player, const std::wstring& repGroup);

	void CmdMsg(const std::variant<uint, std::wstring>& player, const std::wstring& text);
	void CmdMsgS(const std::wstring& systemName, const std::wstring& text);
	void CmdMsgU(const std::wstring& text);
	void CmdFMsg(const std::variant<uint, std::wstring>& player, const std::wstring& XML);
	void CmdFMsgS(const std::wstring& systemName, const std::wstring& XML);
	void CmdFMsgU(const std::wstring& XML);

	void CmdEnumCargo(const std::variant<uint, std::wstring>& player);
	void CmdRemoveCargo(const std::variant<uint, std::wstring>& player, ushort cargoId, uint count);
	void CmdAddCargo(const std::variant<uint, std::wstring>& player, const std::wstring& good, uint count, bool mission);

	void CmdRename(const std::variant<uint, std::wstring>& player, const std::wstring& newCharname);
	void CmdDeleteChar(const std::variant<uint, std::wstring>& player);

	void CmdReadCharFile(const std::variant<uint, std::wstring>& player);
	void CmdWriteCharFile(const std::variant<uint, std::wstring>& player, const std::wstring& data);

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
	void CmdSetReservedSlot(const std::variant<uint, std::wstring>& player, int reservedSlot);
	void CmdSetAdmin(const std::variant<uint, std::wstring>& player, const std::wstring& rights);
	void CmdGetAdmin(const std::variant<uint, std::wstring>& player);
	void CmdDelAdmin(const std::variant<uint, std::wstring>& player);

	void CmdLoadPlugins();
	void CmdLoadPlugin(const std::wstring& plugin);
	void CmdListPlugins();
	void CmdUnloadPlugin(const std::wstring& plugin);
	void CmdReloadPlugin(const std::wstring& plugin);
	void CmdShutdown();

	void ExecuteCommandString(const std::wstring& Cmd);
	void SetRightsByString(const std::string& rightStr);
	std::wstring currCmdString;
	#endif

public:
	virtual void DoPrint(const std::string& text) = 0;
	DLL void PrintError(Error err);
	DLL std::wstring ArgCharname(uint arg);
	DLL int ArgInt(uint arg);
	DLL uint ArgUInt(uint arg);
	DLL float ArgFloat(uint arg);
	DLL std::wstring ArgStr(uint arg);
	DLL std::wstring ArgStrToEnd(uint arg);
	DLL void Print(const std::string& text);
	DLL virtual std::wstring GetAdminName() { return L""; };
	DLL virtual bool IsPlayer() { return false; }
};

class CInGame final : public CCmds
{
public:
	uint client;
	std::wstring adminName;
	DLL void DoPrint(const std::string& text) override;
	DLL void ReadRights(const std::string& iniFile);
	DLL std::wstring GetAdminName() override;
	DLL bool IsPlayer() override { return true; }
};

// FuncLog

DLL void HandleCheater(ClientId client, bool ban, const std::string& reason);
DLL bool AddCheaterLog(const std::variant<uint, std::wstring>& player, const std::string& reason);
DLL bool AddKickLog(ClientId client, const std::string& reason);
DLL bool AddConnectLog(ClientId client, const std::string& reason);

DLL void UserCmd_SetDieMsg(ClientId& client, const std::wstring& param);
DLL void UserCmd_SetChatFont(ClientId& client, const std::wstring& param);
DLL void PrintUserCmdText(ClientId client, const std::wstring& text);
DLL void PrintLocalUserCmdText(ClientId client, const std::wstring& msg, float distance);

DLL extern bool g_NonGunHitsBase;
DLL extern float g_LastHitPts;

// namespaces
namespace IServerImplHook
{
	struct SubmitData
	{
		bool inSubmitChat;
		std::wstring characterName;
	};

	const DLL extern std::unique_ptr<SubmitData> chatData;
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
extern DLL void* client;
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
