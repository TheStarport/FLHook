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

void Logger::SetLogSource(void* addr)
{
    if (HMODULE dll; RtlPcToFileHeader(addr, (void**)&dll))
    {
        WCHAR maxPath[MAX_PATH];
        // If successful, prepend
        if (GetModuleFileNameW(dll, maxPath, MAX_PATH))
        {
            const std::wstring path = maxPath;
            logPrefix = std::format(L"({} {}) ", TimeUtils::CurrentDate(), path.substr(path.find_last_of(L"\\") + 1));
        }
    }
}

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
                queue.push(cmd);
            }
        }
    }
}

void Logger::PrintToConsole(LogLevel level, std::wstring_view str) const
{
    switch (level)
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
        WriteConsoleW(consoleOutput, str.data(), str.length(), &_, nullptr);
    }
    else
    {
        std::wcout << str << std::endl << std::flush;
    }

    // Reset
    SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::White));
}

Logger::Logger()
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
    // https://patorjk.com/software/taag/#p=display&f=Big&t=FLHook%204.1
    std::wstring welcomeText = LR"(
  __ _      _    _             _      _  _  __                 _ _           
 |  __| |    | |  | |           | |    | || |/_ |               | | |          
 | |__  | |    | |__| | _   _ | | __ | || |_| |    _ __   __ _| | | __ _ _ 
 |  __| | |    |  __  |/ _ \ / _ \| |/ / |__   _| |   | '_ \ / _` | | |/ _` / __|
 | |    | |__| |  | | (_) | (_) |   <     | |_| |   | |_) | (_| | | | (_| \__ \
 |_|    |__|_|  |_|\_/ \_/|_|\_\    |_(_)_|   | .__/ \__,_|_|_|\__,_|_/
                                                      | |                        
                                                      |_|                        
                                                                       )";
    welcomeText += L"\n\n";
    DWORD _;
    WriteConsole(consoleOutput, welcomeText.c_str(), welcomeText.length(), &_, nullptr);

    consoleThread = std::jthread(std::bind_front(&Logger::GetConsoleInput, this));
}

Logger::~Logger()
{
    SetConsoleCtrlHandler(ConsoleHandler, FALSE);
    FreeConsole();
}

void Logger::Log(LogFile file, LogLevel level, std::wstring_view str)
{
    if ((!FLHookConfig::i()->debug.logTraceLevel && level == LogLevel::Trace) || (!FLHookConfig::i()->debug.debugMode && level == LogLevel::Debug))
    {
        return;
    }

    if (logPrefix.empty())
    {
        SetLogSource(_ReturnAddress());
    }

    const std::wstring log = std::format(L"{}{}\n", logPrefix, str);
    logPrefix.clear();

    PrintToConsole(level, log);

    // TODO: Finish setting up log files
    if (file == LogFile::ConsoleOnly)
    {
        return;
    }
}

void Logger::Log(LogLevel level, std::wstring_view str)
{
    SetLogSource(_ReturnAddress());
    Log(LogFile::Default, level, str);
}

std::optional<std::wstring> Logger::GetCommand()
{
    if (queue.empty())
    {
        return {};
    }

    std::wstring ret;
    if (const auto val = queue.try_pop(ret); !val)
    {
        return {};
    }

    return StringUtils::Trim(ret);
}
