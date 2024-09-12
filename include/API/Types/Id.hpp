#pragma once

class DLL Id
{
        uint value;

    public:
        explicit Id(const uint val) : value(val) {}
        explicit Id(const std::wstring_view nickName) : value(CreateID(StringUtils::wstos(nickName).c_str())){};
        explicit Id(const std::string_view nickName) : value(CreateID(nickName.data())) {}

        Id() : value(0) {}
        bool operator==(const Id& next) const { return value == next.value; }
        explicit operator bool() const { return value; };

        uint GetValue() const { return value; }

        ShipId AsShip() { return ShipId(value); }
        ObjectId AsObject() { return ObjectId(value); }
        SystemId AsSystem() { return SystemId(value); }
        BaseId AsBase() { return BaseId(value); }
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
