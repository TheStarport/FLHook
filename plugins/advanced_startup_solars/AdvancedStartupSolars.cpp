/**
 * @date April, 2024
 * @author IrateRedKite
 * @defgroup AdvancedStartupSolars Advanced Startup Solars
 * @brief
 * This plugin allows server owners to specify startupSolars with more finesse, allowing for mutually exclusive spawns
 *
 * @paragraph cmds Player Commands
 * None
 *
 * @paragraph adminCmds Admin Commands
 * None
 *
 * @paragraph configuration Configuration
 * @code
 *
 * {
 *  "solarFamilies": [
 *      {
 *          "name": "largestation1",
 *          "solarFormations": [
 *              {
 *                  "formation": "largestation1",
 *                  "npcs": {
 *                       "example": 2
 *                   },
 *                  "spawnWeight": 1
 *              }
 *          ],
 *          "spawnChance": 75,
 *          "spawnLocations": [
 *              {
 *                  "location": [
 *                      -31630.4453125,
 *                      1000.0,
 *                      -25773.7421875
 *                  ],
 *                  "system": "li01"
 *              },
 *              {
 *                  "location": [
 *                      -30401.986328125,
 *                      -1000.0,
 *                      -25730.84375
 *                  ],
 *                  "system": "li01"
 *              }
 *          ],
 *          "spawnQuantity": 1
 *      }
 *   ]
 * }
 *
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * None
 *
 * @paragraph optional Optional Plugin Dependencies
 * Solar Control and NPC Control
 */

// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "AdvancedStartupSolars.hpp"

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

	SolarFormation SelectSolarFormation(SolarFamily family)
	{
		std::vector<int> weights {};

		for (auto formation : family.solarFormations)
		{
			weights.emplace_back(formation.spawnWeight);
		}

		std::discrete_distribution<> dist(weights.begin(), weights.end());
		static std::mt19937 engine;
		auto numberIndex = dist(engine);

		auto solarFormation = family.solarFormations[numberIndex];
		family.solarFormations.erase(family.solarFormations.begin() + numberIndex);

		return solarFormation;
	}

	Position SelectSpawnLocation(SolarFamily& family)
	{
		auto locationIndex = RandomNumber(0, family.spawnLocations.size() - 1);

		Position spawnPosition;

		spawnPosition.location = {{family.spawnLocations[locationIndex].location[0]},
		    {family.spawnLocations[locationIndex].location[1]},
		    {family.spawnLocations[locationIndex].location[2]}};
		spawnPosition.system = family.spawnLocations[locationIndex].system;

		// Remove the spawnLocation from the pool of possible locations before returning the vector;
		family.spawnLocations.erase(family.spawnLocations.begin() + locationIndex);

		return spawnPosition;
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

		// Validate our user defined config
		int formationCount = 0;
		for (auto solarFamily : global->config->solarFamilies)
		{
			// Clamps spawnChance between 0 and 100
			solarFamily.spawnChance = std::clamp(solarFamily.spawnChance, 0, 100);

			for (const auto& formation : solarFamily.solarFormations)
			{
				Console::ConDebug(std::format("Loaded formation '{}' into the {} solarFamily pool", wstos(formation.formation), solarFamily.name));
				formationCount++;
			}
		}

		Console::ConDebug(std::format("Loaded {} solarFamilies into the spawn pool", global->config->solarFamilies.size()));
		Console::ConDebug(std::format("Loaded a total of {} formations between the collective solarFamily pool", formationCount));
	}

	// We have to spawn here since the Startup/LoadSettings hooks are too early
	void Login([[maybe_unused]] struct SLoginInfo const& loginInfo, [[maybe_unused]] const uint& client)
	{
		if (!global->firstRun)
		{
			return;
		}

		for (auto& family : global->config->solarFamilies)
		{
			auto dist = RandomNumber(0, 100);

			if (dist <= family.spawnChance)
			{
				for (int i = 0; i < family.spawnQuantity; i++)
				{
					auto spawnPosition = SelectSpawnLocation(family);
					auto solarFormation = SelectSolarFormation(family);
					auto spawnSystem = CreateID(spawnPosition.system.c_str());

					Vector spawnLocation = {{spawnPosition.location[0]}, {spawnPosition.location[1]}, {spawnPosition.location[2]}};

					global->solarCommunicator->CreateSolarFormation(solarFormation.formation, spawnLocation, spawnSystem);

					for (const auto& [key, value] : solarFormation.npcs)
					{
						for (int j = 0; j < value; j++)
						{
							global->npcCommunicator->CreateNpc(key, spawnLocation, EulerMatrix({0, 0, 0}), spawnSystem, true);
						}
					}
				}
			}
		}

		global->firstRun = false;
	}

} // namespace Plugins::AdvancedStartupSolars

using namespace Plugins::AdvancedStartupSolars;
REFL_AUTO(type(SolarFormation), field(formation), field(npcs), field(spawnWeight));
REFL_AUTO(type(Position), field(location), field(system));
REFL_AUTO(type(SolarFamily), field(name), field(solarFormations), field(spawnLocations), field(spawnChance), field(spawnQuantity));
REFL_AUTO(type(Config), field(solarFamilies));

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Advanced Startup Solars");
	pi->shortName("advanced_startup_solars");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &Login);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}