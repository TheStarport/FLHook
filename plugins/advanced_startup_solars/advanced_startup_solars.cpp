// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "advanced_startup_solars.hpp"

#include <random>

namespace Plugins::AdvancedStartupSolars
{
	const auto global = std::make_unique<Global>();

	// Function: Generates a random int between min and max
	int RandomNumber(int min, int max)
	{
		static std::random_device dev;
		static auto engine = std::mt19937(dev());
		auto range = std::uniform_int_distribution(min, max);
		return range(engine);
	}

	// Put things that are performed on plugin load here!
	void LoadSettings()
	{
		// Load JSON config
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(std::move(config));

		// Set the npcCommunicator and solarCommunicator interfaces and check if they are availlable
		global->npcCommunicator =
		    static_cast<Plugins::Npc::NpcCommunicator*>(PluginCommunicator::ImportPluginCommunicator(Plugins::Npc::NpcCommunicator::pluginName));

		global->solarCommunicator = static_cast<Plugins::SolarControl::SolarCommunicator*>(
		    PluginCommunicator::ImportPluginCommunicator(Plugins::SolarControl::SolarCommunicator::pluginName));

		// Prevent the plugin from progressing further and disable all functions if either interface is not found.
		if (!global->npcCommunicator)
		{
			Console::ConErr(std::format("npc.dll not found. The plugin is required for this module to function."));
			global->pluginActive = false;
		}
		if (!global->solarCommunicator)
		{
			Console::ConErr(std::format("solar.dll not found. The plugin is required for this module to function."));
			global->pluginActive = false;
		}
		if (!global->pluginActive)
		{
			Console::ConErr(
			    std::format("Critical components of Advanced Startup Solars were not found or were configured incorrectly. The plugin has been disabled."));
			return;
		}

		// Validate our user define config
		int formationCount = 0;
		for (auto solarFamily : global->config->solarFamilies)
		{
			// Clamps spawnChance between 0 and 100
			solarFamily.spawnChance = std::clamp(solarFamily.spawnChance, 0, 100);

			for (const auto& formation : solarFamily.solarFormations)
			{
				Console::ConDebug(std::format("Loaded formation '{}' into the {} solarFamily pool", formation.formation, solarFamily.name));
				formationCount++;
			}
		}

		Console::ConDebug(std::format("Loaded {} solarFamilies into the spawn pool", global->config->solarFamilies.size()));
		Console::ConDebug(std::format("Loaded a total of {} formations between the collective solarFamily pool", formationCount));
	}

} // namespace Plugins::AdvancedStartupSolars

using namespace Plugins::AdvancedStartupSolars;
REFL_AUTO(type(SolarFormation), field(formation), field(npcs), field(spawnWeight));
REFL_AUTO(type(Position), field(x), field(y), field(z));
REFL_AUTO(type(SolarFamily), field(name), field(solarFormations), field(spawnLocations), field(spawnChance), field(spawnQuantity));
REFL_AUTO(type(Config), field(solarFamilies));

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	// Full name of your plugin
	pi->name("Advanced Startup Solars");
	// Shortened name, all lower case, no spaces. Abbreviation when possible.
	pi->shortName("advanced_startup_solars");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}