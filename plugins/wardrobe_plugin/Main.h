#pragma once
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Wardrobe
{

	struct Wardrobe final
	{
		std::wstring characterName;
		std::wstring directory;
		std::wstring characterFile;
		std::string costume;
		bool head; // 1 Head, 0 Body
	};

	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/wardrobe.json"; }
	};

	struct Global final
	{
		std::map<std::string, std::string> heads;
		std::map<std::string, std::string> bodies;
		ReturnCode returncode = ReturnCode::Default;
		std::list<Wardrobe> pendingRestarts;
	};
}