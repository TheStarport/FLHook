#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Template
{
	// Loadable json configuration
	struct Config : Reflectable
	{
		std::string File() override { return "flhook_plugins/template.json"; }
		bool overrideUserNumber = false;
	};

	struct Global
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
	};
}