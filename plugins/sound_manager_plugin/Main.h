#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::SoundManager
{
	struct Config final : Reflectable
	{
		// This json file should contain a "sounds" array containing a list of sounds for e.g. dx_s032a_01a01_hvis_xtr_1
		std::string File() override { return "flhook_plugins/soundmanager.json"; }

		// Reflectable fields
		std::vector<std::string> sounds;

		// Non-reflectable fields
		std::vector<uint> sound_ids;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		// Other fields
		ReturnCode returncode = ReturnCode::Default;
	};
}

