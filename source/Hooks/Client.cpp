#include "Global.hpp"

namespace Hk::Client
{
	cpp::result<const uint, HkError> HkGetClientID(const std::wstring_view& character)
	{
		bIdString = character.find(L"id ") == 0;

		HkError hkErr = HkResolveId(character, clientId);
		if (hkErr != HKE_OK)
		{
			if (hkErr == InvalidIdString)
			{
				hkErr = HkResolveShortCut(character, clientId);
				if ((hkErr == AmbiguousShortcut) || (hkErr == HkError::NoMatchingPlayer))
					return hkErr;
				if (hkErr == HkError::InvalidShortcutString)
				{
					clientId = HkGetClientIdFromCharname(character);
					if (clientId != (uint)-1)
						return HKE_OK;
					else
						return PlayerNotLoggedIn;
				}
			}
		}
		return hkErr;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const uint, HkError> HkGetClientIdFromAccount(CAccount* acc)
	{
		struct PlayerData* playerDb = nullptr;
		while (playerDb = Players.traverse_active(playerDb))
		{
			if (playerDb->Account == acc)
			{
				return playerDb->iOnlineID;
			}
		}

		return cpp::fail(HkError::PlayerNotLoggedIn);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<CAccount*, HkError> HkGetAccountByCharName(std::wstring& character)
	{
		st6::wstring flStr((ushort*)character.c_str());
		CAccount* acc = Players.FindAccountFromCharacterName(flStr);

		if (!acc)
			return cpp::fail(HkError::CharacterDoesNotExist);

		return acc;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const uint, HkError> HkGetClientIdFromCharName(const std::wstring& character)
	{
		const auto acc = HkGetAccountByCharName(character);
		if (acc.has_error())
			return cpp::fail(acc.error());

		const auto clientId = HkGetClientIdFromAccount(acc.value());
		if (clientId.has_error())
			return cpp::fail(clientId.error());

		const auto newCharacter = HkGetCharacterNameById(clientId.value());
		if (newCharacter.has_error())
			return cpp::fail(newCharacter.error());


		if (ToLower(newCharacter.value()).compare(ToLower(character)) != 0)
			return cpp::fail(HkError::CharacterDoesNotExist);

		return clientId;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, HkError> HkGetAccountID(CAccount* acc)
	{
		if (acc && acc->wszAccID)
			return acc->wszAccID;

		return cpp::fail(HkError::CannotGetAccount);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool HkIsEncoded(const std::string& scFilename)
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

	bool HkIsInCharSelectMenu(const std::variant<uint, std::wstring>& player)
	{
		const uint clientId = HkExtractClientId(player);
		if (clientId == -1)
			return false;

		uint iBase = 0;
		uint iSystem = 0;
		pub::Player::GetBase(clientId, iBase);
		pub::Player::GetSystem(clientId, iSystem);
		if (!iBase && !iSystem)
			return true;
		else
			return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool HkIsValidClientID(uint clientId)
	{
		struct PlayerData* playerDb = nullptr;
		while (playerDb = Players.traverse_active(playerDb))
		{
			if (playerDb->iOnlineID == clientId)
				return true;
		}

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, HkError> HkGetCharacterNameById(const uint& clientId)
	{
		if (!HkIsValidClientID(clientId))
			return cpp::fail(HkError::InvalidClientId);

		return reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const uint, HkError> HkResolveId(const std::wstring& character)
	{
		if (const std::wstring characterLower = ToLower(character); characterLower.find(L"id ") == 0)
		{
			uint iID = 0;
			swscanf_s(characterLower.c_str(), L"id %u", &iID);
			if (!HkIsValidClientID(iID))
				return cpp::fail(HkError::InvalidClientId);

			return iID;
		}

		return cpp::fail(HkError::InvalidIdString);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const uint, HkError> HkResolveShortCut(const std::wstring& wscShortcut)
	{
		std::wstring wscShortcutLower = ToLower(wscShortcut);
		if (wscShortcutLower.find(L"sc ") != 0)
			return cpp::fail(HkError::InvalidShortcutString);

		wscShortcutLower = wscShortcutLower.substr(3);

		uint clientIdFound = UINT_MAX;
		struct PlayerData* playerDb = nullptr;
		while (playerDb = Players.traverse_active(playerDb))
		{
			const auto characterName = HkGetCharacterNameById(playerDb->iOnlineID);
			if (characterName.has_error())
				continue;

			if (ToLower(characterName.value()).find(wscShortcutLower) != -1)
			{
				if (clientIdFound == UINT_MAX)
					clientIdFound = playerDb->iOnlineID;
				else
					return cpp::fail(HkError::AmbiguousShortcut);
			}
		}

		if (clientIdFound == UINT_MAX)
			return cpp::fail(HkError::NoMatchingPlayer);

		return clientIdFound;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const uint, HkError> HkGetClientIDByShip(const uint iShip)
	{
		for (uint i = 0; i <= MaxClientId; i++)
		{
			if (ClientInfo[i].iShip == iShip || ClientInfo[i].iShipOld == iShip)
				return i;
		}

		return cpp::fail(HkError::InvalidShip);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::wstring HkGetAccountDirName(CAccount* acc)
	{
		const auto GetFLName = reinterpret_cast<_GetFLName>(reinterpret_cast<char*>(hModServer) + 0x66370);

		char szDir[1024] = "";
		GetFLName(szDir, acc->wszAccID);
		return stows(szDir);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, HkError> HkGetAccountDirName(const std::variant<uint, std::wstring>& player, std::wstring& wscDir)
	{
		CAccount* acc = HkExtractAccount(player);
		return HkGetAccountDirName(acc, wscDir);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, HkError> HkGetCharFileName(const std::variant<uint, std::wstring>& player, std::wstring& wscFilename)
	{
		static _GetFLName GetFLName = nullptr;
		if (!GetFLName)
			GetFLName = (_GetFLName)((char*)hModServer + 0x66370);

		std::string buffer;
		buffer.reserve(1024);

		if (const uint clientId = HkExtractClientId(player); clientId != UINT_MAX)
		{
			const auto character = HkGetCharacterNameById(clientId);
			if (character.has_error())
				return cpp::fail(character.error());

			GetFLName(buffer.data(), reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId)));
		}
		else if (player.index() && HkGetAccountByCharName(std::get<std::wstring>(player)))
		{
			GetFLName(buffer.data(), std::get<std::wstring>(player).c_str());
		}
		else
		{
			return cpp::fail(HkError::InvalidClientId);
		}

		return wscFilename = stows(buffer);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, HkError> HkGetBaseNickByID(uint baseId)
	{
		std::string base;
		base.reserve(1024);
		pub::GetBaseNickname(base.data(), base.capacity(), baseId);

		if (base.empty())
			return cpp::fail(HkError::InvalidBase);

		return stows(base);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, HkError> HkGetSystemNickByID(uint systemId)
	{
		std::string system;
		system.reserve(1024);
		pub::GetSystemNickname(system.data(), system.capacity(), systemId);

		if (system.empty())
			return cpp::fail(HkError::InvalidSystem);
		
		return stows(system);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<const std::wstring, HkError> HkGetPlayerSystem(uint clientId)
	{
		if (!HkIsValidClientID(clientId))
			return cpp::fail(HkError::InvalidClientId);

		uint systemId;
		pub::Player::GetSystem(clientId, systemId);
		return HkGetSystemNickByID(systemId);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, HkError> HkLockAccountAccess(CAccount* acc, bool bKick)
	{
		const std::array<char, 1> jmp = {'\xEB'};
		const std::array<char, 1> jbe = {'\x76'};

		const auto accountId = HkGetAccountID(acc);
		if (accountId.has_error())
			return cpp::fail(accountId.error());

		st6::wstring flStr((ushort*)accountId.value().c_str());

		if (!bKick)
			WriteProcMem((void*)0x06D52A6A, jmp.data(), 1);

		Players.LockAccountAccess(flStr); // also kicks player on this account
		if (!bKick)
			WriteProcMem((void*)0x06D52A6A, jbe.data(), 1);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cpp::result<void, HkError> HkUnlockAccountAccess(CAccount* acc)
	{
		const auto accountId = HkGetAccountID(acc);
		if (accountId.has_error())
			return cpp::fail(accountId.error());

		st6::wstring flStr((ushort*)accountId.value().c_str());
		Players.UnlockAccountAccess(flStr);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void HkGetItemsForSale(uint baseId, std::list<uint>& lstItems)
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

	cpp::result<IObjInspectImpl*, HkError> HkGetInspect(uint clientId)
	{
		uint iShip;
		pub::Player::GetShip(clientId, iShip);
		uint iDunno;
		IObjInspectImpl* inspect;
		if (!GetShipInspect(iShip, inspect, iDunno))
			return cpp::fail(HkError::InvalidShip);
		else
			return inspect;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EngineState HkGetEngineState(uint clientId)
	{
		if (ClientInfo[clientId].bTradelane)
			return ES_TRADELANE;
		else if (ClientInfo[clientId].bCruiseActivated)
			return ES_CRUISE;
		else if (ClientInfo[clientId].bThrusterActivated)
			return ES_THRUSTER;
		else if (!ClientInfo[clientId].bEngineKilled)
			return ES_ENGINE;
		else
			return ES_KILLED;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	EquipmentType HkGetEqType(Archetype::Equipment* eq)
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
			Archetype::Gun* gun = (Archetype::Gun*)eq;
			Archetype::Equipment* eqAmmo = Archetype::GetEquipment(gun->iProjectileArchID);
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

	cpp::result<const uint, HkError> HkGetSystemByNickname(std::variant<std::string, std::wstring> nickname)
	{
		uint system = 0;
		const std::string nick = nickname.index() == 0 ? std::get<std::string>(nickname) : wstos(std::get<std::wstring>(nickname));
		pub::GetSystemID(system, nick.c_str());
		if (!system)
			return cpp::fail(HkError::InvalidSystem);

		return system;
	}

	CShip* HkCShipFromShipDestroyed(const DWORD** ecx)
	{
		return reinterpret_cast<CShip*>((*ecx)[4]); // NOLINT(performance-no-int-to-ptr)
	}

	uint HkExtractClientId(const std::variant<uint, std::wstring>& player)
	{
		// If index is 0, we just use the client ID we are given
		if (!player.index())
		{
			const uint id = std::get<uint>(player);
			return HkIsValidClientID(id) ? id : -1;
		}

		// Otherwise we have a character name
		const std::wstring characterName = std::get<std::wstring>(player);

		// Check if its an id string
		if (characterName.rfind(L"id ", 0) != std::wstring::npos)
		{
			uint id;
			const HkError err = HkResolveId(characterName, id);
			return err == HKE_OK ? id : -1;
		}

		return HkGetClientIdFromCharname(characterName);
	}

	CAccount* HkExtractAccount(const std::variant<uint, std::wstring>& player)
	{
		uint clientId = HkExtractClientId(player);
		if (clientId != -1)
			return Players.FindAccountFromClientID(clientId);

		if (!player.index())
			return nullptr;

		return HkGetAccountByCharName(std::get<std::wstring>(player));
	}
}