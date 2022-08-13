#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Cloak
{

	enum INFO_STATE
	{
		STATE_CLOAK_INVALID = 0,
		STATE_CLOAK_OFF = 1,
		STATE_CLOAK_CHARGING = 2,
		STATE_CLOAK_ON = 3,
	};

	struct CloakArch
	{
		std::string scNickName;
		int warmupTime;
		int cooldownTime;
		int holdSizeLimit;
		std::map<uint, uint> fuelToUsage;
		bool dropShieldsOnUncloak;
	};

	struct CloakInfo
	{
		CloakInfo()
		{
			arch.warmupTime = 0;
			arch.cooldownTime = 0;
			arch.holdSizeLimit = 0;
			arch.fuelToUsage.clear();
			arch.dropShieldsOnUncloak = false;
		}

		uint cloakSlot = 0;
		bool canCloak = false;
		mstime cloakTime = 0;
		uint state = STATE_CLOAK_INVALID;
		bool admin = false;
		CloakArch arch;
	};

	//! Config data for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/cloak.json"; }

		//! Struct to hold each cloaking device
		struct CloakArch : Reflectable
		{
			//! Warm up time of the device
			int warmupTime = 0;
			//! Cooldown time of the device
			int cooldownTime = 0;
			//! Cannot fit on ships with a larger hold size than this value
			int holdSizeLimit = 0;
			//! Fuel the cloak uses and how much
			std::map<std::string, int> fuelToUsage = {{"commodity_prisoners", 1}};
			//! Whether the shields should drop when the cloak is deactivated
			bool dropShieldsOnUncloak = false;
		};

		CloakArch example;

		// Reflectable fields
		std::map<std::string, CloakArch> cloakingDevices = {{"example", example}};
		bool dsAce = false;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		// Other fields
		ReturnCode returncode = ReturnCode::Default;

		std::map<uint, CloakInfo> clientCloakingInfo;
		std::map<uint, CloakArch> cloakingDevices;
		std::wstring cloakingText = L" Cloaking";
		std::wstring uncloakingText = L" Uncloaking";
	};
}

