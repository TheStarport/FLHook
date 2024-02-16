// ReSharper disable CppClangTidyClangDiagnosticReservedMacroIdentifier

#pragma once

// Disable specific clang warnings
#pragma clang diagnostic ignored "-Wdynamic-class-memaccess"
#pragma clang diagnostic ignored "-Wmicrosoft-cast"

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
#include <unordered_map>
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
#include "xbyak/xbyak.h"

#define MAGIC_ENUM_USING_ALIAS_STRING_VIEW using string_view = std::wstring_view;
#define MAGIC_ENUM_USING_ALIAS_STRING      using string = std::wstring;
#include <magic_enum.hpp>
#include <magic_enum_flags.hpp>

#include <rfl.hpp>
#include <rfl/bson.hpp>
#include <rfl/json.hpp>

#include <External/Wildcard.hpp>
#include <External/inipp.hpp>

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

#include "Core/PluginManager.hpp"

namespace Json
{
    template <typename T>
    bool Save(const T& obj, std::string_view path, const bool useSaveGameFolder = false)
    {
        std::string relativePath;
        if (useSaveGameFolder)
        {
            static std::array<char, MAX_PATH> pathArr{};
            std::fill(pathArr.begin(), pathArr.end(), '\0');

            GetUserDataPath(pathArr.data());
            relativePath += std::string(pathArr.data(), strlen(pathArr.data()));
        }

        std::string newPath = std::format("{}{}", relativePath.empty() ? "" : relativePath + "\\", path);
        std::ofstream stream(newPath);
        if (!stream.is_open())
        {
            Logger::Log(LogLevel::Warn, L"Unable to save JSON file.");
            return false;
        }

        rfl::json::write(obj, stream, rfl::json::pretty);
        Logger::Log(LogLevel::Debug, std::format(L"Successfully saved JSON file: {}", StringUtils::stows(newPath)));
        return true;
    }
} // namespace Json

#pragma warning(pop)
