#pragma once
#include <iostream>
#include <ranges>
#include <Features/Logger.hpp>

#include "Concepts.hpp"

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

	static FARPROC PatchCallAddr(char* mod, DWORD installAddress, const char* hookFunction);
};

class StringUtils
{
public:
	StringUtils() = delete;

	template<typename Ret, typename Str>
	    requires(IsStringView<Str> || StringRestriction<Str>) && (std::is_integral_v<Ret> || std::is_floating_point_v<Ret>)
	static Ret Cast(Str str)
	{
		if (!IsAscii(str))
		{
			return Ret();
		}

		Ret ret;
		std::conditional_t<std::is_same_v<std::string, Str> || std::is_same_v<std::string_view, Str>, std::string_view, std::wstring_view> input = str;
		std::from_chars(reinterpret_cast<const char*>(str.data()), reinterpret_cast<const char*>(str.data() + str.size()), ret);
		return ret; // TODO: Add trace log for failure to convert
	}

	//! Converts numeric value with a metric suffix to the full value, eg 10k translates to 10000
	static uint MultiplyUIntBySuffix(std::wstring_view valueString);
	static std::wstring XmlText(std::wstring_view text);
	
	static std::wstring stows(const std::string& text);
	static std::string wstos(const std::wstring& text);

	template<typename Str>
		requires StringRestriction<Str> || IsStringView<Str>
	static bool IsAscii(Str str)
	{
		return !std::any_of(str.begin(), str.end(), [](auto c) { return static_cast<unsigned char>(c) > 127; });
	}

	template<typename Str, typename ReturnStr = std::conditional_t<std::is_same_v<Str, std::string> || std::is_same_v<Str, std::string_view>, std::string, std::wstring>>
	    requires IsStringView<Str> || StringRestriction<Str>
	static ReturnStr ToLower(Str str)
	{
		ReturnStr retStr;
		retStr.reserve(str.size());

		// If we are a string view we need to convert it back.
		// String views use an explicit constructor
		auto before = ReturnStr(str);
		std::ranges::copy(before | std::ranges::views::transform([](auto c) { return std::tolower(c); }), std::back_inserter(retStr));
		return retStr;
	}

	template<typename Str>
	static Str Trim(const Str& stringInput)
	    requires StringRestriction<Str> || IsStringView<Str>
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

private:
	template<typename TTransformView, typename ViewType>
	static ViewType GetParam(TTransformView view, int pos)
	{
		if (pos >= 0)
		{
			throw std::invalid_argument("GetParam pos must be positive");
		}

		if (pos >= std::distance(view.begin(), view.end()))
		{
			return ViewType();
		}

		return *std::ranges::get<0>(std::ranges::subrange {std::next(view.begin(), pos), view.end()});
	}

public:
	static std::wstring_view GetParam(IsConvertibleRangeOf<std::wstring_view> auto view, int pos)
	{
		return GetParam<decltype(view), std::wstring_view>(view, pos);
	}

	static std::string_view GetParam(IsConvertibleRangeOf<std::string_view> auto view, int pos)
	{
		return GetParam<decltype(view), std::string_view>(view, pos);
	}

	template<typename TStr, typename TChar>
	static auto GetParams(TStr line, TChar splitChar)
	{
		return line | std::ranges::views::split(splitChar) |
		    std::ranges::views::transform([](auto&& rng) { return TStr(&*rng.begin(), std::ranges::distance(rng)); });
	}

	template<typename TTransformView, typename TViewType = typename first_template_type<typename first_template_type<TTransformView>::type_t>::type_t>
	static TViewType GetParamToEnd(TTransformView view, uint pos)
	{
		if (pos == 0)
		{
			return TViewType();
		}

		// If specified pos is over the max amount of items return an empty string
		if (static_cast<int>(pos) >= std::distance(view.begin(), view.end()))
		{
			return TViewType();
		}

		auto offset = view.begin();
		std::advance(offset, pos);

		auto newRange = std::ranges::views::counted(offset, std::distance(offset, view.end()));
		auto finalRange = newRange | std::ranges::views::take(std::distance(offset, view.end())) | std::ranges::views::join;
		return TViewType(&*finalRange.begin(), std::ranges::distance(finalRange) + 1);
	}

	template<typename TString, typename TChar>
	static TString GetParamToEnd(TString line, TChar splitChar, uint pos)
	{
		if (pos == 0)
		{
			return line;
		}

		auto params = GetParams(line, splitChar);
		return GetParamToEnd(params, pos);
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
