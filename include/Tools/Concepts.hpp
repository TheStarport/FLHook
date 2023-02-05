#pragma once
#include <string>
#include <type_traits>

template<typename T>
concept StringRestriction = std::is_same_v<std::string, T> || std::is_same_v<std::wstring, T>;

template<typename ... T>
concept AtLeastOne = (sizeof...(T) > 0);

template<typename Base, typename Derrived>
concept IsDerivedFrom = std::derived_from<Derrived, Base>;

template<typename Base, typename Derrived>
concept IsBaseOf = std::is_base_of_v<Base, Derrived>;