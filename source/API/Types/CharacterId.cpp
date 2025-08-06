#include <algorithm>

#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/Types/CharacterId.hpp"

#include "Defs/FLHookConfig.hpp"
#include "Defs/Database/MongoResult.hpp"

#define RET_SUCCESS \
    co_return Action<MongoResult> { MongoResult::Success }

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

Action<MongoResult> CharacterId::UpdateCharacterDocument(std::string_view name, const bsoncxx::document::view updateDoc)
{
    const auto db = FLHook::GetDbClient();
    const auto config = FLHook::GetConfig();
    auto charactersCollection = db->database(config->database.dbName)[config->database.charactersCollection];

    const auto findCharDoc = B_MDOC(B_KVP("characterName", name));
    auto query = FLHook::GetDatabase()->BeginDatabaseQuery();

    try
    {
        const auto update = query.UpdateFromCollection(DatabaseCollection::Character, findCharDoc, updateDoc);
        const auto newDoc = query.FindFromCollection(DatabaseCollection::Character, findCharDoc);

        query.ConcludeQuery(true);

        if (update.matched_count() == 0)
        {
            return { cpp::fail(Error::CharacterNameNotFound) };
        }

        if (update.modified_count() == 0)
        {
            return { MongoResult::MatchButNoChange };
        }

        const auto targetCharacterCode = CharacterId{ StringUtils::stows(name) }.GetCharacterCode();
        const auto accountId = newDoc->find("accountId")->get_string().value;
        for (auto& account : AccountManager::accounts)
        {
            std::scoped_lock lock{ account.mutex };

            // If the updated character's account is currently logged in we need to update their character cache
            if (account.account._id == accountId)
            {
                account.characters[targetCharacterCode] = Character{ newDoc->view() };
                break;
            }
        }

        return { MongoResult::Success };
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
        if (client.characterId == *this)
        {
            return &client;
        }
    }

    return nullptr;
}

std::string CharacterId::GetCharacterCode() const
{
    if (characterName.empty())
    {
        return {};
    }

    char charNameBuf[50];
    AccountManager::getFlName(charNameBuf, characterName.c_str());
    return charNameBuf;
}

concurrencpp::result<Action<MongoResult>> CharacterId::Delete() const
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
            co_return Action<MongoResult>{ cpp::fail(Error::CharacterNameNotFound) };
        }

        auto accountId = charDoc.view()["accountId"].get_string().value;
        auto charOid = charDoc.view()["_id"].get_oid().value;
        const auto findAccDoc = B_MDOC(B_KVP("_id", accountId));
        const auto updateDoc = B_MDOC(B_KVP("$pull", B_MDOC(B_KVP("characters", charOid))));
        const auto updateResult = query.UpdateFromCollection(DatabaseCollection::Accounts, findAccDoc, updateDoc);

        query.ConcludeQuery(true);
        if (updateResult.matched_count() == 0)
        {
            co_return Action<MongoResult>{ cpp::fail(Error::CharacterNameNotFound) };
        }

        if (updateResult.modified_count() == 0)
        {
            co_return Action<MongoResult>{ MongoResult::MatchButNoChange };
        }

        THREAD_MAIN;

        auto characterIdStr = GetCharacterCode();
        // Check if the account connected was online and update in memory
        for (auto& account : AccountManager::accounts)
        {
            std::scoped_lock lock{ account.mutex };
            auto character = account.characters.find(characterIdStr);
            if (character == account.characters.end())
            {
                continue;
            }

            std::erase_if(account.account.characters, [&charOid](const bsoncxx::oid& oid) { return oid == charOid; });
            account.characters.erase(character);
            account.internalAccount->numberOfCharacters--;
            break;
        }

        RET_SUCCESS;
    }
    catch (mongocxx::exception& ex)
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::DatabaseError) };
    }
}

