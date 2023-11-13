#pragma once

#include "ObjectId.hpp"
#include "RepId.hpp"

class BaseId
{
        uint value;

    public:
        explicit BaseId(const uint val) : value(val) {}
        explicit BaseId(std::wstring_view name, bool isWildCard = false);
        explicit operator uint() const noexcept { return value; }
        explicit BaseId() : value(0) {}
        bool operator==(const BaseId next) const { return value == next.value; }
        explicit operator bool() const;

        uint GetValue() const { return value; }

        Action<ObjectId, Error> GetSpaceId() const;
        Action<RepId, Error> GetAffiliation() const;
        Action<std::wstring_view, Error> GetName() const;
        Action<std::pair<std::wstring_view, std::wstring_view>, Error> GetDescription();

        Action<std::vector<uint>, Error> GetItemsForSale() const;

        Action<std::vector<ClientId>, Error> GetDockedPlayers();

        bool ToggleDocking(bool locked); // TODO: THis would be done via the CSolar.
};
