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
		uint cash;
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
		std::wstring taxRequestReceived = L"You have received a tax request: Pay %d credits to %s! Type \"/pay\" to pay the tax.";
		std::wstring huntingMessage = L"You are being hunted by %s. Run for cover, they want to kill you!";
		std::wstring huntingMessageOriginator = L"Good luck hunting %s !";
		std::wstring cannotPay = L"This rogue isn't interested in money. Run for cover, they want to kill you!";
		bool killDisconnectingPlayers = true;
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

