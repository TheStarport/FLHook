#include "global.h"

#include <intrin.h>

void Console::Log(std::wstring& wStr, va_list args, void* addr)
{
	wchar_t wszBuf[1024 * 8] = L"";

	_vsnwprintf_s(wszBuf, (sizeof(wszBuf) / 2) - 1, wStr.c_str(), args);
	unsigned long iCharsWritten;
	std::string scText = wstos(wszBuf) + "\n";

	// Get DLL Name
	HMODULE hmod;
	if (RtlPcToFileHeader(addr, (void**)&hmod))
	{
		CHAR sz[MAX_PATH];
		// If successful, prepend
		if (GetModuleFileName(hmod, sz, MAX_PATH))
		{
			std::string path = sz;
			scText =
			    std::string("(") + wstos(GetTimeString(FLHookConfig::i()->general.localTime)) +
			    path.substr(path.find_last_of("\\") + 1) + ") " + scText;
		}
	}

	WriteConsole(GetStdHandle(STD_OUTPUT_HANDLE), scText.c_str(), DWORD(scText.length()), &iCharsWritten, nullptr);

	// Reset
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::WHITE));
}

void Console::ConPrint(std::string str, ...)
{
	va_list marker;
	va_start(marker, str);
	ConPrint(stows(str), marker);
}

void Console::ConErr(std::string str, ...)
{
	va_list marker;
	va_start(marker, str);
	ConErr(stows(str), marker);
}

void Console::ConWarn(std::string str, ...)
{
	va_list marker;
	va_start(marker, str);
	ConWarn(stows(str), marker);
}

void Console::ConInfo(std::string str, ...)
{
	va_list marker;
	va_start(marker, str);
	ConInfo(stows(str), marker);
}

void Console::ConDebug(std::string str, ...)
{
	va_list marker;
	va_start(marker, str);
	ConDebug(stows(str), marker);
}

void Console::ConPrint(std::wstring wStr, ...)
{
	va_list marker;
	va_start(marker, wStr);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::STRONG_WHITE));
	Log(wStr, marker, _ReturnAddress());
}

void Console::ConErr(std::wstring wStr, ...)
{
	wStr = L"ERR: " + wStr;
	va_list marker;
	va_start(marker, wStr);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::STRONG_RED));
	Log(wStr, marker, _ReturnAddress());
}

void Console::ConWarn(std::wstring wStr, ...)
{
	wStr = L"WARN: " + wStr;
	va_list marker;
	va_start(marker, wStr);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::STRONG_YELLOW));
	Log(wStr, marker, _ReturnAddress());
}

void Console::ConInfo(std::wstring wStr, ...)
{
	wStr = L"INFO: " + wStr;
	va_list marker;
	va_start(marker, wStr);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::STRONG_CYAN));
	Log(wStr, marker, _ReturnAddress());
}

void Console::ConDebug(std::wstring wStr, ...)
{
	if (!FLHookConfig::i()->general.debugMode)
		return;

	wStr = L"DEBUG: " + wStr;
	va_list marker;
	va_start(marker, wStr);

	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), WORD(ConsoleColor::STRONG_GREEN));
	Log(wStr, marker, _ReturnAddress());
}