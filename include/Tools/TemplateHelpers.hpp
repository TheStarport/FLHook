// ReSharper disable CppClangTidyClangDiagnosticReturnStackAddress
#pragma once

#include "Utils.hpp"

// Default transform arg - assume is integral or floating point and straight cast it
template <class T>
T TransformArg(std::wstring_view s)
{
    return StringUtils::Cast<T>(s);
}

template <>
inline std::wstring_view TransformArg(std::wstring_view s)
{
    return s;
}

template <>
inline const std::wstring& TransformArg(std::wstring_view s)
{
    return std::wstring(s);
}

template <>
inline bool TransformArg(std::wstring_view s)
{
    const auto lower = StringUtils::ToLower(s);
    return lower == L"true" || lower == L"yes" || lower == L"1";
}

template <>
inline const std::vector<std::wstring_view>& TransformArg(std::wstring_view s)
{
    std::vector<std::wstring_view> views;
    auto params = StringUtils::GetParams(s, L' ');
    for (const auto& i : params)
    {
        views.emplace_back(i);
    }

    return views;
}

template <typename... Args, std::size_t... Is>
auto CreateTupleImpl(std::index_sequence<Is...>, const std::vector<std::wstring>& arguments)
{
    return std::make_tuple(TransformArg<Args>(arguments[Is])...);
}

template <typename... Args>
auto CreateTuple(const std::vector<std::wstring>& arguments)
{
    return CreateTupleImpl<Args...>(std::index_sequence_for<Args...>{}, arguments);
}

template <typename F, F f>
class ClassFunctionWrapper;

template <class Ret, class Cl, class... Args, Ret (Cl::*func)(Args...)>
class ClassFunctionWrapper<Ret (Cl::*)(Args...), func>
{
    public:
        static Ret ProcessParam(Cl cl, const std::vector<std::wstring>& params)
        {
            auto arg = CreateTuple<Args...>(params);
            auto lambda = std::function<Ret(Args...)>{ [=](Args... args) mutable { return (cl.*func)(args...); } };
            return std::apply(lambda, arg);
        }
};

template <typename T>
struct first_template_type;

template <template <typename T, typename...> class t, typename T, typename... Args>
struct first_template_type<t<T, Args...>>
{
        typedef T Type;
};

template <typename T>
using FirstTemplateType = typename first_template_type<T>::Type;
