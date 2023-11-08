#pragma once

class SystemId;

class ShipId
{
        const uint value;

    public:
        explicit ShipId(const uint val) : value(val) {}
        explicit operator uint() const noexcept { return value; }
        explicit ShipId() : value(0) {}
        bool operator==(const ShipId next) const { return value == next.value; }
        bool operator!() const; // TODO: Check if shipId is valid

        std::wstring GetNickname();
        std::wstring GetName();
        CShip* GetCShip();
        Archetype::Ship* GetArchetype();
        float GetHealth(bool percentage = false);
        st6::list<EquipDesc>& GetEquipment(); // TODO: We should define a lambda to make it easy for people to get Equip lists.
        float GetShields(bool percentage = false);
        void* GetCargo(); // TODO: Similar to equipment but for cargo (duh).
        void* GetNpcPersonality();
        std::optional<ShipId> GetTarget();
        std::wstring GetAffiliation();
        // TODO: AI states such as formation, go to, dock etc.
        SystemId GetSystem();
        Vector GetPosition();
        Matrix GetOrientation();
        Vector GetVelocity();
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

        // TODO: SetShields() potentially, @Aingar should know more about this.
};