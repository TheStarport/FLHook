#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::CrashCatcher
{
	//! Global data for this plugin
	struct Global final
	{
		ReturnCode returncode = ReturnCode::Default;

		bool bPatchInstalled = false;
		FARPROC fpOldTimingSeconds = 0;

		HMODULE hModServerAC;
		HMODULE hEngBase;
		HMODULE hModContentAC;

		std::map<uint, mstime> mapSaveTimes;
	};

	#define LOG_EXCEPTION_INTERNAL()								\
	{																\
		AddLogInternal("ERROR Exception in %s", __FUNCTION__);		\
		AddExceptionInfoLog();										\
	};
} // namespace Plugins::CrashCatcher

IMPORT struct CObject* __cdecl GetRoot(struct CObject const*);