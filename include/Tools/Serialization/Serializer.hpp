#ifndef SERIALIZE
#define SERIALIZE

#include <fstream>
#include <nlohmann/json.hpp>

#include "refl.hpp"

template<typename T>
constexpr auto IsEnum = std::is_enum_v<T>;
template<typename T>
constexpr auto IsBool = std::is_same_v<T, bool>;
template<typename T>
constexpr auto IsInt = std::is_same_v<T, int> || std::is_same_v<T, uint> || IsEnum<T>;
template<typename T>
constexpr auto IsBigInt = std::is_same_v<T, long long> || std::is_same_v<T, unsigned long long>;
template<typename T>
constexpr auto IsFloat = std::is_same_v<T, float>;
template<typename T>
constexpr auto IsString = std::is_same_v<T, std::string>;
template<typename T>
constexpr auto IsWString = std::is_same_v<T, std::wstring>;
template<typename T>
constexpr auto IsReflectable = std::is_base_of_v<Reflectable, T>;
template<typename T>
constexpr auto IsAnyValidType = IsBool<T> || IsInt<T> || IsBigInt<T> || IsFloat<T> || IsString<T> || IsWString<T> || IsReflectable<T>;

template<typename>
struct IsVector : std::false_type
{
};
template<typename T, typename A>
struct IsVector<std::vector<T, A>> : std::true_type
{
};
template<typename T, typename TT>
constexpr auto IsVectorOfType = std::is_same_v<T, std::vector<TT>>;

template<typename>
struct IsMap : std::false_type
{
};
template<typename T, typename A>
struct IsMap<std::map<T, A>> : std::true_type
{
};
template<typename T, typename TT, typename TTT>
constexpr auto IsMapOfType = std::is_same_v<T, std::map<TT, TTT>>;

class Serializer
{
	template<typename DeclType, typename Json = nlohmann::json, typename Member, typename ReturnVector = std::vector<typename DeclType::value_type>>
	static ReturnVector ReadVector(Json json, Member member)

	{
		if constexpr (IsBool<typename DeclType::value_type> || IsInt<typename DeclType::value_type> || IsBigInt<typename DeclType::value_type> ||
		    IsFloat<typename DeclType::value_type> || IsString<typename DeclType::value_type>)
		{
			if (json.is_array())
			{
				return json.get<std::vector<typename DeclType::value_type>>();
			}
			return json[member.name.c_str()].template get<std::vector<typename DeclType::value_type>>();
		}
		else if constexpr (IsWString<typename DeclType::value_type>)
		{
			std::vector<std::string> vectorOfString =
			    json.is_array() ? json.get<std::vector<std::string>>() : json[member.name.c_str()].template get<std::vector<std::string>>();

			std::vector<std::wstring> vectorOfWstring;
			for (auto& i : vectorOfString)
			{
				vectorOfWstring.emplace_back(stows(i));
			}
			return vectorOfWstring;
		}
		else if constexpr (IsReflectable<typename DeclType::value_type>)
		{
			auto jArr = json.is_array() ? json.get<nlohmann::json::array_t>() : json[member.name.c_str()].template get<nlohmann::json::array_t>();

			auto declArr = std::vector<typename DeclType::value_type>();
			for (auto iter = jArr.begin(); iter != jArr.end(); ++iter)
			{
				typename DeclType::value_type reflectable;
				ReadObject(*iter, reflectable);
				declArr.emplace_back(reflectable);
			}

			return declArr;
		}
		else
		{
			AssertPropertyIsReflectable(member.name.str().c_str(), typeid(DeclType::value_type).name());
		}
	}

