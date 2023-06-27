#pragma once

#include <Core/Logger.hpp>
#include <fstream>
#include <nlohmann/json.hpp>

namespace nlohmann
{
    template <>
    struct adl_serializer<std::wstring>
    {
            static void to_json(json& j, const std::wstring& str) { j = StringUtils::wstos(str); }

            static void from_json(const json& j, std::wstring& str) { str = StringUtils::stows(j.get<std::string>()); }
    };
} // namespace nlohmann

class Serializer
{
    public:
        template <typename T>
            requires RequireParamterlessConstructor<T>
        static nlohmann::json ObjToJson(T& type)
        {
            return type;
        }

        /// <summary>
        /// Save an instance of a class/struct to a JSON file.
        /// Reflectable values are int, uint, bool, float, string, Reflectable, and std::vectors of the previous types.
        /// Exceptions will be thrown for invalid file names.
        /// </summary>
        /// <typeparam name="T">The type you want to save as an JSON file.</typeparam>
        /// <param name="t">The instance of the class you would like to serialize.</param>
        /// <param name="fileToSave">Where you would like to save the file.</param>
        template <typename T>
            requires RequireParamterlessConstructor<T>
        static void SaveToJson(T& type, std::wstring fileToSave)
        {
            // If no file is provided, we can search the class metadata.
            if (fileToSave.empty())
            {
                Logger::i()->Log(LogLevel::Err, L"While trying to serialize, a file the fileName was empty.");
                throw std::invalid_argument("While trying to serialize, a file the fileName was empty.");
            }

            // Create our JSON object to write
            const auto json = ObjToJson(type);

            if (std::filesystem::path folderPath(fileToSave); folderPath.has_root_directory())
            {
                folderPath.remove_filename();
                if (!create_directories(folderPath) && !exists(folderPath))
                {
                    Logger::i()->Log(LogLevel::Warn, std::format(L"Unable to create directories for {} when serializing json.", folderPath.wstring()));
                    return;
                }
            }

            std::ofstream out(fileToSave);
            if (!out.good() || !out.is_open())
            {
                Logger::i()->Log(LogLevel::Warn, std::format(L"Unable to open {} for writing.", fileToSave));
                return;
            }

            out << json.dump(4);
            out.close();
        }

        template <typename T>
            requires RequireParamterlessConstructor<T>
        static std::unique_ptr<T> JsonToObject(const std::wstring_view json)
        {
            try
            {
                T result = nlohmann::json::parse(json);
                return std::make_unique<T>(result);
            }
            catch ([[maybe_unused]] nlohmann::json::parse_error& exc)
            {
                Logger::i()->Log(LogLevel::Err, L"Unable to process JSON. It could not be parsed. See log for more detail.");
            }
            catch ([[maybe_unused]] nlohmann::json::type_error& exc)
            {
                Logger::i()->Log(LogLevel::Err, L"Unable to process JSON. It could not be parsed. See log for more detail.");
            }
            catch ([[maybe_unused]] nlohmann::json::exception& exc)
            {
                Logger::i()->Log(LogLevel::Err, L"Unable to process JSON. It could not be parsed. See log for more detail.");
            }

            return std::make_unique<T>();
        }

        template <typename T>
            requires RequireParamterlessConstructor<T>
        static std::unique_ptr<T> LoadFromJson(std::wstring fileName, const bool createIfNotExists = true)
        {
            // If no file is provided, we can search the class metadata.
            if (fileName.empty())
            {
                std::string err = "While trying to deserialize, a file, the fileName was empty.";
                Logger::i()->Log(LogLevel::Err, StringUtils::stows(err));
                throw std::invalid_argument(err);
            }

            const bool exists = std::filesystem::exists(fileName);
            if (!exists && !createIfNotExists)
            {
                Logger::i()->Log(LogLevel::Err, std::format(L"Couldn't load JSON File ({})", fileName));
                return std::make_unique<T>();
            }

            if (!exists)
            {
                // Default constructor
                auto ret = std::make_unique<T>();
                SaveToJson(*ret, fileName);
                return ret;
            }

            std::wifstream file(fileName);
            if (!file || !file.is_open() || !file.good())
            {
                Logger::i()->Log(LogLevel::Warn, std::format(L"Unable to open JSON file {}", fileName));
                return std::make_unique<T>();
            }

            // Load data from file.
            std::wstringstream buffer;
            buffer << file.rdbuf();
            file.close();

            auto ret = JsonToObject<T>(std::wstring_view(buffer.str()));

            // If we resave the file after processing, it will trim any unrelated data, and add any missing fields
            SaveToJson(*ret, fileName);

            return ret;
        }
};
