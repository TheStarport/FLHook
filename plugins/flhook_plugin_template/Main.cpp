// This is a template with the bare minimum to have a functional plugin.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.hpp"

#include <random>

namespace Plugins::Template
{
	const auto global = std::make_unique<Global>();

	// Put things that are performed on plugin load here!
	void LoadSettings()
	{
		// Load JSON config
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(std::move(config));
	}

	// Demo command
	void UserCmdTemplate(const uint& iClientID, const std::wstring_view& wscParam)
	{
		// Access our config value
		if (global->config->overrideUserNumber)
		{
			std::random_device dev;
			std::mt19937 rng(dev());
			const std::uniform_int_distribution<std::mt19937::result_type> dist(1, 6); // distribution in range [1, 1000]

			PrintUserCmdText(iClientID, L"The gods decided your number is actually: " +  std::to_wstring(dist(rng)));
			return;
		}

		if (const auto number = ToInt(GetParam(wscParam, ' ', 0)); number > 0)
		{
			PrintUserCmdText(iClientID, L"You put in the following number: " + std::to_wstring(number));
		}
		else
		{
			PrintUserCmdText(iClientID, L"ERR: You must provide a valid positive non-zero number.");
		}
	}

	// Define usable chat commands here
	const std::vector<UserCommand> commands = {{
	    { L"/template", L"<number>", UserCmdTemplate, L"Outputs a user provided non-zero number." },
	}};

	// Demo admin command
	void AdminCmdTemplate(CCmds* cmds, float number)
	{
		if (cmds->ArgStrToEnd(1).length() == 0)
		{
			cmds->Print(L"ERR Usage: template <number>");
			return;
		}

		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		cmds->Print(L"Template is %0.0f", number);
		return;
	}


	// This is called when an admin does .help
	void CmdHelp(CCmds* command)
	{
		command->Print(L"template <number>");
	}

	// Admin command callback. Compare the chat entry to see if it match a command
	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		if (wscCmd == L"template")
		{
			AdminCmdTemplate(cmds, cmds->ArgFloat(1));
		}
		else
		{
			return false;	
		}

		global->returnCode = ReturnCode::SkipAll;
		return true;
	}

}

using namespace Plugins::Template;

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	// Full name of your plugin
	pi->name("$projectname$");
	// Shortened name, all lower case, no spaces. Abbreviation when possible.
	pi->shortName("$safeprojectname$");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Help, &CmdHelp);
}