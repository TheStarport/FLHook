#pragma once

#include <FLHook.hpp>
#include <plugin.h>
#include <random>

namespace Plugins::LootTables
{
	struct DropWeight final : Reflectable
	{
		uint weighting = 2; // Weights are relative, meaning to compute drop likelihood, do w_i / sum(w_1, w_2, ...)
		uint itemHashed = 0;
		std::string item = "missile01_mark02_ammo";
		uint dropCount = 3;
	};

	struct LootTable final : Reflectable
	{
		uint rollCount = 5;
		bool applyToPlayers = false;
		bool applyToNpcs = false;
		std::string triggerItem = "missile01_mark02";
		uint triggerItemHashed = 0;
		std::vector<DropWeight> dropWeights = { DropWeight() };
	};

	// Loadable json configuration
	struct Config final : Reflectable
	{
		// What json file to use / write to
		std::string File() override { return "config/loottables.json"; }

		// Crate to use for loot
		std::string lootDropContainer = "lootcrate_ast_loot_metal";
		uint lootDropContainerHashed = 0;

		// Loot Tables
		std::vector<LootTable> lootTables = { LootTable() };
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
	};
} // namespace Plugins::Template