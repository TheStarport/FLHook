#pragma once
#pragma warning(push, 0)

#include <WinSock2.h>
#include <Windows.h>
#include <string>
#include <set>
#include <list>
#include <map>
#include <filesystem>
#include <variant>
#include <numbers>
#include <memory>
#include <vector>
#include <thread>
#include <functional>
#include <optional>
#include <fstream>
#include <tuple>

// TODO: Split up the following and reorganize

#include "Tools/Typedefs.hpp"
#include "Tools/Enums.hpp"
#include "Tools/Constexpr.hpp"
#include "Tools/Macros.hpp"

#include "FLCore/FLCoreCommon.h"
#include "FLCore/FLCoreServer.h"
#include "FLCore/FLCoreRemoteClient.h"
#include "FLCore/FLCoreDALib.h"

#include "Tools/Structs.hpp"
#include "Tools/Concepts.hpp"
#include "Tools/Utils.hpp"

#include <magic_enum.hpp>
#include <ext/Singleton.h>
#include <ext/Sql.hpp>
#include <ext/jpcre2.hpp>
#include <ext/result.hpp>
#include <ext/Wildcard.hpp>

#pragma comment(lib, "pcre2-8.lib")
#pragma comment(lib, "pcre2-16.lib")
#pragma comment(lib, "pcre2-32.lib")
#pragma comment(lib, "pcre2-posix.lib")

using Jp = jpcre2::select<char>;
using JpWide = jpcre2::select<wchar_t>;

#ifndef DLL
	#ifndef FLHOOK
		#define DLL __declspec(dllimport)
	#else
		#define DLL __declspec(dllexport)
	#endif
#endif

#pragma warning(pop)