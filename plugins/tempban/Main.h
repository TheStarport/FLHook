#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Tempban
{
	//! Struct that contains an account Id, when the ban started and how long its for
	struct TempbanInfo
	{
		std::wstring accountId;
		mstime banStart;
		mstime banDuration;
	};

	//! Communicator class for this plugin. This is used by other plugins
	class TempBanCommunicator : public PluginCommunicator
	{
	  public:
		inline static const char* pluginName = "TempBan Plugin by w0dk4";
		explicit TempBanCommunicator(std::string plug);

		HK_ERROR PluginCall(TempBan, const std::wstring& characterName, uint duration);
	};

	//! Global data for this plugin
	struct Global final
	{
		std::list<TempbanInfo> TempBans;

		ReturnCode returncode = ReturnCode::Default;
		TempBanCommunicator* communicator = nullptr;
	};
}
