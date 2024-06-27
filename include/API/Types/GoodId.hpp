#pragma once

#include <format>
#include <string>

// All methods associated with ClientId will return a failure of Invalid clientId if the client Id is not an active client or outside acceptable range (1 -255)
class DLL GoodId
{
        uint value = 0;

    public:
        explicit GoodId(const uint val) : value(val) {}
        explicit GoodId() = default;
        GoodId(const GoodId &client) : value(client.value){};

        explicit operator uint() const noexcept { return value; }
        bool operator==(const GoodId &next) const { return value == next.value; }
        explicit operator bool() const;
        uint GetValue() const { return value; }
};

template <>
struct std::formatter<GoodId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context &ctx) const { return ctx.begin(); }
        auto format(const GoodId &good, std::wformat_context &ctx) const { return std::format_to(ctx.out(),
            L"{}", good.GetValue()); }
};

template <>
struct std::hash<GoodId>
{
        std::size_t operator()(const GoodId &id) const noexcept { return std::hash<uint>()(id.GetValue()); }
};
