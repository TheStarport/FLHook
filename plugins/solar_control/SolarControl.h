#pragma once
#include <FLHook.hpp>
#include <plugin.h>
#include <spdlog/logger.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Plugins::SolarControl
{
	struct StartupSolar final : Reflectable
	{
		std::wstring name = L"largestation1";
		std::string system = "li01";
		std::vector<float> position = {-30367, 120, -25810};
		std::vector<float> rotation = {0, 0, 0};

		uint systemId {};
		Vector pos {};
		Matrix rot {};
	};

	struct SolarArch final : Reflectable
	{
		std::string solarArch = "largestation1";
		std::string loadout;
		std::string iff;
		uint infocard = 197808;
		std::string base;
		std::string pilot;

		uint solarArchId {};
		uint loadoutId {};
		uint baseId {};
	};

	struct SolarArchFormationComponent final : Reflectable
	{
		std::string solarArchName;
		std::vector<float> relativePosition;
		std::vector<float> rotation;
	};

	struct SolarArchFormation final : Reflectable
	{
		std::vector<SolarArchFormationComponent> components;
	};

	struct Config final : Reflectable
	{
		std::vector<StartupSolar> startupSolars = {StartupSolar()};
		std::map<std::wstring, SolarArch> solarArches = {{L"largestation1", SolarArch()}};
		std::map<std::string, std::string> baseRedirects = {{"li01_01_base", "li01_02_base"}};
		std::map<uint, uint> hashedBaseRedirects;
		//! The config file we load out of
		std::string File() override { return "config/solar.json"; }
		std::map<std::wstring, SolarArchFormation> solarArchFormations;
	};

	//! Communicator class for this plugin. This is used by other plugins
	class SolarCommunicator : public PluginCommunicator
	{
	  public:
		inline static const char* pluginName = "Solar Control";
		explicit SolarCommunicator(const std::string& plug);

		uint PluginCall(CreateSolar, const std::wstring& name, Vector position, const Matrix& rotation, SystemId system, bool varyPosition, bool mission);
		std::vector<uint> PluginCall(CreateSolarFormation, const std::wstring& formation, const Vector& position, uint system);
	};

	//! Global data for this plugin
	struct Global final
	{
		std::vector<uint> pendingDockingRequests;
		std::map<uint, pub::SpaceObj::SolarInfo> spawnedSolars;
		bool firstRun = true;
		ReturnCode returnCode = ReturnCode::Default;
		std::unique_ptr<Config> config = nullptr;
		std::shared_ptr<spdlog::logger> log = nullptr;
		SolarCommunicator* communicator = nullptr;
		std::map<uint, uint> pendingRedirects;
	};
} // namespace Plugins::SolarControl