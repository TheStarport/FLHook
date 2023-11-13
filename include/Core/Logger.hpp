#pragma once

#include <Singleton.hpp>

#include <Core/Templates/Macros.hpp>
#include <concurrent_queue.h>
#include <optional>
#include <spdlog/spdlog.h>

enum class LogLevel
{
    Trace = spdlog::level::trace,
    Debug = spdlog::level::debug,
    Info = spdlog::level::info,
    Warn = spdlog::level::warn,
    Err = spdlog::level::err,
};

enum class LogFile
{
    // Log to FLHook.log & Console
    Default,
    // Only log to console
    ConsoleOnly,
};

class DLL Logger final
{
        bool consoleAllocated = true;
        HANDLE consoleInput;
        HANDLE consoleOutput;

        std::jthread consoleThread;
        concurrency::concurrent_queue<std::wstring> queue;

        // The DLL name + timestamp of where the log request came from
        std::wstring logPrefix;

        void SetLogSource(void* addr);

        void GetConsoleInput(std::stop_token st);
        void PrintToConsole(LogLevel level, std::wstring_view str) const;

    public:
        Logger();
        ~Logger();

        void Log(LogFile file, LogLevel level, std::wstring_view str);
        void Log(LogLevel level, std::wstring_view str);

        std::optional<std::wstring> GetCommand();
};
