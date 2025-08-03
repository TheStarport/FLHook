#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/Types/CharacterId.hpp"

#include "API/FLHook/BsonHelper.hpp"

#define RET_VOID           \
    co_return Action<void> \
    {                      \
        {}                 \
    }

Action<bsoncxx::document::value> CharacterId::GetCharacterDocument(std::string_view name)
{
    const auto db = FLHook::GetDbClient();
    const auto config = FLHook::GetConfig();
    auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];

    const auto findCharDoc = B_MDOC(B_KVP("characterName", name));

    try
    {
        if (const auto checkCharNameDoc = charactersCollection.find_one(findCharDoc.view()); checkCharNameDoc.has_value())
        {
            return { checkCharNameDoc.value() };
        }

        return { cpp::fail(Error::CharacterNameNotFound) };
    }
    catch (mongocxx::exception& ex)
    {
        return { cpp::fail(Error::CharacterNameNotFound) };
    }
}

CharacterId::CharacterId(const std::wstring_view characterName) : characterName(characterName) {}

CharacterId::operator bool() const { return !characterName.empty(); }

ClientData* CharacterId::GetOnlineData() const
{
    for (auto& client : FLHook::Clients())
    {
        if (client.characterName == characterName)
        {
            return &client;
        }
    }

    return nullptr;
}

concurrencpp::result<Action<int>> CharacterId::GetCash() const
{
    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        co_return Action<int>{ data->playerData->money };
    }

    const std::string characterNameStr = StringUtils::wstos(characterName);
    THREAD_BACKGROUND;

    const auto character = GetCharacterDocument(characterNameStr).Handle();
    auto value = character.find("money");
    co_return Action<int>{ value->get_int32().value };
}

concurrencpp::result<Action<SystemId>> CharacterId::GetSystem() const
{
    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        co_return Action<SystemId>{ data->playerData->systemId };
    }

    const std::string characterNameStr = StringUtils::wstos(characterName);
    THREAD_BACKGROUND;

    const auto character = GetCharacterDocument(characterNameStr).Handle();
    auto system = character.find("system");
    co_return Action<SystemId>{ SystemId{ static_cast<uint>(system->get_int32().value) } };
}

concurrencpp::result<Action<RepGroupId>> CharacterId::GetAffiliation() const
{
    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        const auto rep = RepId(data->playerData->reputation);
        co_return rep.GetAffiliation();
    }

    const std::string characterNameStr = StringUtils::wstos(characterName);
    THREAD_BACKGROUND;

    const auto character = GetCharacterDocument(characterNameStr).Handle();
    auto value = character.find("affiliation");
    if (value == character.end())
    {
        co_return Action<RepGroupId>{ cpp::fail(Error::NoAffiliation) };
    }

    co_return Action<RepGroupId>{ RepGroupId(static_cast<uint>(value->get_int32().value)) };
}

concurrencpp::result<Action<Vector>> CharacterId::GetPosition() const
{
    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        if (data->baseId)
        {
            co_return Action<Vector>{ cpp::fail(Error::PlayerNotInSpace) };
        }

        co_return Action<Vector>{ data->ship.GetPosition().Handle() };
    }

    const std::string characterNameStr = StringUtils::wstos(characterName);
    THREAD_BACKGROUND;

    const auto character = GetCharacterDocument(characterNameStr).Handle();
    auto pos = character.find("pos");
    if (pos == character.end())
    {
        co_return Action<Vector>{ cpp::fail(Error::PlayerNotInSpace) };
    }

    const auto posArr = pos->get_array().value;
    co_return Action<Vector>{
        Vector{ static_cast<float>(posArr[0].get_double()), static_cast<float>(posArr[1].get_double()), static_cast<float>(posArr[2].get_double()) }
    };
}

concurrencpp::result<Action<bsoncxx::document::value>> CharacterId::GetCharacterData() const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);
    THREAD_BACKGROUND;
    co_return GetCharacterDocument(characterNameStr);
}

std::wstring_view CharacterId::GetValue() const { return characterName; }
