
// Includes
#include "LootTables.hpp"

#include <random>

namespace Plugins::LootTables
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void ItemDrop(const CShip* cShip, std::string const * itemNickname, uint dropCount)
	{
		const Vector deathPosition = cShip->get_position();
		auto itemArchId = CreateID((*itemNickname).c_str());
		auto lootCrateId = CreateID(global->lootDropContainer.c_str());
		auto deathSystem = cShip->iSystem;
		ClientId client = cShip->GetOwnerPlayer();
		Server.MineAsteroid(deathSystem, deathPosition, lootCrateId, itemArchId, dropCount, client);
	}

	// This is temporarily used to fetch equipement, it will have to be redone once FLHook functionality
	// is updated accordingly. (Hopefully)
	bool CheckForItem(const CShip* cShip, std::string itemNickname)
	{
		CEquipManager* eqManager = GetEquipManager((CEqObj*)cShip);
		CEquipTraverser tr(65536);
		while (const CEquip* equipment = eqManager->Traverse(tr))
		{
			const GoodInfo* goodInfo = GoodList_get()->find_by_archetype(equipment->archetype->get_id());
			if (CreateID(itemNickname.c_str()) == goodInfo->iArchId)
			{
				return true;
			}
		}
		return false;
	}

	// Hook on ship destroyed
	void ShipDestroyed([[maybe_unused]] DamageList** dmgList, const DWORD** ecx, [[maybe_unused]] const uint& kill)
	{
		// Get cShip from NPC?
		const CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);
		for (uint i = 0; i <= global->exampleLootTables.size(); i++)
		{
			LootTable currentLootTable = global->exampleLootTables[i];

			// Check if the killed Ship has an Item on board, which would trigger the loot table
			if (!CheckForItem(cShip, currentLootTable.triggerItemNickname))
			{
				return;
			}

			// Check if the Loot Table in question applies to the destroyed ship
			bool isPlayer = cShip->is_player();
			if (!((isPlayer == currentLootTable.applyToPlayers) || (isPlayer == currentLootTable.applyToNpcs)))
			{
				// LootTable does not apply, drop nothing
				return;
			}

			// RNG
			auto lootTableSize = currentLootTable.dropWeights.size();
			std::vector<float> probabilitiesFromWeights;
			for (const auto& [weight,nickname]: currentLootTable.dropWeights)
			{
				probabilitiesFromWeights.push_back((float)weight / (float)lootTableSize);
			}

			// Calculate what Item to drop
			float randomFloat = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			float Sum = 0.0;
			int correspondingVectorIndex = 0;
			for (const auto& [weight, nickname] : currentLootTable.dropWeights)
			{
				Sum += probabilitiesFromWeights[correspondingVectorIndex];
				if (randomFloat <= Sum && (nickname != "None"))
				{
					std::string itemToDropNickname = nickname;
					ItemDrop(cShip, &itemToDropNickname, currentLootTable.dropCount);
					return;
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
} // namespace Plugins::LootTables

using namespace Plugins::LootTables;

REFL_AUTO(type(LootTable), field(dropCount), field(applyToPlayers), field(applyToNpcs), field(triggerItemNickname), field(dropWeights));
REFL_AUTO(type(Config), field(lootDropContainer), field(exampleLootTables));

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