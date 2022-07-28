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
				const auto bank = Serializer::JsonToObject<Bank>(json, false);
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
		const auto dev = std::random_device {};
		std::mt19937 r(dev);
		const std::uniform_int_distribution<std::size_t> randLetter(0, letters.size() - 1);
		const std::uniform_int_distribution<std::size_t> randNumber(0, 9);
		std::stringstream ss;
		ss << randLetter(r) << randNumber(r) << randNumber(r) << randNumber(r) << randNumber(r);
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

	void WithdrawMoneyFromAccount(const ClientId& clientId, const CAccount* acc, int amount) {}

	void WithdrawMoneyByPassword(const ClientId& clientId, const std::wstring_view& param) {}

	void UserCmdWithdraw(const ClientId& clientId, const std::wstring_view& param) {}

	void UserCmdDeposit(const ClientId& clientId, const std::wstring_view& param) {}

	void UserCmdPassword(const ClientId& clientId, const std::wstring_view& param) {}

	void UserCmdInfo(const ClientId& clientId, const std::wstring_view& param) {}

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

			if (const int withdrawAmount = ToInt(GetParam(param, ' ', 1)) - 1; withdrawAmount > 0)
			{
				const CAccount* acc = Players.FindAccountFromClientID(clientId);
				WithdrawMoneyFromAccount(clientId, acc, withdrawAmount);
				return;
			}

			WithdrawMoneyByPassword(clientId, param);
		}
		else if (cmd == L"deposit")
		{
		}
		else if (cmd == L"password")
		{
		}
		else if (cmd == L"info")
		{
			const auto bank = GetBank(clientId);

			const bool showPass = GetParam(param, ' ', 2) == L"pass";
			PrintUserCmdText(clientId, L"Your Bank Information: ");
			PrintUserCmdText(clientId, L"|    Account ID: %s", bank->accountId.c_str());
			PrintUserCmdText(clientId, L"|    Password: %s", showPass ? bank->bankPassword.c_str() : L"*****");
			PrintUserCmdText(clientId, L"|    Credits: %u", bank->cash);
			if (!bank->transactions.empty())
			{
				PrintUserCmdText(clientId, L"|    Last Accessed: %s", GetHumanTime(bank->transactions.front().lastAccessedUnix).c_str());	
			}

			if (!showPass)
			{
				PrintUserCmdText(clientId, L"Use /bank info pass to make the password visible.");
			}
		}
		else
		{
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