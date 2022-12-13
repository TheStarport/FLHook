// Money Manager Plugin by Cannon
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

// Setup Doxygen Group

/** @defgroup CashManager Cash Manager */

namespace Plugins::CashManager
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	//! It checks character's givecash history and prints out any received cash messages. Also fixes the money fix list, we can do this because this plugin is
	//! called before the money fix list is accessed.
	static void CheckTransferLog(ClientId client)
	{
		std::wstring characterName = ToLower(reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client)));

		const std::string logFile = GetUserFilePath(characterName, "-givecashlog.txt");
		if (logFile.empty())
			return;

		FILE* f;
		fopen_s(&f, logFile.c_str(), "r");
		if (!f)
			return;

		// A fixed length buffer be a little dangerous, but char name lengths are
		// fixed to about 30ish characters so this should be okay, and in the worst
		// case we will catch the exception.
		try
		{
			char buf[1000];
			while (fgets(buf, 1000, f) != nullptr)
			{
				std::string string = buf;
				std::wstring msg;
				uint hiByte;
				uint loByte;
				while (string.length() > 3 && sscanf_s(string.c_str(), "%02X%02X", &hiByte, &loByte) == 2)
				{
					string = string.substr(4);
					msg.append(1, static_cast<wchar_t>((hiByte << 8) | loByte));
				}
				PrintUserCmdText(client, L"%s", msg.c_str());
			}
		}
		catch (...)
		{
			AddLog(LogType::Normal, LogLevel::Err, L"Unable to read %s", logFile.c_str());
		}
		// Always close the file and remove the givecash log.
		fclose(f);
		remove(logFile.c_str());
	}

	//! Save a transfer to disk so that we can inform the receiving character when they log in. The log is recorded in ascii hex to support wide char sets.
	static void LogTransfer(std::wstring toCharacterName, std::wstring msg)
	{
		const std::string logFile = GetUserFilePath(toCharacterName, "-givecashlog.txt");
		if (logFile.empty())
			return;
		FILE* f;
		fopen_s(&f, logFile.c_str(), "at");
		if (!f)
			return;

		try
		{
			for (uint i = 0; (i < msg.length()); i++)
			{
				const char hiByte = msg[i] >> 8;
				const char loByte = msg[i] & 0xFF;
				fprintf(f, "%02X%02X", static_cast<uint>(hiByte) & 0xFF, static_cast<uint>(loByte) & 0xFF);
			}
			fprintf(f, "\n");
		}
		catch (...)
		{
			AddLog(LogType::Normal, LogLevel::Err, L"Unable to log transfer %s", logFile.c_str());
		}
		fclose(f);
		return;
	}

	//! Return if this char is in the blocked system
	static bool InBlockedSystem(const std::wstring& characterName)
	{
		// An optimisation if we have no blocked systems.
		if (global->config->blockedSystemId == 0)
			return false;

		// If the char is logged in we can check in memory.
		if (ClientId client = Hk::Client::GetClientIdFromCharName(characterName))
		{
			uint system = 0;
			pub::Player::GetSystem(client, system);
			if (system == global->config->blockedSystemId)
				return true;
			return false;
		}

		// Have to check the charfile.
		std::wstring systemNickname;
		if (FLIniGet(characterName, L"system", systemNickname) != E_OK)
			return false;

		uint system = 0;
		pub::GetSystemID(system, wstos(systemNickname).c_str());
		if (system == global->config->blockedSystemId)
			return true;
		return false;
	}

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		config.blockedSystemId = CreateID(config.blockedSystem.c_str());
		global->config = std::make_unique<Config>(config);
	}

	//! Check for cash transfer while this char was offline whenever they enter or leave a base.
	void PlayerLaunch(uint& ship, ClientId& client) { CheckTransferLog(client); }

	//! Check for cash transfer while this char was offline whenever they enter or leave a base.
	void BaseEnter(uint& baseId, ClientId& client) { CheckTransferLog(client); }

	//! Process a give cash command
	void UserCmdGiveCash(ClientId& client, const std::wstring_view& param)
	{
		// The last error.
		Error error;

		// Get the current character name
		std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

		// Get the parameters from the user command.
		std::wstring targetCharacter = GetParam(param, L' ', 0);
		std::wstring wscCash = GetParam(param, L' ', 1);
		const std::wstring wscAnon = GetParam(param, L' ', 2);
		wscCash = ReplaceStr(wscCash, L".", L"");
		wscCash = ReplaceStr(wscCash, L",", L"");
		wscCash = ReplaceStr(wscCash, L"$", L"");
		const int cash = ToInt(wscCash);
		if ((!targetCharacter.length() || cash <= 0) || (!wscAnon.empty() && wscAnon != L"anon"))
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /givecash <charname> <cash> [anon]");
			return;
		}

		bool bAnon = false;
		if (wscAnon == L"anon")
			bAnon = true;

		if (GetAccountByCharname(targetCharacter) == nullptr)
		{
			PrintUserCmdText(client, L"ERR char does not exist");
			return;
		}

		int secs = 0;
		GetOnlineTime(characterName, secs);
		if (secs < global->config->minimumTime)
		{
			PrintUserCmdText(client, L"ERR insufficient time online");
			return;
		}

		if (InBlockedSystem(characterName) || InBlockedSystem(targetCharacter))
		{
			PrintUserCmdText(client, L"ERR cash transfer blocked");
			return;
		}

		// Read the current number of credits for the player
		// and check that the character has enough cash.
		int currentCash = 0;
		if ((error = GetCash(characterName, currentCash)) != E_OK)
		{
			PrintUserCmdText(client, L"ERR " + ErrGetText(error));
			return;
		}
		if (cash < global->config->minimumTransfer || cash < 0)
		{
			PrintUserCmdText(client, L"ERR Transfer too small, minimum transfer " + ToMoneyStr(global->config->minimumTransfer) + L" credits");
			return;
		}
		if (currentCash < cash)
		{
			PrintUserCmdText(client, L"ERR Insufficient credits");
			return;
		}

		// Prevent target ship from becoming corrupt.
		float targetValue = 0.0f;
		if (GetShipValue(targetCharacter, targetValue) != E_OK)
		{
			PrintUserCmdText(client, L"ERR " + ErrGetText(error));
			return;
		}
		if ((targetValue + static_cast<float>(cash)) > 2000000000.0f)
		{
			PrintUserCmdText(client, L"ERR Transfer will exceed credit limit");
			return;
		}

		// Calculate the new cash
		int expectedCash = 0;
		if ((error = GetCash(targetCharacter, expectedCash)) != E_OK)
		{
			PrintUserCmdText(client, L"ERR Get cash failed err=" + ErrGetText(error));
			return;
		}
		expectedCash += cash;

		// Do an anticheat check on the receiving character first.
		uint targetClientId = Hk::Client::GetClientIdFromCharName(targetCharacter);
		if (targetClientId && !IsInCharSelectMenu(targetClientId))
		{
			if (AntiCheat(targetClientId) != E_OK)
			{
				PrintUserCmdText(client, L"ERR Transfer failed");
				AddLog(LogType::Cheater,
				    LogLevel::Info,
				    L"NOTICE: Possible cheating when sending %s credits from %s (%s) to %s (%s)",
				    ToMoneyStr(cash).c_str(),
				    characterName.c_str(),
				    GetAccountID(GetAccountByCharname(characterName)).c_str(),
				    targetCharacter.c_str(),
				    GetAccountID(GetAccountByCharname(targetCharacter)).c_str());
				return;
			}
			SaveChar(targetClientId);
		}

		if (targetClientId && (ClientInfo[client].iTradePartner || ClientInfo[targetClientId].iTradePartner))
		{
			PrintUserCmdText(client, L"ERR Trade window open");
			AddLog(LogType::Normal,
			    LogLevel::Info,
			    L"NOTICE: Trade window open when sending %s credits from %s (%s) to %s (%s) %u %u",
			    ToMoneyStr(cash).c_str(),
			    characterName.c_str(),
			    GetAccountID(GetAccountByCharname(characterName)).c_str(),
			    targetCharacter.c_str(),
			    GetAccountID(GetAccountByCharname(targetCharacter)).c_str(),
			    client,
			    targetClientId);
			return;
		}

		// Remove cash from current character and save it checking that the
		// save completes before allowing the cash to be added to the target ship.
		if ((error = AddCash(characterName, 0 - cash)) != E_OK)
		{
			PrintUserCmdText(client, L"ERR Remove cash failed err=" + ErrGetText(error));
			return;
		}

		if (AntiCheat(client) != E_OK)
		{
			PrintUserCmdText(client, L"ERR Transfer failed");
			AddLog(LogType::Cheater,
			    LogLevel::Info,
			    L"NOTICE: Possible cheating when sending %s credits from %s (%s) to %s (%s)",
			    ToMoneyStr(cash).c_str(),
			    characterName.c_str(),
			    GetAccountID(GetAccountByCharname(characterName)).c_str(),
			    targetCharacter.c_str(),
			    GetAccountID(GetAccountByCharname(targetCharacter)).c_str());
			return;
		}
		SaveChar(client);

		// Add cash to target character
		if ((error = AddCash(targetCharacter, cash)) != E_OK)
		{
			PrintUserCmdText(client, L"ERR Add cash failed err=" + ErrGetText(error));
			return;
		}

		if (targetClientId && !IsInCharSelectMenu(targetClientId))
		{
			if (AntiCheat(targetClientId) != E_OK)
			{
				PrintUserCmdText(client, L"ERR Transfer failed");
				AddLog(LogType::Cheater,
				    LogLevel::Info,
				    L"NOTICE: Possible cheating when sending %s credits from %s (%s) to %s (%s)",
				    ToMoneyStr(cash).c_str(),
				    characterName.c_str(),
				    GetAccountID(GetAccountByCharname(characterName)).c_str(),
				    targetCharacter.c_str(),
				    GetAccountID(GetAccountByCharname(targetCharacter)).c_str());
				return;
			}
			SaveChar(targetClientId);
		}

		// Check that receiving character has the correct amount of cash.
		if (int targetCurrentCash; (GetCash(targetCharacter, targetCurrentCash)) != E_OK || targetCurrentCash != expectedCash)
		{
			AddLog(LogType::Normal,
			    LogLevel::Err,
			    L"Cash transfer error when sending %s credits from %s (%s) "
			    "to "
			    "%s (%s) current %s credits expected %s credits ",
			    ToMoneyStr(cash).c_str(),
			    characterName.c_str(),
			    GetAccountID(GetAccountByCharname(characterName)).c_str(),
			    targetCharacter.c_str(),
			    GetAccountID(GetAccountByCharname(targetCharacter)).c_str(),
			    ToMoneyStr(targetCurrentCash).c_str(),
			    ToMoneyStr(expectedCash).c_str());
			PrintUserCmdText(client, L"ERR Transfer failed");
			return;
		}

		// If the target player is online then send them a message saying
		// telling them that they've received the cash.

		if (targetClientId && !IsInCharSelectMenu(targetClientId))
		{
			const std::wstring msg = L"You have received " + ToMoneyStr(cash) + L" credits from " + (bAnon ? L"anonymous" : characterName);
			PrintUserCmdText(targetClientId, L"%s", msg.c_str());
		}
		// Otherwise we assume that the character is offline so we record an entry
		// in the character's givecash.ini. When they come online we inform them
		// of the transfer. The ini is cleared when ever the character logs in.
		else
		{
			const std::wstring msg = L"You have received " + ToMoneyStr(cash) + L" credits from " + (bAnon ? L"anonymous" : characterName);
			LogTransfer(targetCharacter, msg);
		}

		AddLog(LogType::Normal,
		    LogLevel::Info,
		    L"Send %s credits from %s (%s) to %s (%s)",
		    ToMoneyStr(cash).c_str(),
		    characterName.c_str(),
		    GetAccountID(GetAccountByCharname(characterName)).c_str(),
		    targetCharacter.c_str(),
		    GetAccountID(GetAccountByCharname(targetCharacter)).c_str());

		// A friendly message explaining the transfer.
		std::wstring msg = L"You have sent " + ToMoneyStr(cash) + L" credits to " + targetCharacter;
		if (bAnon)
			msg += L" anonymously";
		PrintUserCmdText(client, L"%s", msg.c_str());
		return;
	}

	//! Process a set cash code command
	void UserCmdSetCashCode(ClientId& client, const std::wstring_view& param)
	{
		std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));
		const std::string scFile = GetUserFilePath(characterName, "-givecash.ini");
		if (scFile.empty())
			return;

		const std::wstring code = GetParam(param, L' ', 0);

		if (code.empty())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /set cashcode <code>");
		}
		else if (code == L"none")
		{
			IniWriteW(scFile, "Settings", "Code", L"");
			PrintUserCmdText(client, L"OK Account code cleared");
		}
		else
		{
			IniWriteW(scFile, "Settings", "Code", code);
			PrintUserCmdText(client, L"OK Account code set to " + code);
		}
		return;
	}

	//! Process a show cash command
	void UserCmdShowCash(ClientId& client, const std::wstring_view& param)
	{
		// The last error.
		Error error;

		// Get the current character name
		std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

		// Get the parameters from the user command.
		std::wstring targetCharacterName = GetParam(param, L' ', 0);
		const std::wstring code = GetParam(param, L' ', 1);

		if (!targetCharacterName.length() || !code.length())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /showcash <charname> <code>");
			return;
		}

		if (CAccount const* acc = GetAccountByCharname(targetCharacterName); acc == nullptr)
		{
			PrintUserCmdText(client, L"ERR char does not exist");
			return;
		}

		const std::string file = GetUserFilePath(targetCharacterName, "-givecash.ini");
		if (file.empty())
			return;

		if (const std::wstring targetCode = IniGetWS(file, "Settings", "Code", L""); !targetCode.length() || targetCode != code)
		{
			PrintUserCmdText(client, L"ERR cash account access denied");
			return;
		}

		int cash = 0;
		if ((error = GetCash(targetCharacterName, cash)) != E_OK)
		{
			PrintUserCmdText(client, L"ERR " + ErrGetText(error));
			return;
		}

		PrintUserCmdText(client, L"OK Account " + targetCharacterName + L" has " + ToMoneyStr(cash) + L" credits");
		return;
	}

	//! Process a draw cash command
	void UserCmdDrawCash(ClientId& client, const std::wstring_view& param)
	{
		// The last error.
		Error error;

		// Get the current character name
		std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

		// Get the parameters from the user command.
		std::wstring targetCharacterName = GetParam(param, L' ', 0);
		const std::wstring code = GetParam(param, L' ', 1);
		std::wstring cashString = GetParam(param, L' ', 2);
		cashString = ReplaceStr(cashString, L".", L"");
		cashString = ReplaceStr(cashString, L",", L"");
		cashString = ReplaceStr(cashString, L"$", L"");
		const int cash = ToInt(cashString);
		if (!targetCharacterName.length() || !code.length() || !cash)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /drawcash <charname> <code> <cash>");
			return;
		}

		if (CAccount const* iTargetAcc = GetAccountByCharname(targetCharacterName); iTargetAcc == nullptr)
		{
			PrintUserCmdText(client, L"ERR char does not exist");
			return;
		}

		int secs = 0;
		GetOnlineTime(targetCharacterName, secs);
		if (secs < global->config->minimumTime)
		{
			PrintUserCmdText(client, L"ERR insufficient time online");
			return;
		}

		if (InBlockedSystem(characterName) || InBlockedSystem(targetCharacterName))
		{
			PrintUserCmdText(client, L"ERR cash transfer blocked");
			return;
		}

		const std::string scFile = GetUserFilePath(targetCharacterName, "-givecash.ini");
		if (scFile.empty())
			return;

		if (const std::wstring wscTargetCode = IniGetWS(scFile, "Settings", "Code", L""); !wscTargetCode.length() || wscTargetCode != code)
		{
			PrintUserCmdText(client, L"ERR cash account access denied");
			return;
		}

		if (cash < global->config->minimumTransfer || cash < 0)
		{
			PrintUserCmdText(client, L"ERR Transfer too small, minimum transfer " + ToMoneyStr(global->config->minimumTransfer) + L" credits");
			return;
		}

		int tCash = 0;
		if ((error = GetCash(targetCharacterName, tCash)) != E_OK)
		{
			PrintUserCmdText(client, L"ERR " + ErrGetText(error));
			return;
		}
		if (tCash < cash)
		{
			PrintUserCmdText(client, L"ERR Insufficient credits");
			return;
		}

		// Check the adding this cash to this player will not
		// exceed the maximum ship value.
		float fTargetValue = 0.0f;
		if (GetShipValue(characterName, fTargetValue) != E_OK)
		{
			PrintUserCmdText(client, L"ERR " + ErrGetText(error));
			return;
		}
		if ((fTargetValue + static_cast<float>(cash)) > 2000000000.0f)
		{
			PrintUserCmdText(client, L"ERR Transfer will exceed credit limit");
			return;
		}

		// Calculate the new cash
		int iExpectedCash = 0;
		if ((error = GetCash(characterName, iExpectedCash)) != E_OK)
		{
			PrintUserCmdText(client, L"ERR " + ErrGetText(error));
			return;
		}
		iExpectedCash += cash;

		// Do an anticheat check on the receiving ship first.
		if (AntiCheat(client) != E_OK)
		{
			PrintUserCmdText(client, L"ERR Transfer failed");
			AddLog(LogType::Cheater,
			    LogLevel::Info,
			    L"NOTICE: Possible cheating when drawing %s credits from %s (%s) to "
			    "%s (%s)",
			    ToMoneyStr(cash).c_str(),
			    targetCharacterName.c_str(),
			    GetAccountID(GetAccountByCharname(targetCharacterName)).c_str(),
			    characterName.c_str(),
			    GetAccountID(GetAccountByCharname(characterName)).c_str());
			return;
		}
		SaveChar(client);

		uint targetClientId = Hk::Client::GetClientIdFromCharName(targetCharacterName);
		if (targetClientId && ClientInfo[client].iTradePartner || ClientInfo[targetClientId].iTradePartner)
		{
			PrintUserCmdText(client, L"ERR Trade window open");
			AddLog(LogType::Normal,
			    LogLevel::Info,
			    L"NOTICE: Trade window open when drawing %s credits from %s "
			    "(%s) "
			    "to %s (%s) %u %u",
			    ToMoneyStr(cash).c_str(),
			    targetCharacterName.c_str(),
			    GetAccountID(GetAccountByCharname(targetCharacterName)).c_str(),
			    characterName.c_str(),
			    GetAccountID(GetAccountByCharname(characterName)).c_str(),
			    client,
			    targetClientId);
			return;
		}

		// Remove cash from target character
		if ((error = AddCash(targetCharacterName, 0 - cash)) != E_OK)
		{
			PrintUserCmdText(client, L"ERR " + ErrGetText(error));
			return;
		}

		if (targetClientId && !IsInCharSelectMenu(targetClientId))
		{
			if (AntiCheat(targetClientId) != E_OK)
			{
				PrintUserCmdText(client, L"ERR Transfer failed");
				AddLog(LogType::Cheater,
				    LogLevel::Info,
				    L"NOTICE: Possible cheating when drawing %s credits from %s (%s) to %s (%s)",
				    ToMoneyStr(cash).c_str(),
				    targetCharacterName.c_str(),
				    GetAccountID(GetAccountByCharname(targetCharacterName)).c_str(),
				    characterName.c_str(),
				    GetAccountID(GetAccountByCharname(characterName)).c_str());
				return;
			}
			SaveChar(targetClientId);
		}

		// Add cash to this player
		if ((error = AddCash(characterName, cash)) != E_OK)
		{
			PrintUserCmdText(client, L"ERR " + ErrGetText(error));
			return;
		}

		if (AntiCheat(client) != E_OK)
		{
			PrintUserCmdText(client, L"ERR Transfer failed");
			AddLog(LogType::Cheater,
			    LogLevel::Info,
			    L"NOTICE: Possible cheating when drawing %s credits from %s (%s) to "
			    "%s (%s)",
			    ToMoneyStr(cash).c_str(),
			    targetCharacterName.c_str(),
			    GetAccountID(GetAccountByCharname(targetCharacterName)).c_str(),
			    characterName.c_str(),
			    GetAccountID(GetAccountByCharname(characterName)).c_str());
			return;
		}
		SaveChar(client);

		// Check that receiving player has the correct amount of cash.
		if (int currentCash; (GetCash(characterName, currentCash)) != E_OK || currentCash != iExpectedCash)
		{
			AddLog(LogType::Normal,
			    LogLevel::Err,
			    L"Cash transfer error when drawing %s credits from %s (%s) to %s (%s) current %s credits expected %s credits ",
			    ToMoneyStr(cash).c_str(),
			    targetCharacterName.c_str(),
			    GetAccountID(GetAccountByCharname(targetCharacterName)).c_str(),
			    characterName.c_str(),
			    GetAccountID(GetAccountByCharname(characterName)).c_str(),
			    ToMoneyStr(currentCash).c_str(),
			    ToMoneyStr(iExpectedCash).c_str());
			PrintUserCmdText(client, L"ERR Transfer failed");
		}

		// If the target player is online then send them a message saying
		// telling them that they've received transfered cash.
		std::wstring msg = L"You have transferred " + ToMoneyStr(cash) + L" credits to " + characterName;
		if (targetClientId && !IsInCharSelectMenu(targetClientId))
		{
			PrintUserCmdText(targetClientId, L"%s", msg.c_str());
		}
		// Otherwise we assume that the character is offline so we record an entry
		// in the character's givecash.ini. When they come online we inform them
		// of the transfer. The ini is cleared when ever the character logs in.
		else
		{
			LogTransfer(targetCharacterName, msg);
		}

		AddLog(LogType::Normal,
		    LogLevel::Info,
		    L"NOTICE: Draw %s credits from %s (%s) to %s (%s)",
		    ToMoneyStr(cash).c_str(),
		    targetCharacterName.c_str(),
		    GetAccountID(GetAccountByCharname(targetCharacterName)).c_str(),
		    characterName.c_str(),
		    GetAccountID(GetAccountByCharname(characterName)).c_str());

		// A friendly message explaining the transfer.
		msg = GetTimeString(FLHookConfig::i()->general.localTime) + L": You have drawn " + ToMoneyStr(cash) + L" credits from " + targetCharacterName;
		PrintUserCmdText(client, L"%s", msg.c_str());
		return;
	}

	// Client command processing
	const std::vector commands = {{CreateUserCommand(L"/givecash", L"<charname> <cash> [anon]", UserCmdGiveCash, L"Sends credits to a player."),
	    CreateUserCommand(L"/sendcash", L"<charname> <cash> [anon]", UserCmdGiveCash, L"Alias of the above command."),
	    CreateUserCommand(L"/set cashcode", L"<code>", UserCmdSetCashCode,
	        L"Sets the ""code"" of this character. Other characters can withdraw cash from this character via /drawcash if they know this."),
	    CreateUserCommand(L"/showcash", L" <charname> <code>", UserCmdShowCash, L"Show the cash currently on a character."),
	    CreateUserCommand(L"/drawcash", L"<charname> <code> <cash>", UserCmdDrawCash, L"Withdrawn cash from a character.")}};
} // namespace Plugins::CashManager

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::CashManager;

// REFL_AUTO must be global namespace
REFL_AUTO(type(Config), field(minimumTransfer), field(blockedSystem), field(cheatDetection), field(minimumTime))

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
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
}
