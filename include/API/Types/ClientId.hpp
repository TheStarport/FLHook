#pragma once

#include "API/Types/BaseId.hpp"
#include "API/Types/ShipId.hpp"
#include "API/Types/SystemId.hpp"

class ShipId;
class ClientData;
class GroupId;

class ClientId
{
        uint value = 0;

        [[nodiscard]]
        Action<void, Error> AdjustCash(int amount) const;

        [[nodiscard]]
        bool IsValidClientId() const;

        [[nodiscard]]
        static uint GetClientIdFromCharacterName(std::wstring_view name);

    public:
        explicit ClientId(const uint val) : value(val) {}
        explicit ClientId(const SpecialChatIds id) { value = static_cast<const uint>(id); }
        explicit ClientId(const std::wstring_view str) : value(GetClientIdFromCharacterName(str)){};
        explicit ClientId() = default;

        explicit operator uint() const noexcept { return value; }
        bool operator==(const ClientId next) const { return value == next.value; }
        explicit operator bool() const;

        // Returns the underlying value of the ClientId, it is generally recommended to not use this.

        [[nodiscard]]
        uint GetValue() const
        {
            return value;
        }

        // Type Conversions

        [[nodiscard]]
        Action<std::wstring_view, Error> GetCharacterName() const;
        [[nodiscard]]
        Action<BaseId, Error> GetCurrentBase() const;
        [[nodiscard]]
        Action<SystemId, Error> GetSystemId() const;
        [[nodiscard]]
        Action<CAccount *, Error> GetAccount() const;
        [[nodiscard]]
        Action<const Archetype::Ship *, Error> GetShipArch();
        [[nodiscard]]
        Action<ShipId, Error> GetShipId() const;
        [[nodiscard]]
        Action<CPlayerGroup *, Error> GetGroup();
        [[nodiscard]]
        Action<RepId, Error> GetReputation() const;
        [[nodiscard]]
        Action<CShip *, Error> GetShip() const;
        [[nodiscard]]
        Action<uint, Error> GetRank() const;
        [[nodiscard]]
        Action<uint, Error> GetWealth() const;
        [[nodiscard]]
        Action<int, Error> GetPvpKills() const;
        [[nodiscard]]
        Action<uint, Error> GetCash() const;
        [[nodiscard]]
        Action<std::list<CargoInfo>, Error> EnumCargo(int &remainingHoldSize) const;
        [[nodiscard]]
        ClientData &GetData() const;

        // State Checks

        [[nodiscard]]
        bool InSpace() const;
        [[nodiscard]]
        bool IsDocked() const;
        [[nodiscard]]
        bool InCharacterSelect() const;
        [[nodiscard]]
        bool IsAlive() const;

        // Manipulation

        Action<void, Error> AddToGroup(GroupId group) const;
        Action<void, Error> Kick(const std::optional<std::wstring_view> &reason = {}, std::optional<uint> delay = {}) const;
        Action<void, Error> MessageAndKick(std::wstring_view reason, uint delay = 10) const;
        Action<void, Error> SaveChar() const;
        Action<void, Error> SetPvpKills(uint killAmount) const;
        Action<void, Error> AddCash(uint amount) const;
        Action<void, Error> RemoveCash(uint amount) const;
        Action<void, Error> Beam(std::variant<BaseId, std::wstring_view> base) const;
        Action<void, Error> Rename(std::wstring_view) const;
        void MarkObject(uint objId, int markStatus) const;

        // Chat

        Action<void, Error> Message(std::wstring_view message, MessageFormat format = MessageFormat::Normal, MessageColor color = MessageColor::Default) const;
        Action<void, Error> MessageLocal(std::wstring_view message, float range = 10'000.0f, MessageFormat format = MessageFormat::Normal,
                                         MessageColor color = MessageColor::Default) const;
        Action<void, Error> MessageFrom(ClientId destinationClient, std::wstring message) const;
};
