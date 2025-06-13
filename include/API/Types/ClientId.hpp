#pragma once

#include "API/InternalApi.hpp"
#include "API/Types/AccountId.hpp"
#include "API/Types/BaseId.hpp"
#include "API/Types/ShipId.hpp"
#include "API/Types/SystemId.hpp"
#include "API/Utils/FlufPayload.hpp"
#include "Defs/Structs.hpp"

#include <format>
#include <rfl/msgpack.hpp>
#include <string>

class ShipId;
struct ClientData;
class GroupId;

// All methods associated with ClientId will return a failure of Invalid clientId if the client Id is not an active client or outside acceptable range (1 -255)
class DLL ClientId
{
        uint value = 0;

        [[nodiscard]]
        Action<void> AdjustCash(int amount) const;

        [[nodiscard]]
        static ClientId GetClientIdFromCharacterName(std::wstring_view name);

    public:
        explicit ClientId(const uint val) : value(val) {}
        explicit ClientId(const SpecialChatIds id) { value = static_cast<const uint>(id); }
        explicit ClientId(const std::wstring_view str) : value(GetClientIdFromCharacterName(str)) {}
        ClientId() = default;
        ClientId(const ClientId& client) = default;

        explicit operator uint() const noexcept { return value; }
        bool operator==(const ClientId& next) const { return value == next.value; }
        bool operator<(const ClientId& right) const { return value < right.value; }
        explicit operator bool() const;

        [[nodiscard]]
        bool IsValidClientId() const;

        /**
         * @brief Returns the underlying value of the ClientId
         * @note It is generally recommended to not use this, but is sometimes required for casting purposes.
         * @returns An unsigned integer representing the client id of the user. There is no check for validity.
         */
        [[nodiscard]]
        uint GetValue() const
        {
            return value;
        }

        // Type Conversions

        /**
         * @brief Gets the character name of the character the client is logged in as.
         * @returns On success : string_view of the name
         * @returns On fail : Will return error code of character not selected if in character select.
         */
        [[nodiscard]]
        Action<std::wstring_view> GetCharacterName() const;

        /**
         * @brief Gets the BaseId of the base the client is docked on if they are.
         * @returns On success : BaseId of the base the client is docked on
         * @returns On fail : Error code of PlayerNotDocked or InCharacterSelect
         */
        [[nodiscard]]
        Action<BaseId> GetCurrentBase() const;

        /**
         * @brief Gets the SystemId of the system the client currently is in.
         * @returns On success : BaseId of the base the client is docked on
         * @returns On fail : Error code of InvalidSystem or InCharacterSelect.
         */
        [[nodiscard]]
        Action<SystemId> GetSystemId() const;

        /**
         * @brief Gets the Account of the clientId
         * @returns On success : An account id class representing the Account.
         * @returns On fail : InvalidClientId if the client check fails.
         */
        [[nodiscard]]
        Action<AccountId> GetAccount() const;

        /**
         * @brief Gets the Shiparch of the ship the client is using
         * @returns On success : Archetype::Ship pointer.
         * @returns On fail : InvalidShip error
         */
        [[nodiscard]]
        Action<EquipmentId> GetShipArch() const;

        /**
         * @brief Gets the ShipId of the ship the client is using
         * @returns On success : ShipId
         * @returns On fail : InvalidShip error or InCharacterSelect error
         */
        [[nodiscard]]
        Action<ShipId> GetShip() const;

        /**
         * @brief Get the average latency of the clients connection to the server
         * @returns On success : The average latency in ms
         * @returns On fail : InvalidClientId error
         */
        [[nodiscard]]
        Action<uint> GetLatency() const;

        /**
         * @brief Gets the Group of the player
         * @returns On success : CPlayerGroup pointer
         * @returns On fail : PlayerNotInGroup error or InCharacterSelect.
         */
        [[nodiscard]]
        Action<GroupId> GetGroup() const;

        /**
         * @brief Gets the Reputation of the player
         * @returns On success : RepId
         * @returns On fail : InvalidReputation error  or InCharacterSelect.
         */
        [[nodiscard]]
        Action<RepId> GetReputation() const;

        /**
         * @brief Gets the rank of the character the client is logged on as
         * @returns On success : Returns the rank of the player.
         * @returns On fail : InCharacterSelect
         */
        [[nodiscard]]
        Action<uint> GetRank() const;

        /**
         * @brief Gets the wealth, a combination of asset value and cash
         * @returns On success : Returns the wealth of the player.
         * @returns On fail : InCharacterSelect
         */
        [[nodiscard]]
        Action<uint> GetWealth() const;

        /**
         * @brief Get current player position. Exact point in space in case of in-space player, docked solar position when not
         * @returns Player Position
         */
        Action<Vector> GetPosition() const;

