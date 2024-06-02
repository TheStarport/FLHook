#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "Exceptions/InvalidParameterException.hpp"

// Template specializations for arg transformations

template <>
ClientId TransformArg(const std::wstring_view s, const size_t paramNumber)
{
    if (s.empty())
    {
        throw InvalidParameterException(s, paramNumber);
    }

    if (std::ranges::all_of(s, [](const wchar_t c) { return iswdigit(c); }) && s.size() < 4)
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

    for (auto& client : FLHook::Clients())
    {
        if (client.characterName == s)
        {
            return client.id;
        }
    }

    throw InvalidParameterException(s, paramNumber);
}

template <>
Archetype::Ship* TransformArg(const std::wstring_view s, const size_t paramNumber)
{
    const std::string str = StringUtils::wstos(std::wstring(s));
    auto ship = Archetype::GetShip(CreateID(str.c_str()));

    if (ship)
    {
        return ship;
    }

    // ReSharper disable once CppLocalVariableMayBeConst
    auto ids = ID_String();
    strncpy_s(reinterpret_cast<char*>(ids.data), sizeof(ids.data), str.c_str(), str.size());
    ship = Archetype::GetShipByName(ids);

    if (!ship)
    {
        throw InvalidParameterException(s, paramNumber);
    }

    return ship;
}

template<>
Archetype::Equipment* TransformArg(const std::wstring_view s, const size_t paramNumber)
{
    const std::string str = StringUtils::wstos(std::wstring(s));
    auto equipment = Archetype::GetEquipment(CreateID(str.c_str()));

    if (equipment)
    {
        return equipment;
    }

    // ReSharper disable once CppLocalVariableMayBeConst
    auto ids = ID_String();
    strncpy_s(reinterpret_cast<char*>(ids.data), sizeof(ids.data), str.c_str(), str.size());
    equipment = Archetype::GetEquipmentByName(ids);

    if (!equipment)
    {
        throw InvalidParameterException(s, paramNumber);
    }

    return equipment;
}

template <>
std::wstring_view TransformArg(std::wstring_view s, size_t paramNumber)
{
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return s;
}

template <>
bool TransformArg(const std::wstring_view s, size_t paramNumber)
{
    const auto lower = StringUtils::ToLower(s);
    return lower == L"true" || lower == L"yes" || lower == L"1" || lower == L"on";
}

template <>
std::vector<std::wstring_view> TransformArg(std::wstring_view s, size_t paramNumber)
{
    // If we have nothing but whitespace, return an empty list
    if (std::ranges::all_of(s, [](const wchar_t c) { return c == L' '; }))
    {
        return {};
    }

    std::vector<std::wstring_view> views;
    for (auto params = StringUtils::GetParams(s, L' '); auto i : params)
    {
        views.emplace_back(i);
    }

    return views;
}

// Include our JSON parser
#include <yyjson.c> // NOLINT
