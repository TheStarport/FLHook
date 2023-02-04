#pragma once
#include <string>
#include <type_traits>

template<typename T>
concept StringRestriction = std::is_same_v<std::string, T> || std::is_same_v<std::wstring, T>;

template<typename ... T>
concept AtLeastOne = (sizeof...(T) > 0);