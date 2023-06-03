#include "PCH.hpp"

#include "Global.hpp"
#include "Defs/CoreGlobals.hpp"
#include "Defs/FLHookConfig.hpp"
#include "Helpers/Admin.hpp"
#include "Helpers/Chat.hpp"
#include "Helpers/Client.hpp"

namespace IEngineHook
{
	void __cdecl UpdateTime(double interval);
	void __stdcall ElapseTime(float interval);
	int __cdecl DockCall(const uint& shipId, const uint& spaceId, int flags, DOCK_HOST_RESPONSE response);
	int __cdecl FreeReputationVibe(const int& p1);

	void Naked__CShip__Init();
	void Naked__CShip__Destroy();
	void Naked__LaunchPosition();
	void Naked__LoadReputationFromCharacterFile();

	extern FARPROC g_OldInitCShip;
	extern FARPROC g_OldDestroyCShip;
	extern FARPROC g_OldLaunchPosition;
	extern FARPROC g_OldLoadReputationFromCharacterFile;
} // namespace IEngineHook

void __stdcall ShipDestroyed(DamageList* dmgList, DWORD* ecx, uint kill);
void __stdcall NonGunWeaponHitsBaseBefore(char* ECX, char* p1, DamageList* dmg);
void __stdcall SendChat(ClientId client, ClientId clientTo, uint size, void* rdl);

extern FARPROC g_OldShipDestroyed;
extern FARPROC g_OldNonGunWeaponHitsBase;
extern FARPROC g_OldDamageHit, g_OldDamageHit2;
extern FARPROC g_OldDisconnectPacketSent;
extern FARPROC g_OldGuidedHit;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

PatchInfo piFLServerEXE = {"flserver.exe",
    0x0400000,
    {
        {0x041B094, &IEngineHook::UpdateTime, 4, nullptr, false},
        {0x041BAB0, &IEngineHook::ElapseTime, 4, nullptr, false},

        {0, nullptr, 0, nullptr} // terminate
    }};

PatchInfo piContentDLL = {"content.dll",
    0x6EA0000,
    {
        {0x6FB358C, &IEngineHook::DockCall, 4, nullptr, false},

        {0, nullptr, 0, nullptr} // terminate
    }};

PatchInfo piCommonDLL = {"common.dll",
    0x6260000,
    {

        {0x0639C138, &IEngineHook::Naked__CShip__Init, 4, &IEngineHook::g_OldInitCShip, false},
        {0x0639C064, &IEngineHook::Naked__CShip__Destroy, 4, &IEngineHook::g_OldDestroyCShip, false},

        {0, nullptr, 0, nullptr} // terminate
    }};

PatchInfo piServerDLL = {"server.dll",
    0x6CE0000,
    {
        {0x6D67274, &Naked__ShipDestroyed, 4, &g_OldShipDestroyed, false},
        {0x6D641EC, &Naked__AddDamageEntry, 4, nullptr, false},
        {0x6D67320, &Naked__GuidedHit, 4, &g_OldGuidedHit, false},
        {0x6D65448, &Naked__GuidedHit, 4, nullptr, false},
        {0x6D67670, &Naked__GuidedHit, 4, nullptr, false},
        {0x6D653F4, &Naked__DamageHit, 4, &g_OldDamageHit, false},
        {0x6D672CC, &Naked__DamageHit, 4, nullptr, false},
        {0x6D6761C, &Naked__DamageHit, 4, nullptr, false},
        {0x6D65458, &Naked__DamageHit2, 4, &g_OldDamageHit2, false},
        {0x6D67330, &Naked__DamageHit2, 4, nullptr, false},
        {0x6D67680, &Naked__DamageHit2, 4, nullptr, false},
        {0x6D67668, &Naked__NonGunWeaponHitsBase, 4, &g_OldNonGunWeaponHitsBase, false},
        {0x6D6420C, &IEngineHook::Naked__LaunchPosition, 4, &IEngineHook::g_OldLaunchPosition, false},
        {0x6D648E0, &IEngineHook::FreeReputationVibe, 4, nullptr, false},

        {0, nullptr, 0, nullptr} // terminate
    }};

PatchInfo piRemoteClientDLL = {"remoteclient.dll",
    0x6B30000,
    {
        {0x6B6BB80, &SendChat, 4, &RCSendChatMsg, false},

        {0, nullptr, 0, nullptr} // terminate
    }};

