#include <time.h>
#include "global.h"
#include "hook.h"
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring stows(const std::string &scText) {
    int iSize = MultiByteToWideChar(CP_ACP, 0, scText.c_str(), -1, 0, 0);
    wchar_t *wszText = new wchar_t[iSize];
    MultiByteToWideChar(CP_ACP, 0, scText.c_str(), -1, wszText, iSize);
    std::wstring wscRet = wszText;
    delete[] wszText;
    return wscRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string wstos(const std::wstring &wscText) {
    uint iLen = (uint)wscText.length() + 1;
    char *szBuf = new char[iLen];
    WideCharToMultiByte(CP_ACP, 0, wscText.c_str(), -1, szBuf, iLen, 0, 0);
    std::string scRet = szBuf;
    delete[] szBuf;
    return scRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring ToLower(std::wstring wscStr) {
    std::transform(wscStr.begin(), wscStr.end(), wscStr.begin(), towlower);
    return wscStr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string ToLower(std::string scStr) {
    std::transform(scStr.begin(), scStr.end(), scStr.begin(), tolower);
    return scStr;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int ToInt(const std::wstring &wscStr) { return wcstoul(wscStr.c_str(), 0, 10); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

uint ToUInt(const std::wstring &wscStr) {
    return wcstoul(wscStr.c_str(), 0, 10);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float ToFloat(const std::wstring &wscStr) {
    return (float)atof(wstos(wscStr).c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string itohexs(uint value) {
    char buf[16];
    sprintf_s(buf, "%08X", value);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 999.999.999

std::wstring ToMoneyStr(int iCash) {
    int iMillions = iCash / 1000000;
    int iThousands = (iCash % 1000000) / 1000;
    int iRest = (iCash % 1000);
    wchar_t wszBuf[32];

    if (iMillions)
        swprintf_s(wszBuf, L"%d.%.3d.%.3d", iMillions, abs(iThousands),
                   abs(iRest));
    else if (iThousands)
        swprintf_s(wszBuf, L"%d.%.3d", iThousands, abs(iRest));
    else
        swprintf_s(wszBuf, L"%d", iRest);

    return wszBuf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string IniGetS(const std::string &scFile, const std::string &scApp,
                    const std::string &scKey, const std::string &scDefault) {
    char szRet[2048 * 2];
    GetPrivateProfileString(scApp.c_str(), scKey.c_str(), scDefault.c_str(),
                            szRet, sizeof(szRet), scFile.c_str());
    return szRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IniGetI(const std::string &scFile, const std::string &scApp,
            const std::string &scKey, int iDefault) {
    return GetPrivateProfileInt(scApp.c_str(), scKey.c_str(), iDefault,
                                scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float IniGetF(const std::string &scFile, const std::string &scApp,
              const std::string &scKey, float fDefault) {
    char szRet[2048 * 2];
    char szDefault[16];
    sprintf_s(szDefault, "%f", fDefault);
    GetPrivateProfileString(scApp.c_str(), scKey.c_str(), szDefault, szRet,
                            sizeof(szRet), scFile.c_str());
    return (float)atof(szRet);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IniGetB(const std::string &scFile, const std::string &scApp,
             const std::string &scKey, bool bDefault) {
    std::string val =
        ToLower(IniGetS(scFile, scApp, scKey, bDefault ? "true" : "false"));
    return val.compare("yes") == 0 || val.compare("true") == 0 ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWrite(const std::string &scFile, const std::string &scApp,
              const std::string &scKey, const std::string &scValue) {
    WritePrivateProfileString(scApp.c_str(), scKey.c_str(), scValue.c_str(),
                              scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWriteW(const std::string &scFile, const std::string &scApp,
               const std::string &scKey, const std::wstring &wscValue) {
    std::string scValue = "";
    for (uint i = 0; (i < wscValue.length()); i++) {
        char cHiByte = wscValue[i] >> 8;
        char cLoByte = wscValue[i] & 0xFF;
        char szBuf[8];
        sprintf_s(szBuf, "%02X%02X", ((uint)cHiByte) & 0xFF,
                  ((uint)cLoByte) & 0xFF);
        scValue += szBuf;
    }
    WritePrivateProfileString(scApp.c_str(), scKey.c_str(), scValue.c_str(),
                              scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring IniGetWS(const std::string &scFile, const std::string &scApp,
                      const std::string &scKey,
                      const std::wstring &wscDefault) {
    char szRet[2048 * 2];
    GetPrivateProfileString(scApp.c_str(), scKey.c_str(), "", szRet,
                            sizeof(szRet), scFile.c_str());
    std::string scValue = szRet;
    if (!scValue.length())
        return wscDefault;

    std::wstring wscValue = L"";
    long lHiByte;
    long lLoByte;
    while (sscanf_s(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2) {
        scValue = scValue.substr(4);
        wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
        wscValue.append(1, wChar);
    }

    return wscValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelete(const std::string &scFile, const std::string &scApp,
               const std::string &scKey) {
    WritePrivateProfileString(scApp.c_str(), scKey.c_str(), NULL,
                              scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelSection(const std::string &scFile, const std::string &scApp) {
    WritePrivateProfileString(scApp.c_str(), NULL, NULL, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniGetSection(const std::string &scFile, const std::string &scApp,
                   std::list<INISECTIONVALUE> &lstValues) {
    lstValues.clear();
    char szBuf[0xFFFF];
    GetPrivateProfileSection(scApp.c_str(), szBuf, sizeof(szBuf),
                             scFile.c_str());
    char *szNext = szBuf;
    while (strlen(szNext) > 0) {
        INISECTIONVALUE isv;
        char szKey[0xFFFF] = "";
        char szValue[0xFFFF] = "";
        sscanf_s(szNext, "%[^=]=%[^\n]", szKey, std::size(szKey), szValue,
                 std::size(szValue));
        isv.scKey = szKey;
        isv.scValue = szValue;
        lstValues.push_back(isv);

        szNext += strlen(szNext) + 1;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Determine the path name of a file in the charname account directory with the
provided extension. The resulting path is returned in the path parameter.
*/
std::string GetUserFilePath(const std::wstring &wscCharname,
                            const std::string &scExtension) {
    // init variables
    char szDataPath[MAX_PATH];
    GetUserDataPath(szDataPath);
    std::string scAcctPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\";

    std::wstring wscDir;
    std::wstring wscFile;
    if (HkGetAccountDirName(wscCharname, wscDir) != HKE_OK)
        return "";
    if (HkGetCharFileName(wscCharname, wscFile) != HKE_OK)
        return "";

    return scAcctPath + wstos(wscDir) + "\\" + wstos(wscFile) + scExtension;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring XMLText(const std::wstring &wscText) {
    std::wstring wscRet;
    for (uint i = 0; (i < wscText.length()); i++) {
        if (wscText[i] == '<')
            wscRet.append(L"&#60;");
        else if (wscText[i] == '>')
            wscRet.append(L"&#62;");
        else if (wscText[i] == '&')
            wscRet.append(L"&#38;");
        else
            wscRet.append(1, wscText[i]);
    }

    return wscRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void WriteProcMem(void *pAddress, void *pMem, int iSize) {
    HANDLE hProc =
        OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
                    FALSE, GetCurrentProcessId());
    DWORD dwOld;
    VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
    WriteProcessMemory(hProc, pAddress, pMem, iSize, 0);
    CloseHandle(hProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ReadProcMem(void *pAddress, void *pMem, int iSize) {
    HANDLE hProc =
        OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
                    FALSE, GetCurrentProcessId());
    DWORD dwOld;
    VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
    ReadProcessMemory(hProc, pAddress, pMem, iSize, 0);
    CloseHandle(hProc);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring GetParam(const std::wstring &wscLine, wchar_t wcSplitChar,
                      uint iPos) {
    uint i = 0, j = 0;

    std::wstring wscResult = L"";
    for (i = 0, j = 0; (i <= iPos) && (j < wscLine.length()); j++) {
        if (wscLine[j] == wcSplitChar) {
            while (((j + 1) < wscLine.length()) &&
                   (wscLine[j + 1] == wcSplitChar))
                j++; // skip "whitechar"

            i++;
            continue;
        }

        if (i == iPos)
            wscResult += wscLine[j];
    }

    return wscResult;
}

std::string GetParam(std::string scLine, char cSplitChar, uint iPos) {
    uint i = 0, j = 0;

    std::string scResult;
    for (i = 0, j = 0; (i <= iPos) && (j < scLine.length()); j++) {
        if (scLine[j] == cSplitChar) {
            while (((j + 1) < scLine.length()) && (scLine[j + 1] == cSplitChar))
                j++; // skip "whitechar"

            i++;
            continue;
        }

        if (i == iPos)
            scResult += scLine[j];
    }

    return scResult;
}

/**
This function is similar to GetParam but instead returns everything
from the parameter specified by iPos to the end of wscLine.

wscLine - the std::string to get parameters from
wcSplitChar - the seperator character
iPos - the parameter number to start from.
*/
std::wstring GetParamToEnd(const std::wstring &wscLine, wchar_t wcSplitChar,
                           uint iPos) {
    for (uint i = 0, j = 0; (i <= iPos) && (j < wscLine.length()); j++) {
        if (wscLine[j] == wcSplitChar) {
            while (((j + 1) < wscLine.length()) &&
                   (wscLine[j + 1] == wcSplitChar))
                j++; // skip "whitechar"
            i++;
            continue;
        }
        if (i == iPos) {
            return wscLine.substr(j);
        }
    }
    return L"";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring ReplaceStr(const std::wstring &wscSource,
                        const std::wstring &wscSearchFor,
                        const std::wstring &wscReplaceWith) {
    uint lPos, sPos = 0;

    std::wstring wscResult = wscSource;
    while ((lPos = (uint)wscResult.find(wscSearchFor, sPos)) != -1) {
        wscResult.replace(lPos, wscSearchFor.length(), wscReplaceWith);
        sPos = lPos + wscReplaceWith.length();
    }

    return wscResult;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

mstime timeInMS() {
    mstime iCount;
    QueryPerformanceCounter((LARGE_INTEGER *)&iCount);
    mstime iFreq;
    QueryPerformanceFrequency((LARGE_INTEGER *)&iFreq);
    return 1000 * iCount / iFreq;
}

/// Use this function to get the ticks since system startup. The FLHook
/// timeInMS() function seems to report inaccurate time when the FLServer.exe
/// process freezes (which happens due to other bugs).
mstime GetTimeInMS() {
    static mstime msBaseTime = 0;
    static mstime msLastTickCount = 0;

    mstime msCurTime = GetTickCount();
    // GetTickCount is 32 bits and so wraps around ever 49.5 days
    // If a wrap around has occurred then
    if (msCurTime < msLastTickCount)
        msBaseTime += (2 ^ 32);
    msLastTickCount = msCurTime;
    return msBaseTime + msLastTickCount;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SwapBytes(void *ptr, uint iLen) {
    if (iLen % 4)
        return;

    for (uint i = 0; i < iLen; i += 4) {
        char *ptr1 = (char *)ptr + i;
        unsigned long temp;
        memcpy(&temp, ptr1, 4);
        char *ptr2 = (char *)&temp;
        memcpy(ptr1, ptr2 + 3, 1);
        memcpy(ptr1 + 1, ptr2 + 2, 1);
        memcpy(ptr1 + 2, ptr2 + 1, 1);
        memcpy(ptr1 + 3, ptr2, 1);
    }
}

FARPROC PatchCallAddr(char *hMod, DWORD dwInstallAddress,
                      char *dwHookFunction) {
    DWORD dwRelAddr;
    ReadProcMem(hMod + dwInstallAddress + 1, &dwRelAddr, 4);

    DWORD dwOffset =
        (DWORD)dwHookFunction - (DWORD)(hMod + dwInstallAddress + 5);
    WriteProcMem(hMod + dwInstallAddress + 1, &dwOffset, 4);

    return (FARPROC)(hMod + dwRelAddr + dwInstallAddress + 5);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL FileExists(LPCTSTR szPath) {
    DWORD dwAttrib = GetFileAttributes(szPath);

    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
            !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Remove leading and trailing spaces from the std::string  ~FlakCommon by Motah.
*/
template <typename Str> Str Trim(const Str &scIn) {
    if (scIn.empty())
        return scIn;

    using Char = typename Str::value_type;
    constexpr auto trimmable = []() constexpr {
        if constexpr (std::is_same_v<Char, char>)
            return " \t\n\r";
        else if constexpr (std::is_same_v<Char, wchar_t>)
            return L" \t\n\r";
    }
    ();

    auto start = scIn.find_first_not_of(trimmable);
    auto end = scIn.find_last_not_of(trimmable);

    return scIn.substr(start, end - start + 1);
}

std::wstring GetTimeString(bool bLocalTime) {
    SYSTEMTIME st;
    if (bLocalTime)
        GetLocalTime(&st);
    else
        GetSystemTime(&st);

    wchar_t wszBuf[100];
    _snwprintf_s(wszBuf, sizeof(wszBuf), L"%04d-%02d-%02d %02d:%02d:%02d SMT ",
                 st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute,
                 st.wSecond);
    return wszBuf;
}

void ini_write_wstring(FILE *file, const std::string &parmname,
                       const std::wstring &in) {
    fprintf(file, "%s=", parmname.c_str());
    for (int i = 0; i < (int)in.size(); i++) {
        UINT v1 = in[i] >> 8;
        UINT v2 = in[i] & 0xFF;
        fprintf(file, "%02x%02x", v1, v2);
    }
    fprintf(file, "\n");
}

void ini_get_wstring(INI_Reader &ini, std::wstring &wscValue) {
    std::string scValue = ini.get_value_string();
    wscValue = L"";
    long lHiByte;
    long lLoByte;
    while (sscanf_s(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2) {
        scValue = scValue.substr(4);
        wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
        wscValue.append(1, wChar);
    }
}
