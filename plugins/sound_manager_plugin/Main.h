#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::SoundManager
{
	//! Config data for this plugin
	struct Config final : Reflectable
	{
		//! This json file should contain a "sounds" array containing a list of sounds for e.g. dx_s032a_01a01_hvis_xtr_1
		std::string File() override { return "flhook_plugins/sound_manager.json"; }

		//! A vector of sounds in their Freelancer name format
		std::vector<std::string> sounds;

		//! A vector of sounds converted to their ids
		std::vector<uint> sound_ids;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
	};
}

