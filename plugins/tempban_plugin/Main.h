#pragma once

#include <FLHook.h>
#include <plugin.h>
#include <plugin_comms.h>

struct TEMPBAN_INFO {
    std::wstring wscID;
    mstime banstart;
    mstime banduration;
};

#define IS_CMD(a) !wscCmd.compare(L##a)