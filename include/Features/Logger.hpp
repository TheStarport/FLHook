#pragma once

#include <ext/Singleton.h>
#include <thread>
#include <list>

class Logger : public Singleton<Logger>
{
	HANDLE consoleInput;
	HANDLE consoleOutput;

	std::jthread consoleThread;
	std::list<std::wstring> consoleCommands;

	void GetConsoleInput(std::stop_token st) const;

  public:
	Logger();
	~Logger();
};