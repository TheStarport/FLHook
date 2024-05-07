#pragma once

class DLL EquipmentId
{
        uint value = 0;

    public:
        explicit EquipmentId(const uint val) : value(val) {}
        EquipmentId() = default;
        bool operator==(const EquipmentId next) const { return value == next.value; }
        explicit operator bool() const;

        // Returns the underlying value of the ClientId, it is generally recommended to not use this.
        [[nodiscard]]
        uint GetValue() const
        {
            return value;
        }

        Action<EquipmentType, Error> GetType() const;
};

template <>
struct std::formatter<EquipmentId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context &ctx) const { return ctx.begin(); }
        auto format(const EquipmentId &value, std::wformat_context &ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};
