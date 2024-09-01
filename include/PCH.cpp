#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/InfocardManager.hpp"
#include "Exceptions/InvalidParameterException.hpp"
#include "FLCore/Common/Globals.hpp"

// Template specializations for arg transformations

template <>
ClientId TransformArg(const std::wstring_view s, const size_t paramNumber)
{
    if (s.empty())
    {
        throw InvalidParameterException(s, paramNumber);
    }

    // Explicitly allow the the number '0' to mean CONSOLE
    if (s.size() == 1 && s[0] == L'0')
    {
        return ClientId();
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

template <>
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
std::wstring_view TransformArg(const std::wstring_view s, size_t paramNumber)
{
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return s;
}

template <>
bool TransformArg(const std::wstring_view s, size_t paramNumber)
{
    const auto lower = StringUtils::ToLower(s);
    if(lower == L"true" || lower == L"yes" || lower == L"1" || lower == L"on")
    {
        return true;
    }
    if(lower == L"false" || lower == L"no" || lower == L"0" || lower == L"off")
    {
        return false;
    }

    throw InvalidParameterException(s, paramNumber);
}

template <>
GoodInfo* TransformArg(std::wstring_view s, size_t paramNumber)
{
    if (const auto good = GoodList::find_by_nickname(StringUtils::wstos(std::wstring(s)).c_str()))
    {
        return const_cast<GoodInfo*>(good);
    }

    const auto& im = FLHook::GetInfocardManager();
    auto* goods = GoodList_get();
    for (const auto good : *goods->get_list())
    {
        if (wildcards::match(im.GetInfocard(good->idsName), s))
        {
            return good;
        }
    }

    // Not Found
    throw InvalidParameterException(s, paramNumber);
}

template <>
BaseId TransformArg(std::wstring_view s, size_t paramNumber)
{
    const std::string str = StringUtils::wstos(std::wstring(s));
    auto base = Universe::get_base(CreateID(str.c_str()));

    if (base)
    {
        return BaseId(base->baseId);
    }

    const auto& im = FLHook::GetInfocardManager();
    base = Universe::GetFirstBase();
    do
    {
        if (auto name = im.GetInfocard(base->baseIdS); wildcards::match(name, s))
        {
            return BaseId(base->baseId);
        }

        base = Universe::GetNextBase();
    }
    while (base);

    throw InvalidParameterException(s, paramNumber);
}

template <>
SystemId TransformArg(std::wstring_view s, size_t paramNumber)
{
    const std::string str = StringUtils::wstos(std::wstring(s));
    auto system = Universe::get_system(CreateID(str.c_str()));

    if (system)
    {
        return SystemId(system->id);
    }

    const auto& im = FLHook::GetInfocardManager();

    system = Universe::GetFirstSystem();
    do
    {
        if (auto name = im.GetInfocard(system->idsName); wildcards::match(name, s))
        {
            return SystemId(system->id);
        }

        system = Universe::GetNextSystem();
    }
    while (system);

    throw InvalidParameterException(s, paramNumber);
}

template <>
RepGroupId TransformArg(std::wstring_view s, size_t paramNumber)
{
    const std::string str = StringUtils::wstos(std::wstring(s));

    uint ids = 0;
    if (const uint id = MakeId(str.c_str()); pub::Reputation::GetShortGroupName(id, ids) == 0)
    {
        return RepGroupId(id);
    }

    const auto& im = FLHook::GetInfocardManager();
    for (const auto group : GameData::repGroups)
    {
        if (auto name = im.GetInfocard(group->data.nameIds); wildcards::match(name, s))
        {
            return RepGroupId(group->key);
        }

        if (auto name = im.GetInfocard(group->data.shortNameIds); wildcards::match(name, s))
        {
            return RepGroupId(group->key);
        }
    }

    throw InvalidParameterException(s, paramNumber);
}

// Include our JSON parser
#include <yyjson.c> // NOLINT
