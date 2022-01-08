#pragma once

#include <FLHook.h>
#include <plugin.h>

static int set_iPluginDebug = 0;
ReturnCode returncode;

/** The messaging plugin message log for offline players */
static std::string MSG_LOG = "-mail.ini";