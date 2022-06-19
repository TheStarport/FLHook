#pragma once

// Included
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::BountyHunt
{
	//! Macros
	#define PRINT_DISABLED() PrintUserCmdText(iClientID, L"Command disabled");

	//! Structs
	struct BOUNTY_HUNT
	{
		uint uiTargetID;
		uint uiInitiatorID;
		std::wstring wscTarget;
		std::wstring wscInitiator;
		int iCash;
		mstime msEnd;
	};

	//! Configurable fields for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/bountyhunt.json"; }

		// Reflectable fields
		bool EnableBountyHunt = true;
		int LevelProtect = 0;
	};

	
	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
		std::list<BOUNTY_HUNT> lstBountyHunt;
	};
}

