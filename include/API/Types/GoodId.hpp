#pragma once

#include "EquipmentId.hpp"
#include "FLCore/Common/GoodType.hpp"

#include <format>

struct GoodInfo;

// All methods associated with ClientId will return a failure of Invalid clientId if the client Id is not an active client or outside acceptable range (1 -255)
class DLL GoodId
{
        const GoodInfo *const value = nullptr;

    public:
        explicit GoodId(uint hash);
        explicit GoodId() = default;
        GoodId(const GoodId &client) = default;

        explicit operator const GoodInfo *() const noexcept;
        bool operator==(const GoodId &next) const;
        explicit operator bool() const;

        [[nodiscard]]
        const GoodInfo *GetValue() const;

        [[nodiscard]]
        Action<uint, Error> GetHash() const;

        [[nodiscard]]
        Action<EquipmentId, Error> GetEquipment() const;

        [[nodiscard]]
        Action<std::wstring_view, Error> GetName() const;

        [[nodiscard]]
        Action<float, Error> GetPrice() const;

        [[nodiscard]]
        Action<GoodType, Error> GetType() const;

        [[nodiscard]]
        bool IsCommodity() const;
};

template <>
struct std::formatter<GoodId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context &ctx) const { return ctx.begin(); }
        auto format(const GoodId &good, std::wformat_context &ctx) const { return std::format_to(ctx.out(), L"{}", good.GetHash().Unwrap()); }
};

template <>
struct std::hash<GoodId>
{
        std::size_t operator()(const GoodId &id) const noexcept { return std::hash<uint>()(id.GetHash().Unwrap()); }
};
