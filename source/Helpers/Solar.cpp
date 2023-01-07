#include "Global.hpp"

namespace Hk::Solar
{
	cpp::result<const SystemId, Error> GetSystemBySpaceId(uint spaceObjId)
	{
		uint system;
		pub::SpaceObj::GetSystem(spaceObjId, system);
		if (!system)
			return cpp::fail(Error::InvalidSystem);

		return system;
	}

	cpp::result<std::pair<Vector, Matrix>, Error> GetLocation(uint id, IdType type)
	{
		switch (type)
		{
			case IdType::Client: {
				uint ship = 0;
				pub::Player::GetShip(id, ship);
				if (!ship)
					return cpp::fail(Error::PlayerNotInSpace);
				id = ship;
			}
			[[fallthrough]];
			case IdType::Solar: {
				Vector pos;
				Matrix rot;
				pub::SpaceObj::GetLocation(id, pos, rot);
				return std::make_pair(pos, rot);
				break;
			}
			default:
				return cpp::fail(Error::InvalidIdType);
		}

		// return Hk::Math::VectorToSectorCoord<std::wstring>(iSystemId, pos);
	}
} // namespace Solar