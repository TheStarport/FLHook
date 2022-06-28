#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::CargoDrop
{
	class Info
	{
	  public:
		Info() : f1DisconnectProcessed(false), lastTimestamp(0) {}
		bool f1DisconnectProcessed;
		double lastTimestamp;
		Vector lastPosition;
		Quaternion lastDirection;
	};

	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/cargo_drop.json"; }

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

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::map<uint, Info> info;
		uint cargoDropContainerId;
		uint hullDrop1NickNameId;
		uint hullDrop2NickNameId;
		std::vector<uint> noLootItemsIds;
	};
} // namespace Plugins::CargoDrop