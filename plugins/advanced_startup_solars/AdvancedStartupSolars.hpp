#pragma once

#include <FLHook.hpp>
#include <plugin.h>
#include <ranges>
#include "../npc_control/NPCControl.h"
#include "../solar_control/SolarControl.h"

namespace Plugins::AdvancedStartupSolars
{

	//! Additional data for formations to spawn from solarFamilies
	struct SolarFormation final : Reflectable
	{
		//! The formation name from solar.json to use
		std::wstring formation;
		//! NPCs from npc.json and the number to spawn
		std::map<std::wstring, int> npcs;
		//! The weight for this formation to spawn within the SolarFamily
		int spawnWeight = 0;
	};

	//! Positional data for SolarFamily
	struct Position final : Reflectable
	{
		std::vector<float> location;
		std::string system;
	};

	//! A SolarFamily struct that contains grouped formations and potential spawn locations
	struct SolarFamily final : Reflectable
	{
		//! The name of the SolarFamily. Principally used for debug messages
		std::string name;
		//! A vector containing possible solarFormations to spawn
		std::vector<SolarFormation> solarFormations;
		//! A vector containing possible positions to spawn solarFormations at
		std::vector<Position> spawnLocations;
		//! The overall spawn chance for the whole SolarFamily
		int spawnChance = 0;
		//! The number of formations to spawn within the family
		int spawnQuantity = 0;
	};

	struct Config final : Reflectable
	{
		std::string File() override { return "config/advanced_startup_solars.json"; }
		//! A vector containing overal SolarFamily groups
		std::vector<SolarFamily> solarFamilies;
	};

	//! Global data for this plugin
	struct Global
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		Plugins::Npc::NpcCommunicator* npcCommunicator = nullptr;
		Plugins::SolarControl::SolarCommunicator* solarCommunicator = nullptr;
		bool pluginActive = true;
		bool firstRun = true;
	};
} // namespace Plugins::AdvancedStartupSolars