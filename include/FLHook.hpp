#pragma once

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

class CCmds
{
protected:
	~CCmds() = default;

private:
	bool id{};
	bool shortCut{};
	bool self{};
	bool target{};

public:
	ulong rights{};
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

	void CmdRemoveCargo(const std::variant<uint, std::wstring>& player, ushort cargoId, uint count);
	void CmdAddCargo(const std::variant<uint, std::wstring>& player, const std::wstring& good, uint count, bool mission);

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
	std::wstring currCmdString;
	#endif

public:
	DLL void PrintError(Error err);
	DLL std::wstring ArgCharname(uint arg);
	DLL int ArgInt(uint arg);
	DLL uint ArgUInt(uint arg);
	DLL float ArgFloat(uint arg);
	DLL std::wstring ArgStr(uint arg);
	DLL std::wstring ArgStrToEnd(uint arg);
	DLL void Print(const std::string& text);
	DLL virtual bool IsPlayer() { return false; }
};

class CInGame final : public CCmds
{
public:
	uint client;
	std::wstring adminName;
	DLL bool IsPlayer() override { return true; }
};

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
extern DLL void* client;
extern DLL _RCSendChatMsg RCSendChatMsg;
extern DLL _CRCAntiCheat CRCAntiCheat;
extern DLL IClientImpl* FakeClient;
extern DLL IClientImpl* HookClient;
extern DLL char* OldClient;

extern DLL std::array<CLIENT_INFO, MaxClientId + 1> ClientInfo;