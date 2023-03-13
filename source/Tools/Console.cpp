#include "Global.hpp"

#include <intrin.h>

void Console::Log(const std::string& str, void* addr)
{
	unsigned long iCharsWritten;
	std::string text = str + "\n";

	// Get DLL Name
	if (HMODULE dll; RtlPcToFileHeader(addr, (void**)&dll))
	{
		CHAR maxPath[MAX_PATH];
		// If successful, prepend
		if (GetModuleFileName(dll, maxPath, MAX_PATH))
		{
			const std::string path = maxPath;
			text = std::string("(") + wstos(GetTimeString(FLHookConfig::i()->general.localTime)) + path.substr(path.find_last_of("\\") + 1) + ") " + text;
		}
	}

	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), text.c_str(), static_cast<DWORD>(text.length()), &iCharsWritten, nullptr);

	// Reset
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(ConsoleColor::White));
}

void Console::ConPrint(const std::string& str)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(ConsoleColor::StrongWhite));
	Log(str, _ReturnAddress());
}

void Console::ConErr(const std::string& str)
{
	const auto newStr = "ERR: " + str;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(ConsoleColor::StrongRed));
	Log(newStr, _ReturnAddress());
}

void Console::ConWarn(const std::string& str)
{
	const auto newStr = "WARN: " + str;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(ConsoleColor::StrongYellow));
	Log(newStr, _ReturnAddress());
}

void Console::ConInfo(const std::string& str)
{
	const auto newStr = "INFO: " + str;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(ConsoleColor::StrongCyan));
	Log(newStr, _ReturnAddress());
}

void Console::ConDebug(const std::string& str)
{
	if (!FLHookConfig::i()->general.debugMode)
		return;

	const auto newStr = "DEBUG: " + str;

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(ConsoleColor::StrongGreen));
	Log(newStr, _ReturnAddress());
}
