#include "PCH.hpp"
#include "Global.hpp"
#include "Helpers/Client.hpp"
#include "Tools/Utils.hpp"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string IniGetS(const std::string& file, const std::string& app, const std::string& key, const std::string& def)
{
	char ret[2048 * 2];
	GetPrivateProfileString(app.c_str(), key.c_str(), def.c_str(), ret, sizeof(ret), file.c_str());
	return ret;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int IniGetI(const std::string& file, const std::string& app, const std::string& key, int def)
{
	return GetPrivateProfileInt(app.c_str(), key.c_str(), def, file.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float IniGetF(const std::string& file, const std::string& app, const std::string& key, float def)
{
	char ret[2048 * 2];
	char defaultVal[16];
	sprintf_s(defaultVal, "%f", def);
	GetPrivateProfileString(app.c_str(), key.c_str(), defaultVal, ret, sizeof(ret), file.c_str());
	return static_cast<float>(atof(ret));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IniGetB(const std::string& file, const std::string& app, const std::string& key, const bool def)
{
	const std::string val = ToLower(IniGetS(file, app, key, def ? "true" : "false"));
	return val == "yes" || val == "true";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWrite(const std::string& file, const std::string& app, const std::string& key, const std::string& Value)
{
	WritePrivateProfileString(app.c_str(), key.c_str(), Value.c_str(), file.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniWriteW(const std::string& file, const std::string& app, const std::string& key, const std::wstring& value)
{
	std::string newValue;
	for (uint i = 0; (i < value.length()); i++)
	{
		const char highByte = value[i] >> 8;
		const char lowByte = value[i] & 0xFF;
		char buf[8];
		sprintf_s(buf, "%02X%02X", static_cast<uint>(highByte) & 0xFF, static_cast<uint>(lowByte) & 0xFF);
		newValue += buf;
	}
	WritePrivateProfileString(app.c_str(), key.c_str(), newValue.c_str(), file.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring IniGetWS(const std::string& file, const std::string& app, const std::string& key, const std::wstring& def)
{
	char ret[2048 * 2];
	GetPrivateProfileString(app.c_str(), key.c_str(), "", ret, sizeof(ret), file.c_str());
	const std::string value = ret;
	if (!value.length())
		return def;

	std::wstring newValue;
	long highByte;
	long lowByte;
	while (sscanf_s(value.c_str(), "%02X%02X", &highByte, &lowByte) == 2)
	{
		newValue = newValue.substr(4);
		const wchar_t wChar = static_cast<wchar_t>((highByte << 8) | lowByte);
		newValue.append(1, wChar);
	}

	return newValue;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelete(const std::string& file, const std::string& app, const std::string& key)
{
	// TODO: DELETE ME, fix referneces
	WritePrivateProfileString(app.c_str(), key.c_str(), nullptr, file.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void IniDelSection(const std::string& file, const std::string& app)
{
	// TODO: DELETE ME, fix referneces
	WritePrivateProfileString(app.c_str(), nullptr, nullptr, file.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
Determine the path name of a file in the charname account directory with the
provided extension. The resulting path is returned in the path parameter.
*/
std::string GetUserfilePath(const std::variant<uint, std::wstring>& player, const std::string& Extension)
{
	// init variables
	char DataPath[MAX_PATH];
	GetUserDataPath(DataPath);
	const std::string AcctPath = std::string(DataPath) + "\\Accts\\MultiPlayer\\";

	const auto acc = Hk::Client::GetAccountByCharName(std::get<std::wstring>(player));
	if (acc.has_error())
		return "";

	const auto dir = Hk::Client::GetAccountDirName(acc.value());
	const auto file = Hk::Client::GetCharFileName(player);
	if (file.has_error())
		return "";

	return AcctPath + wstos(dir) + "\\" + wstos(file.value()) + Extension;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring GetTimeString(bool localTime)
{
	SYSTEMTIME st;
	if (localTime)
		GetLocalTime(&st);
	else
		GetSystemTime(&st);

	wchar_t buf[100];
	_snwprintf_s(
		buf,
		sizeof(buf),
		L"%04d-%02d-%02d %02d:%02d:%02d ",
		st.wYear,
		st.wMonth,
		st.wDay,
		st.wHour,
		st.wMinute,
		st.wSecond);
	return buf;
}

