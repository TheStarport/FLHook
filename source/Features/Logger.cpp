#include "PCH.hpp"
#include <functional>
#include <iostream>
#include <Features/Logger.hpp>
#include <Windows.h>
#include <string>

#include "FLHook.hpp"
#include "Defs/FLHookConfig.hpp"
#include "Tools/CommandLineParser.hpp"


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

BOOL WINAPI ConsoleHandler(DWORD ctrlType)
{
	return ctrlType == CTRL_CLOSE_EVENT;
}

void Logger::SetLogSource(void* addr)
{
	if (HMODULE dll; RtlPcToFileHeader(addr, (void**)&dll))
	{
		CHAR maxPath[MAX_PATH];
		// If successful, prepend
		if (GetModuleFileName(dll, maxPath, MAX_PATH))
		{
			const std::string path = maxPath;
			logPrefix = std::string("(") + wstos(GetTimeString(FLHookConfig::i()->general.localTime)) + path.substr(path.find_last_of("\\") + 1) + ") ";
		}
	}
}

void Logger::GetConsoleInput(std::stop_token st)
{
	while (!st.stop_requested())
	{
		DWORD bytesRead;
		std::string cmd;
		cmd.resize(1024);
		if (ReadConsole(consoleInput, cmd.data(), cmd.size(), &bytesRead, nullptr))
		{
			if (cmd[cmd.length() - 1] == '\n')
				cmd = cmd.substr(0, cmd.length() - 1);
			if (cmd[cmd.length() - 1] == '\r')
				cmd = cmd.substr(0, cmd.length() - 1);

			if (!cmd.empty())
			{
				queue.push(cmd);
			}
		}
	}
}

void Logger::PrintToConsole(LogLevel level, const std::string& str) const
{
	switch (level)
	{
		case LogLevel::Trace: {
			SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::Blue));
			break;
		}
		case LogLevel::Debug: {
			SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::Green));
			break;
		}
		case LogLevel::Info: {
			SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongCyan));
			break;
		}
		case LogLevel::Warn: {
			SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongYellow));
			break;
		}
		case LogLevel::Err: {
			SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::StrongRed));
			break;
		}
	}

	if (consoleAllocated)
	{
		ulong _;
		WriteConsole(consoleOutput, str.c_str(), str.length(), &_, nullptr);
	}
	else
	{
		std::cout << str << std::endl << std::flush;	
	}

	// Reset
	SetConsoleTextAttribute(consoleOutput, static_cast<WORD>(ConsoleColor::White));
}

Logger::Logger()
{
	if (const CommandLineParser cmd; cmd.CmdOptionExists("-noconsole"))
	{
		consoleAllocated = false;
	}
	else
	{
		AllocConsole();
		SetConsoleTitle("FLHook");

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
	std::string welcomeText = R"(
  ______ _      _    _             _      _  _  __                 _ _           
 |  ____| |    | |  | |           | |    | || |/_ |               | | |          
 | |__  | |    | |__| | ___   ___ | | __ | || |_| |    _ __   __ _| | | __ _ ___ 
 |  __| | |    |  __  |/ _ \ / _ \| |/ / |__   _| |   | '_ \ / _` | | |/ _` / __|
 | |    | |____| |  | | (_) | (_) |   <     | |_| |   | |_) | (_| | | | (_| \__ \
 |_|    |______|_|  |_|\___/ \___/|_|\_\    |_(_)_|   | .__/ \__,_|_|_|\__,_|___/
                                                      | |                        
                                                      |_|                        
                                                                       )";
	welcomeText += "\n\n";
	DWORD _;
	WriteConsole(consoleOutput, welcomeText.c_str(), welcomeText.length(), &_, nullptr);

	consoleThread = std::jthread(std::bind_front(&Logger::GetConsoleInput, this));
}

Logger::~Logger()
{
	SetConsoleCtrlHandler(ConsoleHandler, FALSE);
	FreeConsole();
}

void Logger::Log(LogFile file, LogLevel level, const std::string& str)
{
	if (logPrefix.empty())
	{
		SetLogSource(_ReturnAddress());
	}

	const std::string log = logPrefix + str + "\n";
	logPrefix.clear();

	PrintToConsole(level, log);

	// TODO: Finish setting up log files
	if (file == LogFile::ConsoleOnly)
	{
		return;
	}
}

void Logger::Log(LogLevel level, const std::string& str)
{
	SetLogSource(_ReturnAddress());
	Log(LogFile::Default, level, str);
}

std::optional<std::string> Logger::GetCommand()
{
	if (queue.empty())
		return {};

	std::string ret;
	if (const auto val = queue.try_pop(ret); !val)
		return {};

	return ret;
}

