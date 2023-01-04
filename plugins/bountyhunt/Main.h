#pragma once

// Included
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::BountyHunt
{
	//! Structs
	struct BountyHunt
	{
		uint targetId;
		uint initiatorId;
		std::wstring target;
		std::wstring initiator;
		int cash;
		mstime end;
	};

	//! Configurable fields for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/bountyhunt.json"; }

		// Reflectable fields
		bool enableBountyHunt = true;
		int levelProtect = 0;
		uint minimalHuntTime = 1;
		uint maximumHuntTime = 240;
		uint defaultHuntTime = 30;
	};

	
	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::vector<BountyHunt> bountyHunt;
	};
}

