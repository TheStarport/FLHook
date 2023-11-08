#pragma once

class BaseId
{
        const uint value;

    public:
        explicit BaseId(const uint val) : value(val) {}
        explicit BaseId(std::wstring_view name, bool isWildCard = false);
        explicit operator uint() const noexcept { return value; }
        bool operator==(const BaseId next) const { return value == next.value; }
        bool operator!() const; // TODO: Check if BaseId is valid here



        std::vector<ClientId> GetDockedPlayers();
        void* GetMarket(); // Grab the bases market data.
        std::optional<std::wstring> GetAffiliation();
        //TODO: @Laz, look into getting the physical base from abstract baseID.
        std::wstring GetName();
       // std::pair<std::wstring, uint> GetDescription();

        bool ToggleDocking(bool locked); //TODO: THis would be done via the CSolar.

};