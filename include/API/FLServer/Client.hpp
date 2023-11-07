#pragma once

namespace Hk::Client
{

    /**
     * Gets the account id in a wide string
     * @param fileName The account
     * @returns On success : wide string of account Id
     * @returns On fail : [CannotGetAccount] The function could not find the account.
     */
    DLL bool IsEncoded(const std::wstring& fileName);

    DLL std::wstring GetAccountDirName(const CAccount* acc);
    DLL Action<std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring_view>& player);
    DLL Action<std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring_view>& player, bool returnValueIfNoFile);
    DLL Action<std::wstring, Error> GetBaseNickByID(uint baseId);
    DLL Action<std::wstring, Error> GetPlayerSystem(ClientId client);
    DLL Action<std::wstring, Error> GetSystemNickByID(uint systemId);
    DLL std::vector<uint> getAllPlayersInSystem(SystemId system);
    DLL Action<void, Error> LockAccountAccess(CAccount* acc, bool kick);
    DLL Action<void, Error> UnlockAccountAccess(CAccount* acc);
    DLL Action<void, Error> PlaySoundEffect(ClientId client, uint soundId);
    DLL void GetItemsForSale(uint baseId, std::list<uint>& items);
    DLL Action<IObjInspectImpl*, Error> GetInspect(ClientId client);
    DLL EngineState GetEngineState(ClientId client);
    DLL EquipmentType GetEqType(Archetype::Equipment* eq);
} // namespace Hk::Client
