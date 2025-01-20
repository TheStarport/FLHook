#include "PCH.hpp"

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
            INFO(L"Spent {0} μs in this function, longest so far. ", {L"timeDelta", std::to_wstring(timeDelta)})

            max = timeDelta;
        }
        else if (timeDelta > 100)
        {
            INFO(L"Spent {0} μs in this function", {L"timeDelta", std::to_wstring(timeDelta)})
        }
    }
    return timeDelta;
}
