#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::CrashCatcher
{
	//! Global data for this plugin
	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;

		bool patchInstalled = false;
		FARPROC oldTimingSeconds = nullptr;

		HMODULE hModServerAC;
		HMODULE hEngBase;
		HMODULE contentAC;

		std::map<uint, mstime> mapSaveTimes;
	};

	#define LOG_EXCEPTION_INTERNAL()								\
	{																\
		AddLogInternal("ERROR Exception in %s", __FUNCTION__);		\
		AddExceptionInfoLog();										\
	};
} // namespace Plugins::CrashCatcher

IMPORT CObject* __cdecl GetRoot(CObject const*);