// Money Manager Plugin by Cannon
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"
#include "random"

// Setup Doxygen Group

/** @defgroup CashManager Cash Manager (plugin) */

namespace Plugins::CashManager
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	constexpr const char* jsonPath = R"(\AccountBank.json)";

	void LoadSettings() 
	{
		const auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(std::move(config));

		for (const auto& system : global->config->blockedSystems)
		{
			global->config->blockedSystemsHashed.emplace_back(CreateID(system.c_str()));
		}

	}

	void OnPlayerLogin(const SLoginInfo& li, ClientId& clientId)
	{
		CAccount *acc = Players.FindAccountFromClientID(clientId);
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
		if (std::filesystem::exists(path))
			return;
		
		Bank newBank;
		Serializer::SaveToJson<Bank>(newBank, path);
	}

	std::wstring GenerateAccountId()
	{
		const std::vector letters = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
                     'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r',
                     's', 't', 'u', 'v', 'w', 'x', 'y', 'z' };
		std::mt19937 r(std::random_device{});
		std::uniform_int_distribution<std::size_t> randLetter(0, letters.size() - 1);
		std::uniform_int_distribution<std::size_t> randNumber(0, 9);
		std::stringstream ss;
		ss << randLetter(r) << randNumber(r) << randNumber(r) << randNumber(r) << randNumber(r);
		return stows(ss.str());
	}
	
	void GetBank(std::variant<ClientId, std::wstring> targetBank)
	{
		CAccount* acc;
		if (targetBank.index() == 0) 
		{
			acc = Players.FindAccountFromClientID(clientId);
		}
		else
		{
			
		}
	}

	void SaveBank() 
	{
		
	}

	void UserCmdWithdraw(const ClientId& clientId, const std::wstring_view& param)
	{
		

		
	}

	void UserCmdDeposit(const ClientId& clientId, const std::wstring_view& param)
	{
		
	}

	void UserCmdPassword(const ClientId& clientId, const std::wstring_view& param)
	{
		
	}

	
	void UserCmdInfo(const ClientId& clientId, const std::wstring_view& param)
	{
		
	}

	void UserCommandHandler(const ClientId& clientId, const std::wstring_view& param)
	{
		const auto cmd = GetParam(param, L' ', 1);
		if (cmd == L"withdraw")
		{



		}
		else if (cmd == L"deposit")
		{


		}
		else if (cmd == L"password")
		{
			
		}
		else if (cmd == L"info")
		{


		}
		else
		{
			
		}
	}

	std::vector<UserCommand> commands = {{
		CreateUserCommand(L"/bank", L"", UserCommandHandler, L"A series of commands for storing money that can be shared among multiple characters.")
	}}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::CashManager;

// REFL_AUTO must be global namespace
REFL_AUTO(
    type(Config), field(minimumTransfer),field(eraseTransactionsAfterDaysPassed), field(blockedSystems), 
	field(preventTransactionsNearThreshold), field(maximumTransfer), field(cheatDetection), field(minimumTime));
REFL_AUTO(type(Bank), field(accountId), field(bankPassword), field(cash), field(lastAccessedUnix), field(transactions));
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
