#include "CConsole.h"
#include <wchar.h>

void CConsole::DoPrint(const std::wstring &wscText) { Console::ConPrint(wscText); }

std::wstring CConsole::GetAdminName() { return L"Admin console"; }
