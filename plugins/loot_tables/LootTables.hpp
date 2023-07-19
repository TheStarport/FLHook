#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::LootTables
{
	// Loadable json configuration
	struct Config final : Reflectable
	{
		std::string File() override { return "config/loottables.json"; }
		bool overrideUserNumber = false;
		// Change according to dummy item, here to test:
		// missile01_mark01_rtc
		uint iIdSNameToCheck = 263146;
		// Test bool for me to try something
		bool test_bool = true;
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
	};
} // namespace Plugins::Template