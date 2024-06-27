#pragma once

class DLL UnknownId
{
        uint value;

    public:
        explicit UnknownId(const uint val) : value(val) {}
        explicit UnknownId(const std::wstring_view nickName) : value(CreateID(StringUtils::wstos(nickName).c_str())) {};
        explicit UnknownId(const std::string_view nickName) : value(CreateID(nickName.data())) {}

        UnknownId() : value(0) {}
        bool operator==(const UnknownId& next) const { return value == next.value; }
        explicit operator bool() const { return value; };

        uint GetValue() const { return value; }
};

template <>
struct std::formatter<UnknownId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context &ctx) const { return ctx.begin(); }
        auto format(const UnknownId &value, std::wformat_context &ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};

template <>
struct std::hash<UnknownId>
{
    std::size_t operator()(const UnknownId &id) const noexcept { return std::hash<uint>()(id.GetValue()); }
};