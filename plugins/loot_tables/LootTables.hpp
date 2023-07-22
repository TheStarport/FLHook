#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::LootTables
{
	struct LootTable final : Reflectable
	{
		// Parameters
		bool Players;
		bool NPCs;
		std::string Item;
		std::map<float, std::string> Probabilities;
		float NoDropChance;

		// Constructors
		LootTable(bool PlayersBool, bool NPCsBool, std::string TriggerItem, std::map<float, std::string> DropProbabilities);
		LootTable() = default;
	};

	// Loadable json configuration
	struct Config final : Reflectable
	{
		std::string File() override { return "config/loottables.json"; }

		// Parameters for default loot tables
		bool TrackPlayers_1 = true;
		bool TrackNPCs_1 = false;
		std::string DefaultItem_1 = "missile01_mark01_rtc";
		std::map<float, std::string> DefaultProbabilities_1 = 
		{
			{0.5, "missile01_mark01_rtc_ammo"}, 
			{0.25, "missile01_mark01"}, 
			{0.1, "missile01_mark01_ammo"}
		};
		bool TrackPlayers_2 = false;
		bool TrackNPCs_2 = true;
		std::string DefaultItem_2 = "missile01_mark02";
		std::map<float, std::string> DefaultProbabilities_2 =
		{
			{0.5, "missile01_mark02_ammo"}, 
			{0.5, "missile01_mark03"}
		};

		// Example loot tables
		std::vector<LootTable> ExampleLootTables = {
		    LootTable(TrackPlayers_1, TrackNPCs_1, DefaultItem_1, DefaultProbabilities_1), 
			LootTable(TrackPlayers_2, TrackNPCs_2, DefaultItem_2, DefaultProbabilities_2)
		};
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
	};
} // namespace Plugins::Template