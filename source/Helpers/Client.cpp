#include "Global.hpp"

namespace Hk::Client
{
	cpp::result<const uint, Error> GetClientID(const std::wstring& character)
	{
		if (character.find(L"id ") == std::string::npos) 
		{
			return cpp::fail(Error::InvalidIdString);
		}

		auto resolvedId = ResolveID(character);
		if (resolvedId.has_error() || resolvedId.error() == Error::InvalidIdString)
		{
			resolvedId = ResolveShortCut(character);
			if (resolvedId.has_error())
			{
				if ((resolvedId.error() == Error::AmbiguousShortcut) || (resolvedId.error() == Error::NoMatchingPlayer))
				{
					return cpp::fail(resolvedId.error());
				}

				if (resolvedId.error() == Error::InvalidShortcutString)
				{
					resolvedId = GetClientIdFromCharName(character);
					if (resolvedId.has_value())
						return resolvedId.value();

					else
						return cpp::fail(Error::PlayerNotLoggedIn);
				}

				return cpp::fail(resolvedId.error());
			}
		}

		return resolvedId.value();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const uint, Error> GetClientIdFromAccount(CAccount* acc)
	{
		struct PlayerData* playerDb = nullptr;
		while (playerDb = Players.traverse_active(playerDb))
		{
			if (playerDb->Account == acc)
			{
				return playerDb->iOnlineId;
			}
		}

		return cpp::fail(Error::PlayerNotLoggedIn);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<CAccount*, Error> GetAccountByCharName(const std::wstring& character)
	{
		st6::wstring flStr((ushort*)character.c_str());
		CAccount* acc = Players.FindAccountFromCharacterName(flStr);

		if (!acc)
			return cpp::fail(Error::CharacterDoesNotExist);

		return acc;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const uint, Error> GetClientIdFromCharName(const std::wstring& character)
	{
		const auto acc = GetAccountByCharName(character);
		if (acc.has_error())
			return cpp::fail(acc.error());

		const auto client = GetClientIdFromAccount(acc.value());
		if (client.has_error())
			return cpp::fail(client.error());

		const auto newCharacter = GetCharacterNameByID(client.value());
		if (newCharacter.has_error())
			return cpp::fail(newCharacter.error());


		if (ToLower(newCharacter.value()).compare(ToLower(character)) != 0)
			return cpp::fail(Error::CharacterDoesNotExist);

		return client;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, Error> GetAccountID(CAccount* acc)
	{
		if (acc && acc->wszAccId)
			return acc->wszAccId;

		return cpp::fail(Error::CannotGetAccount);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool IsEncoded(const std::string& scFilename)
	{
		bool bRet = false;
		FILE* f;
		fopen_s(&f, scFilename.c_str(), "r");
		if (!f)
			return false;

		char szMagic[] = "FLS1";
		char szFile[sizeof(szMagic)] = "";
		fread(szFile, 1, sizeof(szMagic), f);
		if (!strncmp(szMagic, szFile, sizeof(szMagic) - 1))
			bRet = true;
		fclose(f);

		return bRet;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool IsInCharSelectMenu(const uint& player)
	{
		ClientId client = Hk::Client::ExtractClientID(player);
		if (client == UINT_MAX)
			return false;

		uint iBase = 0;
		uint iSystem = 0;
		pub::Player::GetBase(client, iBase);
		pub::Player::GetSystem(client, iSystem);
		if (!iBase && !iSystem)
			return true;
		else
			return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool IsValidClientID(ClientId client)
	{
		struct PlayerData* playerDb = nullptr;
		while (playerDb = Players.traverse_active(playerDb))
		{
			if (playerDb->iOnlineId == client)
				return true;
		}

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, Error> GetCharacterNameByID(ClientId& client)
	{
		if (!IsValidClientID(client))
			return cpp::fail(Error::InvalidClientId);

		return reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const uint, Error> ResolveID(const std::wstring& character)
	{
		if (const std::wstring characterLower = ToLower(character); characterLower.find(L"id ") == 0)
		{
			uint iId = 0;
			swscanf_s(characterLower.c_str(), L"id %u", &iId);
			if (!IsValidClientID(iId))
				return cpp::fail(Error::InvalidClientId);

			return iId;
		}

		return cpp::fail(Error::InvalidIdString);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<ClientId, Error> ResolveShortCut(const std::wstring& wscShortcut)
	{
		std::wstring wscShortcutLower = ToLower(wscShortcut);
		if (wscShortcutLower.find(L"sc ") != 0)
			return cpp::fail(Error::InvalidShortcutString);

		wscShortcutLower = wscShortcutLower.substr(3);

		uint clientFound = UINT_MAX;
		struct PlayerData* playerDb = nullptr;
		while (playerDb = Players.traverse_active(playerDb))
		{
			const auto characterName = GetCharacterNameByID(playerDb->iOnlineId);
			if (characterName.has_error())
				continue;

			if (ToLower(characterName.value()).find(wscShortcutLower) != -1)
			{
				if (clientFound == UINT_MAX)
					clientFound = playerDb->iOnlineId;
				else
					return cpp::fail(Error::AmbiguousShortcut);
			}
		}

		if (clientFound == UINT_MAX)
			return cpp::fail(Error::NoMatchingPlayer);

		return clientFound;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<ClientId, Error> GetClientIdByShip(const ShipId ship)
	{
		for (uint i = 0; i <= MaxClientId; i++)
		{
			if (ClientInfo[i].ship == ship || ClientInfo[i].shipOld == ship)
				return i;
		}

		return cpp::fail(Error::InvalidShip);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::wstring GetAccountDirName(const CAccount* acc)
	{
		const auto GetFLName = reinterpret_cast<_GetFLName>(reinterpret_cast<char*>(hModServer) + 0x66370);

		char szDir[1024] = "";
		GetFLName(szDir, acc->wszAccId);
		return stows(szDir);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring>& player, bool returnValueIfNoFile)
	{
		static _GetFLName GetFLName = nullptr;
		if (!GetFLName)
			GetFLName = (_GetFLName)((char*)hModServer + 0x66370);

		std::string buffer;
		buffer.reserve(1024);

		if (ClientId client = Hk::Client::ExtractClientID(player); client != UINT_MAX)
		{
			if (const auto character = GetCharacterNameByID(client); character.has_error())
				return cpp::fail(character.error());

			GetFLName(buffer.data(), reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client)));
		}
		else if ((player.index() && GetAccountByCharName(std::get<std::wstring>(player))) || returnValueIfNoFile)
		{
			GetFLName(buffer.data(), std::get<std::wstring>(player).c_str());
		}
		else
		{
			return cpp::fail(Error::InvalidClientId);
		}

		return stows(buffer);
	}

	cpp::result<const std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring>& player)
	{
		return GetCharFileName(player, false);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, Error> GetBaseNickByID(uint baseId)
	{
		std::string base;
		base.resize(1024);
		pub::GetBaseNickname(base.data(), base.capacity(), baseId);
		base.resize(1024); // Without calling another core function will result in length not being updated

		if (base.empty())
			return cpp::fail(Error::InvalidBase);

		return stows(base);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, Error> GetSystemNickByID(uint systemId)
	{
		std::string system;
		system.resize(1024);
		pub::GetSystemNickname(system.data(), system.capacity(), systemId);
		system.resize(1024); // Without calling another core function will result in length not being updated

		if (system.empty())
			return cpp::fail(Error::InvalidSystem);
		
		return stows(system);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, Error> GetPlayerSystem(ClientId client)
	{
		if (!IsValidClientID(client))
			return cpp::fail(Error::InvalidClientId);

		uint systemId;
		pub::Player::GetSystem(client, systemId);
		return GetSystemNickByID(systemId);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> LockAccountAccess(CAccount* acc, bool bKick)
	{
		const std::array<char, 1> jmp = {'\xEB'};
		const std::array<char, 1> jbe = {'\x76'};

		const auto accountId = GetAccountID(acc);
		if (accountId.has_error())
			return cpp::fail(accountId.error());

		st6::wstring flStr((ushort*)accountId.value().c_str());

		if (!bKick)
			WriteProcMem((void*)0x06D52A6A, jmp.data(), 1);

		Players.LockAccountAccess(flStr); // also kicks player on this account
		if (!bKick)
			WriteProcMem((void*)0x06D52A6A, jbe.data(), 1);

		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, Error> UnlockAccountAccess(CAccount* acc)
	{
		const auto accountId = GetAccountID(acc);
		if (accountId.has_error())
			return cpp::fail(accountId.error());

		st6::wstring flStr((ushort*)accountId.value().c_str());
		Players.UnlockAccountAccess(flStr);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GetItemsForSale(uint baseId, std::list<uint>& lstItems)
	{
		lstItems.clear();
		const std::array<char, 2> nop = {'\x90', '\x90'};
		const std::array<char, 2> jnz = {'\x75', '\x1D'};
		WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), nop.data(), 2); // patch, else we only get commodities


		std::array<int, 1024> arr;
		int size = 256;
		pub::Market::GetCommoditiesForSale(baseId, reinterpret_cast<uint* const>(arr.data()), &size);
		WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), jnz.data(), 2);

		for (int i = 0; (i < size); i++)
			lstItems.push_back(arr[i]);
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<IObjInspectImpl*, Error> GetInspect(ClientId client)
	{
		uint ship;
		pub::Player::GetShip(client, ship);
		uint iDunno;
		IObjInspectImpl* inspect;
		if (!GetShipInspect(ship, inspect, iDunno))
			return cpp::fail(Error::InvalidShip);
		else
			return inspect;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EngineState GetEngineState(ClientId client)
	{
		if (ClientInfo[client].bTradelane)
			return ES_TRADELANE;
		else if (ClientInfo[client].bCruiseActivated)
			return ES_CRUISE;
		else if (ClientInfo[client].bThrusterActivated)
			return ES_THRUSTER;
		else if (!ClientInfo[client].bEngineKilled)
			return ES_ENGINE;
		else
			return ES_KILLED;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EquipmentType GetEqType(Archetype::Equipment* eq)
	{
		uint iVFTableMine = (uint)hModCommon + ADDR_COMMON_VFTABLE_MINE;
		uint iVFTableCM = (uint)hModCommon + ADDR_COMMON_VFTABLE_CM;
		uint iVFTableGun = (uint)hModCommon + ADDR_COMMON_VFTABLE_GUN;
		uint iVFTableShieldGen = (uint)hModCommon + ADDR_COMMON_VFTABLE_SHIELDGEN;
		uint iVFTableThruster = (uint)hModCommon + ADDR_COMMON_VFTABLE_THRUSTER;
		uint iVFTableShieldBat = (uint)hModCommon + ADDR_COMMON_VFTABLE_SHIELDBAT;
		uint iVFTableNanoBot = (uint)hModCommon + ADDR_COMMON_VFTABLE_NANOBOT;
		uint iVFTableMunition = (uint)hModCommon + ADDR_COMMON_VFTABLE_MUNITION;
		uint iVFTableEngine = (uint)hModCommon + ADDR_COMMON_VFTABLE_ENGINE;
		uint iVFTableScanner = (uint)hModCommon + ADDR_COMMON_VFTABLE_SCANNER;
		uint iVFTableTractor = (uint)hModCommon + ADDR_COMMON_VFTABLE_TRACTOR;
		uint iVFTableLight = (uint)hModCommon + ADDR_COMMON_VFTABLE_LIGHT;

		uint iVFTable = *((uint*)eq);
		if (iVFTable == iVFTableGun)
		{
			Archetype::Gun const* gun = (Archetype::Gun*)eq;
			Archetype::Equipment* eqAmmo = Archetype::GetEquipment(gun->iProjectileArchId);
			int iMissile;
			memcpy(&iMissile, (char*)eqAmmo + 0x90, 4);
			uint iGunType = gun->get_hp_type_by_index(0);
			if (iGunType == 36)
				return ET_TORPEDO;
			else if (iGunType == 35)
				return ET_CD;
			else if (iMissile)
				return ET_MISSILE;
			else
				return ET_GUN;
		}
		else if (iVFTable == iVFTableCM)
			return ET_CM;
		else if (iVFTable == iVFTableShieldGen)
			return ET_SHIELDGEN;
		else if (iVFTable == iVFTableThruster)
			return ET_THRUSTER;
		else if (iVFTable == iVFTableShieldBat)
			return ET_SHIELDBAT;
		else if (iVFTable == iVFTableNanoBot)
			return ET_NANOBOT;
		else if (iVFTable == iVFTableMunition)
			return ET_MUNITION;
		else if (iVFTable == iVFTableMine)
			return ET_MINE;
		else if (iVFTable == iVFTableEngine)
			return ET_ENGINE;
		else if (iVFTable == iVFTableLight)
			return ET_LIGHT;
		else if (iVFTable == iVFTableScanner)
			return ET_SCANNER;
		else if (iVFTable == iVFTableTractor)
			return ET_TRACTOR;
		else
			return ET_OTHER;
	}

	uint ExtractClientID(const std::variant<uint, std::wstring>& player)
	{
		// If index is 0, we just use the client Id we are given
		if (!player.index())
		{
			const uint id = std::get<uint>(player);
			return IsValidClientID(id) ? id : -1;
		}

		// Otherwise we have a character name
		const std::wstring characterName = std::get<std::wstring>(player);

		// Check if its an id string
		if (characterName.rfind(L"id ", 0) != std::wstring::npos)
		{
			const auto val = ResolveID(characterName);
			if (val.has_error())
			{
				return -1;
			}

			return val.value();
		}

		const auto client = GetClientIdFromCharName(characterName);
		if (client.has_error())
		{
			return -1;
		}

		return client.value();
	}

	cpp::result<CAccount*, Error> ExtractAccount(const std::variant<uint, std::wstring>& player)
	{
		if (ClientId client = Hk::Client::ExtractClientID(player); client != -1)
			return Players.FindAccountFromClientID(client);

		if (!player.index())
			return nullptr;

		const auto acc = GetAccountByCharName(std::get<std::wstring>(player));
		if (acc.has_error())
		{
			return cpp::fail(acc.error());
		}

		return acc.value();
	}

	CAccount* GetAccountByClientID(ClientId client)
	{
		if (!Hk::Client::IsValidClientID(client))
			return nullptr;

		return Players.FindAccountFromClientID(client);
	}

	std::wstring GetAccountIdByClientID(ClientId client)
	{
		if (Hk::Client::IsValidClientID(client))
		{
			CAccount const* acc = GetAccountByClientID(client);
			if (acc && acc->wszAccId)
			{
				return acc->wszAccId;
			}
		}
		return L"";
	}

	cpp::result<void, Error> PlaySoundEffect(ClientId client, uint soundId) { 
		if (Hk::Client::IsValidClientID(client))
		{
			pub::Audio::PlaySoundEffect(client, soundId);
			return {};
		}
		return cpp::fail(Error::PlayerNotLoggedIn);
	}
}