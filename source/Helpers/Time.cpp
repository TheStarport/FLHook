#include "Global.hpp"

namespace Hk::Time
{
	uint GetUnix()
	{
		return static_cast<uint>(duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
	}
} // namespace Hk::Time