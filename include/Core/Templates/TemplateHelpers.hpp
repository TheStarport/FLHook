// ReSharper disable CppClangTidyClangDiagnosticReturnStackAddress
#pragma once

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
inline bool TransformArg(std::wstring_view s)
{
    const auto lower = StringUtils::ToLower(s);
    return lower == L"true" || lower == L"yes" || lower == L"1";
}

template <>
inline std::vector<std::wstring_view> TransformArg(std::wstring_view s)
{
    std::vector<std::wstring_view> views;
    for (auto params = StringUtils::GetParams(s, L' '); auto i : params)
    {
        views.emplace_back(i);
    }

    return views;
}

template <typename... Args, std::size_t... Is>
auto CreateTupleImpl(std::index_sequence<Is...>, std::vector<std::wstring>& arguments)
{
    constexpr size_t size = sizeof...(Is);
    // If not enough params we need to pad the list
    for (uint i = arguments.size(); arguments.size() < size; i++)
    {
        arguments.emplace_back();
    }

    if constexpr (size != 0)
    {
        using LastType = std::tuple_element_t<size - 1, std::tuple<Args...>>;
        if constexpr (std::is_same_v<LastType, std::vector<std::wstring_view>>)
        {
            std::vector extraArgs = {
                std::pair{size - 1, arguments.size()}
            };
            auto view = extraArgs |
                        std::views::transform(
                            [&arguments](auto r)
                            {
                                auto [beg, end] = r;
                                return std::views::counted(arguments.begin() + beg, end - beg);
                            }) |
                        std::views::join;

            std::wstring newArg;
            std::ranges::for_each(view, [&newArg](auto& v) { newArg += std::format(L"{} ", std::wstring(v)); });

            arguments[size - 1] = StringUtils::Trim(newArg);
        }
    }

    return std::make_tuple(TransformArg<Args>(arguments[Is])...);
}

template <typename... Args>
auto CreateTuple(std::vector<std::wstring>& arguments)
{
    return CreateTupleImpl<Args...>(std::index_sequence_for<Args...>{}, arguments);
}

template <typename F, F f>
class ClassFunctionWrapper;

template <class Ret, class Cl, class... Args, Ret (Cl::*func)(Args...)>
class ClassFunctionWrapper<Ret (Cl::*)(Args...), func>
{
    public:
        static Ret ProcessParam(Cl* cl, std::vector<std::wstring>& params)
        {
            auto arg = CreateTuple<Args...>(params);
            auto lambda = std::function<Ret(Args...)>{ [=](Args... args) mutable { return (cl->*func)(args...); } };
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
