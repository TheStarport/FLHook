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

#include "Core/Templates/Constexpr.hpp"
#include "Core/Templates/Typedefs.hpp"

#define MAGIC_ENUM_USING_ALIAS_STRING_VIEW using string_view = std::wstring_view;
#define MAGIC_ENUM_USING_ALIAS_STRING      using string = std::wstring;
#include <External/magic_enum.hpp>

#include <nlohmann/json.hpp>

#include <External/Singleton.h>
#include <External/Wildcard.hpp>
#include <External/inipp.hpp>
#include <External/jpcre2.hpp>
#include <External/result.hpp>

#include "Core/Templates/Macros.hpp"

#include "FLCore/FLCoreCommon.h"
#include "FLCore/FLCoreDALib.h"
#include "FLCore/FLCoreServer.h"

#include "Defs/Enums.hpp"
#include "Defs/Structs.hpp"

#include "Core/Templates/Concepts.hpp"
#include "Core/Templates/TemplateHelpers.hpp"

#include "Core/Action.hpp"
#include "FLCore/FLCoreRemoteClient.h"

#include "API/Utils/Utils.hpp"

#include "FLHook.hpp"

#include "Core/Commands/AbstractAdminCommandProcessor.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

#include <sqlite_orm/sqlite_orm.h>

#include "Defs/CoreGlobals.hpp"
#include "Defs/FLHookConfig.hpp"
#include "Defs/FLPacket.hpp"
#include "Defs/ShipArchDefs.hpp"
#include "Defs/WeaponEquipDefs.hpp"

#include "API/FLServer/Admin.hpp"
#include "API/FLServer/Chat.hpp"
#include "API/FLServer/Client.hpp"
#include "API/FLServer/Math.hpp"
#include "API/FLServer/Personalities.hpp"
#include "API/FLServer/Player.hpp"
#include "API/FLServer/Solar.hpp"
#include "API/FLServer/ZoneUtilities.hpp"
#include "API/Utils/FileUtils.hpp"
#include "API/Utils/IniUtils.hpp"

#include "API/FLHook/MailManager.hpp"
#include "Core/Codec.hpp"

#include "Core/Action.hpp"
#include "Core/MessageHandler.hpp"
#include "Core/TempBan.hpp"
#include "Defs/ServerStats.hpp"

#include <Core/Logger.hpp>

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

#include "API/FLHook/Plugin.hpp"

#pragma warning(pop)
