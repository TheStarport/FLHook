#pragma once

#include "ObjectId.hpp"
#include "RepId.hpp"

class ShipId final : public ObjectId
{
    public:
        explicit ShipId(const uint val) : ObjectId(val) {}
        explicit ShipId() = default;

        ~ShipId() override = default;

        [[nodiscard]]
        Action<CShipPtr, Error> GetCShip(bool increment);
        Action<Archetype::Ship*, Error> GetShipArchetype();
        Action<float, Error> GetHealth(bool percentage = false);
        Action<float, Error> GetShields(bool percentage = false);
        void* GetNpcPersonality();
        std::optional<ClientId> GetPlayer();
        std::optional<ShipId> GetTarget();
        Action<RepId, Error> GetReputation();
        // TODO: AI states such as formation, go to, dock etc.
        Action<float, Error> GetSpeed();

        bool IsPlayer();
        bool IsNpc();
        bool IsInTradeLane();

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

            CEquipTraverser traverser(equipmentType);
            CEquipManager* manager = GetEquipManager(ship.get());
            CEquip* equip = manager->Traverse(traverser);

            std::list<EquipType*> equipment;

            while (equip)
            {
                equipment.emplace_back(static_cast<EquipType*>(equip));
                equip = manager->Traverse(traverser);
            }

            return { equipment };
        }

        Action<void, Error> IgniteFuse(uint fuseId, float id = 0.0f) const;
        Action<void, Error> ExtinguishFuse(uint fuseId, float id = 0.0f) const;
};

template <>
struct std::formatter<ShipId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) { return ctx.begin(); }
        auto format(const ShipId& value, std::wformat_context& ctx) { return std::format_to(ctx.out(), L"{}", value.GetValue()); }
};
