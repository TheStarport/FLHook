#pragma once

#include <concurrentqueue.h>
#include <optional>

enum class LogLevel
{
    Trace,
    Debug,
    Info,
    Warn,
    Err,
};

class FLHook;
class DLL Logger final
{
        struct LogMessage
        {
                LogLevel level;
                std::wstring message;
                void* retAddress;
        };

        friend FLHook;
        inline static bool consoleAllocated = true;
        inline static HANDLE consoleInput;
        inline static HANDLE consoleOutput;

        inline static std::jthread commandThread;
        inline static std::jthread loggingThread;
        inline static moodycamel::ConcurrentQueue<LogMessage> logQueue;       // NOLINT
        inline static moodycamel::ConcurrentQueue<std::wstring> commandQueue; // NOLINT

        static std::wstring SetLogSource(void* addr);

        // Thread Funcs

        static void GetConsoleInput(std::stop_token st);
        static void PrintToConsole(std::stop_token st);

        static void Init();

    public:
        static void Trace(std::wstring_view str);
        static void Debug(std::wstring_view str);
        static void Info(std::wstring_view str);
        static void Warn(std::wstring_view str);
        static void Err(std::wstring_view str);

        static std::optional<std::wstring> GetCommand();
};
