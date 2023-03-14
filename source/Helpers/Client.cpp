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
					return cpp::fail(Error::PlayerNotLoggedIn);
				}

				return cpp::fail(resolvedId.error());
			}
		}

		return resolvedId.value();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const uint, Error> GetClientIdFromAccount(const CAccount* acc)
	{
		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			if (playerDb->account == acc)
			{
				return playerDb->onlineId;
			}
		}

		return cpp::fail(Error::PlayerNotLoggedIn);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<CAccount*, Error> GetAccountByCharName(const std::wstring& character)
	{
		st6::wstring fr((ushort*)character.c_str());
		CAccount* acc = Players.FindAccountFromCharacterName(fr);

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
		if (acc && acc->accId)
			return acc->accId;

		return cpp::fail(Error::CannotGetAccount);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool IsEncoded(const std::string& fileName)
	{
		bool bRet = false;
		FILE* f;
		fopen_s(&f, fileName.c_str(), "r");
		if (!f)
			return false;

		const char Magic[] = "FLS1";
		char File[sizeof(Magic)] = "";
		fread(File, 1, sizeof(Magic), f);
		if (!strncmp(Magic, File, sizeof(Magic) - 1))
			bRet = true;
		fclose(f);

		return bRet;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool IsInCharSelectMenu(const uint& player)
	{
		ClientId client = ExtractClientID(player);
		if (client == UINT_MAX)
			return false;

		uint base = 0;
		uint system = 0;
		pub::Player::GetBase(client, base);
		pub::Player::GetSystem(client, system);
		if (!base && !system)
			return true;
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool IsValidClientID(ClientId client)
	{
		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			if (playerDb->onlineId == client)
				return true;
		}

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, Error> GetCharacterNameByID(ClientId& client)
	{
		if (!IsValidClientID(client) || IsInCharSelectMenu(client))
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

	cpp::result<ClientId, Error> ResolveShortCut(const std::wstring& Shortcut)
	{
		std::wstring ShortcutLower = ToLower(Shortcut);
		if (ShortcutLower.find(L"sc ") != 0)
			return cpp::fail(Error::InvalidShortcutString);

		ShortcutLower = ShortcutLower.substr(3);

		uint clientFound = UINT_MAX;
		PlayerData* playerDb = nullptr;
		while ((playerDb = Players.traverse_active(playerDb)))
		{
			const auto characterName = GetCharacterNameByID(playerDb->onlineId);
			if (characterName.has_error())
				continue;

			if (ToLower(characterName.value()).find(ShortcutLower) != -1)
			{
				if (clientFound == UINT_MAX)
					clientFound = playerDb->onlineId;
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
		if (const auto foundClient = std::ranges::find_if(ClientInfo, [ship](const CLIENT_INFO& ci) { return ci.ship == ship; });
			foundClient != ClientInfo.end())
			return std::ranges::distance(ClientInfo.begin(), foundClient);

		return cpp::fail(Error::InvalidShip);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::wstring GetAccountDirName(const CAccount* acc)
	{
		const auto GetFLName = reinterpret_cast<_GetFLName>(reinterpret_cast<char*>(server) + 0x66370);

		char Dir[1024] = "";
		GetFLName(Dir, acc->accId);
		return stows(Dir);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring>& player, bool returnValueIfNoFile)
	{
		static _GetFLName GetFLName = nullptr;
		if (!GetFLName)
			GetFLName = (_GetFLName)((char*)server + 0x66370);

		std::string buffer;
		buffer.reserve(1024);

		if (ClientId client = ExtractClientID(player); client != UINT_MAX)
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

		st6::wstring fr((ushort*)accountId.value().c_str());

		if (!bKick)
			WriteProcMem((void*)0x06D52A6A, jmp.data(), 1);

		Players.LockAccountAccess(fr); // also kicks player on this account
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

		st6::wstring fr((ushort*)accountId.value().c_str());
		Players.UnlockAccountAccess(fr);
		return {};
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GetItemsForSale(uint baseId, std::list<uint>& Items)
	{
		Items.clear();
		const std::array<char, 2> nop = {'\x90', '\x90'};
		const std::array<char, 2> jnz = {'\x75', '\x1D'};
		WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), nop.data(), 2); // patch, else we only get commodities

		std::array<int, 1024> arr;
		int size = 256;
		pub::Market::GetCommoditiesForSale(baseId, reinterpret_cast<uint* const>(arr.data()), &size);
		WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), jnz.data(), 2);

		for (int i = 0; (i < size); i++)
			Items.push_back(arr[i]);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<IObjInspectImpl*, Error> GetInspect(ClientId client)
	{
		uint ship;
		pub::Player::GetShip(client, ship);
		uint dunno;
		IObjInspectImpl* inspect;
		if (!GetShipInspect(ship, inspect, dunno))
			return cpp::fail(Error::InvalidShip);
		return inspect;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EngineState GetEngineState(ClientId client)
	{
		if (ClientInfo[client].tradelane)
			return ES_TRADELANE;
		if (ClientInfo[client].cruiseActivated)
			return ES_CRUISE;
		if (ClientInfo[client].thrusterActivated)
			return ES_THRUSTER;
		if (!ClientInfo[client].engineKilled)
			return ES_ENGINE;
		return ES_KILLED;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EquipmentType GetEqType(Archetype::Equipment* eq)
	{
		const uint iVFTableMine = (uint)common + ADDR_COMMON_VFTABLE_MINE;
		const uint iVFTableCM = (uint)common + ADDR_COMMON_VFTABLE_CM;
		const uint iVFTableGun = (uint)common + ADDR_COMMON_VFTABLE_GUN;
		const uint iVFTableShieldGen = (uint)common + ADDR_COMMON_VFTABLE_SHIELDGEN;
		const uint iVFTableThruster = (uint)common + ADDR_COMMON_VFTABLE_THRUSTER;
		const uint iVFTableShieldBat = (uint)common + ADDR_COMMON_VFTABLE_SHIELDBAT;
		const uint iVFTableNanoBot = (uint)common + ADDR_COMMON_VFTABLE_NANOBOT;
		const uint iVFTableMunition = (uint)common + ADDR_COMMON_VFTABLE_MUNITION;
		const uint iVFTableEngine = (uint)common + ADDR_COMMON_VFTABLE_ENGINE;
		const uint iVFTableScanner = (uint)common + ADDR_COMMON_VFTABLE_SCANNER;
		const uint iVFTableTractor = (uint)common + ADDR_COMMON_VFTABLE_TRACTOR;
		const uint iVFTableLight = (uint)common + ADDR_COMMON_VFTABLE_LIGHT;

		const uint iVFTable = *((uint*)eq);
		if (iVFTable == iVFTableGun)
		{
			const Archetype::Gun* gun = static_cast<Archetype::Gun*>(eq);
			Archetype::Equipment* eqAmmo = Archetype::GetEquipment(gun->projectileArchId);
			int iMissile;
			memcpy(&iMissile, (char*)eqAmmo + 0x90, 4);
			const uint iGunType = gun->get_hp_type_by_index(0);
			if (iGunType == 36)
				return ET_TORPEDO;
			if (iGunType == 35)
				return ET_CD;
			if (iMissile)
				return ET_MISSILE;
			return ET_GUN;
		}
		if (iVFTable == iVFTableCM)
			return ET_CM;
		if (iVFTable == iVFTableShieldGen)
			return ET_SHIELDGEN;
		if (iVFTable == iVFTableThruster)
			return ET_THRUSTER;
		if (iVFTable == iVFTableShieldBat)
			return ET_SHIELDBAT;
		if (iVFTable == iVFTableNanoBot)
			return ET_NANOBOT;
		if (iVFTable == iVFTableMunition)
			return ET_MUNITION;
		if (iVFTable == iVFTableMine)
			return ET_MINE;
		if (iVFTable == iVFTableEngine)
			return ET_ENGINE;
		if (iVFTable == iVFTableLight)
			return ET_LIGHT;
		if (iVFTable == iVFTableScanner)
			return ET_SCANNER;
		if (iVFTable == iVFTableTractor)
			return ET_TRACTOR;
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
		if (ClientId client = ExtractClientID(player); client != UINT_MAX)
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
		if (!IsValidClientID(client))
			return nullptr;

		return Players.FindAccountFromClientID(client);
	}

	std::wstring GetAccountIdByClientID(ClientId client)
	{
		if (IsValidClientID(client))
		{
			const CAccount* acc = GetAccountByClientID(client);
			if (acc && acc->accId)
			{
				return acc->accId;
			}
		}
		return L"";
	}

	cpp::result<void, Error> PlaySoundEffect(ClientId client, uint soundId)
	{
		if (IsValidClientID(client))
		{
			pub::Audio::PlaySoundEffect(client, soundId);
			return {};
		}
		return cpp::fail(Error::PlayerNotLoggedIn);
	}

	std::vector<uint> getAllPlayersInSystem(SystemId system)
	{
		PlayerData* playerData = nullptr;
		std::vector<uint> playersInSystem;
		while ((playerData = Players.traverse_active(playerData)))
		{
			if (playerData->systemId == system)
				playersInSystem.push_back(playerData->onlineId);
		}
		return playersInSystem;
	}
}
