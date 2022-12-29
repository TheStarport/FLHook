/**
 * @date July 2022
 * @author Lazrius & MrNen
 * @defgroup CashManager Cash Manager
 * @brief
 * The "Cash Manager" plugin serves as a utility for which players can manage their money through a shared bank
 * and transfer money between characters with ease. It also exposes functionality for other plugins to leverage
 * this shared pool of money.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - bank withdraw <cash> - Withdraw money from the bank of the active account
 * - bank withdraw <bankId> <password> - Withdraw cash from a different account bank using an id and password.
 * - bank deposit <cash> - Deposit cash into the current account bank
 * - bank transfer <bankId> <cash> - Transfer money from the current account bank to another one.
 * - bank password [confirm] - Generates a password for the current bank, will warn if confirm not specified.
 * - bank info [pass] - Shows your bank account information, if pass provided, will include the password (if set)
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *   "minimumTransfer": 0,
 *   "maximumTransfer": 0,
 *   "preventTransactionsNearThreshold": true,
 *   "cashThreshold": 1800000000,
 *   "blockedSystems": [],
 *   "cheatDetection": true,
 *   "minimumTime": 60,
 *   "eraseTransactionsAfterDaysPassed": 365,
 *	 "transferFee": 0
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * - ConsumeBankCash - Use this to take money directly from the current account bank for an action.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no dependencies.
 */

#include "Main.h"
#include "refl.hpp"

constexpr int TransactionsPerPage = 10;

// Setup Doxygen Group

/** @defgroup CashManager Cash Manager */

