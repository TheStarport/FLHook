#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::DeathPenalty
{
	struct CLIENT_DATA
	{
		bool displayDPOnLaunch = true;
		uint deathPenaltyCredits = 0;
	};

	//! Configurable fields for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "config/deathpenalty.json"; }

		// Reflectable fields
		//! Percentage of player's worth deducted upon death, where 1.0f stands for all of his worth.
		float DeathPenaltyFraction = 0.0f;
		//! Percentage of death penalty transferred to the killer, where 1.0f means the killer gets as much as the victim lost.
		float DeathPenaltyFractionKiller = 0.0f;
		//! List of system where death penalty/kill reward is disabled in.
		std::vector<std::string> ExcludedSystems = {};
		//! Map of ship archetypes to a penalty multiplier.
		//! For example, {li_elite, 2} causes Defenders to lose twice the amount compared to unlisted ships on death.
		std::map<std::string, float> FractionOverridesByShip = {};
	};

	struct Global final
	{
		std::vector<uint> ExcludedSystemsIds;
		std::map<uint, float> FractionOverridesByShipIds;

		std::map<uint, CLIENT_DATA> MapClients;

		std::unique_ptr<Config> config = nullptr;

		ReturnCode returncode = ReturnCode::Default;
	};
} // namespace Plugins::DeathPenalty
