// ReSharper disable CppClangTidyClangDiagnosticReservedMacroIdentifier

#pragma once

// Disable specific clang warnings
#pragma clang diagnostic ignored "-Wdynamic-class-memaccess"
#pragma clang diagnostic ignored "-Wmicrosoft-cast"

#pragma warning(push, 0)

#define _SILENCE_CXX20_CISO646_REMOVED_WARNING // NOLINT

#include <WinSock2.h>
#include <Windows.h>
#include <any>
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

// Due to a bug in the wildcard library, it falsly assumes clang as a requirement for certain features
// Without these, it fails to compile when using certain patterns or wstrings. For this reason, we fool it into working.
#define __clang__ // NOLINT(*-reserved-identifier)
#include <wildcards.hpp.in>
#undef __clang__

#include "API/Utils/Action.hpp"
#include "Core/Templates/Macros.hpp"
#include "Defs/Enums.hpp"
#include "Utils/Utils.hpp"

#include "API/Types/BaseId.hpp"
#include "API/Types/ClientId.hpp"
#include "API/Types/EquipmentId.hpp"
#include "API/Types/GoodId.hpp"
#include "API/Types/GroupId.hpp"
#include "API/Types/ObjectId.hpp"
#include "API/Types/RepGroupId.hpp"
#include "API/Types/RepId.hpp"
#include "API/Types/ShipId.hpp"

#include "API/Types/Id.hpp"
#include "API/Utils/Reflection.hpp" // For reflectiong custom types with rfl
#include "FLCore/Common.hpp"
#include "FLCore/FLCoreDALib.h"
#include "FLCore/FLCoreServer.h"

#include "FLCore/FLCoreRemoteClient.h"

#include "API/Utils/TransformArgs.hpp"

#include "Defs/SehException.hpp"

#include "Defs/FLHookConfig.hpp"

#include "Core/PluginManager.hpp"

namespace Json
{
    enum class LoadState
    {
        DoesNotExist,
        FailedToValidate,
        UnableToRead,
        Success
    };

    template <typename T>
    bool Save(const T& obj, std::string_view path, const bool useSaveGameFolder = false)
    {
        std::string relativePath;
        if (useSaveGameFolder)
        {
            static std::array<char, MAX_PATH> pathArr{};
            std::ranges::fill(pathArr, '\0');

            GetUserDataPath(pathArr.data());
            relativePath += std::string(pathArr.data(), strlen(pathArr.data()));
        }

        const std::string newPath = std::format("{}{}", relativePath.empty() ? "" : relativePath + "\\", path);
        std::ofstream stream(newPath);
        if (!stream.is_open())
        {
            Logger::Warn(L"Unable to save JSON file.");
            return false;
        }

        rfl::json::write(obj, stream, rfl::json::pretty);
        Logger::Debug(std::format(L"Successfully saved JSON file: {}", StringUtils::stows(newPath)));
        return true;
    }

    /**
     * Opens the specified path and attempts to construct an object of the specified type from it.
     * @tparam T The object type to serialize to/from JSON
     * @param path The relative path for the JSON file
     * @param createIfNotExist If true, the file will be created if it is not found when attempting to read it.
     * @param useSaveGameFolder If true, the path provided will be relative to the save game folder, instead of the current working directory.
     * @return An std::optional of type T. If reading fails for any reason, then a std::nullopt shall be returned.
     */
    template <typename T>
        requires std::is_default_constructible_v<T>
    std::pair<LoadState, std::optional<T>> Load(std::string_view path, const bool createIfNotExist = true, const bool useSaveGameFolder = false)
    {
        std::string relativePath;
        if (useSaveGameFolder)
        {
            static std::array<char, MAX_PATH> pathArr{};
            std::ranges::fill(pathArr, '\0');

            GetUserDataPath(pathArr.data());
            relativePath += std::string(pathArr.data(), strlen(pathArr.data()));
        }

        const std::string newPath = std::format("{}{}", relativePath.empty() ? "" : relativePath + "\\", path);

        if (!std::filesystem::exists(newPath))
        {
            if (createIfNotExist)
            {
                std::optional<T> obj{ T{} };
                Save(obj, path, useSaveGameFolder);
                return { LoadState::Success, obj };
            }

            return { LoadState::DoesNotExist, std::nullopt };
        }

        std::ifstream stream(newPath);
        if (!stream.is_open())
        {
            Logger::Warn(std::format(L"Unable to read JSON file: {}", StringUtils::stows(newPath)));
            return { LoadState::UnableToRead, std::nullopt };
        }

        auto result = rfl::json::read<T>(stream);
        if (auto err = result.error(); err.has_value())
        {
            Logger::Err(std::format(L"Error while trying to serialize '{}': {}", StringUtils::stows(path), StringUtils::stows(err.value().what())));
            return { LoadState::FailedToValidate, std::nullopt };
        }

        return { LoadState::Success, result.value() };
    }
} // namespace Json

#define LoadJsonWithValidation(confgCls, config, path)                                             \
    if (const auto conf = Json::Load<confgCls>(path); conf.first == Json::LoadState::DoesNotExist) \
    {                                                                                              \
        Json::Save(config, path);                                                                  \
    }                                                                                              \
    else if (conf.first == Json::LoadState::Success)                                               \
    {                                                                                              \
        config = conf.second.value();                                                              \
    }                                                                                              \
    else                                                                                           \
    {                                                                                              \
        return false;                                                                              \
    }

#pragma warning(pop)
