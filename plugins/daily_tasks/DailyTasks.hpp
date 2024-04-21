#pragma once

#include <FLHook.hpp>
#include <plugin.h>
#include <ranges>
#include <random>

namespace Plugins::DailyTasks
{
	// Loadable json configuration
	struct Config : Reflectable
	{
		std::string File() override { return "config/daily_tasks.json"; }
		//! The number of randomly generated tasks a player will receive each day.
		int taskQuantity = 3;
		//! The minimum possible amount of credits that can be awarded for completing a daily task.
		int minCreditsReward = 5000;
		//! The maximum possible amount of credits that can be awarded for completing a daily task.
		int maxCreditsReward = 10000;
		//! A pool of possible item rewards for completing a daily task.
		std::map<std::string, std::vector<int>> itemRewardPool {
		    {{"commodity_luxury_consumer_goods"}, {{5}, {10}}}, {{"commodity_diamonds"}, {{25}, {40}}}, {{"commodity_alien_artifacts"}, {{10}, {25}}}};
		//! Possible target bases for trade tasks.
		std::vector<std::string> taskTradeBaseTargets {{"li03_01_base"}, {"li03_02_base"}, {"li03_03_base"}};
		//! Possible target items and their minimum and maximum quantity for trade tasks.
		std::map<std::string, std::vector<int>> taskTradeItemTargets {
		    {{"commodity_cardamine"}, {{5}, {10}}}, {{"commodity_scrap_metal"}, {{25}, {40}}}, {{"commodity_construction_machinery"}, {{10}, {25}}}};
		//! Possible options for item acquisition tasks. Parameters are item and quantity.
		std::map<std::string, std::vector<int>> taskItemAcquisitionTargets {
		    {{"commodity_optronics"}, {{3}, {5}}}, {{"commodity_super_alloys"}, {{10}, {15}}}, {{"commodity_mox_fuel"}, {{8}, {16}}}};
		//! Possible options for NPC kill tasks. Parameters are target faction and quantity.
		std::map<std::string, std::vector<int>> taskNpcKillTargets = {{{"fc_x_grp"}, {{3}, {5}}}, {{"li_n_grp"}, {{10}, {15}}}};
		//! Possible options for player kill tasks. Parameters are minimum and maximum quantity of kills needed.
		std::vector<int> taskPlayerKillTargets = {{1}, {3}};
		//! The server hour at which players will be able to use the reset tasks command again.
		int resetTime = 12;
		//! The amount of time in seconds a player has to complete a set of assigned tasks before they get cleaned up during the hourly check or at login.
		int taskDuration = 86400;
		//! Whether or not to display a message with daily task data when a player logs in.
		bool displayMessage = true;
	};

	enum class TaskType
	{
		GetItem,
		KillNpc,
		KillPlayer,
		SellItem
	};

	struct Task : Reflectable
	{
		TaskType taskType = TaskType::GetItem;
		int quantity = 0;
		int quantityCompleted = 0;
		uint itemTarget = 0;
		uint baseTarget = 0;
		uint npcFactionTarget = 0;
		std::string taskDescription;
		bool isCompleted = false;
		uint setTime = 0;
	};

	struct Tasks : Reflectable
	{
		std::vector<Task> tasks;
	};

	struct Global
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::map<uint, bool> playerTaskAllocationStatus;
		std::map<uint, std::vector<int>> itemRewardPool;
		std::map<uint, std::vector<int>> taskTradeItemTargets;
		std::map<uint, std::vector<int>> taskItemAcquisitionTargets;
		std::map<uint, std::vector<int>> taskNpcKillTargets;
		std::vector<uint> taskTradeBaseTargets;
		std::vector<TaskType> taskTypePool;
		std::map<CAccount*, Tasks> accountTasks;
		std::map<CAccount*, bool> tasksReset;
		bool dailyReset;
		std::map<uint, float> goodList;
	};
} // namespace Plugins::DailyTasks