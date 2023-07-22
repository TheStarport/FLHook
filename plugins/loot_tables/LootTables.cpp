
/*
*	Temporary Notes:
*		. ask for sensible default behaviour for config probabilities >100%
*		. find out why its still creating this weird Loot Tables dll
*		. How item drop? (Get location, why mine asteroid?)
*		. cShip required, get from NPC?
*/

// Includes
#include "LootTables.hpp"

#include <random>

namespace Plugins::LootTables
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void ItemDrop(const CShip* ship, std::string ItemNickname)
	{
		// Placeholder
	}

	// This is temporarily used to fetch equipement, it will have to be redone once FLHook functionality
	// is updated accordingly. (Hopefully)
	bool CheckForItem(const CShip* ship, std::string ItemNickname)
	{
		CEquipManager* eqManager = GetEquipManager((CEqObj*)ship);
		CEquipTraverser tr(65536);
		while (CEquip* eq = eqManager->Traverse(tr))
		{
			const GoodInfo* gi = GoodList_get()->find_by_archetype(eq->archetype->get_id());
			if (CreateID(ItemNickname.c_str()) == gi->iArchId)
			{
				return true;
			}
		}
		return false;
	}

	// Hook on ship destroyed
	void ShipDestroyed(DamageList** dmgList, const DWORD** ecx, const uint& kill)
	{
		// Get cShip from NPC?
		const CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);
		for (int i = 0; i <= global->config->ExampleLootTables.size(); i++)
		{
			LootTable CurrentLootTable = global->config->ExampleLootTables[i];

			// Check if the Loot Table in question applies to the destroyed ship
			bool IsPlayer = cShip->is_player();
			bool LootTableApplies = ((IsPlayer == CurrentLootTable.Players) || (IsPlayer == CurrentLootTable.NPCs));
			if (!LootTableApplies)
			{
				// LootTable does not apply, drop nothing
				return;
			}

			// RNG
			float AccumulatedProbability = (1.0 - CurrentLootTable.NoDropChance);
			float RandomFloat = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);

			if (RandomFloat <= CurrentLootTable.NoDropChance)
			{
				// Drop nothing
				return;
			}

			// Calculate what Item to drop
			float Sum = (0.0 + CurrentLootTable.NoDropChance);
			for (auto Item : CurrentLootTable.Probabilities)
			{
				Sum += Item.first;
				if (RandomFloat <= Sum)
				{
					// Placeholder
				}
			}
		}
	}

	// Put things that are performed on plugin load here!
	void LoadSettings()
	{
		// Load JSON config
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(std::move(config));
	}

	LootTable::LootTable(bool PlayersBool, bool NPCsBool, std::string TriggerItem, std::map<float, std::string> DropProbabilities)
	{
		this->Players = PlayersBool;
		this->NPCs = NPCsBool;
		this->Item = TriggerItem;

		// Check if probabilities given to Constructor make sense
		// if sum of probabilities >100% [insert behaviour]
		// if sum of probabilities <100% chance to drop nothing
		float ProbabilitySum = 0;
		for (auto it = DropProbabilities.begin(); it != DropProbabilities.end(); it++)
		{
			ProbabilitySum += it->first;
		}
		if (ProbabilitySum <= 1.0)
		{
			this->Probabilities = DropProbabilities;
			this->NoDropChance = (1.0 - ProbabilitySum);
		}
		else
		{
			// Placeholder
		}
	}

} // namespace Plugins::LootTables

using namespace Plugins::LootTables;

REFL_AUTO(type(LootTable), field(Players), field(NPCs), field(Item), field(Probabilities), field(NoDropChance));
REFL_AUTO(type(Config), field(ExampleLootTables));

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