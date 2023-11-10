#pragma once

#include "ObjectId.hpp"

class SystemId;

class ShipId final : ObjectId
{
    public:
        explicit ShipId(const uint val) : ObjectId(val) {}
        explicit ShipId() {}

        Action<CShipPtr, Error> GetCShip(bool increment) const;
        Archetype::Ship* GetShipArchetype();
        float GetHealth(bool percentage = false);
        st6::list<EquipDesc>& GetEquipment(); // TODO: We should define a lambda to make it easy for people to get Equip lists.
        float GetShields(bool percentage = false);
        void* GetCargo(); // TODO: Similar to equipment but for cargo (duh).
        void* GetNpcPersonality();
        std::optional<ShipId> GetTarget();
        std::wstring GetAffiliation();
        // TODO: AI states such as formation, go to, dock etc.
        float GetSpeed();

        bool IsPlayer();
        bool IsNpc();
        bool IsInTradeLane();

        void Destroy(DestroyType type = DestroyType::Fuse);
        void SetHealth(float amount, bool percentage = false);
        void AddCargo(std::wstring_view good, int count, bool mission);
        void SetEquip(const st6::list<EquipDesc>& equip);
        void AddEquip(uint goodId, const std::wstring& hardpoint);
        void Relocate(Vector, std::optional<Matrix> orientation);
};
