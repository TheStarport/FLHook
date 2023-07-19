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
		std::wstring Item;
		std::map<std::wstring, float> Probabilities;
		float NoDropChance;

		// Constructors
		LootTable(bool PlayersBool, bool NPCsBool, std::wstring TriggerItem, std::map<std::wstring, float> DropProbabilities);
		LootTable() = default;
	};

	// Loadable json configuration
	struct Config final : Reflectable
	{
		std::string File() override { return "config/loottables.json"; }

		// Parameters for default loot tables
		std::wstring DefaultItem_1 = L"missile01_mark01_rtc";
		std::map<std::wstring, float> DefaultProbabilities_1 = 
		{
		    {L"missile01_mark01_rtc_ammo", 0.5},
			{L"missile01_mark01", 0.25},
			{L"missile01_mark01_ammo", 0.1}
		};
		std::wstring DefaultItem_2 = L"missile01_mark02";
		std::map<std::wstring, float> DefaultProbabilities_2 =
		{
			{L"missile01_mark02_ammo", 0.5},
			{L"missile01_mark03", 0.5}
		};

		// Example loot tables
		std::vector<LootTable> ExampleLootTables = {
		    LootTable(true, false, DefaultItem_1, DefaultProbabilities_1), 
			LootTable(false, true, DefaultItem_2, DefaultProbabilities_2)
		};
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
	};
} // namespace Plugins::Template