#pragma once

#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <FLHook.h>
#include <plugin.h>
#include "PluginUtilities.h"

static int set_iPluginDebug = 0;
PLUGIN_RETURNCODE returncode;

typedef void (*_UserCmdProc)(uint, const std::wstring&);

struct USERCMD
{
	wchar_t* wszCmd;
	_UserCmdProc proc;
};

#define IS_CMD(a) !wscCmd.compare(L##a)

void AddExceptionInfoLog(struct SEHException* pep);
#define LOG_EXCEPTION { AddLog("ERROR Exception in %s", __FUNCTION__); AddExceptionInfoLog(0); }

std::list<INISECTIONVALUE> lstRanks;