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
        throw InvalidParameterException(s, paramNumber, std::wstring_view(L"non-empty online character name/ID"));
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
            throw InvalidParameterException(s, paramNumber, std::wstring_view(std::format(L"valid client ID (between 0 and {}", MaxClientId)));
        }

        if (FLHook::Clients()[number].isValid)
        {
            return ClientId(number);
        }

        throw InvalidParameterException(s, paramNumber, std::wstring_view(L"client ID to match an online player"));
    }

    for (auto& client : FLHook::Clients())
    {
        if (client.characterName == s)
        {
            return client.id;
        }
    }

    throw InvalidParameterException(s, paramNumber, std::wstring_view(L"online character name/ID"));
}
OptionalTransformArg(ClientId);

template <>
Archetype::Ship* TransformArg(const std::wstring_view s, const size_t paramNumber)
{
    const std::string str = StringUtils::wstos(std::wstring(s));
    auto ship = Archetype::GetShip(CreateID(str.c_str()));

    if (!ship)
    {
        throw InvalidParameterException(s, paramNumber, std::wstring_view(L"ship nickname"));
    }
    // TODO: find ship by IDSName

    return ship;
}
OptionalTransformArg(Archetype::Ship*);

template <>
Archetype::Equipment* TransformArg(const std::wstring_view s, const size_t paramNumber)
{
    const std::string str = StringUtils::wstos(std::wstring(s));
    auto equipment = Archetype::GetEquipment(CreateID(str.c_str()));

    if (!equipment)
    {
        throw InvalidParameterException(s, paramNumber, std::wstring_view(L"equipment nickname"));
    }

    // TODO: find ship by IDSName

    return equipment;
}
OptionalTransformArg(Archetype::Equipment*);

template <>
std::wstring_view TransformArg(const std::wstring_view s, size_t paramNumber)
{
    // ReSharper disable once CppDFALocalValueEscapesFunction
    return s;
}
OptionalTransformArg(std::wstring_view);

template <>
StrToEnd TransformArg<StrToEnd>(const std::wstring_view s, size_t paramNumber)
{
    return StrToEnd(s);
}
OptionalTransformArg(StrToEnd);

template <>
bool TransformArg(const std::wstring_view s, size_t paramNumber)
{
    const auto lower = StringUtils::ToLower(s);
    if (lower == L"true" || lower == L"yes" || lower == L"1" || lower == L"on")
    {
        return true;
    }
    if (lower == L"false" || lower == L"no" || lower == L"0" || lower == L"off")
    {
        return false;
    }

    throw InvalidParameterException(s, paramNumber, std::wstring_view(L"boolean true/false"));
}
OptionalTransformArg(bool);

template <>
int TransformArg(const std::wstring_view s, size_t paramNumber)
{
    if (s.empty())
    {
        throw InvalidParameterException(s, paramNumber, std::wstring_view(L"numeric string"));
    }

    std::wstring sToValidate = std::wstring(s);
    if (s[0] == L'-')
    {
        sToValidate = sToValidate.substr(1);
    }

    if (std::ranges::all_of(sToValidate, [](const wchar_t c) { return iswdigit(c); }))
    {
        const auto number = StringUtils::Cast<int>(s);
        return number;
    }

    throw InvalidParameterException(s, paramNumber, std::wstring_view(L"fully numeric string"));
}
OptionalTransformArg(int);

template <>
GroupId TransformArg(const std::wstring_view s, size_t paramNumber)
{
    if (s.empty())
    {
        throw InvalidParameterException(s, paramNumber, std::wstring_view(L"numeric string up to 3 characters"));
    }

    if (std::ranges::all_of(s, [](const wchar_t c) { return iswdigit(c); }))
    {
        const auto number = StringUtils::Cast<uint>(s);
        return GroupId(number);
    }

    throw InvalidParameterException(s, paramNumber, std::wstring_view(L"fully numeric string up to 3 characters"));
}
OptionalTransformArg(GroupId);

template <>
uint TransformArg(const std::wstring_view s, size_t paramNumber)
{
    if (s.empty())
    {
        throw InvalidParameterException(s, paramNumber, std::wstring_view(L"non-negative numeric string"));
    }

    if (std::ranges::all_of(s, [](const wchar_t c) { return iswdigit(c); }) && s.size() < 4)
    {
        const auto number = StringUtils::Cast<uint>(s);
        return number;
    }

    throw InvalidParameterException(s, paramNumber, std::wstring_view(L"non-negative fully numeric string"));
}
OptionalTransformArg(uint);

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
        if (wildcards::match(im->GetInfoName(good->idsName), s))
        {
            return good;
        }
    }

    // Not Found
    throw InvalidParameterException(s, paramNumber, std::wstring_view(L"good display name (potentially including wildcards) or good nickname"));
}
OptionalTransformArg(GoodInfo*);

template <>
BaseId TransformArg(std::wstring_view s, size_t paramNumber)
{
    const auto foundBase = BaseId(s, true);
    if (!foundBase)
    {
        throw InvalidParameterException(s, paramNumber, std::wstring_view(L"base display name (potentially including wilcards) or base nickname"));
    }

    return foundBase;
}
OptionalTransformArg(BaseId);

template <>
SystemId TransformArg(std::wstring_view s, size_t paramNumber)
{
    const std::string str = StringUtils::wstos(std::wstring(s));
    auto system = Universe::get_system(CreateID(str.c_str()));

    if (system)
    {
        return system->id;
    }

    const auto& im = FLHook::GetInfocardManager();

    system = Universe::GetFirstSystem();
    do
    {
        if (auto name = im->GetInfoName(system->idsName); wildcards::match(name, s))
        {
            return system->id;
        }

        system = Universe::GetNextSystem();
    }
    while (system);

    throw InvalidParameterException(s, paramNumber, std::wstring_view(L"system display name (potentially including wildcards) or system nickname"));
}
OptionalTransformArg(SystemId);

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
        if (auto name = im->GetInfoName(group->data.nameIds); wildcards::match(name, s))
        {
            return RepGroupId(group->key);
        }

        if (auto name = im->GetInfoName(group->data.shortNameIds); wildcards::match(name, s))
        {
            return RepGroupId(group->key);
        }
    }

    throw InvalidParameterException(
        s, paramNumber, std::wstring_view(L"faction (optionally short) display name (potentially including wildcards) or faction nickname"));
}
OptionalTransformArg(RepGroupId);

// Include our JSON parser
#include <yyjson.c> // NOLINT
