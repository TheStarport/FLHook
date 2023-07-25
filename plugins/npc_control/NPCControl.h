#pragma once
#include <FLHook.hpp>
#include <plugin.h>
#include <spdlog/logger.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <random>

namespace Plugins::Npc
{
	//! A struct that represents an npc that can be spawned
	struct Npc : Reflectable
	{
		std::string shipArch = "ge_fighter";
		std::string loadout = "MP_ge_fighter";
		std::string iff = "fc_fl_grp";
		uint iffId;
		uint infocardId = 197808;
		uint infocard2Id = 197809;
		std::string pilot = "pilot_pirate_ace";
		std::string graph = "FIGHTER"; // NOTHING, FIGHTER, TRANSPORT, GUNBOAT, CRUISER. Possibly (unconfirmed) MINER, CAPITAL, FREIGHTER

		uint shipArchId;
		uint loadoutId;
	};

	// A struct that represents a fleet that can be spawned
	struct Fleet : Reflectable
	{
		std::wstring name = L"example";
		std::map<std::wstring, int> member = {{L"example", 5}};
	};

	// A struct that represents an NPC that is spawned on startup
	struct StartupNpc : Reflectable
	{
		std::wstring name = L"example";
		std::string system = "li01";
		float spawnChance = 1;
		std::vector<float> position = {-33367, 120, -28810};
		std::vector<float> rotation = {0, 0, 0};

		uint systemId = 0;
		Matrix rotationMatrix = {0, 0, 0};
		Vector positionVector = {0, 0, 0};
	};

	//! Config data for this plugin
	struct Config : Reflectable
	{
		//! Map of npcs that can be spawned
		std::map<std::wstring, Npc> npcInfo = {{L"example", Npc()}};
		//! Map of fleets that can be spawned
		std::map<std::wstring, Fleet> fleetInfo = {{L"example", Fleet()}};
		//! Vector of npcs that are spawned on startup
		std::vector<StartupNpc> startupNpcs = {StartupNpc()};
		//! Vector containing Infocard Ids used for naming npcs
		std::vector<uint> npcInfocardIds {197808};
		//! The config file we load out of
		std::string File() override { return "config/npc.json"; }
	};

	//! Communicator class for this plugin. This is used by other plugins
	class NpcCommunicator : public PluginCommunicator
	{
	  public:
		inline static const char* pluginName = "NPC Control";
		explicit NpcCommunicator(const std::string& plug);

		uint PluginCall(CreateNpc, const std::wstring& name, Vector position, const Matrix& rotation, SystemId systemId, bool varyPosition);
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::vector<const char*> listGraphs {};
		std::vector<uint> spawnedNpcs {};
		std::shared_ptr<spdlog::logger> Log = nullptr;
		uint dockNpc = 0;
		NpcCommunicator* communicator = nullptr;
	};
} // namespace Plugins::Npc