concurrencpp::result<Action<MongoResult>> CharacterId::Transfer(AccountId targetAccount, std::wstring_view code) const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);
    const std::string transferCode = StringUtils::wstos(code);

    THREAD_BACKGROUND;

    auto query = FLHook::GetDatabase()->BeginDatabaseQuery();

    // Extract and preemptively remove the transfer code and update the account id (if invalid we won't commit the change)
    const auto findCharDoc = B_MDOC(B_KVP("characterName", characterNameStr));
    auto updateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("accountId", targetAccount.GetValue()))), B_KVP("$unset", B_MDOC(B_KVP("transferCode", ""))));
    auto projection = B_MDOC(B_KVP("_id", 1), B_KVP("accountId", 1), B_KVP("transferCode", 1));
    const auto charDoc = query.FindAndUpdate(DatabaseCollection::Character, findCharDoc.view(), updateDoc, projection.view());

    if (!charDoc.has_value())
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::CharacterNameNotFound) };
    }

    // Check that we both have a transfer code and that it is valid
    if (auto currentTransferCode = charDoc->find("transferCode"); currentTransferCode == charDoc->end())
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::CharacterHasNoTransferCode) };
    }
    else if (currentTransferCode->get_string().value != transferCode)
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::CharacterHasAnInvalidTransferCode) };
    }

    const std::string_view currentAccountId = charDoc->find("accountId")->get_string().value;
    const auto charOid = charDoc->find("_id")->get_oid().value;

    // We have verified the code is good and are ready to transfer the character
    // But first we need to ensure that both accounts have no one using them
    THREAD_MAIN;

    bool shouldWait = false;

    for (auto& client : FLHook::Clients())
    {
        if (client.account && client.account->_id == targetAccount.GetValue() || client.account->_id == currentAccountId)
        {
            client.id.Kick(L"Attempting character transfer", 5);
            shouldWait = true;
        }
    }

    if (shouldWait)
    {
        co_await FLHook::GetTaskScheduler()->Delay(5s, true);
    }

    THREAD_BACKGROUND;

    // Remove this character from the current account's char array

    auto findAccountDoc = B_MDOC(B_KVP("_id", currentAccountId));
    updateDoc = B_MDOC(B_KVP("$pull", B_MDOC(B_KVP("characters", charOid))));
    auto updateResult = query.UpdateFromCollection(DatabaseCollection::Accounts, findAccountDoc.view(), updateDoc.view());

    if (updateResult.matched_count() != 1)
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::AccountNotFound) };
    }

    if (updateResult.modified_count() != 1)
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::DatabaseError) };
    }

    // Now we add it to the new account
    findAccountDoc = B_MDOC(B_KVP("_id", targetAccount.GetValue()));
    updateDoc = B_MDOC(B_KVP("$push", B_MDOC(B_KVP("characters", charOid))));
    projection = B_MDOC(B_KVP("characters", 1));
    auto newAccountDoc = query.FindAndUpdate(DatabaseCollection::Accounts, findAccountDoc.view(), updateDoc.view(), projection.view());

    if (!newAccountDoc.has_value())
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::AccountNotFound) };
    }

    // TODO: Unhardcode maximum character count!
    if (newAccountDoc->find("characters")->get_array().value.length() >= 5)
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::AccountHasTooManyCharacters) };
    }

    if (updateResult.modified_count() != 1)
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::DatabaseError) };
    }

    query.ConcludeQuery(true);
    RET_SUCCESS;
}

concurrencpp::result<Action<MongoResult>> CharacterId::SetTransferCode(std::wstring_view code) const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);
    const std::string transferCode = StringUtils::wstos(code);
    THREAD_BACKGROUND;

    const auto updateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("transferCode", transferCode))));
    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
}

concurrencpp::result<Action<MongoResult>> CharacterId::ClearTransferCode() const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);
    THREAD_BACKGROUND;

    const auto updateDoc = B_MDOC(B_KVP("$unset", B_MDOC(B_KVP("transferCode", ""))));
    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
}

concurrencpp::result<Action<MongoResult>> CharacterId::Rename(std::wstring_view name) const
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
        co_return Action<MongoResult>{ cpp::fail(Error::CharacterAlreadyExists) };
    }

    const auto updateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("characterName", newCharName))));
    co_return UpdateCharacterDocument(oldCharName, updateDoc);
}

concurrencpp::result<Action<MongoResult>> CharacterId::AdjustCash(int cash) const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);

    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        pub::Player::AdjustCash(data->id.GetValue(), cash);
        data->id.SaveChar();
        co_return Action<MongoResult>{ MongoResult::PerformedSynchronously };
    }

    THREAD_BACKGROUND;

    const auto updateDoc = B_MDOC(B_KVP("$inc", B_MDOC(B_KVP("money", cash))));
    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
}

concurrencpp::result<Action<MongoResult>> CharacterId::AddCash(const int cash) const { co_return co_await AdjustCash(cash); }
concurrencpp::result<Action<MongoResult>> CharacterId::RemoveCash(const int cash) const { co_return co_await AdjustCash(-std::abs(cash)); }

concurrencpp::result<Action<MongoResult>> CharacterId::SetCash(int cash) const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);

    cash = std::clamp(cash, 0, 1'900'000'000);

    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        int currentMoney{};
        pub::Player::InspectCash(data->id.GetValue(), currentMoney);
        pub::Player::AdjustCash(data->id.GetValue(), cash);
        data->id.SaveChar();
        co_return Action<MongoResult>{ MongoResult::PerformedSynchronously };
    }

    THREAD_BACKGROUND;

    const auto updateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("money", cash))));
    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
}

