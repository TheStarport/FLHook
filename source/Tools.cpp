#include "Global.hpp"

#include <algorithm>
#include <time.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string itohexs(uint value)
{
	char buf[16];
	sprintf_s(buf, "%08X", value);
	return buf;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string IniGetS(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::string& scDefault)
{
	char szRet[2048 * 2];
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), scDefault.c_str(), szRet, sizeof(szRet), scFile.c_str());
	return szRet;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IniGetI(const std::string& scFile, const std::string& scApp, const std::string& scKey, int iDefault)
{
	return GetPrivateProfileInt(scApp.c_str(), scKey.c_str(), iDefault, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float IniGetF(const std::string& scFile, const std::string& scApp, const std::string& scKey, float fDefault)
{
	char szRet[2048 * 2];
	char szDefault[16];
	sprintf_s(szDefault, "%f", fDefault);
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), szDefault, szRet, sizeof(szRet), scFile.c_str());
	return (float)atof(szRet);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IniGetB(const std::string& scFile, const std::string& scApp, const std::string& scKey, bool bDefault)
{
	std::string val = ToLower(IniGetS(scFile, scApp, scKey, bDefault ? "true" : "false"));
	return val.compare("yes") == 0 || val.compare("true") == 0 ? true : false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWrite(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::string& scValue)
{
	WritePrivateProfileString(scApp.c_str(), scKey.c_str(), scValue.c_str(), scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWriteW(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::wstring& wscValue)
{
	std::string scValue = "";
	for (uint i = 0; (i < wscValue.length()); i++)
	{
		char cHiByte = wscValue[i] >> 8;
		char cLoByte = wscValue[i] & 0xFF;
		char szBuf[8];
		sprintf_s(szBuf, "%02X%02X", ((uint)cHiByte) & 0xFF, ((uint)cLoByte) & 0xFF);
		scValue += szBuf;
	}
	WritePrivateProfileString(scApp.c_str(), scKey.c_str(), scValue.c_str(), scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring IniGetWS(const std::string& scFile, const std::string& scApp, const std::string& scKey, const std::wstring& wscDefault)
{
	char szRet[2048 * 2];
	GetPrivateProfileString(scApp.c_str(), scKey.c_str(), "", szRet, sizeof(szRet), scFile.c_str());
	std::string scValue = szRet;
	if (!scValue.length())
		return wscDefault;

	std::wstring wscValue = L"";
	long lHiByte;
	long lLoByte;
	while (sscanf_s(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
	{
		scValue = scValue.substr(4);
		wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
		wscValue.append(1, wChar);
	}

	return wscValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelete(const std::string& scFile, const std::string& scApp, const std::string& scKey)
{
	WritePrivateProfileString(scApp.c_str(), scKey.c_str(), NULL, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelSection(const std::string& scFile, const std::string& scApp)
{
	WritePrivateProfileString(scApp.c_str(), NULL, NULL, scFile.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Determine the path name of a file in the charname account directory with the
provided extension. The resulting path is returned in the path parameter.
*/
std::string GetUserFilePath(const std::variant<uint, std::wstring>& player, const std::string& scExtension)
{
	// init variables
	char szDataPath[MAX_PATH];
	GetUserDataPath(szDataPath);
	std::string scAcctPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\";


	const auto acc = Hk::Client::GetAccountByCharName(std::get<std::wstring>(player));
	if (acc.has_error())
		return "";

	auto dir = Hk::Client::GetAccountDirName(acc.value());
	const auto file = Hk::Client::GetCharFileName(player);
	if (file.has_error())
		return "";

	return scAcctPath + wstos(dir) + "\\" + wstos(file.value()) + scExtension;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL FileExists(LPCTSTR szPath)
{
	DWORD dwAttrib = GetFileAttributes(szPath);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring GetTimeString(bool bLocalTime)
{
	SYSTEMTIME st;
	if (bLocalTime)
		GetLocalTime(&st);
	else
		GetSystemTime(&st);

	wchar_t wszBuf[100];
	_snwprintf_s(
	    wszBuf, sizeof(wszBuf), L"%04d-%02d-%02d %02d:%02d:%02d ", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute,
	    st.wSecond);
	return wszBuf;
}

void ini_write_wstring(FILE* file, const std::string& parmname, const std::wstring& in)
{
	fprintf(file, "%s=", parmname.c_str());
	for (int i = 0; i < (int)in.size(); i++)
	{
		UINT v1 = in[i] >> 8;
		UINT v2 = in[i] & 0xFF;
		fprintf(file, "%02x%02x", v1, v2);
	}
	fprintf(file, "\n");
}

void ini_get_wstring(INI_Reader& ini, std::wstring& wscValue)
{
	std::string scValue = ini.get_value_string();
	wscValue = L"";
	long lHiByte;
	long lLoByte;
	while (sscanf_s(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
	{
		scValue = scValue.substr(4);
		wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
		wscValue.append(1, wChar);
	}
}
