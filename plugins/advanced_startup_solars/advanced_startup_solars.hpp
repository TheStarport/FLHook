#pragma once

#include <FLHook.hpp>
#include <plugin.h>
#include <ranges>
#include "../npc_control/NPCControl.h"
#include "../solar_control/SolarControl.h"

namespace Plugins::AdvancedStartupSolars
{

	struct SolarFormation final : Reflectable
	{
		std::string formation;
		std::map<std::wstring, int> npcs;
		int spawnWeight = 0;
	};

	// Struct for position as vectors are not supported as reflectables in serializer.hpp
	struct Position final : Reflectable
	{
		float x;
		float y;
		float z;
	};

	struct SolarFamily final : Reflectable
	{
		std::string name;
		std::vector<SolarFormation> solarFormations;
		std::vector<Position> spawnLocations;
		int spawnChance = 0;
		int spawnQuantity = 0;
	};

	// Loadable json configuration
	struct Config final : Reflectable
	{
		std::string File() override { return "config/advanced_startup_solars.json"; }
		std::vector<SolarFamily> solarFamilies;
	};

	struct Global
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		Plugins::Npc::NpcCommunicator* npcCommunicator = nullptr;
		Plugins::SolarControl::SolarCommunicator* solarCommunicator = nullptr;
		bool pluginActive = true;
	};
} // namespace Plugins::AdvancedStartupSolars