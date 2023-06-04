#include "PCH.hpp"
#include "Global.hpp"
#include "Helpers/Admin.hpp"

#include "Defs/CoreGlobals.hpp"
#include "Helpers/Client.hpp"

bool g_bNPCDisabled;

namespace Hk::Admin
{
	std::wstring GetPlayerIP(ClientId client)
	{
		const CDPClientProxy* cdpClient = clientProxyArray[client - 1];
		if (!cdpClient)
			return L"";

		// get ip
		char* P1;
		char* IdirectPlay8Address;
		wchar_t hostname[] = L"hostname";
		memcpy(&P1, (char*)cdpSrv + 4, 4);

		wchar_t wIP[1024] = L"";
		long sizeofIP = sizeof wIP;
		long dataType = 1;
		__asm {
			push 0 ; flags
			lea edx, IdirectPlay8Address
			push edx ; address
			mov edx, [cdpClient]
			mov edx, [edx+8]
			push edx ; dpnid
			mov eax, [P1]
			push eax
			mov ecx, [eax]
			call dword ptr[ecx + 0x28] ; GetClientAddress
			cmp eax, 0
			jnz some_error

			lea eax, dataType
			push eax
			lea eax, sizeofIP
			push eax
			lea eax, wIP
			push eax
			lea eax, hostname
			push eax
			mov ecx, [IdirectPlay8Address]
			push ecx
			mov ecx, [ecx]
			call dword ptr[ecx+0x40] ; GetComponentByName

			mov ecx, [IdirectPlay8Address]
			push ecx
			mov ecx, [ecx]
			call dword ptr[ecx+0x08] ; Release
			some_error:
		}

		return wIP;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<PlayerInfo, Error> GetPlayerInfo(const std::variant<uint, std::wstring_view>& player, bool alsoCharmenu)
	{
		ClientId client = Client::ExtractClientID(player);

		if (client == UINT_MAX || (Client::IsInCharSelectMenu(client) && !alsoCharmenu))
			return cpp::fail(Error::PlayerNotLoggedIn);

		PlayerInfo pi;
		const wchar_t* activeCharname = (wchar_t*)Players.GetActiveCharacterName(client);

		pi.client = client;
		pi.character = activeCharname ? activeCharname : L"";
		pi.baseName = pi.systemName = L"";

		uint base = 0;
		uint system = 0;
		pub::Player::GetBase(client, base);
		pub::Player::GetSystem(client, system);
		pub::Player::GetShip(client, pi.ship);

		if (base)
		{
			char Basename[1024] = "";
			pub::GetBaseNickname(Basename, sizeof Basename, base);
			pi.baseName = StringUtils::stows(Basename);
		}

		if (system)
		{
			char Systemname[1024] = "";
			pub::GetSystemNickname(Systemname, sizeof Systemname, system);
			pi.systemName = StringUtils::stows(Systemname);
			pi.system = system;
		}

		// get ping
		auto ci = GetConnectionStats(client);
		if (ci.has_error())
		{
			Logger::i()->Log(LogLevel::Warn, L"Invalid client ID provided when getting connection stats");
			return cpp::fail(Error::PlayerNotLoggedIn);
		}
		pi.connectionInfo = ci.value();

		// get ip
		pi.IP = GetPlayerIP(client);

		pi.hostname = ClientInfo[client].hostname;

		return pi;
	}

	std::list<PlayerInfo> GetPlayers()
	{
		std::list<PlayerInfo> ret;

		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			ClientId client = playerDb->onlineId;

			if (Client::IsInCharSelectMenu(client))
				continue;

			auto pi = GetPlayerInfo(client, false);
			auto a = std::move(pi).value();
			ret.emplace_back(a);
		}
		return ret;
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

	cpp::result<void, Error> ChangeNPCSpawn(bool disable)
	{
		if (CoreGlobals::c()->disableNpcs && disable)
			return {};
		if (!CoreGlobals::c()->disableNpcs && !disable)
			return {};

		char jump[1];
		char cmp[1];
		if (disable)
		{
			jump[0] = '\xEB';
			cmp[0] = '\xFF';
		}
		else
		{
			jump[0] = '\x75';
			cmp[0] = '\xF9';
		}

		void* address = CONTENT_ADDR(ADDR_DISABLENPCSPAWNS1);
		MemUtils::WriteProcMem(address, &jump, 1);
		address = CONTENT_ADDR(ADDR_DISABLENPCSPAWNS2);
		MemUtils::WriteProcMem(address, &cmp, 1);
		g_bNPCDisabled = disable;
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<BaseHealth, Error> GetBaseStatus(const std::wstring& basename)
	{
		uint baseId = 0;
		pub::GetBaseID(baseId, StringUtils::wstos(basename).c_str());
		if (!baseId)
		{
			return cpp::fail(Error::InvalidBaseName);
		}

		float curHealth;
		float maxHealth;

		const Universe::IBase* base = Universe::get_base(baseId);
		pub::SpaceObj::GetHealth(base->spaceObjId, curHealth, maxHealth);
		BaseHealth bh = {curHealth, maxHealth};
		return bh;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Fuse* GetFuseFromID(uint fuseId)
	{
		int dunno = 0;
		Fuse* fuse = nullptr;
		__asm {
			mov edx, 0x6CFD390
			call edx

			lea ecx, fuseId
			push ecx
			lea ecx, dunno
			push ecx
			mov ecx, eax
			mov edx, 0x6D15D10
			call edx
			mov edx, [dunno]
			mov edi, [edx+0x10]
			mov fuse, edi
		}
		return fuse;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/// Return the CEqObj from the IObjRW
	__declspec(naked) CEqObj* __stdcall GetEqObjFromObjRW_(IObjRW* objRW)
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

	CEqObj* GetEqObjFromObjRW(IObjRW* objRW)
	{
		return GetEqObjFromObjRW_(objRW);
	}

	cpp::result<void, Error> AddRole(const std::wstring& characterName, const std::wstring& role)
	{
	}

	cpp::result<void, Error> RemoveRole(const std::wstring& characterName, const std::wstring& role)
	{
	}

	cpp::result<void, Error> SetRoles(const std::wstring& characterName, const std::vector<std::wstring>& roles)
	{
	}

	__declspec(naked) bool __stdcall LightFuse_(IObjRW* ship, uint fuseId, float delay, float lifetime, float skip)
	{
		__asm {
			lea eax, [esp+8] // fuseId
			push [esp+20] // skip
			push [esp+16] // delay
			push 0 // SUBOBJ_Id_NONE
			push eax
			push [esp+32] // lifetime
			mov ecx, [esp+24]
			mov eax, [ecx]
			call [eax+0x1E4]
			ret 20
		}
	}

	bool LightFuse(IObjRW* ship, uint fuseId, float delay, float lifetime, float skip)
	{
		return LightFuse_(ship, fuseId, delay, lifetime, skip);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Returns true if a fuse was unlit
	__declspec(naked) bool __stdcall UnLightFuse_([[maybe_unused]] const IObjRW* ship, [[maybe_unused]] uint fuseId, [[maybe_unused]] float dunno)
	{
		__asm {
			mov ecx, [esp+4]
			lea eax, [esp+8] // fuseId
			push [esp+12] // fdunno
			push 0 // SUBOBJ_Id_NONE
			push eax // fuseId
			mov eax, [ecx]
			call [eax+0x1E8]
			ret 12
		}
	}

	bool UnLightFuse(IObjRW* ship, uint fuseId)
	{
		return UnLightFuse_(ship, fuseId, 0.f);
	}
} // namespace Hk::Admin
