#pragma once

// Includes
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::AFK
{
	//! Global data for this plugin
	struct Global final
	{
		// Other fields
		ReturnCode returncode = ReturnCode::Default;

		std::set<uint> afks;
	};
}