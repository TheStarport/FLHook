#pragma once
#include <FLHook.hpp>
#include <plugin.h>

struct Costume
{
	uint body = 0;
	uint head = 0;
	uint lefthand = 0;
	uint righthand = 0;
	uint accessory[8] = {};
	int accessories = 0;
};

namespace Plugins::WaveDefence
{
	struct CostumeStrings : Reflectable
	{
		std::string body;
		std::string head;
		std::string lefthand;
		std::string righthand;
		std::vector<std::string> accessory;
		int accessories = 0;
	};

	struct Character : Reflectable
	{
		// Non-reflectable
		uint voiceId = 0;
		Costume costume;

		// Reflectable
		uint infocard = 0;
		std::string voice;
		CostumeStrings costumeStrings;
	};

	struct VoiceLine : Reflectable
	{
		// Reflectable
		std::string voiceLineString = "rmb_morehostilesatwaypoint_01 -";
		
		// Non-reflectable
		ID_String voiceLine;
	};

	struct Wave : Reflectable
	{
		std::vector<std::string> npcs;
		uint reward = 0;
		VoiceLine startVoiceLine;
		VoiceLine endVoiceLine;
	};

	struct System : Reflectable
	{
		// Non-reflectable properties
		uint systemId = 0;
		Vector positionVector;

		// Reflectable properties
		std::vector<Wave> waves;
		std::string system;
		float posX = 0;
		float posY = 0;
		float posZ = 0;
		Character character;
	};

	struct Game
	{
		uint waveNumber = 0;
		uint groupId = 0;
		std::vector<uint> members;
		std::vector<uint> spawnedNpcs;
		System system;
	};

	struct Config : Reflectable
	{
		std::vector<System> systems;
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::vector<Game> games;
	};
} // namespace Plugins::WaveDefence
