#pragma once

#include <FLHook.h>
#include <iostream>
#include <plugin.h>
#include <sstream>

static int set_iPluginDebug = 0;

typedef bool (*_UserCmdProc)(uint, const std::wstring &, const std::wstring &,
                             const wchar_t *);

struct USERCMD {
    wchar_t *wszCmd;
    _UserCmdProc proc;
    wchar_t *usage;
};

#define IS_CMD(a) !wscCmd.compare(L##a)
