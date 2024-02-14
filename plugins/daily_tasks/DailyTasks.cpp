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
 * There are no admin commands in this plugin.
 * @paragraph configuration Configuration
 * @code
 *{
 *   "itemRewardPool": {
 *       "commodity_alien_artifacts": [
 *           10,
 *           25
 *       ],
 *       "commodity_diamonds": [
 *           25,
 *           40
 *       ],
 *       "commodity_luxury_consumer_goods": [
 *           5,
 *           10
 *       ]
 *   },
 *   "maxCreditsReward": 10000,
 *   "minCreditsReward": 5000,
 *   "resetTime": 12,
 *   "taskDuration": 86400,
 *   "taskItemAcquisitionTargets": {
 *       "commodity_mox_fuel": [
 *           8,
 *           16
 *       ],
 *       "commodity_optronics": [
 *           3,
 *           5
 *       ],
 *       "commodity_super_alloys": [
 *           10,
 *           15
 *       ]
 *   },
 *   "taskNpcKillTargets": {
 *       "fc_x_grp": [
 *           3,
 *           5
 *       ],
 *       "li_n_grp": [
 *           10,
 *           15
 *       ]
 *   },
 *   "taskPlayerKillTargets": [
 *       1,
 *       3
 *   ],
 *   "taskQuantity": 3,
 *   "taskTradeBaseTargets": [
 *       "li03_01_base",
 *       "li03_02_base",
 *       "li03_03_base"
 *   ],
 *   "taskTradeItemTargets": {
 *       "commodity_cardamine": [
 *           5,
 *           10
 *       ],
 *       "commodity_construction_machinery": [
 *           10,
 *           25
 *       ],
 *       "commodity_scrap_metal": [
 *           25,
 *           40
 *       ]
 *   }
 *}
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no optional dependencies.
 */

// Includes
#include "DailyTasks.hpp"

namespace Plugins::DailyTasks
{
	const auto global = std::make_unique<Global>();

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

