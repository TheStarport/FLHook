// A plugin that assigns small tasks to players daily, and provides a random reward from a pool of items.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "DailyTasks.hpp"

#include <random>

namespace Plugins::DailyTasks
{
	const auto global = std::make_unique<Global>();

	// Put things that are performed on plugin load here!
	void LoadSettings()
	{
		// Load JSON config
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(std::move(config));

		// Check if task config values are populated. If they're populated, add them to the pool.
		if (!global->config->taskItemAcquisitionTargets.empty())
		{
			global->taskTypePool.emplace_back("Acquire Items");
		}
		if (!global->config->taskNpcKillTargets.empty())
		{
			global->taskTypePool.emplace_back("Kill NPCs");
		}
		if (!global->config->taskPlayerKillTargets.empty())
		{
			global->taskTypePool.emplace_back("Kill Players");
		}
		if (!global->config->taskTradeBaseTargets.empty() && !global->config->taskTradeItemTargets.empty())
		{
			global->taskTypePool.emplace_back("Sell Cargo");
		}

		// Check if taskTypePool is empty after these checks and if so throw an error in the console.
		if (global->taskTypePool.empty())
		{
			AddLog(LogType::Normal, LogLevel::Err, "No tasks have been defined in daily_tasks.json. No daily tasks will be generated.");
			return;
		}
		AddLog(LogType::Normal,
		    LogLevel::Info,
		    std::format("{} possible random daily tasks have been loaded into the pool.", static_cast<int>(global->taskTypePool.size())));

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
			global->taskNpcKillTargets[MakeId(key.c_str())] = value;
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
		std::advance(iterator, RandomNumber(0, map.size() - 1));
		auto& outputId = iterator->first;
		return outputId;
	}

	// Function: Keeps track of time.
	void DailyTimerTick()
	{
		// TODO: Create and implement this function.
	}

	void SaveTaskStatus()
	{
		// TODO: Create and implement this function.
	}

	//Function: Brief hook on ship destroyed to see if a task needs to be updated.
	void ShipDestroyed()
	{
	}

	// Function: Brief hook on ship destroyed to see if a task needs to be updated.
	void ItemSold()
	{
	}

	void ItemAcquired()
	{
	}

	// TODO: Remove client from this
	// Function: Generates a daily task.
	void GenerateDailyTask(ClientId& client)
	{
		// TODO: Have these return json objects that can be written to a file in the player save directory?
		// Choose and create a random task from the available pool.
		const auto& randomTask = global->taskTypePool[RandomNumber(0, global->taskTypePool.size() - 1)];

		if (randomTask == "Acquire Items")
		{
			auto itemAcquisitionTarget = RandomIdKey(global->taskItemAcquisitionTargets);
			auto itemQuantity =
			    RandomNumber(global->taskItemAcquisitionTargets.at(itemAcquisitionTarget)[0], global->taskItemAcquisitionTargets.at(itemAcquisitionTarget)[1]);
			auto itemArch = Archetype::GetEquipment(itemAcquisitionTarget);
			auto taskDescription = std::format("Acquire {} units of {}", itemQuantity, wstos(Hk::Message::GetWStringFromIdS(itemArch->iIdsName)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating an 'Acquire Items' task to '{}'", taskDescription));
		}
		if (randomTask == "Kill NPCs")
		{
			const auto& npcFactionTarget = RandomIdKey(global->taskNpcKillTargets);
			auto npcQuantity = RandomNumber(global->taskNpcKillTargets.at(npcFactionTarget)[0], global->taskNpcKillTargets.at(npcFactionTarget)[1]);
			uint npcFactionIds;
			pub::Reputation::GetGroupName(npcFactionTarget, npcFactionIds);
			auto taskDescription = std::format("Destroy {} ships belonging to the {}", npcQuantity, wstos(Hk::Message::GetWStringFromIdS(npcFactionIds)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Kill NPCs' task to '{}'", taskDescription));
		}
		if (randomTask == "Kill Players")
		{
			auto playerQuantity = RandomNumber(global->config->taskPlayerKillTargets[0], global->config->taskPlayerKillTargets[1]);
			auto taskDescription = std::format("Destroy {} player ships", playerQuantity);
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Kill Players' task to '{}'", taskDescription));
		}
		if (randomTask == "Sell Cargo")
		{
			const auto& tradeBaseTarget = global->config->taskTradeBaseTargets[RandomNumber(0, global->config->taskTradeBaseTargets.size())];
			auto tradeItemTarget = RandomIdKey(global->taskTradeItemTargets);
			auto tradeItemQuantity = RandomNumber(global->taskTradeItemTargets.at(tradeItemTarget)[0], global->taskTradeItemTargets.at(tradeItemTarget)[1]);
			auto baseArch = Universe::get_base(CreateID(tradeBaseTarget.c_str()));
			auto itemArch = Archetype::GetEquipment(tradeItemTarget);
			auto taskDescription = std::format("Sell {} units of {} at {}",
			    tradeItemQuantity,
			    wstos(Hk::Message::GetWStringFromIdS(itemArch->iIdsName)),
			    wstos(Hk::Message::GetWStringFromIdS(baseArch->baseIdS)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Sell Cargo' task to '{}'", taskDescription));
		}
	}

	// Function: Assigns a daily task to a character.
	void AssignDailyTask(ClientId& client)
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

	// TODO: Remove /gen
	// Define usable chat commands here
	const std::vector commands = {{
	    CreateUserCommand(L"/showdailies", L"", UserCmdShowDailyTasks, L"Shows a list of current daily tasks for the user"),
	    CreateUserCommand(L"/gen", L"", GenerateDailyTask, L"Generates a daily task for testing"),
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

	//pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	//pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &ItemSold);
	//pi->emplaceHook(HookedCall::IServerImpl__GFGoodSell, &ItemAcquired);
	//pi->emplaceHook(HookedCall::IServerImpl__TractorObjects, &ItemAcquired);
}