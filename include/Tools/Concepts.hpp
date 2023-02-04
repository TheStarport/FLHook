#pragma once
#include <string>
#include <type_traits>

template <typename T>
concept StringRestriction = std::is_same<std::string, T>::value || std::is_same<std::wstring, T>::value;