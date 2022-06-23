#pragma once

#include <FLHook.hpp>
#include <plugin.h>
#include "../mail_plugin/Main.h"
#include "../tempban_plugin/Main.h"

namespace Plugins::Message
{
	/** the data for a single online player */
	class INFO
	{
	  public:
		INFO() : LastPmClientID(-1), TargetClientID(-1), ShowChatTime(false), GreetingShown(false), SwearWordWarnings(0) {}

		static const int NUMBER_OF_SLOTS = 10;
		std::wstring slot[NUMBER_OF_SLOTS];

		// Client ID of last PM.
		uint LastPmClientID;

		// Client ID of selected target
		uint TargetClientID;

		// Current chat time settings
		bool ShowChatTime;

		// True if the login banner has been displayed.
		bool GreetingShown;

		// Swear word warn level
		int SwearWordWarnings;
	};

	struct Config : Reflectable
	{
		std::string File() override { return "flhook_plugins/message.json"; }

		/** help text for when user types /help */
		std::map<std::string, std::string> HelpLines;

		/** greetings text for when user logins in */
		std::vector<std::wstring> GreetingBannerLines;

		/** special banner text, on timer */
		std::list<std::wstring> SpecialBannerLines;

		/** standard banner text, on timer */
		std::vector<std::vector<std::wstring>> StandardBannerLines;

		/** Time in second to repeat display of special banner */
		int SpecialBannerTimeout = 5;

		/** Time in second to repeat display of standard banner */
		int StandardBannerTimeout = 60;

		/** true if we override flhook built in help */
		bool CustomHelp = false;

		/** true if we don't echo mistyped user and admin commands to other players. */
		bool SuppressMistypedCommands = true;

		/** if true support the /showmsg and /setmsg commands. This needs a client hook*/
		bool EnableSetMessage = false;

		// Enable /me and /do commands
		bool EnableMe = true;
		bool EnableDo = true;

		std::wstring DisconnectSwearingInSpaceMsg = L"%player has been kicked for swearing";

		float DisconnectSwearingInSpaceRange = 5000.0f;

		/** list of swear words */
		std::vector<std::wstring> SwearWords;
	};

	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;

		/** cache of preset messages for the online players (by client ID) */
		std::map<uint, INFO> Info;

		std::unique_ptr<Config> config = nullptr;

		/** This parameter is sent when we send a chat time line so that we don't print
		a time chat line recursively. */
		bool SendingTime = false;

		Plugins::Mail::MailCommunicator* mailCommunicator = nullptr;
		Plugins::Tempban::TempBanCommunicator* tempBanCommunicator = nullptr;
	};

	/** A random macro to make things easier */
	#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

	void UserCmd_ReplyToLastPMSender(const uint& iClientID, const std::wstring_view& wscParam);
	void UserCmd_SendToLastTarget(const uint& iClientID, const std::wstring_view& wscParam);
}




