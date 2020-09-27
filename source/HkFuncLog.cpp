#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddDebugLog(const char *szString, ...) {
    if (!set_bDebug || (!fLogDebug && !IsDebuggerPresent()))
        return;

    char szBufString[1024];
    va_list marker;
    va_start(marker, szString);
    _vsnprintf_s(szBufString, sizeof(szBufString) - 1, szString, marker);

    if (fLogDebug) {
        if (ftell(fLogDebug) > ((int)set_iDebugMaxSize << 10)) {
            fclose(fLogDebug);
            _unlink(sDebugLog.c_str());
            fopen_s(&fLogDebug, sDebugLog.c_str(), "at");
        }

        char szBuf[64];
        time_t tNow = time(0);
        tm t;
        localtime_s(&t, &tNow);
        strftime(szBuf, sizeof(szBuf), "%d.%m.%Y %H:%M:%S", &t);
        fprintf(fLogDebug, "[%s] %s\n", szBuf, szBufString);
        fflush(fLogDebug);
    }
    if (IsDebuggerPresent()) {
        OutputDebugString(
            ("[DEBUG] " + std::string(szBufString) + "\n").c_str());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void AddLog(const char *szString, ...) {
    if (!fLog && !IsDebuggerPresent())
        return;

    char szBufString[1024];
    va_list marker;
    va_start(marker, szString);
    _vsnprintf_s(szBufString, sizeof(szBufString) - 1, szString, marker);

    if (fLog) {
        char szBuf[64];
        time_t tNow = time(0);
        tm t;
        localtime_s(&t, &tNow);
        strftime(szBuf, sizeof(szBuf), "%d.%m.%Y %H:%M:%S", &t);
        fprintf(fLog, "[%s] %s\n", szBuf, szBufString);
        fflush(fLog);
    }
    if (IsDebuggerPresent()) {
        OutputDebugString(("[LOG] " + std::string(szBufString) + "\n").c_str());
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkHandleCheater(uint iClientID, bool bBan, std::wstring wscReason, ...) {
    wchar_t wszBuf[1024 * 8] = L"";
    va_list marker;
    va_start(marker, wscReason);

    _vsnwprintf_s(wszBuf, (sizeof(wszBuf) / 2) - 1, wscReason.c_str(), marker);

    HkAddCheaterLog(iClientID, wszBuf);

    if (wscReason[0] != '#' && Players.GetActiveCharacterName(iClientID)) {
        std::wstring wscCharname =
            (wchar_t *)Players.GetActiveCharacterName(iClientID);

        wchar_t wszBuf2[500];
        swprintf_s(wszBuf2, L"Possible cheating detected: %s",
                   wscCharname.c_str());
        HkMsgU(wszBuf2);
    }

    if (bBan)
        HkBan(ARG_CLIENTID(iClientID), true);
    if (wscReason[0] != '#')
        HkKick(ARG_CLIENTID(iClientID));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkAddCheaterLog(const std::wstring &wscCharname,
                     const std::wstring &wscReason) {
    FILE *f;
    fopen_s(&f, ("./flhook_logs/flhook_cheaters.log"), "at");
    if (!f)
        return false;

    CAccount *acc = HkGetAccountByCharname(wscCharname);
    std::wstring wscAccountDir = L"???";
    std::wstring wscAccountID = L"???";
    if (acc) {
        HkGetAccountDirName(acc, wscAccountDir);
        wscAccountID = HkGetAccountID(acc);
    }

    uint iClientID = HkGetClientIdFromCharname(wscCharname);
    std::wstring wscHostName = L"???";
    std::wstring wscIp = L"???";
    if (iClientID != -1) {
        wscHostName = ClientInfo[iClientID].wscHostname;
        HkGetPlayerIP(iClientID, wscIp);
    }

    time_t tNow = time(nullptr);
    tm stNow;
    localtime_s(&stNow, &tNow);
    fprintf(f,
            "%.2d/%.2d/%.4d %.2d:%.2d:%.2d Possible cheating detected (%s) by "
            "%s(%s)(%s) [%s %s]\n",
            stNow.tm_mon + 1, stNow.tm_mday, stNow.tm_year + 1900,
            stNow.tm_hour, stNow.tm_min, stNow.tm_sec, wstos(wscReason).c_str(),
            wstos(wscCharname).c_str(), wstos(wscAccountDir).c_str(),
            wstos(wscAccountID).c_str(), wstos(wscHostName).c_str(),
            wstos(wscIp).c_str());
    fclose(f);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkAddCheaterLog(const uint &iClientID, const std::wstring &wscReason) {
    FILE *f;
    fopen_s(&f, ("./flhook_logs/flhook_cheaters.log"), "at");
    if (!f)
        return false;

    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    std::wstring wscAccountDir = L"???";
    std::wstring wscAccountID = L"???";
    if (acc) {
        HkGetAccountDirName(acc, wscAccountDir);
        wscAccountID = HkGetAccountID(acc);
    }

    std::wstring wscHostName = L"???";
    std::wstring wscIp = L"???";

    wscHostName = ClientInfo[iClientID].wscHostname;
    HkGetPlayerIP(iClientID, wscIp);

    std::wstring wscCharname =
        L"? ? ?"; // spaces to make clear it's not a player name
    if (Players.GetActiveCharacterName(iClientID)) {
        wscCharname = (wchar_t *)Players.GetActiveCharacterName(iClientID);
    }

    time_t tNow = time(0);
    tm stNow;
    localtime_s(&stNow, &tNow);
    fprintf(f,
            "%.2d/%.2d/%.4d %.2d:%.2d:%.2d Possible cheating detected (%s) by "
            "%s(%s)(%s) [%s %s]\n",
            stNow.tm_mon + 1, stNow.tm_mday, stNow.tm_year + 1900,
            stNow.tm_hour, stNow.tm_min, stNow.tm_sec, wstos(wscReason).c_str(),
            wstos(wscCharname).c_str(), wstos(wscAccountDir).c_str(),
            wstos(wscAccountID).c_str(), wstos(wscHostName).c_str(),
            wstos(wscIp).c_str());
    fclose(f);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkAddKickLog(uint iClientID, std::wstring wscReason, ...) {
    wchar_t wszBuf[1024 * 8] = L"";
    va_list marker;
    va_start(marker, wscReason);

    _vsnwprintf_s(wszBuf, (sizeof(wszBuf) / 2) - 1, wscReason.c_str(), marker);

    FILE *f;
    fopen_s(&f, ("./flhook_logs/flhook_kicks.log"), "at");
    if (!f)
        return false;

    const wchar_t *wszCharname =
        (wchar_t *)Players.GetActiveCharacterName(iClientID);
    if (!wszCharname)
        wszCharname = L"";

    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    std::wstring wscAccountDir;
    HkGetAccountDirName(acc, wscAccountDir);

    time_t tNow = time(0);
    tm stNow;
    localtime_s(&stNow, &tNow);
    fprintf(f, "%.2d/%.2d/%.4d %.2d:%.2d:%.2d Kick (%s): %s(%s)(%s)\n",
            stNow.tm_mon + 1, stNow.tm_mday, stNow.tm_year + 1900,
            stNow.tm_hour, stNow.tm_min, stNow.tm_sec, wstos(wszBuf).c_str(),
            wstos(wszCharname).c_str(), wstos(wscAccountDir).c_str(),
            wstos(HkGetAccountID(acc)).c_str());
    fclose(f);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool HkAddConnectLog(uint iClientID, std::wstring wscReason, ...) {
    wchar_t wszBuf[1024 * 8] = L"";
    va_list marker;
    va_start(marker, wscReason);

    _vsnwprintf_s(wszBuf, (sizeof(wszBuf) / 2) - 1, wscReason.c_str(), marker);

    FILE *f;
    fopen_s(&f, ("./flhook_logs/flhook_connects.log"), "at");
    if (!f)
        return false;

    const wchar_t *wszCharname =
        (wchar_t *)Players.GetActiveCharacterName(iClientID);
    if (!wszCharname)
        wszCharname = L"";

    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    std::wstring wscAccountDir;
    HkGetAccountDirName(acc, wscAccountDir);

    time_t tNow = time(0);
    tm stNow;
    localtime_s(&stNow, &tNow);
    fprintf(f, "%.2d/%.2d/%.4d %.2d:%.2d:%.2d Connect (%s): %s(%s)(%s)\n",
            stNow.tm_mon + 1, stNow.tm_mday, stNow.tm_year + 1900,
            stNow.tm_hour, stNow.tm_min, stNow.tm_sec, wstos(wszBuf).c_str(),
            wstos(wszCharname).c_str(), wstos(wscAccountDir).c_str(),
            wstos(HkGetAccountID(acc)).c_str());
    fclose(f);
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

static void AddLog(FILE *fLog, const char *szString, ...) {
    char szBufString[1024];
    va_list marker;
    va_start(marker, szString);
    _vsnprintf_s(szBufString, sizeof(szBufString) - 1, szString, marker);

    char szBuf[64];
    time_t tNow = time(0);
    tm t;
    localtime_s(&t, &tNow);
    strftime(szBuf, sizeof(szBuf), "%d.%m.%Y %H:%M:%S", &t);
    fprintf(fLog, "[%s] %s\n", szBuf, szBufString);
    fflush(fLog);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkAddAdminCmdLog(const char *szString, ...) {
    char szBufString[1024];
    va_list marker;
    va_start(marker, szString);
    _vsnprintf_s(szBufString, sizeof(szBufString) - 1, szString, marker);

    FILE *f;
    fopen_s(&f, ("./flhook_logs/flhook_admincmds.log"), "at");
    if (!f)
        return;

    AddLog(f, "%s", szBufString);

    fclose(f);
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkAddSocketCmdLog(const char *szString, ...) {
    char szBufString[1024];
    va_list marker;
    va_start(marker, szString);
    _vsnprintf_s(szBufString, sizeof(szBufString) - 1, szString, marker);

    FILE *f;
    fopen_s(&f, ("./flhook_logs/flhook_socketcmds.log"), "at");
    if (!f)
        return;

    AddLog(f, "%s", szBufString);

    fclose(f);
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkAddUserCmdLog(const char *szString, ...) {
    char szBufString[1024];
    va_list marker;
    va_start(marker, szString);
    _vsnprintf_s(szBufString, sizeof(szBufString) - 1, szString, marker);

    FILE *f;
    fopen_s(&f, ("./flhook_logs/flhook_usercmds.log"), "at");
    if (!f)
        return;

    AddLog(f, "%s", szBufString);

    fclose(f);
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkAddPerfTimerLog(const char *szString, ...) {
    char szBufString[1024];
    va_list marker;
    va_start(marker, szString);
    _vsnprintf_s(szBufString, sizeof(szBufString) - 1, szString, marker);

    FILE *f;
    fopen_s(&f, ("./flhook_logs/flhook_perftimers.log"), "at");
    if (!f)
        return;

    AddLog(f, "%s", szBufString);

    fclose(f);
    return;
}
