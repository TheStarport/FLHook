#include "hook.h"

#include <process.h>

namespace HkIEngine
{
	void __cdecl UpdateTime(double interval);
	void __stdcall ElapseTime(float interval);
	int __cdecl DockCall(const uint& shipID, const uint& spaceID, int flags, DOCK_HOST_RESPONSE response);
	int __cdecl FreeReputationVibe(int const& p1);

	void Naked__CShip__Init();
	void Naked__CShip__Destroy();
	void Naked__LaunchPosition();
	void Naked__LoadReputationFromCharacterFile();

	extern FARPROC g_OldInitCShip;
	extern FARPROC g_OldDestroyCShip;
	extern FARPROC g_OldLaunchPosition;
	extern FARPROC g_OldLoadReputationFromCharacterFile;
} // namespace HkIEngine

void __stdcall ShipDestroyed(DamageList* dmgList, DWORD* ecx, uint kill);
void __stdcall NonGunWeaponHitsBaseBefore(char* ECX, char* p1, DamageList* dmg);
void __stdcall SendChat(uint clientID, uint clientIDTo, uint size, void* rdl);

extern FARPROC g_OldShipDestroyed;
extern FARPROC g_OldNonGunWeaponHitsBase;
extern FARPROC g_OldDamageHit, g_OldDamageHit2;
extern FARPROC g_OldDisconnectPacketSent;
extern FARPROC g_OldGuidedHit;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

PATCH_INFO piFLServerEXE = { "flserver.exe",
	                         0x0400000,
	                         {
	                             { 0x041B094, &HkIEngine::UpdateTime, 4, 0, false },
	                             { 0x041BAB0, &HkIEngine::ElapseTime, 4, 0, false },

	                             { 0, 0, 0, 0 } // terminate
	                         } };

PATCH_INFO piContentDLL = { "content.dll",
	                        0x6EA0000,
	                        {
	                            { 0x6FB358C, &HkIEngine::DockCall, 4, 0, false },

	                            { 0, 0, 0, 0 } // terminate
	                        } };

PATCH_INFO piCommonDLL = { "common.dll",
	                       0x6260000,
	                       {

	                           { 0x0639C138, &HkIEngine::Naked__CShip__Init, 4, &HkIEngine::g_OldInitCShip, false },
	                           { 0x0639C064, &HkIEngine::Naked__CShip__Destroy, 4, &HkIEngine::g_OldDestroyCShip,
	                             false },

	                           { 0, 0, 0, 0 } // terminate
	                       } };

PATCH_INFO piServerDLL = { "server.dll",
	                       0x6CE0000,
	                       {
	                           { 0x6D67274, &Naked__ShipDestroyed, 4, &g_OldShipDestroyed, false },
	                           { 0x6D641EC, &Naked__AddDamageEntry, 4, 0, false },
	                           { 0x6D67320, &Naked__GuidedHit, 4, &g_OldGuidedHit, false },
	                           { 0x6D65448, &Naked__GuidedHit, 4, 0, false },
	                           { 0x6D67670, &Naked__GuidedHit, 4, 0, false },
	                           { 0x6D653F4, &Naked__DamageHit, 4, &g_OldDamageHit, false },
	                           { 0x6D672CC, &Naked__DamageHit, 4, 0, false },
	                           { 0x6D6761C, &Naked__DamageHit, 4, 0, false },
	                           { 0x6D65458, &Naked__DamageHit2, 4, &g_OldDamageHit2, false },
	                           { 0x6D67330, &Naked__DamageHit2, 4, 0, false },
	                           { 0x6D67680, &Naked__DamageHit2, 4, 0, false },
	                           { 0x6D67668, &Naked__NonGunWeaponHitsBase, 4, &g_OldNonGunWeaponHitsBase, false },
	                           { 0x6D6420C, &HkIEngine::Naked__LaunchPosition, 4, &HkIEngine::g_OldLaunchPosition,
	                             false },
	                           { 0x6D648E0, &HkIEngine::FreeReputationVibe, 4, 0, false },

	                           { 0, 0, 0, 0 } // terminate
	                       } };

