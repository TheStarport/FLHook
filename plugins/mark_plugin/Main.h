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
	char HkMarkObject(uint iClientID, uint iObject);
	void HkUnMarkAllObjects(uint iClientID);
	char HkUnMarkObject(uint iClientID, uint iObject);

	void UserCmd_AutoMark(uint iClientID, const std::wstring& wscParam);
	void UserCmd_MarkObj(uint iClientID, const std::wstring& wscParam);
	void UserCmd_MarkObjGroup(uint iClientID, const std::wstring& wscParam);
	void UserCmd_SetIgnoreGroupMark(uint iClientID, const std::wstring& wscParam);
	void UserCmd_UnMarkObj(uint iClientID, const std::wstring& wscParam);
	void UserCmd_UnMarkObjGroup(uint iClientID, const std::wstring& wscParam);
	void UserCmd_UnMarkAllObj(uint iClientID, const std::wstring& wscParam);

	void HkTimerMarkDelay();
	void HkTimerSpaceObjMark();

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