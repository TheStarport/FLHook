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
#include <random>
#include <sstream>

constexpr long long SecondsInADay = 86400;

// Setup Doxygen Group

/** @defgroup CashManager Cash Manager */

namespace Plugins::CashManager
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();
	constexpr const char* jsonPath = R"(\AccountBank.json)";

	void CreateAccountBank(const std::string& path, const std::wstring& accountId)
	{
		Bank newBank;
		newBank.flAccountId = accountId;
		Serializer::SaveToJson<Bank>(newBank, path);
		global->banks[accountId] = std::make_shared<Bank>(std::move(newBank));
	}

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

		std::string saveGameDir;
		saveGameDir.reserve(MAX_PATH);
		GetUserDataPath(saveGameDir.data());

		const std::string path = saveGameDir + R"(\Accts\MultiPlayer\)";
		for (const auto& entry : std::filesystem::directory_iterator(path))
		{
			if (!entry.is_directory())
				continue;

			const auto json = entry.path().string() + "/" + jsonPath;
			if (std::filesystem::exists(json))
			{
				auto bank = Serializer::JsonToObject<Bank>(json, false);
				auto accId = entry.path().filename().wstring();
				bank.flAccountId = accId;
				global->banks[accId] = std::make_shared<Bank>(bank);
				if (global->config->eraseTransactionsAfterDaysPassed > 0)
				{
					const long long currentTime = std::chrono::seconds(std::time(nullptr)).count();
					auto transaction = &global->banks[accId]->transactions;
					transaction->erase(std::remove_if(transaction->begin(),
					                       transaction->end(),
					                       [currentTime](const Transaction& t) {
						                       return currentTime - t.lastAccessedUnix > (global->config->eraseTransactionsAfterDaysPassed * SecondsInADay);
					                       }),
					    transaction->end());
				}
			}
			else
			{
				CreateAccountBank(json, entry.path().filename().wstring());
			}
		}
	}

	void OnPlayerLogin(const SLoginInfo& li, const ClientId& client)
	{
		CAccount const* acc = Players.FindAccountFromClientID(client);
		if (!acc)
		{
			return;
		}

		const std::wstring dir = Hk::Client::GetAccountDirName(acc);

		std::string saveGameDir;
		saveGameDir.reserve(MAX_PATH);
		GetUserDataPath(saveGameDir.data());

		const std::string path = saveGameDir + R"(\Accts\MultiPlayer\)" + wstos(dir) + jsonPath;
		if (!std::filesystem::exists(path))
		{
			CreateAccountBank(path, acc->wszAccId);
		}
	}

	std::string GenerateBankPassword()
	{
		const std::vector letters = {
		    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
		std::random_device dev;
		std::mt19937 r(dev());
		const std::uniform_int_distribution<std::size_t> randLetter(0, letters.size() - 1);
		const std::uniform_int_distribution<std::size_t> randNumber(0, 9);
		std::stringstream ss;
		ss << letters[randLetter(r)] << randNumber(r) << randNumber(r) << randNumber(r) << randNumber(r);
		return ss.str();
	}

	void Bank::Save()
	{
		std::string saveGameDir;
		saveGameDir.reserve(MAX_PATH);
		GetUserDataPath(saveGameDir.data());
		const std::string path = saveGameDir + R"(\Accts\MultiPlayer\)" + wstos(flAccountId) + jsonPath;
		Serializer::SaveToJson(*this, path);
	}

	std::shared_ptr<Bank> GetBank(std::variant<ClientId, std::wstring> targetBank)
	{
		std::shared_ptr<Bank> bank = nullptr;
		if (targetBank.index() == 0)
		{
			const auto* acc = Players.FindAccountFromClientID(std::get<ClientId>(targetBank));
			const auto bankIter = global->banks.find(acc->wszAccId);
			if (bankIter == global->banks.end())
			{
				throw std::logic_error("Attempted to access bank of account that does not have a bank.");
			}
			bank = bankIter->second;
		}
		else
		{
			const auto& bankId = std::get<std::wstring>(targetBank);
			const auto bankIter = global->banks.find(bankId);
			if (bankIter != global->banks.end())
			{
				bank = bankIter->second;
			}
		}

		return bank;
	}

	void WithdrawMoney(std::shared_ptr<Bank> bank, int withdrawal, ClientId client)
	{
		if (const auto currentValue = Hk::Player::GetShipValue(client); 
			currentValue.has_error() || (global->config->cashThreshold > 0 && currentValue.value() > static_cast<float>(global->config->cashThreshold)))
		{
			PrintUserCmdText(client, L"Error: Your ship value is too high. Unload some credits or decrease ship value before withdrawing.");
		}

		if (bank->cash < withdrawal)
		{
			PrintUserCmdText(client, L"Error: Not enough credits, this bank only has %u", bank->cash);
			return;
		}

		if (withdrawal <= 0)
		{
			PrintUserCmdText(client, L"Error: Invalid withdraw amount, please input a positive number. %u", bank->cash);
			return;
		}

		const auto fee = static_cast<long long>(withdrawal) + global->config->transferFee;
		if (global->config->transferFee > 0 && bank->cash - fee < 0)
		{
			PrintUserCmdText(client, L"Error: Not enough cash in bank for withdrawal and fee (%u).", fee);
			return;
		}

		Hk::Player::AddCash(client, withdrawal);

		bank->cash -= fee;

		Transaction transaction;

		transaction.accessor = Hk::Client::GetCharacterNameByID(client).value();
		transaction.amount = -fee;

		bank->transactions.emplace(bank->transactions.begin(), transaction);
		bank->Save();
	}

	void DepositMoney(std::shared_ptr<Bank> bank, int deposit, ClientId client)
	{
		if (const int playerCash = Hk::Player::GetCash(client).value(); playerCash < deposit)
		{
			PrintUserCmdText(client, L"Error: Not enough credits, make sure to input a deposit number less than your balance.");
			return;
		}

		if (deposit <= 0)
		{
			PrintUserCmdText(client, L"Error: Invalid deposit amount, please input a positive number.");
			return;
		}

		Hk::Player::AddCash(client, -deposit);

		bank->cash += deposit;

		Transaction transaction;

		transaction.accessor = Hk::Client::GetCharacterNameByID(client).value();
		transaction.amount = deposit;

		bank->transactions.emplace(bank->transactions.begin(), transaction);
		bank->Save();
	}

	// /bank withdraw account password amount
	void WithdrawMoneyByPassword(const ClientId& client, const std::wstring_view& param)
	{
		const auto accountId = GetParam(param, ' ', 1);
		const auto password = GetParam(param, ' ', 2);
		const auto withdrawal = ToInt(GetParam(param, ' ', 3));

		if (withdrawal <= 0)
		{
			PrintUserCmdText(client, L"Error: Invalid withdraw amount, please input a positive number.");
			return;
		}

		const auto bank = GetBank(accountId);
		if (!bank)
		{
			PrintUserCmdText(client, L"Error: Bank accountId could not be found.");
			return;
		}
		if (password != bank->bankPassword)
		{
			PrintUserCmdText(client, L"Error: Invalid Password.");
			return;
		}
		WithdrawMoney(bank, withdrawal, client);
	}

	// /bank transfer targetBank amount
	void TransferMoney(const ClientId client, const std::wstring_view& param)
	{
		const auto sourceBank = GetBank(client);
		const auto targetBank = GetBank(GetParam(param, ' ', 1));
		const auto amount = ToInt(GetParam(param, ' ', 2));

		if (!targetBank)
		{
			PrintUserCmdText(client, L"Error: Target Bank not found");
			return;
		}

		if (amount <= 0)
		{
			PrintUserCmdText(client, L"Error: Invalid amount, please input a positive number.");
			return;
		}

		if (sourceBank->cash - amount < 0)
		{
			PrintUserCmdText(client, L"Error: Not enough cash in bank for transfer.");
			return;
		}

		const auto fee = static_cast<long long>(amount) + global->config->transferFee;
		if (global->config->transferFee > 0 && sourceBank->cash - fee < 0)
		{
			PrintUserCmdText(client, L"Error: Not enough cash in bank for transfer and fee (%u).", fee);
			return;
		}

		sourceBank->cash -= fee;
		targetBank->cash += amount;

		const auto charName = Hk::Client::GetCharacterNameByID(client).value();

		Transaction sourceTransaction;
		sourceTransaction.accessor = charName;
		sourceTransaction.amount = fee;
		sourceBank->transactions.emplace(sourceBank->transactions.begin(), std::move(sourceTransaction));

		Transaction targetTransaction;
		targetTransaction.accessor = charName;
		targetTransaction.amount = amount;
		targetBank->transactions.emplace(targetBank->transactions.begin(), std::move(targetTransaction));

		sourceBank->Save();
		targetBank->Save();
	}

	void ShowBankInfo(const ClientId& client, std::shared_ptr<Bank> bank, bool showPass)
	{
		PrintUserCmdText(client, L"Your Bank Information: ");
		PrintUserCmdText(client, L"|    Account ID: %s", bank->accountId.c_str());
		if (bank->bankPassword.empty())
		{
			PrintUserCmdText(client, L"|    Password: None Set");
		}
		else
		{
			PrintUserCmdText(client, L"|    Password: %s", showPass ? bank->bankPassword.c_str() : L"*****");
		}

		PrintUserCmdText(client, L"|    Credits: %u", bank->cash);
		if (!bank->transactions.empty())
		{
			PrintUserCmdText(client, L"|    Last Accessed: %s", GetHumanTime(bank->transactions.front().lastAccessedUnix).c_str());
		}

		if (!showPass && !bank->bankPassword.empty())
		{
			PrintUserCmdText(client, L"Use /bank info pass to make the password visible.");
		}
	}

	void UserCommandHandler(const ClientId& client, const std::wstring_view& param)
	{
		// Checks before we handle any sort of command or process.
		if (const int secs = Hk::Player::GetOnlineTime(Hk::Client::GetCharacterNameByID(client).value()).value(); secs < global->config->minimumTime / 60)
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

		if (const auto cmd = GetParam(param, L' ', 1); cmd == L"withdraw")
		{
			if (global->config->preventTransactionsNearThreshold)
			{
				const int cash = Hk::Player::GetCash(client).value();

				if (cash > global->config->cashThreshold)
				{
					PrintUserCmdText(client, L"You cannot withdraw more cash. Your current value is dangerously high. Please deposit money to bring your value back into normal range.");
					return;
				}
			}

			if (const int withdrawAmount = ToInt(GetParam(param, ' ', 1)) - 1)
			{
				const CAccount* acc = Players.FindAccountFromClientID(client);
				const auto bank = GetBank(acc->wszAccId);
				WithdrawMoney(bank, withdrawAmount, client);
				return;
			}

			WithdrawMoneyByPassword(client, param);
		}
		else if (cmd == L"deposit")
		{
			const int depositAmount = ToInt(GetParam(param, ' ', 1)) - 1;
			const CAccount* acc = Players.FindAccountFromClientID(client);
			const auto bank = GetBank(acc->wszAccId);
			DepositMoney(bank, depositAmount, client);
		}

		else if (cmd == L"password")
		{
			const CAccount* acc = Players.FindAccountFromClientID(client);
			const auto bank = GetBank(acc->wszAccId);

			if (GetParam(param, ' ', 2) == L"confirm")
			{
				bank->bankPassword = GenerateBankPassword();
				PrintUserCmdText(client, L"Your bank account information has been updated.");
				ShowBankInfo(client, bank, true);
				return;
			}

			if (bank->bankPassword.empty())
			{
				PrintUserCmdText(client, L"Your bank currently does not have a password set");
				PrintUserCmdText(client, L"Generating a password means anybody with the password and Bank ID can access your bank");
				PrintUserCmdText(client, L"Are you sure you want to generate a password for this account?");
				PrintUserCmdText(client, L"Please type \"/bank password confirm\" to proceed.");
				return;
			}
			PrintUserCmdText(client, L"This will generate a new password and the previous will be invalid");
			PrintUserCmdText(client,
			    L"Your currently set password is " + bank->bankPassword +
			        L" if you are sure you want to regenerate your password type \"/bank password confirm\". ");
		}
		else if (cmd == L"transfer")
		{
			TransferMoney(client, param);
		}
		else if (cmd == L"info")
		{
			const auto bank = GetBank(client);
			const bool showPass = GetParam(param, ' ', 1) == L"pass";
			ShowBankInfo(client, bank, showPass);
		}
		else
		{
			PrintUserCmdText(client, L"Here are the available commands for the bank plugin");
			PrintUserCmdText(client, L"\"/bank withdraw \" will withdraw money from your account's bank");
			PrintUserCmdText(client, L"\"/bank deposit\" will deposit money from your character to your bank");
			PrintUserCmdText(client, L"\"/bank withdraw bankID password\" will withdraw money from a specified bank that has a password set up");
			PrintUserCmdText(client, L"\"/bank transfer bankID \" transfer money from your current bank to the target Bank ID");
			PrintUserCmdText(client, L"\"/bank password \" will generate a password for your bank or regen one of you already have one");
			PrintUserCmdText(client, L"\"/bank info \" will display information regarding your current account's bank");
		}
	}

	const std::vector commands = {
	    {CreateUserCommand(L"/bank", L"", UserCommandHandler, L"A series of commands for storing money that can be shared among multiple characters.")}};

	BankCode __stdcall IpcConsumeBankCash(std::wstring accountId, int cashAmount, const std::wstring_view& transactionSource)
	{
		if (cashAmount <= 0)
		{
			return BankCode::CannotWithdrawNegativeNumber;
		}

		if (cashAmount > global->config->maximumTransfer)
		{
			return BankCode::AboveMaximumTransferThreshold;
		}

		if (cashAmount < global->config->minimumTransfer)
		{
			return BankCode::AboveMaximumTransferThreshold;
		}

		const auto bank = GetBank(accountId);
		if (!bank)
		{
			return BankCode::NoBank;
		}

		if (bank->cash < cashAmount)
		{
			return BankCode::NotEnoughMoney;
		}

		const auto fee = static_cast<long long>(cashAmount) + global->config->transferFee;
		if (global->config->transferFee > 0 && bank->cash - fee < 0)
		{
			return BankCode::BankCouldNotAffordTransfer;
		}

		bank->cash -= fee;

		Transaction transaction;
		transaction.accessor = transactionSource;
		transaction.amount = fee;
		bank->transactions.emplace(bank->transactions.begin(), std::move(transaction));

		bank->Save();
		return BankCode::Success;
	}

	bool ShouldSuppressBuy(const void* dummy [[maybe_unused]], const ClientId& client)
	{
		if (!global->config->preventTransactionsNearThreshold)
			return false;

		if (const auto currentValue = Hk::Player::GetShipValue(client).value(); global->config->cashThreshold < static_cast<int>(currentValue))
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
REFL_AUTO(type(Bank), field(accountId), field(bankPassword), field(cash), field(transactions));
REFL_AUTO(type(Transaction), field(lastAccessedUnix), field(accessor), field(amount));

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
	pi->emplaceHook(HookedCall::IServerImpl__Login, &OnPlayerLogin);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodSell, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqAddItem, &ReqAddItem);
	pi->emplaceHook(HookedCall::IServerImpl__ReqChangeCash, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqEquipment, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqHullStatus, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqSetCash, &ShouldSuppressBuy);
	pi->emplaceHook(HookedCall::IServerImpl__ReqShipArch, &ShouldSuppressBuy);
}