#pragma once

// TODO: Rename, and find more appropriate file/location
class CLIENT_INFO
{
  public:
	void SaveAccountData();

	// JSON object of various keys and custom data. Some of these will map directly on to members of this class, others are by accessor only.
	nlohmann::json accountData;

	uint client;
	std::wstring characterName;

	// kill msgs
	uint ship;
	uint shipOld;
	mstime tmSpawnTime;

	DamageList dmgLast;

	// money cmd
	std::list<MONEY_FIX> moneyFix;

	// anticheat
	uint tradePartner;

	// change cruise disruptor behaviour
	bool cruiseActivated;
	bool thrusterActivated;
	bool engineKilled;
	bool tradelane;

	// idle kicks
	uint baseEnterTime;
	uint charMenuEnterTime;

	// msg, wait and kick
	mstime tmKickTime;

	// eventmode
	uint lastExitedBaseId;
	bool disconnected;

	// f1 laming
	mstime tmF1Time;
	mstime tmF1TimeDisconnect;

	// ignore usercommand
	std::list<IgnoreInfo> ignoreInfoList;

	// user settings
	DIEMSGTYPE dieMsg;
	CHATSIZE dieMsgSize;
	CHATSTYLE dieMsgStyle;
	CHATSIZE chatSize;
	CHATSTYLE chatStyle;

	// MultiKillMessages
	uint killsInARow;

	// bans
	uint connects; // incremented when player connects

	// Group
	uint groupId;

	// other
	std::wstring hostname;

	bool spawnProtected;

	// Your randomly assigned formation tag, e.g. Navy Lambda 1-6
	uint formationNumber1;
	uint formationNumber2;
	uint formationTag;
};

// Magic Enum Extensions
using namespace magic_enum::bitwise_operators; // NOLINT
using namespace magic_enum::ostream_operators; // NOLINT

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

extern DLL CDPServer* cdpSrv;
extern DLL _GetShipInspect GetShipInspect;
extern DLL char* g_FLServerDataPtr;
extern DLL CDPClientProxy** clientProxyArray;
extern DLL _RCSendChatMsg RCSendChatMsg;
extern DLL _CRCAntiCheat CRCAntiCheat;
extern DLL IClientImpl* FakeClient;
extern DLL IClientImpl* HookClient;
extern DLL char* OldClient;

extern DLL std::array<CLIENT_INFO, MaxClientId + 1> ClientInfo;