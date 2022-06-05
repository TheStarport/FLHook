#pragma once

#include <FLHook.h>
#include <plugin.h>

namespace Plugins::CargoDrop
{
	class INFO
	{
	  public:
		INFO() : bF1DisconnectProcessed(false), dLastTimestamp(0) {}
		bool bF1DisconnectProcessed;
		double dLastTimestamp;
		Vector vLastPosition;
		Quaternion vLastDir;
	};

	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/cargo_drop.json"; }

		// Reflectable fields
		bool ReportDisconnectingPlayers = false;
		bool KillDisconnectingPlayers = false;
		bool LootDisconnectingPlayers = false;
		float DisconnectingPlayersRange = 5000.0f;
		float HullDropFactor = 0.1f;
		std::string DisconnectMsg = "%player is attempting to engage cloaking device";
		std::string CargoDropContainer = "lootcrate_ast_loot_metal";
		std::string HullDrop1NickName = "commodity_super_alloys";
		std::string HullDrop2NickName = "commodity_engine_components";
		std::vector<std::string> NoLootItems = {};
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
		std::map<uint, INFO> mapInfo;
		uint CargoDropContainerId;
		uint HullDrop1NickNameId;
		uint HullDrop2NickNameId;
		std::vector<uint> NoLootItemsIds;
	};
} // namespace Plugins::CargoDrop