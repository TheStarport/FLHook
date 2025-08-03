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

Action<void> CharacterId::UpdateCharacterDocument(std::string_view name, bsoncxx::document::view updateDoc)
{
    const auto db = FLHook::GetDbClient();
    const auto config = FLHook::GetConfig();
    auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];

    const auto findCharDoc = B_MDOC(B_KVP("characterName", name));
    auto query = FLHook::GetDatabase()->BeginDatabaseQuery();

    try
    {
        query.UpdateFromCollection(DatabaseCollection::Character, findCharDoc, updateDoc);
        query.ConcludeQuery(true);
        return { {} };
    }
    catch (mongocxx::exception& ex)
    {
        query.ConcludeQuery(false);
        return { cpp::fail(Error::CharacterNameNotFound) };
    }
}

CharacterId::CharacterId(const std::wstring_view characterName) : characterName(characterName) {}

CharacterId::operator bool() const { return !characterName.empty(); }

concurrencpp::result<bool> CharacterId::CharacterExists(std::wstring_view characterName)
{
    const auto findCharDoc = B_MDOC(B_KVP("characterName", StringUtils::wstos(characterName)));
    THREAD_BACKGROUND;

    const auto db = FLHook::GetDbClient();
    const auto config = FLHook::GetConfig();
    auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];

    try
    {
        mongocxx::options::find options;
        options.projection(B_MDOC(B_KVP("_id", 1)));
        co_return charactersCollection.find_one(findCharDoc.view(), options).has_value();
    }
    catch (mongocxx::exception& ex)
    {
        co_return false;
    }
}

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

concurrencpp::result<Action<void>> CharacterId::Delete() const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);
    THREAD_BACKGROUND;

    const auto db = FLHook::GetDbClient();
    const auto config = FLHook::GetConfig();
    auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];

    const auto findCharDoc = B_MDOC(B_KVP("characterName", characterNameStr));
    auto query = FLHook::GetDatabase()->BeginDatabaseQuery();

    try
    {
        auto charDoc = query.FindAndDelete(DatabaseCollection::Character, findCharDoc, B_MDOC(B_KVP("_id", 1), B_KVP("accountId", 1)));
        if (charDoc.empty())
        {
            query.ConcludeQuery(false);
            co_return Action<void>{ cpp::fail(Error::CharacterNameNotFound) };
        }

        const auto findAccDoc = B_MDOC(B_KVP("_id", charDoc.view()["accountId"].get_string().value));
        const auto updateDoc = B_MDOC(B_KVP("$pull", B_MDOC(B_KVP("characters", charDoc.view()["_id"].get_oid().value))));
        const auto updateResult = query.UpdateFromCollection(DatabaseCollection::Accounts, findAccDoc, updateDoc);

        if (updateResult.modified_count() != 1)
        {
            query.ConcludeQuery(false);
            co_return Action<void>{ cpp::fail(Error::DatabaseError) };
        }

        query.ConcludeQuery(true);
        RET_VOID;
    }
    catch (mongocxx::exception& ex)
    {
        query.ConcludeQuery(false);
        co_return Action<void>{ cpp::fail(Error::DatabaseError) };
    }
}

concurrencpp::result<Action<void>> CharacterId::SetTransferCode(std::wstring_view code) const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);
    const std::string transferCode = StringUtils::wstos(code);
    THREAD_BACKGROUND;

    const auto updateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("transferCode", transferCode))));
    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
}

concurrencpp::result<Action<void>> CharacterId::ClearTransferCode() const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);
    THREAD_BACKGROUND;

    const auto updateDoc = B_MDOC(B_KVP("$unset", B_MDOC(B_KVP("transferCode", ""))));
    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
}

concurrencpp::result<Action<void>> CharacterId::Rename(std::wstring_view name) const
{
    const auto oldCharNameWide = std::wstring{ name };
    const std::string oldCharName = StringUtils::wstos(characterName);
    const std::string newCharName = StringUtils::wstos(name);

    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        data->id.Kick(L"Renaming character", 5);
        co_await FLHook::GetTaskScheduler()->Delay(5s, true);
    }

    if (co_await CharacterExists(oldCharNameWide))
    {
        co_return Action<void>{ cpp::fail(Error::CharacterAlreadyExists) };
    }

    const auto updateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("characterName", newCharName))));
    co_return UpdateCharacterDocument(oldCharName, updateDoc);
}

concurrencpp::result<Action<void>> CharacterId::AdjustCash(int cash) const
{
    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        pub::Player::AdjustCash(data->id.GetValue(), cash);
        data->id.SaveChar();
        RET_VOID;
    }

    const std::string characterNameStr = StringUtils::wstos(characterName);
    THREAD_BACKGROUND;

    const auto updateDoc = B_MDOC(B_KVP("$inc", B_MDOC(B_KVP("money", cash))));
    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
}

concurrencpp::result<Action<void>> CharacterId::AddCash(const int cash) const { co_return AdjustCash(std::abs(cash)); }
concurrencpp::result<Action<void>> CharacterId::RemoveCash(const int cash) const { co_return AdjustCash(-std::abs(cash)); }

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
