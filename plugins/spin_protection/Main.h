#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::SpinProtection
{
	//! Config data for this plugin
	struct Config : Reflectable
	{
		//! The minimum amount of mass a ship must have for spin protection to kick in
		float spinProtectionMass = 180.0f;

		//! The higher this value is the more aggressive the spin protection is
		float spinImpulseMultiplier = -1.0f;

		//! File location of the json config file
		std::string File() override { return "config/spin_protection.json"; }
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
	};
} // namespace Plugins::SpinProtection
