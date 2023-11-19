// ReSharper disable CppClangTidyClangDiagnosticReservedMacroIdentifier

#pragma once
#pragma warning(push, 0)

#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING // NOLINT
#define _SILENCE_CXX20_CISO646_REMOVED_WARNING            // NOLINT

#include <WinSock2.h>
#include <Windows.h>
#include <array>
#include <filesystem>
#include <format>
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
#include <utility>
#include <variant>
#include <vector>

#undef SendMessage

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

#include "FLCore/FLCoreRemoteClient.h"

#include "FlPtr.hpp"

#include "Defs/Enums.hpp"
#include "Defs/Structs.hpp"

#include "Utils/Utils.hpp"

#include "API/Types/BaseId.hpp"
#include "API/Types/ClientId.hpp"
#include "API/Types/EquipmentId.hpp"
#include "API/Types/GroupId.hpp"
#include "API/Types/ObjectId.hpp"
#include "API/Types/RepGroupId.hpp"
#include "API/Types/RepId.hpp"
#include "API/Types/ShipId.hpp"

#include "API/Utils/TransformArgs.hpp"

#include "Defs/SehException.hpp"

#include "Defs/FLHookConfig.hpp"
#include "Defs/FLPacket.hpp"

#pragma comment(lib, "pcre2-8.lib")
#pragma comment(lib, "pcre2-16.lib")
#pragma comment(lib, "pcre2-32.lib")
#pragma comment(lib, "pcre2-posix.lib")

using Jp = jpcre2::select<char>;
using JpWide = jpcre2::select<wchar_t>;

#include "Core/PluginManager.hpp"

#pragma warning(pop)
