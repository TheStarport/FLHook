#pragma once
#include "FLCore/Common/Archetype/Root/Equipment.hpp"

namespace Archetype
{
    struct Gun;
} // namespace Archetype
// TODO: Allow assigning to uint variables without the need to extract GetValue explicitly
class DLL EquipmentId
{
        Archetype::Root* value = nullptr;

    public:
        explicit EquipmentId(uint val);
        EquipmentId() = default;
        bool operator==(const EquipmentId next) const { return value == next.value; }
        bool operator<(const EquipmentId& right) const { return value && value->archId < right.value->archId; }
        EquipmentId& operator=(const EquipmentId& right)
        {
            value = right.value;
            return *this;
        }
        explicit operator bool() const;

        // Returns the underlying value of the ClientId, it is generally recommended to not use this.
        [[nodiscard]]
        Archetype::Root* GetValue() const
        {
            return value;
        }

        [[nodiscard]]
        uint GetId() const
        {
            if (!value)
            {
                return 0;
            }

            return value->archId;
        }

        [[nodiscard]]
        Action<EquipmentType> GetType() const;

        [[nodiscard]]
        Action<std::wstring_view> GetName() const;

        [[nodiscard]]
        Action<float> GetVolume() const;

        template <typename T>
        Action<T> Cast()
        {
            auto type = GetType().Raw();

            if (type.has_error())
            {
                return { cpp::fail(type.error()) };
            }

            if constexpr (std::is_same_v<T, Archetype::Gun> && type.value() == EquipmentType::Gun)
            {
                return reinterpret_cast<T*>(value);
            }
        }
};

template <>
struct std::formatter<EquipmentId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const EquipmentId& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetId()); }
};

template <>
struct std::hash<EquipmentId>
{
        std::size_t operator()(const EquipmentId& id) const noexcept { return std::hash<uint>()(id.GetValue()->archId); }
};
