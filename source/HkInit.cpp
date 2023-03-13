#include "Global.hpp"

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
} // namespace IEngine

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
	                         {0x0639C064, &IEngineHook::Naked__CShip__Destroy, 4, &IEngineHook::g_OldDestroyCShip,
	                          false},

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
	                         {0x6D6420C, &IEngineHook::Naked__LaunchPosition, 4, &IEngineHook::g_OldLaunchPosition,
	                          false},
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

		char* address = (char*)hMod + (pi.piEntries[i].address - pi.pBaseAddress);
		if (!pi.piEntries[i].pOldValue)
		{
			pi.piEntries[i].pOldValue = new char[pi.piEntries[i].iSize];
			pi.piEntries[i].bAlloced = true;
		}
		else
			pi.piEntries[i].bAlloced = false;

		ReadProcMem(address, pi.piEntries[i].pOldValue, pi.piEntries[i].iSize);
		WriteProcMem(address, &pi.piEntries[i].pNewValue, pi.piEntries[i].iSize);
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

		char* address = (char*)hMod + (pi.piEntries[i].address - pi.pBaseAddress);
		WriteProcMem(address, pi.piEntries[i].pOldValue, pi.piEntries[i].iSize);
		if (pi.piEntries[i].bAlloced)
			delete[] pi.piEntries[i].pOldValue;
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

	info->dieMsg = DiemsgAll;
	info->ship = 0;
	info->shipOld = 0;
	info->tmSpawnTime = 0;
	info->MoneyFix.clear();
	info->iTradePartner = 0;
	info->iBaseEnterTime = 0;
	info->iCharMenuEnterTime = 0;
	info->bCruiseActivated = false;
	info->tmKickTime = 0;
	info->iLastExitedBaseId = 0;
	info->bDisconnected = false;
	info->bCharSelected = false;
	info->tmF1Time = 0;
	info->tmF1TimeDisconnect = 0;

	const DamageList dmg;
	info->dmgLast = dmg;
	info->dieMsgSize = CS_DEFAULT;
	info->chatSize = CS_DEFAULT;
	info->chatStyle = CST_DEFAULT;

	info->Ignore.clear();
	info->iKillsInARow = 0;
	info->Hostname = L"";
	info->bEngineKilled = false;
	info->bThrusterActivated = false;
	info->bTradelane = false;
	info->iGroupId = 0;

	info->bSpawnProtected = false;

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
load settings from flhookhuser.ini
**************************************************************************************************************/

void LoadUserSettings(ClientId client)
{
	auto* info = &ClientInfo[client];

	const CAccount* acc = Players.FindAccountFromClientID(client);
	const std::wstring dir = Hk::Client::GetAccountDirName(acc);
	const std::string userFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini";

	// read diemsg settings
	info->dieMsg = static_cast<DIEMSGTYPE>(IniGetI(userFile, "settings", "DieMsg", DiemsgAll));
	info->dieMsgSize = static_cast<CHATSIZE>(IniGetI(userFile, "settings", "DieMsgSize", CS_DEFAULT));

	// read chatstyle settings
	info->chatSize = static_cast<CHATSIZE>(IniGetI(userFile, "settings", "ChatSize", CS_DEFAULT));
	info->chatStyle = static_cast<CHATSTYLE>(IniGetI(userFile, "settings", "ChatStyle", CST_DEFAULT));

	// read ignorelist
	info->Ignore.clear();
	for (int i = 1;; i++)
	{
		std::wstring Ignore = IniGetWS(userFile, "IgnoreList", std::to_string(i), L"");
		if (!Ignore.length())
			break;

		IgnoreInfo ii;
		ii.character = GetParam(Ignore, ' ', 0);
		ii.Flags = GetParam(Ignore, ' ', 1);
		info->Ignore.push_back(ii);
	}
}

/**************************************************************************************************************
load settings from flhookhuser.ini (specific to character)
**************************************************************************************************************/

void LoadUserCharSettings(ClientId client)
{
	CallPluginsAfter(HookedCall::FLHook__LoadCharacterSettings, client);
}

