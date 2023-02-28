#pragma once

#ifdef SERVER
	#include <ext/Sql.hpp>
	#pragma warning(push, 0)
	#include <ext/jpcre2.hpp>
	#pragma warning(pop)
using jp = jpcre2::select<char>;
using jpWide = jpcre2::select<wchar_t>;
	#include <ext/result.hpp>
	#include <ext/Wildcard.hpp>
	#include "magic_enum.hpp"

	#pragma comment(lib, "pcre2-8.lib")
	#pragma comment(lib, "pcre2-16.lib")
	#pragma comment(lib, "pcre2-32.lib")
	#pragma comment(lib, "pcre2-posix.lib")
#else
	#ifdef FMT
		#include "fmt/core.h"
		#include "fmt/format.h"
		#include "fmt/xchar.h"
	#endif

	#ifdef SQL
		#include <ext/Sql.hpp>
	#endif

	#ifdef PCRE2
		#pragma warning(push, 0)
		#include <ext/jpcre2.hpp>
		#pragma warning(pop)
using jp = jpcre2::select<char>;
using jpWide = jpcre2::select<wchar_t>;
		#pragma comment(lib, "pcre2-8.lib")
		#pragma comment(lib, "pcre2-16.lib")
		#pragma comment(lib, "pcre2-32.lib")
		#pragma comment(lib, "pcre2-posix.lib")
	#endif

	#ifdef RESULT
		#include <ext/result.hpp>
	#endif

	#ifdef WILDCARD
		#include <ext/Wildcard.hpp>
	#endif

	#ifdef ENUM
		#include "magic_enum.hpp"
	#endif

	#ifdef JSON
		#include <nlohmann/json.hpp>
	#endif

	#ifdef REFL
		#include "refl.hpp"
	#endif

#endif