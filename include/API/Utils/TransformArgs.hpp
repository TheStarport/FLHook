#pragma once
#include "Exceptions/InvalidParameterException.hpp"

#define OptionalTransformArg(x)                                            \
    template <>                                                            \
    std::optional<x> TransformArg(std::wstring_view s, size_t paramNumber) \
    {                                                                      \
        try                                                                \
        {                                                                  \
            return TransformArg<x>(s, paramNumber);                        \
        }                                                                  \
        catch (InvalidParameterException)                                  \
        {                                                                  \
            return {};                                                     \
        }                                                                  \
    }

#define DefineTransformArg(x)\
template <>\
x TransformArg(std::wstring_view s, size_t paramNumber);\
template <>\
std::optional<x> TransformArg(std::wstring_view s, size_t paramNumber)

// NOLINTBEGIN(*-identifier-length)
template <typename Test, template <typename...> class Ref>
struct IsSpecialization : std::false_type
{};

template <template <typename...> class Ref, typename... Args>
struct IsSpecialization<Ref<Args...>, Ref> : std::true_type
{};

// Default implementation is not allowed, explicit types only
template <typename T>
T TransformArg(std::wstring_view s, size_t paramNumber) = delete;

template <typename T>
    requires std::is_arithmetic_v<T>
T TransformArg(std::wstring_view s, size_t paramNumber)
{
    if constexpr (std::is_unsigned_v<T>)
    {
        if (s.size() > 0 && s[0] == '-')
        {
            // trying to put a negative number into an unsigned type
            throw InvalidParameterException(s, paramNumber, L"non-negative number");
        }
    }

    return StringUtils::Cast<T>(s);
}

struct StrToEnd
{
        std::wstring_view end;
};

DefineTransformArg(std::wstring_view);
DefineTransformArg(StrToEnd);
DefineTransformArg(bool);
DefineTransformArg(uint);
DefineTransformArg(int);
DefineTransformArg(Archetype::Ship*);
DefineTransformArg(Archetype::Equipment*);
DefineTransformArg(ClientId);
DefineTransformArg(GoodInfo*);
DefineTransformArg(BaseId);
DefineTransformArg(SystemId);
DefineTransformArg(RepGroupId);

template <typename T>
    requires IsSpecialization<T, std::vector>::value
T TransformArg(std::wstring_view s, size_t paramNumber)
{
    if (std::ranges::all_of(s, [](const wchar_t c) { return c == L' '; }))
    {
        return {};
    }

    T views;
    for (auto params = StringUtils::GetParams(s, L' '); auto i : params)
    {
        if constexpr (std::is_same_v<T, std::vector<std::wstring_view>>)
        {
            views.emplace_back(i);
        }
        else
        {
            views.emplace_back(StringUtils::Cast<std::decay_t<decltype(*views.begin())>>(i));
        }
    }

    return views;
}

template <typename T, size_t ParamNumber, size_t TotalParams>
constexpr int ValidateTransformationArgument()
{
    static_assert(!(ParamNumber != TotalParams - 1 && IsSpecialization<T, std::vector>::value), "A vector can only be the last parameter of any command.");
    static_assert(!(ParamNumber != TotalParams - 1 && std::is_same_v<T, StrToEnd>), "A StrToEnd can only be the last parameter of any command.");
    return 0;
}

/**
 *
 * @tparam Args The parameter pack of the possible argument types
 * @tparam Is An integer sequence for each index of the parameter pack
 * @param arguments vector of string that represent the passed args from the command
 * @return A transformed tuple where each index has been converted to it's appropriate type as defined from Args...
 */
template <typename... Args, std::size_t... Is>
auto CreateTupleImpl(std::index_sequence<Is...>, std::vector<std::wstring_view>& arguments)
{
    constexpr size_t size = sizeof...(Is);
    // If not enough params we need to pad the list
    for (uint i = arguments.size(); arguments.size() < size; i++)
    {
        arguments.emplace_back();
    }

    if constexpr (size != 0)
    {
        std::make_tuple(ValidateTransformationArgument<Args, Is, size>()...);

        using FirstType = std::tuple_element_t<0, std::tuple<Args...>>;
        static_assert(std::is_same_v<FirstType, ClientId>, "The first parameter of any command must be a client id");

        using LastType = std::tuple_element_t<size - 1, std::tuple<Args...>>;
        if constexpr (IsSpecialization<LastType, std::vector>::value || std::is_same_v<LastType, StrToEnd>)
        {
            // Given we know all the values come from the same string (same buffer),
            // We can do some janky pointer logic here to get the wstring_view
            const auto lastArg = StringUtils::Trim(*(arguments.end() - 1));
            arguments[size - 1] = std::wstring_view(arguments[size - 1].data(), lastArg.data() + lastArg.size());
        }
    }

    return std::make_tuple(TransformArg<Args>(arguments[Is], Is)...);
}

template <typename... Args>
auto CreateTuple(std::vector<std::wstring_view>& arguments)
{
    return CreateTupleImpl<Args...>(std::index_sequence_for<Args...>{}, arguments);
}

template <typename F, F f>
class ClassFunctionWrapper;

#define DefaultClassFunctionWrapper                                                                                                       \
    { public : static auto ProcessParam(Cl * cls, std::vector<std::wstring_view> & params)->Ret{ auto arg = CreateTuple<Args...>(params); \
    auto lambda = std::function<Ret(Args...)>{ [=](Args... args) mutable { return (cls->*func)(args...); } };                             \
    return std::apply(lambda, arg);                                                                                                       \
    }                                                                                                                                     \
    }

template <class Ret, class Cl, class... Args, Ret (Cl::*func)(Args...)>
class ClassFunctionWrapper<Ret (Cl::*)(Args...), func> DefaultClassFunctionWrapper;

template <class Ret, class Cl, class... Args, Ret (Cl::*func)(Args...) const>
class ClassFunctionWrapper<Ret (Cl::*)(Args...) const, func> DefaultClassFunctionWrapper;

#undef DefaultClassFunctionWrapper

// Static Class Function Wrapper
template <class Ret, class Cl, class... Args, Ret (*func)(Args...)>
class ClassFunctionWrapper<Ret (Cl::*)(Args...), func>
{
    public:
        static auto ProcessParam(Cl* cls, std::vector<std::wstring_view>& params) -> Ret
        {
            auto arg = CreateTuple<Args...>(params);
            auto lambda = std::function<Ret(Args...)>{ [=](Args... args) mutable { return func(args...); } };
            return std::apply(lambda, arg);
        }
};

// NOLINTEND(*-identifier-length)
