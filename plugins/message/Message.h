#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Message
{
	//! Number of auto message slots
	constexpr int NumberOfSlots = 10;

	//! A struct to represent each client
	class ClientInfo
	{
	  public:
		ClientInfo() = default;

		//! Slots that contain prewritten messages
		std::array<std::wstring, NumberOfSlots> slots;

		//! Client Id of last PM.
		uint lastPmClientId = -1;

		//! Client Id of selected target
		uint targetClientId = -1;

		//! Current chat time settings
		bool showChatTime = false;

		//! True if the login banner has been displayed.
		bool greetingShown = true;

		//! Swear word warn level
		int swearWordWarnings = 0;
	};

	//! Config data for this plugin
	struct Config : Reflectable
	{
		//! The json file we load out of
		std::string File() override { return "config/message.json"; }

		//! Greetings text for when user logins in
		std::vector<std::wstring> greetingBannerLines;

		//! special banner text, on timer
		std::vector<std::wstring> specialBannerLines;

		//! standard banner text, on timer
		std::vector<std::wstring> standardBannerLines;

		//! Time in second to repeat display of special banner
		int specialBannerTimeout = 5;

		//! Time in second to repeat display of standard banner
		int standardBannerTimeout = 60;

		//! true if we override flhook built in help
		bool customHelp = false;

		//! true if we don't echo mistyped user and admin commands to other players.
		bool suppressMistypedCommands = true;

		//! if true support the /showmsg and /setmsg commands. This needs a client hook
		bool enableSetMessage = false;

		//! Enable /me command
		bool enableMe = true;

		//! Enable /do command
		bool enableDo = true;

		//! String that stores the disconnect message for swearing in space
		std::wstring disconnectSwearingInSpaceMsg = L"%player has been kicked for swearing";

		//! Amount of time in minutes for which to tempban a player in case of swearing
		uint swearingTempBanDuration = 10;

		//! What radius around the player the message should be broadcasted to
		float disconnectSwearingInSpaceRange = 5000.0f;

		//! Vector of swear words
		std::vector<std::wstring> swearWords;
	};

	//! Global data for this plugin
	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;
		std::unique_ptr<Config> config = nullptr;

		//! Cache of preset messages for the online players (by client Id)
		std::map<uint, ClientInfo> info;

		//! This parameter is sent when we send a chat time line so that we don't print a time chat line recursively.
		bool sendingTime = false;
	};

//! A random macro to make things easier
#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

	void UserCmd_ReplyToLastPMSender(ClientId& client, const std::wstring& wscParam);
	void UserCmd_SendToLastTarget(ClientId& client, const std::wstring& wscParam);
} // namespace Plugins::Message
