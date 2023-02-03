#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::DeathPenalty
{
	struct CLIENT_DATA
	{
		bool bDisplayDPOnLaunch = true;
		uint DeathPenaltyCredits = 0;
	};

	//! Configurable fields for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "config/deathpenalty.json"; }

		// Reflectable fields
		float DeathPenaltyFraction = 0;
		float DeathPenaltyFractionKiller = 0;
		std::vector<std::string> ExcludedSystems = {};
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
