#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::CargoDrop
{
	//! Used to store info about each client
	class Info
	{
	  public:
		Info() : f1DisconnectProcessed(false), lastTimestamp(0) {}
		bool f1DisconnectProcessed;
		double lastTimestamp;
		Vector lastPosition;
		Quaternion lastDirection;
	};

	//! Config data for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "config/cargo_drop.json"; }

		// Reflectable fields

		//! If true, inform all players within range specified in disconnectingPlayersRange about the player disconnecting.
		bool reportDisconnectingPlayers = false;
		//! If true, kills the player should they disconnect while another player is within range specified in disconnectingPlayersRange.
		bool killDisconnectingPlayers = false;
		//! If true, drops the players cargo into space if there were players within range specified in disconnectingPlayersRange.
		bool lootDisconnectingPlayers = false;
		//! If true, causes players to drop all non-mounted equipment and commodities on death.
		bool enablePlayerCargoDropOnDeath = false;
		//! Caps the amount of any individual cargo type(weapon, commodity) being left behind on death.
		//! Sum of various cargos can exceed this number.
		int maxPlayerCargoDropCount = 3000; 
		//! Distance used to decide if report/player death should occur based on proximity to other players.
		float disconnectingPlayersRange = 5000.0f;
		//! Ratio of ship's unused cargo space to 'ship parts' commodities left behind on death, specified in hullDrop1NickName and hullDrop2NickName.
		float hullDropFactor = 0.1f;
		//! Message broadcasted to nearby players upon disconnect if reportDisconnectingPlayers is true.
		std::string disconnectMsg = "%player is attempting to engage cloaking device";
		//! Nickname of loot container model containing cargo/ship parts left behind upon death/disconnection.
		std::string cargoDropContainer = "lootcrate_ast_loot_metal";
		//! Ship parts commodity left behind on death if hullDropFactor is above zero.
		std::string hullDrop1NickName = "commodity_super_alloys";
		//! Ship parts commodity left behind on death if hullDropFactor is above zero.
		std::string hullDrop2NickName = "commodity_engine_components";
		//! Contains a list of nicknames of items that will not be dropped into space under any circumstances.
		std::vector<std::string> noLootItems = {};
	};

	//! Global data for this plugin
	struct Global final
	{
		//! Contains config data defined above
		std::unique_ptr<Config> config = nullptr;
		//! Default return code of the plugin
		ReturnCode returnCode = ReturnCode::Default;
		//! Map of ClientIds and the Info Struct defined above
		std::map<uint, Info> info;
		//! The id of the container to drop
		uint cargoDropContainerId;
		//! These two ids are commodities to represent the ship after destruction
		uint hullDrop1NickNameId;
		uint hullDrop2NickNameId;
		//! Items we don't want to be looted
		std::vector<uint> noLootItemsIds;
	};
} // namespace Plugins::CargoDrop