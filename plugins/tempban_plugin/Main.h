#pragma once

#include <FLHook.h>
#include <plugin.h>
#include <utility>

struct TEMPBAN_INFO
{
	std::wstring wscID;
	mstime banstart;
	mstime banduration;
};

class TempBanCommunicator : public PluginCommunicator
{
  public:
	inline static const char* pluginName = "TempBan Plugin by w0dk4";
	explicit TempBanCommunicator(std::string plug);

	HK_ERROR PluginCall(TempBan, const std::wstring& characterName, uint duration);
};