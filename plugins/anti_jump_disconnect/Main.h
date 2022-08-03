#pragma once

#include "../tempban/Main.h"

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::AntiJumpDisconnect
{
	//! Global data for this plugin
	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;

		//! This plugin communicates with the Temp Ban plugin
		Tempban::TempBanCommunicator* tempBanCommunicator = nullptr;

		struct INFO
		{
			bool bInWrapGate;
		};

		//! Map of client ids and whether they are in a gate or not
		std::map<uint, INFO> mapInfo;
	};
} // namespace Plugins::AntiJumpDisconnect