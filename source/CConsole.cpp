#include "CConsole.h"

#include <wchar.h>

// I hate this
void CConsole::DoPrint(const std::string& text)
{
	if (text.find("ERROR", 0) == 0)
	{
		Console::ConErr(ReplaceStr(text, "ERROR", ""));
	}
	else if (text.find("error", 0) == 0)
	{
		Console::ConErr(ReplaceStr(text, "error", ""));
	}
	else if (text.find("Error", 0) == 0)
	{
		Console::ConErr(ReplaceStr(text, "Error", ""));
	}
	else if (text.find("ERR", 0) == 0)
	{
		Console::ConErr(ReplaceStr(text, "ERR", ""));
	}
	else if (text.find("err", 0) == 0)
	{
		Console::ConErr(ReplaceStr(text, "err", ""));
	}
	else
		Console::ConPrint(text);
}

std::wstring CConsole::GetAdminName()
{
	return L"Admin console";
}
