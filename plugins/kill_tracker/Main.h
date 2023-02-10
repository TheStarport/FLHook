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
		//! If enabled, lists all NPC ship types killed, along with kill counts.
		bool enableNPCKillOutput = false;
		//! If enabled, broadcasts a system message announcing the player with the biggest contribution to the kill.
		bool enableDamageTracking = false;
		//! Message broadcasted systemwide upon ship's death if enableDamageTracking is true.
		//! {0} is replaced with victim's name, {1} with player who dealt the most damage to them,
		//! {2} with percentage of hull damage taken byt that player.
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
