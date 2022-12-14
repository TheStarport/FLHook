#pragma once

#include <FLHook.hpp>

namespace Plugins::Warehouse
{
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/warehouse.json"; }

		std::vector<std::string> restrictedBases;
		std::vector<uint> restrictedBasesHashed;

		std::vector<std::string> restrictedItems;
		std::vector<uint> restrictedItemsHashed;

		uint costPerStackWithdraw = 0;
		uint costPerStackStore = 0;
	};

	struct WareHouseItem
	{
		__int64 id;
		uint equipArchId;
		__int64 quantity;
	};

	//! Global data for this plugin
	struct Global final
	{
		// Other fields
		ReturnCode returnCode = ReturnCode::Default;

		std::unique_ptr<SqlManager> sql;
		Config config;
	};

	extern const std::unique_ptr<Global> global;

	void CreateSqlTables();
	int64_t GetOrAddBase(BaseId& base);
	int64_t GetOrAddPlayer(int64_t baseId, const CAccount* acc);
	WareHouseItem GetOrAddItem(EquipId& item, int64_t playerId, int64_t quantity = 0);
	__int64 RemoveItem(EquipId& item, int64_t playerId, __int64 quantity);
	std::vector<WareHouseItem> GetAllItemsOnBase(__int64 playerId);
	std::optional<WareHouseItem> GetItemById(__int64 itemId, __int64 playerId);
} // namespace Plugins::Warehouse
