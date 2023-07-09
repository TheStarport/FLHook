// This is a template with the bare minimum to have a functional plugin.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "LootTables.hpp"

#include <random>

namespace Plugins::Template
{
	const auto global = std::make_unique<Global>();

	// Put things that are performed on plugin load here!
	void LoadSettings()
	{
		// Load JSON config
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(std::move(config));
	}

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

	void ShipDestroyed(DamageList** dmgList, const DWORD** ecx, const uint& kill)
	{
		if (kill == 1)
		{
			const CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);
		}
	}

} // namespace Plugins::Template

using namespace Plugins::Template;

REFL_AUTO(type(Config), field(overrideUserNumber));

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	// Full name of your plugin
	pi->name("$projectname$");
	// Shortened name, all lower case, no spaces. Abbreviation when possible.
	pi->shortName("$safeprojectname$");
	pi->mayUnload(true);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}