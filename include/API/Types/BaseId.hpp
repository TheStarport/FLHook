#pragma once

#include "GoodId.hpp"
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
        bool operator==(const BaseId &next) const { return value == next.value; }
        bool operator<(const BaseId &right) const { return value < right.value; }
        explicit operator bool() const;

        [[nodiscard]]
        uint GetValue() const
        {
            return value;
        }

        [[nodiscard]]
        Action<ObjectId> GetSpaceId() const;
        [[nodiscard]]
        Action<SystemId> GetSystem() const;
        [[nodiscard]]
        Action<RepId> GetAffiliation() const;
        [[nodiscard]]
        Action<std::wstring_view> GetName() const;
        [[nodiscard]]
        Action<std::pair<float, float>> GetBaseHealth() const;
        [[nodiscard]]
        Action<std::pair<std::wstring_view, std::wstring_view>> GetDescription() const;
        [[nodiscard]]
        Action<std::vector<uint>> GetItemsForSale() const;
        [[nodiscard]]
        Action<float> GetCommodityPrice(GoodId goodId) const;

        [[nodiscard]]
        Action<std::vector<ClientId>> GetDockedPlayers() const;
};

template <>
struct std::formatter<BaseId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context &ctx) const { return ctx.begin(); }
        auto format(const BaseId &value, std::wformat_context &ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};

template <>
struct std::hash<BaseId>
{
        std::size_t operator()(const BaseId &id) const noexcept { return std::hash<uint>()(id.GetValue()); }
};
