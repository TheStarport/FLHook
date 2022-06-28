#pragma once

// Included
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Tax
{
	//! Structs
	struct Tax
	{
		uint targetId;
		uint initiatorId;
		std::wstring target;
		std::wstring initiator;
		int cash;
		bool f1;
	};

	//! Configurable fields for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/tax.json"; }

		// Reflectable fields	
		std::vector<std::string> excludedSystems = {};
		int minplaytimeSec = 0;
		int maxTax = 300;
		MessageColor customColor = MessageColor::LightGreen;
		MessageFormat customFormat = MessageFormat::Small;
	};
	
	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::list<Tax> lsttax;
		std::vector<uint> excludedsystemsIds;
	};
}

