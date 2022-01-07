#pragma once

#include <FLHook.h>
#include <plugin.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

static int set_iPluginDebug = 0;
ReturnCode returncode;

typedef void (*_UserCmdProc)(uint, const std::wstring &);

struct USERCMD {
    wchar_t *wszCmd;
    _UserCmdProc proc;
};

#define IS_CMD(a) !wscCmd.compare(L##a)

std::list<INISECTIONVALUE> lstRanks;
