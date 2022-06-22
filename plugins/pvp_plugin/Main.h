#pragma once
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Pvp
{
	struct Duel
	{
		uint clientId;
		uint clientId2;
		int amount;
		bool accepted;
	};

	struct Contestant
	{
		bool accepted;
		bool loser;
	};

	struct FreeForAll
	{
		std::map<uint, Contestant> contestants;
		int entryAmount;
		int pot;
	};

	struct Global final
	{
		ReturnCode returnCode = ReturnCode::Default;
		std::list<Duel> duels;
		std::map<uint, FreeForAll> freeForAlls; // uint is iSystemID
	};
}
