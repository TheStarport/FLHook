#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::CashManager
{
	enum class BankCode
	{
		InternalServerError,
		NotEnoughMoney,
		AboveMaximumTransferThreshold,
		BelowMinimumTransferThreshold,
		CannotWithdrawNegativeNumber,
		BankCouldNotAffordTransfer,
		Success
	};
	class CashManagerCommunicator final : public PluginCommunicator
	{
	public:
		inline static const char* pluginName = "Cash Manager";
		explicit CashManagerCommunicator(const std::string& plugin);

		/** @ingroup CashManager
		 * @brief Withdraw money from the specified accountId.
		 */
		BankCode PluginCall(ConsumeBankCash, const CAccount* account, uint cashAmount, const std::string& transactionSource);
	};

	
  
	struct Config final : Reflectable
	{
		std::string File() override { return "config/cash_manager.json"; }

		//! The minimum transfer amount.
		uint minimumTransfer = 0;
		
		// It is best to set this to a low enough value as to prevent players from
		// accidentally corrupting their accounts via their value (approximately 2 billion credits)
		uint maximumTransfer = 0;

		// If someone's cash goes above a threshold, deposit surplus money.
		bool depositSurplusOnDock = false;
		
		// When safety above kicks in, how much cash should be taken to prevent the mechanism from activating immediately on next transaction.
		uint safetyMargin = 0;

		// Message displayed when they hit the threshold
		std::wstring preventTransactionMessage = L"Transaction barred. Your ship value is too high. Deposit some cash into your bank using the /bank command.";

		// If the above is true, any amount above this value will cause the character to lock.
		uint cashThreshold = 1'800'000'000;

		// Transfers are not allowed to/from chars in this system.
		std::vector<std::string> blockedSystems;
		std::vector<uint> blockedSystemsHashed;

		//! Enable in dock cash cheat detection
		bool cheatDetection = false;

		// Prohibit transfers if the character has not been online for at least this time
		uint minimumTime = 0;
		// Remove transaction logs older than the amount of days indicated
		uint eraseTransactionsAfterDaysPassed = 365;

		// Cost in credits per transfer
		uint transferFee = 0;
	};

	struct Transaction
	{
		// When did this happen?
		uint64 timestamp;
		// Who accessed the bank?
		std::wstring accessor;
		// How much was taken/removed?
		int64 amount;
		// What bank did this involve?
		std::string bankId;
	};

	struct Bank
	{
		std::string accountId;
		std::wstring bankPassword;
		std::wstring identifier;
		uint64 cash = 0;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		// Other fields
		ReturnCode returnCode = ReturnCode::Default;
		SQLite::Database sql = SqlHelpers::Create("banks.sqlite");
	};

	extern const std::unique_ptr<Global> global;

	namespace Sql
	{
		void CreateSqlTables();
		std::optional<Bank> GetBankByIdentifier(std::wstring identifier);
		Bank GetOrCreateBank(const CAccount* account);
		bool WithdrawCash(const Bank& bank, int64 withdrawalAmount);
		bool DepositCash(const Bank& bank, uint depositAmount);
		bool TransferCash(const Bank& source, const Bank& target, int amount, int fee);
		std::vector<Transaction> ListTransactions(const Bank& bank, int amount = 20, int skip = 0);
		int CountTransactions(const Bank& bank);
		std::wstring SetNewPassword(const Bank& bank);
		void AddTransaction(const Bank& receiver, const std::string& sender, const int64& amount);
		int RemoveTransactionsOverSpecifiedDays(uint days);
		void SetOrClearIdentifier(const Bank& bank, const std::string& identifier);
	}
}