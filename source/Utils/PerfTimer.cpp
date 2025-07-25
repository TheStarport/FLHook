﻿#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"

PerfTimer::PerfTimer(std::wstring_view func, int64 warn) : function(func), warning(warn) {}

void PerfTimer::Start() { tmStart = TimeUtils::UnixTime<std::chrono::microseconds>(); }

int64 PerfTimer::Stop()
{
    auto timeDelta = TimeUtils::UnixTime<std::chrono::microseconds>() - tmStart;

    if (FLHook::GetConfig()->logging.logPerformanceTimers)
    {
        if (timeDelta > max && timeDelta > warning)
        {
            INFO("Spent {{timeDelta}} μs in this function, longest so far. ", { "timeDelta", timeDelta });

            max = timeDelta;
        }
        else if (timeDelta > 100)
        {
            INFO("Spent {{timeDelta}} μs in this function", { "timeDelta", timeDelta });
        }
    }
    return timeDelta;
}
