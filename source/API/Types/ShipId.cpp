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
};
