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
} // namespace Solar