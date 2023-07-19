
/*
*	Temporary Notes:
*		. ask for sensible default behaviour for config probabilities >100%
*		. find out why its still creating this weird Loot Tables dll
*		. how to structure the config json not sillily?
*/

// Includes
#include "LootTables.hpp"

#include <random>

namespace Plugins::LootTables
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	// Maybe not needed, here for now
	bool IsNPC(CShip* ship)
	{
		if (ship->is_player() == true)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	// This is temporarily used to fetch equipement, it will have to be redone once FLHook functionality
	// is updated accordingly.
	bool CheckForItem(const CShip* ship, std::wstring ItemNickname)
	{
		CEquipManager* eqManager = GetEquipManager((CEqObj*)ship);
		CEquipTraverser tr(65536);
		while (CEquip* eq = eqManager->Traverse(tr))
		{
			const GoodInfo* gi = GoodList_get()->find_by_archetype(eq->archetype->get_id());
			//if (gi->iArchId == )
		}
		return true;
	}

	// Hook on ship destroyed, like in KillTracker
	void ShipDestroyed(DamageList** dmgList, const DWORD** ecx, const uint& kill)
	{

		const CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);
		if (CheckForItem(cShip, L"placeholder"))
		{
			const std::wstring message = L"Test message.";
			Hk::Message::MsgU(message);
		}
	}

	// Put things that are performed on plugin load here!
	void LoadSettings()
	{
		// Load JSON config
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(std::move(config));
	}

	LootTable::LootTable(bool PlayersBool, bool NPCsBool, std::wstring TriggerItem, std::map<std::wstring, float> DropProbabilities)
	{
		this->Players = PlayersBool;
		this->NPCs = NPCsBool;
		this->Item = TriggerItem;

		// Check if probabilities given to Constructor make sense
		// if sum of probabilities >100% [insert behaviour]
		// if sum of probabilities <100% chance to drop nothing
		float ProbabilitySum = 0;
		for (auto it = DropProbabilities.begin(); it != DropProbabilities.end(); ++it)
		{
			ProbabilitySum += it->second;
		}
		if (ProbabilitySum <= 1.0)
		{
			this->Probabilities = DropProbabilities;
			this->NoDropChance = (1.0 - ProbabilitySum);
		}
		else
		{
			int my_var = 1;
		}
	}

} // namespace Plugins::LootTables

using namespace Plugins::LootTables;

REFL_AUTO(type(LootTable), field(Players), field(NPCs), field(Item), field(Probabilities), field(NoDropChance));
REFL_AUTO(type(Config), field(DefaultItem_1), field(DefaultProbabilities_1), field(DefaultItem_2), field(DefaultProbabilities_2));

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