namespace Plugins::CashManager
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	std::wstring GetHumanTime(long long unix)
	{
		tm ts;
		localtime_s(&ts, &unix);
		std::wstring time;
		time.reserve(80);
		wcsftime(time.data(), time.size(), L"%a %Y-%m-%d %H:%M:%S %Z", &ts);
		return time;
	}

	void LoadSettings()
	{
		const auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

		for (const auto& system : global->config->blockedSystems)
		{
			global->config->blockedSystemsHashed.emplace_back(CreateID(system.c_str()));
		}

		if (global->config->transferFee < 0)
		{
			Console::ConWarn(L"Transfer Fee is a negative number!");
		}
		if (global->config->maximumTransfer < 0)
		{
			Console::ConWarn(L"Maximum Transfer is a negative number!");
		}
		if (global->config->minimumTransfer < 0)
		{
			Console::ConWarn(L"Minimum Transfer is a negative number!");
		}

		Sql::CreateSqlTables();
		Sql::RemoveTransactionsOverSpecifiedDays(global->config->eraseTransactionsAfterDaysPassed);
	}

	void WithdrawMoneyFromBank(const Bank& bank, uint withdrawal, ClientId client)
	{
		if (const auto currentValue = Hk::Player::GetShipValue(client);
		    currentValue.has_error() || (global->config->cashThreshold > 0 && currentValue.value() > global->config->cashThreshold))
		{
			PrintUserCmdText(client, L"Error: Your ship value is too high. Unload some credits or decrease ship value before withdrawing.");
		}

		if (bank.cash < withdrawal)
		{
			PrintUserCmdText(client, L"Error: Not enough credits, this bank only has %u", bank.cash);
			return;
		}

		if (withdrawal == 0)
		{
			PrintUserCmdText(client, L"Error: Invalid withdraw amount, please input a positive number. %u", bank.cash);
			return;
		}

		const int64 fee = static_cast<long long>(withdrawal) + global->config->transferFee;
		if (global->config->transferFee > 0 && static_cast<int64>(bank.cash) - fee < 0)
		{
			PrintUserCmdText(client, L"Error: Not enough cash in bank for withdrawal and fee (%s).", ToMoneyStr(static_cast<int>(fee)).c_str());
			return;
		}

		if (Sql::WithdrawCash(bank, fee))
		{
			Hk::Player::AddCash(client, withdrawal);
			PrintUserCmdText(client, L"Successfully withdrawn %s credits", ToMoneyStr(withdrawal).c_str());
			return;
		}

		PrintUserCmdText(client, L"Unknown error. Unable to withdraw cash.");
	}

	void DepositMoney(const Bank& bank, uint deposit, ClientId client)
	{
		if (deposit == 0)
		{
			PrintUserCmdText(client, L"Error: Invalid deposit amount, please input a positive number.");
			return;
		}

		if (const uint playerCash = Hk::Player::GetCash(client).value(); playerCash < deposit)
		{
			PrintUserCmdText(client, L"Error: Not enough credits, make sure to input a deposit number less than your balance.");
			return;
		}

		if (Sql::DepositCash(bank, deposit))
		{
			Hk::Player::RemoveCash(client, deposit);
			PrintUserCmdText(client, L"Successfully deposited %s credits", ToMoneyStr(deposit).c_str());
			return;
		}

		PrintUserCmdText(client, L"Unknown Error. Unable to deposit cash.");
	}

	// /bank withdraw account password amount
	void UserCmdWithdrawMoneyByPassword(const ClientId& client, const std::wstring& param)
	{
		const auto accountIdentifier = GetParam(param, ' ', 1);
		const auto password = GetParam(param, ' ', 2);
		const auto withdrawal = ToUInt(GetParam(param, ' ', 3));

		if (withdrawal == 0)
		{
			PrintUserCmdText(client, L"Error: Invalid withdraw amount, please input a positive number.");
			return;
		}

		const auto bank = Sql::GetBankByIdentifier(accountIdentifier);
		if (!bank.has_value())
		{
			PrintUserCmdText(client, L"Error: Bank identifier could was not valid.");
			return;
		}
		if (password != bank->bankPassword)
		{
			PrintUserCmdText(client, L"Error: Invalid Password.");
			return;
		}

		WithdrawMoneyFromBank(bank.value(), withdrawal, client);
	}

	// /bank transfer targetBank amount
	void TransferMoney(const ClientId client, const std::wstring& param, const Bank& bank)
	{
		const auto targetBankIdentifier = GetParam(param, ' ', 1);
		const auto amount = ToUInt(GetParam(param, ' ', 2));

		if (targetBankIdentifier.empty() || bank.identifier.empty())
		{
			PrintUserCmdText(client, L"Error: No bank identifier provided or sending from a bank without an identifier.");
			return;
		}

		if (amount == 0)
		{
			PrintUserCmdText(client, L"Error: Invalid amount, please input a positive number.");
			return;
		}

		const auto fee = amount + global->config->transferFee;
		if (static_cast<int64>(bank.cash) - (amount + fee) < 0)
		{
			const auto ifFee = L"and fee (%u)";
			PrintUserCmdText(client, L"Error: Not enough cash in bank for transfer%s.", fee > 0 ? ifFee : L"");
			return;
		}

		const auto targetBank = Sql::GetBankByIdentifier(targetBankIdentifier);
		if (!targetBank)
		{
			PrintUserCmdText(client, L"Error: Target Bank not found");
			return;
		}

		if (!Sql::TransferCash(bank, targetBank.value(), amount, fee))
		{
			PrintUserCmdText(client, L"Internal server error. Failed to transfer cash.");
			return;
		}

		PrintUserCmdText(client, L"Successfully transferred %s credits to %s", ToMoneyStr(amount).c_str(), bank.identifier.c_str());
		Sql::AddTransaction(bank, fmt::format("Bank {} -> Bank {}", wstos(bank.identifier), wstos(targetBank->identifier)), -(static_cast<int>((amount + fee))));
	}

	void ShowBankInfo(const ClientId& client, const Bank& bank, bool showPass)
	{
		PrintUserCmdText(client, L"Your Bank Information: ");
		PrintUserCmdText(client, L"|    Identifier: %s", bank.identifier.empty() ? L"N/A" : bank.identifier.c_str());
		PrintUserCmdText(client, L"|    Password: %s", showPass ? bank.bankPassword.c_str() : L"*****");
		PrintUserCmdText(client, L"|    Credits: %s", ToMoneyStr(bank.cash).c_str());

		if (!showPass)
		{
			PrintUserCmdText(client, L"Use /bank info pass to make the password visible.");
		}
	}

	void UserCommandHandler(const ClientId& client, const std::wstring& param)
	{
		// Checks before we handle any sort of command or process.
		if (const int secs = Hk::Player::GetOnlineTime(Hk::Client::GetCharacterNameByID(client).value()).value(); static_cast<uint>(secs) < global->config->minimumTime / 60)
		{
			PrintUserCmdText(client, L"Error: You cannot interact with the bank. This character is too new.");
			return;
		}

		if (ClientInfo[client].iTradePartner)
		{
			PrintUserCmdText(client, L"Error: You are currently in a trade.");
			return;
		}

		const auto currentSystem = Hk::Player::GetSystem(client);
		if (currentSystem.has_error())
		{
			PrintUserCmdText(client, L"Unable to decipher player location.");
			return;
		}

		if (const auto blockedSystems = &global->config->blockedSystemsHashed;
		    std::find(blockedSystems->begin(), blockedSystems->end(), currentSystem.value()) != blockedSystems->end())
		{
			PrintUserCmdText(client, L"Error: You are in a blocked system, you are unable to access the bank.");
			return;
		}

		const CAccount* acc = Players.FindAccountFromClientID(client);

		// Sending money from one bank to another
		if (const auto cmd = GetParam(param, L' ', 1); cmd == L"transfer")
		{
			const auto bank = Sql::GetOrCreateBank(acc);
			TransferMoney(client, param, bank);
		}
		else if (cmd == L"password")
		{
			const auto bank = Sql::GetOrCreateBank(acc);

			if (GetParam(param, ' ', 2) == L"confirm")
			{
				Sql::SetNewPassword(bank);
				ShowBankInfo(client, bank, true);
				return;
			}

			PrintUserCmdText(client, L"This will generate a new password and the previous will be invalid");
			PrintUserCmdText(client,
			    L"Your currently set password is %s if you are sure you want to regenerate your password type \"/bank password confirm\". ",
			    bank.bankPassword.c_str());
		}
		else if (cmd == L"identifier")
		{
			const auto bank = Sql::GetOrCreateBank(acc);
			const auto identifier = GetParam(param, ' ', 1);
			if (bank.identifier.empty() && identifier.empty())
			{
				PrintUserCmdText(client, L"Your bank currently does not have an identifier set");
				PrintUserCmdText(client, L"Generating an identifier means anybody with the identifier and password can access your bank.");
				PrintUserCmdText(client, L"Are you sure you want to generate an identifier for this account?");
				PrintUserCmdText(client, L"Please type \"/bank identifier <your banks identifier>\" to set one.");
				return;
			}

			if (const auto existingBank = Sql::GetBankByIdentifier(identifier); existingBank.has_value())
			{
				PrintUserCmdText(client, L"Another bank is already using this identifier. Identifiers must be unique.");
				return;
			}

			Sql::SetOrClearIdentifier(bank, wstos(identifier));
			PrintUserCmdText(client, L"Bank identifier set to: %s", identifier.c_str());
		}
		else if (cmd == L"withdraw")
		{
			if (global->config->preventTransactionsNearThreshold)
			{
				if (const auto value = Hk::Player::GetShipValue(client).value(); value > global->config->cashThreshold)
				{
					PrintUserCmdText(client,
					    L"You cannot withdraw more cash. Your current value is dangerously high. Please deposit money to bring your value back into normal "
					    L"range.");
					return;
				}
			}

			if (const int withdrawAmount = ToInt(GetParam(param, ' ', 1)); withdrawAmount > 0)
			{
				const auto bank = Sql::GetOrCreateBank(acc);
				WithdrawMoneyFromBank(bank, withdrawAmount, client);
				return;
			}

			UserCmdWithdrawMoneyByPassword(client, param);
		}
		else if (cmd == L"deposit")
		{
			const int depositAmount = ToInt(GetParam(param, ' ', 1)) - 1;
			const auto bank = Sql::GetOrCreateBank(acc);
			DepositMoney(bank, depositAmount, client);
		}
		else if (cmd == L"info")
		{
			const auto bank = Sql::GetOrCreateBank(acc);
			const bool showPass = GetParam(param, ' ', 1) == L"pass";
			ShowBankInfo(client, bank, showPass);
		}
		else if (cmd == L"transactions")
		{
			const auto bank = Sql::GetOrCreateBank(acc);
			const auto list = GetParam(param, ' ', 1);

			if (list == L"list")
			{
				int totalTransactions = Sql::CountTransactions(bank);
				const auto page = ToInt(GetParam(param, ' ', 2));

				if (!page)
				{
					PrintUserCmdText(client,
					    L"You currently have a total of %u transactions, spanning %u pages. "
					    L"Run this command again, adding a page number to view the desired transactions.",
					    totalTransactions,
					    totalTransactions / TransactionsPerPage);
					return;
				}

				if (page > totalTransactions / TransactionsPerPage)
				{
					PrintUserCmdText(client, L"Page not found.");
					return;
				}

				uint i = page * TransactionsPerPage;
				const auto transactions = Sql::ListTransactions(bank, TransactionsPerPage, i);
				for (const auto& transaction : transactions)
				{
					PrintUserCmdText(client, L"%u.) %s %llu", i++, transaction.accessor.c_str(), transaction.amount);
				}
				return;
			}

			const auto transactions = Sql::ListTransactions(bank, TransactionsPerPage);
			if (transactions.empty())
			{
				PrintUserCmdText(client, L"You have no transactions for this bank currently.");
				return;
			}

			int currentTransactions = Sql::CountTransactions(bank);
			PrintUserCmdText(client, L"Showing you %u of %u total transactions (most recent):", transactions.size(), currentTransactions);

			uint i = 0;
			for (const auto& transaction : transactions)
			{
				PrintUserCmdText(client, L"%u.) %s %llu", ++i, transaction.accessor.c_str(), transaction.amount);
			}
		}
		else
		{
			PrintUserCmdText(client, L"Here are the available commands for the bank plugin");
			PrintUserCmdText(client, L"\"/bank withdraw <amount>\" will withdraw money from your account's bank");
			PrintUserCmdText(
			    client, L"\"/bank withdraw <identifier> <password> <amount>\" will withdraw money from a specified bank that has a password set up");

			PrintUserCmdText(client, L"\"/bank deposit <amount>\" will deposit money from your character to your bank");
			PrintUserCmdText(client, L"\"/bank transfer <identifier> <amount>\" transfer money from your current bank to the target bank's identifier");
			PrintUserCmdText(client, L"\"/bank password \" will regenerate your password");
			PrintUserCmdText(client,
			    L"\"/bank identifier \" will allow you set an identifier. This will allow you to make transfers to other banks and access money from other accounts.");
			PrintUserCmdText(client, L"\"/bank info \" will display information regarding your current account's bank");
			PrintUserCmdText(client, fmt::format(L"\"/bank transactions \" will display the last {} transactions", TransactionsPerPage));
			PrintUserCmdText(client, L"\"/bank transactions <list> [page]\" will display the full list of transactions");
		}
	}

	const std::vector commands = {
	    {CreateUserCommand(L"/bank", L"", UserCommandHandler, L"A series of commands for storing money that can be shared among multiple characters.")}};

	BankCode __stdcall IpcConsumeBankCash(const CAccount* account, int cashAmount, const std::string& transactionSource)
	{
		if (cashAmount <= 0)
		{
			return BankCode::CannotWithdrawNegativeNumber;
		}

		if (static_cast<uint>(cashAmount) > global->config->maximumTransfer)
		{
			return BankCode::AboveMaximumTransferThreshold;
		}

		if (static_cast<uint>(cashAmount) < global->config->minimumTransfer)
		{
			return BankCode::BelowMinimumTransferThreshold;
		}

		const auto bank = Sql::GetOrCreateBank(account);

		if (bank.cash < cashAmount)
		{
			return BankCode::NotEnoughMoney;
		}

		const auto fee = cashAmount + global->config->transferFee;
		if (global->config->transferFee > 0 && static_cast<int64>(bank.cash) - fee < 0)
		{
			return BankCode::BankCouldNotAffordTransfer;
		}

		if (Sql::WithdrawCash(bank, fee))
		{
			Sql::AddTransaction(bank, transactionSource, cashAmount);
			return BankCode::Success;
		}

		return BankCode::InternalServerError;
	}

	bool ShouldSuppressBuy(const void* dummy [[maybe_unused]], const ClientId& client)
	{
		if (!global->config->preventTransactionsNearThreshold)
			return false;

		if (const auto currentValue = Hk::Player::GetShipValue(client).value(); global->config->cashThreshold < currentValue)
		{
			PrintUserCmdText(client, L"Transaction barred. Your ship value is too high. Deposit some cash into your bank using the /bank command.");
			return true;
		}

		return false;
	}

	bool ReqAddItem(const uint& goodID [[maybe_unused]], char const* hardpoint [[maybe_unused]], const int& count [[maybe_unused]],
	    const float& status [[maybe_unused]], const bool& mounted [[maybe_unused]], const uint& client)
	{
		// First value is dummy garbage
		return ShouldSuppressBuy(&goodID, client);
	}

	CashManagerCommunicator::CashManagerCommunicator(const std::string& plugin) : PluginCommunicator(plugin) { this->ConsumeBankCash = IpcConsumeBankCash; }

} // namespace Plugins::CashManager

using namespace Plugins::CashManager;

// REFL_AUTO must be global namespace
REFL_AUTO(type(Config), field(minimumTransfer), field(eraseTransactionsAfterDaysPassed), field(blockedSystems), field(preventTransactionsNearThreshold),
    field(maximumTransfer), field(cheatDetection), field(minimumTime), field(transferFee));

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name(CashManagerCommunicator::pluginName);
	pi->shortName("cash_manager");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodSell, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqAddItem, &ReqAddItem);
	pi->emplaceHook(HookedCall::IServerImpl__ReqChangeCash, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqEquipment, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqHullStatus, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqSetCash, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqShipArch, &ShouldSuppressBuy);
}