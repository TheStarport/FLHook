#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::KillTracker
{
	//! Struct to hold the Kill Streaks
	struct KillMessage final : Reflectable
	{
		uint number;
		std::wstring message;
		KillMessage(int number_, std::wstring message_)
		{
			number = number_;
			message = message_;
		}
		KillMessage() {}
	};

	//! Configurable fields for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "config/killtracker.json"; }

		// Reflectable fields
		//! If enabled, lists all NPC ship types killed, along with kill counts.
		bool enableNPCKillOutput = false;
		//! If enabled, broadcasts a system message announcing the player with the biggest contribution to the kill.
		bool enableDamageTracking = false;
		//! If enabled, broadcasts
		//! Message broadcasted systemwide upon ship's death if enableDamageTracking is true.
		//! {0} is replaced with victim's name, {1} with player who dealt the most damage to them,
		//! {2} with percentage of hull damage taken byt that player.
		std::wstring deathDamageTemplate = L"{0} took most hull damage from {1}, {2}%";
		//! Message broadcasted systemwide upon ship's death if template isn't empty.
		//! {0} corresponds to the killers name, {1} corresponds to the victims name,
		//! {2} corresponds to the kill count
		std::vector<KillMessage> killStreakTemplates = 
		{
		    KillMessage(5, L"{0} is on a killing spree!"), 
			KillMessage(10, L"{0} has claimed {2} kills!"), 
			KillMessage(15, L"{0} has slain {1} for kill {2}")
		};
		//! Message broadcasted systemwide when player reaches certain kill counts,
		//! if template isn't empty. {0} corresponds to said players name
		std::vector<KillMessage> milestoneTemplates = 
		{
			KillMessage(100, L"{0} has killed their 100th enemy!"),
		    KillMessage(1000, L"{0} has killed their 1,000th enemy!"),
		    KillMessage(10000, L"{0} has killed their 10,000th enemy!")
		};
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		std::array<std::array<float, MaxClientId + 1>, MaxClientId + 1> damageArray;
		std::array<float, MaxClientId + 1> lastPlayerHealth;
		ReturnCode returncode = ReturnCode::Default;
		std::map<ClientId, uint> killStreaks;
		std::map<int, std::wstring> killStreakTemplates;
		std::map<int, std::wstring> milestoneTemplates;
	};
} // namespace Plugins::KillTracker
