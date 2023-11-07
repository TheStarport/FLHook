#pragma once

class DLL PerfTimer
{
    public:
        PerfTimer(std::wstring_view function, uint warning);
        void Start();
        ulong Stop();

    private:
        mstime tmStart = 0;
        ulong max = 0;
        std::wstring_view function;
        ulong warning;
};
