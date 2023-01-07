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
	}

	cpp::result<float, Error> getMass(uint spaceObjId) { 
		uint system;
		pub::SpaceObj::GetSystem(spaceObjId, system);
		if (!spaceObjId || !system)
		{
			return cpp::fail(Error::InvalidSpaceObjId);
		}
		float mass;
		pub::SpaceObj::GetMass(spaceObjId, mass);

		return mass;
	}

	cpp::result<std::pair<Vector, Vector>, Error> GetMotion(uint spaceObjId)
	{
		uint system;
		pub::SpaceObj::GetSystem(spaceObjId, system);
		if (!spaceObjId || !system)
		{
			return cpp::fail(Error::InvalidSpaceObjId);
		}
		Vector v1, v2;
		pub::SpaceObj::GetMotion(spaceObjId, v1, v2);
		return std::make_pair(v1, v2);
	}

	cpp::result<uint, Error> GetType(uint spaceObjId) {
		uint system;
		pub::SpaceObj::GetSystem(spaceObjId, system);
		if (!spaceObjId || !system)
		{
			return cpp::fail(Error::InvalidSpaceObjId);
		}
		uint type;
		pub::SpaceObj::GetType(spaceObjId, type);
		return type;
	}
} // namespace Solar