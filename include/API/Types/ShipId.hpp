#pragma once

#include "FLCore/FLCoreServer.h"
#include "ObjectId.hpp"
#include "RepId.hpp"

class DLL ShipId final : public ObjectId
{
    public:
        explicit ShipId(const uint val) : ObjectId(val) {}
        ShipId() = default;
        bool operator==(const ShipId& next) const { return value == next.value; }

        ~ShipId() = default;

        [[nodiscard]]
        Action<CShipPtr, Error> GetCShip(bool increment) const;
        Action<Archetype::Ship*, Error> GetShipArchetype() const;
        Action<float, Error> GetHealth(bool percentage = false) const;
        Action<float, Error> GetShields(bool percentage = false) const;
        void* GetNpcPersonality() const;
        std::optional<ClientId> GetPlayer() const;
        std::optional<ShipId> GetTarget() const;
        Action<RepId, Error> GetReputation() const;
        // TODO: AI states such as formation, go to, dock etc.
        Action<float, Error> GetSpeed() const;

        bool IsPlayer() const;
        bool IsNpc() const;
        bool IsInTradeLane() const;

        void Destroy(DestroyType type = DestroyType::Fuse);
        Action<void, Error> SetHealth(float amount, bool percentage = false);
        Action<void, Error> AddCargo(uint good, uint count, bool mission);
        void Relocate(const Vector& pos, const std::optional<Matrix>& orientation, bool sendPlayerLaunchPacket = false);

        template <typename EquipType = CEquip>
            requires std::is_base_of_v<CEquip, EquipType>
        Action<std::list<EquipType*>, Error> GetEquipment(EquipmentClass equipmentType = EquipmentClass::All)
        {
            auto ship = GetCShip(false).Unwrap();
            if (!ship)
            {
                return { cpp::fail(Error::InvalidShip) };
            }

            CEquipTraverser traverser(static_cast<int>(equipmentType));
            CEquipManager& manager = ship.get()->equip_manager;
            CEquip* equip = manager.Traverse(traverser);

            std::list<EquipType*> equipment;

            while (equip)
            {
                equipment.emplace_back(static_cast<EquipType*>(equip));
                equip = manager.Traverse(traverser);
            }

            return { equipment };
        }

        Action<void, Error> IgniteFuse(uint fuseId, float id = 0.0f) const;
        Action<void, Error> ExtinguishFuse(uint fuseId, float id = 0.0f) const;
};

template <>
struct std::formatter<ShipId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const ShipId& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};

template <>
struct std::hash<ShipId>
{
    std::size_t operator()(const ShipId &id) const noexcept { return std::hash<uint>()(id.GetValue()); }
};