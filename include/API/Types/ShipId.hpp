#pragma once

#include "ObjectId.hpp"
#include "RepId.hpp"

class ShipId final : ObjectId
{
    public:
        explicit ShipId(const uint val) : ObjectId(val) {}
        explicit ShipId() {}

        [[nodiscard]]
        Action<CShipPtr, Error> GetCShip(bool increment);
        Action<Archetype::Ship*, Error> GetShipArchetype();
        Action<float, Error> GetHealth(bool percentage = false);
        Action<float, Error> GetShields(bool percentage = false);
        void* GetNpcPersonality();
        std::optional<ShipId> GetTarget();
        Action<RepId, Error> GetReputation();
        std::wstring GetAffiliation();
        // TODO: AI states such as formation, go to, dock etc.
        Action<float, Error> GetSpeed();

        bool IsPlayer();
        bool IsNpc();
        bool IsInTradeLane();

        void Destroy(DestroyType type = DestroyType::Fuse);
        Action<void, Error> SetHealth(float amount, bool percentage = false);
        Action<void, Error> AddCargo(uint good, uint count, bool mission);
        void SetEquip(const st6::list<EquipDesc>& equip);
        void AddEquip(uint goodId, const std::wstring& hardpoint);
        void Relocate(Vector, std::optional<Matrix> orientation);

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
};
