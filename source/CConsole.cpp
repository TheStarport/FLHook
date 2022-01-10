#include "CConsole.h"
#include <wchar.h>

// I hate this
void CConsole::DoPrint(const std::wstring &wscText) {
    if (wscText.find(L"ERROR", 0) == 0) {
        Console::ConErr(ReplaceStr(wscText, L"ERROR", L""));
    } else if (wscText.find(L"error", 0) == 0) {
        Console::ConErr(ReplaceStr(wscText, L"error", L""));
    }
    else if (wscText.find(L"Error", 0) == 0) {
        Console::ConErr(ReplaceStr(wscText, L"Error", L""));
    }
    else if (wscText.find(L"ERR", 0) == 0) {
        Console::ConErr(ReplaceStr(wscText, L"ERR", L""));
    } else if (wscText.find(L"err", 0) == 0) {
        Console::ConErr(ReplaceStr(wscText, L"err", L""));
    }else 
        Console::ConPrint(wscText);
}

std::wstring CConsole::GetAdminName() { return L"Admin console"; }
