#include "PCH.hpp"

#include "API/Types/ShipId.hpp"

#include "API/FLHook/ResourceManager.hpp"

#include <glm/gtc/quaternion.hpp>

#define IsValidShip                                             \
    auto ship = std::dynamic_pointer_cast<CShip>(value.lock()); \
    if (!ship)                                                  \
    {                                                           \
        return { cpp::fail(Error::InvalidEquipment) };              \
    }

ShipId::ShipId(const uint val) { value = FLHook::GetResourceManager()->Get<CShip>(val); }

bool ShipId::operator==(const ShipId& next) const { return static_cast<const ObjectId&>(*this) == next; }

Action<const Archetype::Ship*> ShipId::GetShipArchetype() const
{
    IsValidShip;

    return { ship->shiparch() };
}

Action<float> ShipId::GetShields(bool percentage) const
{
    IsValidShip;

    auto shield = reinterpret_cast<CEShield*>(ship->equipManager.FindFirst(static_cast<unsigned int>(EquipmentClass::Shield)));
    if (!shield || shield->maxShieldHitPoints < 1.0f)
    {
        return { 0.0f };
    }

    if (percentage)
    {
        return { shield->currShieldHitPoints / shield->maxShieldHitPoints };
    }

    return { shield->currShieldHitPoints };
}

Action<ClientId> ShipId::GetPlayer() const
{
    IsValidShip;

    if (!ship->ownerPlayer)
    {
        return { cpp::fail(Error::ObjectIsNotAPlayer) };
    }

    return { ClientId(ship->ownerPlayer) };
}

Action<ObjectId> ShipId::GetTarget() const
{
    IsValidShip;

    // Returns IObjectRW*, whatever that is.
    const auto target = ship->get_target();
    if (target)
    {
        return { ObjectId(target->get_id()) };
    }

    return { {} };
}

Action<RepId> ShipId::GetReputation() const
{
    IsValidShip;

    return { RepId(ship->repVibe) };
}

Action<float> ShipId::GetSpeed() const
{
    IsValidShip;

    return { glm::length<3, float, glm::highp>(ship->get_velocity()) };
}

bool ShipId::IsPlayer() const { return !value.expired() && std::dynamic_pointer_cast<CShip>(value.lock())->is_player(); }

bool ShipId::IsNpc() const { return !IsPlayer(); }

bool ShipId::IsInTradeLane() const { return !value.expired() && std::dynamic_pointer_cast<CShip>(value.lock())->inTradeLane; }

Action<void> ShipId::Destroy(DestroyType type)
{
    IsValidShip;

    // TODO: Check for failure
    pub::SpaceObj::Destroy(ship->id, type);

    return { {} };
}

Action<void> ShipId::SetHealth(float amount, bool percentage)
{
    IsValidShip;

    // If provided an invalid percentage along with requesting a percentage scale it will simply fail.
    if ((amount < 0.0f || amount > 1.0f) && percentage)
    {
        return { cpp::fail(Error::UnknownError) };
    }

    if (!percentage)
    {
        ship->set_hit_pts(amount);
        return { {} };
    }

    auto maxHp = ship->shiparch()->hitPoints;
    ship->set_hit_pts(amount * maxHp);
    return { {} };
}

