#include "Global.hpp"

void HkGetPlayerIP(uint clientId, std::wstring& wscIP)
{
	wscIP = L"";
	CDPClientProxy* cdpClient = g_cClientProxyArray[clientId - 1];
	if (!cdpClient)
		return;

	// get ip
	char* szP1;
	char* szIDirectPlay8Address;
	wchar_t wszHostname[] = L"hostname";
	memcpy(&szP1, (char*)cdpSrv + 4, 4);

	wchar_t wszIP[1024] = L"";
	long lSize = sizeof(wszIP);
	long lDataType = 1;
	__asm {
        push 0                          ; dwFlags
        lea edx, szIDirectPlay8Address
        push edx                        ; pAddress 
        mov edx, [cdpClient]
        mov edx, [edx+8]
        push edx                        ; dpnid
        mov eax, [szP1]
        push eax
        mov ecx, [eax]
        call dword ptr[ecx + 0x28]      ; GetClientAddress
        cmp eax, 0
        jnz some_error

        lea eax, lDataType
        push eax
        lea eax, lSize
        push eax
        lea eax, wszIP
        push eax
        lea eax, wszHostname
        push eax
        mov ecx, [szIDirectPlay8Address]
        push ecx
        mov ecx, [ecx]
        call dword ptr[ecx+0x40]        ; GetComponentByName

        mov ecx, [szIDirectPlay8Address]
        push ecx
        mov ecx, [ecx]
        call dword ptr[ecx+0x08]        ; Release
some_error:
	}

	wscIP = wszIP;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkGetPlayerInfo(const std::variant<uint, std::wstring>& player, HKPLAYERINFO& pi, bool bAlsoCharmenu)
{
	const uint clientId = HkExtractClientId(player);

	if (clientId == -1 || (HkIsInCharSelectMenu(clientId) && !bAlsoCharmenu))
		return PlayerNotLoggedIn; // not on server

	const wchar_t* wszActiveCharname = (wchar_t*)Players.GetActiveCharacterName(clientId);

	pi.clientId = clientId;
	pi.character = wszActiveCharname ? wszActiveCharname : L"";
	pi.wscBase = pi.wscSystem = L"";

	uint iBase = 0;
	uint iSystem = 0;
	pub::Player::GetBase(clientId, iBase);
	pub::Player::GetSystem(clientId, iSystem);
	pub::Player::GetShip(clientId, pi.iShip);

	if (iBase)
	{
		char szBasename[1024] = "";
		pub::GetBaseNickname(szBasename, sizeof(szBasename), iBase);
		pi.wscBase = stows(szBasename);
	}

	if (iSystem)
	{
		char szSystemname[1024] = "";
		pub::GetSystemNickname(szSystemname, sizeof(szSystemname), iSystem);
		pi.wscSystem = stows(szSystemname);
		pi.iSystem = iSystem;
	}

	// get ping
	DPN_CONNECTION_INFO ci;
	HkGetConnectionStats(clientId, ci);
	pi.ci = ci;

	// get ip
	HkGetPlayerIP(clientId, pi.wscIP);

	pi.wscHostname = ClientInfo[clientId].wscHostname;

	return HKE_OK;
}

std::list<HKPLAYERINFO> HkGetPlayers()
{
	std::list<HKPLAYERINFO> lstRet;
	std::wstring wscRet;

	struct PlayerData* playerDb = 0;
	while (playerDb = Players.traverse_active(playerDb))
	{
		uint clientId = HkGetClientIdFromPD(playerDb);

		if (HkIsInCharSelectMenu(clientId))
			continue;

		HKPLAYERINFO pi;
		HkGetPlayerInfo(clientId, pi, false);
		lstRet.push_back(pi);
	}

	return lstRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkGetConnectionStats(uint clientId, DPN_CONNECTION_INFO& ci)
{
	if (clientId < 1 || clientId > MaxClientId)
		return InvalidClientId;

	CDPClientProxy* cdpClient = g_cClientProxyArray[clientId - 1];

	if (!cdpClient || !cdpClient->GetConnectionStats(&ci))
		return InvalidClientId;

	return HKE_OK;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkSetAdmin(const std::variant<uint, std::wstring>& player, const std::wstring& wscRights)
{
	auto* acc = HkExtractAccount(player);
	if (acc == nullptr)
	{
		return CharDoesNotExist;
	}

	std::wstring wscDir;
	HkGetAccountDirName(acc, wscDir);
	std::string scAdminFile = scAcctPath + wstos(wscDir) + "\\flhookadmin.ini";
	IniWrite(scAdminFile, "admin", "rights", wstos(wscRights));
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkGetAdmin(const std::variant<uint, std::wstring>& player, std::wstring& wscRights)
{
	wscRights = L"";
	auto* acc = HkExtractAccount(player);
	if (acc == nullptr)
	{
		return CharDoesNotExist;
	}

	std::wstring wscDir;
	HkGetAccountDirName(acc, wscDir);
	std::string scAdminFile = scAcctPath + wstos(wscDir) + "\\flhookadmin.ini";

	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(scAdminFile.c_str(), &fd);
	if (hFind == INVALID_HANDLE_VALUE)
		return NoAdmin;
	;

	FindClose(hFind);
	wscRights = stows(IniGetS(scAdminFile, "admin", "rights", ""));

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkDelAdmin(const std::variant<uint, std::wstring>& player)
{
	auto* acc = HkExtractAccount(player);
	if (acc == nullptr)
	{
		return CharDoesNotExist;
	}

	std::wstring wscDir;
	HkGetAccountDirName(acc, wscDir);
	std::string scAdminFile = scAcctPath + wstos(wscDir) + "\\flhookadmin.ini";
	DeleteFile(scAdminFile.c_str());
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool g_bNPCDisabled = false;

HkError HkChangeNPCSpawn(bool bDisable)
{
	if (g_bNPCDisabled && bDisable)
		return HKE_OK;
	else if (!g_bNPCDisabled && !bDisable)
		return HKE_OK;

	char szJump[1];
	char szCmp[1];
	if (bDisable)
	{
		szJump[0] = '\xEB';
		szCmp[0] = '\xFF';
	}
	else
	{
		szJump[0] = '\x75';
		szCmp[0] = '\xF9';
	}

	void* pAddress = CONTENT_ADDR(ADDR_DISABLENPCSPAWNS1);
	WriteProcMem(pAddress, &szJump, 1);
	pAddress = CONTENT_ADDR(ADDR_DISABLENPCSPAWNS2);
	WriteProcMem(pAddress, &szCmp, 1);
	g_bNPCDisabled = bDisable;
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HkError HkGetBaseStatus(const std::wstring& wscBasename, float& fHealth, float& fMaxHealth)
{
	uint baseId = 0;
	pub::GetBaseID(baseId, wstos(wscBasename).c_str());
	if (!baseId)
	{
		return InvalidBaseName;
	}

	Universe::IBase* base = Universe::get_base(baseId);
	pub::SpaceObj::GetHealth(base->lSpaceObjID, fHealth, fMaxHealth);
	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

Fuse* HkGetFuseFromID(uint iFuseID)
{
	int iDunno;
	Fuse* fuse;
	__asm {
        mov edx, 0x6CFD390
        call edx

        lea ecx, iFuseID
        push ecx
        lea ecx, iDunno
        push ecx
        mov ecx, eax
        mov edx, 0x6D15D10
        call edx
        mov edx, [iDunno]
        mov edi, [edx+0x10]
        mov fuse, edi
	}
	return fuse;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Return the CEqObj from the IObjRW
__declspec(naked) CEqObj* __stdcall HkGetEqObjFromObjRW_(struct IObjRW* objRW)
{
	__asm {
        push ecx
        push edx
        mov ecx, [esp+12]
        mov edx, [ecx]
        call dword ptr[edx+0x150]
        pop edx
        pop ecx
        ret 4
	}
}

CEqObj* HkGetEqObjFromObjRW(struct IObjRW* objRW)
{
	return HkGetEqObjFromObjRW_(objRW);
}

__declspec(naked) bool __stdcall HkLightFuse_(IObjRW* ship, uint iFuseID, float fDelay, float fLifetime, float fSkip)
{
	__asm {
        lea eax, [esp+8] // iFuseID
        push [esp+20] // fSkip
        push [esp+16] // fDelay
        push 0 // SUBOBJ_ID_NONE
        push eax
        push [esp+32] // fLifetime
        mov ecx, [esp+24]
        mov eax, [ecx]
        call [eax+0x1E4]
        ret 20
	}
}

bool HkLightFuse(IObjRW* ship, uint iFuseID, float fDelay, float fLifetime, float fSkip)
{
	return HkLightFuse_(ship, iFuseID, fDelay, fLifetime, fSkip);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Returns true if a fuse was unlit
__declspec(naked) bool __stdcall HkUnLightFuse_(IObjRW* ship, uint iFuseID, float fDunno)
{
	__asm {
        mov ecx, [esp+4]
        lea eax, [esp+8] // iFuseID
        push [esp+12] // fDunno
        push 0 // SUBOBJ_ID_NONE
        push eax // iFuseID
        mov eax, [ecx]
        call [eax+0x1E8]
        ret 12
	}
}

bool HkUnLightFuse(IObjRW* ship, uint iFuseID)
{
	return HkUnLightFuse_(ship, iFuseID, 0.f);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint HkGetClientIDFromArg(const std::wstring& wscArg)
{
	uint clientId;

	if (HkResolveId(wscArg, clientId) == HKE_OK)
		return clientId;

	if (HkResolveShortCut(wscArg, clientId) == HKE_OK)
		return clientId;

	return HkGetClientIdFromCharname(wscArg);
}
