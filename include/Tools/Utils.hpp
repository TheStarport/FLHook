#pragma once
#include <ranges>

class TimeUtils
{
  public:
	TimeUtils() = delete;

	DLL static std::chrono::sys_time<std::chrono::seconds> UnixToSysTime(int64 time);
	DLL static uint UnixSeconds();
	DLL static uint64 UnixMilliseconds();
	DLL static std::string HumanReadableTime(std::chrono::seconds dur);

	template<typename T>
	static std::chrono::microseconds ToMicroseconds(T duration) { return std::chrono::duration_cast<std::chrono::microseconds>(duration); }

	template<typename T> 
	static std::chrono::milliseconds ToMilliseconds(T duration) { return std::chrono::duration_cast<std::chrono::milliseconds>(duration); }

	template<typename T>
	static std::chrono::seconds ToSeconds(T duration) { return std::chrono::duration_cast<std::chrono::seconds>(duration); }

	template<typename T>
	static std::chrono::minutes ToMinutes(T duration) { return std::chrono::duration_cast<std::chrono::minutes>(duration); }

	template<typename T>
	static std::chrono::hours ToHours(T duration) { return std::chrono::duration_cast<std::chrono::hours>(duration); }

	template<typename T>
	static std::chrono::nanoseconds ToNanoseconds(T duration) { return std::chrono::duration_cast<std::chrono::nanoseconds>(duration); }
};

class MemUtils
{
public:
	MemUtils() = delete;

	static HWND GetFlServerHwnd();
	static void SwapBytes(void* ptr, uint len);
	static void WriteProcMem(void* address, const void* mem, uint size);
	static void ReadProcMem(void* address, void* mem, uint size);

	FARPROC PatchCallAddr(char* mod, DWORD installAddress, const char* hookFunction);
};

class StringUtils
{
public:
	StringUtils() = delete;

	static int ToInt(const std::wstring& str);
	static int64 ToInt64(const std::wstring& str);
	static uint ToUInt(const std::wstring& str);

	//! Converts numeric value with a metric suffix to the full value, eg 10k translates to 10000
	static uint MultiplyUIntBySuffix(const std::wstring& valueString);
	static std::wstring XmlText(const std::wstring& text);

	static float ToFloat(const std::wstring& string);
	static std::wstring ToLower(std::wstring string);
	static std::string ToLower(std::string string);
	static std::wstring ViewToWString(const std::wstring& wstring);
	static std::string ViewToString(const std::string_view& stringView);
	static std::wstring stows(const std::string& text);
	static std::string wstos(const std::wstring& text);

	template<typename Str>
	static Str Trim(const Str& stringInput)
	    requires StringRestriction<Str> || IsView<Str>
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

	template<typename TString>
	static TString ExpandEnvironmentVariables(const TString& input)
	{
		std::string accumulator = "";
		std::string output = "";
		bool percentFound = false;

		for (uint i = 0; i < input.length(); i++)
		{
			const auto ch = input[i];
			if (ch == '%')
			{
				if (percentFound || (input[i + 1] != '%'))
				{
					percentFound = !percentFound;
					if (percentFound)
						accumulator.clear();
					else
					{
						auto var = std::getenv(accumulator.c_str());
						accumulator = var ? var : accumulator;
						output += accumulator;
					}
				}
				else
				{
					i++; // Extra percentage sign, escape it.
				}
			}
			else
			{
				if (percentFound)
					accumulator += ch;
				else
					output += ch;
			}
		}

		TString ret = Trim(output);
		return ret;
	}

	template<typename TStr, typename TChar>
	static auto GetParams(const TStr& line, TChar splitChar)
	    requires StringRestriction<TStr> || IsView<TStr>
	{
		return line
			| std::ranges::views::split(splitChar)
			| std::ranges::views::transform([](auto&& rng) { return std::string_view(&*rng.begin(), std::ranges::distance(rng)); });
	}

	template<typename TString, typename TChar>
	static TString GetParamToEnd(const TString& line, TChar splitChar, uint pos)
	    requires StringRestriction<TString> || IsView<TString>
	{
		auto params = GetParams(line, splitChar);
		
	}

	template<typename TString, typename TTStr, typename TTTStr>
	static TString ReplaceStr(const TString& source, const TTStr& searchForRaw, const TTTStr& replaceWithRaw)
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
	static std::wstring ToMoneyStr(T cash)
	{
		std::wstringstream ss;
		ss.imbue(std::locale(""));
		ss << std::fixed << cash;
		return ss.str();
	}

	template<typename TStr>
	static auto strswa(TStr str)
	    requires StringRestriction<TStr>
	{
		if constexpr (std::is_same_v<TStr, std::string>)
		{
			return stows(str);
		}
		else
		{
			return StringUtils::wstos(str);
		}
	}
};
