#pragma once

inline void SwapBytes(void* ptr, uint iLen)
{
	if (iLen % 4)
		return;

	for (uint i = 0; i < iLen; i += 4)
	{
		char* ptr1 = (char*)ptr + i;
		unsigned long temp;
		memcpy(&temp, ptr1, 4);
		char* ptr2 = (char*)&temp;
		memcpy(ptr1, ptr2 + 3, 1);
		memcpy(ptr1 + 1, ptr2 + 2, 1);
		memcpy(ptr1 + 2, ptr2 + 1, 1);
		memcpy(ptr1 + 3, ptr2, 1);
	}
}

inline void WriteProcMem(void* pAddress, const void* pMem, int iSize)
{
	HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	DWORD dwOld;
	VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
	WriteProcessMemory(hProc, pAddress, pMem, iSize, 0);
	CloseHandle(hProc);
}
inline void ReadProcMem(void* pAddress, void* pMem, int iSize)
{
	HANDLE hProc = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
	DWORD dwOld;
	VirtualProtectEx(hProc, pAddress, iSize, PAGE_EXECUTE_READWRITE, &dwOld);
	ReadProcessMemory(hProc, pAddress, pMem, iSize, 0);
	CloseHandle(hProc);
}

inline int ToInt(const std::wstring& wscStr)
{
	return wcstol(wscStr.c_str(), nullptr, 10);
}

inline uint ToUInt(const std::wstring& wscStr)
{
	if (wscStr.find(L"-") != std::wstring::npos) {
		return 0;
	}
	return wcstoul(wscStr.c_str(), nullptr, 10);
}

inline std::wstring XMLText(const std::wstring& text)
{
	std::wstring wscRet;
	for (uint i = 0; (i < text.length()); i++)
	{
		if (text[i] == '<')
			wscRet.append(L"&#60;");
		else if (text[i] == '>')
			wscRet.append(L"&#62;");
		else if (text[i] == '&')
			wscRet.append(L"&#38;");
		else
			wscRet.append(1, text[i]);
	}

	return wscRet;
}

template<typename TStr, typename TChar>
inline TStr GetParam(const TStr& line, TChar wcSplitChar, uint iPos)
{
	uint i, j;

	TStr wscResult;
	for (i = 0, j = 0; (i <= iPos) && (j < line.length()); j++)
	{
		if (line[j] == wcSplitChar)
		{
			while (((j + 1) < line.length()) && (line[j + 1] == wcSplitChar))
				j++; // skip "whitechar"

			i++;
			continue;
		}

		if (i == iPos)
			wscResult += line[j];
	}

	return wscResult;
}

template<typename TString, typename TChar>
TString GetParamToEnd(const TString& wscLine, TChar wcSplitChar, uint iPos)
{
	for (uint i = 0, j = 0; (i <= iPos) && (j < wscLine.length()); j++)
	{
		if (wscLine[j] == wcSplitChar)
		{
			while (((j + 1) < wscLine.length()) && (wscLine[j + 1] == wcSplitChar))
				j++; // skip "whitechar"
			i++;
			continue;
		}
		if (i == iPos)
		{
			return wscLine.substr(j);
		}
	}

	return TString();
}

template<typename TString, typename TTStr, typename TTTStr>
TString ReplaceStr(const TString& source, const TTStr& searchForRaw, const TTTStr& replaceWithRaw)
{
	const TString searchFor = searchForRaw;
	const TString replaceWith = replaceWithRaw;

	uint lPos, sPos = 0;

	TString result = source;
	while ((lPos = static_cast<uint>(result.find(searchFor, sPos))) != -1)
	{
		result.replace(lPos, searchFor.length(), replaceWith);
		sPos = lPos + replaceWith.length();
	}

	return result;
}

template<typename T>
std::wstring ToMoneyStr(T cash)
{
	std::wstringstream ss;
	ss.imbue(std::locale(""));
	ss << std::fixed << cash;
	return ss.str();
}

inline float ToFloat(const std::wstring& wscStr)
{
	return wcstof(wscStr.c_str(), nullptr);
}

inline FARPROC PatchCallAddr(char* hMod, DWORD dwInstallAddress, char* dwHookFunction)
{
	DWORD dwRelAddr;
	ReadProcMem(hMod + dwInstallAddress + 1, &dwRelAddr, 4);

	DWORD dwOffset = (DWORD)dwHookFunction - (DWORD)(hMod + dwInstallAddress + 5);
	WriteProcMem(hMod + dwInstallAddress + 1, &dwOffset, 4);

	return (FARPROC)(hMod + dwRelAddr + dwInstallAddress + 5);
}

inline std::wstring ToLower(std::wstring wscStr)
{
	std::transform(wscStr.begin(), wscStr.end(), wscStr.begin(), towlower);
	return wscStr;
}

inline std::string ToLower(std::string scStr)
{
	std::transform(scStr.begin(), scStr.end(), scStr.begin(), tolower);
	return scStr;
}

/**
Remove leading and trailing spaces from the std::string  ~FlakCommon by Motah.
*/
template<typename Str>
Str Trim(const Str& scIn)
{
	if (scIn.empty())
		return scIn;

	using Char = typename Str::value_type;
	constexpr auto trimmable = []() constexpr
	{
		if constexpr (std::is_same_v<Char, char>)
			return " \t\n\r";
		else if constexpr (std::is_same_v<Char, wchar_t>)
			return L" \t\n\r";
	}
	();

	auto start = scIn.find_first_not_of(trimmable);
	auto end = scIn.find_last_not_of(trimmable);

	if (start == end)
		return scIn;

	return scIn.substr(start, end - start + 1);
}

inline std::wstring ViewToWString(const std::wstring& sv)
{
	return {sv.begin(), sv.end()};
}

inline std::string ViewToString(const std::string_view& sv)
{
	return {sv.begin(), sv.end()};
}

inline std::wstring stows(const std::string& scText)
{
	int iSize = MultiByteToWideChar(CP_ACP, 0, scText.c_str(), -1, 0, 0);
	wchar_t* wszText = new wchar_t[iSize];
	MultiByteToWideChar(CP_ACP, 0, scText.c_str(), -1, wszText, iSize);
	std::wstring wscRet = wszText;
	delete[] wszText;
	return wscRet;
}

inline std::string wstos(const std::wstring& text)
{
	uint iLen = (uint)text.length() + 1;
	char* szBuf = new char[iLen];
	WideCharToMultiByte(CP_ACP, 0, text.c_str(), -1, szBuf, iLen, 0, 0);
	std::string scRet = szBuf;
	delete[] szBuf;
	return scRet;
}


template<typename TStr>
auto strswa(TStr str)
{
	if constexpr (std::is_same_v<TStr, std::string>)
	{
		return stows(str);
	}
	else
	{
		return wstos(str);
	}
}