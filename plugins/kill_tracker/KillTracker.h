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
		//! If enabled, broadcasts a system message announcing the kill counts at certain milestones
		bool enableMilestones = false;
		//! Message broadcasted systemwide upon ship's death if enableDamageTracking is true.
		//! {0} is replaced with victim's name, {1} with player who dealt the most damage to them,
		//! {2} with percentage of hull damage taken byt that player.
		std::wstring deathDamageTemplate = L"{0} took most hull damage from {1}, {2}%";
		//! Message broadcasted systemwide upon ship's death if enableMilstones is true.
		//! {0} corresponds to the killers name, {1} corresponds to the victims name,
		//! {2} corresponds to the kill count
		std::map<int, std::wstring> milestoneTemplate = {
		    {1, L"{0} has claimed first blood on {1}"}, {5, L"{0} is on a killing spree!"}, {10, L"{0} has claimed {2} kills!"}, {15, L"{0} has slain {1} for kill {2}"}};
	};
	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		std::array<std::array<float, MaxClientId + 1>, MaxClientId + 1> damageArray;
		std::array<float, MaxClientId + 1> lastPlayerHealth;
		ReturnCode returncode = ReturnCode::Default;
	};
} // namespace Plugins::KillTracker
