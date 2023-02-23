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
		std::string File() override { return "config/tax.json"; }

		// Reflectable fields
		//! Systems where demands cannot be made.
		std::vector<std::string> excludedSystems = {};
		//! Minimal playtime in seconds required for a character to be a valid demand target.
		int minplaytimeSec = 0;
		//! Maximum amount of credits a player can demand from another.
		uint maxTax = 300;
		//! Color of messages that will be broadcasted by this plugin.
		MessageColor customColor = MessageColor::LightGreen;
		//! Formatting of the messages broadcasted by this plugin.
		MessageFormat customFormat = MessageFormat::Small;
		//! Message letting the target know about the size of the demand, as well as informing them on how to comply.
		std::wstring taxRequestReceived = L"You have received a tax request: Pay {} credits to {}! Type \"/pay\" to pay the tax.";
		//! Message letting the target know they're being attacked.
		std::wstring huntingMessage = L"You are being hunted by {}. Run for cover, they want to kill you!";
		//! Confirmation message to the aggressor that the victim has been informed.
		std::wstring huntingMessageOriginator = L"Good luck hunting {} !";
		//! Message received if payment attempt is made on
		std::wstring cannotPay = L"This rogue isn't interested in money. Run for cover, they want to kill you!";
		//! If true, kills the players who disconnect while having a demand levied against them.
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
} // namespace Plugins::Tax
