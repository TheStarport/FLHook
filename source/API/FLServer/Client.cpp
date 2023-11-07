#include "PCH.hpp"

#include "API/FLServer/Client.hpp"

namespace Hk::Client
{
    std::wstring GetAccountDirName(const CAccount* acc)
    {
        const auto GetFLName = reinterpret_cast<_GetFLName>(reinterpret_cast<char*>(server) + 0x66370);

        char Dir[1024] = "";
        GetFLName(Dir, acc->accId);
        return StringUtils::stows(Dir);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring_view>& player, bool returnValueIfNoFile)
    {
        static _GetFLName GetFLName = nullptr;
        if (!GetFLName)
        {
            GetFLName = (_GetFLName)((char*)server + 0x66370);
        }

        std::wstring buffer;
        buffer.reserve(1024);

        if (ClientId client = ExtractClientID(player); client != UINT_MAX)
        {
            if (const auto character = GetCharacterNameByID(client).Raw(); character.has_error())
            {
                return { cpp::fail(character.error()) };
            }

            GetFLName(reinterpret_cast<char*>(buffer.data()), reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client)));
        }
        else if ((player.index() && GetAccountByCharName(std::get<std::wstring_view>(player)).Raw()) || returnValueIfNoFile)
        {
            GetFLName(reinterpret_cast<char*>(buffer.data()), std::get<std::wstring_view>(player).data());
        }
        else
        {
            return { cpp::fail(Error::InvalidClientId) };
        }

        return { buffer };
    }

    Action<std::wstring, Error> GetCharFileName(const std::variant<uint, std::wstring_view>& player) { return GetCharFileName(player, false); }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<std::wstring, Error> GetBaseNickByID(uint baseId)
    {
        std::wstring base;
        base.resize(1024);
        pub::GetBaseNickname(reinterpret_cast<char*>(base.data()), base.capacity(), baseId);
        base.resize(1024); // Without calling another core function will result in length not being updated

        if (base.empty())
        {
            return { cpp::fail(Error::InvalidBase) };
        }

        return { base };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<std::wstring, Error> GetSystemNickByID(uint systemId)
    {
        std::wstring system;
        system.resize(1024);
        pub::GetSystemNickname(reinterpret_cast<char*>(system.data()), system.capacity(), systemId);
        system.resize(1024); // Without calling another core function will result in length not being updated

        if (system.empty())
        {
            return { cpp::fail(Error::InvalidSystem) };
        }

        return { system };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<std::wstring, Error> GetPlayerSystem(ClientId client)
    {
        if (!IsValidClientID(client))
        {
            return { cpp::fail(Error::InvalidClientId) };
        }

        uint systemId;
        pub::Player::GetSystem(client, systemId);
        return GetSystemNickByID(systemId);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void, Error> LockAccountAccess(CAccount* acc, bool kick)
    {
        const std::array<char, 1> jmp = { '\xEB' };
        const std::array<char, 1> jbe = { '\x76' };

        const auto accountId = GetAccountID(acc).Raw();
        if (accountId.has_error())
        {
            return { cpp::fail(accountId.error()) };
        }

        st6::wstring fr((ushort*)accountId.value().c_str());

        if (!kick)
        {
            MemUtils::WriteProcMem((void*)0x06D52A6A, jmp.data(), 1);
        }

        Players.LockAccountAccess(fr); // also kicks player on this account
        if (!kick)
        {
            MemUtils::WriteProcMem((void*)0x06D52A6A, jbe.data(), 1);
        }

        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void, Error> UnlockAccountAccess(CAccount* acc)
    {
        const auto accountId = GetAccountID(acc).Raw();
        if (accountId.has_error())
        {
            return { cpp::fail(accountId.error()) };
        }

        st6::wstring fr((ushort*)accountId.value().c_str());
        Players.UnlockAccountAccess(fr);
        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void GetItemsForSale(uint baseId, std::list<uint>& items)
    {
        items.clear();
        const std::array<char, 2> nop = { '\x90', '\x90' };
        const std::array<char, 2> jnz = { '\x75', '\x1D' };
        MemUtils::WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), nop.data(), 2); // patch, else we only get commodities

        std::array<int, 1024> arr;
        int size = 256;
        pub::Market::GetCommoditiesForSale(baseId, reinterpret_cast<uint* const>(arr.data()), &size);
        MemUtils::WriteProcMem(SRV_ADDR(ADDR_SRV_GETCOMMODITIES), jnz.data(), 2);

        for (int i = 0; i < size; i++)
        {
            items.push_back(arr[i]);
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<IObjInspectImpl*, Error> GetInspect(ClientId client)
    {
        uint ship;
        pub::Player::GetShip(client, ship);
        uint dunno;
        IObjInspectImpl* inspect;
        if (!GetShipInspect(ship, inspect, dunno))
        {
            return { cpp::fail(Error::InvalidShip) };
        }
        return { inspect };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    EngineState GetEngineState(ClientId client)
    {
        if (ClientInfo::At(client).tradelane)
        {
            return ES_TRADELANE;
        }
        if (ClientInfo::At(client).cruiseActivated)
        {
            return ES_CRUISE;
        }
        if (ClientInfo::At(client).thrusterActivated)
        {
            return ES_THRUSTER;
        }
        if (!ClientInfo::At(client).engineKilled)
        {
            return ES_ENGINE;
        }
        return ES_KILLED;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    EquipmentType GetEqType(Archetype::Equipment* eq)
    {
        const uint VFTableMine = (uint)common + ADDR_COMMON_VFTABLE_MINE;
        const uint VFTableCM = (uint)common + ADDR_COMMON_VFTABLE_CM;
        const uint VFTableGun = (uint)common + ADDR_COMMON_VFTABLE_GUN;
        const uint VFTableShieldGen = (uint)common + ADDR_COMMON_VFTABLE_SHIELDGEN;
        const uint VFTableThruster = (uint)common + ADDR_COMMON_VFTABLE_THRUSTER;
        const uint VFTableShieldBat = (uint)common + ADDR_COMMON_VFTABLE_SHIELDBAT;
        const uint VFTableNanoBot = (uint)common + ADDR_COMMON_VFTABLE_NANOBOT;
        const uint VFTableMunition = (uint)common + ADDR_COMMON_VFTABLE_MUNITION;
        const uint VFTableEngine = (uint)common + ADDR_COMMON_VFTABLE_ENGINE;
        const uint VFTableScanner = (uint)common + ADDR_COMMON_VFTABLE_SCANNER;
        const uint VFTableTractor = (uint)common + ADDR_COMMON_VFTABLE_TRACTOR;
        const uint VFTableLight = (uint)common + ADDR_COMMON_VFTABLE_LIGHT;

        const uint VFTable = *(uint*)eq;
        if (VFTable == VFTableGun)
        {
            const Archetype::Gun* gun = static_cast<Archetype::Gun*>(eq);
            Archetype::Equipment* eqAmmo = Archetype::GetEquipment(gun->projectileArchId);
            int missile;
            memcpy(&missile, (char*)eqAmmo + 0x90, 4);
            const uint gunType = gun->get_hp_type_by_index(0);
            if (gunType == 36)
            {
                return ET_TORPEDO;
            }
            if (gunType == 35)
            {
                return ET_CD;
            }
            if (missile)
            {
                return ET_MISSILE;
            }
            return ET_GUN;
        }
        if (VFTable == VFTableCM)
        {
            return ET_CM;
        }
        if (VFTable == VFTableShieldGen)
        {
            return ET_SHIELDGEN;
        }
        if (VFTable == VFTableThruster)
        {
            return ET_THRUSTER;
        }
        if (VFTable == VFTableShieldBat)
        {
            return ET_SHIELDBAT;
        }
        if (VFTable == VFTableNanoBot)
        {
            return ET_NANOBOT;
        }
        if (VFTable == VFTableMunition)
        {
            return ET_MUNITION;
        }
        if (VFTable == VFTableMine)
        {
            return ET_MINE;
        }
        if (VFTable == VFTableEngine)
        {
            return ET_ENGINE;
        }
        if (VFTable == VFTableLight)
        {
            return ET_LIGHT;
        }
        if (VFTable == VFTableScanner)
        {
            return ET_SCANNER;
        }
        if (VFTable == VFTableTractor)
        {
            return ET_TRACTOR;
        }
        return ET_OTHER;
    }

    cpp::result<CAccount*, Error> ExtractAccount(const std::variant<uint, std::wstring_view>& player)
    {
        if (ClientId client = ExtractClientID(player); client != UINT_MAX)
        {
            return Players.FindAccountFromClientID(client);
        }

        if (!player.index())
        {
            return nullptr;
        }

        const auto acc = GetAccountByCharName(std::get<std::wstring_view>(player)).Raw();
        if (acc.has_error())
        {
            return { cpp::fail(acc.error()) };
        }

        return acc.value();
    }

    CAccount* GetAccountByClientID(ClientId client)
    {
        if (!IsValidClientID(client))
        {
            return nullptr;
        }

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

    Action<void, Error> PlaySoundEffect(ClientId client, uint soundId)
    {
        if (IsValidClientID(client))
        {
            pub::Audio::PlaySoundEffect(client, soundId);
            return { {} };
        }
        return { cpp::fail(Error::PlayerNotLoggedIn) };
    }

    std::vector<uint> getAllPlayersInSystem(SystemId system)
    {
        PlayerData* playerData = nullptr;
        std::vector<uint> playersInSystem;
        while ((playerData = Players.traverse_active(playerData)))
        {
            if (playerData->systemId == system)
            {
                playersInSystem.push_back(playerData->onlineId);
            }
        }
        return playersInSystem;
    }
} // namespace Hk::Client
