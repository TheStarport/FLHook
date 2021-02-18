#pragma once

#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <list>
#include <FLHook.h>
#include <plugin.h>

static int set_iPluginDebug = 0;
PLUGIN_RETURNCODE returncode;

typedef bool (*_UserCmdProc)(uint, const std::wstring &, const std::wstring &,
                             const wchar_t *);

struct USERCMD {
    wchar_t *wszCmd;
    _UserCmdProc proc;
    wchar_t *usage;
};

#define IS_CMD(a) !wscCmd.compare(L##a)
