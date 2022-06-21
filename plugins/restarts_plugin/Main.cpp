// Restarts plugin - Feb 2010 by Cannon
//
// Ported by Raikkonen & Nen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

namespace Plugins::Restart
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

void LoadSettings()
{
	auto config = Serializer::JsonToObject<Config>();

	global->config = std::make_unique<Config>(config);

}

/* User Commands */

void UserCmd_ShowRestarts(uint iClientID, const std::wstring& wscParam)
{
	if (global->config->restartCosts.empty())
	{
		PrintUserCmdText(iClientID, L"There are no restarts available." );

	}

	PrintUserCmdText(iClientID, L"You can use these restarts:");
	for (const auto& [key,value] : global->config->restartCosts )
	{
		if (global->config->enableRestartCost)
		{
			PrintUserCmdText(iClientID, L"%s - $%i", key.c_str(), value);
		}
		else
		{
			PrintUserCmdText(iClientID, L"%s", key.c_str());
		}

	}
}

void UserCmd_Restart(uint iClientID, const std::wstring& wscParam)
{
	std::wstring restartTemplate = GetParam(wscParam, ' ', 0);
	if (!restartTemplate.length())
	{
		PrintUserCmdText(iClientID, L"ERR Invalid parameters");
		PrintUserCmdText(iClientID, L"/restart <template>");
	}

	// Get the character name for this connection.
	Restart restart;
	restart.characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(iClientID));

	// Searching restart

	for (const auto entity : std::filesystem::directory_iterator("flhook_plugins/restart"))
	{
		if (entity.is_directory() || entity.path().extension().string() != ".fl")
		{
			continue;
		}
		if (entity.path().filename().string() == wstos(restartTemplate + L".fl") )
		{
			PrintUserCmdText(iClientID, L"ERR Template does not exist");
		}
	}


	// Saving the characters forces an anti-cheat checks and fixes
	// up a multitude of other problems.
	HkSaveChar(iClientID);
	if (!HkIsValidClientID(iClientID))
		return;

	uint iBaseID;
	pub::Player::GetBase(iClientID, iBaseID);
	if (!iBaseID)
	{
		PrintUserCmdText(iClientID, L"ERR Not in base");
		return;
	}

	if (global->config->maxRank != 0)
	{
		int rank = 0;
		HkGetRank(restart.characterName, rank);
		if (rank == 0 || rank > global->config->maxRank)
		{
			PrintUserCmdText(
			    iClientID,
			    L"ERR You must create a new char to "
			    L"restart. Your rank is too high");
			return;
		}
	}

	HK_ERROR err;
	int cash = 0;
	if ((err = HkGetCash(restart.characterName, cash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}

	if (global->config->maxCash != 0 && cash > global->config->maxCash)
	{
		PrintUserCmdText(
		    iClientID,
		    L"ERR You must create a new char to "
		    L"restart. Your cash is too high");
		return;
	}

	if (global->config->enableRestartCost)
	{
		if (cash < global->config->restartCosts[restartTemplate])
		{
			PrintUserCmdText(
			    iClientID, L"You need $" + std::to_wstring(global->config->restartCosts[restartTemplate] - cash) +
			        L" more credits to use this template");
			return;
		}
		restart.cash = cash - global->config->restartCosts[restartTemplate];
	}
	else
		restart.cash = cash;

	CAccount* acc = Players.FindAccountFromClientID(iClientID);
	if (acc)
	{
		HkGetAccountDirName(acc, restart.directory);
		HkGetCharFileName(restart.characterName, restart.characterFile);
		global->pendingRestarts.push_back(restart);
		HkKickReason(restart.characterName, L"Updating character, please wait 10 seconds before reconnecting");
	}
	return;
}

/* Hooks */

void Timer()
{
	while (global->pendingRestarts.size())
	{
		Restart restart = global->pendingRestarts.front();
		if (HkGetClientIdFromCharname(restart.characterName) != -1)
			return;

		global->pendingRestarts.pop_front();

		try
		{
			// Overwrite the existing character file
			std::string scCharFile = scAcctPath + wstos(restart.directory) + "\\" + wstos(restart.characterFile) + ".fl";
			std::string scTimeStampDesc = IniGetS(scCharFile, "Player", "description", "");
			std::string scTimeStamp = IniGetS(scCharFile, "Player", "tstamp", "0");
			if (!::CopyFileA(restart.restartFile.c_str(), scCharFile.c_str(), FALSE))
				throw("copy template");

			flc_decode(scCharFile.c_str(), scCharFile.c_str());
			IniWriteW(scCharFile, "Player", "name", restart.characterName);
			IniWrite(scCharFile, "Player", "description", scTimeStampDesc);
			IniWrite(scCharFile, "Player", "tstamp", scTimeStamp);
			IniWrite(scCharFile, "Player", "money", std::to_string(restart.cash));

			if (!FLHookConfig::i()->general.disableCharfileEncryption)
				flc_encode(scCharFile.c_str(), scCharFile.c_str());

			AddLog(LogType::Normal, LogLevel::Info, L"NOTICE: User restart %s for %s", restart.restartFile.c_str(),
			    wstos(restart.characterName).c_str());
		}
		catch (char* err)
		{
			AddLog(LogType::Normal, LogLevel::Info, L"ERROR: User restart failed (%s) for %s", err, wstos(restart.characterName).c_str());
		}
		catch (...)
		{
			AddLog(LogType::Normal, LogLevel::Info, L"ERROR: User restart failed for %s", wstos(restart.characterName).c_str());
		}
	}
}

// Client command processing
const std::array<USERCMD, 2> UserCmds = {{
    {L"/restart", UserCmd_Restart},
    {L"/showrestarts", UserCmd_ShowRestarts},
}};

// Hook on /help
void UserCmd_Help(uint& iClientID, const std::wstring& wscParam)
{
	PrintUserCmdText(iClientID, L"/restart <template>");
	PrintUserCmdText(iClientID, L"/showrestarts");
}

	} // namespace Plugins::RESTART
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::Restart;

REFL_AUTO(type(Config), field(maxCash), field(maxRank), field(enableRestartCost), field(restartCosts))



bool ProcessUserCmds(uint& clientId, const std::wstring& param)
{
	return DefaultUserCommandHandling(clientId, param, UserCmds, global->returnCode);
}

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Restarts");
	pi->shortName("restarts");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &Timer);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &ProcessUserCmds);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
}