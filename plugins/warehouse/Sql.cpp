#include "Main.h"

namespace Plugins::Warehouse
{
	void CreateSqlTables()
	{
		global->sql = SqlManager::GetDatabase("warehouse.sqlite");

		if (!global->sql->GetDb().tableExists("bases"))
		{
			auto query = global->sql->StartQuery("CREATE TABLE bases ("
			                        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
			                        "baseId INTEGER NOT NULL CHECK(baseId >= 0));"
			                        "CREATE TABLE players(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,"
			                        "baseId REFERENCES bases(baseId) ON UPDATE CASCADE NOT NULL,"
			                        "accountId TEXT(32, 32) NOT NULL);"
			                        "CREATE TABLE items(id INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL UNIQUE,"
			                        "quantity INTEGER NOT NULL CHECK(quantity > 0),"
			                        "playerId INTEGER REFERENCES players(id) ON UPDATE CASCADE NOT NULL,"
			                        "itemId INTEGER NOT NULL);"
			                        "CREATE INDEX "
			                        "IDX_baseId"
			                        " ON bases(baseId);"
			                        "CREATE INDEX "
			                        "IDX_accountId"
			                        " ON players(accountId);"
			                        "CREATE INDEX "
			                        "IDX_item_id"
			                        " ON items(itemId);"
			                        ");");
			query.exec();
			global->sql->Commit();
		}
	}

	int64_t GetOrAddBase(BaseId& base)
	{
		auto baseId = global->sql->StartQuery("SELECT id FROM bases WHERE baseId = ?;", false);
		baseId.bind(1, base);
		if (baseId.executeStep())
		{
			return baseId.getColumn(0).getInt64();
		}

		auto query = global->sql->StartQuery("INSERT INTO bases (baseId) VALUES(?);");
		query.bind(1, base);
		query.exec();
		global->sql->Commit();

		return global->sql->GetDb().getLastInsertRowid();
	}

	int64_t GetOrAddPlayer(int64_t baseId, const CAccount* acc)
	{
		const std::string accName = wstos(acc->wszAccId);
		auto playerId = global->sql->StartQuery("SELECT id FROM players WHERE baseId = ? AND accountId = ?;", false);
		playerId.bind(1, baseId);
		playerId.bind(2, accName);
		if (playerId.executeStep())
		{
			return playerId.getColumn(0).getInt64();
		}

		auto query = global->sql->StartQuery("INSERT INTO players (baseId, accountId) VALUES(?, ?);");
		query.bind(1, baseId);
		query.bind(2, accName);
		query.exec();
		global->sql->Commit();

		return global->sql->GetDb().getLastInsertRowid();
	}

	void AdjustItemCount(__int64 itemId, __int64 count)
	{
		auto query = global->sql->StartQuery("UPDATE items SET quantity = ? WHERE id = ?");
		query.bind(1, count);
		query.bind(2, itemId);
		query.exec();
		global->sql->Commit();
	}

	std::pair<__int64, __int64> GetOrAddItem(EquipId& item, int64_t playerId, int64_t quantity)
	{
		auto itemId = global->sql->StartQuery("SELECT id, quantity FROM items WHERE playerId = ? AND itemId = ?;", false);
		itemId.bind(1, playerId);
		itemId.bind(2, item);
		if (itemId.executeStep())
		{
			auto currentId = itemId.getColumn(0).getInt64();
			auto currentQuantity = itemId.getColumn(1).getInt64();

			if (quantity > 0)
			{
				currentQuantity += quantity;
				AdjustItemCount(currentId, currentQuantity);
			}

			return {currentId, currentQuantity};
		}

		auto query = global->sql->StartQuery("INSERT INTO items (itemId, quantity, playerId) VALUES(?, ?, ?);");
		query.bind(1, item);
		query.bind(2, quantity);
		query.bind(3, playerId);
		query.exec();
		global->sql->Commit();

		return {global->sql->GetDb().getLastInsertRowid(), quantity};
	}

	__int64 RemoveItem(EquipId& item, int64_t playerId, __int64 quantity)
	{
		const auto [itemId, currentQuantity] = GetOrAddItem(item, playerId, quantity);
		if (quantity >= currentQuantity)
		{
			auto query = global->sql->StartQuery("DELETE FROM items WHERE id = ?;");
			query.bind(1, itemId);
			query.exec();
			global->sql->Commit();
			return currentQuantity;
		}

		AdjustItemCount(itemId, currentQuantity - quantity);
		return quantity;
	}
}