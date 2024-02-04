// A plugin that assigns small tasks to players daily, and provides a random reward from a pool of items.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "DailyTasks.hpp"
#include <ranges>
#include <random>

// TODO: Tasks only written when assigned for first time OR when a single task is completed to save file writes.

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
			global->taskTypePool.emplace_back(0);
		}
		if (!global->config->taskNpcKillTargets.empty())
		{
			global->taskTypePool.emplace_back(1);
		}
		if (!global->config->taskPlayerKillTargets.empty())
		{
			global->taskTypePool.emplace_back(2);
		}
		if (!global->config->taskTradeBaseTargets.empty() && !global->config->taskTradeItemTargets.empty())
		{
			global->taskTypePool.emplace_back(3);
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
		for (const auto& base : global->config->taskTradeBaseTargets)
		{
			global->taskTradeBaseTargets.emplace_back(CreateID(base.c_str()));
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
		int currentHour = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now().time_since_epoch()).count() % 24;
		if (currentHour == global->config->resetTime || currentHour == global->config->resetTime + 1 || currentHour == global->config->resetTime + 2)
		{
			// TODO: Reset the daily tasks
			// TODO: Single json file, account name tied to task
		}
	}

	// Writes created and assigned tasks to the relevant account's json file.
	void SaveTaskStatusToJson(CAccount* account)
	{
		auto& taskList = global->accountTasks.at(account);
		auto taskJsonPath = Hk::Client::GetAccountDirName(account);

		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		Serializer::SaveToJson(taskList, std::format("{}\\Accts\\MultiPlayer\\{}\\daily_tasks.json", szDataPath, wstos(taskJsonPath)));
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("Saving a task status update to {}\\Accts\\MultiPlayer\\{}\\daily_tasks.json", szDataPath, wstos(taskJsonPath)));
	}

	// Loads tasks from an account's relevant json file and and checks the date against the current time.
	void LoadTaskStatusFromJson(CAccount* account)
	{
		auto& taskList = global->accountTasks.at(account);
		auto taskJsonPath = Hk::Client::GetAccountDirName(account);

		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		auto obj = Serializer::JsonToObject<Tasks>(std::format("{}\\Accts\\MultiPlayer\\{}\\daily_tasks.json", szDataPath, wstos(taskJsonPath)), true);

		// auto config = Serializer::JsonToObject<Config>();
		// global->config = std::make_unique<Config>(std::move(config));
	}

	// Function: Brief hook on ship destroyed to see if a task needs to be updated.
	void ShipDestroyed()
	{
		// TODO: Create and implement this function.
	}

	// Function: Brief hook on ship destroyed to see if a task needs to be updated.
	void ItemSold()
	{
		// TODO: Create and implement this function.
	}
	// Function: Brief hook on item bought to see if a task needs to be updated.
	void ItemAcquired()
	{
		// TODO: Create and implement this function.
	}

	// Function: Generates a daily task.
	void GenerateDailyTask(CAccount* account)
	{
		// Choose and create a random task from the available pool.
		const auto& randomTask = global->taskTypePool[RandomNumber(0, global->taskTypePool.size() - 1)];

		if (randomTask == 0)
		{
			// Create an item acquisition task
			auto itemAcquisitionTarget = RandomIdKey(global->taskItemAcquisitionTargets);
			auto itemQuantity =
			    RandomNumber(global->taskItemAcquisitionTargets.at(itemAcquisitionTarget)[0], global->taskItemAcquisitionTargets.at(itemAcquisitionTarget)[1]);
			auto itemArch = Archetype::GetEquipment(itemAcquisitionTarget);
			auto taskDescription = std::format("Acquire {} units of {}", itemQuantity, wstos(Hk::Message::GetWStringFromIdS(itemArch->iIdsName)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating an 'Acquire Items' task to '{}'", taskDescription));

			Task task;
			task.taskType = 0;
			task.itemTarget = itemAcquisitionTarget;
			task.quantity = itemQuantity;
			task.taskDescription = taskDescription;
			task.isCompleted = false;
			task.setTime = Hk::Time::GetUnixSeconds();

			if (!global->accountTasks.contains(account))
			{
				global->accountTasks[account] = {};
			}
			global->accountTasks[account].tasks.emplace_back(task);
		}
		if (randomTask == 1)
		{
			// Create an NPC kill task
			const auto& npcFactionTarget = RandomIdKey(global->taskNpcKillTargets);
			auto npcQuantity = RandomNumber(global->taskNpcKillTargets.at(npcFactionTarget)[0], global->taskNpcKillTargets.at(npcFactionTarget)[1]);
			uint npcFactionIds;
			pub::Reputation::GetGroupName(npcFactionTarget, npcFactionIds);
			auto taskDescription = std::format("Destroy {} ships belonging to the {}", npcQuantity, wstos(Hk::Message::GetWStringFromIdS(npcFactionIds)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Kill NPCs' task to '{}'", taskDescription));

			Task task;
			task.taskType = 1;
			task.npcFactionTarget = npcFactionTarget;
			task.quantity = npcQuantity;
			task.taskDescription = taskDescription;
			task.isCompleted = false;
			task.setTime = Hk::Time::GetUnixSeconds();

			if (!global->accountTasks.contains(account))
			{
				global->accountTasks[account] = {};
			}
			global->accountTasks[account].tasks.emplace_back(task);
		}
		if (randomTask == 2)
		{
			// Create a player kill task
			auto playerQuantity = RandomNumber(global->config->taskPlayerKillTargets[0], global->config->taskPlayerKillTargets[1]);
			auto taskDescription = std::format("Destroy {} player ships", playerQuantity);
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Kill Players' task to '{}'", taskDescription));

			Task task;
			task.taskType = 2;
			task.quantity = playerQuantity;
			task.taskDescription = taskDescription;
			task.isCompleted = false;
			task.setTime = Hk::Time::GetUnixSeconds();

			if (!global->accountTasks.contains(account))
			{
				global->accountTasks[account] = {};
			}
			global->accountTasks[account].tasks.emplace_back(task);
		}
		if (randomTask == 3)
		{
			// Create a trade task
			const auto& tradeBaseTarget = global->taskTradeBaseTargets[RandomNumber(0, global->taskTradeBaseTargets.size() - 1)];
			auto tradeItemTarget = RandomIdKey(global->taskTradeItemTargets);
			auto tradeItemQuantity = RandomNumber(global->taskTradeItemTargets.at(tradeItemTarget)[0], global->taskTradeItemTargets.at(tradeItemTarget)[1]);
			auto baseArch = Universe::get_base(tradeBaseTarget);
			auto itemArch = Archetype::GetEquipment(tradeItemTarget);
			auto taskDescription = std::format("Sell {} units of {} at {}",
			    tradeItemQuantity,
			    wstos(Hk::Message::GetWStringFromIdS(itemArch->iIdsName)),
			    wstos(Hk::Message::GetWStringFromIdS(baseArch->baseIdS)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Sell Cargo' task to '{}'", taskDescription));

			Task task;
			task.taskType = 3;
			task.baseTarget = tradeBaseTarget;
			task.itemTarget = tradeItemTarget;
			task.quantity = tradeItemQuantity;
			task.taskDescription = taskDescription;
			task.isCompleted = false;
			task.setTime = Hk::Time::GetUnixSeconds();

			if (!global->accountTasks.contains(account))
			{
				global->accountTasks[account] = {};
			}
			global->accountTasks[account].tasks.emplace_back(task);
		}
	}

	// Function: A command to display the current daily tasks a player has.
	void UserCmdShowDailyTasks(ClientId& client, const std::wstring& param)
	{
		auto account = Hk::Client::GetAccountByClientID(client);
		PrintUserCmdText(client, L"CURRENT DAILY TASKS");
		for (auto& task : global->accountTasks[account].tasks)
		{
			if (!task.isCompleted)
			{
				PrintUserCmdText(client, stows(task.taskDescription));
			}
			else
			{
				PrintUserCmdText(client, stows(task.taskDescription + "TASK COMPLETED"));
			}
		}
	}

	// Function: A command to reset user tasks.
	void UserCmdResetUserDailyTasks()
	{
		// TODO: Create and implement this function.
	}

	// Function: Hook on player login to assign and check tasks.
	void OnLogin([[maybe_unused]] struct SLoginInfo const& li, ClientId& client)
	{
		auto account = Hk::Client::GetAccountByClientID(client);

		for (int i = 0; i < global->config->taskQuantity; i++)
		{
			GenerateDailyTask(account);
		}

		SaveTaskStatusToJson(account);
	}

	// Define usable chat commands here
	const std::vector commands = {{
	    CreateUserCommand(L"/showdailies", L"", UserCmdShowDailyTasks, L"Shows a list of current daily tasks for the user"),
	}};

} // namespace Plugins::DailyTasks

using namespace Plugins::DailyTasks;

REFL_AUTO(type(Config), field(taskQuantity), field(taskResetAmount), field(minCreditsReward), field(maxCreditsReward), field(itemRewardPool),
    field(taskTradeBaseTargets), field(taskTradeItemTargets), field(taskItemAcquisitionTargets), field(taskNpcKillTargets), field(taskPlayerKillTargets));

REFL_AUTO(type(Tasks), field(tasks));

REFL_AUTO(type(Task), field(taskType), field(quantity), field(itemTarget), field(baseTarget), field(npcFactionTarget), field(taskDescription),
    field(isCompleted), field(setTime));

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
	pi->emplaceHook(HookedCall::IServerImpl__Login, &OnLogin, HookStep::After);

	// pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	// pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &ItemSold);
	// pi->emplaceHook(HookedCall::IServerImpl__GFGoodSell, &ItemAcquired);
	// pi->emplaceHook(HookedCall::IServerImpl__TractorObjects, &ItemAcquired);
}