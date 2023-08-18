#pragma once
#include <FLHook.hpp>
#include <plugin.h>
#include "../npc_control/NPCControl.h"
#include "../solar_control/SolarControl.h"

namespace Plugins::WaveDefence
{
	struct CostumeStrings : Reflectable
	{
		std::string head = "sh_male1_head";
		std::string body = "pi_orillion_body";
		std::string lefthand = "benchmark_male_hand_left";
		std::string righthand = "benchmark_male_hand_right";
		std::vector<std::string> accessory = {{"prop_shades_04"}};
		int accessories = 0;
	};

	struct Character : Reflectable
	{
		// Non-reflectable
		uint voiceId = 0;
		Costume costume;

		// Reflectable
		uint infocard = 13015;
		std::string voice = "mc_leg_m01";
		CostumeStrings costumeStrings = CostumeStrings();
	};

	struct VoiceLine : Reflectable
	{
		// Reflectable
		std::string voiceLineString = "rmb_morehostilesatwaypoint_01 -";
		std::string character = "mc_leg_m01";

		// Non-reflectable
		ID_String voiceLine;
	};

	struct Wave : Reflectable
	{
		std::vector<std::wstring> npcs = {{L"example"}, {L"example"}};
		std::vector<std::wstring> variableNpcs = {{L"example"}, {L"example"}};
		std::vector<std::wstring> solars = {{L"weapon_platform"}};
		uint reward = 1000;
		VoiceLine startVoiceLine = VoiceLine();
		VoiceLine endVoiceLine = VoiceLine();
	};

	struct System : Reflectable
	{
		// Non-reflectable properties
		uint systemId = 0;
		Vector positionVector;

		// Reflectable properties
		std::vector<Wave> waves = {{Wave()}, {Wave()}};
		std::string system = "li01";
		float posX = 0;
		float posY = 0;
		float posZ = 0;
	};

	struct Game
	{
		bool started = false;
		uint waveNumber = 0;
		uint groupId = 0;
		std::vector<uint> members;
		std::vector<uint> spawnedNpcs;
		std::vector<uint> spawnedSolars;
		System system;
	};

	struct Config : Reflectable
	{
		std::vector<System> systems = {{System()}, {System()}};
		std::vector<Character> characters = {{Character(), Character()}};

		// Music
		std::string victoryMusic = "music_victory_long";
		std::string failureMusic = "music_failure";
		pub::Audio::Tryptich victoryMusicId;
		pub::Audio::Tryptich failureMusicId;

		// How many variable npcs per player
		uint npcMultiplier = 2;

		// Fleeing message
		std::wstring fleeingMessage = L"{} has fled the battle.";

		//! The config file we load out of
		std::string File() override { return "config/wave_defence.json"; }
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;

		std::vector<Game> games;
		std::vector<uint> systemsPendingNewWave;

		Plugins::Npc::NpcCommunicator* npcCommunicator = nullptr;
		Plugins::SolarControl::SolarCommunicator* solarCommunicator = nullptr;
	};
} // namespace Plugins::WaveDefence
