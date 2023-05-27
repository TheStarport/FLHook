/**
 * @date Jan, 2023
 * @author Raikkonen, ported by Nen
 * @defgroup Wardrobe Wardrobe
 * @brief
 * The Wardrobe plugin allows players to change their body and head models from a defined list of allowed models.
 *
 * @paragraph cmds Player Commands
 * -wardrobe show <head/body> - lists available heads or bodies
 * -wardrobe change <head/body> - changes your character model to selected head or body
 *
 * @paragraph adminCmds Admin Commands
 * None
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "bodies": {
 *         "ExampleBody": "ku_edo_body"
 *     },
 *     "heads": {
 *         "ExampleHead": "ku_edo_head"
 *     }
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

#include "Wardrobe.h"

namespace Plugins::Wardrobe
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void UserCmdShowWardrobe(ClientId& client, const std::wstring& param)
	{
		const std::wstring type = GetParam(param, ' ', 0);

		if (ToLower(type) == L"head")
		{
			PrintUserCmdText(client, L"Heads:");
			std::wstring heads;
			for (const auto& [name, id] : global->config->heads)
				heads += (StringUtils::stows(name) + L" | ");
			PrintUserCmdText(client, heads);
		}
		else if (ToLower(type) == L"body")
		{
			PrintUserCmdText(client, L"Bodies:");
			std::wstring bodies;
			for (const auto& [name, id] : global->config->bodies)
				bodies += (StringUtils::stows(name) + L" | ");
			PrintUserCmdText(client, bodies);
		}
	}

	void UserCmdChangeCostume(ClientId& client, const std::wstring& param)
	{
		const std::wstring type = GetParam(param, ' ', 0);
		const std::wstring costume = GetParam(param, ' ', 1);

		if (type.empty() || costume.empty())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			return;
		}

		Wardrobe restart;

		if (ToLower(type) == L"head")
		{
			if (!global->config->heads.contains(wstos(costume)))
			{
				PrintUserCmdText(client, L"ERR Head not found. Use \"/warehouse show head\" to get available heads.");
				return;
			}
			restart.head = true;
			restart.costume = global->config->heads[wstos(costume)];
		}
		else if (ToLower(type) == L"body")
		{
			if (!global->config->bodies.contains(wstos(costume)))
			{
				PrintUserCmdText(client, L"ERR Body not found. Use \"/warehouse show body\" to get available bodies.");
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
		Hk::Player::SaveChar(client);
		if (!Hk::Client::IsValidClientID(client))
			return;

		restart.characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

		if (const CAccount* account = Players.FindAccountFromClientID(client))
		{
			restart.directory = Hk::Client::GetAccountDirName(account);
			restart.characterFile = Hk::Client::GetCharFileName(restart.characterName).value();
			global->pendingRestarts.push_back(restart);
			Hk::Player::KickReason(restart.characterName, L"Updating character, please wait 10 seconds before reconnecting");
		}
	}

	void ProcessWardrobeRestarts()
	{
		while (!global->pendingRestarts.empty())
		{
			Wardrobe restart = global->pendingRestarts.back();
			if (Hk::Client::GetClientIdFromCharName(restart.characterName).has_value())
				return;

			global->pendingRestarts.pop_back();

			try
			{
				// Overwrite the existing character file
				std::string charFile = CoreGlobals::c()->accPath + wstos(restart.directory) + "\\" + wstos(restart.characterFile) + ".fl";
				FlcDecodeFile(charFile.c_str(), charFile.c_str());
				if (restart.head)
				{
					IniWrite(charFile, "Player", "head", " " + restart.costume);
				}
				else
					IniWrite(charFile, "Player", "body", " " + restart.costume);

				if (!FLHookConfig::i()->general.disableCharfileEncryption)
					FlcEncodeFile(charFile.c_str(), charFile.c_str());

				Logger::i()->Log(LogLevel::Info, std::format("User {} costume change to {}", wstos(restart.characterFile).c_str(), restart.costume));
			}
			catch (char* err)
			{
				Logger::i()->Log(LogLevel::Err, std::format("User {} costume change to {} ({})", wstos(restart.characterName).c_str(), restart.costume, err));
			}
			catch (...)
			{
				Logger::i()->Log(LogLevel::Err, std::format("User {} costume change to {}", wstos(restart.characterName).c_str(), restart.costume));
			}
		}
	}

	void UserCmdHandle(ClientId& client, const std::wstring& param)
	{
		// Check character is in base
		if (const auto base = Hk::Player::GetCurrentBase(client); base.has_error())
		{
			PrintUserCmdText(client, L"ERR Not in base");
			return;
		}

		const std::wstring command = GetParam(param, ' ', 0);
		if (command == L"list")
		{
			UserCmdShowWardrobe(client, GetParamToEnd(param, ' ', 1));
		}
		else if (command == L"change")
		{
			UserCmdChangeCostume(client, GetParamToEnd(param, ' ', 1));
		}
		else
		{
			PrintUserCmdText(client, L"Command usage:");
			PrintUserCmdText(client, L"/wardrobe list <head/body> - lists available bodies/heads");
			PrintUserCmdText(client, L"/wardrobe change <head/body> <name> - changes your head/body to the chosen model");
		}
	}

	const std::vector<Timer> timers = {{ProcessWardrobeRestarts, 1}};

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
	    CreateUserCommand(L"/wardrobe", L"<show/change> <head/body> [name]", UserCmdHandle, L"Shows the available heads or bodies."),
	}};

} // namespace Plugins::Wardrobe

using namespace Plugins::Wardrobe;

REFL_AUTO(type(Config), field(heads), field(bodies))

DefaultDllMainSettings(LoadSettings);

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Wardrobe Plugin");
	pi->shortName("wardrobe");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->timers(&timers);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}
