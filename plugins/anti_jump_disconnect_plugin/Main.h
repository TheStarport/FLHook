#pragma once

#include "../tempban_plugin/Main.h"

#include <FLHook.h>
#include <plugin.h>

ReturnCode returncode = ReturnCode::Default;

TempBanCommunicator* tempBanCommunicator = nullptr;

struct INFO
{
	bool bInWrapGate;
};
static std::map<uint, INFO> mapInfo;