        /**
         * @brief Gets PVP kills of the character the player is logged in as
         * @returns On success : Returns the kill count as an integer.
         * @returns On fail : InCharacterSelect
         */
        [[nodiscard]]
        Action<int> GetPvpKills() const;

        /**
         * @brief Gets the cash of the character the player is logged in as.
         * @returns On success : Returns cash as an unsigned int.
         * @returns On fail : InCharacterSelect
         */
        [[nodiscard]]
        Action<uint> GetCash() const;

        /**
         * @brief Gets the percentage of the ship health. Not guaranteed to be up to date when player is in space.
         * @returns On success : Returns health percentage as a float between 0 and 1.
         * @returns On fail : InCharacterSelect
         */
        [[nodiscard]]
        Action<float> GetRelativeHealth() const;

        /**
         * @brief Sets the percentage of the ship health. To be used only when docked.
         * @returns On success : Sets health percentage as a float between 0 and 1.
         * @returns On fail : InCharacterSelect, NotDocked
         */
        [[nodiscard]]
        Action<void> SetRelativeHealth(float setHealth) const;

        /**
         * @brief Gets the equipment and cargo items in the ship of the player.
         * @returns On success : Reference to st6::list of EquipDesc of all cargo and equipment the player has.
         * @returns On fail : InCharacterSelect
         */
        [[nodiscard]]
        Action<st6::list<EquipDesc>* const> GetEquipCargo() const;

        /**
         * @brief Gets the remaining cargo hold of the player
         * @returns On success : float value representing the remaining cargo hold capacity
         * @returns On fail : InCharacterSelect
         */
        [[nodiscard]]
        Action<float> GetRemainingCargo() const;

        /**
         * @brief Gets the collision groups of the player ship.
         * @returns On success : Reference to st6::list of CollisionGroupDesc of all collision groups the player ship has.
         * @returns On fail : InCharacterSelect
         */
        [[nodiscard]]
        Action<st6::list<CollisionGroupDesc>* const> GetCollisionGroups() const;

        /**
         * @brief Gets overall data related to the clientID
         * @returns On success : ClientData associated with that clientId
         * @returns On fail : InvalidClientId
         */
        [[nodiscard]]
        ClientData& GetData() const;

        /**
         * @brief Gets the ip of the connected client
         * @returns On success : A string containing the player's ip address
         * @returns On fail : InvalidClientId
         */
        [[nodiscard]]
        Action<std::wstring> GetPlayerIp() const;

        /**
         * @brief Get's the current engine state of the client
         * @returns A enum representing the engine state, defaulting to the player not being in space.
         */
        [[nodiscard]]
        EngineState GetEngineState() const;

        // State Checks

        /**
         * @brief Checks if the player is in Space.
         * @returns On success : True if player is in space, false if they are docked or in character select.
         * @returns On fail : No fail condition.
         */
        [[nodiscard]]
        bool InSpace() const;

        /**
         * @brief Checks if the player is docked at a base.
         * @returns On success : True if player is docked, false if they are in space or in character select.
         * @returns On fail : No fail condition.
         */
        [[nodiscard]]
        bool IsDocked() const;

        /**
         * @brief Checks if the player is in character select.
         * @returns On success : True if player is in character select, false if they are in space or docked.
         * @returns On fail : No fail condition.
         */
        [[nodiscard]]
        bool InCharacterSelect() const;

        /**
         * @brief Checks if the player is currently alive
         * @returns On success : True if the player is alive or in Character Select, False if dead and on the Respawn menu.
         * @returns On fail : No fail condition.
         */
        [[nodiscard]]
        bool IsAlive() const;

        // Manipulation

        /**
         * Kicks the player
         * @param  reason if provided will post a global message, delay: if provided will kick them after that in seconds.
         * @param  delay if provided will kick them after that in seconds.
         * @returns On success : void
         * @returns On fail : InCharacterSelect
         */
        Action<void> Kick(const std::optional<std::wstring_view>& reason = {}, std::optional<uint> delay = {}) const;

        /**
         * Saves the account of the player
         * @returns On success : void
         * @returns On fail : InCharacterSelect
         */
        Action<void> SaveChar() const;

        /**
         * Sets the pvp kills of the character the client is logged in as.
         * @param killAmount the amount of kills you want to set the character to.
         * @returns On success : void
         * @returns On fail : InCharacterSelect
         */
        Action<void> SetPvpKills(uint killAmount) const;

        Action<void> SetCash(uint amount) const;

        /**
         * Adds specified amount of cash to the character the client is logged in as.
         * @param amount the amount of cash you wish to add to the character.
         * @returns On success : void
         * @returns On fail : InCharacterSelect
         */
        Action<void> AddCash(uint amount) const;

