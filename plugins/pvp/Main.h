#pragma once
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Pvp
{
	//! A struct to hold a duel between two players. This holds the amount of cash they're betting on, and whether it's been accepted or not
	struct Duel
	{
		uint clientId;
		uint clientId2;
		int amount;
		bool accepted;
	};

	//! A struct to hold a contestant for a Free-For-All
	struct Contestant
	{
		bool accepted;
		bool loser;
	};

	//! A struct to hold a Free-For-All competition. This holds the contestants, how much it costs to enter, and the total pot to be won by the eventual winner
	struct FreeForAll
	{
		std::map<uint, Contestant> contestants;
		int entryAmount;
		int pot;
	};

	//! Global data for this plugin
	struct Global final
	{
		ReturnCode returnCode = ReturnCode::Default;
		std::list<Duel> duels;
		std::map<uint, FreeForAll> freeForAlls; // uint is iSystemID
	};
}
