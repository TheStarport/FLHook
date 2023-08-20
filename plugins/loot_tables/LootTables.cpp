/**
 * @date 23/07/2023
 * @author Shiniri
 * @defgroup LootTables Loot Tables
 * @brief
 * This plugin implements functionality which allows for more control over what items get
 * dropped when a ship is destroyed, and over their drop probabilities.
 *
 * @paragraph cmds Player Commands
 * There are no cmds player commands in this plugin.
 * 
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * No configuration file is needed.
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin depends on <random>
 */

// Includes
#include "LootTables.hpp"

namespace Plugins::LootTables
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup LootTables
	* @brief Checks if a certain item is on board a ship. (Potentially replaced in future)
	* For now this also only works for commodities!
	*/
	bool CheckForItem(CShip* ship, const uint triggerItemHashed)
	{
		CEquipTraverser traverser(UINT_MAX);
		CEquip const* equip = GetEquipManager((ship))->Traverse(traverser);
		while (equip)
		{
			EquipDesc equipDesc;
			equip->GetEquipDesc(equipDesc);

			if (equipDesc.iArchId == triggerItemHashed)
			{
				return true;
			}

			equip = GetEquipManager((ship))->Traverse(traverser);
		}
		return false;
	}

	/** @ingroup LootTables
	 * @brief Hook on ShipDestroyed. Applies loot table if possible, drops one of the items from the table at random.
	 */
	void ShipDestroyed([[maybe_unused]] DamageList** dmgList, const DWORD** ecx, [[maybe_unused]] const uint& kill)
	{
		// Get cShip from NPC?
		CShip* ship = Hk::Player::CShipFromShipDestroyed(ecx);
		for (auto const& lootTable : global->config->lootTables)
		{
			// Check if the killed Ship has an Item on board, which would trigger the loot table
			if (!CheckForItem(ship, lootTable.triggerItemHashed))
			{
				// Drop nothing
				return;
			}

			// Check if the Loot Table in question applies to the destroyed ship
			if (const bool isPlayer = ship->is_player(); !((isPlayer && lootTable.applyToPlayers) || (!isPlayer && lootTable.applyToNpcs)))
			{
				// Drop nothing
				return;
			}

			// Calculate what Item to drop
			std::random_device randomDevice; // Used to obtain a seed
			std::mt19937 mersenneTwisterEngine(randomDevice()); //  Mersenne Twister algorithm seeded with the variable above
			std::uniform_real_distribution dist(0.0f, 1.0f);
			const float randomFloat = dist(mersenneTwisterEngine);
			float sum = 0.0;
			for (const auto& [weight, itemHashed, item] : lootTable.dropWeights)
			{
				sum += weight;
				if (randomFloat <= sum && itemHashed)
				{
     					Server.MineAsteroid(
					    ship->iSystem, 
						ship->get_position(), 
						global->config->lootDropContainerHashed, 
						itemHashed, 
						lootTable.dropCount, 
						ship->GetOwnerPlayer());
					return;
				}
			}
		}
	}

	/** @ingroup KillTracker
	 * @brief LoadSettings hook. Loads/generates config file
	 */
	void LoadSettings()
	{
		// Load JSON config
		auto config = Serializer::JsonToObject<Config>();

		// Hash nicknames
		config.lootDropContainerHashed = CreateID(config.lootDropContainer.c_str());

		for (auto& lootTable : config.lootTables)
		{
			lootTable.triggerItemHashed = CreateID(lootTable.triggerItem.c_str());

			// Check that weighting in this loot table entry add up to 1
			float weightingCheck = 0;
			for (auto& weighting : lootTable.dropWeights)
			{
				weightingCheck += weighting.weighting;
				if (ToLower(weighting.item) != "none")
				{
					weighting.itemHashed = CreateID(weighting.item.c_str());
				}
				
			}

			if (abs(weightingCheck - 1.0f) > 1e-9) // Can't just use != due to floating point precision
			{
				Console::ConErr(std::format("Loot Table for trigger item: {} does not have weightings that add up to exactly 1.", lootTable.triggerItem));
			}
		}

		global->config = std::make_unique<Config>(std::move(config));
	}
} // namespace Plugins::LootTables

using namespace Plugins::LootTables;

REFL_AUTO(type(DropWeight), field(weighting), field(item));
REFL_AUTO(type(LootTable), field(dropCount), field(applyToPlayers), field(applyToNpcs), field(triggerItem), field(dropWeights));
REFL_AUTO(type(Config), field(lootDropContainer), field(lootTables));

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Loot Tables");
	pi->shortName("loottables");
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
}