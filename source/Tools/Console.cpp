#include "Global.hpp"

#include <intrin.h>

void Console::Log(const std::string& str, void* addr)
{
	unsigned long iCharsWritten;
	std::string text = str + "\n";

	// Get DLL Name
	if (HMODULE hmod; RtlPcToFileHeader(addr, (void**)&hmod))
	{
		CHAR sz[MAX_PATH];
		// If successful, prepend
		if (GetModuleFileName(hmod, sz, MAX_PATH))
		{
			std::string path = sz;
			text = std::string("(") + wstos(GetTimeString(FLHookConfig::i()->general.localTime)) + path.substr(path.find_last_of("\\") + 1) + ") " + text;
		}
	}

	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), text.c_str(), DWORD(text.length()), &iCharsWritten, nullptr);

	// Reset
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::WHITE));
}

void Console::ConPrint(const std::string& str)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::STRONG_WHITE));
	Log(str, _ReturnAddress());
}

void Console::ConErr(const std::string& str)
{
	const auto newStr = "ERR: " + str;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::STRONG_RED));
	Log(newStr, _ReturnAddress());
}

void Console::ConWarn(const std::string& str)
{
	const auto newStr = "WARN: " + str;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::STRONG_YELLOW));
	Log(newStr, _ReturnAddress());
}

void Console::ConInfo(const std::string& str)
{
	const auto newStr = "INFO: " + str;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::STRONG_CYAN));
	Log(newStr, _ReturnAddress());
}

void Console::ConDebug(const std::string& str)
{
	if (!FLHookConfig::i()->general.debugMode)
		return;

	const auto newStr = "DEBUG: " + str;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::STRONG_GREEN));
	Log(newStr, _ReturnAddress());
}