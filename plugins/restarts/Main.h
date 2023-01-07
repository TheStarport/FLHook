#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Restart
{
	//! A struct containing a pending restart
	struct Restart final
	{
		//! Name of the character
		std::wstring characterName;
		//! The name of the restart .fl file they wish to be applied to them
		std::string restartFile;
		//! The directory of the character file
		std::wstring directory;
		//! The name of the character file
		std::wstring characterFile;
		//! The amount of cash to be saved to the character file
		uint cash;
	};

	//! Config data for this plugin
	struct Config final : Reflectable
	{
		//! Players with a cash above this value cannot use the restart command.
		uint maxCash = 1000000;

		//! Players with a rank above this value cannot use the restart command.
		int maxRank = 5;

		//! Where to enable the restarts costing money or being free
		bool enableRestartCost;

		//! File path of the json config file
		std::string File() override { return "config/restarts.json"; }

		//! A map of restart names and their cost
		std::map<std::wstring, uint> availableRestarts;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;

		//! A vector of currently pending restarts
		std::vector<Restart> pendingRestarts;
	};
}