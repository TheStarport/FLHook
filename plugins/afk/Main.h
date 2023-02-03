#pragma once

// Includes
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Afk
{
	//! Global data for this plugin
	struct Global final
	{
		// Other fields
		ReturnCode returnCode = ReturnCode::Default;

		std::vector<uint> awayClients;
	};
} // namespace Plugins::Afk