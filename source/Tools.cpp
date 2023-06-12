#include "PCH.hpp"

#include "Helpers/Client.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring IniGetS(const std::wstring& file, const std::wstring& app, const std::wstring& key, const std::wstring& def)
{
    wchar_t ret[2048 * 2];
    GetPrivateProfileStringW(app.c_str(), key.c_str(), def.c_str(), ret, sizeof ret, file.c_str());
    return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IniGetI(const std::wstring& file, const std::wstring& app, const std::wstring& key, int def)
{
    return GetPrivateProfileIntW(app.c_str(), key.c_str(), def, file.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float IniGetF(const std::wstring& file, const std::wstring& app, const std::wstring& key, float def)
{
    wchar_t ret[2048 * 2];
    wchar_t defaultVal[16];
    wsprintfW(defaultVal, L"%f", def);
    GetPrivateProfileStringW(app.c_str(), key.c_str(), defaultVal, ret, sizeof ret, file.c_str());
    return static_cast<float>(_wtof(ret));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IniGetB(const std::wstring& file, const std::wstring& app, const std::wstring& key, const bool def)
{
    const std::wstring val = StringUtils::ToLower(IniGetS(file, app, key, def ? L"true" : L"false"));
    return val == L"yes" || val == L"true";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWrite(const std::wstring& file, const std::wstring& app, const std::wstring& key, const std::wstring& Value)
{
    WritePrivateProfileStringW(app.c_str(), key.c_str(), Value.c_str(), file.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWriteW(const std::wstring& file, const std::wstring& app, const std::wstring& key, const std::wstring& value)
{
    std::wstring newValue;
    for (uint i = 0; i < value.length(); i++)
    {
        const char highByte = value[i] >> 8;
        const char lowByte = value[i] & 0xFF;
        wchar_t buf[8];
        swprintf_s(buf, L"%02X%02X", static_cast<uint>(highByte) & 0xFF, static_cast<uint>(lowByte) & 0xFF);
        newValue += buf;
    }
    WritePrivateProfileStringW(app.c_str(), key.c_str(), newValue.c_str(), file.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring IniGetWS(const std::wstring& file, const std::wstring& app, const std::wstring& key, const std::wstring& def)
{
    wchar_t ret[2048 * 2];
    GetPrivateProfileStringW(app.c_str(), key.c_str(), L"", ret, sizeof ret, file.c_str());
    const std::wstring value = ret;
    if (!value.length())
    {
        return def;
    }

    std::wstring newValue;
    long highByte;
    long lowByte;
    while (swscanf_s(value.c_str(), L"%02X%02X", &highByte, &lowByte) == 2)
    {
        newValue = newValue.substr(4);
        const wchar_t wChar = static_cast<wchar_t>(highByte << 8 | lowByte);
        newValue.append(1, wChar);
    }

    return newValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelete(const std::wstring& file, const std::wstring& app, const std::wstring& key)
{
    // TODO: DELETE ME, fix referneces
    WritePrivateProfileStringW(app.c_str(), key.c_str(), nullptr, file.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelSection(const std::wstring& file, const std::wstring& app)
{
    // TODO: DELETE ME, fix referneces
    WritePrivateProfileStringW(app.c_str(), nullptr, nullptr, file.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Determine the path name of a file in the charname account directory with the
provided extension. The resulting path is returned in the path parameter.
*/
std::wstring GetUserfilePath(const std::variant<uint, std::wstring_view>& player, const std::wstring& Extension)
{
    // init variables
    char DataPath[MAX_PATH];
    GetUserDataPath(DataPath);
    const std::wstring AcctPath = StringUtils::stows(std::string(DataPath) + "\\Accts\\MultiPlayer\\");

    const auto acc = Hk::Client::GetAccountByCharName(std::get<std::wstring_view>(player)).Raw();
    if (acc.has_error())
    {
        return L"";
    }

    const auto dir = Hk::Client::GetAccountDirName(acc.value());
    const auto file = Hk::Client::GetCharFileName(player).Raw();
    if (file.has_error())
    {
        return L"";
    }

    return AcctPath + dir + L"\\" + file.value() + Extension;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring GetTimeString(bool localTime)
{
    SYSTEMTIME st;
    if (localTime)
    {
        GetLocalTime(&st);
    }
    else
    {
        GetSystemTime(&st);
    }

    wchar_t buf[100];
    _snwprintf_s(buf, sizeof buf, L"%04d-%02d-%02d %02d:%02d:%02d ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
    return buf;
}
