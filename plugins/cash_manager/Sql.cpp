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
		                 "accessor TEXT(32, 32) NOT NULL) "
		                 "bankId TEXT(36, 36) REFERENCES banks(id) ON UPDATE CASCADE NOT NULL;");

		global->sql.exec("CREATE INDEX IDX_bankId ON transactions (bankId DESC);");
	}

	std::optional<Bank> GetBankByIdentifier(std::wstring identifier)
	{
		SQLite::Statement findExistingQuery(global->sql, "SELECT id, bankPassword, cash FROM banks WHERE identifier = ?");
		findExistingQuery.bind(1, wstos(identifier));

		if (!findExistingQuery.executeStep())
		{
			return std::nullopt;
		}

		return std::make_optional<Bank>({stows(findExistingQuery.getColumn(0).getString()),
		    stows(findExistingQuery.getColumn(1).getString()),
		    identifier,
		    static_cast<uint64>(findExistingQuery.getColumn(2).getInt64())});
	}

	Bank GetOrCreateBank(CAccount* account)
	{
		const auto accountIdString = wstos(account->wszAccId);
		SQLite::Statement findExistingQuery(global->sql, "SELECT bankPassword, identifier, cash FROM banks WHERE id = ?");
		findExistingQuery.bind(1, accountIdString);

		// If already exists
		if (findExistingQuery.executeStep())
		{
			return {account->wszAccId,
			    stows(findExistingQuery.getColumn(0).getString()),
			    stows(findExistingQuery.getColumn(1).getString()),
			    static_cast<uint64>(findExistingQuery.getColumn(2).getInt64())};
		}

		const auto password = GenerateBankPassword();

		SQLite::Statement createBankQuery(global->sql, "INSERT INTO banks (id, bankPassword) VALUES(?, ?);");
		createBankQuery.bind(1, accountIdString);
		createBankQuery.bind(2, password);
		createBankQuery.exec();

		return {account->wszAccId, stows(password), L"", 0};
	}

	// Returns 0 if it failed to withdraw cash, 1 otherwise.
	bool WithdrawCash(const Bank& bank, uint withdrawalAmount)
	{
		SQLite::Statement transaction(global->sql, "UPDATE banks SET cash = cash - ? WHERE id = ? AND cash - ? > 0 ;");
		transaction.bind(1, withdrawalAmount);
		transaction.bind(2, wstos(bank.accountId));
		transaction.bind(3, withdrawalAmount);

		return static_cast<bool>(transaction.exec());
	}

	// Returns 0 if it failed to deposit cash, 1 otherwise.
	bool DepositCash(const Bank& bank, uint depositAmount)
	{
		SQLite::Statement transaction(global->sql, "UPDATE banks SET cash = cash + ? WHERE id = ?;");
		transaction.bind(1, depositAmount);
		transaction.bind(2, wstos(bank.accountId));

		return static_cast<bool>(transaction.exec());
	}

	std::vector<Transaction> ListTransactions(const Bank& bank, int amount)
	{
		SQLite::Statement transactions(global->sql,
		    "SELECT id, timestamp, accessor, amount FROM transactions "
		    "WHERE bankId = ? "
		    "ORDER BY timestamp DESC "
		    "LIMIT ?;");

		transactions.bind(1, wstos(bank.accountId));
		transactions.bind(2, amount);

		std::vector<Transaction> transactionsList;

		while (transactions.executeStep())
		{
			Transaction t = {
			    static_cast<uint64>(transactions.getColumn(0).getInt64()), stows(transactions.getColumn(1).getString()), transactions.getColumn(2).getInt64()};

			transactionsList.emplace_back(t);
		}
		return transactionsList;
	}

	bool AddTransaction(const Bank& receiver, std::wstring sender, int64 amount)
	{
		SQLite::Statement transaction(global->sql, "INSERT INTO transactions (bankId, accessor, amount) VALUES(?, ?, ?);");
		transaction.bind(1, wstos(receiver.accountId));
		transaction.bind(2, wstos(sender));
		transaction.bind(3, amount);

		return static_cast<bool>(transaction.exec());
	}

} // namespace Plugins::CashManager