	template<typename StrType, typename Target, typename Json, typename Ptr, typename Member>
	static void ReadMap(Json json, Ptr ptr, Member member)
	{
		if constexpr (IsReflectable<Target>)
		{
			nlohmann::json::object_t jObj = json[member.name.c_str()].template get<nlohmann::json::object_t>();
			if constexpr (IsWString<StrType>)
			{
				auto declMap = std::map<std::wstring, Target>();

				for (auto& i : jObj)
				{
					Target reflectable;
					ReadObject(i.second, reflectable);
					declMap[stows(i.first)] = reflectable;
				}
				*static_cast<std::map<std::wstring, Target>*>(ptr) = declMap;
			}
			else
			{
				auto declMap = std::map<std::string, Target>();

				for (auto& i : jObj)
				{
					Target reflectable;
					ReadObject(i.second, reflectable);
					declMap[i.first] = reflectable;
				}
				*static_cast<std::map<std::string, Target>*>(ptr) = declMap;
			}
		}
		else if constexpr (IsVector<Target>::value)
		{
			nlohmann::json::object_t jObj = json[member.name.c_str()].template get<nlohmann::json::object_t>();
			if constexpr (IsWString<StrType>)
			{
				auto declMap = std::map<std::wstring, Target>();
				for (auto& i : jObj)
				{
					declMap[stows(i.first)] = ReadVector<Target>(json, member);
				}
				*static_cast<std::map<std::wstring, Target>*>(ptr) = declMap;
			}
			else
			{
				auto declMap = std::map<std::string, Target>();
				for (auto& i : jObj)
				{
					declMap[i.first] = ReadVector<Target>(i.second, member);
				}
				*static_cast<std::map<std::string, Target>*>(ptr) = declMap;
			}
		}
		else if constexpr (IsWString<Target>)
		{
			std::map<std::string, std::string> mapOfString = json[member.name.c_str()].template get<std::map<std::string, std::string>>();
			if constexpr (IsWString<StrType>)
			{
				std::map<std::wstring, std::wstring> mapOfWstring(mapOfString.size());
				for (auto& i : mapOfString)
				{
					mapOfWstring[stows(i.first)] = stows(i.second);
				}
				*static_cast<std::map<std::wstring, std::wstring>*>(ptr) = mapOfWstring;
			}
			else
			{
				std::map<std::string, std::wstring> mapOfWstring(mapOfString.size());
				for (auto& i : mapOfString)
				{
					mapOfWstring[i.first] = stows(i.second);
				}
				*static_cast<std::map<std::string, std::wstring>*>(ptr) = mapOfWstring;
			}
		}
		// The next two handle all primitive types
		else if constexpr (IsString<StrType>)
		{
			*static_cast<std::map<std::string, Target>*>(ptr) = json[member.name.c_str()].template get<std::map<std::string, Target>>();
		}
		else if constexpr (IsWString<StrType>)
		{
			std::map<std::string, Target> sMap = json[member.name.c_str()].template get<std::map<std::string, Target>>();
			std::map<std::wstring, Target> wMap;
			for (const auto& [key, value] : sMap)
				wMap[stows(key)] = value;
			*static_cast<std::map<std::wstring, Target>*>(ptr) = wMap;
		}
		else
		{
			static_assert(IsString<StrType>, "Non string passed into HandleMapKey");
		}
	}

	template<typename DeclType, typename Json, typename Ptr, typename Member>
	static void ReadType(Json json, Ptr ptr, Member member)
	{
		if constexpr (IsBool<DeclType>)
		{
			*static_cast<bool*>(ptr) = json[member.name.c_str()].template get<bool>();
		}
		else if constexpr (IsInt<DeclType>)
		{
			*static_cast<int*>(ptr) = json[member.name.c_str()].template get<int>();
		}
		else if constexpr (IsBigInt<DeclType>)
		{
			*static_cast<long long*>(ptr) = json[member.name.c_str()].template get<long long>();
		}
		else if constexpr (IsFloat<DeclType>)
		{
			*static_cast<float*>(ptr) = json[member.name.c_str()].template get<float>();
		}
		else if constexpr (IsString<DeclType>)
		{
			*static_cast<std::string*>(ptr) = json[member.name.c_str()].template get<std::string>();
		}
		else if constexpr (IsWString<DeclType>)
		{
			*static_cast<std::wstring*>(ptr) = stows(json[member.name.c_str()].template get<std::string>());
		}
		else if constexpr (IsReflectable<DeclType>)
		{
			nlohmann::json jObj = json[member.name.c_str()].template get<nlohmann::json::object_t>();
			DeclType reflectable;
			ReadObject(jObj, reflectable);
			*static_cast<DeclType*>(ptr) = reflectable;
		}
		else if constexpr (IsVector<DeclType>::value)
		{
			*static_cast<std::vector<typename DeclType::value_type>*>(ptr) = ReadVector<DeclType>(json, member);
		}
		else if constexpr (IsMap<DeclType>::value)
		{
			ReadMap<DeclType>(json, ptr, member);
		}
	}