PATCH_INFO piRemoteClientDLL = { "remoteclient.dll",
	                             0x6B30000,
	                             {
	                                 { 0x6B6BB80, &SendChat, 4, &RCSendChatMsg, false },

	                                 { 0, 0, 0, 0 } // terminate
	                             } };

PATCH_INFO piDaLibDLL = { "dalib.dll",
	                      0x65C0000,
	                      {
	                          { 0x65C4BEC, &Naked__DisconnectPacketSent, 4, &g_OldDisconnectPacketSent, false },

	                          { 0, 0, 0, 0 } // terminate
	                      } };

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Patch(PATCH_INFO& pi)
{
	HMODULE hMod = GetModuleHandle(pi.szBinName);
	if (!hMod)
		return false;

	for (uint i = 0; (i < sizeof(pi.piEntries) / sizeof(PATCH_INFO_ENTRY)); i++)
	{
		if (!pi.piEntries[i].pAddress)
			break;

		char* pAddress = (char*)hMod + (pi.piEntries[i].pAddress - pi.pBaseAddress);
		if (!pi.piEntries[i].pOldValue)
		{
			pi.piEntries[i].pOldValue = new char[pi.piEntries[i].iSize];
			pi.piEntries[i].bAlloced = true;
		}
		else
			pi.piEntries[i].bAlloced = false;

		ReadProcMem(pAddress, pi.piEntries[i].pOldValue, pi.piEntries[i].iSize);
		WriteProcMem(pAddress, &pi.piEntries[i].pNewValue, pi.piEntries[i].iSize);
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool RestorePatch(PATCH_INFO& pi)
{
	HMODULE hMod = GetModuleHandle(pi.szBinName);
	if (!hMod)
		return false;

	for (uint i = 0; (i < sizeof(pi.piEntries) / sizeof(PATCH_INFO_ENTRY)); i++)
	{
		if (!pi.piEntries[i].pAddress)
			break;

		char* pAddress = (char*)hMod + (pi.piEntries[i].pAddress - pi.pBaseAddress);
		WriteProcMem(pAddress, pi.piEntries[i].pOldValue, pi.piEntries[i].iSize);
		if (pi.piEntries[i].bAlloced)
			delete[] pi.piEntries[i].pOldValue;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

CDPClientProxy** g_cClientProxyArray;

CDPServer* cdpSrv;

HkIClientImpl* FakeClient;
HkIClientImpl* HookClient;
char* OldClient;

_CRCAntiCheat CRCAntiCheat;
_CreateChar CreateChar;

std::string scAcctPath;

CLIENT_INFO ClientInfo[MAX_CLIENT_ID + 1];

uint g_iServerLoad = 0;
uint g_iPlayerCount = 0;

char* g_FLServerDataPtr;

_GetShipInspect GetShipInspect;

std::list<BASE_INFO> lstBases;

char szRepFreeFixOld[5];

/**************************************************************************************************************
clear the clientinfo
**************************************************************************************************************/

void ClearClientInfo(uint clientID)
{
	auto* info = &ClientInfo[clientID];

	info->dieMsg = DIEMSG_ALL;
	info->iShip = 0;
	info->iShipOld = 0;
	info->tmSpawnTime = 0;
	info->lstMoneyFix.clear();
	info->iTradePartner = 0;
	info->iBaseEnterTime = 0;
	info->iCharMenuEnterTime = 0;
	info->bCruiseActivated = false;
	info->tmKickTime = 0;
	info->iLastExitedBaseID = 0;
	info->bDisconnected = false;
	info->bCharSelected = false;
	info->tmF1Time = 0;
	info->tmF1TimeDisconnect = 0;

	DamageList dmg;
	info->dmgLast = dmg;
	info->dieMsgSize = CS_DEFAULT;
	info->chatSize = CS_DEFAULT;
	info->chatStyle = CST_DEFAULT;

	info->bAutoBuyMissiles = false;
	info->bAutoBuyMines = false;
	info->bAutoBuyTorps = false;
	info->bAutoBuyCD = false;
	info->bAutoBuyCM = false;
	info->bAutoBuyReload = false;

	info->lstIgnore.clear();
	info->iKillsInARow = 0;
	info->wscHostname = L"";
	info->bEngineKilled = false;
	info->bThrusterActivated = false;
	info->bTradelane = false;
	info->iGroupID = 0;

	info->bSpawnProtected = false;

	for (auto& i : info->mapPluginData)
	{
		i.second.fill(0x0);
	}

	HkCharacterClearClientInfo(clientID);

	CallPluginsAfter(HookedCall::FLHook__ClearClientInfo, clientID);
}

/**************************************************************************************************************
load settings from flhookhuser.ini
**************************************************************************************************************/

void LoadUserSettings(uint iClientID)
{
	auto* info = &ClientInfo[iClientID];

	CAccount* acc = Players.FindAccountFromClientID(iClientID);
	std::wstring wscDir;
	HkGetAccountDirName(acc, wscDir);
	std::string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

	// read diemsg settings
	info->dieMsg = (DIEMSGTYPE)IniGetI(scUserFile, "settings", "DieMsg", DIEMSG_ALL);
	info->dieMsgSize = (CHATSIZE)IniGetI(scUserFile, "settings", "DieMsgSize", CS_DEFAULT);

	// read chatstyle settings
	info->chatSize = (CHATSIZE)IniGetI(scUserFile, "settings", "ChatSize", CS_DEFAULT);
	info->chatStyle = (CHATSTYLE)IniGetI(scUserFile, "settings", "ChatStyle", CST_DEFAULT);

	// read ignorelist
	info->lstIgnore.clear();
	for (int i = 1;; i++)
	{
		std::wstring wscIgnore = IniGetWS(scUserFile, "IgnoreList", std::to_string(i), L"");
		if (!wscIgnore.length())
			break;

		IGNORE_INFO ii;
		ii.wscCharname = GetParam(wscIgnore, ' ', 0);
		ii.wscFlags = GetParam(wscIgnore, ' ', 1);
		info->lstIgnore.push_back(ii);
	}
}

/**************************************************************************************************************
load settings from flhookhuser.ini (specific to character)
**************************************************************************************************************/

void LoadUserCharSettings(uint clientID)
{
	auto* info = &ClientInfo[clientID];

	CAccount* acc = Players.FindAccountFromClientID(clientID);
	std::wstring wscDir;
	HkGetAccountDirName(acc, wscDir);
	std::string scUserFile = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";

	// read autobuy
	std::wstring wscFilename;
	HkGetCharFileName((wchar_t*)Players.GetActiveCharacterName(clientID), wscFilename);
	std::string scSection = "autobuy_" + wstos(wscFilename);

	info->bAutoBuyMissiles = IniGetB(scUserFile, scSection, "missiles", false);
	info->bAutoBuyMines = IniGetB(scUserFile, scSection, "mines", false);
	info->bAutoBuyTorps = IniGetB(scUserFile, scSection, "torps", false);
	info->bAutoBuyCD = IniGetB(scUserFile, scSection, "cd", false);
	info->bAutoBuyCM = IniGetB(scUserFile, scSection, "cm", false);
	info->bAutoBuyReload = IniGetB(scUserFile, scSection, "reload", false);

	CallPluginsAfter(HookedCall::FLHook__LoadCharacterSettings, clientID);
}

/**************************************************************************************************************
install the callback hooks
**************************************************************************************************************/

bool InitHookExports()
{
	// init critial sections
	InitializeCriticalSection(&csIPResolve);
	DWORD dwID;
	DWORD dwParam[34]; // else release version crashes, dont ask me why...
	hThreadResolver = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HkThreadResolver, &dwParam, 0, &dwID);

	GetShipInspect = (_GetShipInspect)SRV_ADDR(ADDR_SRV_GETINSPECT);

	// install IServerImpl callbacks in remoteclient.dll
	auto* pServer = reinterpret_cast<char*>(&Server);
	memcpy(&pServer, pServer, 4);
	for (uint i = 0; i < std::size(HkIServerImplEntries); i++)
	{
		char* pAddress = pServer + HkIServerImplEntries[i].dwRemoteAddress;
		ReadProcMem(pAddress, &HkIServerImplEntries[i].fpOldProc, 4);
		WriteProcMem(pAddress, &HkIServerImplEntries[i].fpProc, 4);
	}

	// patch it
	Patch(piFLServerEXE);
	Patch(piContentDLL);
	Patch(piCommonDLL);
	Patch(piServerDLL);
	Patch(piRemoteClientDLL);
	Patch(piDaLibDLL);

	// patch rep array free
	char szNOPs[] = { '\x90', '\x90', '\x90', '\x90', '\x90' };
	char* pAddress = ((char*)hModServer + ADDR_SRV_REPARRAYFREE);
	ReadProcMem(pAddress, szRepFreeFixOld, 5);
	WriteProcMem(pAddress, szNOPs, 5);

	// patch flserver so it can better handle faulty house entries in char files

	// divert call to house load/save func
	pAddress = SRV_ADDR(0x679C6);
	char szDivertJump[] = { '\x6F' };

	WriteProcMem(pAddress, szDivertJump, 1);

	// install hook at new address
	pAddress = SRV_ADDR(0x78B39);

	char szMovEAX[] = { '\xB8' };
	char szJMPEAX[] = { '\xFF', '\xE0' };

	FARPROC fpHkLoadRepFromCharFile = (FARPROC)HkIEngine::Naked__LoadReputationFromCharacterFile;

	WriteProcMem(pAddress, szMovEAX, 1);
	WriteProcMem(pAddress + 1, &fpHkLoadRepFromCharFile, 4);
	WriteProcMem(pAddress + 5, szJMPEAX, 2);

	HkIEngine::g_OldLoadReputationFromCharacterFile = (FARPROC)SRV_ADDR(0x78B40);

	// crc anti-cheat
	CRCAntiCheat = (_CRCAntiCheat)((char*)hModServer + ADDR_CRCANTICHEAT);

	// get CDPServer
	pAddress = DALIB_ADDR(ADDR_CDPSERVER);
	ReadProcMem(pAddress, &cdpSrv, 4);

	// read g_FLServerDataPtr(used for serverload calc)
	pAddress = FLSERVER_ADDR(ADDR_DATAPTR);
	ReadProcMem(pAddress, &g_FLServerDataPtr, 4);

	// some setting relate hooks
	HookRehashed();

	// get client proxy array, used to retrieve player pings/ips
	pAddress = (char*)hModRemoteClient + ADDR_CPLIST;
	char* szTemp;
	ReadProcMem(pAddress, &szTemp, 4);
	szTemp += 0x10;
	memcpy(&g_cClientProxyArray, &szTemp, 4);

	// init variables
	char szDataPath[MAX_PATH];
	GetUserDataPath(szDataPath);
	scAcctPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\";

	// clear ClientInfo
	for (uint i = 0; (i < sizeof(ClientInfo) / sizeof(CLIENT_INFO)); i++)
	{
		ClientInfo[i].iConnects = 0; // only set to 0 on start
		ClearClientInfo(i);
	}

	return true;
}

void PatchClientImpl()
{
	// install HkIClientImpl callback

	FakeClient = new HkIClientImpl;
	HookClient = &Client;

	memcpy(&OldClient, &Client, 4);
	WriteProcMem(&Client, FakeClient, 4);
}

/**************************************************************************************************************
uninstall the callback hooks
**************************************************************************************************************/

void UnloadHookExports()
{
	char* pAddress;

	// uninstall IServerImpl callbacks in remoteclient.dll
	char* pServer = (char*)&Server;
	if (pServer)
	{
		memcpy(&pServer, pServer, 4);
		for (uint i = 0; i < std::size(HkIServerImplEntries); i++)
		{
			void* pAddress = (void*)((char*)pServer + HkIServerImplEntries[i].dwRemoteAddress);
			WriteProcMem(pAddress, &HkIServerImplEntries[i].fpOldProc, 4);
		}
	}

	// reset npc spawn setting
	HkChangeNPCSpawn(false);

	// restore other hooks
	RestorePatch(piFLServerEXE);
	RestorePatch(piContentDLL);
	RestorePatch(piCommonDLL);
	RestorePatch(piServerDLL);
	RestorePatch(piRemoteClientDLL);
	RestorePatch(piDaLibDLL);

	// unpatch rep array free
	pAddress = ((char*)GetModuleHandle("server.dll") + ADDR_SRV_REPARRAYFREE);
	WriteProcMem(pAddress, szRepFreeFixOld, 5);

	// unpatch flserver so it can better handle faulty house entries in char
	// files

	// undivert call to house load/save func
	pAddress = SRV_ADDR(0x679C6);
	char szDivertJump[] = { '\x76' };

	// anti-death-msg
	char szOld[] = { '\x74' };
	pAddress = SRV_ADDR(ADDR_ANTIDIEMSG);
	WriteProcMem(pAddress, szOld, 1);

	// plugins
	PluginManager::i()->unloadAll();

	// help
	lstHelpEntries.clear();
}

/**************************************************************************************************************
settings were rehashed
sometimes adjustments need to be made after a rehash
**************************************************************************************************************/

void HookRehashed()
{
	char* pAddress;

	// anti-deathmsg
	if (set_bDieMsg)
	{ // disables the "old" "A Player has died: ..." messages
		char szJMP[] = { '\xEB' };
		pAddress = SRV_ADDR(ADDR_ANTIDIEMSG);
		WriteProcMem(pAddress, szJMP, 1);
	}
	else
	{
		char szOld[] = { '\x74' };
		pAddress = SRV_ADDR(ADDR_ANTIDIEMSG);
		WriteProcMem(pAddress, szOld, 1);
	}

	// charfile encyption(doesn't get disabled when unloading FLHook)
	if (set_bDisableCharfileEncryption)
	{
		char szBuf[] = { '\x14', '\xB3' };
		pAddress = SRV_ADDR(ADDR_DISCFENCR);
		WriteProcMem(pAddress, szBuf, 2);
		pAddress = SRV_ADDR(ADDR_DISCFENCR2);
		WriteProcMem(pAddress, szBuf, 2);
	}
	else
	{
		char szBuf[] = { '\xE4', '\xB4' };
		pAddress = SRV_ADDR(ADDR_DISCFENCR);
		WriteProcMem(pAddress, szBuf, 2);
		pAddress = SRV_ADDR(ADDR_DISCFENCR2);
		WriteProcMem(pAddress, szBuf, 2);
	}

	// maximum group size
	if (set_iMaxGroupSize > 0)
	{
		char cNewGroupSize = set_iMaxGroupSize & 0xFF;
		pAddress = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE);
		WriteProcMem(pAddress, &cNewGroupSize, 1);
		pAddress = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE2);
		WriteProcMem(pAddress, &cNewGroupSize, 1);
	}
	else
	{ // default
		char cNewGroupSize = 8;
		pAddress = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE);
		WriteProcMem(pAddress, &cNewGroupSize, 1);
		pAddress = SRV_ADDR(ADDR_SRV_MAXGROUPSIZE2);
		WriteProcMem(pAddress, &cNewGroupSize, 1);
	}
}
