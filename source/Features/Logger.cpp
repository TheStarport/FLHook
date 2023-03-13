#include <Features/Logger.hpp>
#include <Windows.h>
#include <string>

BOOL WINAPI ConsoleHandler(DWORD dwCtrlType)
{
	switch (dwCtrlType)
	{
		case CTRL_CLOSE_EVENT: {
			return TRUE;
		}
		default:
			return FALSE;
		break;
	}
}

void Logger::GetConsoleInput(std::stop_token st)
{
	while (!st.stop_requested())
	{
		DWORD dwBytesRead;
		std::string cmd;
		cmd.resize(1024);
		if (ReadConsole(consoleInput, cmd.data(), cmd.size(), &dwBytesRead, nullptr))
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

Logger::Logger()
{
	// start console
	AllocConsole();
	SetConsoleTitle("FLHook");

	HWND console = GetConsoleWindow();
	RECT r;
	GetWindowRect(console, &r);

	MoveWindow(console, r.left, r.top, 1366, 768, TRUE);

	SetConsoleCtrlHandler(ConsoleHandler, TRUE);
	consoleInput = GetStdHandle(STD_INPUT_HANDLE);
	consoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);

	// change version number here:
	// https://patorjk.com/software/taag/#p=display&f=Big&t=FLHook%204.0
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
	WriteConsole(consoleOutput, welcomeText.c_str(), DWORD(welcomeText.length()), &_, nullptr);

	consoleThread = std::jthread(&Logger::GetConsoleInput, this);
}

Logger::~Logger()
{
	SetConsoleCtrlHandler(ConsoleHandler, FALSE);
	FreeConsole();
}

std::optional<std::string> Logger::GetCommand()
{
	if (queue.empty())
		return {};

	std::string ret;
	const auto val = queue.try_pop(ret);

	return ret;
}
