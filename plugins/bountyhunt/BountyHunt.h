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
		std::string File() override { return "config/bountyhunt.json"; }

		// Reflectable fields
		bool enableBountyHunt = true;
		int levelProtect = 0;
		//! Minimal time a hunt can be set to, in minutes.
		uint minimalHuntTime = 1;
		//! Maximum time a hunt can be set to, in minutes.
		uint maximumHuntTime = 240;
		//! Hunt time in minutes, if not explicitly specified.
		uint defaultHuntTime = 30;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::vector<BountyHunt> bountyHunt;
	};
} // namespace Plugins::BountyHunt
