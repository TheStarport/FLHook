#pragma once

#include "FLHook.h"
#include "plugin.h"
#include <string>

struct TEMPBAN_INFO {
  std::wstring wscID;
  mstime banstart;
  mstime banduration;
};
