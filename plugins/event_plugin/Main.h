#pragma once
#include <FLHook.h>
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
		std::string File() override { return "flhook_plugins/event.json"; }

		struct NPC_MISSION_REFLECTABLE : Reflectable
		{
			std::string nickname = "Example Nickname";
			std::string system = "ExampleSystem";
			std::string sector = "ExampleSector";
			std::string reputation = "ExampleReputation";
			int required_amount = 99;
			int current_amount = 0;
		};

		struct CARGO_MISSION_REFLECTABLE : Reflectable
		{
			std::string nickname = "Example Nickname";
			std::string base = "ExampleBase";
			std::string item = "ExampleItem";
			int required_amount = 99;
			int current_amount = 0;
		};

		CARGO_MISSION_REFLECTABLE example;
		NPC_MISSION_REFLECTABLE example2;

		// Reflectable fields
		std::map<std::string, CARGO_MISSION_REFLECTABLE> CargoMissions;//= {{example.nickname, example}};
		std::map<std::string, NPC_MISSION_REFLECTABLE> NpcMissions;//= {{example2.nickname, example2}};
	};

	struct Global final
	{
		// Map of base ID to mission structure
		std::vector<CARGO_MISSION> CargoMissions;

		// Map of repgroup ID to mission structure
		std::vector<NPC_MISSION> NpcMissions;

		// A return code to indicate to FLHook if we want the hook processing to
		// continue.
		ReturnCode returncode = ReturnCode::Default;

		std::unique_ptr<Config> config = nullptr;
	};
}