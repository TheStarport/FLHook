#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::SpinProtection
{
	struct Config : Reflectable
	{
		float spinProtectionMass = 180.0f;
		float spinImpulseMultiplier = -1.0f;
		std::string File() override { return "flhook_plugins/spin_protection.json"; }
	};

	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
	};
}
