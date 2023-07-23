#pragma once

#include <FLHook.hpp>
#include <plugin.h>

#include <random>

namespace Plugins::LootTables
{
	struct DropWeight final : Reflectable
	{
		uint weighting = 0;
		std::string item;

		DropWeight(uint weightingParam, std::string const& itemParam) : weighting(weightingParam), item(itemParam) {}
		DropWeight() = default;
	};

	struct LootTable final : Reflectable
	{
		// Parameters
		uint dropCount = 0;
		bool applyToPlayers = false;
		bool applyToNpcs = false;
		std::string triggerItemNickname;
		std::vector<DropWeight> dropWeights;

		// Constructors
		LootTable(uint dropCountParam,
			bool applyToPlayersParam,
			bool applyToNpcsParam, 
			std::string const& triggerItemNicknameParam,
		    std::vector<DropWeight> dropWeightsParam)
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

		// Loot Tables
		// Set the item corresponding to a weight to "None" if there should be a chance for nothing to be dropped
		std::vector<LootTable> lootTables = 
		{
			LootTable(1, true, false, "missile01_mark01_rtc", { DropWeight(4, "missile01_mark01_rtc_ammo"), DropWeight(2, "missile01_mark01"), DropWeight(2, "missile01_mark01_ammo")}),
		    LootTable(2, false, true, "missile01_mark02", { DropWeight(1, "missile01_mark02_ammo"), DropWeight(1, "missile01_mark03")})
		};
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
	};
} // namespace Plugins::Template