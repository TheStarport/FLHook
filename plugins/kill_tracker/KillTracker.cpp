

#include "PCH.hpp"

#include "KillTracker.hpp"

namespace Plugins
{

}

DefaultDllMain();

const PluginInfo Info(L"KillTracker", L"killtracker", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(KillTrackerPlugin, Info);