        /**
         * Removes specified amount of cash to the character the client is logged in as.
         * @param amount the amount of cash you wish to remove from the character.
         * @returns On success : void
         * @returns On fail : InCharacterSelect
         */
        Action<void> RemoveCash(uint amount) const;

        /**
         * Beams the character the client is logged on to a specified base, will fail if they are docked.
         * @param base either the BaseId or name of the base you wish to beam the client to.
         * @returns On success : void
         * @returns On fail : InCharacterSelect or PlayerNotInSpace
         */
        Action<void> Beam(BaseId base) const;

        /**
         * Renames and kicks the character the player is logged in as.
         * @param name the name you wish to rename the character to.
         * @returns On success : void
         * @returns On fail : InCharacterSelect.
         */
        // TODO: Implement Rename
        Action<void> Rename(std::wstring_view name) const;

        /**
         * Marks a specific object to make it easier to notice among other targets.
         * @param objId The id of the object that you intend to mark for this client.
         * @param markStatus The new state of the mark.
         * @returns On success : void
         * @returns On fail : InCharacterSelect or InvalidInput.
         */
        Action<void> MarkObject(uint objId, int markStatus) const;

        // Chat

        /**
         * Messages the player with provided text and format/color.
         * @param message the text you wish to send.
         * @param format format style of the message
         * @param color text color of the message
         * @returns On success : void
         * @returns On fail : InCharacterSelect.
         */
        Action<void> Message(std::wstring_view message, MessageFormat format = MessageFormat::Normal, MessageColor color = MessageColor::Default) const;

        /**
         * Messages the player with provided text with default formatting and red text.
         * @param message the text you wish to send.
         * @returns On success : void
         * @returns On fail : InCharacterSelect.
         */
        Action<void> MessageErr(std::wstring_view message) const;

        /**
         * sends a local message to the player and other players within a specified radius.
         * @param message the text you wish to send.
         * @param range the range in meters of which other players will receive the message
         * @param format format style of the message
         * @param color text color of the message
         * @returns On success : void
         * @returns On fail : InCharacterSelect or PlayerNotInSpace.
         */
        Action<void> MessageLocal(std::wstring_view message, float range = 10'000.0f, MessageFormat format = MessageFormat::Normal,
                                  MessageColor color = MessageColor::Default) const;

        /**
         * sends a private message to a specified client as if this client sent it.
         * @param destinationClient the client you wish to send the message to.
         * @param message the text you wish to send.
         * @returns On success : void
         * @returns On fail : InCharacterSelect.
         */
        Action<void> MessageFrom(ClientId destinationClient, std::wstring_view message) const;

        Action<void> MessageCustomXml(std::wstring_view rawXml) const;

        /**
         * Force the ships equipment list to match the one provided. May cause undefined behaviour if the target is in space.
         * @param equip the client you wish to send the message to.
         * @returns On success : void
         * @returns On fail : InCharacterSelect, PacketError.
         */
        Action<void> SetEquip(const st6::list<EquipDesc>& equip) const;

        Action<void> AddEquip(uint goodId, const std::wstring& hardpoint) const;

        Action<void> AddCargo(uint goodId, uint count, bool isMission) const;

        Action<void> RemoveCargo(rfl::Variant<GoodId, EquipmentId, ushort> goodId, uint count) const;

        Action<void> Undock(Vector pos, std::optional<Matrix> orientation = std::nullopt) const;

        Action<void> PlayMusic(const pub::Audio::Tryptich& info) const;

        Action<void> PlaySound(uint hash) const;

        Action<void> DisplayMissionObjective(uint ids) const;

        Action<DPN_CONNECTION_INFO> GetConnectionData() const;

        /**
         * Invites the target client to this clients group. If this client is not in a group, a new group will be created.
         * @param  otherClient the other person to invite to this client's group
         * @returns On success : void
         * @returns On fail : InCharacterSelect or InvalidClientId
         */
        Action<void> InvitePlayer(ClientId otherClient) const;

        Action<void> SendBestPath(SystemId targetSystem, Vector targetPosition) const;

        bool HasFluf() const;

        template <typename T>
        Action<void> SendFlufPayload(const char header[4], const T& payload) const
        {
            if (!IsValidClientId())
            {
                return { cpp::fail(Error::InvalidClientId) };
            }

            auto flufPayload = FlufPayload::ToPayload<T>(payload, header);
            auto bytes = flufPayload.ToBytes();

            InternalApi::FMsgSendChat(*this, bytes.data(), bytes.size());
            return { {} };
        }
};

template <>
struct std::formatter<ClientId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const ClientId& client, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", client.GetValue()); }
};

template <>
struct std::hash<ClientId>
{
        std::size_t operator()(const ClientId& id) const noexcept { return std::hash<uint>()(id.GetValue()); }
};
