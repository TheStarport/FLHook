#include "CConsole.h"
#include <wchar.h>

void CConsole::DoPrint(const std::wstring &wscText)
{
	ConPrint(wscText);
}

std::wstring CConsole::GetAdminName()
{
	return L"Admin console";
}
