#include "Global.hpp"

namespace Hk::Time
{
	uint GetUnixSeconds()
	{
		return static_cast<uint>(duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}

	uint GetUnixMiliseconds()
	{
		return static_cast<uint>(duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}
} // namespace Hk::Time
