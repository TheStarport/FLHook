#pragma once

#include "Shlwapi.h"

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Restart
{
	struct Restart final
	{
		std::wstring characterName;
		std::string restartFile;
		std::wstring directory;
		std::wstring characterFile;
		int cash;
	};

	struct Config final : Reflectable
	{
		// Players with a cash above this value cannot use the restart command.
		int maxCash = 1000000;

		// Players with a rank above this value cannot use the restart command.
		int maxRank = 5;

		bool enableRestartCost;

		std::string File() override { return "flhook_plugins/restarts.json"; }
		std::map<std::wstring, int> restartCosts;
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::list<Restart> pendingRestarts;
	};
}