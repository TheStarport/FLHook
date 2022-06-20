#pragma once

#include <FLHook.hpp>
#include <plugin.h>
#include <plugin.h>

namespace Plugins::Mail
{
	class MailCommunicator final : public PluginCommunicator
	{
	  public:
		inline static const char* pluginName = "Mail";
		explicit MailCommunicator(const std::string& plugin);
		void PluginCall(SendMail, std::wstring character, std::wstring msg);

		enum class MailEvent
		{
			MailSent = 0
		};

		struct MailSent
		{
			std::wstring character;
			std::wstring msg;
		};
	};

	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;

		/** The messaging plugin message log for offline players */
		std::string MSG_LOG = "-mail.ini";
		static const int MAX_MAIL_MSGS = 40;

		MailCommunicator* communicator;
	};
}