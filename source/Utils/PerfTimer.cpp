#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"

PerfTimer::PerfTimer(std::wstring_view func, uint warn) : function(func), warning(warn) {}

void PerfTimer::Start() { tmStart = TimeUtils::UnixTime<std::chrono::microseconds>(); }

ulong PerfTimer::Stop()
{
    auto timeDelta = static_cast<uint>(TimeUtils::UnixTime<std::chrono::microseconds>() - tmStart);

    if (FLHookConfig::i()->debug.logPerformanceTimers)
    {
        if (timeDelta > max && timeDelta > warning)
        {
            Logger::Log(LogLevel::Info, std::format(L"Spent {} ms in {}, longest so far.", timeDelta, function));
            max = timeDelta;
        }
        else if (timeDelta > 100)
        {
            Logger::Log(LogLevel::Info, std::format(L"Spent {} ms in {}", timeDelta, function));
        }
    }
    return timeDelta;
}
