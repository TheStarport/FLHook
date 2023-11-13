#pragma once

// Default transform arg - assume is integral or floating point and straight cast it
template <class T>
T TransformArg(std::wstring_view s, size_t paramNumber)
{
    return StringUtils::Cast<T>(s);
}

template <>
std::wstring_view TransformArg(std::wstring_view s, size_t paramNumber);

template <>
bool TransformArg(std::wstring_view s, size_t paramNumber);

template <>
std::vector<std::wstring_view> TransformArg(std::wstring_view s, size_t paramNumber);

template <>
Archetype::Ship* TransformArg(std::wstring_view s, size_t paramNumber);

template <>
ClientId TransformArg(std::wstring_view s, size_t paramNumber);

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

    return std::make_tuple(TransformArg<Args>(arguments[Is], Is)...);
}
