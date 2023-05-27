#include "PCH.hpp"

int StringUtils::ToInt(const std::wstring& str)
{
	return wcstol(str.c_str(), nullptr, 10);
}

int64 StringUtils::ToInt64(const std::wstring& str)
{
	return str.empty() ? 0 : wcstoll(str.c_str(), nullptr, 10);
}

uint StringUtils::ToUInt(const std::wstring& str)
{
	if (str.find(L'-') != std::wstring::npos)
	{
		return 0;
	}
	return wcstoul(str.c_str(), nullptr, 10);
}

uint StringUtils::MultiplyUIntBySuffix(const std::wstring& valueString)
{
	const uint value = wcstoul(valueString.c_str(), nullptr, 10);
	const auto lastChar = valueString.back();
	if (lastChar == *L"k" || lastChar == *L"K")
	{
		return value * 1000;
	}
	if (lastChar == *L"m" || lastChar == *L"M")
	{
		return value * 1000000;
	}
	return value;
}

std::wstring StringUtils::XmlText(const std::wstring& text)
{
	std::wstring ret;
	for (uint i = 0; (i < text.length()); i++)
	{
		if (text[i] == '<')
			ret.append(L"&#60;");
		else if (text[i] == '>')
			ret.append(L"&#62;");
		else if (text[i] == '&')
			ret.append(L"&#38;");
		else
			ret.append(1, text[i]);
	}

	return ret;
}

float StringUtils::ToFloat(const std::wstring& string)
{
	return wcstof(string.c_str(), nullptr);
}

std::wstring StringUtils::ToLower(std::wstring string)
{
	std::ranges::transform(string, string.begin(), towlower);
	return string;
}

std::string StringUtils::ToLower(std::string string)
{
	std::ranges::transform(string, string.begin(), tolower);
	return string;
}

std::wstring StringUtils::ViewToWString(const std::wstring& wstring)
{
	return {wstring.begin(), wstring.end()};
}

std::string StringUtils::StringUtils::ViewToString(const std::string_view& stringView)
{
	return {stringView.begin(), stringView.end()};
}

std::wstring StringUtils::StringUtils::stows(const std::string& text)
{
	const int size = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, nullptr, 0);
	const auto wideText = new wchar_t[size];
	MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, wideText, size);
	std::wstring ret = wideText;
	delete[] wideText;
	return ret;
}

std::string StringUtils::wstos(const std::wstring& text)
{
	const uint len = text.length() + 1;
	const auto buf = new char[len];
	WideCharToMultiByte(CP_ACP, 0, text.c_str(), -1, buf, len, nullptr, nullptr);
	std::string ret = buf;
	delete[] buf;
	return ret;
}
