
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
	bool CheckForItem(const CShip* ship)
	{
		CEquipManager* eqManager = GetEquipManager((CEqObj*)ship);
		CEquipTraverser tr(65536);
		while (CEquip* eq = eqManager->Traverse(tr))
		{
			const GoodInfo* gi = GoodList_get()->find_by_archetype(eq->archetype->get_id());
			// gi->iType == 0 means actual commodity and not ammo or unmounted guns
			if (gi->iIdSName == global->config->iIdSNameToCheck)
			{
				return true;
			}
			return false;
		}
	}

	// Hook on ship destroyed, like in KillTracker
	void ShipDestroyed(DamageList** dmgList, const DWORD** ecx, const uint& kill)
	{

		const CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);
		if (CheckForItem(cShip))
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

} // namespace Plugins::Template

using namespace Plugins::LootTables;

REFL_AUTO(type(Config), field(overrideUserNumber), field(iIdSNameToCheck), field(test_bool));

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	// Full name of your plugin
	pi->name("Loot Tables");
	// Shortened name, all lower case, no spaces. Abbreviation when possible.
	pi->shortName("loottables");
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
}