#pragma once
#include <FLHook.hpp>
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <plugin.h>

namespace Plugins::Stats
{
	struct FileName : Reflectable
	{
		std::string File() override { return "flhook_plugins/Stats.json"; }
		std::string FilePath = "EXPORTS\\stats.json";
	};

	struct Global final
	{
		// Global Variables
		ReturnCode returncode = ReturnCode::Default;

		FileName jsonFileName;
		std::map<uint, std::wstring> Ships;
	};
}
