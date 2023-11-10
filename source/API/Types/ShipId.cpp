#include "PCH.hpp"

#include "API/Types/ShipId.hpp"

Action<CShipPtr, Error> ShipId::GetCShip(bool increment) const
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
    auto ship = GetCShip(false);
    if (ship.Raw().has_error())
    {
        return { cpp::fail(Error::InvalidShip) };
    }

    return { reinterpret_cast<Archetype::Ship*>(ship.Raw().value()->get_archetype()) };
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
    auto ship = GetCShip(false);
    if (ship.Raw().has_error())
    {
        return { cpp::fail(Error::InvalidShip) };
    }

    auto& shipVal = ship.Raw().value();
    // TODO: Getting Shield value seems a lot more involved and will likely require crawling ship parts or equipment can't remember which.

    EquipDescVector equip;
    shipVal->get_equip_desc_list(equip);
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
    auto target = shipVal->get_target()->ship;
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

Action<void, Error> ShipId::AddCargo(std::wstring good, int count, bool mission)
{
    auto ship = GetCShip(false);
    if (ship.Raw().has_error())
    {
        return { cpp::fail(Error::InvalidShip) };
    }
    auto& shipVal = ship.Raw().value();


    const auto goodStr = StringUtils::wstos(good);

    const GoodInfo* goodInfo;
    if (!(goodInfo = GoodList::find_by_nickname(goodStr.c_str())))
    {
        return { cpp::fail(Error::InvalidGood) };
    }
    bool multiCount;

    //TODO:@Laz Magic number here, need to document goodInfo Offsets.
    memcpy(&multiCount, (char*)goodInfo + 0x70, 1);

   //TODO: We don't seem to have any player-agnostic way of adding cargo, every example is assuming the ship is a player.
}