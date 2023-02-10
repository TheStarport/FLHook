#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::KillTracker
{
	//! Configurable fields for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "config/killtracker.json"; }

		// Reflectable fields
		bool enableNPCKillOutput = false;
		bool enableDamageTracking = false;
		std::wstring deathDamageTemplate = L"{0} took most hull damage from {1}, {2}%";
	};
	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		std::array<std::array<float, MaxClientId + 1>, MaxClientId + 1> damageArray;
		std::array<float, MaxClientId + 1> lastPlayerHealth;
		ReturnCode returncode = ReturnCode::Default;
	};
}
