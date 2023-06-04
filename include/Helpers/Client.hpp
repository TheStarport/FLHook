#pragma once

namespace Hk::Client
{
	/**
	 * Gets the current client id of the account.
	 * @param acc CAccount
	 * @returns On success : Client id of the active user of the account.
	 * @returns On fail : [PlayerNotLoggedIn] The function could not find a client id associated with the account id.
	 */
	DLL cpp::result<const uint, Error> GetClientIdFromAccount(const CAccount* acc);

	DLL std::wstring GetAccountIdByClientID(ClientId client);

	DLL CAccount* GetAccountByClientID(ClientId client);

	/**
	 * Gets the current client id of the character.
	 * @param character Wide string of the character name
	 * @returns On success : current client Id associated with that character name
	 * @returns On fail : [CharacterDoesNotExist] The function could not find a client id associated with this character name.
	 */
	DLL cpp::result<const uint, Error> GetClientIdFromCharName(std::wstring_view character);

	/**
	 * Checks to see if the client Id is valid
	 * @param client Client Id
	 * @returns If Valid: true
	 * @returns If Not Valid: false
	 */
	DLL bool IsValidClientID(ClientId client);

	/**
	 * Gets the account of the character
	 * @param character Wide string of the character name
	 * @returns On success : the account Id for that character
	 * @returns On fail : [CharacterDoesNotExist] The function could not find the account id associated with this character name.
	 */
	DLL cpp::result<CAccount*, Error> GetAccountByCharName(std::wstring_view character);

	/**
	 * Gets the account id in a wide string
	 * @param acc The account
	 * @returns On success : wide string of account Id
	 * @returns On fail : [CannotGetAccount] The function could not find the account.
	 */
	DLL cpp::result<const std::wstring, Error> GetAccountID(CAccount* acc);

	/**
	 * Gets the account id in a wide string
	 * @param fileName The account
	 * @returns On success : wide string of account Id
	 * @returns On fail : [CannotGetAccount] The function could not find the account.
	 */
	DLL bool IsEncoded(const std::wstring& fileName);

	DLL bool IsInCharSelectMenu(const uint& player);

	DLL cpp::result<const std::wstring, Error> GetCharacterNameByID(ClientId& client);
	DLL cpp::result<const uint, Error> ResolveID(const std::wstring& player);
	DLL cpp::result<ClientId, Error> ResolveShortCut(const std::wstring& shortcut);
	DLL cpp::result<ClientId, Error> GetClientIdByShip(ShipId ship);
	DLL std::wstring GetAccountDirName(const CAccount* acc);
	DLL cpp::result<const std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring_view>& player);
	DLL cpp::result<const std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring_view>& player, bool returnValueIfNoFile);
	DLL cpp::result<const std::wstring, Error> GetBaseNickByID(uint baseId);
	DLL cpp::result<const std::wstring, Error> GetPlayerSystem(ClientId client);
	DLL cpp::result<const std::wstring, Error> GetSystemNickByID(uint systemId);
	DLL std::vector<uint> getAllPlayersInSystem(SystemId system);
	DLL cpp::result<void, Error> LockAccountAccess(CAccount* acc, bool kick);
	DLL cpp::result<void, Error> UnlockAccountAccess(CAccount* acc);
	DLL cpp::result<void, Error> PlaySoundEffect(ClientId client, uint soundId);
	DLL void GetItemsForSale(uint baseId, std::list<uint>& items);
	DLL cpp::result<IObjInspectImpl*, Error> GetInspect(ClientId client);
	DLL EngineState GetEngineState(ClientId client);
	DLL EquipmentType GetEqType(Archetype::Equipment* eq);
}