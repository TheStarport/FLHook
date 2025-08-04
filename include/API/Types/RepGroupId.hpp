#pragma once

class RepId;

class DLL RepGroupId final
{
        uint value = 0;

    public:
        explicit RepGroupId(const uint val) : value(val) {}
        explicit RepGroupId(std::wstring_view nickName);

        explicit RepGroupId() = default;
        ~RepGroupId() = default;

        explicit operator uint() const noexcept { return value; }
        bool operator==(const RepGroupId& next) const { return value == next.value; }
        bool operator<(const RepGroupId& next) const { return value < next.value; }
        explicit operator bool() const;

        uint GetValue() const { return value; }

        Action<std::wstring_view> GetName() const;
        Action<std::wstring_view> GetShortName() const;
        Action<float> GetAttitudeTowardsRepId(RepId target) const;
};

template <>
struct std::formatter<RepGroupId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const RepGroupId& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};

template <>
struct std::hash<RepGroupId>
{
        std::size_t operator()(const RepGroupId& id) const noexcept { return std::hash<uint>()(id.GetValue()); }
};
