#pragma once
#include <string>

template<typename T>
concept StringRestriction = std::is_same_v<std::string, T> || std::is_same_v<std::wstring, T>;

template<typename ... T>
concept AtLeastOne = (sizeof...(T) > 0);

template<typename T>
concept IsNumeric = std::is_integral_v<T> || std::is_floating_point_v<T>;

template<typename T>
concept IsSignedIntegral = IsNumeric<T> && std::is_signed_v<T>;

template<typename T>
concept IsUnsignedIntegral = IsNumeric<T> && !IsSignedIntegral<T>;

template<typename Base, typename Derrived>
concept IsDerivedFrom = std::derived_from<Derrived, Base>;

template<typename Base, typename Derrived>
concept IsBaseOf = std::is_base_of_v<Base, Derrived>;