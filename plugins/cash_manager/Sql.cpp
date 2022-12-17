#include "Main.h"

namespace Plugins::CashManager
{
	void CreateSqlTables()
	{
		if (global->sql.tableExists("banks"))
		{
			return;
		}

		global->sql.exec("CREATE TABLE banks "
			"(id TEXT(36, 36) PRIMARY KEY UNIQUE NOT NULL, "
			"bankPassword TEXT(5, 5) NOT NULL, "
			"cash INTEGER NOT NULL DEFAULT(0), "
			"identifier TEXT(12, 12) UNIQUE);"
			"CREATE TABLE transactions(id INTEGER PRIMARY KEY AUTOINCREMENT UNIQUE NOT NULL, "
		    "timestamp INTEGER NOT NULL AS(UNIXEPOCH()), "
		    "amount INTEGER NOT NULL, "
		    "accessor TEXT(32, 32) NOT NULL);");

		global->sql.exec("CREATE INDEX IDX_accessor ON transactions (accessor DESC);");
	}

	std::optional<Bank> GetBankByIdentifier()
	{
		
	}

	void GetOrCreateBank(CAccount* account)
	{
		SQLite::Statement findExistingQuery(global->sql, "SELECT bankPassword, cash, identifier FROM banks WHERE id = ?");
		findExistingQuery.bind(1, wstos(account->wszAccId));

		// If already exists 
		if (findExistingQuery.executeStep())
		{

			return;
		}


	}

	int64 WithdrawCash();
	void DepositCash();
	void ListTransactions(int amount);
}