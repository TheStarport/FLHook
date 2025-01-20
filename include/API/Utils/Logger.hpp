#pragma once

#include <concurrentqueue.h>
#include <optional>

enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warn,
    Error,
};

#define WARN(message, ...) Logger::Log({__FUNCSIG__, LogLevel::Warn, { __VA_ARGS__ }, message, TimeUtils::UnixTime<std::chrono::seconds>()});
#define ERROR(message, ...) Logger::Log({__FUNCSIG__, LogLevel::Error, { __VA_ARGS__ }, message, TimeUtils::UnixTime<std::chrono::seconds>()});
#define DEBUG(message, ...) Logger::Log({__FUNCSIG__, LogLevel::Debug, { __VA_ARGS__ }, message, TimeUtils::UnixTime<std::chrono::seconds>()});
#define INFO(message, ...) Logger::Log({__FUNCSIG__, LogLevel::Info, { __VA_ARGS__ }, message, TimeUtils::UnixTime<std::chrono::seconds>()});
#define TRACE(message, ...) Logger::Log({__FUNCSIG__, LogLevel::Trace, { __VA_ARGS__ }, message, TimeUtils::UnixTime<std::chrono::seconds>()});

#define UNWRAP_VECTOR(v) std::accumulate(v.begin(), v.end(), std::wstring{}, [](const std::wstring& a, auto b) { return std::format(L"{}, {}", a, b); });
#define UNWRAP_MAP_KEYS(v) std::accumulate((v  | std::views::values).begin(), (v | std::views::values).end(), std::wstring{}, [](const std::wstring& a, auto b) { return std::format(L"{}, {}", a, b); });
#define UNWRAP_MAP_VALUES(v) std::accumulate((v  | std::views::values).begin(), (v | std::views::values).end(), std::wstring{}, [](const std::wstring& a, auto b) { return std::format(L"{}, {}", a, b); });


struct Log
{
        std::string function;
        LogLevel level;
        std::unordered_map<std::wstring, std::wstring> valueMap;
        std::wstring message;
        __int64 logTime;
};

class FLHook;
class DLL Logger final
{
        friend FLHook;
        inline static bool consoleAllocated = true;
        inline static HANDLE consoleInput;
        inline static HANDLE consoleOutput;

        inline static std::jthread commandThread;
        inline static std::jthread loggingThread;
        inline static moodycamel::ConcurrentQueue<Log> logQueue;       // NOLINT
        inline static moodycamel::ConcurrentQueue<std::wstring> commandQueue; // NOLINT

        // Thread Funcs

        static void GetConsoleInput(std::stop_token st);
        static void PrintToConsole(std::stop_token st);

        static void Init();

    public:
        static void Log(const Log& log);

        static std::optional<std::wstring> GetCommand();
};
