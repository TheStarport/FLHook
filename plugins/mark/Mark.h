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
		uint objId;
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
		std::string File() override { return "config/mark.json"; }

		//! Radius within which all targets get marked, if autoMark is enabled.
		float AutoMarkRadiusInM = 2000;
	};

	// Functions
	char MarkObject(ClientId client, uint object);
	void UnMarkAllObjects(ClientId client);
	char UnMarkObject(ClientId client, uint object);

	void UserCmd_AutoMark(ClientId& client, const std::wstring& param);
	void UserCmd_MarkObj(ClientId& client, const std::wstring& param);
	void UserCmd_MarkObjGroup(ClientId& client, const std::wstring& param);
	void UserCmd_SetIgnoreGroupMark(ClientId& client, const std::wstring& param);
	void UserCmd_UnMarkObj(ClientId& client, const std::wstring& param);
	void UserCmd_UnMarkObjGroup(ClientId& client, const std::wstring& param);
	void UserCmd_UnMarkAllObj(ClientId& client, const std::wstring& param);

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
#define PRINT_ERROR()                                                        \
	{                                                                        \
		for (uint i = 0; (i < sizeof(Error) / sizeof(std::wstring)); i++) \
			client.Message(Error[i]);                           \
		return;                                                              \
	}
} // namespace Plugins::Mark