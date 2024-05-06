#include "PCH.hpp"

#include "API/Types/ShipId.hpp"

#include <glm/gtc/quaternion.hpp>

Action<CShipPtr, Error> ShipId::GetCShip(bool increment)
{
    auto ship = CShipPtr(value);
    if (!ship)
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    return { CShipPtr(ship.get(), increment) };
}

Action<Archetype::Ship*, Error> ShipId::GetShipArchetype()
{
    uint archId;
    if (pub::SpaceObj::GetArchetypeID(value, archId) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    return { Archetype::GetShip(archId) };
}

// If provided a value of true in the argument it will provide a percentage value of the ship's health, with 0.0f being no hp and 1.0f being full health.
// Otherwise simply gives the health as an exact value.
Action<float, Error> ShipId::GetHealth(bool percentage)
{
    auto ship = GetCShip(false);
    if (ship.Raw().has_error())
    {
        return { cpp::fail(Error::InvalidShip) };
    }

    auto& shipVal = ship.Raw().value();

    if (!percentage)
    {
        return { shipVal->hitPoints };
    }

    // Health on the ship is the current health on the ship, while the shipArch is the max Health.
    return { shipVal->hitPoints / shipVal->shiparch()->hitPoints };
}

Action<float, Error> ShipId::GetShields(bool percentage)
{
    float currentShield, maxShield;
    bool shieldUp;
    if (pub::SpaceObj::GetShieldHealth(value, currentShield, maxShield, shieldUp) != static_cast<int>(ResponseCode::Success))
    {
        return { cpp::fail(Error::InvalidSpaceObjId) };
    }

    if (!shieldUp)
    {
        return { 0.0f };
    }

    if (!percentage)
    {
        return { currentShield };
    }

    return { currentShield / maxShield };
}

std::optional<ClientId> ShipId::GetPlayer()
{
    auto ship = GetCShip(false);
    if (ship.Raw().has_error())
    {
        return {};
    }

    if (const auto res = ship.Unwrap(); res->is_player())
    {
        return ClientId(res->ownerPlayer);
    }

    return {};
}

std::optional<ShipId> ShipId::GetTarget()
{
    auto ship = GetCShip(false);
    if (ship.Raw().has_error())
    {
        return {};
    }

    auto& shipVal = ship.Raw().value();

    // Returns IObjectRW*, whatever that is.
    const auto target = shipVal->get_target();
    return std::optional(ShipId(target->get_id()));
}

Action<RepId, Error> ShipId::GetReputation()
{
    auto repId = RepId(*this, false);
    if (!repId)
    {
        return { cpp::fail(Error::InvalidShip) };
    }
    return { repId };
}

Action<float, Error> ShipId::GetSpeed()
{
    auto ship = GetCShip(false);
    if (ship.Raw().has_error())
    {
        return { cpp::fail(Error::InvalidShip) };
    }

    auto& shipVal = ship.Raw().value();

    return { glm::length<3, float, glm::highp>(shipVal->get_velocity()) };
}

bool ShipId::IsPlayer()
{
    auto ship = GetCShip(false);
    auto& shipVal = ship.Raw().value();
    return shipVal->is_player();
}

bool ShipId::IsNpc() { return !IsPlayer(); }

bool ShipId::IsInTradeLane()
{
    auto ship = GetCShip(false);
    auto& shipVal = ship.Raw().value();
    return shipVal->is_using_tradelane();
}

void ShipId::Destroy(DestroyType type) { pub::SpaceObj::Destroy(value, type); }

Action<void, Error> ShipId::SetHealth(float amount, bool percentage)
{
    // If provided an invalid percantage along with requesting a percentage scale it will simply fail.
    if ((amount < 0.0f || amount > 1.0f) && percentage == true)
    {
        return { cpp::fail(Error::UnknownError) };
    }
    auto ship = GetCShip(false);
    if (ship.Raw().has_error())
    {
        return { cpp::fail(Error::InvalidShip) };
    }
    auto& shipVal = ship.Raw().value();

    if (percentage == false)
    {
        shipVal->set_hit_pts(amount);
        return { {} };
    }

    auto maxHp = shipVal->get_archetype()->hitPoints;
    shipVal->set_hit_pts(amount * maxHp);
    return { {} };
}

Action<void, Error> ShipId::AddCargo(uint good, uint count, bool mission)
{
    const auto goodInfo = GoodList::find_by_id(good);
    if (!goodInfo)
    {
        return { cpp::fail(Error::InvalidGood) };
    }

    auto ship = GetCShip(false);
    if (ship.Raw().has_error())
    {
        return { cpp::fail(Error::InvalidShip) };
    }

    auto shipVal = ship.Raw().value();

    if (shipVal->is_player())
    {
        auto client = ClientId(shipVal->ownerPlayer);

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
                return { cpp::fail(Error::PlayerNotLoggedIn) };
            }
        }

        if (goodInfo->multiCount)
        {
            // it's a good that can have multiple units(commodities missile ammo, etc)
            int ret;

            // we need to do this, else server or client may crash
            for (const auto cargo = client.EnumCargo(ret).Raw(); auto& item : cargo.value())
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
    shipVal->get_equip_desc_list(existingEquips);

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
    addCargoItem(shipVal.get(), desc);

    // TODO: Figure out how to communicate the change with BaseWatcher

    return { {} };
}

void ShipId::Relocate(const Vector& pos, const std::optional<Matrix>& orientation, const bool sendPlayerLaunchPacket)
{
    if (sendPlayerLaunchPacket)
    {
        const auto client = GetPlayer().value_or(ClientId(0));
        const Quaternion rotation = Quaternion(orientation.value_or(Matrix::Identity()));

        FLPACKET_LAUNCH launchPacket;
        launchPacket.ship = value;
        launchPacket.base = 0;
        launchPacket.state = 0xFFFFFFFF;
        launchPacket.rotate[0] = rotation.w;
        launchPacket.rotate[1] = rotation.x;
        launchPacket.rotate[2] = rotation.y;
        launchPacket.rotate[3] = rotation.z;
        launchPacket.pos[0] = pos.x;
        launchPacket.pos[1] = pos.y;
        launchPacket.pos[2] = pos.z;

        FLHook::hookClientImpl->Send_FLPACKET_SERVER_LAUNCH(client.GetValue(), launchPacket);
    }

    const auto system = GetSystem().Unwrap();
    pub::SpaceObj::Relocate(value, system.GetValue(), pos, orientation.value_or(Matrix::Identity()));
}

Action<void, Error> ShipId::IgniteFuse(uint fuseId, float id) const
{
    auto tempVal = value;
    IObjInspectImpl* inspect;

    if (uint _; !FLHook::GetShipInspect(tempVal, inspect, _))
    {
        return { cpp::fail(Error::InvalidShip) };
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
    if (!success)
    {
        return { cpp::fail(Error::InvalidInput) };
    }

    return { {} };
}

Action<void, Error> ShipId::ExtinguishFuse(uint fuseId, float id) const
{
    auto tempVal = value;
    IObjInspectImpl* inspect;

    if (uint _; !FLHook::GetShipInspect(tempVal, inspect, _))
    {
        return { cpp::fail(Error::InvalidShip) };
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

    // TODO: Validate the above asm left the stack in the correct way
    if (!success)
    {
        return { cpp::fail(Error::InvalidInput) };
    }

    return { {} };
}