Action<void> ShipId::AddCargo(uint good, uint count, bool mission)
{
    IsValidShip;

    const auto goodInfo = GoodList::find_by_id(good);
    if (!goodInfo)
    {
        return { cpp::fail(Error::InvalidGood) };
    }

    if (ship->is_player())
    {
        auto client = ClientId(ship->ownerPlayer);

        uint base, location = 0;
        pub::Player::GetBase(client.GetValue(), base);
        pub::Player::GetLocation(client.GetValue(), location);

        // trick cheat detection
        if (base)
        {
            if (location)
            {
                Server.LocationExit(location, client.GetValue());
            }
            Server.BaseExit(base, client.GetValue());

            if (!client)
            {
                return { cpp::fail(Error::PlayerNotOnline) };
            }
        }

        if (goodInfo->multiCount)
        {
            // we need to do this, else server or client may crash
            for (const auto cargo = client.GetEquipCargo().Handle(); const auto& item : *cargo)
            {
                if (item.archId == good && item.mission != mission)
                {
                    pub::Player::RemoveCargo(client.GetValue(), static_cast<ushort>(item.id), item.count);
                    count += item.count;
                }
            }

            pub::Player::AddCargo(client.GetValue(), good, count, 1, mission);
        }
        else
        {
            for (int i = 0; i < count; i++)
            {
                pub::Player::AddCargo(client.GetValue(), good, 1, 1, mission);
            }
        }

        if (base)
        {
            // player docked on base
            ///////////////////////////////////////////////////
            // fix, else we get anti-cheat msg when undocking
            // this DOES NOT disable anti-cheat-detection, we're
            // just making some adjustments so that we dont get kicked

            Server.BaseEnter(base, client.GetValue());
            if (location)
            {
                Server.LocationEnter(location, client.GetValue());
            }
        }
        return { {} };
    }

    EquipDescVector existingEquips;
    ship->get_equip_desc_list(existingEquips);

    ushort highestId = 0;
    for (auto& equip : existingEquips.equip)
    {
        highestId = equip.id > highestId ? equip.id : highestId;

        if (equip.archId == good)
        {
            equip.count += count;
            return { {} };
        }
    }

    EquipDesc desc;
    desc.count = count;
    desc.archId = good;
    desc.health = 1.0f;
    desc.make_internal();
    desc.mounted = false;
    desc.id = highestId + 1;
    desc.mission = mission;

    using AddCargoItemT = bool(__thiscall*)(CEqObj * obj, const EquipDesc&);
    static auto addCargoItem = reinterpret_cast<AddCargoItemT>(GetProcAddress(GetModuleHandleA("common.dll"), "?add_cargo_item@CEqObj@@IAE_NABUEquipDesc@@@Z"));
    addCargoItem(ship.get(), desc);

    // TODO: Figure out how to communicate the change with BaseWatcher

    return { {} };
}

Action<void> ShipId::Relocate(const Vector& pos, const std::optional<Matrix>& orientation) const
{
    IsValidShip;

    const auto system = GetSystem().Unwrap();

    if (const auto player = GetPlayer().Raw(); player.has_value())
    {
        // We tell the client to adjust its position via a launch packet
        // TODO: This currently doesn't work, figure out how to make it work. We have successfully told clients to undock like this before...
        player->Undock(pos, orientation).Handle();
    }

    pub::SpaceObj::Relocate(ship->id, system.GetValue(), pos, orientation.value_or(Matrix::Identity()));
    return { {} };
}

Action<void> ShipId::IgniteFuse(uint fuseId, float id) const
{
    IsValidShip;

    IObjInspectImpl* inspect;

    if (!FLHook::GetObjInspect(ship->id, inspect))
    {
        return { cpp::fail(Error::InvalidObject) };
    }

    // ReSharper disable once CppTooWideScopeInitStatement
    bool success = false;
    __asm
    {
        push ecx
        lea eax, fuseId
        push 0 // this was previously a float called skip. It is unknown what it does.
        push id
        push 0 // SUBOBJ_Id_NONE
        push eax
        push 0 // this was previously a float called lifetime. It is unknown what it does.
        mov ecx, inspect
        mov eax, [ecx]
        call[eax + 0x1E4]
        mov success, al
        pop ecx
    }

    // TODO: Validate the above asm left the stack in the correct way
    // ReSharper disable quadrice CppDFAConstantConditions
    // ReSharper disable quadrice CppDFAUnreachableCode
    if (!success)
    {
        return { cpp::fail(Error::InvalidFuse) };
    }

    return { {} };
}

Action<void> ShipId::ExtinguishFuse(uint fuseId, float id) const
{
    IsValidShip;

    IObjInspectImpl* inspect;

    if (!FLHook::GetObjInspect(ship->id, inspect))
    {
        return { cpp::fail(Error::InvalidObject) };
    }

    // ReSharper disable once CppTooWideScopeInitStatement
    bool success = false;
    __asm
    {
        push ecx
        mov ecx, inspect
		lea eax, fuseId
		push id
		push 0 // SUBOBJ_Id_NONE
		push eax
		mov eax, [ecx]
		call [eax+0x1E8]
        mov success, al
        pop ecx
    }

    if (!success)
    {
        return { cpp::fail(Error::InvalidFuse) };
    }

    return { {} };
}

Action<CEquipManager*> ShipId::GetEquipmentManager() const
{
    IsValidShip;

    return { &ship->equipManager };
}
