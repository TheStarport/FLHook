#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::KillCounter
{
	//! Configurable fields for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "config/killcounter.json"; }

		// Reflectable fields
		bool enableNPCKillOutput = false;
	};
	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
	};
}
