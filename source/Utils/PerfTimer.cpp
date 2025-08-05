#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Defs/FLHookConfig.hpp"

PerfTimer::PerfTimer(std::wstring_view func, int64 warn) : function(func), warning(warn) {}

void PerfTimer::Start() { tmStart = TimeUtils::UnixTime<std::chrono::microseconds>(); }

int64 PerfTimer::Stop()
{
    auto timeDelta = TimeUtils::UnixTime<std::chrono::microseconds>() - tmStart;

    if (FLHook::GetConfig()->logging.logPerformanceTimers)
    {
        if (timeDelta > max && timeDelta > warning)
        {
            DEBUG("Spent {{timeDelta}} μs in {{function}}, longest so far. ", { "timeDelta", timeDelta }, { "function", function });

            max = timeDelta;
        }
        else if (timeDelta > 100)
        {
            DEBUG("Spent {{timeDelta}} μs in {{function}}", { "timeDelta", timeDelta }, { "function", function });
        }
    }
    return timeDelta;
}
