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
#include <random>
#include <set>
#include <string>
#include <thread>
#include <tuple>
#include <variant>
#include <vector>

#ifndef DLL
    #ifndef FLHOOK
        #define DLL __declspec(dllimport)
    #else
        #define DLL __declspec(dllexport)
    #endif
#endif

#include "Core/Templates/Constexpr.hpp"

#define MAGIC_ENUM_USING_ALIAS_STRING_VIEW using string_view = std::wstring_view;
#define MAGIC_ENUM_USING_ALIAS_STRING      using string = std::wstring;
#include <magic_enum.hpp>

#include <nlohmann/json.hpp>

#include <External/Wildcard.hpp>
#include <External/inipp.hpp>
#include <External/jpcre2.hpp>

#include "Core/Templates/Macros.hpp"

#include "FLCore/FLCoreCommon.h"
#include "FLCore/FLCoreDALib.h"
#include "FLCore/FLCoreServer.h"

#include "Defs/Enums.hpp"
#include "Defs/Structs.hpp"

#include "FLCore/FLCoreRemoteClient.h"

#include "FLHook.hpp"

#include "Core/Commands/AbstractAdminCommandProcessor.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

#include "Concepts.hpp"
#include "Utils/Utils.hpp"

#include "Core/Codec.hpp"
#include "Defs/CoreGlobals.hpp"
#include "Defs/FLHookConfig.hpp"
#include "Defs/FLPacket.hpp"

#include "Core/TempBan.hpp"
#include "Defs/ServerStats.hpp"

#include <Core/Logger.hpp>

#pragma comment(lib, "pcre2-8.lib")
#pragma comment(lib, "pcre2-16.lib")
#pragma comment(lib, "pcre2-32.lib")
#pragma comment(lib, "pcre2-posix.lib")

using Jp = jpcre2::select<char>;
using JpWide = jpcre2::select<wchar_t>;

#include "API/FLHook/Plugin.hpp"

#pragma warning(pop)