PatchInfo piDaLibDLL = {"dalib.dll",
    0x65C0000,
    {
        {0x65C4BEC, &Naked__DisconnectPacketSent, 4, &g_OldDisconnectPacketSent, false},

        {0, nullptr, 0, nullptr} // terminate
    }};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Patch(PatchInfo& pi)
{
	const HMODULE hMod = GetModuleHandle(pi.BinName);
	if (!hMod)
		return false;

	for (uint i = 0; (i < sizeof(pi.piEntries) / sizeof(PatchInfoEntry)); i++)
	{
		if (!pi.piEntries[i].address)
			break;

		char* address = (char*)hMod + (pi.piEntries[i].address - pi.baseAddress);
		if (!pi.piEntries[i].oldValue)
		{
			pi.piEntries[i].oldValue = new char[pi.piEntries[i].size];
			pi.piEntries[i].allocated = true;
		}
		else
			pi.piEntries[i].allocated = false;

		MemUtils::ReadProcMem(address, pi.piEntries[i].oldValue, pi.piEntries[i].size);
		MemUtils::WriteProcMem(address, &pi.piEntries[i].newValue, pi.piEntries[i].size);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RestorePatch(PatchInfo& pi)
{
	const HMODULE hMod = GetModuleHandle(pi.BinName);
	if (!hMod)
		return false;

	for (uint i = 0; (i < sizeof(pi.piEntries) / sizeof(PatchInfoEntry)); i++)
	{
		if (!pi.piEntries[i].address)
			break;

		char* address = (char*)hMod + (pi.piEntries[i].address - pi.baseAddress);
		MemUtils::WriteProcMem(address, pi.piEntries[i].oldValue, pi.piEntries[i].size);
		if (pi.piEntries[i].allocated)
			delete[] pi.piEntries[i].oldValue;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDPClientProxy** clientProxyArray;
CDPServer* cdpSrv;
IClientImpl* FakeClient;
IClientImpl* HookClient;
char* OldClient;
_CRCAntiCheat CRCAntiCheat;
char* g_FLServerDataPtr;
_GetShipInspect GetShipInspect;
std::array<CLIENT_INFO, MaxClientId + 1> ClientInfo;
char RepFreeFixOld[5];

/**************************************************************************************************************
clear the clientinfo
**************************************************************************************************************/

void ClearClientInfo(ClientId client)
{
	auto* info = &ClientInfo[client];

	info->characterName = L"";

	info->dieMsg = DiemsgAll;
	info->ship = 0;
	info->shipOld = 0;
	info->tmSpawnTime = 0;
	info->moneyFix.clear();
	info->tradePartner = 0;
	info->baseEnterTime = 0;
	info->charMenuEnterTime = 0;
	info->cruiseActivated = false;
	info->tmKickTime = 0;
	info->lastExitedBaseId = 0;
	info->disconnected = false;
	info->characterName = L"";
	info->tmF1Time = 0;
	info->tmF1TimeDisconnect = 0;

	const DamageList dmg;
	info->dmgLast = dmg;
	info->dieMsgSize = CS_DEFAULT;
	info->chatSize = CS_DEFAULT;
	info->chatStyle = CST_DEFAULT;

	info->ignoreInfoList.clear();
	info->killsInARow = 0;
	info->hostname = L"";
	info->engineKilled = false;
	info->thrusterActivated = false;
	info->tradelane = false;
	info->groupId = 0;

	info->spawnProtected = false;

	// Reset the dmg list if this client was the inflictor
	for (auto& i : ClientInfo)
	{
		if (i.dmgLast.inflictorPlayerId == client)
			i.dmgLast = dmg;
	}

	Hk::Ini::CharacterClearClientInfo(client);

	CallPluginsAfter(HookedCall::FLHook__ClearClientInfo, client);
}

/**************************************************************************************************************
load settings from accData.json
**************************************************************************************************************/

void LoadUserSettings(ClientId client)
{
	auto* info = &ClientInfo[client];

	const CAccount* acc = Players.FindAccountFromClientID(client);
	const std::wstring dir = Hk::Client::GetAccountDirName(acc);
	const std::wstring userFile = std::format(L"{}{}\\accData.json", CoreGlobals::c()->accPath, dir);

	try
	{
		std::wifstream ifs(userFile);
		std::wstring fileData((std::istreambuf_iterator(ifs)), (std::istreambuf_iterator<wchar_t>()));
		info->accountData = nlohmann::json::parse(fileData);
	}
	catch (nlohmann::json::exception& ex)
	{
		// TODO: Log to a special error file
		Logger::i()->Log(
		    LogLevel::Err, std::format(L"Error while loading account data from account file ({}): {}", userFile, StringUtils::stows(std::string(ex.what()))));
	}

	auto settings = info->accountData.value("settings", nlohmann::json::object());

	info->dieMsg = settings.value("dieMsg", DiemsgAll);
	info->dieMsgSize = settings.value("dieMsgSize", CS_DEFAULT);
	info->chatSize = settings.value("chatSize", CS_DEFAULT);
	info->chatStyle = settings.value("chatStyle", CST_DEFAULT);

	// read ignorelist
	info->ignoreInfoList.clear();

	for (const auto ignoreList = settings.value("ignoreList", nlohmann::json::object()); const auto& [key, value] : ignoreList.items())
	{
		try
		{
			IgnoreInfo ii;
			ii.character = StringUtils::stows(key);
			ii.flags = value;
			info->ignoreInfoList.emplace_back(ii);
		}
		catch (...)
		{
			Logger::i()->Log(LogLevel::Err, std::format(L"Error while loading ignore list from account file ({}): {}", userFile));
		}
	}

	// Don't know if this is a reference or copy - write again to be safe
	info->accountData["settings"] = settings;
}

/**************************************************************************************************************
install the callback hooks
**************************************************************************************************************/

bool InitHookExports()
{
	// init critial sections
	InitializeCriticalSection(&csIPResolve);
	DWORD id;
	DWORD param[34]; // else release version crashes, dont ask me why...
	hThreadResolver = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ThreadResolver, &param, 0, &id);

	GetShipInspect = (_GetShipInspect)SRV_ADDR(ADDR_SRV_GETINSPECT);

	// install IServerImpl callbacks in remoteclient.dll
	auto* serverPointer = reinterpret_cast<char*>(&Server);
	memcpy(&serverPointer, serverPointer, 4);
	for (uint i = 0; i < std::size(IServerImplEntries); i++)
	{
		char* address = serverPointer + IServerImplEntries[i].remoteAddress;
		MemUtils::ReadProcMem(address, &IServerImplEntries[i].fpOldProc, 4);
		MemUtils::WriteProcMem(address, &IServerImplEntries[i].fpProc, 4);
	}

	// patch it
	Patch(piFLServerEXE);
	Patch(piContentDLL);
	Patch(piCommonDLL);
	Patch(piServerDLL);
	Patch(piRemoteClientDLL);
	Patch(piDaLibDLL);

	DetourSendComm();

	// patch rep array free
	const char NOPs[] = {'\x90', '\x90', '\x90', '\x90', '\x90'};
	char* address = ((char*)server + ADDR_SRV_REPARRAYFREE);
	MemUtils::ReadProcMem(address, RepFreeFixOld, 5);
	MemUtils::WriteProcMem(address, NOPs, 5);

	// patch flserver so it can better handle faulty house entries in char files

	// divert call to house load/save func
	address = SRV_ADDR(0x679C6);
	const char DivertJump[] = {'\x6F'};

	MemUtils::WriteProcMem(address, DivertJump, 1);

	// install hook at new address
	address = SRV_ADDR(0x78B39);

	const char MovEAX[] = {'\xB8'};
	const char JMPEAX[] = {'\xFF', '\xE0'};

	const auto fpLoadRepFromCharFile = (FARPROC)IEngineHook::Naked__LoadReputationFromCharacterFile;

	MemUtils::WriteProcMem(address, MovEAX, 1);
	MemUtils::WriteProcMem(address + 1, &fpLoadRepFromCharFile, 4);
	MemUtils::WriteProcMem(address + 5, JMPEAX, 2);

	IEngineHook::g_OldLoadReputationFromCharacterFile = (FARPROC)SRV_ADDR(0x78B40);

	// crc anti-cheat
	CRCAntiCheat = (_CRCAntiCheat)((char*)server + ADDR_CRCANTICHEAT);

	// get CDPServer
	address = DALIB_ADDR(ADDR_CDPSERVER);
	MemUtils::ReadProcMem(address, &cdpSrv, 4);

	// read g_FLServerDataPtr(used for serverload calc)
	address = FLSERVER_ADDR(ADDR_DATAPTR);
	MemUtils::ReadProcMem(address, &g_FLServerDataPtr, 4);

	// anti-deathmsg
	if (FLHookConfig::i()->chatConfig.dieMsg)
	{
		// disables the "old" "A Player has died: ..." chatConfig
		const char JMP[] = {'\xEB'};
		address = SRV_ADDR(ADDR_ANTIdIEMSG);
		MemUtils::WriteProcMem(address, JMP, 1);
	}

	// charfile encyption(doesn't get disabled when unloading FLHook)
	if (FLHookConfig::i()->general.disableCharfileEncryption)
	{
		const char Buf[] = {'\x14', '\xB3'};
		address = SRV_ADDR(ADDR_DISCFENCR);
		MemUtils::WriteProcMem(address, Buf, 2);
		address = SRV_ADDR(ADDR_DISCFENCR2);
		MemUtils::WriteProcMem(address, Buf, 2);
	}

	// maximum group size
	if (FLHookConfig::i()->general.maxGroupSize > 0)
	{
		const char newGroupSize = FLHookConfig::i()->general.maxGroupSize & 0xFF;
		address = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE);
		MemUtils::WriteProcMem(address, &newGroupSize, 1);
		address = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE2);
		MemUtils::WriteProcMem(address, &newGroupSize, 1);
	}

	// get client proxy array, used to retrieve player pings/ips
	address = (char*)remoteClient + ADDR_CPLIST;
	char* Temp;
	MemUtils::ReadProcMem(address, &Temp, 4);
	Temp += 0x10;
	memcpy(&clientProxyArray, &Temp, 4);

	// init variables
	char DataPath[MAX_PATH];
	GetUserDataPath(DataPath);
	CoreGlobals::i()->accPath = std::string(DataPath) + "\\Accts\\MultiPlayer\\";

	// Load DLLs for strings
	Hk::Chat::LoadStringDLLs();

	// clear ClientInfo
	for (uint i = 0; i < ClientInfo.size(); i++)
	{
		ClientInfo[i].client = i;   // Set every client id struct to know of its own id
		ClientInfo[i].connects = 0; // only set to 0 on start
		ClearClientInfo(i);
	}

	// Fix Refire bug
	const std::array<byte, 22> refireBytes = {
	    0x75, 0x0B, 0xC7, 0x84, 0x8C, 0x9C, 00, 00, 00, 00, 00, 00, 00, 0x41, 0x83, 0xC2, 0x04, 0x39, 0xC1, 0x7C, 0xE9, 0xEB};
	MemUtils::WriteProcMem(SRV_ADDR(0x02C057), refireBytes.data(), 22);

	// Enable undocking announcer regardless of distance
	const std::array<byte, 1> undockAnnouncerBytes = {0xEB};
	MemUtils::WriteProcMem(SRV_ADDR(0x173da), undockAnnouncerBytes.data(), 1);

	return true;
}

void PatchClientImpl()
{
	// install IClientImpl callback
	FakeClient = new IClientImpl;
	HookClient = &Client;

	memcpy(&OldClient, &Client, 4);
	MemUtils::WriteProcMem(&Client, FakeClient, 4);
}

/**************************************************************************************************************
uninstall the callback hooks
**************************************************************************************************************/

void UnloadHookExports()
{
	char* address;

	// uninstall IServerImpl callbacks in remoteclient.dll
	if (auto serverAddr = (char*)&Server)
	{
		memcpy(&serverAddr, serverAddr, 4);
		for (uint i = 0; i < std::size(IServerImplEntries); i++)
		{
			const auto address = serverAddr + IServerImplEntries[i].remoteAddress;
			MemUtils::WriteProcMem(address, &IServerImplEntries[i].fpOldProc, 4);
		}
	}

	// reset npc spawn setting
	Hk::Admin::ChangeNPCSpawn(false);

	// restore other hooks
	RestorePatch(piFLServerEXE);
	RestorePatch(piContentDLL);
	RestorePatch(piCommonDLL);
	RestorePatch(piServerDLL);
	RestorePatch(piRemoteClientDLL);
	RestorePatch(piDaLibDLL);

	UnDetourSendComm();

	// unpatch rep array free
	address = ((char*)GetModuleHandle("server.dll") + ADDR_SRV_REPARRAYFREE);
	MemUtils::WriteProcMem(address, RepFreeFixOld, 5);

	// unpatch flserver so it can better handle faulty house entries in char
	// files

	// undivert call to house load/save func
	address = SRV_ADDR(0x679C6);
	char DivertJump[] = {'\x76'};

	// anti-death-msg
	const char Old[] = {'\x74'};
	address = SRV_ADDR(ADDR_ANTIdIEMSG);
	MemUtils::WriteProcMem(address, Old, 1);

	// plugins
	PluginManager::i()->UnloadAll();

	// Undo refire bug
	const std::array<byte, 22> refireBytes = {0x74, 0x0A, 0x41, 0x83, 0xC2, 0x04, 0x3B, 0xC8, 0x7C, 0xF4, 0xEB, 0x0B, 0xC7, 0x84, 0x8C, 0x9C, 0, 0, 0, 0, 0, 0};
	MemUtils::WriteProcMem(SRV_ADDR(0x02C057), refireBytes.data(), 22);

	// undocking announcer regardless of distance
	const std::array<byte, 1> undockAnnouncerBytes = {0x74};
	MemUtils::WriteProcMem(SRV_ADDR(0x173da), undockAnnouncerBytes.data(), 1);
}

void CLIENT_INFO::SaveAccountData()
{
	const CAccount* acc = Players.FindAccountFromClientID(client);
	const std::wstring dir = Hk::Client::GetAccountDirName(acc);
	const std::wstring userFile = std::format(L"{}{}\\accData.json", CoreGlobals::c()->accPath, dir);

	const auto content = accountData.dump(4);
	std::ofstream of(userFile);
	of.write(content.c_str(), content.size());
}
