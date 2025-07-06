#include "PCH.hpp"

#include "API/FLHook/InfocardManager.hpp"
#include "API/Types/EquipmentId.hpp"

#define ValidEquipmentCheck                            \
    if (!this->operator bool())                        \
    {                                                  \
        return { cpp::fail(Error::InvalidEquipment) }; \
    }

EquipmentId::EquipmentId(const uint val)
{
    value = Archetype::GetEquipment(val);

    if (!value)
    {
        value = Archetype::GetShip(val);
    }
}

EquipmentId::EquipmentId(const Id val) { EquipmentId(val.GetValue()); }

bool EquipmentId::operator<(const EquipmentId& right) const { return value && value->archId < right.value->archId; }

EquipmentId::operator bool() const { return value != nullptr; }

Id EquipmentId::GetId() const
{
    if (!value)
    {
        return Id();
    }

    return Id(value->archId);
}

Action<EquipmentType> EquipmentId::GetType() const
{
    ValidEquipmentCheck;

    static const uint vftMine = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableMine);
    static const uint vftMineDropper = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableMineDropper);
    static const uint vftCm = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableCm);
    static const uint vftCmDropper = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableCmDropper);
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
    static const uint vftShip = FLHook::Offset(FLHook::BinaryType::Common, AddressList::CommonVfTableShip);

    const uint vft = *reinterpret_cast<const uint*>(value); // NOLINT
    if (vft == vftGun)
    {
        const Archetype::Gun* gun = reinterpret_cast<Archetype::Gun*>(value);
        Archetype::Equipment* eqAmmo = Archetype::GetEquipment(gun->projectileArchId.GetValue());
        int missile;
        memcpy(&missile, reinterpret_cast<char*>(eqAmmo) + 0x90, 4);
        const uint gunType = gun->get_hp_type_by_index(0);
        if (gunType == 36)
        {
            return { EquipmentType::TorpedoLauncher };
        }

        if (gunType == 35)
        {
            return { EquipmentType::CdLauncher };
        }

        if (missile)
        {
            return { EquipmentType::MissileLauncher };
        }

        return { EquipmentType::Gun };
    }

    if (vft == vftCm)
    {
        return { EquipmentType::Cm };
    }

    if (vft == vftCmDropper)
    {
        return { EquipmentType::CmDropper };
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

    if (vft == vftMineDropper)
    {
        return { EquipmentType::MineDropper };
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

    if (vft == vftShip)
    {
        return { EquipmentType::Ship };
    }

    return { EquipmentType::Other };
}

Action<std::wstring_view> EquipmentId::GetName() const
{
    ValidEquipmentCheck;

    return { FLHook::GetInfocardManager()->GetInfoName(value->idsName) };
}

Action<float> EquipmentId::GetVolume() const
{
    ValidEquipmentCheck;

    if (dynamic_cast<Archetype::Equipment*>(value))
    {
        return { reinterpret_cast<const Archetype::Equipment*>(value)->volume };
    }

    return { reinterpret_cast<const Archetype::Ship*>(value)->holdSize };
}
