// A plugin that assigns small tasks to players daily, and provides a random reward from a pool of items.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "DailyTasks.hpp"

#include <random>

// TODO: Format strings for task descriptions

namespace Plugins::DailyTasks
{
	const auto global = std::make_unique<Global>();

	// Put things that are performed on plugin load here!
	void LoadSettings()
	{
		// Load JSON config
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(std::move(config));

		// Load the string DLLS
		Hk::Message::LoadStringDLLs();

		// TODO: Include taskTradeBaseTargets in this set of loops.
		// Convert the config inputs into something we can work with.
		for (const auto& [key, value] : global->config->itemRewardPool)
		{
			global->itemRewardPool[CreateID(key.c_str())] = value;
		}
		for (const auto& [key, value] : global->config->taskTradeItemTargets)
		{
			global->taskTradeItemTargets[CreateID(key.c_str())] = value;
		}
		for (const auto& [key, value] : global->config->taskItemAcquisitionTargets)
		{
			global->taskItemAcquisitionTargets[CreateID(key.c_str())] = value;
		}
		for (const auto& [key, value] : global->config->taskNpcKillTargets)
		{
			global->taskNpcKillTargets[CreateID(key.c_str())] = value;
		}
	}

	// Function: Generates a random int between min and max
	int RandomNumber(int min, int max)
	{
		static std::random_device dev;
		static auto engine = std::mt19937(dev());
		auto range = std::uniform_int_distribution(min, max);
		return range(engine);
	}

	// Function: Picks a random key from a map
	uint RandomIdKey(std::map<uint, std::vector<int>> map)
	{
		auto iterator = map.begin();
		std::advance(iterator, RandomNumber(1, map.size()));
		auto& outputId = iterator->first;
		return outputId;
	}

	// Function: Keeps track of time.
	void DailyTimerTick()
	{
		// TODO: Create and implement this function.
	}
	// Function: Generates a daily task.
	void GenerateDailyTask()
	{
		std::vector<std::string> taskTypePool;
		std::vector<std::string> rewardPool;

		// Check if task config values are populated. If they're populated, add them to the pool.
		if (!global->config->taskItemAcquisitionTargets.empty())
		{
			taskTypePool.emplace_back("Acquire Items");
		}
		if (!global->config->taskNpcKillTargets.empty())
		{
			taskTypePool.emplace_back("Kill NPCs");
		}
		if (!global->config->taskPlayerKillTargets.empty())
		{
			taskTypePool.emplace_back("Kill Players");
		}
		if (!global->config->taskTradeBaseTargets.empty() && !global->config->taskTradeItemTargets.empty())
		{
			taskTypePool.emplace_back("Sell Cargo");
		}

		// Check if taskTypePool is empty after these checks and if so throw an error in the console.
		if (taskTypePool.empty())
		{
			AddLog(LogType::Normal, LogLevel::Err, "No tasks have been defined in daily_tasks.json");
			return;
		}

		// Choose a random task from the availlable pool.
		const auto& randomTask = taskTypePool[RandomNumber(1, taskTypePool.size())];

		if (randomTask == "Acquire Items")
		{
			auto itemAcquisitionTarget = RandomIdKey(global->taskItemAcquisitionTargets);
			auto itemQuantity =
			    RandomNumber(global->taskItemAcquisitionTargets.at(itemAcquisitionTarget)[0], global->taskItemAcquisitionTargets.at(itemAcquisitionTarget)[1]);

			auto taskDescription = "Acquire {} units of {}";
		}
		if (randomTask == "Kill NPCs")
		{
			auto npcFactionTarget = RandomIdKey(global->taskNpcKillTargets);
			auto npcQuantity = RandomNumber(global->taskNpcKillTargets.at(npcFactionTarget)[0], global->taskNpcKillTargets.at(npcFactionTarget)[1]);

			// Grab the faction data so it can be included in the taskDescription string.
			uint npcFactionAffiliation;
			pub::Reputation::GetAffiliation(npcFactionTarget, npcFactionAffiliation);
			uint npcFactionIds;
			pub::Reputation::GetGroupName(npcFactionAffiliation, npcFactionIds);
			auto taskDescription = "Destroy {} ships belonging to {}";
		}
		if (randomTask == "Kill Players")
		{
			auto playerQuantity = RandomNumber(global->config->taskPlayerKillTargets[0], global->config->taskPlayerKillTargets[1]);
			auto taskDescription = "Destroy {} player ships";
		}
		if (randomTask == "Sell Cargo")
		{
			// Pick a random base and a random item from the config pools.
			auto tradeBaseTarget = global->config->taskTradeBaseTargets[RandomNumber(1, global->taskTradeBaseTargets.size())];
			auto tradeItemTarget = RandomIdKey(global->taskTradeItemTargets);
			auto tradeItemQuantity = RandomNumber(global->taskTradeItemTargets.at(tradeItemTarget)[0], global->taskTradeItemTargets.at(tradeItemTarget)[1]);
			auto taskDescription = "Sell {} units of {} at {}";
		}
	}

	// Function: Assigns a daily task to a character.
	void AssignDailyTask()
	{
		// TODO: Create and implement this function.
	}

	// Function: A command to display the current daily tasks a player has.
	void UserCmdShowDailyTasks(ClientId& client, const std::wstring& param)
	{
		// TODO: Create and implement this function.
	}

	// Function: A command to reset user tasks.
	void UserCmdResetUserDailyTasks()
	{
		// TODO: Create and implement this function.
	}

	// Define usable chat commands here
	const std::vector commands = {{
	    CreateUserCommand(L"/showdailies", L"<number>", UserCmdShowDailyTasks, L"Shows a list of current daily tasks for the user"),
	}};

} // namespace Plugins::DailyTasks

using namespace Plugins::DailyTasks;

REFL_AUTO(type(Config), field(taskQuantity), field(taskResetAmount), field(minCreditsReward), field(maxCreditsReward), field(itemRewardPool),
    field(taskTradeBaseTargets), field(taskTradeItemTargets), field(taskItemAcquisitionTargets), field(taskNpcKillTargets), field(taskPlayerKillTargets));

DefaultDllMainSettings(LoadSettings);
const std::vector<Timer> timers = {{DailyTimerTick, 3600}};
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	// Full name of your plugin
	pi->name("Daily Tasks");
	// Shortened name, all lower case, no spaces. Abbreviation when possible.
	pi->shortName("dailytasks");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->timers(&timers);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}