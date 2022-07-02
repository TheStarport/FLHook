#pragma once
#include <FLHook.hpp>
#include <plugin.h>
#include <spdlog/logger.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Plugins::Npc
{
	//! A struct that represents an npc that can be spawned
	struct Npc : Reflectable
	{
		std::string shipArch;
		std::string loadout;
		std::string iff;
		uint infocardId;
		uint infocard2Id;
		std::string pilot;
		std::string graph; // NOTHING, FIGHTER, TRANSPORT, GUNBOAT, CRUISER. Possibly (unconfirmed) MINER, CAPITAL, FREIGHTER

		uint shipArchId;
		uint loadoutId;
		uint iffId;
	};

	// A struct that represents a fleet that can be spawned
	struct Fleet : Reflectable
	{
		std::wstring name;
		std::map<std::wstring, int> member;
	};

	// A struct that represents an NPC that is spawned on startup
	struct StartupNpc : Reflectable
	{
		std::wstring name;
		std::string system;
		std::vector<float> position;
		std::vector<std::vector<float>> rotation;

		uint systemId;
		Matrix rotationMatrix;
		Vector positionVector;
	};

	//! Config data for this plugin
	struct Config : Reflectable
	{
		//! Map of npcs that can be spawned
		std::map<std::wstring, Npc> npcInfo;
		//! Map of fleets that can be spawned
		std::map<std::wstring, Fleet> fleetInfo;
		//! Vector of npcs that are spawned on startup
		std::vector<StartupNpc> startupNpcs;
		//! Vector containing Infocard Ids used for naming npcs
		std::vector<uint> npcInfocardIds {};
		//! The config file we load out of
		std::string File() override { return "flhook_plugins/npc_control.json"; }
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::vector<const char*> listGraphs {};
		std::vector<uint> spawnedNpcs{};
		std::shared_ptr<spdlog::logger> Log = nullptr;
	};
}

