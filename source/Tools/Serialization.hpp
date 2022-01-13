#ifndef SERIALIZE
#define SERIALIZE

#include <fstream>
#include <nlohmann/json.hpp>

#include "refl.hpp"

// A base class used for denoting that a class can be scanned.
// Reflectable values are int, uint, bool, float, string, Reflectable, and std::vectors of the previous types.
// Reflectables are interepreted as headers of the provided name.
// Circular references are not handled and will crash.
class Reflectable {};
template <typename T> constexpr auto IsBool = std::is_same_v<T, bool>;
template <typename T> constexpr auto IsInt = std::is_same_v<T, int> || std::is_same_v<T, uint>;
template <typename T> constexpr auto IsFloat = std::is_same_v<T, float>;
template <typename T> constexpr auto IsString = std::is_same_v<T, std::string> || std::is_same_v<T, std::wstring>;
template <typename T> constexpr auto IsReflectable = std::is_base_of_v<Reflectable, T>;
template <typename> struct IsVector : std::false_type {};
template <typename T, typename A> struct IsVector<std::vector<T, A>> : std::true_type {};
template <typename T, typename TT>
constexpr auto IsVectorOfType = std::is_same_v<T, std::vector<TT>>;

class Serializer {

    template <typename T>
    static void ReadObject(nlohmann::json &json, T& obj) {
        constexpr auto type = refl::reflect<T>();

        refl::util::for_each(type.members, [&type, &obj, &json](auto member) {
            // Get our type with the reference removeds
            typedef std::remove_reference_t<decltype(member(obj))> DeclType;

            void* ptr = PVOID(std::addressof(obj.*member.pointer));

            // Ensure someone isn't sending us a wide string.
            // Rather than ignoring it, lets give them an error.
            static_assert(!(std::is_same_v<std::remove_reference_t<decltype(member(obj))>, std::wstring>), "Wide strings are not supported for serialization.");

            // Key if our json key exists. If it doesn't we don't care.
            if (!json.contains(member.name.c_str())) {
                // Not found
                return;
            }

            if constexpr (IsBool<DeclType>) {
                *static_cast<bool *>(ptr) = json[member.name.c_str()].template get<bool>();
            } else if constexpr (IsInt<DeclType>) {
                *static_cast<int *>(ptr) = json[member.name.c_str()].template get<int>();
            } else if constexpr (IsFloat<DeclType>) {
                *static_cast<float *>(ptr) = json[member.name.c_str()].template get<float>();
            } else if constexpr (IsString<DeclType>) {
                *static_cast<std::string *>(ptr) = json[member.name.c_str()].template get<std::string>();
            } else if constexpr (IsReflectable<DeclType>) {

            } else if constexpr (IsVector<DeclType>::value) {
                if constexpr (IsBool<typename DeclType::value_type>) {
                    *static_cast<std::vector<bool>*>(ptr) = json[member.name.c_str()].template get<std::vector<bool>>();
                } else if constexpr (IsInt<typename DeclType::value_type>) {
                    *static_cast<std::vector<int> *>(ptr) = json[member.name.c_str()].template get<std::vector<int>>();
                } else if constexpr (IsFloat<typename DeclType::value_type>) {
                    *static_cast<std::vector<float> *>(ptr) = json[member.name.c_str()].template get<std::vector<float>>();
                } else if constexpr (IsString<typename DeclType::value_type>) {
                    *static_cast<std::vector<std::string> *>(ptr) = json[member.name.c_str()].template get<std::vector<std::string>>();
                } else if constexpr (IsReflectable<typename DeclType::value_type>) {

                } else {
                    if (set_bDebug)
                        Console::ConWarn(L"Non-reflectable property (%s) present within vector on %s.", member.name, type.name.c_str());
                }
            } else {
                if (set_bDebug)
                    Console::ConWarn(L"Non-reflectable property (%s) present on %s.", member.name, type.name.c_str());
            }
        });
    }

    template <typename T>
    static void WriteObject(std::ostringstream& ss, T& obj) {
        
    }

  public:
    /// <summary>
    /// Save an instance of a class/struct to an INI file.
    /// Only base types (int, string, bool, float) will be parsed.
    /// </summary>
    /// <typeparam name="T">The type you want to save as an INI file. The section name will be the class.</typeparam>
    /// <param name="t">The instance of the class you would like to convert.</param>
    template <typename T>
    inline static void SaveToIni(T &t) {
        static_assert(IsReflectable<T>, "Only classes that inherit from 'Reflectable' can be serialized.");

        constexpr auto type = refl::reflect<T>();
        refl::util::for_each(type.members, [&type, &t](auto member) {
            // We ignore static and constant values.
            // Not going to report this as there are use cases where you might want to store constant/static data with the regular
            if (member.is_static || !member.is_writable)
                return;

            // Get our type with the reference removeds
            typedef std::remove_reference_t<decltype(member(t))> DeclType;

            // Ensure someone isn't sending us a wide string.
            // Rather than ignoring it, lets give them an error.
            static_assert(!(std::is_same_v<std::remove_reference_t<decltype(member(t))>, std::wstring>), "Wide strings are not supported for serialization.");

            if (IsBool<DeclType>) {

            } else if (IsInt<DeclType>) {

            } else if (IsFloat<DeclType>) {

            } else if (IsString<DeclType>) {

            } else if (IsReflectable<DeclType>) {

            } else {
                if (set_bDebug)
                    Console::ConWarn(L"Non-reflectable property (%s) present on %s.", member.name, type.name.c_str());
            }
        });
    }

    template <typename T>
    inline static T IniToObject(const std::string fileName, bool createIfNotExist = true) {
        static_assert(IsReflectable<T>, "Only classes that inherit from 'Reflectable' can be serialized.");

        bool exists = std::filesystem::exists(fileName);
        if (!exists && !createIfNotExist) {
            Console::ConErr(L"Couldn't load INI File (%s)", fileName.c_str());
            return T();
        }

        if (!exists) {
            // Default constructor
            /*auto obj = std::make_shared<T>();
            SaveToIni(*obj);
            return obj;*/
        }


        std::ifstream file(fileName);
        if (!file || !file.is_open() || !file.good()) {
            Console::ConWarn(L"Unable to open JSON file %s", fileName.c_str());
            return T();
        }

        // Load data from file.
        std::stringstream buffer;
        buffer << file.rdbuf();
        file.close();
        nlohmann::json json = nlohmann::json::parse(buffer.str());

        //auto obj = std::make_shared<T>();
        T t;
        ReadObject(json, t);

        return t;
    }
};

#endif