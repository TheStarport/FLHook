#pragma once

#include <ext/Singleton.h>
#include <thread>
#include <concurrent_queue.h>
#include <optional>
#include <string>

class Logger : public Singleton<Logger>
{
	HANDLE consoleInput;
	HANDLE consoleOutput;

	std::jthread consoleThread;
	concurrency::concurrent_queue<std::string> queue;

	void GetConsoleInput(std::stop_token st);

  public:
	Logger();
	~Logger();

	std::optional<std::string> GetCommand();
};