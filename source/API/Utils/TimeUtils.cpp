#include "PCH.hpp"

using namespace std::chrono;

uint64 TimeUtils::UnixMilliseconds() { return static_cast<uint>(duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()); }

uint TimeUtils::UnixSeconds() { return static_cast<uint>(duration_cast<seconds>(system_clock::now().time_since_epoch()).count()); }

sys_time<seconds> TimeUtils::UnixToSysTime(int64 time) { return sys_time{ seconds{ time } }; }

std::wstring TimeUtils::AsTimePassed(seconds dur)
{
    using Days = duration<int, std::ratio<86400>>;
    const auto d = duration_cast<Days>(dur);
    dur -= d;
    const auto h = duration_cast<hours>(dur);
    dur -= h;
    const auto m = duration_cast<minutes>(dur);
    dur -= m;
    const auto s = duration_cast<seconds>(dur);

    const auto dc = d.count();
    const auto hc = h.count();
    const auto mc = m.count();
    const auto sc = s.count();

    std::wstringstream ss;
    ss.fill('0');
    if (dc)
    {
        ss << d.count() << "d ";
    }
    if (dc || hc)
    {
        if (dc)
        {
            ss << std::setw(2);
        } // pad if second set of numbers
        ss << h.count() << "h ";
    }
    if (dc || hc || mc)
    {
        if (dc || hc)
        {
            ss << std::setw(2);
        }
        ss << m.count() << "m ";
    }
    if (dc || hc || mc || sc)
    {
        if (dc || hc || mc)
        {
            ss << std::setw(2);
        }
        ss << s.count() << 's';
    }

    return ss.str();
}

std::wstring TimeUtils::AsDate(const seconds total)
{
    sys_time time{ total };
    return std::format(L"{0:%Y-%m-%d %H:%M:%S}", time);
}

std::wstring TimeUtils::CurrentDate() { return AsDate(seconds{ UnixSeconds() }); }
