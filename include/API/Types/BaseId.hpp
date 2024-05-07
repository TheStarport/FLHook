#pragma once

#include "ObjectId.hpp"
#include "RepId.hpp"

class DLL BaseId
{
        uint value;

    public:
        explicit BaseId(const uint val) : value(val) {}
        explicit BaseId(std::wstring_view name, bool isWildCard = false);
        explicit operator uint() const noexcept { return value; }
        BaseId() : value(0) {}
        bool operator==(const BaseId next) const { return value == next.value; }
        explicit operator bool() const;

        uint GetValue() const { return value; }

        Action<ObjectId, Error> GetSpaceId() const;
        Action<RepId, Error> GetAffiliation() const;
        Action<std::wstring_view, Error> GetName() const;
        Action<std::pair<float, float>, Error> GetBaseHealth() const;
        Action<std::pair<std::wstring_view, std::wstring_view>, Error> GetDescription() const;
        Action<std::vector<uint>, Error> GetItemsForSale() const;
        Action<float, Error> GetCommodityPrice(GoodId goodId) const;

        Action<std::vector<ClientId>, Error> GetDockedPlayers();
};

template <>
struct std::formatter<BaseId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context &ctx) const { return ctx.begin(); }
        auto format(const BaseId &value, std::wformat_context &ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};
