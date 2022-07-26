#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::KillCounter
{
	//! Global data for this plugin
	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;
	};
}
