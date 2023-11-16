#pragma once

class EquipmentId
{
        uint value = 0;

    public:
        explicit EquipmentId(const uint val) : value(val) {}
        explicit EquipmentId() = default;
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
struct std::formatter<EquipmentId> : std::formatter<uint>
{
        auto format(const EquipmentId &value, std::format_context &ctx) { return std::formatter<uint>::format(value.GetValue(), ctx); }
};
