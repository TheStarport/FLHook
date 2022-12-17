#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::CashManager
{
	enum class BankCode
	{
		NoBank,
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
		BankCode PluginCall(ConsumeBankCash, std::wstring accountId, int cashAmount, const std::wstring_view& transactionSource);
	};

	std::wstring GenerateAccountId();
  
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/cash_manager.json"; }

		//! The minimum transfer amount.
		int minimumTransfer = 0;
		
		// It is best to set this to a low enough value as to prevent players from
		// accidentally corrupting their accounts via their value (approximately 2 billion credits)
		int maximumTransfer = 0;

		// If someone's cash goes above a threshold, 'lock' the account until they unload some into the bank
		bool preventTransactionsNearThreshold = false;

		// If the above is true, any amount above this value will cause the character to lock.
		int cashThreshold = 1'800'000'000;

		// Transfers are not allowed to/from chars in this system.
		std::vector<std::string> blockedSystems;
		std::vector<uint> blockedSystemsHashed;

		//! Enable in dock cash cheat detection
		bool cheatDetection = false;

		// Prohibit transfers if the character has not been online for at least this time
		int minimumTime = 0;

		// Remove transaction logs older than the amount of days indicated
		int eraseTransactionsAfterDaysPassed = 365;

		// Cost in credits per transfer
		int transferFee = 0;
	};

	struct Transaction
	{
		// When did this happen?
		uint64 lastAccessedUnix;
		// Who accessed the bank?
		std::wstring accessor;
		// How much was taken/removed?
		uint64 amount;
	};

	struct Bank
	{
		std::wstring accountId;
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

	void CreateSqlTables();
	std::optional<Bank> GetBankByIdentifier();
	Bank GetOrCreateBank(CAccount* account);
	int64 WithdrawCash();
	void DepositCash();
	void ListTransactions(int amount = 20);
}