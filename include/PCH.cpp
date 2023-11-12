#include "PCH.hpp"

// Dummy file to force build!

#include "Exceptions/InvalidParameterException.hpp"

// Template specializations for arg transformations

template <>
ClientId TransformArg(const std::wstring_view s, const size_t paramNumber)
{
    const auto number = StringUtils::Cast<size_t>(s);
    if (number == 0 || number > MaxClientId)
    {
        throw InvalidParameterException(s, paramNumber);
    }

    if (FLHook::Clients()[number].isValid)
    {
        return ClientId(number);
    }

    throw InvalidParameterException(s, paramNumber);
}

template <>
Archetype::Ship* TransformArg(const std::wstring_view s, const size_t paramNumber)
{
    const std::string str = StringUtils::wstos(std::wstring(s));
    // ReSharper disable once CppLocalVariableMayBeConst
    auto ids = ID_String();
    strncpy_s(reinterpret_cast<char*>(ids.data), sizeof(ids.data), str.c_str(), str.size());
    const auto ship = Archetype::GetShipByName(ids);

    if (!ship)
    {
        throw InvalidParameterException(s, paramNumber);
    }

    return ship;
}

template <>
std::wstring_view TransformArg(std::wstring_view s, size_t paramNumber)
{
    return s;
}

template <>
bool TransformArg(std::wstring_view s, size_t paramNumber)
{
    const auto lower = StringUtils::ToLower(s);
    return lower == L"true" || lower == L"yes" || lower == L"1";
}

template <>
std::vector<std::wstring_view> TransformArg(std::wstring_view s, size_t paramNumber)
{
    std::vector<std::wstring_view> views;
    for (auto params = StringUtils::GetParams(s, L' '); auto i : params)
    {
        views.emplace_back(i);
    }

    return views;
}
