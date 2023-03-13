#include "Global.hpp"

bool g_bNPCDisabled;

namespace Hk::Admin
{
	std::wstring GetPlayerIP(ClientId client)
	{
		CDPClientProxy const* cdpClient = clientProxyArray[client - 1];
		if (!cdpClient)
			return L"";

		// get ip
		char* P1;
		char* IdirectPlay8Address;
		wchar_t wHostname[] = L"hostname";
		memcpy(&P1, (char*)cdpSrv + 4, 4);

		wchar_t wIP[1024] = L"";
		long lSize = sizeof(wIP);
		long lDataType = 1;
		__asm {
        push 0                          ; dwFlags
        lea edx, IdirectPlay8Address
        push edx                        ; address 
        mov edx, [cdpClient]
        mov edx, [edx+8]
        push edx                        ; dpnid
        mov eax, [P1]
        push eax
        mov ecx, [eax]
        call ord ptr[ecx + 0x28]      ; GetClientAddress
        cmp eax, 0
        jnz some_error

        lea eax, lDataType
        push eax
        lea eax, lSize
        push eax
        lea eax, wIP
        push eax
        lea eax, wHostname
        push eax
        mov ecx, [IdirectPlay8Address]
        push ecx
        mov ecx, [ecx]
        call ord ptr[ecx+0x40]        ; GetComponentByName

        mov ecx, [IdirectPlay8Address]
        push ecx
        mov ecx, [ecx]
        call ord ptr[ecx+0x08]        ; Release
some_error:
		}

		return wIP;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<PlayerInfo, Error> GetPlayerInfo(const std::variant<uint, std::wstring>& player, bool bAlsoCharmenu)
	{
		ClientId client = Hk::Client::ExtractClientID(player);

		if (client == UINT_MAX || (Hk::Client::IsInCharSelectMenu(client) && !bAlsoCharmenu))
			return cpp::fail(Error::PlayerNotLoggedIn);

		PlayerInfo pi;
		const wchar_t* wActiveCharname = (wchar_t*)Players.GetActiveCharacterName(client);

		pi.client = client;
		pi.character = wActiveCharname ? wActiveCharname : L"";
		pi.Base = pi.System = L"";

		uint iBase = 0;
		uint iSystem = 0;
		pub::Player::GetBase(client, iBase);
		pub::Player::GetSystem(client, iSystem);
		pub::Player::GetShip(client, pi.ship);

		if (iBase)
		{
			char Basename[1024] = "";
			pub::GetBaseNickname(Basename, sizeof(Basename), iBase);
			pi.Base = stows(Basename);
		}

		if (iSystem)
		{
			char Systemname[1024] = "";
			pub::GetSystemNickname(Systemname, sizeof(Systemname), iSystem);
			pi.System = stows(Systemname);
			pi.iSystem = iSystem;
		}

		// get ping
		auto ci = GetConnectionStats(client);
		if (ci.has_error())
		{
			AddLog(LogType::Normal, LogLevel::Warn, wstos(Hk::Err::ErrGetText(ci.error())));
			return cpp::fail(Error::PlayerNotLoggedIn);
		}
		pi.connectionInfo = ci.value();

		// get ip
		pi.IP = GetPlayerIP(client);

		pi.Hostname = ClientInfo[client].Hostname;

		return pi;
	}

	std::list<PlayerInfo> GetPlayers()
	{
		std::list<PlayerInfo> Ret;
		std::wstring Ret;

		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			ClientId client = playerDb->iOnlineId;

			if (Hk::Client::IsInCharSelectMenu(client))
				continue;

			auto pi = GetPlayerInfo(client, false);
			auto a = std::move(pi).value();
			Ret.emplace_back(a);
		}
		return Ret;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<DPN_CONNECTION_INFO, Error> GetConnectionStats(ClientId client)
	{
		if (client < 1 || client > MaxClientId)
			return cpp::fail(Error::InvalidClientId);

		CDPClientProxy* cdpClient = clientProxyArray[client - 1];

		DPN_CONNECTION_INFO ci;
		if (!cdpClient || !cdpClient->GetConnectionStats(&ci))
			return cpp::fail(Error::InvalidClientId);

		return ci;
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> SetAdmin(const std::variant<uint, std::wstring>& player, const std::wstring& Rights)
	{
		auto acc = Hk::Client::ExtractAccount(player);
		if (acc.has_error())
		{
			return cpp::fail(Error::CharacterDoesNotExist);
		}

		auto dir = Hk::Client::GetAccountDirName(acc.value());

		std::string AdminFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookadmin.ini";
		IniWrite(scAdminFile, "admin", "rights", wstos(Rights));
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
		std::string AdminFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookadmin.ini";

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
		std::string AdminFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookadmin.ini";
		DeleteFile(scAdminFile.c_str());
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> ChangeNPCSpawn(bool bDisable)
	{
		if (CoreGlobals::c()->disableNpcs && bDisable)
			return {};
		else if (!CoreGlobals::c()->disableNpcs && !bDisable)
			return {};

		char Jump[1];
		char Cmp[1];
		if (bDisable)
		{
			Jump[0] = '\xEB';
			Cmp[0] = '\xFF';
		}
		else
		{
			Jump[0] = '\x75';
			Cmp[0] = '\xF9';
		}

		void* address = CONTENT_ADDR(ADDR_DISABLENPCSPAWNS1);
		WriteProcMem(address, &Jump, 1);
		address = CONTENT_ADDR(ADDR_DISABLENPCSPAWNS2);
		WriteProcMem(address, &Cmp, 1);
		g_bNPCDisabled = bDisable;
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<BaseHealth, Error> GetBaseStatus(const std::wstring& Basename)
	{
		uint baseId = 0;
		pub::GetBaseID(baseId, wstos(Basename).c_str());
		if (!baseId)
		{
			return cpp::fail(Error::InvalidBaseName);
		}

		float curHealth;
		float maxHealth;

		Universe::IBase const* base = Universe::get_base(baseId);
		pub::SpaceObj::GetHealth(base->lSpaceObjId, curHealth, maxHealth);
		BaseHealth bh = {curHealth, maxHealth};
		return bh;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Fuse* GetFuseFromID(uint iFuseId)
	{
		int iDunno = 0;
		Fuse* fuse = nullptr;
		__asm {
        mov edx, 0x6CFD390
        call edx

        lea ecx, iFuseId
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
        call ord ptr[edx+0x150]
        pop edx
        pop ecx
        ret 4
		}
	}

	CEqObj* GetEqObjFromObjRW(struct IObjRW* objRW)
	{
		return GetEqObjFromObjRW_(objRW);
	}

	__declspec(naked) bool __stdcall LightFuse_(IObjRW* ship, uint iFuseId, float fDelay, float fLifetime, float fSkip)
	{
		__asm {
        lea eax, [esp+8] // iFuseId
        push [esp+20] // fSkip
        push [esp+16] // fDelay
        push 0 // SUBOBJ_Id_NONE
        push eax
        push [esp+32] // fLifetime
        mov ecx, [esp+24]
        mov eax, [ecx]
        call [eax+0x1E4]
        ret 20
		}
	}

	bool LightFuse(IObjRW* ship, uint iFuseId, float fDelay, float fLifetime, float fSkip)
	{
		return LightFuse_(ship, iFuseId, fDelay, fLifetime, fSkip);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Returns true if a fuse was unlit
	__declspec(naked) bool __stdcall UnLightFuse_(IObjRW* ship, uint iFuseId, float fDunno)
	{
		__asm {
        mov ecx, [esp+4]
        lea eax, [esp+8] // iFuseId
        push [esp+12] // fDunno
        push 0 // SUBOBJ_Id_NONE
        push eax // iFuseId
        mov eax, [ecx]
        call [eax+0x1E8]
        ret 12
		}
	}

	bool UnLightFuse(IObjRW* ship, uint iFuseId)
	{
		return UnLightFuse_(ship, iFuseId, 0.f);
	}
} // namespace Hk::Admin