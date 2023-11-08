#pragma once

class BaseId;
class ShipId;
class SystemId;

class ClientId
{
        const uint value;

        Action<void, Error> AdjustCash(int amount);

        bool IsValidClientId();

    public:
        explicit ClientId(const uint val) : value(val) {}
        explicit ClientId(std::wstring_view name);
        explicit operator uint() const noexcept { return value; }
        bool operator==(const ClientId next) const { return value == next.value; }
        bool operator!() const { return !(value > 0 && value < 256); }


        // Returns the underlying value of the ClientId, it is generally recommended to not use this.
        uint GetValue();

        // Type Conversions

        Action<std::wstring,Error> GetCharacterName();
        // TODO: These eventually will be their own types as well
        Action<BaseId, Error> GetCurrentBase();
        SystemId GetSystemId();
        CAccount* GetAccount();
        Archetype::Ship* GetShipArch();
        ShipId GetShipId();
        st6::list<EquipDesc>& GetEquipment();
        CPlayerGroup* GetGroup();
        std::optional<std::wstring> GetAffiliation();
        CShip* GetShip();
        uint GetRank();
        uint GetWealth();
        Action<int, Error> GetPvpKills();
        Action<uint, Error> GetCash();

        // State Checks

        bool InSpace();
        bool InCharacterSelect();
        bool IsAlive();

        // Manipulation

        Action<void, Error> Kick(std::optional<std::wstring_view> reason = {}, std::optional<uint> delay = 10);
        Action<void, Error> SaveChar();
        Action<void, Error> SetPvpKills(uint killAmount);
        Action<void, Error> AddCash(uint amount);
        Action<void, Error> RemoveCash(uint amount);
        Action<void, Error> Beam(std::variant<BaseId, std::wstring_view> base);
        Action<void, Error> Message(std::wstring message, std::optional<MessageFormat> format, std::optional<MessageColor> color);
        Action<void, Error> SetRep(std::variant<ushort, std::wstring_view> repGroup, float rep);
        Action<void, Error> Rename(std::wstring_view);
        void MarkObject(uint objId, int markStatus);
};