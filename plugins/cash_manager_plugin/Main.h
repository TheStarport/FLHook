#pragma once

#include <FLHook.h>
#include <plugin.h>

namespace Plugins::CashManager
{
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/cash_manager.json"; }

		// The minimum transfer amount.
		int set_iMinTransfer = 0;

		// Transfers are not allowed to/from chars in this system.
		uint set_iBlockedSystem = 0;

		// Enable in dock cash cheat detection
		bool set_bCheatDetection = false;

		// Prohibit transfers if the character has not been online for at least this
		// time
		int set_iMinTime = 0;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		// Other fields
		ReturnCode returncode = ReturnCode::Default;
	};
}

