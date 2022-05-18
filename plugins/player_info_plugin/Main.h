#pragma once

#include <FLHook.h>
#include <algorithm>
#include <list>
#include <map>
#include <math.h>
#include <plugin.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

static int set_iPluginDebug = 0;
ReturnCode returncode;

void AddExceptionInfoLog(struct SEHException* pep);
#define LOG_EXCEPTION                                  \
	{                                                  \
		AddLog("ERROR Exception in %s", __FUNCTION__); \
		AddExceptionInfoLog(0);                        \
	}