#include "PCH.hpp"

#include "API/Types/EquipmentId.hpp"

#define ValidEquipmentCheck                            \
    if (!this->operator bool())                        \
    {                                                  \
        return { cpp::fail(Error::InvalidEquipment) }; \
    }

EquipmentId::operator bool() const { return Archetype::GetEquipment(value) != nullptr; }

Action<EquipmentType, Error> EquipmentId::GetType() const
{
    ValidEquipmentCheck;

    static const uint vftMine = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableMine);
    static const uint vftCm = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableCm);
    static const uint vftGun = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableGun);
    static const uint vftShieldGen = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableShieldGen);
    static const uint vftThruster = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableThruster);
    static const uint vftShieldBat = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableShieldBat);
    static const uint vftNanobot = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableNanobot);
    static const uint vftMunition = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableMunition);
    static const uint vftEngine = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableEngine);
    static const uint vftScanner = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableScanner);
    static const uint vftTractor = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableTractor);
    static const uint vftLight = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableLight);

    Archetype::Equipment* eq = Archetype::GetEquipment(value);
    const uint vft = eq->vtable; // NOLINT
    if (vft == vftGun)
    {
        const Archetype::Gun* gun = reinterpret_cast<Archetype::Gun*>(eq);
        Archetype::Equipment* eqAmmo = Archetype::GetEquipment(gun->projectileArchId);
        int missile;
        memcpy(&missile, reinterpret_cast<char*>(eqAmmo) + 0x90, 4);
        const uint gunType = gun->get_hp_type_by_index(0);
        if (gunType == 36)
        {
            return { EquipmentType::Torpedo };
        }
        if (gunType == 35)
        {
            return { EquipmentType::Cd };
        }
        if (missile)
        {
            return { EquipmentType::Missile };
        }

        return { EquipmentType::Gun };
    }
    if (vft == vftCm)
    {
        return { EquipmentType::Cm };
    }
    if (vft == vftShieldGen)
    {
        return { EquipmentType::ShieldGen };
    }
    if (vft == vftThruster)
    {
        return { EquipmentType::Thruster };
    }
    if (vft == vftShieldBat)
    {
        return { EquipmentType::ShieldBattery };
    }
    if (vft == vftNanobot)
    {
        return { EquipmentType::Nanobot };
    }
    if (vft == vftMunition)
    {
        return { EquipmentType::Munition };
    }
    if (vft == vftMine)
    {
        return { EquipmentType::Mine };
    }
    if (vft == vftEngine)
    {
        return { EquipmentType::Engine };
    }
    if (vft == vftLight)
    {
        return { EquipmentType::Light };
    }
    if (vft == vftScanner)
    {
        return { EquipmentType::Scanner };
    }
    if (vft == vftTractor)
    {
        return { EquipmentType::Tractor };
    }
    return { EquipmentType::Other };
}
