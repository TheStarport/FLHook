#include "Global.hpp"

bool g_bNPCDisabled;

namespace Hk::Admin
{
	std::wstring GetPlayerIP(uint clientId)
	{
		CDPClientProxy* cdpClient = g_cClientProxyArray[clientId - 1];
		if (!cdpClient)
			return L"";

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

		return wszIP;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<PLAYERINFO, Error> GetPlayerInfo(const std::variant<uint, std::wstring>& player, bool bAlsoCharmenu)
	{
		const uint clientId = Hk::Client::ExtractClientId(player);

		if (clientId == -1 || (Hk::Client::IsInCharSelectMenu(clientId) && !bAlsoCharmenu))
			return cpp::fail(Error::PlayerNotLoggedIn);

		PLAYERINFO pi;
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
		GetConnectionStats(clientId);
		pi.connectionInfo = ci;

		// get ip
		pi.wscIP = GetPlayerIP(clientId);

		pi.wscHostname = ClientInfo[clientId].wscHostname;

		return pi;
	}

	std::list<PLAYERINFO> GetPlayers()
	{
		std::list<PLAYERINFO> lstRet;
		std::wstring wscRet;

		struct PlayerData* playerDb = nullptr;
		while (playerDb = Players.traverse_active(playerDb))
		{
			uint clientId = playerDb->iOnlineID;

			if (Hk::Client::IsInCharSelectMenu(clientId))
				continue;

			/*auto pi = GetPlayerInfo(clientId, false);
			auto a = std::move(pi).value();
			lstRet.emplace_back(a);*/
		}

		return lstRet;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<DPN_CONNECTION_INFO, Error> GetConnectionStats(uint clientId)
	{
		if (clientId < 1 || clientId > MaxClientId)
			return cpp::fail(Error::InvalidClientId);

		CDPClientProxy* cdpClient = g_cClientProxyArray[clientId - 1];

		DPN_CONNECTION_INFO ci;
		if (!cdpClient || !cdpClient->GetConnectionStats(&ci))
			return cpp::fail(Error::InvalidClientId);

		return ci;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> SetAdmin(const std::variant<uint, std::wstring>& player, const std::wstring& wscRights)
	{
		auto acc = Hk::Client::ExtractAccount(player);
		if (acc.has_error())
		{
			return cpp::fail(Error::CharacterDoesNotExist);
		}

		auto dir = Hk::Client::GetAccountDirName(acc.value());

		std::string scAdminFile = scAcctPath + wstos(dir) + "\\flhookadmin.ini";
		IniWrite(scAdminFile, "admin", "rights", wstos(wscRights));
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<std::wstring, Error> GetAdmin(const std::variant<uint, std::wstring>& player)
	{
		auto acc = Hk::Client::ExtractAccount(player);
		if (acc.has_error())
		{
			return cpp::fail(acc.error());
		}

		std::wstring dir = Hk::Client::GetAccountDirName(acc.value());
		std::string scAdminFile = scAcctPath + wstos(dir) + "\\flhookadmin.ini";

		WIN32_FIND_DATA fd;
		HANDLE hFind = FindFirstFile(scAdminFile.c_str(), &fd);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			return cpp::fail(Error::NoAdmin);
		}

		FindClose(hFind);
		return stows(IniGetS(scAdminFile, "admin", "rights", ""));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> DelAdmin(const std::variant<uint, std::wstring>& player)
	{
		auto acc = Hk::Client::ExtractAccount(player);
		if (acc.has_error())
		{
			return cpp::fail(acc.error());
		}

		std::wstring dir = Hk::Client::GetAccountDirName(acc.value());
		std::string scAdminFile = scAcctPath + wstos(dir) + "\\flhookadmin.ini";
		DeleteFile(scAdminFile.c_str());
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool g_bNPCDisabled = false;

	cpp::result<void, Error> ChangeNPCSpawn(bool bDisable)
	{
		if (g_bNPCDisabled && bDisable)
			return {};
		else if (!g_bNPCDisabled && !bDisable)
			return {};

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
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<BaseHealth, Error> GetBaseStatus(const std::wstring& wscBasename)
	{
		uint baseId = 0;
		pub::GetBaseID(baseId, wstos(wscBasename).c_str());
		if (!baseId)
		{
			return cpp::fail(Error::InvalidBaseName);
		}

		float curHealth;
		float maxHealth;

		Universe::IBase const* base = Universe::get_base(baseId);
		pub::SpaceObj::GetHealth(base->lSpaceObjID, curHealth, maxHealth);
		BaseHealth bh = { curHealth, maxHealth };
		return bh;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Fuse* GetFuseFromID(uint iFuseID)
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
	__declspec(naked) CEqObj* __stdcall GetEqObjFromObjRW_(struct IObjRW* objRW)
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

	CEqObj* GetEqObjFromObjRW(struct IObjRW* objRW) { return GetEqObjFromObjRW_(objRW); }

	__declspec(naked) bool __stdcall LightFuse_(IObjRW* ship, uint iFuseID, float fDelay, float fLifetime, float fSkip)
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

	bool LightFuse(IObjRW* ship, uint iFuseID, float fDelay, float fLifetime, float fSkip) { return LightFuse_(ship, iFuseID, fDelay, fLifetime, fSkip); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Returns true if a fuse was unlit
	__declspec(naked) bool __stdcall UnLightFuse_(IObjRW* ship, uint iFuseID, float fDunno)
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

	bool UnLightFuse(IObjRW* ship, uint iFuseID) { return UnLightFuse_(ship, iFuseID, 0.f); }
}