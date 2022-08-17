#pragma once

#include <FLHook.hpp>
#include <plugin.h>
#include <spdlog/logger.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Plugins::SolarControl
{
	struct StartupSolar : Reflectable
	{
		std::wstring name = L"largestation1";
		std::string system = "li01";
		std::vector<float> position = {-30367, 120, -25810};
		std::vector<float> rotation = {0, 0, 0};

		uint systemId{};
		Vector pos{};
		Matrix rot{};
	};

	struct SolarArch : Reflectable
	{
		std::string solarArch = "largestation1";
		std::string loadout = "space_station";
		std::string iff = "fc_uk_grp";
		uint infocard = 197808;
		std::string base = "li01_01_base";
		std::string pilot = "pilot_solar_hardest";

		uint solarArchId{};
		uint loadoutId{};
		uint baseId{};
	};

	struct Config : Reflectable
	{
		StartupSolar exampleStartupSolar;
		SolarArch exampleSolarArch;

		std::vector<StartupSolar> startupSolars = {exampleStartupSolar};
		std::map<std::wstring, SolarArch> solarArches = {{L"largestation1", exampleSolarArch}};
		//! The config file we load out of
		std::string File() override { return "flhook_plugins/solar_control.json"; }
	};

	//! Global data for this plugin
	struct Global final
	{
		std::map<uint, std::wstring> spawnedSolars;
		bool firstRun = true;
		ReturnCode returnCode = ReturnCode::Default;
		std::unique_ptr<Config> config = nullptr;
		std::shared_ptr<spdlog::logger> Log = nullptr;
	};
}
