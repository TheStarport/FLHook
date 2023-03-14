#include "Warehouse.h"

namespace Plugins::Warehouse
{
	void CreateSqlTables()
	{
		if (!global->sql.tableExists("bases"))
		{
			global->sql.exec("CREATE TABLE bases ("
			                 "id INTEGER PRIMARY KEY AUTOINCREMENT,"
			                 "baseId INTEGER NOT NULL CHECK(baseId >= 0));"
			                 "CREATE TABLE players(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,"
			                 "baseId REFERENCES bases(baseId) ON UPDATE CASCADE NOT NULL,"
			                 "accountId TEXT(32, 32) NOT NULL);"
			                 "CREATE TABLE items(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,"
			                 "quantity INTEGER NOT NULL CHECK(quantity > 0),"
			                 "playerId INTEGER REFERENCES players(id) ON UPDATE CASCADE NOT NULL,"
			                 "itemId INTEGER NOT NULL);");
			global->sql.exec("CREATE INDEX IDX_baseId ON bases(baseId);"
			                 "CREATE INDEX IDX_accountId ON players(accountId);"
			                 "CREATE INDEX IDX_item_id ON items(itemId);");
		}
	}

	int64 GetOrAddBase(BaseId& base)
	{
		SQLite::Statement baseId(global->sql, "SELECT id FROM bases WHERE baseId = ?;");
		baseId.bind(1, base);
		if (baseId.executeStep())
		{
			return baseId.getColumn(0).getInt64();
		}

		SQLite::Statement query(global->sql, "INSERT INTO bases (baseId) VALUES(?);");
		query.bind(1, base);
		query.exec();

		return global->sql.getLastInsertRowid();
	}

	int64 GetOrAddPlayer(int64 baseId, const CAccount* acc)
	{
		const std::string accName = wstos(acc->accId);
		SQLite::Statement playerId(global->sql, "SELECT id FROM players WHERE baseId = ? AND accountId = ?;");
		playerId.bind(1, baseId);
		playerId.bind(2, accName);
		if (playerId.executeStep())
		{
			return playerId.getColumn(0).getInt64();
		}

		SQLite::Statement query(global->sql, "INSERT INTO players (baseId, accountId) VALUES(?, ?);");
		query.bind(1, baseId);
		query.bind(2, accName);
		query.exec();

		return global->sql.getLastInsertRowid();
	}

	void AdjustItemCount(int64 itemId, int64 count)
	{
		SQLite::Statement query(global->sql, "UPDATE items SET quantity = ? WHERE id = ?");
		query.bind(1, count);
		query.bind(2, itemId);
		query.exec();
	}

	WareHouseItem GetOrAddItem(EquipId& item, int64 playerId, int64 quantity)
	{
		SQLite::Statement itemId(global->sql, "SELECT id, quantity FROM items WHERE playerId = ? AND itemId = ?;");
		itemId.bind(1, playerId);
		itemId.bind(2, item);
		if (itemId.executeStep())
		{
			const auto currentId = itemId.getColumn(0).getInt64();
			auto currentQuantity = itemId.getColumn(1).getInt64();

			if (quantity > 0)
			{
				currentQuantity += quantity;
				AdjustItemCount(currentId, currentQuantity);
			}

			return {currentId, item, currentQuantity};
		}

		if (quantity <= 0)
		{
			return {0, 0};
		}

		SQLite::Statement query(global->sql, "INSERT INTO items (itemId, quantity, playerId) VALUES(?, ?, ?);");
		query.bind(1, item);
		query.bind(2, quantity);
		query.bind(3, playerId);
		query.exec();

		return {global->sql.getLastInsertRowid(), item, quantity};
	}

	int64 RemoveItem(const int64& sqlId, int64 playerId, int64 quantity)
	{
		SQLite::Statement itemQuery(global->sql, "SELECT quantity FROM items WHERE playerId = ? AND id = ?;");
		itemQuery.bind(1, playerId);
		itemQuery.bind(2, sqlId);
		if(!itemQuery.executeStep())
		{
			return 0;
		}

		const auto itemCount = itemQuery.getColumn(0).getInt64();
		if (quantity >= itemCount)
		{
			SQLite::Statement query(global->sql, "DELETE FROM items WHERE id = ?;");
			query.bind(1, sqlId);
			return query.exec() ? itemCount : 0;
		}

		AdjustItemCount(sqlId, itemCount - quantity);
		return quantity;
	}

	std::vector<WareHouseItem> GetAllItemsOnBase(int64 playerId)
	{
		std::vector<WareHouseItem> itemList;
		SQLite::Statement query(global->sql, "SELECT id, itemId, quantity FROM items WHERE playerId = ?;");
		query.bind(1, playerId);
		while (query.executeStep())
		{
			itemList.emplace_back(query.getColumn(0).getInt64(), query.getColumn(1).getUInt(), query.getColumn(2).getInt64());
		}
		return itemList;
	}

} // namespace Plugins::Warehouse