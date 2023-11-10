#pragma once

#include "API/Types/BaseId.hpp"
#include "API/Types/ShipId.hpp"
#include "API/Types/SystemId.hpp"

class ShipId;

class ClientId
{
        const uint value = 0;

        Action<void, Error> AdjustCash(int amount);

        [[nodiscard]]
        bool IsValidClientId() const;

        static uint GetClientIdFromCharacterName(std::wstring_view name);

    public:
        explicit ClientId(const uint val) : value(val) {}
        explicit ClientId(const std::wstring_view str) : value(GetClientIdFromCharacterName(str)){};
        explicit ClientId() = default;

        explicit operator uint() const noexcept { return value; }
        bool operator==(const ClientId next) const { return value == next.value; }
        bool operator!() const;

        // Returns the underlying value of the ClientId, it is generally recommended to not use this.

        [[nodiscard]]
        uint GetValue() const
        {
            return value;
        }

        // Type Conversions

        Action<std::wstring, Error> GetCharacterName();
        // TODO: These eventually will be their own types as well
        Action<BaseId, Error> GetCurrentBase();
        Action<SystemId, Error> GetSystemId();
        Action<CAccount*, Error> GetAccount();
        Action<const Archetype::Ship*, Error> GetShipArch();
        Action<ShipId, Error> GetShipId();
        Action<std::list<EquipDesc>, Error> GetEquipment();
        Action<CPlayerGroup*, Error> GetGroup();
        Action<std::optional<std::wstring>, Error> GetAffiliation();
        Action<CShip*, Error> GetShip();
        Action<uint, Error> GetRank();
        Action<uint, Error> GetWealth();
        Action<int, Error> GetPvpKills();
        Action<uint, Error> GetCash();
        Action<std::wstring_view, Error> GetActiveCharacterName();

        // State Checks

        bool InSpace();
        bool IsDocked();
        bool InCharacterSelect();
        bool IsAlive();

        // Manipulation

        Action<void, Error> Kick(std::optional<std::wstring_view> reason = {}, std::optional<uint> delay);
        Action<void, Error> MessageAndKick(std::wstring_view reason, uint delay = 10);
        Action<void, Error> SaveChar();
        Action<void, Error> SetPvpKills(uint killAmount);
        Action<void, Error> AddCash(uint amount);
        Action<void, Error> RemoveCash(uint amount);
        Action<void, Error> Beam(std::variant<BaseId, std::wstring_view> base);
        Action<void, Error> Message(std::wstring message, MessageFormat format = MessageFormat::Normal, MessageColor color = MessageColor::Green);
        Action<void, Error> SetRep(std::variant<ushort, std::wstring_view> repGroup, float rep);
        Action<void, Error> Rename(std::wstring_view);
        void MarkObject(uint objId, int markStatus);
};
