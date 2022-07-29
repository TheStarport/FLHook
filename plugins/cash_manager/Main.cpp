// Cash Manager Plugin
// Written by Lazrius & MrNen
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"
#include "refl.hpp"
#include <random>
#include <sstream>

// Setup Doxygen Group

/** @defgroup CashManager Cash Manager (plugin) */

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
			}
			else
			{
				CreateAccountBank(json, entry.path().filename().wstring());
			}
		}
	}

	void OnPlayerLogin(const SLoginInfo& li, const ClientId& clientId)
	{
		CAccount* acc = Players.FindAccountFromClientID(clientId);
		if (!acc)
		{
			return;
		}

		std::wstring dir;
		HkGetAccountDirName(acc, dir);

		std::string saveGameDir;
		saveGameDir.reserve(MAX_PATH);
		GetUserDataPath(saveGameDir.data());

		const std::string path = saveGameDir + R"(\Accts\MultiPlayer\)" + wstos(dir) + jsonPath;
		if (!std::filesystem::exists(path))
		{
			CreateAccountBank(path, acc->wszAccID);
		}
	}

	std::wstring GenerateAccountId()
	{
		const std::vector letters = {
		    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z'};
		std::random_device dev;
		std::mt19937 r(dev());
		const std::uniform_int_distribution<std::size_t> randLetter(0, letters.size() - 1);
		const std::uniform_int_distribution<std::size_t> randNumber(0, 9);
		std::stringstream ss;
		ss << letters[randLetter(r)] << randNumber(r) << randNumber(r) << randNumber(r) << randNumber(r);
		return stows(ss.str());
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
			const auto bankIter = global->banks.find(acc->wszAccID);
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

	void WithdrawMoney(std::shared_ptr<Bank> bank, int withdrawal, ClientId clientId)
	{
		if (bank->cash < withdrawal)
		{
			PrintUserCmdText(clientId, L"Error: Not enough credits, this bank only has %u", bank->cash);
			return;
		}

		if (withdrawal <= 0)
		{
			PrintUserCmdText(clientId, L"Error: Invalid withdraw amount, please input a positive number. %u", bank->cash);
			return;
		}

		HkFunc(HkAddCash, clientId, withdrawal);

		bank->cash -= withdrawal;

		Transaction transaction;

		transaction.accessor = HkGetCharacterNameById(clientId);
		transaction.amount = -withdrawal;

		bank->transactions.emplace(bank->transactions.begin(), transaction);
		bank->Save();
	}

	void DepositMoney(std::shared_ptr<Bank> bank, int deposit, ClientId clientId)
	{
		int playerCash;
		HkFunc(HkGetCash, clientId, playerCash);

		if (playerCash < deposit)
		{
			PrintUserCmdText(clientId, L"Error: Not enough credits, make sure to input a deposit number less than your balance.");
			return;
		}

		if (deposit <= 0)
		{
			PrintUserCmdText(clientId, L"Error: Invalid deposit amount, please input a positive number.");
			return;
		}

		HkFunc(HkAddCash, clientId, -deposit);

		bank->cash += deposit;

		Transaction transaction;

		transaction.accessor = HkGetCharacterNameById(clientId);
		transaction.amount = deposit;

		bank->transactions.emplace(bank->transactions.begin(), transaction);
		bank->Save();
	}

	// /bank withdraw account password amount
	void WithdrawMoneyByPassword(const ClientId& clientId, const std::wstring_view& param)
	{
		const auto accountId = GetParam(param, ' ', 1);
		const auto password = GetParam(param, ' ', 2);
		const auto withdrawal = ToInt(GetParam(param, ' ', 3));

		if (withdrawal <= 0)
		{
			PrintUserCmdText(clientId, L"Error: Invalid withdraw amount, please input a positive number.");
			return;
		}
		auto bank = GetBank(accountId);

		if (!bank)
		{
			PrintUserCmdText(clientId, L"Error: Bank accountId could not be found.");
			return;
		}
		if (password != bank->bankPassword)
		{
			PrintUserCmdText(clientId, L"Error: Invalid Password.");
			return;
		}
		WithdrawMoney(bank, withdrawal, clientId);
	}

	// /bank transfer targetBank amount
	void TransferMoney(const ClientId clientId, const std::wstring_view& param)
	{
		auto sourceBank = GetBank(clientId);
		auto targetBank = GetBank(GetParam(param, ' ', 1));
		const auto amount = ToInt(GetParam(param, ' ', 2));

		if (!targetBank)
		{
			PrintUserCmdText(clientId, L"Error: Target Bank not found");
			return;
		}

		if (amount <= 0)
		{
			PrintUserCmdText(clientId, L"Error: Invalid amount, please input a positive number.");
			return;
		}
		sourceBank->cash -= amount;
		targetBank->cash += amount;
	}

	void ShowBankInfo(const ClientId& clientId, std::shared_ptr<Bank> bank, bool showPass)
	{
		PrintUserCmdText(clientId, L"Your Bank Information: ");
		PrintUserCmdText(clientId, L"|    Account ID: %s", bank->accountId.c_str());
		if (bank->bankPassword.empty())
		{
			PrintUserCmdText(clientId, L"|    Password: None Set");
		}
		else 
		{
			PrintUserCmdText(clientId, L"|    Password: %s", showPass ? bank->bankPassword.c_str() : L"*****");
		}
		
		PrintUserCmdText(clientId, L"|    Credits: %u", bank->cash);
		if (!bank->transactions.empty())
		{
			PrintUserCmdText(clientId, L"|    Last Accessed: %s", GetHumanTime(bank->transactions.front().lastAccessedUnix).c_str());
		}

		if (!showPass && !bank->bankPassword.empty())
		{
			PrintUserCmdText(clientId, L"Use /bank info pass to make the password visible.");
		}
	}

	void UserCommandHandler(const ClientId& clientId, const std::wstring_view& param)
	{
		const auto cmd = GetParam(param, L' ', 1);
		if (cmd == L"withdraw")
		{
			if (global->config->preventTransactionsNearThreshold)
			{
				int cash;
				HkFunc(HkGetCash, clientId, cash);

				if (cash > global->config->cashThreshold)
				{
					PrintUserCmdText(clientId,
					    L"You cannot withdraw more cash. Your current value is dangerously high. "
					    L"Please deposit money to bring your value back into normal range.");
					return;
				}
			}

			if (const int withdrawAmount = ToInt(GetParam(param, ' ', 1)) - 1)
			{
				const CAccount* acc = Players.FindAccountFromClientID(clientId);
				const auto bank = GetBank(acc->wszAccID);
				WithdrawMoney(bank, withdrawAmount, clientId);
				return;
			}

			WithdrawMoneyByPassword(clientId, param);
		}
		else if (cmd == L"deposit")
		{
			const int depositAmount = ToInt(GetParam(param, ' ', 1)) - 1;
			const CAccount* acc = Players.FindAccountFromClientID(clientId);
			const auto bank = GetBank(acc->wszAccID);
			DepositMoney(bank, depositAmount, clientId);
		}

		else if (cmd == L"password")
		{
			const CAccount* acc = Players.FindAccountFromClientID(clientId);
			const auto bank = GetBank(acc->wszAccID);

			if (GetParam(param, ' ', 2) == L"confirm")
			{
				bank->bankPassword = GenerateAccountId();
				PrintUserCmdText(clientId, L"Your bank account information has been updated.");
				ShowBankInfo(clientId, bank, true);
				return;
			}

			if (bank->bankPassword.empty())
			{
				PrintUserCmdText(clientId, L"Your bank currently does not have a password set");
				PrintUserCmdText(clientId, L"Generating a password means anybody with the password and Bank ID can access your bank");
				PrintUserCmdText(clientId, L"Are you sure you want to generate a password for this account?");
				PrintUserCmdText(clientId, L"Please type \"/bank password confirm\" to proceed.");
				return;
			}
			PrintUserCmdText(clientId, L"This will generate a new password and the previous will be invalid");
			PrintUserCmdText(clientId,
			    L"Your currently set password is " + bank->bankPassword + L" if you are sure you want to regenerate your password type \"/bank password confirm\". ");
			
		}
		else if (cmd == L"transfer")
		{
			TransferMoney(clientId, param);
		}
		else if (cmd == L"info")
		{
			const auto bank = GetBank(clientId);
			const bool showPass = GetParam(param, ' ', 1) == L"pass";
			ShowBankInfo(clientId, bank, showPass);
		}
		else
		{
			PrintUserCmdText(clientId, L"Here are the available commands for the bank plugin");
			PrintUserCmdText(clientId, L"\"/bank withdraw \" will withdraw money from your account's bank");
			PrintUserCmdText(clientId, L"\"/bank deposit\" will deposit money from your character to your bank");
			PrintUserCmdText(clientId, L"\"/bank withdraw bankID password\" will withdraw money from a specified bank that has a password set up");
			PrintUserCmdText(clientId, L"\"/bank transfer bankID \" transfer money from your current bank to the target Bank ID");
			PrintUserCmdText(clientId, L"\"/bank password \" will generate a password for your bank or regen one of you already have one");
			PrintUserCmdText(clientId, L"\"/bank info \" will display information regarding your current account's bank");
		}
	}

	std::vector<UserCommand> commands = {
	    {CreateUserCommand(L"/bank", L"", UserCommandHandler, L"A series of commands for storing money that can be shared among multiple characters.")}};
} // namespace Plugins::CashManager

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::CashManager;

// REFL_AUTO must be global namespace
REFL_AUTO(type(Config), field(minimumTransfer), field(eraseTransactionsAfterDaysPassed), field(blockedSystems), field(preventTransactionsNearThreshold),
    field(maximumTransfer), field(cheatDetection), field(minimumTime));
REFL_AUTO(type(Bank), field(accountId), field(bankPassword), field(cash), field(transactions));
REFL_AUTO(type(Transaction), field(lastAccessedUnix), field(accessor), field(amount));

DefaultDllMainSettings(LoadSettings)

    extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Cash Manager Plugin");
	pi->shortName("cash_manager");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &OnPlayerLogin);
}