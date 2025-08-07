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
#include <chrono>
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
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

using namespace std::chrono_literals;

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

#include <concurrencpp/concurrencpp.h>

#include "API/Utils/Action.hpp"
#include "Core/Templates/Macros.hpp"
#include "Defs/Enums.hpp"
#include "Utils/Utils.hpp"

#include "API/Types/Id.hpp"
#include "API/Types/ClientId.hpp"
#include "API/Types/SystemId.hpp"
#include "API/Types/GoodId.hpp"
#include "API/Types/GroupId.hpp"
#include "API/Types/RepGroupId.hpp"
#include "API/Types/RepId.hpp"
#include "API/Types/ShipId.hpp"
#include "API/Types/BaseId.hpp"
#include "API/Types/ObjectId.hpp"
#include "API/Types/EquipmentId.hpp"
#include "API/Types/CharacterId.hpp"

#include "API/Utils/Reflection.hpp" // For reflectiong custom types with rfl
#include "API/Utils/TransformArgs.hpp"

#include "API/Utils/Logger.hpp"

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
            WARN("Unable to save JSON file.");
            return false;
        }

        rfl::json::write(obj, stream, rfl::json::pretty);
        DEBUG("Successfully saved JSON file {{path}}", { "path", newPath });

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
            WARN("Unable to read JSON file {{path}}", { "path", newPath });

            return { LoadState::UnableToRead, std::nullopt };
        }

        auto result = rfl::json::read<T, rfl::DefaultIfMissing>(stream);
        if (!result)
        {
            ERROR("Error while trying to serialize {{path}} {{error}}", { "path", path }, { "error", result.error().what() });

            return { LoadState::FailedToValidate, std::nullopt };
        }

        return { LoadState::Success, result.value() };
    }
} // namespace Json

#define LoadJsonWithValidation(configCls, config, path)                                             \
    if (const auto conf = Json::Load<configCls>(path); conf.first == Json::LoadState::DoesNotExist) \
    {                                                                                               \
        Json::Save(config, path);                                                                   \
    }                                                                                               \
    else if (conf.first == Json::LoadState::Success)                                                \
    {                                                                                               \
        config = conf.second.value();                                                               \
    }                                                                                               \
    else                                                                                            \
    {                                                                                               \
        return false;                                                                               \
    }

#define LoadJsonWithPrompt(configCls, config, path)                                                                                                     \
    if (const auto conf = Json::Load<configCls>(path); conf.first == Json::LoadState::DoesNotExist)                                                     \
    {                                                                                                                                                   \
        Json::Save(config, path);                                                                                                                       \
    }                                                                                                                                                   \
    else if (conf.first == Json::LoadState::Success)                                                                                                    \
    {                                                                                                                                                   \
        config = conf.second.value();                                                                                                                   \
    }                                                                                                                                                   \
    else if (conf.first == Json::LoadState::FailedToValidate)                                                                                           \
    {                                                                                                                                                   \
        if (MessageBoxA(nullptr,                                                                                                                        \
                        std::format("Error trying to read file: {}\nThis file was trying to be serialized to {} but could not be. See the console for " \
                                    "details on why serialization failed.\n\nShould this file be deleted and a fresh copy be generated?\n"              \
                                    "Press 'OK' to regenerate, 'Cancel' to stop processing.",                                                           \
                                    path,                                                                                                               \
                                    typeid(configCls).name())                                                                                           \
                            .c_str(),                                                                                                                   \
                        "Failed to read JSON",                                                                                                          \
                        MB_OKCANCEL) == IDOK)                                                                                                           \
        {                                                                                                                                               \
            Json::Save(config, path);                                                                                                                   \
        }                                                                                                                                               \
        else                                                                                                                                            \
        {                                                                                                                                               \
            return false;                                                                                                                               \
        }                                                                                                                                               \
    }                                                                                                                                                   \
    else                                                                                                                                                \
    {                                                                                                                                                   \
        return false;                                                                                                                                   \
    }

#pragma warning(pop)
