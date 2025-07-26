#pragma once
#include "FLCore/Common/Archetype/Root/Equipment.hpp"

namespace Archetype
{
    struct Gun;
    struct Mine;
    struct Launcher;
    struct ShieldGenerator;
    struct Thruster;
    struct Engine;
    struct Scanner;
    struct Tractor;
    struct Light;
    struct Munition;
    struct CounterMeasureDropper;
    struct RepairKit;
    struct ShieldBattery;
    struct CloakingDevice;
} // namespace Archetype
// TODO: Allow assigning to uint variables without the need to extract GetValue explicitly
class Id;
class DLL EquipmentId
{
        Archetype::Root* value = nullptr;

    public:
        explicit EquipmentId(uint val);
        explicit EquipmentId(Id val);
        EquipmentId() = default;
        bool operator==(const EquipmentId next) const { return value == next.value; }
        bool operator<(const EquipmentId& right) const;
        EquipmentId& operator=(const EquipmentId& right) = default;
        explicit operator bool() const;

        // Returns the underlying value of the ClientId, it is generally recommended to not use this.
        [[nodiscard]]
        Archetype::Root* GetValue() const
        {
            return value;
        }

        [[nodiscard]]
        Id GetId() const;

        [[nodiscard]]
        Action<EquipmentType> GetType() const;

        [[nodiscard]]
        Action<std::wstring_view> GetName() const;

        [[nodiscard]]
        Action<float> GetVolume() const;

        template <typename T>
        Action<T*> Cast()
        {
            auto type = GetType().Raw();

            if (type.has_error())
            {
                return { cpp::fail(type.error()) };
            }

            if constexpr (std::is_same_v<T, Archetype::Gun>)
            {
                if(type.value() == EquipmentType::Gun)
                {
                    return {reinterpret_cast<T*>(value)};
                }
            }
            if constexpr (std::is_same_v<T, Archetype::Ship>)
            {
                if(type.value() == EquipmentType::Ship)
                {
                    return {reinterpret_cast<T*>(value)};
                }
            }
            if constexpr (std::is_same_v<T, Archetype::Mine>)
            {
                if(type.value() == EquipmentType::Mine)
                {
                    return {reinterpret_cast<T*>(value)};
                }
            }
            if constexpr (std::is_same_v<T, Archetype::ShieldGenerator>)
            {
                if(type.value() == EquipmentType::ShieldGen)
                {
                    return {reinterpret_cast<T*>(value)};
                }
            }
            if constexpr (std::is_same_v<T, Archetype::Thruster>)
            {
                if(type.value() == EquipmentType::Thruster)
                {
                    return {reinterpret_cast<T*>(value)};
                }
            }
            if constexpr (std::is_same_v<T, Archetype::Light>)
            {
                if(type.value() == EquipmentType::Light)
                {
                    return {reinterpret_cast<T*>(value)};
                }
            }
            if constexpr (std::is_same_v<T, Archetype::Engine>)
            {
                if(type.value() == EquipmentType::Engine)
                {
                    return {reinterpret_cast<T*>(value)};
                }
            }
            if constexpr (std::is_same_v<T, Archetype::Munition>)
            {
                if(type.value() == EquipmentType::Munition)
                {
                    return {reinterpret_cast<T*>(value)};
                }
            }
            if constexpr (std::is_same_v<T, Archetype::CounterMeasureDropper>)
            {
                if(type.value() == EquipmentType::CmDropper)
                {
                    return {reinterpret_cast<T*>(value)};
                }
            }
            if constexpr (std::is_same_v<T, Archetype::RepairKit>)
            {
                if(type.value() == EquipmentType::Nanobot)
                {
                    return {reinterpret_cast<T*>(value)};
                }
            }
            if constexpr (std::is_same_v<T, Archetype::ShieldBattery>)
            {
                if (type.value() == EquipmentType::ShieldBattery)
                {
                    return { reinterpret_cast<T*>(value) };
                }
            }
            if constexpr (std::is_same_v<T, Archetype::CloakingDevice>)
            {
                if (type.value() == EquipmentType::CloakingDevice)
                {
                    return { reinterpret_cast<T*>(value) };
                }
            }

            return { cpp::fail(type.error()) };
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
        std::size_t operator()(const EquipmentId& id) const noexcept { return std::hash<uint>()(id.GetValue()->archId.GetValue()); }
};
