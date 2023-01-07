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
		bool reportDisconnectingPlayers = false;
		bool killDisconnectingPlayers = false;
		bool lootDisconnectingPlayers = false;
		float disconnectingPlayersRange = 5000.0f;
		float hullDropFactor = 0.1f;
		std::string disconnectMsg = "%player is attempting to engage cloaking device";
		std::string cargoDropContainer = "lootcrate_ast_loot_metal";
		std::string hullDrop1NickName = "commodity_super_alloys";
		std::string hullDrop2NickName = "commodity_engine_components";
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