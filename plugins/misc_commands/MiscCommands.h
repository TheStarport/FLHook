#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::MiscCommands
{
	//! The struct that holds client info for this plugin
	struct MiscClientInfo
	{
		bool lightsOn = false;
		bool shieldsDown = false;
	};

	//! Config data for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "config/misc_commands.json"; }

		//! The amount it costs to use the /droprep command.
		uint repDropCost = 0;
		bool enableDropRep = true;
		//! The message that will be displayed when the /stuck command is used.
		std::wstring stuckMessage = L"Attention! Stand Clear. Towing %player";
		//! The message that will be displayed when the /dice command is used.
		std::wstring diceMessage = L"%player rolled %number of %max";
		//! The message that will be displayed when the /coin command is used.
		std::wstring coinMessage = L"%player tosses %result";
		//! The music that will be played when an admin activates .smiteall
		std::string smiteMusicId = "music_danger";
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		// Other fields
		ReturnCode returncode = ReturnCode::Default;

		//! The hash smite music.
		uint smiteMusicHash = 0;

		//! A map of client id to client data
		std::map<uint, MiscClientInfo> mapInfo;
	};
} // namespace Plugins::MiscCommands