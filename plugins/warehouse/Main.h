#pragma once

#include <FLHook.hpp>

namespace Plugins::Warehouse
{
	struct Config final : Reflectable
	{
		std::string File() override { return "config/warehouse.json"; }

		std::vector<std::string> restrictedBases;
		std::vector<uint> restrictedBasesHashed;

		std::vector<std::string> restrictedItems;
		std::vector<uint> restrictedItemsHashed;

		uint costPerStackWithdraw = 0;
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
	std::vector<WareHouseItem> GetAllItemsOnBase(int64 playerId);
	std::optional<WareHouseItem> GetItemById(int64 itemId, int64 playerId);
} // namespace Plugins::Warehouse