/**************************************************************************************************************
install the callback hooks
**************************************************************************************************************/

bool InitHookExports()
{
	// init critial sections
	InitializeCriticalSection(&csIPResolve);
	DWORD dwId;
	DWORD dwParam[34]; // else release version crashes, dont ask me why...
	hThreadResolver = CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)ThreadResolver, &dwParam, 0, &dwId);

	GetShipInspect = (_GetShipInspect)SRV_ADDR(ADDR_SRV_GETINSPECT);

	// install IServerImpl callbacks in remoteclient.dll
	auto* pServer = reinterpret_cast<char*>(&Server);
	memcpy(&pServer, pServer, 4);
	for (uint i = 0; i < std::size(IServerImplEntries); i++)
	{
		char* address = pServer + IServerImplEntries[i].dwRemoteAddress;
		ReadProcMem(address, &IServerImplEntries[i].fpOldProc, 4);
		WriteProcMem(address, &IServerImplEntries[i].fpProc, 4);
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
	ReadProcMem(address, RepFreeFixOld, 5);
	WriteProcMem(address, NOPs, 5);

	// patch flserver so it can better handle faulty house entries in char files

	// divert call to house load/save func
	address = SRV_ADDR(0x679C6);
	const char DivertJump[] = {'\x6F'};

	WriteProcMem(address, DivertJump, 1);

	// install hook at new address
	address = SRV_ADDR(0x78B39);

	const char MovEAX[] = {'\xB8'};
	const char JMPEAX[] = {'\xFF', '\xE0'};

	const auto fpLoadRepFromCharFile = (FARPROC)IEngineHook::Naked__LoadReputationFromCharacterFile;

	WriteProcMem(address, MovEAX, 1);
	WriteProcMem(address + 1, &fpLoadRepFromCharFile, 4);
	WriteProcMem(address + 5, JMPEAX, 2);

	IEngineHook::g_OldLoadReputationFromCharacterFile = (FARPROC)SRV_ADDR(0x78B40);

	// crc anti-cheat
	CRCAntiCheat = (_CRCAntiCheat)((char*)server + ADDR_CRCANTICHEAT);

	// get CDPServer
	address = DALIB_ADDR(ADDR_CDPSERVER);
	ReadProcMem(address, &cdpSrv, 4);

	// read g_FLServerDataPtr(used for serverload calc)
	address = FLSERVER_ADDR(ADDR_DATAPTR);
	ReadProcMem(address, &g_FLServerDataPtr, 4);

	// some setting relate hooks
	HookRehashed();

	// get client proxy array, used to retrieve player pings/ips
	address = (char*)remoteClient + ADDR_CPLIST;
	char* Temp;
	ReadProcMem(address, &Temp, 4);
	Temp += 0x10;
	memcpy(&clientProxyArray, &Temp, 4);

	// init variables
	char DataPath[MAX_PATH];
	GetUserDataPath(DataPath);
	CoreGlobals::i()->accPath = std::string(DataPath) + "\\Accts\\MultiPlayer\\";

	// Load DLLs for strings
	Hk::Message::LoadStringDLLs();

	// clear ClientInfo
	for (uint i = 0; i < ClientInfo.size(); i++)
	{
		ClientInfo[i].iConnects = 0; // only set to 0 on start
		ClearClientInfo(i);
	}

	// Fix Refire bug
	const std::array<byte, 22> refireBytes = {0x75, 0x0B, 0xC7, 0x84, 0x8C, 0x9C, 00, 00, 00, 00, 00, 00, 00, 0x41, 0x83, 0xC2, 0x04, 0x39, 0xC1, 0x7C, 0xE9, 0xEB};
	WriteProcMem(SRV_ADDR(0x02C057), refireBytes.data(), 22);

	// Enable undocking announcer regardless of distance
	const std::array<byte, 1> undockAnnouncerBytes = {0xEB};
	WriteProcMem(SRV_ADDR(0x173da), undockAnnouncerBytes.data(), 1);

	return true;
}

