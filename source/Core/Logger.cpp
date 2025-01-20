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
                commandQueue.enqueue(cmd);
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

        ::Log logMessage;
        while (logQueue.try_dequeue(logMessage))
        {
            if (!consoleAllocated)
            {
                std::cout << rfl::json::write(logMessage) << std::endl;
            }
            else
            {
                switch (logMessage.level)
                {
                    case LogLevel::Trace:
                        {
                            SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongWhite));
                            break;
                        }
                    case LogLevel::Debug:
                        {

                            SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongGreen));
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
                    case LogLevel::Error:
                        {
                            SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongRed));
                            break;
                        }
                }

                int i = 0;
                for (const auto& arg : logMessage.valueMap)
                {
                    i++;
                    logMessage.message =
                        StringUtils::ReplaceStr(logMessage.message, std::wstring_view(std::format(L"{{{}}}", i)), std::wstring_view(arg.second));
                }

                std::wcout << std::format(L"{} {}: {}", TimeUtils::CurrentDate(), magic_enum::enum_name(logMessage.level), logMessage.message) << std::endl;

                // Reset
                SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongWhite));
            }
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

        const auto console = GetConsoleWindow();
        RECT r;
        GetWindowRect(console, &r);

        MoveWindow(console, r.left, r.top, 1366, 768, TRUE);
        FILE* fDummy;
        freopen_s(&fDummy, "CONOUT$", "w", stdout);
        freopen_s(&fDummy, "CONOUT$", "w", stderr);
        freopen_s(&fDummy, "CONIN$", "r", stdin);
        std::cout.clear();
        std::clog.clear();
        std::cerr.clear();
        std::cin.clear();

        SetStdHandle(STD_OUTPUT_HANDLE, stdout);
        SetStdHandle(STD_ERROR_HANDLE, stderr);
        SetStdHandle(STD_INPUT_HANDLE, stdin);
        std::wcout.clear();
        std::wclog.clear();
        std::wcerr.clear();
        std::wcin.clear();
    }

    SetConsoleCtrlHandler(ConsoleHandler, TRUE);
    consoleInput = GetStdHandle(STD_INPUT_HANDLE);
    consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

    // change version number here:
    // https://patorjk.com/software/taag/#p=display&f=Doom&t=FLHook%205.0%20pallas
    std::wstring welcomeText = LR"(
______ _      _   _             _      _____  _____               _ _
|  ___| |    | | | |           | |    |  ___||  _  |             | | |
| |_  | |    | |_| | ___   ___ | | __ |___ \ | |/' |  _ __   __ _| | | __ _ ___
|  _| | |    |  _  |/ _ \ / _ \| |/ /     \ \|  /| | | '_ \ / _` | | |/ _` / __|
| |   | |____| | | | (_) | (_) |   <  /\__/ /\ |_/ / | |_) | (_| | | | (_| \__ \
\_|   \_____/\_| |_/\___/ \___/|_|\_\ \____(_)\___/  | .__/ \__,_|_|_|\__,_|___/
                                                     | |
                                                     |_|                        )";
    welcomeText += L"\n\n";
    std::wcout << welcomeText << std::flush;

    commandThread = std::jthread(std::bind_front(&Logger::GetConsoleInput));
    loggingThread = std::jthread(std::bind_front(&Logger::PrintToConsole));
}

void Logger::Log(const ::Log& log)
{
    if (static_cast<int>(FLHook::GetConfig()->logging.minLogLevel) <= 3)
    {
        logQueue.enqueue(log);
    }
}

std::optional<std::wstring> Logger::GetCommand()
{
    std::wstring cmd;
    if (!commandQueue.try_dequeue(cmd))
    {
        return {};
    }

    return StringUtils::Trim(cmd);
}