	/** @ingroup DailyTasks
	 * @brief Fetches the value of goods defined in itemRewardPool for later use.
	 */
	void GetGoodBaseValues()
	{
		for (auto& good : *GoodList_get()->get_list())
		{
			if ((good->iType == 0 || good->iType == 1) && good->fPrice != 0 && global->itemRewardPool.contains(good->iArchId))
			{
				global->goodList.insert({good->iArchId, good->fPrice});
				auto ids = good->iIdSName;

				auto var = Hk::Message::GetWStringFromIdS(ids);
				AddLog(LogType::Normal, LogLevel::Debug, std::format("Load prices in for {}", wstos(var)));
			}
		}
		AddLog(LogType::Normal, LogLevel::Debug, std::format("Loaded {} goods into the reward pool", global->goodList.size()));
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

	/** @ingroup DailyTasks
	 * @brief Saves the task status of an account to the appropriate tasks.json file.
	 */

	void SaveTaskStatusToJson(CAccount* account)
	{
		auto& taskList = global->accountTasks.at(account);
		auto taskJsonPath = Hk::Client::GetAccountDirName(account);

		char dataPath[MAX_PATH];
		GetUserDataPath(dataPath);
		Serializer::SaveToJson(taskList, std::format("{}\\Accts\\MultiPlayer\\{}\\daily_tasks.json", dataPath, wstos(taskJsonPath)));
		AddLog(LogType::Normal,
		    LogLevel::Debug,
		    std::format("Saving a task status update to {}\\Accts\\MultiPlayer\\{}\\daily_tasks.json", dataPath, wstos(taskJsonPath)));
	}

	/** @ingroup DailyTasks
	 * @brief Writes the status of an account to the appropriate tasks.json file.
	 */

	void LoadTaskStatusFromJson(CAccount* account)
	{
		auto taskJsonPath = Hk::Client::GetAccountDirName(account);
		char dataPath[MAX_PATH];
		GetUserDataPath(dataPath);
		auto taskList = Serializer::JsonToObject<Tasks>(std::format("{}\\Accts\\MultiPlayer\\{}\\daily_tasks.json", dataPath, wstos(taskJsonPath)), true);
		global->accountTasks[account] = taskList;
	}

	/** @ingroup DailyTasks
	 * @brief Generates random rewards for users who complete daily tasks, based on parameters set in daily_tasks.json.
	 */

	void GenerateReward(ClientId& client, float holdSize = 0.f)
	{
		auto creditReward = RandomNumber(global->config->minCreditsReward, global->config->maxCreditsReward);
		auto itemReward = RandomIdKey(global->itemRewardPool);
		auto itemQuantity = RandomNumber(global->itemRewardPool[itemReward][0], global->itemRewardPool[itemReward][1]);
		int surplusCreditReward = 0;

		if (itemQuantity > static_cast<int>(holdSize))
		{
			surplusCreditReward = ((static_cast<int>(holdSize) - itemQuantity) * -1) * static_cast<int>(global->goodList[itemReward]);
			itemQuantity = static_cast<int>(holdSize);
		}

		Hk::Player::AddCash(client, creditReward + surplusCreditReward);
		if (itemQuantity > 0)
		{
			// Hk::Player::AddCargo causes a kick here, so we have to do this with the pub function
			pub::Player::AddCargo(client, itemReward, itemQuantity, 1, false);
		}
		PrintUserCmdText(client,
		    std::format(L"Task completed! You have been awarded {} credits and {} units of {}.",
		        creditReward + surplusCreditReward,
		        itemQuantity,
		        Hk::Message::GetWStringFromIdS(Archetype::GetEquipment(itemReward)->iIdsName)));
	}

	/** @ingroup DailyTasks
	 * @brief Hook on ShipDestroyed to see if a task needs to be updated.
	 */

	void ShipDestroyed([[maybe_unused]] DamageList** damage, const DWORD** ecx, const uint& kill)
	{
		if (kill == 1)
		{
			const CShip* ship = Hk::Player::CShipFromShipDestroyed(ecx);
			if (ClientId client = ship->GetOwnerPlayer())
			{
				const DamageList* dmg = *damage;
				const auto killerId = Hk::Client::GetClientIdByShip(
				    dmg->get_cause() == DamageCause::Unknown ? ClientInfo[client].dmgLast.get_inflictor_id() : dmg->get_inflictor_id());
				const auto victimId = Hk::Client::GetClientIdByShip(ship->get_id());
				for (auto& task : global->accountTasks[Hk::Client::GetAccountByClientID(killerId.value())].tasks)
				{
					if (task.taskType == TaskType::KillPlayer && !task.isCompleted && victimId.has_value())
					{
						task.quantityCompleted++;
					}
					if (task.quantityCompleted == task.quantity && task.taskType == TaskType::KillPlayer && task.isCompleted == false)
					{
						task.isCompleted = true;
						SaveTaskStatusToJson(Hk::Client::GetAccountByClientID(killerId.value()));
						PrintUserCmdText(client, std::format(L"You have completed {}", stows(task.taskDescription)));
						Hk::Client::PlaySoundEffect(client, CreateID("ui_gain_level"));
						GenerateReward(client);
					}
				}
			}
			else
			{
				const DamageList* dmg = *damage;
				const auto killerId = Hk::Client::GetClientIdByShip(
				    dmg->get_cause() == DamageCause::Unknown ? ClientInfo[client].dmgLast.get_inflictor_id() : dmg->get_inflictor_id());
				int reputation;
				pub::SpaceObj::GetRep(ship->get_id(), reputation);
				uint affiliation;
				pub::Reputation::GetAffiliation(reputation, affiliation);

				for (auto& task : global->accountTasks[Hk::Client::GetAccountByClientID(killerId.value())].tasks)
				{
					if (task.taskType == TaskType::KillNpc && !task.isCompleted && task.npcFactionTarget == affiliation)
					{
						task.quantityCompleted++;
					}
					if (task.quantityCompleted == task.quantity && task.taskType == TaskType::KillNpc && !task.isCompleted &&
					    task.npcFactionTarget == affiliation)
					{
						task.isCompleted = true;
						SaveTaskStatusToJson(Hk::Client::GetAccountByClientID(killerId.value()));
						PrintUserCmdText(client, std::format(L"You have completed {}", stows(task.taskDescription)));
						Hk::Client::PlaySoundEffect(client, CreateID("ui_gain_level"));
						GenerateReward(client);
					}
				}
			}
		}
	}

	/** @ingroup DailyTasks
	 * @brief Hook on GFGoodSell to see if a task needs to be updated.
	 */
	void ItemSold(const struct SGFGoodSellInfo& gsi, ClientId& client)
	{
		auto base = Hk::Player::GetCurrentBase(client);
		auto account = Hk::Client::GetAccountByClientID(client);
		auto remainingHoldSize = 0.f;
		pub::Player::GetRemainingHoldSize(client, remainingHoldSize);
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
					GenerateReward(client, remainingHoldSize);
				}
			}
			else if (task.taskType == TaskType::GetItem && task.itemTarget == gsi.iArchId)
			{
				task.quantityCompleted = std::clamp(task.quantityCompleted - gsi.iCount, 0, task.quantity);
				SaveTaskStatusToJson(account);
			}
		}
	}
	/** @ingroup DailyTasks
	 * @brief Hook on GFGoodBuy to see if a task needs to be updated.
	 */
	void ItemPurchased(SGFGoodBuyInfo const& gbi, ClientId& client)
	{
		auto base = Hk::Player::GetCurrentBase(client);
		auto account = Hk::Client::GetAccountByClientID(client);
		auto remainingHoldSize = 0.f;
		pub::Player::GetRemainingHoldSize(client, remainingHoldSize);
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
					auto purchasedCargoAmount = static_cast<float>(gbi.iCount);
					GenerateReward(client, remainingHoldSize - purchasedCargoAmount);
				}
			}
			else if (task.taskType == TaskType::SellItem && task.baseTarget == base.value() && task.itemTarget == gbi.iGoodId)
			{
				task.quantityCompleted = std::clamp(task.quantityCompleted - gbi.iCount, 0, task.quantity);
				SaveTaskStatusToJson(account);
			}
		}
	}

	void GenerateIndividualTask(CAccount* account, TaskType taskType)
	{
		using enum TaskType;
		Task task;
		task.isCompleted = false;
		task.quantityCompleted = 0;
		task.setTime = Hk::Time::GetUnixSeconds();

		if (taskType == GetItem)
		{
			auto itemAcquisitionTarget = RandomIdKey(global->taskItemAcquisitionTargets);
			auto itemArch = Archetype::GetEquipment(itemAcquisitionTarget);
			if (!itemArch)
			{
				AddLog(LogType::Normal, LogLevel::Err, "Failed to generate a GetItem task. No valid item ArchID was found. The task was not created.");
				return;
			}
			task.taskType = GetItem;
			task.itemTarget = itemAcquisitionTarget;
			task.quantity =
			    RandomNumber(global->taskItemAcquisitionTargets.at(itemAcquisitionTarget)[0], global->taskItemAcquisitionTargets.at(itemAcquisitionTarget)[1]);
			task.taskDescription = std::format("Buy {} units of {}.", task.quantity, wstos(Hk::Message::GetWStringFromIdS(itemArch->iIdsName)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating an 'Acquire Items' task to '{}'", task.taskDescription));
		}
		else if (taskType == KillNpc)
		{
			const auto& npcFactionTarget = RandomIdKey(global->taskNpcKillTargets);
			auto npcQuantity = RandomNumber(global->taskNpcKillTargets.at(npcFactionTarget)[0], global->taskNpcKillTargets.at(npcFactionTarget)[1]);
			uint npcFactionIds;
			pub::Reputation::GetGroupName(npcFactionTarget, npcFactionIds);

			task.taskType = KillNpc;
			task.taskDescription = std::format("Destroy {} ships belonging to {}.", npcQuantity, wstos(Hk::Message::GetWStringFromIdS(npcFactionIds)));
			task.npcFactionTarget = npcFactionTarget;
			task.quantity = npcQuantity;
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Kill NPCs' task to '{}'", task.taskDescription));
		}
		else if (taskType == KillPlayer)
		{
			task.taskType = KillPlayer;
			task.quantity = RandomNumber(global->config->taskPlayerKillTargets[0], global->config->taskPlayerKillTargets[1]);
			task.taskDescription = std::format("Destroy {} player ships.", task.quantity);
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Kill Players' task to '{}'", task.taskDescription));
		}
		else if (taskType == SellItem)
		{
			const auto& tradeBaseTarget = global->taskTradeBaseTargets[RandomNumber(0, global->taskTradeBaseTargets.size() - 1)];
			auto tradeItemTarget = RandomIdKey(global->taskTradeItemTargets);
			auto baseArch = Universe::get_base(tradeBaseTarget);
			auto itemArch = Archetype::GetEquipment(tradeItemTarget);

			if (!baseArch || !itemArch)
			{
				AddLog(LogType::Normal,
				    LogLevel::Err,
				    "Failed to generate a SellItem task. No valid item ArchID or base BaseID was found. The task was not created.");
				return;
			}

			task.taskType = SellItem;
			task.baseTarget = tradeBaseTarget;
			task.itemTarget = tradeItemTarget;
			task.quantity = RandomNumber(global->taskTradeItemTargets.at(tradeItemTarget)[0], global->taskTradeItemTargets.at(tradeItemTarget)[1]);
			task.taskDescription = std::format("Sell {} units of {} at {}.",
			    task.quantity,
			    wstos(Hk::Message::GetWStringFromIdS(itemArch->iIdsName)),
			    wstos(Hk::Message::GetWStringFromIdS(baseArch->baseIdS)));
			AddLog(LogType::Normal, LogLevel::Debug, std::format("Creating a 'Sell Cargo' task to '{}'", task.taskDescription));
		}
		if (!global->accountTasks.contains(account))
		{
			global->accountTasks[account] = {};
		}
		global->accountTasks[account].tasks.emplace_back(task);
	}

	/** @ingroup DailyTasks
	 * @brief Generates a daily task for a player and writes it to their config.json
	 */
	void GenerateDailyTask(CAccount* account)
	{
		using enum TaskType;
		// Choose and create a random task from the available pool.
		const auto& randomTask = global->taskTypePool[RandomNumber(0, global->taskTypePool.size() - 1)];

		if (randomTask == GetItem)
		{
			GenerateIndividualTask(account, GetItem);
		}
		if (randomTask == KillNpc)
		{
			GenerateIndividualTask(account, KillNpc);
		}
		if (randomTask == KillPlayer)
		{
			GenerateIndividualTask(account, KillPlayer);
		}
		if (randomTask == SellItem)
		{
			GenerateIndividualTask(account, SellItem);
		}
	}

	/** @ingroup DailyTasks
	 * @brief Hook OnBaseEnter to save any task updates to the user's config.json file.
	 */
	void SaveTaskStatusOnBaseEnter([[maybe_unused]] BaseId& baseId, ClientId& client)
	{
		auto account = Hk::Client::GetAccountByClientID(client);
		SaveTaskStatusToJson(account);
	}

	void PrintTasks(ClientId& client)
	{
		auto account = Hk::Client::GetAccountByClientID(client);
		PrintUserCmdText(client, L"CURRENT DAILY TASKS");
		for (auto& task : global->accountTasks[account].tasks)
		{
			int taskExpiry = ((86400 - (Hk::Time::GetUnixSeconds() - task.setTime)) / 60) / 60;
			if (!task.isCompleted)
			{
				PrintUserCmdText(client,
				    std::format(
				        L"{} | Expires in {} hours | {}/{} remaining.", stows(task.taskDescription), taskExpiry, task.quantityCompleted, task.quantity));
			}
			else
			{
				PrintUserCmdText(client, stows(task.taskDescription + " | TASK COMPLETED"));
			}
		}
	}

	/** @ingroup DailyTasks
	 * @brief Hook on PlayerLaunch to display the task list when the player undocks.
	 */
	void DisplayTasksOnLaunch([[maybe_unused]] const uint& ship, ClientId& client)
	{
		PrintTasks(client);
		PrintUserCmdText(client, L"To view this list again, type /showtasks in chat.");
	}

	/** @ingroup DailyTasks
	 * @brief Iterates over online players and checks the time status of their tasks, clearing and resetting them if they exceed 24 hours.
	 */
	void DailyTaskCheck()
	{
		auto onlinePlayers = Hk::Admin::GetPlayers();
		auto currentTime = Hk::Time::GetUnixSeconds();
		for (const auto& players : onlinePlayers)
		{
			auto account = Hk::Client::GetAccountByClientID(players.client);
			auto accountId = account->wszAccId;
			for (const auto& tasks : global->accountTasks[account].tasks)
			{
				if ((currentTime - tasks.setTime) < 86400)
				{
					return;
				}
				AddLog(LogType::Normal, LogLevel::Debug, std::format("Tasks for {} are out of date, refreshing and creating new tasks...", wstos(accountId)));
				global->accountTasks[account].tasks.erase(global->accountTasks[account].tasks.begin(), global->accountTasks[account].tasks.end());
				for (int iterator = 0; iterator < global->config->taskQuantity; iterator++)
				{
					GenerateDailyTask(account);
				}
			}
		}
	}

	/** @ingroup DailyTasks
	 * @brief Keeps track of time and initiates cleanup once per day.
	 */
	void DailyTimerTick()
	{
		// Checks the current hour to see if global->dailyReset should be flipped back to false
		if (std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now().time_since_epoch()).count() % 24 == global->config->resetTime ||
		    std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now().time_since_epoch()).count() % 24 == global->config->resetTime + 1 &&
		        global->dailyReset == false)
		{
			global->dailyReset = true;
			global->tasksReset.clear();
		}
		else
		{
			global->dailyReset = false;
		}
		DailyTaskCheck();
	}

	/** @ingroup DailyTasks
	 * @brief A user command to show the tasks a player currently has and their status.
	 */
	void UserCmdShowDailyTasks(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		PrintTasks(client);
	}

	/** @ingroup DailyTasks
	 * @brief A user command to reset the user's tasks and reroll them if they have not already started to complete them.
	 */
	void UserCmdResetDailyTasks(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		auto account = Hk::Client::GetAccountByClientID(client);
		auto accountId = account->wszAccId;

		for (const auto& tasks : global->accountTasks[account].tasks)
		{
			if (tasks.isCompleted)
			{
				PrintUserCmdText(client,
				    std::format(L"You have completed one or more of your daily tasks today, and cannot reset them until {}:00", global->config->resetTime));
				return;
			}
		}

		if (global->tasksReset[account] == false)
		{
			AddLog(LogType::Normal, LogLevel::Debug, std::format("{} is resetting their daily tasks.", wstos(accountId)));

			global->accountTasks[account].tasks.erase(global->accountTasks[account].tasks.begin(), global->accountTasks[account].tasks.end());
			for (int iterator = 0; iterator < global->config->taskQuantity; iterator++)
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

	/** @ingroup DailyTasks
	 * @brief Hook on Login to check a player's task status and assign new tasks if they don't have any or their previous ones had expired.
	 */
	void OnLogin([[maybe_unused]] struct SLoginInfo const& login, ClientId& client)
	{
		auto account = Hk::Client::GetAccountByClientID(client);
		auto accountId = account->wszAccId;
		LoadTaskStatusFromJson(account);
		AddLog(LogType::Normal, LogLevel::Debug, std::format("Loading tasks for {} from stored json file...", wstos(accountId)));

		if (global->accountTasks[account].tasks.empty())
		{
			AddLog(LogType::Normal, LogLevel::Debug, std::format("No tasks saved for {}, creating new tasks...", wstos(accountId)));
			for (int iterator = 0; iterator < global->config->taskQuantity; iterator++)
			{
				GenerateDailyTask(account);
			}
			SaveTaskStatusToJson(account);
			return;
		}
		else
		{
			// If tasks are older than 24 hours, refresh them.
			if ((Hk::Time::GetUnixSeconds() - global->accountTasks[account].tasks[0].setTime) > 86400)
			{
				AddLog(LogType::Normal, LogLevel::Debug, std::format("Tasks for {} are out of date, refreshing and creating new tasks...", wstos(accountId)));
				global->accountTasks[account].tasks.erase(global->accountTasks[account].tasks.begin(), global->accountTasks[account].tasks.end());
				for (int iterator = 0; iterator < global->config->taskQuantity; iterator++)
				{
					GenerateDailyTask(account);
				}
			}

			SaveTaskStatusToJson(account);
			return;
		}
	}

	const std::vector commands = {{
	    CreateUserCommand(L"/showtasks", L"", UserCmdShowDailyTasks, L"Shows a list of current daily tasks for the user"),
	    CreateUserCommand(L"/resettasks", L"", UserCmdResetDailyTasks, L"Resets the user's daily tasks if none have already been completed"),
	}};

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
	pi->emplaceHook(HookedCall::IServerImpl__Startup, &GetGoodBaseValues, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &OnLogin, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &ItemPurchased, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodSell, &ItemSold, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &SaveTaskStatusOnBaseEnter, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &DisplayTasksOnLaunch);
}