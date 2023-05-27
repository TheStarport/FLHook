#include <random>
#include "CashManager.h"

namespace Plugins::CashManager::Sql
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
		                 "timestamp INTEGER NOT NULL, "
		                 "amount INTEGER NOT NULL, "
		                 "accessor TEXT(32, 32) NOT NULL, "
		                 "bankId TEXT(36, 36) REFERENCES banks(id) ON UPDATE CASCADE NOT NULL);");

		global->sql.exec("CREATE INDEX IDX_bankId ON transactions (bankId DESC);");
	}

	std::string GenerateBankPassword()
	{
		const std::vector letters = {
		    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
		std::random_device dev;
		std::mt19937 r(dev());
		std::uniform_int_distribution<std::size_t> randLetter(0, letters.size() - 1);
		std::uniform_int_distribution<std::size_t> randNumber(0, 9);
		std::stringstream ss;
		ss << letters[randLetter(r)] << randNumber(r) << randNumber(r) << randNumber(r) << randNumber(r);
		return ss.str();
	}

	std::optional<Bank> GetBankByIdentifier(std::wstring identifier)
	{
		SQLite::Statement findExistingQuery(global->sql, "SELECT id, bankPassword, cash FROM banks WHERE identifier = ?");
		findExistingQuery.bind(1, StringUtils::wstos(identifier));

		if (!findExistingQuery.executeStep())
		{
			return std::nullopt;
		}

		return std::make_optional<Bank>({findExistingQuery.getColumn(0).getString(),
		    StringUtils::stows(findExistingQuery.getColumn(1).getString()),
		    identifier,
		    static_cast<uint64>(findExistingQuery.getColumn(2).getInt64())});
	}

	std::wstring SetNewPassword(const Bank& bank)
	{
		const auto newPass = GenerateBankPassword();

		SQLite::Statement replacePassword(global->sql, "UPDATE banks SET bankPassword = ? WHERE id = ?;");
		replacePassword.bind(1, newPass);
		replacePassword.bind(2, bank.accountId);

		return StringUtils::stows(newPass);
	}

	Bank GetOrCreateBank(const CAccount* account)
	{
		const auto accountIdString = StringUtils::wstos(account->accId);
		SQLite::Statement findExistingQuery(global->sql, "SELECT bankPassword, identifier, cash FROM banks WHERE id = ?");
		findExistingQuery.bind(1, accountIdString);

		// If already exists
		if (findExistingQuery.executeStep())
		{
			return {accountIdString,
			    StringUtils::stows(findExistingQuery.getColumn(0).getString()),
			    StringUtils::stows(findExistingQuery.getColumn(1).getString()),
			    static_cast<uint64>(findExistingQuery.getColumn(2).getInt64())};
		}

		const auto password = GenerateBankPassword();

		SQLite::Statement createBankQuery(global->sql, "INSERT INTO banks (id, bankPassword) VALUES(?, ?);");
		createBankQuery.bind(1, accountIdString);
		createBankQuery.bind(2, password);
		createBankQuery.exec();

		return {accountIdString, StringUtils::stows(password), L"", 0};
	}

	// Returns 0 if it failed to withdraw cash, 1 otherwise.
	bool WithdrawCash(const Bank& bank, int64 withdrawalAmount)
	{
		SQLite::Statement transaction(global->sql, "UPDATE banks SET cash = cash - ? WHERE id = ? AND cash - ? >= 0 ;");
		transaction.bind(1, withdrawalAmount);
		transaction.bind(2, bank.accountId);
		transaction.bind(3, withdrawalAmount);

		return static_cast<bool>(transaction.exec());
	}

	// Returns 0 if it failed to deposit cash, 1 otherwise.
	bool DepositCash(const Bank& bank, uint depositAmount)
	{
		SQLite::Statement transaction(global->sql, "UPDATE banks SET cash = cash + ? WHERE id = ?;");
		transaction.bind(1, depositAmount);
		transaction.bind(2, bank.accountId);

		return static_cast<bool>(transaction.exec());
	}

	bool TransferCash(const Bank& source, const Bank& target, const int amount, const int fee)
	{
		SQLite::Transaction transferTransaction(global->sql);

		SQLite::Statement sourceQuery(global->sql, "UPDATE banks SET cash = cash - ? - ? WHERE id = ?;");
		sourceQuery.bind(1, amount);
		sourceQuery.bind(2, fee);
		sourceQuery.bind(3, source.accountId);
		int rowsAffected = sourceQuery.exec();

		SQLite::Statement targetQuery(global->sql, "UPDATE banks SET cash = cash + ? WHERE id = ?;");
		targetQuery.bind(1, amount);
		targetQuery.bind(2, target.accountId);
		rowsAffected += targetQuery.exec();

		if (rowsAffected == 2)
		{
			transferTransaction.commit();
			return true;
		}

		return false;
	}

	int CountTransactions(const Bank& bank)
	{
		SQLite::Statement transactionCount(global->sql, "SELECT id FROM banks WHERE bankId = ? ORDER BY DESC;");
		transactionCount.bind(bank.accountId);

		transactionCount.executeStep();
		return transactionCount.getColumnCount();
	}

	std::vector<Transaction> ListTransactions(const Bank& bank, int amount, int skip)
	{
		SQLite::Statement transactions(global->sql,
		    "SELECT id, timestamp, accessor, amount FROM transactions "
		    "WHERE bankId = ? "
		    "ORDER BY timestamp DESC "
		    "LIMIT ? "
			"OFFSET ?;");

		transactions.bind(1, bank.accountId);
		transactions.bind(2, amount);
		transactions.bind(3, skip);

		std::vector<Transaction> transactionsList;

		while (transactions.executeStep())
		{
			transactionsList.emplace_back(static_cast<uint64>(transactions.getColumn(0).getInt64()),
			    StringUtils::stows(transactions.getColumn(1).getString()),
			    transactions.getColumn(2).getInt64(),
			    bank.accountId);
		}
		return transactionsList;
	}

	void AddTransaction(const Bank& receiver, const std::string& sender, const int64& amount)
	{
		SQLite::Statement transaction(global->sql, "INSERT INTO transactions (bankId, accessor, amount, timestamp) VALUES(?, ?, ?, ?);");
		transaction.bind(1, receiver.accountId);
		transaction.bind(2, sender);
		transaction.bind(3, amount);
		transaction.bind(4, std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());

		transaction.exec();
	}

	void SetOrClearIdentifier(const Bank& bank, const std::string& identifier)
	{
		if (identifier.empty())
		{
			SQLite::Statement clearQuery(global->sql, "UPDATE banks SET identifier = NULL WHERE id = ?;");
			clearQuery.bind(1, bank.accountId);
			return;	
		}

		SQLite::Statement clearQuery(global->sql, "UPDATE banks SET identifier = ? WHERE id = ?;");
		clearQuery.bind(1, identifier);
		clearQuery.bind(2, bank.accountId);
	}

	using namespace std::literals::chrono_literals;
	constexpr int64 SecondsInADay = std::chrono::duration_cast<std::chrono::seconds>(24h).count();
	int RemoveTransactionsOverSpecifiedDays(uint days)
	{
		if (!days)
		{
			return 0;
		}

		const int64 currentTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		const int64 oldestPossibleEntry = currentTime - days * SecondsInADay;

		SQLite::Statement cleaningQuery(global->sql, "DELETE FROM transactions WHERE timestamp < ?;");
		cleaningQuery.bind(1, oldestPossibleEntry);

		return cleaningQuery.exec();
	}
} // namespace Plugins::CashManager