	template<typename T>
	static void ReadObject(nlohmann::json& json, T& obj)
	{
		constexpr auto type = refl::reflect<T>();

		refl::util::for_each(type.members, [&type, &obj, &json](auto member) {
			// We ignore static and constant values.
			// Not going to report this as there are use cases where you might want to store constant/static data with
			// the regular
			if (member.is_static || !member.is_writable)
				return;

			// Get our type with the reference removed
			typedef std::remove_reference_t<decltype(member(obj))> DeclType;

			void* ptr = PVOID(std::addressof(obj.*member.pointer));

			// Key if our json key exists. If it doesn't we don't care.
			if (!json.contains(member.name.c_str()))
			{
				// Not found
				return;
			}

			if constexpr (IsAnyValidType<DeclType>)
			{
				ReadType<DeclType>(json, ptr, member);
			}
			else if constexpr (IsVector<DeclType>::value)
			{
				*static_cast<std::vector<typename DeclType::value_type>*>(ptr) = ReadVector<DeclType>(json, member);
			}
			else if constexpr (IsMap<DeclType>::value)
			{
				typedef std::remove_const<DeclType::value_type::first_type>::type MapType;
				if constexpr (IsWString<MapType> || IsString<MapType>)
				{
					ReadMap<MapType, DeclType::value_type::second_type>(json, ptr, member);
				}
				else
				{
					static_assert(!IsWString<MapType> && !IsString<MapType>, "Key in reflectable map is not a string or wide string.");
				}
			}
			else
			{
				static_assert(!IsMap<DeclType>::value, "Non-reflectable property present on reflectable object.");
			}
		});
	}

