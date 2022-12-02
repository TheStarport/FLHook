// Wardrobe
// By Raikkonen
//
//Ported by Nen 2022

#include "Main.h"

namespace Plugins::Wardrobe
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void UserCmdShowWardrobe(ClientId& client, const std::wstring_view& wscParam)
	{
		const std::wstring type = GetParam(wscParam, ' ', 0);

		if (ToLower(type) == L"heads")
		{
			PrintUserCmdText(client, L"Heads:");
			std::wstring heads;
			for (const auto& [name, id] : global->config->heads)
				heads += (stows(name) + L" | ");
			PrintUserCmdText(client, heads);
		}
		else if (ToLower(type) == L"bodies")
		{
			PrintUserCmdText(client, L"Bodies:");
			std::wstring bodies;
			for (const auto& [name, id] : global->config->bodies)
				bodies += (stows(name) + L" | ");
			PrintUserCmdText(client, bodies);
		}
	}

	void UserCmdChangeCostume(ClientId& client, const std::wstring_view& wscParam)
	{
		const std::wstring type = GetParam(wscParam, ' ', 0);
		const std::wstring costume = GetParam(wscParam, ' ', 1);

		if (!type.length() || !costume.length())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			return;
		}

		Wardrobe restart;

		if (ToLower(type) == L"head")
		{
			if (global->config->heads.find(wstos(costume)) == global->config->heads.end())
			{
				PrintUserCmdText(client, L"ERR Head not found. Use \"/show heads\" to get heads.");
				return;
			}
			restart.head = true;
			restart.costume = global->config->heads[wstos(costume)];
		}
		else if (ToLower(type) == L"body")
		{
			if (global->config->bodies.find(wstos(costume)) == global->config->bodies.end())
			{
				PrintUserCmdText(client, L"ERR Body not found. Use \"/show bodies\" to get bodies.");
				return;
			}
			restart.head = false;
			restart.costume = global->config->bodies[wstos(costume)];
		}
		else
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			return;
		}

		// Saving the characters forces an anti-cheat checks and fixes
		// up a multitude of other problems.
		SaveChar(client);
		if (!IsValidClientID(client))
			return;

		// Check character is in base
		uint baseId;
		pub::Player::GetBase(client, baseId);
		if (!baseId)
		{
			PrintUserCmdText(client, L"ERR Not in base");
			return;
		}

		restart.characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

		if (CAccount* account = Players.FindAccountFromClientID(client))
		{
			GetAccountDirName(account, restart.directory);
			GetCharFileName(restart.characterName, restart.characterFile);
			global->pendingRestarts.push_back(restart);
			KickReason(restart.characterName, L"Updating character, please wait 10 seconds before reconnecting");
		}
	}

	void TimerCheckKick()
	{
		while (!global->pendingRestarts.empty())
		{
			Wardrobe restart = global->pendingRestarts.back();
			if (GetClientIdFromCharname(restart.characterName) != -1)
				return;

			global->pendingRestarts.pop_back();

			try
			{
				// Overwrite the existing character file
				std::string scCharFile = scAcctPath + wstos(restart.directory) + "\\" + wstos(restart.characterFile) + ".fl";
				flc_decode(scCharFile.c_str(), scCharFile.c_str());
				if (restart.head)
				{
					IniWrite(scCharFile, "Player", "head", " " + restart.costume);
				}
				else
					IniWrite(scCharFile, "Player", "body", " " + restart.costume);

				if (!FLHookConfig::i()->general.disableCharfileEncryption)
					flc_encode(scCharFile.c_str(), scCharFile.c_str());

				AddLog(LogType::Normal, LogLevel::Info, L"NOTICE: User %s costume change to %u", wstos(restart.characterFile).c_str(), restart.costume);
			}
			catch (char* err)
			{
				AddLog(
				    LogType::Normal, LogLevel::Info, L"ERROR: User %s costume change to %u (%s)", wstos(restart.characterName).c_str(), restart.costume, err);
			}
			catch (...)
			{
				AddLog(LogType::Normal, LogLevel::Info, L"ERROR: User %s costume change to %u", wstos(restart.characterName).c_str(), restart.costume);
			}
		}
	}

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// USER COMMAND PROCESSING
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Define usable chat commands here
	const std::vector commands = {{
	    CreateUserCommand(L"/show", L"<heads/bodies>", UserCmdShowWardrobe, L"Shows the available heads or bodies."),
	    CreateUserCommand(L"/change", L"<head/body> <name>", UserCmdChangeCostume, L"Changes your head/body to the one specified."),
	}};

} // namespace Plugins::Wardrobe

using namespace Plugins::Wardrobe;

REFL_AUTO(type(Config), field(heads), field(bodies))

DefaultDllMainSettings(LoadSettings)

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Wardrobe Plugin");
	pi->shortName("wardrobe");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &TimerCheckKick);
}
