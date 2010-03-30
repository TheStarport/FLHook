#include "CConsole.h"
#include <wchar.h>

void CConsole::DoPrint(wstring wscText)
{
	ConPrint(wscText);
}

wstring CConsole::GetAdminName()
{
	return L"Admin console";
}