void PatchClientImpl()
{
	// install IClientImpl callback
	FakeClient = new IClientImpl;
	HookClient = &Client;

	memcpy(&OldClient, &Client, 4);
	WriteProcMem(&Client, FakeClient, 4);
}

/**************************************************************************************************************
uninstall the callback hooks
**************************************************************************************************************/

void UnloadHookExports()
{
	char* address;

	// uninstall IServerImpl callbacks in remoteclient.dll
	auto pServer = (char*)&Server;
	if (pServer)
	{
		memcpy(&pServer, pServer, 4);
		for (uint i = 0; i < std::size(IServerImplEntries); i++)
		{
			const auto address = pServer + IServerImplEntries[i].dwRemoteAddress;
			WriteProcMem(address, &IServerImplEntries[i].fpOldProc, 4);
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
	WriteProcMem(address, RepFreeFixOld, 5);

	// unpatch flserver so it can better handle faulty house entries in char
	// files

	// undivert call to house load/save func
	address = SRV_ADDR(0x679C6);
	char DivertJump[] = {'\x76'};

	// anti-death-msg
	const char Old[] = {'\x74'};
	address = SRV_ADDR(ADDR_ANTIdIEMSG);
	WriteProcMem(address, Old, 1);

	// plugins
	PluginManager::i()->unloadAll();

	// Undo refire bug
	const std::array<byte, 22> refireBytes = {0x74, 0x0A, 0x41, 0x83, 0xC2, 0x04, 0x3B, 0xC8, 0x7C, 0xF4, 0xEB, 0x0B, 0xC7, 0x84, 0x8C, 0x9C, 0, 0, 0, 0, 0, 0};
	WriteProcMem(SRV_ADDR(0x02C057), refireBytes.data(), 22);

	// undocking announcer regardless of distance
	const std::array<byte, 1> undockAnnouncerBytes = {0x74};
	WriteProcMem(SRV_ADDR(0x173da), undockAnnouncerBytes.data(), 1);
}

/**************************************************************************************************************
settings were rehashed
sometimes adjustments need to be made after a rehash
**************************************************************************************************************/

void HookRehashed()
{
	char* address;

	// anti-deathmsg
	if (FLHookConfig::i()->messages.dieMsg)
	{
		// disables the "old" "A Player has died: ..." messages
		const char JMP[] = {'\xEB'};
		address = SRV_ADDR(ADDR_ANTIdIEMSG);
		WriteProcMem(address, JMP, 1);
	}
	else
	{
		const char Old[] = {'\x74'};
		address = SRV_ADDR(ADDR_ANTIdIEMSG);
		WriteProcMem(address, Old, 1);
	}

	// charfile encyption(doesn't get disabled when unloading FLHook)
	if (FLHookConfig::i()->general.disableCharfileEncryption)
	{
		const char Buf[] = {'\x14', '\xB3'};
		address = SRV_ADDR(ADDR_DISCFENCR);
		WriteProcMem(address, Buf, 2);
		address = SRV_ADDR(ADDR_DISCFENCR2);
		WriteProcMem(address, Buf, 2);
	}
	else
	{
		const char Buf[] = {'\xE4', '\xB4'};
		address = SRV_ADDR(ADDR_DISCFENCR);
		WriteProcMem(address, Buf, 2);
		address = SRV_ADDR(ADDR_DISCFENCR2);
		WriteProcMem(address, Buf, 2);
	}

	// maximum group size
	if (FLHookConfig::i()->general.maxGroupSize > 0)
	{
		const char cNewGroupSize = FLHookConfig::i()->general.maxGroupSize & 0xFF;
		address = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE);
		WriteProcMem(address, &cNewGroupSize, 1);
		address = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE2);
		WriteProcMem(address, &cNewGroupSize, 1);
	}
	else
	{
		// default
		const char cNewGroupSize = 8;
		address = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE);
		WriteProcMem(address, &cNewGroupSize, 1);
		address = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE2);
		WriteProcMem(address, &cNewGroupSize, 1);
	}
}
