#include <algorithm>

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
        const auto charDoc = query.FindAndDelete(DatabaseCollection::Character, findCharDoc, B_MDOC(B_KVP("_id", 1), B_KVP("accountId", 1)));
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

    // This puts us on the background thread
    if (co_await CharacterExists(oldCharNameWide))
    {
        co_return Action<void>{ cpp::fail(Error::CharacterAlreadyExists) };
    }

    const auto updateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("characterName", newCharName))));
    co_return UpdateCharacterDocument(oldCharName, updateDoc);
}

concurrencpp::result<Action<void>> CharacterId::AdjustCash(int cash) const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);

    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        pub::Player::AdjustCash(data->id.GetValue(), cash);
        co_return data->id.SaveChar();
    }

    THREAD_BACKGROUND;

    const auto updateDoc = B_MDOC(B_KVP("$inc", B_MDOC(B_KVP("money", cash))));
    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
}

concurrencpp::result<Action<void>> CharacterId::AddCash(const int cash) const { co_return AdjustCash(std::abs(cash)); }
concurrencpp::result<Action<void>> CharacterId::RemoveCash(const int cash) const { co_return AdjustCash(-std::abs(cash)); }

concurrencpp::result<Action<void>> CharacterId::AddCargo(GoodId good, uint count, float health, bool mission) const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);
    auto goodId = good.GetHash().Unwrap().GetValue();

    THREAD_MAIN;

    if (health < 0.f || health > 1.f)
    {
        health = 1.f;
    }

    if (const auto* data = GetOnlineData())
    {
        // Returns 0 on success
        if (pub::Player::AddCargo(data->id.GetValue(), goodId, count, health, mission) == 0)
        {
            co_return data->id.SaveChar();
        }

        co_return Action<void>{ cpp::fail(Error::InvalidGood) };
    }

    THREAD_BACKGROUND;

    FLCargo cargo{ static_cast<int>(goodId), static_cast<ushort>(count), health, mission };

    // We start up by assuming the cargo being added does not currently exist, if this fails we then look for existing cargo to add to
    auto findCharDoc = B_MDOC(B_KVP("characterName", characterNameStr), B_KVP("cargo.archId", B_MDOC(B_KVP("$ne", cargo.archId))));
    const auto insertDoc = B_MDOC(B_KVP("$addToSet", B_MDOC(B_KVP("cargo", cargo.ToBson()))));
    auto query = FLHook::GetDatabase()->BeginDatabaseQuery();
    auto update = query.UpdateFromCollection(DatabaseCollection::Character, findCharDoc, insertDoc);

    if (update.modified_count() == 1)
    {
        query.ConcludeQuery(true);
        RET_VOID;
    }

    findCharDoc = B_MDOC(B_KVP("characterName", characterNameStr));
    const auto updateDoc = B_MDOC(B_KVP("$inc", B_MDOC(B_KVP("cargo.$[elem].amount", std::abs(static_cast<int>(count))))));
    const auto arrayFilter = B_MARR(B_MDOC(B_KVP("elem.archId", static_cast<int>(goodId))));
    update = query.UpdateFromCollection(DatabaseCollection::Character, findCharDoc.view(), updateDoc.view(), arrayFilter.view());
    if (!update.matched_count())
    {
        query.ConcludeQuery(false);
        co_return Action<void>{ cpp::fail(Error::CharacterNameNotFound) };
    }

    query.ConcludeQuery(true);
    RET_VOID;
}

concurrencpp::result<Action<void>> CharacterId::RemoveCargo(GoodId good, uint count) const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);
    const auto goodId = good.GetHash().Unwrap().GetValue();

    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        ushort equipId = 0;
        for (auto& eq : data->playerData->equipAndCargo.equip)
        {
            if (eq.archId.GetValue() == goodId)
            {
                count = std::min<uint>(eq.count, count);
                equipId = eq.id;
            }
        }

        if (!equipId)
        {
            co_return Action<void>{ cpp::fail(Error::InvalidGood) };
        }

        pub::Player::RemoveCargo(data->id.GetValue(), equipId, count);
        co_return data->id.SaveChar();
    }

    THREAD_BACKGROUND;

    const auto findCharDoc = B_MDOC(B_KVP("characterName", characterNameStr));
    auto query = FLHook::GetDatabase()->BeginDatabaseQuery();
    auto updateDoc = B_MDOC(B_KVP("$inc", B_MDOC(B_KVP("cargo.$[elem].amount", -std::abs(static_cast<int>(count))))));
    const auto arrayFilter = B_MARR(B_MDOC(B_KVP("elem.archId", static_cast<int>(goodId))));
    const auto update = query.UpdateFromCollection(DatabaseCollection::Character, findCharDoc.view(), updateDoc.view(), arrayFilter.view());

    // If we didn't match, invalid char name
    if (!update.matched_count())
    {
        query.ConcludeQuery(false);
        co_return Action<void>{ cpp::fail(Error::CharacterNameNotFound) };
    }

    // If we didn't modify, invalid good or character didn't have good
    if (!update.modified_count())
    {
        query.ConcludeQuery(false);
        co_return Action<void>{ cpp::fail(Error::InvalidGood) };
    }

    // So we successfully modified! Let's clean up any goods on the character that have zero or negative amounts
    // TODO: Move this to a generic account manager function
    updateDoc = B_MDOC(B_KVP("$pull", B_MDOC(B_KVP("cargo.$.amount", B_MDOC(B_KVP("$lte", 0))))));
    query.UpdateFromCollection(DatabaseCollection::Character, findCharDoc.view(), updateDoc.view());
    query.ConcludeQuery(true);
    RET_VOID;
}

concurrencpp::result<Action<int>> CharacterId::GetCash() const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);

    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        co_return Action<int>{ data->playerData->money };
    }

    THREAD_BACKGROUND;

    const auto character = GetCharacterDocument(characterNameStr).Handle();
    auto value = character.find("money");
    co_return Action<int>{ value->get_int32().value };
}

concurrencpp::result<Action<SystemId>> CharacterId::GetSystem() const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);
    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        co_return Action<SystemId>{ data->playerData->systemId };
    }

    THREAD_BACKGROUND;

    const auto character = GetCharacterDocument(characterNameStr).Handle();
    auto system = character.find("system");
    co_return Action<SystemId>{ SystemId{ static_cast<uint>(system->get_int32().value) } };
}

concurrencpp::result<Action<RepGroupId>> CharacterId::GetAffiliation() const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);

    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        const auto rep = RepId(data->playerData->reputation);
        co_return rep.GetAffiliation();
    }

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
    const std::string characterNameStr = StringUtils::wstos(characterName);

    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        if (data->baseId)
        {
            co_return Action<Vector>{ cpp::fail(Error::PlayerNotInSpace) };
        }

        co_return Action<Vector>{ data->ship.GetPosition().Handle() };
    }

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
