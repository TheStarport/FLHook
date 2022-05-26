#pragma once

#include <FLHook.h>
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

	struct CLOAK_ARCH
	{
		std::string scNickName;
		int iWarmupTime;
		int iCooldownTime;
		int iHoldSizeLimit;
		std::map<uint, uint> mapFuelToUsage;
		bool bDropShieldsOnUncloak;
	};

	struct CLOAK_INFO
	{
		CLOAK_INFO()
		{
			uint iCloakSlot = 0;
			bCanCloak = false;
			mstime tmCloakTime = 0;
			uint iState = STATE_CLOAK_INVALID;
			uint bAdmin = false;

			arch.iWarmupTime = 0;
			arch.iCooldownTime = 0;
			arch.iHoldSizeLimit = 0;
			arch.mapFuelToUsage.clear();
			arch.bDropShieldsOnUncloak = false;
		}

		uint iCloakSlot;
		bool bCanCloak;
		mstime tmCloakTime;
		uint iState;
		bool bAdmin;

		CLOAK_ARCH arch;
	};

	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/cloak.json"; }

		struct CLOAK_ARCH_REFLECTABLE : Reflectable
		{
			int WarmupTime = 0;
			int CooldownTime = 0;
			int HoldSizeLimit = 0;
			std::map<std::string, int> FuelToUsage = {{"SomeItem", 123}};
			bool DropShieldsOnUncloak = false;
		};

		CLOAK_ARCH_REFLECTABLE example;

		// Reflectable fields
		std::map<std::string, CLOAK_ARCH_REFLECTABLE> mapCloakingDevices = {{"example", example}};
		bool DsAce = false;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		// Other fields
		ReturnCode returncode = ReturnCode::Default;

		std::map<uint, CLOAK_INFO> mapClientsCloak;
		std::map<uint, CLOAK_ARCH> mapCloakingDevices;
		std::wstring CloakingText = L" Cloaking";
		std::wstring UncloakingText = L" Uncloaking";
	};
}

