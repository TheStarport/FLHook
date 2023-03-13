#include "Global.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string IniGetS(const std::string& File, const std::string& App, const std::string& Key, const std::string& Default)
{
	char Ret[2048 * 2];
	GetPrivateProfileString(App.c_str(), Key.c_str(), Default.c_str(), Ret, sizeof(Ret), File.c_str());
	return Ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IniGetI(const std::string& File, const std::string& App, const std::string& Key, int iDefault)
{
	return GetPrivateProfileInt(App.c_str(), Key.c_str(), iDefault, File.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float IniGetF(const std::string& File, const std::string& App, const std::string& Key, float fDefault)
{
	char Ret[2048 * 2];
	char Default[16];
	sprintf_s(Default, "%f", fDefault);
	GetPrivateProfileString(App.c_str(), Key.c_str(), Default, Ret, sizeof(Ret), File.c_str());
	return (float)atof(Ret);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IniGetB(const std::string& File, const std::string& App, const std::string& Key, bool bDefault)
{
	std::string val = ToLower(IniGetS(File, App, Key, bDefault ? "true" : "false"));
	return val.compare("yes") == 0 || val.compare("true") == 0 ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWrite(const std::string& File, const std::string& App, const std::string& Key, const std::string& Value)
{
	WritePrivateProfileString(App.c_str(), Key.c_str(), Value.c_str(), File.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWriteW(const std::string& File, const std::string& App, const std::string& Key, const std::wstring& Value)
{
	std::string Value = "";
	for (uint i = 0; (i < Value.length()); i++)
	{
		char cHiByte = Value[i] >> 8;
		char cLoByte = Value[i] & 0xFF;
		char Buf[8];
		sprintf_s(Buf, "%02X%02X", ((uint)cHiByte) & 0xFF, ((uint)cLoByte) & 0xFF);
		Value += Buf;
	}
	WritePrivateProfileString(App.c_str(), Key.c_str(), Value.c_str(), File.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring IniGetWS(const std::string& File, const std::string& App, const std::string& Key, const std::wstring& Default)
{
	char Ret[2048 * 2];
	GetPrivateProfileString(App.c_str(), Key.c_str(), "", Ret, sizeof(Ret), File.c_str());
	std::string Value = Ret;
	if (!Value.length())
		return Default;

	std::wstring Value = L"";
	long lHiByte;
	long lLoByte;
	while (sscanf_s(Value.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
	{
		Value = Value.substr(4);
		wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
		Value.append(1, wChar);
	}

	return Value;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

[]
void IniDelete(const std::string& File, const std::string& App, const std::string& Key)
{
	// TODO: DELETE ME, fix referneces
	WritePrivateProfileString(App.c_str(), Key.c_str(), NULL, File.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelSection(const std::string& File, const std::string& App)
{
	// TODO: DELETE ME, fix referneces
	WritePrivateProfileString(App.c_str(), NULL, NULL, File.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Determine the path name of a file in the charname account directory with the
provided extension. The resulting path is returned in the path parameter.
*/
std::string GetUserFilePath(const std::variant<uint, std::wstring>& player, const std::string& Extension)
{
	// init variables
	char DataPath[MAX_PATH];
	GetUserDataPath(DataPath);
	std::string AcctPath = std::string(DataPath) + "\\Accts\\MultiPlayer\\";


	const auto acc = Hk::Client::GetAccountByCharName(std::get<std::wstring>(player));
	if (acc.has_error())
		return "";

	auto dir = Hk::Client::GetAccountDirName(acc.value());
	const auto file = Hk::Client::GetCharFileName(player);
	if (file.has_error())
		return "";

	return AcctPath + wstos(dir) + "\\" + wstos(file.value()) + Extension;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring GetTimeString(bool bLocalTime)
{
	SYSTEMTIME st;
	if (bLocalTime)
		GetLocalTime(&st);
	else
		GetSystemTime(&st);

	wchar_t wBuf[100];
	_snwprintf_s(
	    wBuf, sizeof(wBuf), L"%04d-%02d-%02d %02d:%02d:%02d ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute,
	    st.wSecond);
	return wBuf;
}