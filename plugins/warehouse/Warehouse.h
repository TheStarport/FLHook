#pragma once

#include <FLHook.hpp>

namespace Plugins::Warehouse
{
	struct Config final : Reflectable
	{
		std::string File() override { return "config/warehouse.json"; }

		//! Vector of base nicknames where warehouse functionality is not available.
		std::vector<std::string> restrictedBases;
		std::vector<uint> restrictedBasesHashed;

		//! Vector of item nicknames which cannot be stored using the warehouse functionality.
		std::vector<std::string> restrictedItems;
		std::vector<uint> restrictedItemsHashed;

		//! Credit cost of withdrawing an item from the warehouse.
		uint costPerStackWithdraw = 0;
		//! Credit cost of depositing an item into the warehouse.
		uint costPerStackStore = 0;
	};

	struct WareHouseItem
	{
		int64 id;
		uint equipArchId;
		int64 quantity;
	};

	//! Global data for this plugin
	struct Global final
	{
		// Other fields
		ReturnCode returnCode = ReturnCode::Default;

		SQLite::Database sql = SqlHelpers::Create("warehouse.sqlite");
		Config config;
	};

	extern const std::unique_ptr<Global> global;

	void CreateSqlTables();
	int64 GetOrAddBase(BaseId& base);
	int64 GetOrAddPlayer(int64 baseId, const CAccount* acc);
	WareHouseItem GetOrAddItem(EquipId& item, int64 playerId, int64 quantity = 0);
	int64 RemoveItem(const int64& sqlId, int64 playerId, int64 quantity);
	std::vector<WareHouseItem> GetAllItemsOnBase(int64 playerId, int64 baseId);
	std::map<int64, std::vector<WareHouseItem>> GetAllBases(int64 playerId);
} // namespace Plugins::Warehouse
