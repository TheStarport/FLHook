#pragma once
#include <FLHook.hpp>
#include <plugin.h>
#include <nlohmann/json.hpp>

namespace Plugins::Event
{
	struct CARGO_MISSION
	{
		std::string nickname;
		uint base;
		uint item;
		int required_amount;
		int current_amount;
	};

	struct NPC_MISSION
	{
		std::string nickname;
		uint system;
		std::string sector;
		uint reputation;
		int required_amount;
		int current_amount;
	};

	//! Configurable fields for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "config/event.json"; }

		struct NPC_MISSION_REFLECTABLE : Reflectable
		{
			std::string nickname = "Example";
			std::string system = "li01";
			std::string sector = "D-6";
			std::string reputation = "fc_lr_grp";
			int required_amount = 99;
			int current_amount = 0;
		};

		struct CARGO_MISSION_REFLECTABLE : Reflectable
		{
			std::string nickname = "Example";
			std::string base = "li01_01_base";
			std::string item = "commodity_gold";
			int required_amount = 99;
			int current_amount = 0;
		};

		CARGO_MISSION_REFLECTABLE example;
		NPC_MISSION_REFLECTABLE example2;

		// Reflectable fields
		std::map<std::string, CARGO_MISSION_REFLECTABLE> CargoMissions = {{example.nickname, example}};
		std::map<std::string, NPC_MISSION_REFLECTABLE> NpcMissions = {{example2.nickname, example2}};
	};

	struct Global final
	{
		// Map of base Id to mission structure
		std::vector<CARGO_MISSION> CargoMissions;

		// Map of repgroup Id to mission structure
		std::vector<NPC_MISSION> NpcMissions;

		std::map<uint, std::string> nicknameToNameMap;

		// A return code to indicate to FLHook if we want the hook processing to
		// continue.
		ReturnCode returncode = ReturnCode::Default;

		std::unique_ptr<Config> config = nullptr;
	};
} // namespace Plugins::Event