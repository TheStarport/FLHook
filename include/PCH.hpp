// ReSharper disable CppClangTidyClangDiagnosticReservedMacroIdentifier
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

#define MAGIC_ENUM_USING_ALIAS_STRING_VIEW using string_view = std::wstring_view;
#define MAGIC_ENUM_USING_ALIAS_STRING using string = std::wstring;
#include <ext/magic_enum.hpp>

#include <nlohmann/json.hpp>

#include <ext/Singleton.h>
#include <ext/jpcre2.hpp>
#include <ext/result.hpp>
#include <ext/Wildcard.hpp>
#include <ext/inipp.hpp>

#include "Tools/Typedefs.hpp"
#include "Tools/Enums.hpp"
#include "Tools/Constexpr.hpp"
#include "Tools/Macros.hpp"

#include "FLCore/FLCoreCommon.h"
#include "FLCore/FLCoreServer.h"
#include "FLCore/FLCoreRemoteClient.h"
#include "FLCore/FLCoreDALib.h"

#include "Tools/TemplateHelpers.hpp"
#include "Tools/Structs.hpp"
#include "Tools/Concepts.hpp"
#include "Tools/Utils.hpp"

#include "Defs/CoreGlobals.hpp"
#include "Defs/FLHookConfig.hpp"
#include "Defs/WeaponEquipDefs.hpp"
#include "Defs/ShipArchDefs.hpp"
#include "Defs/FLPacket.hpp"

#include "Helpers/Admin.hpp"
#include "Helpers/Chat.hpp"
#include "Helpers/Client.hpp"
#include "Helpers/FileUtils.hpp"
#include "Helpers/FlCodec.hpp"
#include "Helpers/IniUtils.hpp"
#include "Helpers/Math.hpp"
#include "Helpers/Personalities.hpp"
#include "Helpers/Player.hpp"
#include "Helpers/Solar.hpp"
#include "Helpers/ZoneUtilities.hpp"

#include <ext/Sql.hpp>

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