// ReSharper disable CppClangTidyClangDiagnosticReservedMacroIdentifier
#pragma once
#pragma warning(push, 0)

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
#define _SILENCE_CXX20_CISO646_REMOVED_WARNING

#include <WinSock2.h>
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <numbers>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <variant>
#include <vector>

#define MAGIC_ENUM_USING_ALIAS_STRING_VIEW using string_view = std::wstring_view;
#define MAGIC_ENUM_USING_ALIAS_STRING      using string = std::wstring;
#include <ext/magic_enum.hpp>

#include <nlohmann/json.hpp>

#include <ext/Singleton.h>
#include <ext/Wildcard.hpp>
#include <ext/inipp.hpp>
#include <ext/jpcre2.hpp>
#include <ext/result.hpp>

#include "Tools/Constexpr.hpp"
#include "Tools/Typedefs.hpp"

#include "Tools/Enums.hpp"
#include "Tools/Macros.hpp"

#include "Tools/Concepts.hpp"
#include "Tools/TemplateHelpers.hpp"

#include "Exceptions/Action.hpp"

#include "FLCore/FLCoreCommon.h"
#include "FLCore/FLCoreDALib.h"
#include "FLCore/FLCoreServer.h"

#include "FLCore/FLCoreRemoteClient.h"

#include "Tools/Structs.hpp"
#include "Tools/Utils.hpp"

#include "FLHook.hpp"

#include <sqlite_orm/sqlite_orm.h>

#include "Defs/CoreGlobals.hpp"
#include "Defs/FLHookConfig.hpp"
#include "Defs/FLPacket.hpp"
#include "Defs/ShipArchDefs.hpp"
#include "Defs/WeaponEquipDefs.hpp"

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

#include <Features/Logger.hpp>

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