concurrencpp::result<Action<MongoResult>> CharacterId::AddCargo(GoodId good, uint count, float health, bool mission) const
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
            data->id.SaveChar();
            co_return Action<MongoResult>{ MongoResult::PerformedSynchronously };
        }

        co_return Action<MongoResult>{ cpp::fail(Error::InvalidGood) };
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
        RET_SUCCESS;
    }

    findCharDoc = B_MDOC(B_KVP("characterName", characterNameStr));
    const auto updateDoc = B_MDOC(B_KVP("$inc", B_MDOC(B_KVP("cargo.$[elem].amount", std::abs(static_cast<int>(count))))));
    const auto arrayFilter = B_MARR(B_MDOC(B_KVP("elem.archId", static_cast<int>(goodId))));
    update = query.UpdateFromCollection(DatabaseCollection::Character, findCharDoc.view(), updateDoc.view(), arrayFilter.view());
    if (!update.matched_count())
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::CharacterNameNotFound) };
    }

    query.ConcludeQuery(true);
    RET_SUCCESS;
}

concurrencpp::result<Action<MongoResult>> CharacterId::RemoveCargo(GoodId good, uint count) const
{
    if (!good)
    {
        co_return Action<MongoResult>{ cpp::fail(Error::InvalidGood) };
    }

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
            co_return Action<MongoResult>{ cpp::fail(Error::InvalidGood) };
        }

        pub::Player::RemoveCargo(data->id.GetValue(), equipId, count);
        data->id.SaveChar();
        co_return Action<MongoResult>{ MongoResult::PerformedSynchronously };
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
        co_return Action<MongoResult>{ cpp::fail(Error::CharacterNameNotFound) };
    }

    // If we didn't modify, invalid good or character didn't have good
    if (!update.modified_count())
    {
        query.ConcludeQuery(false);
        co_return Action<MongoResult>{ cpp::fail(Error::InvalidGood) };
    }

    // So we successfully modified! Let's clean up any goods on the character that have zero or negative amounts
    // TODO: Move this to a generic account manager function
    updateDoc = B_MDOC(B_KVP("$pull", B_MDOC(B_KVP("cargo.$.amount", B_MDOC(B_KVP("$lte", 0))))));
    query.UpdateFromCollection(DatabaseCollection::Character, findCharDoc.view(), updateDoc.view());
    query.ConcludeQuery(true);
    RET_SUCCESS;
}

concurrencpp::result<Action<MongoResult>> CharacterId::SetPosition(Vector pos) const
{
    const std::string characterNameStr = StringUtils::wstos(characterName);

    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        if (data->baseId)
        {
            // TODO: Undock with fluf
            co_return Action<MongoResult>{ MongoResult::PerformedSynchronously };
        }

        pub::SpaceObj::Relocate(data->id.GetValue(), data->playerData->systemId.GetValue(), pos, Matrix::Identity());
        co_return Action<MongoResult>{ MongoResult::PerformedSynchronously };
    }

    THREAD_BACKGROUND;

    const auto updateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("pos", B_MARR(pos.x, pos.y, pos.z)))));
    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
}

concurrencpp::result<Action<MongoResult>> CharacterId::SetSystem(SystemId system) const
{
    if (!system)
    {
        co_return Action<MongoResult>{ cpp::fail(Error::InvalidSystem) };
    }

    const std::string characterNameStr = StringUtils::wstos(characterName);

    THREAD_MAIN;

    if (const auto* data = GetOnlineData())
    {
        if (data->baseId)
        {
            const auto basePos = data->baseId.GetSpaceId().Handle().GetPosition().Handle();
            co_return co_await Undock(basePos, system);
        }

        // TODO: Implement system change with fluf client hook
        co_return Action<MongoResult>{ MongoResult::PerformedSynchronously };
    }

    THREAD_BACKGROUND;

    const auto updateDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("system", static_cast<int>(system.GetValue())))));
    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
}

concurrencpp::result<Action<MongoResult>> CharacterId::Undock(Vector pos, const SystemId system, const Matrix orient) const
{
    if (!system)
    {
        co_return Action<MongoResult>{ cpp::fail(Error::InvalidSystem) };
    }

    const std::string characterNameStr = StringUtils::wstos(characterName);

    THREAD_MAIN;
    if (const auto* data = GetOnlineData())
    {
        if (!data->baseId)
        {
            co_return Action<MongoResult>{ cpp::fail(Error::PlayerNotDocked) };
        }

        // TODO: Implement undock with fluf client hook
        co_return Action<MongoResult>{ MongoResult::PerformedSynchronously };
        ;
    }

    THREAD_BACKGROUND;

    auto euler = orient.ToEuler(true);
    const auto updateDoc = B_MDOC(B_KVP("$set",
                                        B_MDOC(B_KVP("system", static_cast<int>(system.GetValue())),
                                               B_KVP("pos", B_MARR(pos.x, pos.y, pos.z)),
                                               B_KVP("rot", B_MARR(euler.x, euler.y, euler.z)))));

    co_return UpdateCharacterDocument(characterNameStr, updateDoc);
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
