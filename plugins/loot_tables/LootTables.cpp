
// Includes
#include "LootTables.hpp"

#include <random>

namespace Plugins::LootTables
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	// This is temporarily used to fetch equipement, it will have to be redone once FLHook functionality
	// is updated accordingly. (Hopefully)
	bool CheckForItem(const CShip* cShip, const std::string &itemNickname)
	{
		CEquipManager* eqManager = GetEquipManager((CEqObj*)cShip);
		CEquipTraverser tr(65536);
		while (const CEquip* equipment = eqManager->Traverse(tr))
		{
			const GoodInfo* goodInfo = GoodList_get()->find_by_archetype(equipment->archetype->get_id());
			if (CreateID(itemNickname.c_str()) == goodInfo->iArchId)
			{
				const std::wstring message1 = L"Match!";
				Hk::Message::MsgU(message1);
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
		for (auto const& lootTable : global->config->lootTables)
		{
			// Check if the killed Ship has an Item on board, which would trigger the loot table
			if (!CheckForItem(cShip, lootTable.triggerItemNickname))
			{
				// Drop nothing
				return;
			}

			// Check if the Loot Table in question applies to the destroyed ship
			if (bool isPlayer = cShip->is_player(); !((isPlayer && lootTable.applyToPlayers) || (!isPlayer && lootTable.applyToNpcs)))
			{
				// Drop nothing
				return;
			}

			// RNG
			std::vector<float> probabilitiesFromWeights;
			for (const auto& [weight,nickname]: lootTable.dropWeights)
			{
				probabilitiesFromWeights.push_back((static_cast<float>(weight) / static_cast<float>(lootTable.dropWeights.size())));
			}

			// Calculate what Item to drop
			float randomFloat = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
			float sum = 0.0;
			int correspondingVectorIndex = 0;
			for (const auto& [weight, nickname] : lootTable.dropWeights)
			{
				sum += probabilitiesFromWeights[correspondingVectorIndex];
				correspondingVectorIndex++;
				if (randomFloat <= sum && (nickname != "None"))
				{
					std::string itemToDropNickname = nickname;
					Server.MineAsteroid(
					    cShip->iSystem, 
						cShip->get_position(), 
						CreateID(global->config->lootDropContainer.c_str()), 
						CreateID(nickname.c_str()), 
						lootTable.dropCount, 
						cShip->GetOwnerPlayer());
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

REFL_AUTO(type(DropWeight), field(weighting), field(item));
REFL_AUTO(type(LootTable), field(dropCount), field(applyToPlayers), field(applyToNpcs), field(triggerItemNickname), field(dropWeights));
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