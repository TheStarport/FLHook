#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Mark
{
	typedef void (*_TimerFunc)();

	//! Structs
	struct MARK_INFO
	{
		bool MarkEverything;
		bool IgnoreGroupMark;
		float AutoMarkRadius;
		std::vector<uint> MarkedObjects;
		std::vector<uint> DelayedSystemMarkedObjects;
		std::vector<uint> AutoMarkedObjects;
		std::vector<uint> DelayedAutoMarkedObjects;
	};

	struct DELAY_MARK
	{
		uint iObj;
		mstime time;
	};

	struct TIMER
	{
		_TimerFunc proc;
		mstime IntervalMS;
		mstime LastCall;
	};

	// Reflectable config
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/mark.json"; }

		float AutoMarkRadiusInM = 2000;
	};

	// Functions
	char MarkObject(uint iClientID, uint iObject);
	void UnMarkAllObjects(uint iClientID);
	char UnMarkObject(uint iClientID, uint iObject);

	void UserCmd_AutoMark(const uint& iClientID, const std::wstring_view& wscParam);
	void UserCmd_MarkObj(const uint& iClientID, const std::wstring_view& wscParam);
	void UserCmd_MarkObjGroup(const uint& iClientID, const std::wstring_view& wscParam);
	void UserCmd_SetIgnoreGroupMark(const uint& iClientID, const std::wstring_view& wscParam);
	void UserCmd_UnMarkObj(const uint& iClientID, const std::wstring_view& wscParam);
	void UserCmd_UnMarkObjGroup(const uint& iClientID, const std::wstring_view& wscParam);
	void UserCmd_UnMarkAllObj(const uint& iClientID, const std::wstring_view& wscParam);

	void TimerMarkDelay();
	void TimerSpaceObjMark();

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		// Other fields
		ReturnCode returncode = ReturnCode::Default;

		MARK_INFO Mark[250];
		std::list<DELAY_MARK> DelayedMarks;
		std::list<TIMER> Timers;
	};

	extern std::unique_ptr<Global> global;

	//! Macros
	#define PRINT_ERROR()                                                    \
	{                                                                        \
		for (uint i = 0; (i < sizeof(wscError) / sizeof(std::wstring)); i++) \
			PrintUserCmdText(iClientID, wscError[i]);                        \
		return;                                                              \
	}
} // namespace Plugins::Mark