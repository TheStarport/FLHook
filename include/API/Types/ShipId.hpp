#pragma once

#include "API/Utils/Action.hpp"
#include "FLCore/FLCoreServer.h"
#include "ObjectId.hpp"
#include "API/Types/GroupId.hpp"
#include "RepId.hpp"

class DLL ShipId final : public ObjectId
{
    public:
        explicit ShipId(uint val);
        ShipId() = default;
        bool operator==(const ShipId& next) const;

        [[nodiscard]]
        Action<const Archetype::Ship*> GetShipArchetype() const;

        [[nodiscard]]
        Action<float> GetShields(bool percentage = false) const;

        [[nodiscard]]
        std::weak_ptr<CShip> GetValue() const;

        // TODO: Implement Me!
        [[nodiscard]]
        void* GetNpcPersonality() const;

        [[nodiscard]]
        Action<ClientId> GetPlayer() const;

        [[nodiscard]]
        Action<GroupId> GetGroup() const;

        [[nodiscard]]
        Action<ObjectId> GetTarget() const;

        [[nodiscard]]
        Action<RepId> GetReputation() const;

        // TODO: AI states such as formation, go to, dock etc.
        [[nodiscard]]
        Action<float> GetSpeed() const;

        [[nodiscard]]
        bool IsPlayer() const;

        [[nodiscard]]
        bool IsNpc() const;

        [[nodiscard]]
        bool IsInTradeLane() const;

        Action<void> Destroy(DestroyType type = DestroyType::Fuse);
        Action<void> SetHealth(float amount, bool percentage = false);
        Action<void> AddCargo(uint good, uint count, bool mission);
        Action<void> Relocate(std::optional<Vector> pos, const std::optional<Matrix>& orientation = std::nullopt) const;

        template <typename EquipType = CEquip>
            requires std::is_base_of_v<CEquip, EquipType>
        Action<std::list<EquipType*>> GetEquipment(EquipmentClass equipmentType = EquipmentClass::All)
        {
            auto ship = std::dynamic_pointer_cast<CShip>(value.lock());
            if (!ship)
            {
                return { cpp::fail(Error::ObjectIsNotAShip) };
            }

            CEquipTraverser traverser(static_cast<int>(equipmentType));
            CEquipManager& manager = ship->equipManager;
            CEquip* equip = manager.Traverse(traverser);

            std::list<EquipType*> equipment;

            while (equip)
            {
                equipment.emplace_back(static_cast<EquipType*>(equip));
                equip = manager.Traverse(traverser);
            }

            return { equipment };
        }

        Action<void> IgniteFuse(Id fuseId, float id = 0.0f) const;
        Action<void> ExtinguishFuse(Id fuseId, float id = 0.0f) const;
        [[nodiscard]]
        Action<CEquipManager*> GetEquipmentManager() const;
};

template <>
struct std::formatter<ShipId, wchar_t>
{
        constexpr auto parse(std::wformat_parse_context& ctx) const { return ctx.begin(); }
        auto format(const ShipId& value, std::wformat_context& ctx) const { return std::format_to(ctx.out(), L"{}", value.GetId().Unwrap()); }
};

template <>
struct std::hash<ShipId>
{
        std::size_t operator()(const ShipId& id) const noexcept { return std::hash<uint>()(id.GetId().Unwrap().GetValue()); }
};
