#pragma once

#include <string>
#include "FLHook.h"
#include "plugin.h"

struct TEMPBAN_INFO
{
	std::wstring wscID;
	mstime banstart;
	mstime banduration;
};
