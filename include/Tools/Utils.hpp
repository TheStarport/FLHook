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

inline void Detour(unsigned char* pOFunc, void* pHkFunc, unsigned char* originalData)
{
	DWORD dwOldProtection = 0; // Create a DWORD for VirtualProtect calls to allow us to write.
	BYTE bPatch[5];            // We need to change 5 bytes and I'm going to use memcpy so this is the simplest way.
	bPatch[0] = 0xE9;          // Set the first byte of the byte array to the op code for the JMP instruction.
	VirtualProtect((void*)pOFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection); // Allow us to write to the memory we need to change
	DWORD dwRelativeAddress = (DWORD)pHkFunc - (DWORD)pOFunc - 5;               // Calculate the relative JMP address.
	memcpy(&bPatch[1], &dwRelativeAddress, 4);                                  // Copy the relative address to the byte array.
	memcpy(originalData, pOFunc, 5);
	memcpy(pOFunc, bPatch, 5);                                  // Change the first 5 bytes to the JMP instruction.
	VirtualProtect((void*)pOFunc, 5, dwOldProtection, nullptr); // Set the protection back to what it was.
}

inline void NopAddress(unsigned int address, unsigned int pSize)
{
	DWORD dwOldProtection = 0;
	VirtualProtect((void*)address, pSize, PAGE_READWRITE, &dwOldProtection);
	memset((void*)address, 0x90, pSize);
	VirtualProtect((void*)address, pSize, dwOldProtection, NULL);
}

inline void UnDetour(unsigned char* pOFunc, unsigned char* originalData)
{
	DWORD dwOldProtection = 0;                                                  // Create a DWORD for VirtualProtect calls to allow us to write.
	VirtualProtect((void*)pOFunc, 5, PAGE_EXECUTE_READWRITE, &dwOldProtection); // Allow us to write to the memory we need to change
	memcpy(pOFunc, originalData, 5);
	VirtualProtect((void*)pOFunc, 5, dwOldProtection, nullptr); // Set the protection back to what it was.
}

inline int ToInt(const std::wstring& wscStr)
{
	return wcstol(wscStr.c_str(), nullptr, 10);
}

inline int64 ToInt64(const std::wstring& str)
{
	return str.empty() ? 0 : wcstoll(str.c_str(), nullptr, 10);
}

inline uint ToUInt(const std::wstring& wscStr)
{
	if (wscStr.find(L"-") != std::wstring::npos)
	{
		return 0;
	}
	return wcstoul(wscStr.c_str(), nullptr, 10);
}

inline std::chrono::sys_time<std::chrono::seconds> UnixToSysTime(int64 time)
{
	return std::chrono::sys_time<std::chrono::seconds> {std::chrono::seconds {time}};
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
inline TStr GetParam(const TStr& line, TChar splitChar, uint pos)
    requires StringRestriction<TStr>
{
	uint i, j;

	TStr result;
	for (i = 0, j = 0; (i <= pos) && (j < line.length()); j++)
	{
		if (line[j] == splitChar)
		{
			while (((j + 1) < line.length()) && (line[j + 1] == splitChar))
				j++; // skip "whitechar"

			i++;
			continue;
		}

		if (i == pos)
			result += line[j];
	}
	return result;
}

template<typename TString, typename TChar>
TString GetParamToEnd(const TString& line, TChar splitChar, uint pos)
    requires StringRestriction<TString>
{
	for (uint i = 0, j = 0; (i <= pos) && (j < line.length()); j++)
	{
		if (line[j] == splitChar)
		{
			while (((j + 1) < line.length()) && (line[j + 1] == splitChar))
				j++; // skip "whitechar"
			i++;
			continue;
		}
		if (i == pos)
		{
			return line.substr(j);
		}
	}

	return TString();
}

template<typename TString>
auto Split(const TString& input, const TString& splitCharacter)
    requires StringRestriction<TString>
{
	auto inputCopy = input;
	size_t pos = 0;
	std::vector<TString> tokens;
	while ((pos = inputCopy.find(splitCharacter)) != TString::npos)
	{
		TString token = inputCopy.substr(0, pos);
		tokens.emplace_back(token);
		inputCopy.erase(0, pos + splitCharacter.length());
	}

	if (!inputCopy.empty() && inputCopy.size() != input.size())
	{
		tokens.emplace_back(inputCopy);
	}
	return tokens;
}

template<typename TString, typename TChar>
auto Split(const TString& input, const TChar& splitCharacter)
    requires StringRestriction<TString>
{
	return Split(input, TString(1, splitCharacter));
}

template<typename TString, typename TTStr, typename TTTStr>
TString ReplaceStr(const TString& source, const TTStr& searchForRaw, const TTTStr& replaceWithRaw)
    requires StringRestriction<TString>
{
	const TString searchFor = searchForRaw;
	const TString replaceWith = replaceWithRaw;

	uint lPos, sPos = 0;

	TString result = source;
	while ((lPos = static_cast<uint>(result.find(searchFor, sPos))) != UINT_MAX)
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

inline float ToFloat(const std::wstring& string)
{
	return wcstof(string.c_str(), nullptr);
}

inline FARPROC PatchCallAddr(char* mod, DWORD installAddress, char* hookFunction)
{
	DWORD dwRelAddr;
	ReadProcMem(mod + installAddress + 1, &dwRelAddr, 4);

	DWORD dwOffset = (DWORD)hookFunction - (DWORD)(mod + installAddress + 5);
	WriteProcMem(mod + installAddress + 1, &dwOffset, 4);

	return (FARPROC)(mod + dwRelAddr + installAddress + 5);
}

inline std::wstring ToLower(std::wstring string)
{
	std::transform(string.begin(), string.end(), string.begin(), towlower);
	return string;
}

inline std::string ToLower(std::string string)
{
	std::transform(string.begin(), string.end(), string.begin(), tolower);
	return string;
}

/**
Remove leading and trailing spaces from the std::string  ~FlakCommon by Motah.
*/
template<typename Str>
Str Trim(const Str& stringInput)
    requires StringRestriction<Str>
{
	if (stringInput.empty())
		return stringInput;

	using Char = typename Str::value_type;
	constexpr auto trimmable = []() constexpr {
		if constexpr (std::is_same_v<Char, char>)
			return " \t\n\r";
		else if constexpr (std::is_same_v<Char, wchar_t>)
			return L" \t\n\r";
	}();

	auto start = stringInput.find_first_not_of(trimmable);
	auto end = stringInput.find_last_not_of(trimmable);

	if (start == end)
		return stringInput;

	return stringInput.substr(start, end - start + 1);
}

inline std::wstring ViewToWString(const std::wstring& wstring)
{
	return {wstring.begin(), wstring.end()};
}

inline std::string ViewToString(const std::string_view& stringView)
{
	return {stringView.begin(), stringView.end()};
}

inline std::wstring stows(const std::string& text)
{
	int size = MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, 0, 0);
	wchar_t* wideText = new wchar_t[size];
	MultiByteToWideChar(CP_ACP, 0, text.c_str(), -1, wideText, size);
	std::wstring wscRet = wideText;
	delete[] wideText;
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
    requires StringRestriction<TStr>
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