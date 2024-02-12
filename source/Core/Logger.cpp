#include "PCH.hpp"

#include "Core/Commands/CommandLineParser.hpp"
#include "Defs/FLHookConfig.hpp"

enum class ConsoleColor
{
    Blue = 0x0001,
    Green = 0x0002,
    Cyan = Blue | Green,
    Red = 0x0004,
    Purple = Red | Blue,
    Yellow = Red | Green,
    White = Red | Blue | Green,

    StrongBlue = 0x0008 | Blue,
    StrongGreen = 0x0008 | Green,
    StrongCyan = 0x0008 | Cyan,
    StrongRed = 0x0008 | Red,
    StrongPurple = 0x0008 | Purple,
    StrongYellow = 0x0008 | Yellow,
    StrongWhite = 0x0008 | White,
};

BOOL WINAPI ConsoleHandler(DWORD ctrlType) { return ctrlType == CTRL_CLOSE_EVENT; }

std::wstring Logger::SetLogSource(void* addr)
{
    if (HMODULE dll; RtlPcToFileHeader(addr, (void**)&dll))
    {
        WCHAR maxPath[MAX_PATH];
        // If successful, prepend
        if (GetModuleFileNameW(dll, maxPath, MAX_PATH))
        {
            const std::wstring path = maxPath;
            return std::format(L"({} {}) ", TimeUtils::CurrentDate(), path.substr(path.find_last_of(L"\\") + 1));
        }
    }

    return L"";
}

using namespace std::chrono_literals;
void Logger::GetConsoleInput(std::stop_token st)
{
    while (!st.stop_requested())
    {
        DWORD bytesRead;
        std::wstring cmd;
        cmd.resize(1024);
        if (ReadConsole(consoleInput, cmd.data(), cmd.size(), &bytesRead, nullptr))
        {
            if (cmd[cmd.length() - 1] == '\n')
            {
                cmd = cmd.substr(0, cmd.length() - 1);
            }
            if (cmd[cmd.length() - 1] == '\r')
            {
                cmd = cmd.substr(0, cmd.length() - 1);
            }

            if (!cmd.empty())
            {
                commandQueue.push(cmd);
            }
        }

        std::this_thread::sleep_for(1s);
    }
}

void Logger::PrintToConsole(std::stop_token st)
{
    while (!st.stop_requested())
    {
        std::this_thread::sleep_for(0.25s);

        LogMessage logMessage;
        while (logQueue.try_pop(logMessage))
        {
            auto log = std::format(L"{}{}\n", SetLogSource(logMessage.retAddress), logMessage.message);
            switch (logMessage.level)
            {
                case LogLevel::Trace:
                    {
                        SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongWhite));
                        break;
                    }
                case LogLevel::Debug:
                    {

                        SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::Green));
                        break;
                    }
                case LogLevel::Info:
                    {
                        SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongCyan));
                        break;
                    }
                case LogLevel::Warn:
                    {
                        SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongYellow));
                        break;
                    }
                case LogLevel::Err:
                    {
                        SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongRed));
                        break;
                    }
            }

            if (consoleAllocated)
            {
                ulong _;
                WriteConsoleW(consoleOutput, log.data(), log.length(), &_, nullptr);
            }
            else
            {
                std::wcout << log << std::endl << std::flush;
            }

            // Reset
            SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::White));
        }
    }
}

void Logger::Init()
{
    if (const CommandLineParser cmd; cmd.CmdOptionExists(L"-noconsole"))
    {
        consoleAllocated = false;
    }
    else
    {
        AllocConsole();
        SetConsoleTitleW(L"FLHook");

        const HWND console = GetConsoleWindow();
        RECT r;
        GetWindowRect(console, &r);

        MoveWindow(console, r.left, r.top, 1366, 768, TRUE);
    }

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    consoleInput = GetStdHandle(STD_INPUT_HANDLE);
    consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    // change version number here:
    // https://patorjk.com/software/taag/#p=display&f=Doom&t=FLHook%204.1%20pallas
    std::wstring welcomeText = LR"(
______ _      _   _             _        ___   __                _ _           
|  ___| |    | | | |           | |      /   | /  |              | | |          
| |_  | |    | |_| | ___   ___ | | __  / /| | `| |   _ __   __ _| | | __ _ ___ 
|  _| | |    |  _  |/ _ \ / _ \| |/ / / /_| |  | |  | '_ \ / _` | | |/ _` / __|
| |   | |____| | | | (_) | (_) |   <  \___  |__| |_ | |_) | (_| | | | (_| \__ \
\_|   \_____/\_| |_/\___/ \___/|_|\_\     |_(_)___/ | .__/ \__,_|_|_|\__,_|___/
                                                    | |                        
                                                    |_|                        )";
    welcomeText += L"\n\n";
    DWORD _;
    WriteConsole(consoleOutput, welcomeText.c_str(), welcomeText.length(), &_, nullptr);

    commandThread = std::jthread(std::bind_front(&Logger::GetConsoleInput));
    loggingThread = std::jthread(std::bind_front(&Logger::PrintToConsole));
}

void Logger::Log(LogFile file, LogLevel level, std::wstring_view str)
{
    if ((!FLHook::GetConfig().debug.logTraceLevel && level == LogLevel::Trace) || (!FLHook::GetConfig().debug.debugMode && level == LogLevel::Debug))
    {
        return;
    }

    logQueue.push({ level, file, std::wstring(str), _ReturnAddress() });
}

void Logger::Log(LogLevel level, std::wstring_view str)
{
    if ((!FLHook::GetConfig().debug.logTraceLevel && level == LogLevel::Trace) || (!FLHook::GetConfig().debug.debugMode && level == LogLevel::Debug))
    {
        return;
    }

    logQueue.push({ level, LogFile::Default, std::wstring(str), _ReturnAddress() });
}

std::optional<std::wstring> Logger::GetCommand()
{
    if (commandQueue.empty())
    {
        return {};
    }

    std::wstring ret;
    if (const auto val = commandQueue.try_pop(ret); !val)
    {
        return {};
    }

    return StringUtils::Trim(ret);
}