	template<typename T>
	static void WriteObject(nlohmann::json& json, T& obj)
	{
		constexpr auto type = refl::reflect<T>();
		refl::util::for_each(type.members, [&type, &obj, &json](auto member) {
			// We ignore static and constant values.
			// Not going to report this as there are use cases where you might want to store constant/static data with
			// the regular
			if (member.is_static || !member.is_writable)
				return;

			// Get our type with the reference removed
			typedef std::remove_reference_t<decltype(member(obj))> DeclType;
			if constexpr (IsBool<DeclType> || IsInt<DeclType> || IsFloat<DeclType> || IsString<DeclType> || IsWString<DeclType> || IsBigInt<DeclType>)
			{
				if constexpr (IsWString<DeclType>)
				{
					json[member.name.c_str()] = wstos(member(obj));
				}
				else
				{
					json[member.name.c_str()] = member(obj);
				}
			}
			else if constexpr (IsReflectable<DeclType>)
			{
				auto reflectableJson = nlohmann::json::object();
				WriteObject(reflectableJson, member(obj));
				json[member.name.c_str()] = reflectableJson;
			}
			else if constexpr (IsVector<DeclType>::value)
			{
				if constexpr (IsBool<typename DeclType::value_type> || IsInt<typename DeclType::value_type> || IsFloat<typename DeclType::value_type> ||
				    IsString<typename DeclType::value_type> || IsWString<typename DeclType::value_type> || IsBigInt<typename DeclType::value_type>)
				{
					if constexpr (IsWString<typename DeclType::value_type>)
					{
						std::vector<std::string> vectorOfStrings;
						for (std::wstring& i : member(obj))
						{
							vectorOfStrings.emplace_back(wstos(i));
						}
						json[member.name.c_str()] = vectorOfStrings;
					}
					else
					{
						json[member.name.c_str()] = member(obj);
					}
				}
				else if constexpr (IsReflectable<typename DeclType::value_type>)
				{
					std::vector<nlohmann::json::object_t> objects;
					auto arr = member(obj);
					for (auto& i : arr)
					{
						auto newObj = nlohmann::json::object();
						WriteObject(newObj, i);
						objects.emplace_back(newObj);
					}
					json[member.name.c_str()] = objects;
				}
				else
				{
					static_assert(IsReflectable<typename DeclType::value_type>, "Non-reflectable property present on reflectable object.");
				}
			}
			else if constexpr (IsMap<DeclType>::value)
			{
				constexpr bool IsWide = std::is_same_v<class std::basic_string<wchar_t, struct std::char_traits<wchar_t>, class std::allocator<wchar_t>> const,
				    DeclType::value_type::first_type>;
				constexpr bool IsNotWide = std::is_same_v<class std::basic_string<char, struct std::char_traits<char>, class std::allocator<char>> const,
				    DeclType::value_type::first_type>;

				static_assert(IsWide || IsNotWide, "Non-reflectable property present on reflectable object.");

				if constexpr (IsReflectable<typename DeclType::value_type::second_type>)
				{
					auto map = member(obj);
					std::map<std::string, nlohmann::json::object_t> objects;
					for (auto& i : map)
					{
						auto newObj = nlohmann::json::object();
						WriteObject(newObj, i.second);
						if constexpr (IsWide)
						{
							objects[wstos(i.first)] = newObj;
						}
						else
						{
							objects[i.first] = newObj;
						}
					}
					json[member.name.c_str()] = objects;
				}
				else
				{
					if constexpr (IsWide)
					{
						std::map<std::string, typename DeclType::value_type::second_type> map;
						for (const auto& [key, value] : member(obj))
						{
							map[wstos(key)] = value;
						}
						json[member.name.c_str()] = map;
					}
					else
					{
						json[member.name.c_str()] = member(obj);
					}
				}
			}
			else
			{
				static_assert(!IsMap<DeclType>::value, "Non-reflectable property present on reflectable object.");
			}
		});
	}

	template<std::size_t I = 0, typename FuncT, typename... Tp>
	static typename std::enable_if<I == sizeof...(Tp), void>::type for_each(std::tuple<Tp...>, FuncT) // Unused arguments are given no names.
	{
	}

	template<std::size_t I = 0, typename FuncT, typename... Tp>
	    static typename std::enable_if < I<sizeof...(Tp), void>::type for_each(std::tuple<Tp...> t, FuncT f)
	{
		f(std::get<I>(t));
		for_each<I + 1, FuncT, Tp...>(t, f);
	}

	template<typename T>
	static void Validate(T& obj)
	{
		constexpr auto type = refl::reflect<T>();
		refl::util::for_each(type.members, [&type, &obj](auto member) {
			if (member.is_static || !member.is_writable)
				return;

			const auto attrs = refl::descriptor::get_attributes(member);

			for_each(attrs, [obj, member](const auto val) {
				typedef std::remove_reference_t<decltype(val)> DeclType;
				constexpr auto memberType = refl::reflect<DeclType>();
				constexpr auto members = refl::descriptor::get_members(memberType);
				refl::util::apply(members, [val, member, obj](auto func) { 
					cpp::result<void, std::string> valid = refl::descriptor::invoke(func, val, member(obj));
					if (valid.has_error())
					{
						Console::ConErr(std::format("While trying to create reflectable. Failed to validate {} (reason: {})", std::string(member.name.c_str()), valid.error()));
					}
				});
			});
		});
	}

