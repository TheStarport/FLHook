#pragma once
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Wardrobe
{
	//! Struct that holds a pending wardrobe change
	struct Wardrobe final
	{
		std::wstring characterName;
		std::wstring directory;
		std::wstring characterFile;
		std::string costume;
		bool head; // 1 Head, 0 Body
	};

	//! Config data for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/wardrobe.json"; }
		//! A map containing the user friendly name of a head and it's actual name in the FL files
		std::map<std::string, std::string> heads = {{"ExampleHead", "ku_edo_head"}};
		//! A map containing the user friendly name of a body and it's actual name in the FL files
		std::map<std::string, std::string> bodies = {{"ExampleBody", "ku_edo_body"}};
	};

	//! Global data for this plugin
	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;
		std::unique_ptr<Config> config = nullptr;

		//! A vector containing the restarts (wardrobe changes) that are currently pending
		std::vector<Wardrobe> pendingRestarts;
	};
}