#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::LootTables
{
	struct LootTable final : Reflectable
	{
		// Parameters
		uint dropCount;
		bool applyToPlayers;
		bool applyToNpcs;
		std::string triggerItemNickname;
		std::map<uint, std::string> dropWeights;

		// Constructors
		LootTable(uint dropCountParam,
			bool applyToPlayersParam,
			bool applyToNpcsParam, 
			std::string triggerItemNicknameParam,
		    std::map<uint, std::string> dropWeightsParam)
		    : dropCount(dropCountParam), 
			applyToPlayers(applyToPlayersParam),
			applyToNpcs(applyToNpcsParam), 
			triggerItemNickname(triggerItemNicknameParam),
		    dropWeights(dropWeightsParam) {}
		LootTable() = default;
	};

	// Loadable json configuration
	struct Config final : Reflectable
	{
		std::string File() override { return "config/loottables.json"; }

		// Crate to use for loot
		std::string lootDropContainer = "lootcrate_ast_loot_metal";

		// Parameters for default loot tables
		uint dropCount_1 = 1;
		bool applyToPlayers_1 = true;
		bool applyToNPCs_1 = false;
		std::string defaultItem_1 = "missile01_mark01_rtc";
		std::map<uint, std::string> defaultWeights_1 = 
		{
			{4, "missile01_mark01_rtc_ammo"}, 
			{2, "missile01_mark01"}, 
			{2, "missile01_mark01_ammo"}
		};
		uint dropCount_2 = 2;
		bool applyToPlayers_2 = false;
		bool applyToNPCs_2 = true;
		std::string defaultItem_2 = "missile01_mark02";
		std::map<uint, std::string> defaultWeights_2 =
		{
			{1, "missile01_mark02_ammo"}, 
			{1, "missile01_mark03"}
		};

		// Example loot tables
		std::vector<LootTable> exampleLootTables = 
		{
			LootTable(dropCount_1, applyToPlayers_1, applyToNPCs_1, defaultItem_1, defaultWeights_1),
		    LootTable(dropCount_2, applyToPlayers_2, applyToNPCs_2, defaultItem_2, defaultWeights_2)
		};
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
		std::string lootDropContainer;
		std::vector<LootTable> exampleLootTables;
	};
} // namespace Plugins::Template