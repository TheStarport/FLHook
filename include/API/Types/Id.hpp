#pragma once

class SystemId;
class BaseId;
class ShipId;
class ObjectId;

class DLL Id
{
        uint value;

    public:
        explicit Id(const uint val) : value(val) {}
        explicit Id(const std::wstring_view nickName);
        explicit Id(const std::string_view nickName);

        Id() : value(0) {}
        explicit operator uint() const noexcept { return value; }
        bool operator==(const Id& next) const { return value == next.value; }
        bool operator<(const Id& next) const { return value < next.value; }
        explicit operator bool() const { return value; };

        uint GetValue() const { return value; }

        ShipId AsShip();
        ObjectId AsObject();
        SystemId AsSystem();
        BaseId AsBase();
};

template <>
struct std::formatter<Id, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const Id& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};

template <>
struct std::hash<Id>
{
        std::size_t operator()(const Id& id) const noexcept { return std::hash<uint>()(id.GetValue()); }
};

template <>
struct std::less<Id>
{
        bool operator()(const Id& lhs, const Id& rhs) const { return lhs.GetValue() < rhs.GetValue(); }
};
