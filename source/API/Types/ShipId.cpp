#include "PCH.hpp"

#include "API/Types/ShipId.hpp"

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
    if (pub::SpaceObj::GetArchetypeID(value, archId) != (int)ResponseCode::Success)
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
    if (pub::SpaceObj::GetShieldHealth(value, currentShield, maxShield, shieldUp) != (int)ResponseCode::Success)
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
    const auto target = shipVal->get_target()->ship;
    return ShipId(target->get_id());
}

Action<RepId, Error> ShipId::GetReputation() {}

std::wstring ShipId::GetAffiliation() {}

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
    auto& shipVal = ship.Raw().value();

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
    desc.set_hardpoint(CacheString(EquipDesc::CARGO_BAY_HP_NAME));
    desc.mounted = false;
    desc.id = highestId + 1;
    desc.mission = mission;

    shipVal->add_cargo_item(desc);

    return { {} };
}
