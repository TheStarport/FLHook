#pragma once

#include <FLHook.h>
#include <plugin.h>
#include "Shlwapi.h"

ReturnCode returncode = ReturnCode::Default;

struct RESTART {
    std::wstring wscCharname;
    std::string scRestartFile;
    std::wstring wscDir;
    std::wstring wscCharfile;
    int iCash;
};
std::list<RESTART> pendingRestarts;

bool set_bEnableRestartCost;