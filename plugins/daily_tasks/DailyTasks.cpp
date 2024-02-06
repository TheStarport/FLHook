/**
 * @date 2024
 * @author IrateRedKite
 * @defgroup DailyTasks Daily Tasks
 * @brief
 * The plugin assigns randomly generated tasks to players that they can complete for a reward.
 *
 * @paragraph cmds Player Commands
 * - showtasks - Shows the current tasks assigned to the player's account, time remaining and completion status.
 * - resettasks - Resets and rerolls the player's assigned tasks. This can be done once per day.
 * @paragraph adminCmds Admin Commands
 * -resetalltasks - Resets and rerolls player tasks for all current players on the server, and clears the daily_tasks.json file from each account's folder.
 *
 * @paragraph configuration Configuration
 * @code
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

// Includes
#include "DailyTasks.hpp"
#include <ranges>
#include <random>

// TODO: Task Tracking
// TODO: Task Rewards
// TODO:Completion State Saving
// TODO: Usability pass

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

	// Function: Saves tasks to an account's daily_tasks.json file
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

	// Function: Loads tasks from an account's daily_tasks.json file
	void LoadTaskStatusFromJson(CAccount* account)
	{
		auto taskJsonPath = Hk::Client::GetAccountDirName(account);
		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		auto taskList = Serializer::JsonToObject<Tasks>(std::format("{}\\Accts\\MultiPlayer\\{}\\daily_tasks.json", szDataPath, wstos(taskJsonPath)), true);
		global->accountTasks[account] = taskList;
	}

	// Function: Brief hook on ship destroyed to see if a task needs to be updated.
	void ShipDestroyed()
	{
		// TODO: Create and implement this function.
	}

	// Function: Brief hook on item sold to see if a task needs to be updated.
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
			task.quantityCompleted = 0;
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
			task.quantityCompleted = 0;
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
			task.quantityCompleted = 0;
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
			task.quantityCompleted = 0;
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

	// Function: Keeps track of time.
	void DailyTimerTick()
	{
		// Checks the current hour to see if global->dailyReset should be flipped back to false
		if (int currentHour =
		        std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now().time_since_epoch()).count() % 24 == global->config->resetTime ||
		        currentHour == global->config->resetTime + 1 && global->dailyReset == false)
		{
			global->dailyReset = true;
			global->tasksReset.clear();
		}
		else
		{
			global->dailyReset = false;
		}

		// Iterates over online players and checks the time status of their tasks, clearing and resetting them if they exceed 24 hours.
		auto onlinePlayers = Hk::Admin::GetPlayers();
		auto currentTime = Hk::Time::GetUnixSeconds();
		for (auto& players : onlinePlayers)
		{
			auto account = Hk::Client::GetAccountByClientID(players.client);
			auto accountId = account->wszAccId;

			for (auto& tasks : global->accountTasks[account].tasks)
			{
				if ((currentTime - tasks.setTime) > 86400)
				{
					AddLog(
					    LogType::Normal, LogLevel::Debug, std::format("Tasks for {} are out of date, refreshing and creating new tasks...", wstos(accountId)));
					global->accountTasks[account].tasks.erase(global->accountTasks[account].tasks.begin(), global->accountTasks[account].tasks.end());
					for (int i = 0; i < global->config->taskQuantity; i++)
					{
						GenerateDailyTask(account);
					}
				}
			}
		}
	}

	// Function: A command to display the current daily tasks a player has.
	void UserCmdShowDailyTasks(ClientId& client, const std::wstring& param)
	{
		auto account = Hk::Client::GetAccountByClientID(client);
		PrintUserCmdText(client, L"CURRENT DAILY TASKS");
		for (auto& task : global->accountTasks[account].tasks)
		{
			int taskExpiry = ((86400 - (Hk::Time::GetUnixSeconds() - task.setTime)) / 60) / 60;
			if (!task.isCompleted)
			{
				// PrintUserCmdText(client, stows(task.taskDescription + std::format(" expires in {} hours", taskExpiry)));
				PrintUserCmdText(client, std::format(L"{} expires in {} hours", stows(task.taskDescription), taskExpiry));
			}
			else
			{
				PrintUserCmdText(client, stows(task.taskDescription + " TASK COMPLETED"));
			}
		}
	}

	// Function: A command to reset user tasks.
	void UserCmdResetDailyTasks(ClientId& client, const std::wstring& param)
	{
		auto account = Hk::Client::GetAccountByClientID(client);
		auto accountId = account->wszAccId;

		for (auto& tasks : global->accountTasks[account].tasks)
		{
			if (tasks.isCompleted == true)
			{
				PrintUserCmdText(client,
				    std::format(L"You have completed one or more of your daily tasks today, and cannot reset them until {}:00", global->config->resetTime));
				break;
			}
		}

		if (global->tasksReset[account] == false)
		{
			AddLog(LogType::Normal, LogLevel::Debug, std::format("{} is resetting their daily tasks.", wstos(accountId)));

			global->accountTasks[account].tasks.erase(global->accountTasks[account].tasks.begin(), global->accountTasks[account].tasks.end());
			for (int i = 0; i < global->config->taskQuantity; i++)
			{
				GenerateDailyTask(account);
			}

			global->tasksReset[account] = true;
			SaveTaskStatusToJson(account);
			PrintUserCmdText(client, L"Your daily tasks have been reset.");
		}
		else
		{
			PrintUserCmdText(client, L"You've already reset your daily tasks for today.");
		}
	}

	// Function: Resets tasks for all currently online players and clears account folders of daily_task.json files.
	void AdminCmdResetAllTasks(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print("ERR No permission");
			return;
		}
	}

	// Function: Hook on player login to assign and check tasks.
	void OnLogin([[maybe_unused]] struct SLoginInfo const& li, ClientId& client)
	{
		auto account = Hk::Client::GetAccountByClientID(client);
		auto accountId = account->wszAccId;
		LoadTaskStatusFromJson(account);

		if (global->accountTasks[account].tasks.empty())
		{
			AddLog(LogType::Normal, LogLevel::Debug, std::format("No tasks saved for {}, creating new tasks...", wstos(accountId)));
			for (int i = 0; i < global->config->taskQuantity; i++)
			{
				GenerateDailyTask(account);
			}
			SaveTaskStatusToJson(account);
			return;
		}
		else
		{
			auto currentTime = Hk::Time::GetUnixSeconds();

			for (auto& task : global->accountTasks[account].tasks)
			{
				AddLog(LogType::Normal, LogLevel::Debug, std::format("Loading tasks for {} from stored json file...", wstos(accountId)));
				// If tasks are older than 24 hours, refresh them.
				if ((currentTime - task.setTime) > 86400)
				{
					AddLog(
					    LogType::Normal, LogLevel::Debug, std::format("Tasks for {} are out of date, refreshing and creating new tasks...", wstos(accountId)));
					global->accountTasks[account].tasks.erase(global->accountTasks[account].tasks.begin(), global->accountTasks[account].tasks.end());
					for (int i = 0; i < global->config->taskQuantity; i++)
					{
						GenerateDailyTask(account);
					}
				}

				SaveTaskStatusToJson(account);
				return;
			}
		}
	}

	const std::vector commands = {{
	    CreateUserCommand(L"/showtasks", L"", UserCmdShowDailyTasks, L"Shows a list of current daily tasks for the user"),
	    CreateUserCommand(L"/resettasks", L"", UserCmdResetDailyTasks, L"Resets the user's daily tasks if none have already been completed"),
	}};

	bool ExecuteCommandString(CCmds* cmds, const std::wstring& cmd)
	{
		if (cmd == L"template")
		{
			AdminCmdResetAllTasks(cmds);
		}
		else
		{
			return false;
		}

		global->returnCode = ReturnCode::SkipAll;
		return true;
	}

} // namespace Plugins::DailyTasks

using namespace Plugins::DailyTasks;

REFL_AUTO(type(Config), field(taskQuantity), field(minCreditsReward), field(maxCreditsReward), field(itemRewardPool), field(taskTradeBaseTargets),
    field(taskTradeItemTargets), field(taskItemAcquisitionTargets), field(taskNpcKillTargets), field(taskPlayerKillTargets), field(taskDuration),
    field(resetTime));

REFL_AUTO(type(Tasks), field(tasks));

REFL_AUTO(type(Task), field(taskType), field(quantity), field(itemTarget), field(baseTarget), field(npcFactionTarget), field(taskDescription),
    field(isCompleted), field(setTime), field(quantityCompleted));

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