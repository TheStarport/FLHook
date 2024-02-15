#pragma once

#include <Singleton.hpp>

#include <concurrentqueue/concurrentqueue.h>
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

class FLHook;
class DLL Logger final
{
        struct LogMessage
        {
                LogLevel level;
                LogFile file;
                std::wstring message;
                void* retAddress;
        };

        friend FLHook;
        inline static bool consoleAllocated = true;
        inline static HANDLE consoleInput;
        inline static HANDLE consoleOutput;

        inline static std::jthread commandThread;
        inline static std::jthread loggingThread;
        inline static moodycamel::ConcurrentQueue<LogMessage> logQueue; // NOLINT
        inline static moodycamel::ConcurrentQueue<std::wstring> commandQueue; // NOLINT

        static std::wstring SetLogSource(void* addr);

        // Thread Funcs

        static void GetConsoleInput(std::stop_token st);
        static void PrintToConsole(std::stop_token st);

        static void Init();

    public:
        static void Log(LogFile file, LogLevel level, std::wstring_view str);
        static void Log(LogLevel level, std::wstring_view str);

        static std::optional<std::wstring> GetCommand();
};
