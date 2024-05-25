#pragma once

class DLL PerfTimer
{
    public:
        PerfTimer(std::wstring_view function, int64 warning);
        void Start();
        int64 Stop();

    private:
        int64 tmStart = 0;
        int64 max = 0;
        std::wstring function;
        int64 warning;
};
