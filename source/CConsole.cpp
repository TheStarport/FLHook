#include "CConsole.h"

#include <wchar.h>

// I hate this
void CConsole::DoPrint(const std::wstring& text)
{
	if (text.find(L"ERROR", 0) == 0)
	{
		Console::ConErr(ReplaceStr(text, L"ERROR", L""));
	}
	else if (text.find(L"error", 0) == 0)
	{
		Console::ConErr(ReplaceStr(text, L"error", L""));
	}
	else if (text.find(L"Error", 0) == 0)
	{
		Console::ConErr(ReplaceStr(text, L"Error", L""));
	}
	else if (text.find(L"ERR", 0) == 0)
	{
		Console::ConErr(ReplaceStr(text, L"ERR", L""));
	}
	else if (text.find(L"err", 0) == 0)
	{
		Console::ConErr(ReplaceStr(text, L"err", L""));
	}
	else
		Console::ConPrint(text);
}

std::wstring CConsole::GetAdminName()
{
	return L"Admin console";
}
