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

// TODO: Task Rewards
// TODO:Completion State Saving on Dock
// TODO: Hook on tractor item for acquire
// TODO: Handle disconnects
// TODO: Usability pass
// TODO: Admin commands

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
			global->taskTypePool.emplace_back(TaskType::GetItem);
		}
		if (!global->config->taskNpcKillTargets.empty())
		{
			global->taskTypePool.emplace_back(TaskType::KillNpc);
		}
		if (!global->config->taskPlayerKillTargets.empty())
		{
			global->taskTypePool.emplace_back(TaskType::KillPlayer);
		}
		if (!global->config->taskTradeBaseTargets.empty() && !global->config->taskTradeItemTargets.empty())
		{
			global->taskTypePool.emplace_back(TaskType::SellItem);
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

	// Function: Generates and awards a reward from the pool.
	void GenerateReward(ClientId& client)
	{
		auto character = Hk::Client::GetCharacterNameByID(client).value();
		auto creditReward = RandomNumber(global->config->minCreditsReward, global->config->minCreditsReward);
		auto& itemReward = global->itemRewardPool[RandomNumber(0, global->itemRewardPool.size() - 1)];
		auto itemRewardQuantity = RandomNumber(itemReward.at(1), itemReward.at(2));

		Hk::Player::AddCash(character, creditReward);
		auto holdSize = 0;
		const auto& cargo = Hk::Player::EnumCargo(client, holdSize).value();

		for (auto& item : cargo)
		{
			auto freeCargo = item.fStatus;
		}
		// Hk::Player::AddCargo(client, itemReward, itemRewardQuantity, false)

		// TODO: If hold doesn't have enough room, reward what you can and then top up value w/ credits value of the base good (ArchToGood?)
	}

	// Function: Brief hook on ship destroyed to see if a task needs to be updated.
	void ShipDestroyed([[maybe_unused]] DamageList** _dmg, const DWORD** ecx, const uint& kill)
	{
		if (kill == 1)
		{
			const CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);
			if (ClientId client = cShip->GetOwnerPlayer())
			{
				const DamageList* dmg = *_dmg;
				const auto killerId = Hk::Client::GetClientIdByShip(
				    dmg->get_cause() == DamageCause::Unknown ? ClientInfo[client].dmgLast.get_inflictor_id() : dmg->get_inflictor_id());
				const auto victimId = Hk::Client::GetClientIdByShip(cShip->get_id());
				for (auto& task : global->accountTasks[Hk::Client::GetAccountByClientID(killerId.value())].tasks)
				{
					if (task.taskType == TaskType::KillPlayer && task.isCompleted == false && victimId.has_value())
					{
						task.quantityCompleted++;
					}
					if (task.quantityCompleted == task.quantity && task.taskType == TaskType::KillPlayer && task.isCompleted == false)
					{
						task.isCompleted = true;
						SaveTaskStatusToJson(Hk::Client::GetAccountByClientID(killerId.value()));
						PrintUserCmdText(client, std::format(L"You have completed {}", stows(task.taskDescription)));
						Hk::Client::PlaySoundEffect(client, CreateID("ui_gain_level"));
						// TODO: Reward
					}
				}
			}
			else
			{
				const DamageList* dmg = *_dmg;
				const auto killerId = Hk::Client::GetClientIdByShip(
				    dmg->get_cause() == DamageCause::Unknown ? ClientInfo[client].dmgLast.get_inflictor_id() : dmg->get_inflictor_id());
				int reputation;
				pub::SpaceObj::GetRep(cShip->get_id(), reputation);
				uint affiliation;
				pub::Reputation::GetAffiliation(reputation, affiliation);

				for (auto& task : global->accountTasks[Hk::Client::GetAccountByClientID(killerId.value())].tasks)
				{
					if (task.taskType == TaskType::KillNpc && task.isCompleted == false && task.npcFactionTarget == affiliation)
					{
						task.quantityCompleted++;
					}
					if (task.quantityCompleted == task.quantity && task.taskType == TaskType::KillNpc && task.isCompleted == false &&
					    task.npcFactionTarget == affiliation)
					{
						task.isCompleted = true;
						SaveTaskStatusToJson(Hk::Client::GetAccountByClientID(killerId.value()));
						PrintUserCmdText(client, std::format(L"You have completed {}", stows(task.taskDescription)));
						Hk::Client::PlaySoundEffect(client, CreateID("ui_gain_level"));
						// TODO: Reward
					}
				}
			}
		}
	}

	// Function: Brief hook on item sold to see if a task needs to be updated.
	void ItemSold(const struct SGFGoodSellInfo& gsi, ClientId& client)
	{
		auto base = Hk::Player::GetCurrentBase(client);
		auto account = Hk::Client::GetAccountByClientID(client);
		for (auto& task : global->accountTasks[account].tasks)
		{
			if (task.isCompleted)
			{
				continue;
			}
			if (task.taskType == TaskType::SellItem && task.itemTarget == gsi.iArchId && task.baseTarget == base.value())
			{
				task.quantityCompleted += gsi.iCount;
				if (task.quantityCompleted >= task.quantity)
				{
					task.isCompleted = true;
					SaveTaskStatusToJson(account);
					PrintUserCmdText(client, std::format(L"You have completed {}", stows(task.taskDescription)));
					Hk::Client::PlaySoundEffect(client, CreateID("ui_gain_level"));
					// TODO: Reward
				}
			}
			else if (task.taskType == TaskType::GetItem && task.itemTarget == gsi.iArchId)
			{
				task.quantityCompleted = std::clamp(task.quantityCompleted - gsi.iCount, 0, task.quantity);
			}
		}
	}
	// Function: Brief hook on item bought to see if a task needs to be updated.
	void ItemPurchased(SGFGoodBuyInfo const& gbi, ClientId& client)
	{
		auto base = Hk::Player::GetCurrentBase(client);
		auto account = Hk::Client::GetAccountByClientID(client);
		for (auto& task : global->accountTasks[account].tasks)
		{
			if (task.isCompleted)
			{
				continue;
			}
			if (task.taskType == TaskType::GetItem && task.itemTarget == gbi.iGoodId)
			{
				task.quantityCompleted += gbi.iCount;
				if (task.quantityCompleted >= task.quantity)
				{
					task.isCompleted = true;
					SaveTaskStatusToJson(account);
					PrintUserCmdText(client, std::format(L"You have completed {}", stows(task.taskDescription)));
					Hk::Client::PlaySoundEffect(client, CreateID("ui_gain_level"));
					// TODO: Reward
				}
			}
			else if (task.taskType == TaskType::SellItem && task.baseTarget == base.value() && task.itemTarget == gbi.iGoodId)
			{
				task.quantityCompleted = std::clamp(task.quantityCompleted - gbi.iCount, 0, task.quantity);
			}
		}
	}
	// Function: Brief hook on item tractored to see if a task needs to be updated.
	void ItemTractored(ClientId& client, [[maybe_unused]] struct XTractorObjects const& objs)
	{
		// TODO: Create and implement this function.
	}
	// Function: Generates a daily task.
	void GenerateDailyTask(CAccount* account)
	{
		// Choose and create a random task from the available pool.
		const auto& randomTask = global->taskTypePool[RandomNumber(0, global->taskTypePool.size() - 1)];

		if (randomTask == TaskType::GetItem)
		{
			// Create an item acquisition task
			auto itemAcquisitionTarget = RandomIdKey(global->taskItemAcquisitionTargets);
			auto itemQuantity =
			    RandomNumber(global->taskItemAcquisitionTargets.at(itemAcquisitionTarget)[0], global->taskItemAcquisitionTargets.at(itemAcquisitionTarget)[1]);
			auto itemArch = Archetype::GetEquipment(itemAcquisitionTarget);
			auto taskDescription = std::format("Acquire {} units of {}.", itemQuantity, wstos(Hk::Message::GetWStringFromIdS(itemArch->iIdsName)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating an 'Acquire Items' task to '{}'", taskDescription));

			Task task;
			task.taskType = TaskType::KillNpc;
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
		if (randomTask == TaskType::KillNpc)
		{
			// Create an NPC kill task
			const auto& npcFactionTarget = RandomIdKey(global->taskNpcKillTargets);
			auto npcQuantity = RandomNumber(global->taskNpcKillTargets.at(npcFactionTarget)[0], global->taskNpcKillTargets.at(npcFactionTarget)[1]);
			uint npcFactionIds;
			pub::Reputation::GetGroupName(npcFactionTarget, npcFactionIds);
			auto taskDescription = std::format("Destroy {} ships belonging to the {}.", npcQuantity, wstos(Hk::Message::GetWStringFromIdS(npcFactionIds)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Kill NPCs' task to '{}'", taskDescription));

			Task task;
			task.taskType = TaskType::KillNpc;
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
		if (randomTask == TaskType::KillPlayer)
		{
			// Create a player kill task
			auto playerQuantity = RandomNumber(global->config->taskPlayerKillTargets[0], global->config->taskPlayerKillTargets[1]);
			auto taskDescription = std::format("Destroy {} player ships.", playerQuantity);
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Kill Players' task to '{}'", taskDescription));

			Task task;
			task.taskType = TaskType::KillPlayer;
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
		if (randomTask == TaskType::SellItem)
		{
			// Create a trade task
			const auto& tradeBaseTarget = global->taskTradeBaseTargets[RandomNumber(0, global->taskTradeBaseTargets.size() - 1)];
			auto tradeItemTarget = RandomIdKey(global->taskTradeItemTargets);
			auto tradeItemQuantity = RandomNumber(global->taskTradeItemTargets.at(tradeItemTarget)[0], global->taskTradeItemTargets.at(tradeItemTarget)[1]);
			auto baseArch = Universe::get_base(tradeBaseTarget);
			auto itemArch = Archetype::GetEquipment(tradeItemTarget);
			auto taskDescription = std::format("Sell {} units of {} at {}.",
			    tradeItemQuantity,
			    wstos(Hk::Message::GetWStringFromIdS(itemArch->iIdsName)),
			    wstos(Hk::Message::GetWStringFromIdS(baseArch->baseIdS)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Sell Cargo' task to '{}'", taskDescription));

			Task task;
			task.taskType = TaskType::SellItem;
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
				PrintUserCmdText(client,
				    std::format(L"{} Expires in {} hours. {}/{} remaining.", stows(task.taskDescription), taskExpiry, task.quantityCompleted, task.quantity));
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
	pi->name("Daily Tasks");
	pi->shortName("dailytasks");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->timers(&timers);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &OnLogin, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &ItemPurchased);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodSell, &ItemSold);
	// pi->emplaceHook(HookedCall::IServerImpl__TractorObjects, &ItemAcquired);
}