  public:
	/// <summary>
	/// Save an instance of a class/struct to a JSON file.
	/// Reflectable values are int, uint, bool, float, string, Reflectable, and std::vectors of the previous types.
	/// Exceptions will be thrown for invalid file names.
	/// </summary>
	/// <typeparam name="T">The type you want to save as an JSON file.</typeparam>
	/// <param name="t">The instance of the class you would like to serialize.</param>
	/// <param name="fileToSave">Where you would like to save the file. Defaults to empty string. If empty, the class
	/// meta data will be used.</param>
	template<typename T>
	inline static void SaveToJson(T& t, std::string fileToSave = "")
	{
		// If no file is provided, we can search the class metadata.
		if (fileToSave.empty())
		{
			fileToSave = dynamic_cast<Reflectable&>(t).File();
			if (fileToSave.empty())
			{
				Console::ConErr("While trying to serialize, a file, both the metadata of the class and fileName were empty.");
				throw std::invalid_argument("While trying to serialize, a file, both the metadata of the class and fileName were empty.");
			}
		}

		// Create our JSON object to write
		auto json = nlohmann::json::object();

		WriteObject(json, t);

		if (std::filesystem::path folderPath(fileToSave); folderPath.has_root_directory())
		{
			folderPath.remove_filename();
			if (!std::filesystem::create_directories(folderPath) && !std::filesystem::exists(folderPath))
			{
				Console::ConWarn(std::format("Unable to create directories for {} when serializing json.", folderPath.string()));
				return;
			}
		}

		std::ofstream out(fileToSave);
		if (!out.good() || !out.is_open())
		{
			Console::ConWarn(std::format("Unable to open {} for writing.", fileToSave));
			return;
		}

		out << json.dump(4);
		out.close();
	}

	template<typename T>
	inline static T JsonToObject(std::string fileName = "", bool createIfNotExist = true)
	{
		// If we cannot load, we return a default
		T ret;

		// If no file is provided, we can search the class metadata.
		if (fileName.empty())
		{
			fileName = dynamic_cast<Reflectable&>(ret).File();
			if (fileName.empty())
			{
				std::string err = "While trying to deserialize, a file, both the metadata of the class and fileName were empty.";
				Console::ConErr(err);
				throw std::invalid_argument(err);
			}
		}

		bool exists = std::filesystem::exists(fileName);
		if (!exists && !createIfNotExist)
		{
			Console::ConErr(std::format("Couldn't load JSON File ({})", fileName));
			return ret;
		}

		if (!exists)
		{
			// Default constructor
			SaveToJson(ret, fileName);
			return ret;
		}

		std::ifstream file(fileName);
		if (!file || !file.is_open() || !file.good())
		{
			Console::ConWarn(std::format("Unable to open JSON file {}", fileName));
			return ret;
		}

		try
		{
			// Load data from file.
			std::stringstream buffer;
			buffer << file.rdbuf();
			file.close();
			nlohmann::json json = nlohmann::json::parse(buffer.str());

			ReadObject(json, ret);
			Validate(ret);
		}
		catch (nlohmann::json::parse_error& ex)
		{
			Console::ConErr("Unable to process JSON. It could not be parsed. See log for more detail.");
			AddLog(LogType::Normal,
			    LogLevel::Warn,
			    std::format("Unable to process JSON file [{}]. The JSON could not be parsed. EXCEPTION: {}", fileName, ex.what()));
		}
		catch (nlohmann::json::type_error& ex)
		{
			Console::ConErr("Unable to process JSON. It could not be parsed. See log for more detail.");
			AddLog(LogType::Normal,
			    LogLevel::Warn,
			    std::format("Unable to process JSON file [{}]. A type within the JSON object did not match. EXCEPTION: {}", fileName, ex.what()));
		}
		catch (nlohmann::json::exception& ex)
		{
			Console::ConErr("Unable to process JSON. It could not be parsed. See log for more detail.");
			AddLog(LogType::Normal, LogLevel::Warn, std::format("Unable to process JSON file [{}] EXCEPTION: {}", fileName, ex.what()));
		}

		// If we resave the file after processing, it will trim any unrelated data, and any missing fields
		SaveToJson(ret, fileName);

		return ret;
	}
};

#endif
