#pragma once
#include <FLHook.hpp>
#include <plugin.h>
#include <spdlog/logger.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace Plugins::Npc
{
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

	struct Fleet : Reflectable
	{
		std::wstring name;
		std::map<std::wstring, int> member;
	};

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

	struct Config : Reflectable
	{
		std::map<std::wstring, Npc> npcInfo;
		std::map<std::wstring, Fleet> fleetInfo;
		std::vector<StartupNpc> startupNpcs;
		std::vector<uint> npcInfocardIds {};
		std::string File() override { return "flhook_plugins/npc_control.json"; }
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::vector<const char*> listGraphs {};
		std::vector<uint> spawnedNpcs{};
		std::shared_ptr<spdlog::logger> Log = nullptr;
	};
}

