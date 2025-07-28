// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppMemberFunctionMayBeStatic
#include "PCH.hpp"

#include "Core/Commands/AdminCommandProcessor.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/FLHook/InfocardManager.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "API/FLHook/TaskScheduler.hpp"

// TODO: General, a lot of these functions are agnostic about whether or not the player is online and thus has a clientId, so along with the player database
// rework a lot of these functions need to be reworked to account for that.

std::optional<concurrencpp::result<void>> AdminCommandProcessor::ProcessCommand(ClientId user, const AllowedContext currentContext, std::wstring_view cmd,
                                                                                std::vector<std::wstring_view>& paramVector)
{
    for (auto& admin : PluginManager::i()->adminCommands)
    {
        if (admin.expired())
        {
            continue;
        }

        const auto ptr = admin.lock();
        if (auto res = ptr->ProcessCommand(user, currentContext, cmd, paramVector); res.has_value())
        {
            return res;
        }
    }

    // If empty, command not found
    if (auto result = MatchCommand<commands.size()>(this, user, currentContext, cmd, paramVector); result.has_value())
    {
        return std::move(result);
    }

    user.MessageErr(std::format(L"Command not found. ({})", cmd));
    return std::nullopt;
}

std::optional<concurrencpp::result<void>> AdminCommandProcessor::ProcessCommand(ClientId client, const AllowedContext currentContext,
                                                                                const std::wstring_view clientStr, const std::wstring_view commandString)
{
    this->currentContext = currentContext;

    auto params = StringUtils::GetParams(commandString, ' ');

    if (const auto command = params.front(); command.length() < 2)
    {
        return std::nullopt;
    }

    std::vector paramsFiltered(params.begin(), params.end());
    paramsFiltered.insert(paramsFiltered.begin() + 1, clientStr); // Inject the 'client id'
    return ProcessCommand(client, currentContext, commandString, paramsFiltered);
}

concurrencpp::result<void> AdminCommandProcessor::SetCash(ClientId client, std::wstring_view characterNameView, uint amount)
{
    // If true, they are online and that makes this easy for us
    std::wstring characterName{ characterNameView };
    if (const auto* targetClient = FLHook::GetClientByName(characterName))
    {
        targetClient->id.SetCash(amount).Handle();
        targetClient->id.SaveChar().Handle();
        client.Message(std::format(L"{} cash set to {} credits", characterName, amount));

        if (targetClient->id != client)
        {
            targetClient->id.Message(std::format(L"Your credits have been set to {}.", amount));
        }

        co_return;
    }

    THREAD_BACKGROUND;

    auto success = MongoResult::Failure;
    std::wstring message;
    try
    {
        const auto config = FLHook::GetConfig();
        const auto db = FLHook::GetDbClient();
        auto charactersCollection = Database::GetCollection(db, config->database.charactersCollection);

        // They are offline, lets lookup the needed info
        const auto filter = B_MDOC(B_KVP("characterName", StringUtils::wstos(characterName)));
        const auto update = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("money", static_cast<int>(amount)))));

        const auto result = charactersCollection.update_one(filter.view(), update.view());

        assert(result.has_value());

        if (result->modified_count() == 1)
        {
            success = MongoResult::Success;
        }
        else if (result->matched_count() == 1)
        {
            success = MongoResult::MatchButNoChange;
        }
    }
    catch (const mongocxx::exception& ex)
    {
        message = StringUtils::stows(ex.what());
    }

    THREAD_MAIN;

    if (!message.empty())
    {
        client.MessageErr(message).Handle();
        co_return;
    }

    switch (success)
    {
        case MongoResult::Failure: client.MessageErr(std::format(L"Character {} was not found", characterName)); break;
        case MongoResult::MatchButNoChange: client.Message(std::format(L"{} already has {} cash!", characterName, amount)); break;
        case MongoResult::Success: client.Message(std::format(L"{} cash set to {} credits", characterName, amount));
    }

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::GetCash(ClientId client, std::wstring_view characterNameView)
{
    // If true, they are online and that makes this easy for us
    if (const auto* targetClient = FLHook::GetClientByName(characterNameView))
    {
        auto cash = targetClient->id.GetCash().Handle();
        client.Message(std::format(L"{} has {} credits", characterNameView, cash));

        co_return;
    }

    // They are offline, lets lookup the needed info

    std::wstring characterName{ characterNameView };
    THREAD_BACKGROUND;

    std::wstring err;
    int credits;
    try
    {
        const auto config = FLHook::GetConfig();
        const auto db = FLHook::GetDbClient();
        auto charactersCollection = Database::GetCollection(db, config->database.charactersCollection);

        const auto filter = B_MDOC(B_KVP("characterName", StringUtils::wstos(characterName)));
        const auto projection = B_MDOC(B_KVP("money", 1));

        mongocxx::options::find options;
        options.projection(projection.view());

        if (const auto result = charactersCollection.find_one(filter.view(), options); result.has_value())
        {
            credits = result->find("money")->get_int32();
        }
        else
        {
            err = std::format(L"Character {} was not found", characterName);
        }
    }
    catch (const mongocxx::exception& ex)
    {
        err = StringUtils::stows(ex.what());
    }

    THREAD_MAIN;

    if (!err.empty())
    {
        client.MessageErr(err);
    }
    else
    {
        client.Message(std::format(L"{} has {} credits", characterName, credits));
    }

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::AddCash(ClientId client, std::wstring_view characterNameView, int amount)
{
    if (amount == 0)
    {
        client.Message(L"Invalid cash amount provided");
        co_return;
    }

    // If true, they are online and that makes this easy for us
    if (const auto* targetClient = FLHook::GetClientByName(characterNameView))
    {
        targetClient->id.AddCash(amount);
        client.Message(std::format(L"{} now has {} credits", characterNameView, targetClient->id.GetCash().Handle()));

        co_return;
    }

    std::wstring characterName{ characterNameView };
    THREAD_BACKGROUND;

    auto success = MongoResult::Failure;
    std::wstring message;
    try
    {
        const auto config = FLHook::GetConfig();
        const auto db = FLHook::GetDbClient();
        auto charactersCollection = Database::GetCollection(db, config->database.charactersCollection);

        // They are offline, lets lookup the needed info
        const auto filter = B_MDOC(B_KVP("characterName", StringUtils::wstos(characterName)));

        // We get the current value and do the calculation ourselves, to enforce a minimum of $0 cash - remember the
        // amount could be negative! - and use longs to ensure we can't overflow an integer.

        const auto characterResult = charactersCollection.find_one(filter.view());
        if (!characterResult.has_value())
        {
            THREAD_MAIN;
            client.MessageErr(L"Provided character name not found");
            co_return;
        }

        int currMoney = characterResult->find("money")->get_int32().value;
        long long newMoney = ((long long)currMoney) + amount;

        if (newMoney < 0)
        {
            newMoney = 0;
        }
        else if (newMoney >= pow(2, 31))
        {
            newMoney = pow(2, 31) - 1;
        }

        const auto update = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("money", (int32_t)newMoney))));

        const auto result = charactersCollection.update_one(filter.view(), update.view());

        assert(result.has_value());

        if (result->modified_count() == 1)
        {
            success = MongoResult::Success;
        }
        else if (result->matched_count() == 1)
        {
            success = MongoResult::MatchButNoChange;
        }
    }
    catch (const mongocxx::exception& ex)
    {
        message = StringUtils::stows(ex.what());
    }

    THREAD_MAIN;

    if (!message.empty())
    {
        client.MessageErr(message).Handle();
        co_return;
    }

    switch (success)
    {
        case MongoResult::Failure: client.MessageErr(std::format(L"Character {} was not found", characterName)); break;
        case MongoResult::MatchButNoChange: client.MessageErr(std::format(L"Can't add {} credits to {}!", amount, characterName)); break;
        case MongoResult::Success: client.Message(std::format(L"{} given {} credits", characterName, amount));
    }

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::KickPlayer(ClientId client, ClientId target, std::wstring_view reason)
{
    std::wstring targetCharName = std::wstring(target.GetCharacterName().Handle());
    if (reason.empty())
    {
        target.Kick().Handle();
    }
    else
    {
        target.Kick(reason).Handle();
    }

    client.Message(std::format(L"{} has been successfully kicked. Reason: {}", targetCharName, reason));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::BanPlayer(ClientId client, std::wstring_view characterName)
{
    const auto account = AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        client.MessageErr(std::format(L"Unable to find account from character: {}", characterName));
        co_return;
    }

    (void)account->Ban(0);
    client.Message(std::format(L"{} has been successfully banned.", characterName));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::TempbanPlayer(ClientId client, std::wstring_view characterName, uint durationInDays)
{
    const auto account = AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        client.MessageErr(std::format(L"Unable to find account from character: {}", characterName));
        co_return;
    }

    (void)account->Ban(durationInDays);
    client.Message(std::format(L"{} has been successfully banned for {} days.", characterName, durationInDays));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::UnBanPlayer(ClientId client, std::wstring_view characterName)
{
    const auto account = AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        client.Message(std::format(L"Unable to find account from character: {}", characterName));
        co_return;
    }
    std::wstring stackAllocatedCharacterName = std::wstring(characterName);

    THREAD_BACKGROUND;
    (void)account->UnBan();
    THREAD_MAIN;

    client.Message(std::format(L"{} has been successfully unbanned.", stackAllocatedCharacterName));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::GetClientId(ClientId client, ClientId target)
{
    client.Message(std::to_wstring(target.GetValue()));

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::KillPlayer(ClientId client, ClientId target)
{
    target.GetShip().Handle().Destroy();
    client.Message(std::format(L"{} successfully killed", target));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::SetRep(ClientId client, ClientId target, RepGroupId repGroup, float value)
{
    const auto repId = target.GetReputation().Handle();
    repId.SetAttitudeTowardsRepGroupId(repGroup, value).Handle();

    client.Message(std::format(L"{}'s reputation with {} set to {}", target.GetCharacterName().Handle(), repGroup.GetName().Handle(), value));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::GetRep(ClientId client, ClientId target, RepGroupId repGroup)
{
    const auto charRepId = target.GetReputation().Handle();
    const auto rep = charRepId.GetAttitudeTowardsFaction(repGroup).Handle();

    client.Message(std::format(L"{}'s reputation to {} is {}", target.GetCharacterName().Handle(), repGroup.GetName().Handle(), rep));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::MessagePlayer(ClientId client, ClientId target, const std::wstring_view text)
{
    target.Message(text).Handle();
    client.Message(std::format(L"Message sent to {} successfully. Contents: {}", target.GetCharacterName().Handle(), text));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::SendSystemMessage(ClientId client, SystemId system, const std::wstring_view text)
{
    system.Message(text).Handle();
    client.Message(std::format(L"Message successfully sent to {}", system.GetName().Handle()));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::SendUniverseMessage(ClientId client, const std::wstring_view text)
{
    FLHook::MessageUniverse(text).Handle();
    client.Message(std::format(L"Message sent to server."));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::ListCargo(ClientId client, const ClientId target)
{
    const auto cargo = target.GetEquipCargo().Handle();
    std::wstring res;

    for (auto& item : cargo->equip)
    {
        if (item.mounted)
        {
            continue;
        }

        res += std::format(L"id={} archid={} count={} mission={} \n", item.id, item.archId, item.count, item.mission ? 1 : 0);
    }

    client.Message(res);

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::AddCargo(ClientId client, ClientId target, GoodInfo* good, std::optional<uint> optCount,
                                                           std::optional<bool> optMission)
{
    bool mission = optMission.value_or(false);
    bool count = optCount.value_or(1);
    target.GetShip().Handle().AddCargo(good->goodId, count, mission).Handle();

    const auto& im = FLHook::GetInfocardManager();
    client.Message(std::format(L"{} units of {} has been added to {}'s cargo", count, im->GetInfoName(good->idsName), target.GetCharacterName().Handle()));

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::RenameChar(ClientId client, ClientId target, std::wstring_view newName)
{
    std::wstring newNameStr = newName.data();
    if (newNameStr.empty() || newNameStr == target.GetCharacterName().Unwrap())
    {
        (void)client.MessageErr(L"A new name, that is not the current one, must be provided.");
        co_return;
    }

    if (newNameStr.find(L' ') != std::wstring_view::npos)
    {
        (void)client.MessageErr(L"No whitespaces allowed.");
        co_return;
    }

    if (newNameStr.length() > 23)
    {
        (void)client.MessageErr(L"Name too long, max 23 characters allowed");
        co_return;
    }

    // Ban any name that is numeric and might interfere with commands
    if (const auto numeric = StringUtils::Cast<uint>(newNameStr); numeric < 10000 && numeric != 0)
    {
        (void)client.MessageErr(L"Names that are strictly numerical must be at least 5 digits.");
        co_return;
    }

    std::wstring currName = target.GetCharacterName().Handle().data();

    THREAD_BACKGROUND;

    auto errMsg = co_await AccountManager::CheckCharnameTaken(target, newNameStr);
    if (!errMsg.empty())
    {
        THREAD_MAIN;
        (void)client.MessageErr(errMsg);
        co_return;
    }

    target.Message(L"Renaming, you will be kicked.");
    co_await FLHook::GetTaskScheduler()->Delay(5s);
    target.Kick();
    co_await FLHook::GetTaskScheduler()->Delay(0.5s);
    co_await AccountManager::Rename(currName, newNameStr);

    THREAD_MAIN;

    client.Message(std::format(L"{} has been renamed to {}", currName, newNameStr));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::DeleteChar(const ClientId client, const std::wstring_view characterName)
{
    // Kick the player if they are currently online
    for (auto& player : FLHook::Clients())
    {
        if (player.characterName == characterName)
        {
            player.id.Kick();
            break;
        }
    }

    std::wstring stackAllocatedCharacterName = std::wstring(characterName);

    THREAD_BACKGROUND;

    const auto config = FLHook::GetConfig();
    auto db = FLHook::GetDbClient();
    auto accountCollection = Database::GetCollection(db, config->database.accountsCollection);
    auto charactersCollection = Database::GetCollection(db, config->database.charactersCollection);

    auto transaction = db->start_session();
    transaction.start_transaction();

    // TODO: Handle soft delete
    const auto filter = B_MDOC(B_KVP("characterName", StringUtils::wstos(stackAllocatedCharacterName)));

    try
    {
        if (const auto doc = charactersCollection.find_one_and_delete(filter.view()); doc.has_value())
        {
            bsoncxx::oid characterId = doc->find("_id")->get_oid().value;
            auto accountId = doc->find("accountId")->get_string().value;

            const auto accountFilter = B_MDOC(B_KVP("_id", accountId));
            const auto deleteCharacter = B_MDOC(B_KVP("$pull", B_MDOC(B_KVP("characters", characterId))));
            accountCollection.update_one(accountFilter.view(), deleteCharacter.view());

            transaction.commit_transaction();

            THREAD_MAIN;

            for (const auto& player : FLHook::Clients())
            {
                if (auto found = std::ranges::find(player.account->characters, characterId); found != player.account->characters.end())
                {
                    player.account->characters.erase(found);
                }
            }

            client.Message(L"OK");
        }
        else
        {
            // Not found
            THREAD_MAIN;
            client.MessageErr(L"Character name not found.");
        }
    }
    catch (const mongocxx::exception& ex)
    {
        transaction.abort_transaction();
    }

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::GetPlayerInfo(ClientId client, const ClientId target)
{
    client.Message(std::format(L"Name: {}, Id: {}, IP: {}, Ping: {}, Base: {}, System: {}\n",
                               target.GetCharacterName().Unwrap(),
                               target.GetValue(),
                               target.GetPlayerIp().Unwrap(),
                               target.GetLatency().Unwrap(),
                               target.GetCurrentBase().Handle().GetName().Unwrap(),
                               target.GetSystemId().Handle().GetName().Unwrap()));

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::GetRoles(ClientId client, std::wstring_view target)
{
    if (target.empty())
    {
        client.MessageErr(L"Character name not provided.");
        co_return;
    }

    auto character = StringUtils::wstos(target);
    auto stackAllocatedCharacter = std::wstring(target);
    std::vector<std::string> stringRoles;

    THREAD_BACKGROUND;

    const auto config = FLHook::GetConfig();
    const auto dbClient = FLHook::GetDbClient();
    auto accountCollection = dbClient->database(config->database.dbName).collection(config->database.accountsCollection);
    auto charactersCollection = dbClient->database(config->database.dbName).collection(config->database.charactersCollection);

    const auto findCharacterDoc = B_MDOC(B_KVP("characterName", character));

    const auto characterResult = charactersCollection.find_one(findCharacterDoc.view());
    if (!characterResult.has_value())
    {
        THREAD_MAIN;
        client.MessageErr(L"Provided character name not found");
        co_return;
    }

    const auto findAccountDoc = B_MDOC(B_KVP("_id", characterResult->find("accountId")->get_string()));

    const auto accountResult = accountCollection.find_one(findAccountDoc.view());
    if (!accountResult.has_value())
    {
        THREAD_MAIN;
        client.MessageErr(L"Character was found but account was not!");
        co_return;
    }

    auto gameRoles = accountResult->find("gameRoles");
    if (gameRoles == accountResult->end())
    {
        THREAD_MAIN;
        // this case is hit when someone has never had any roles, i.e. they have no gameRoles field set on their account
        client.MessageErr(std::format(L"Player {} has no admin roles.", stackAllocatedCharacter));
        co_return;
    }

    for (auto role : gameRoles->get_array().value)
    {
        stringRoles.emplace_back(role.get_string().value);
    }

    THREAD_MAIN;

    if (stringRoles.size() == 0)
    {
        // this case can be hit when someone has an empty gameRoles list, e.g. because they used to have roles
        client.MessageErr(std::format(L"Player {} has no admin roles.", stackAllocatedCharacter));
        co_return;
    }

    (void)client.Message(std::format(L"Player {} has the following admin roles:", stackAllocatedCharacter));
    for (std::string role : stringRoles)
    {
        (void)client.Message(std::format(L"- {}", StringUtils::stows(role)));
    }
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::AddRoles(ClientId client, const std::wstring_view target, std::vector<std::wstring_view> roles)
{
    if (target.empty())
    {
        client.MessageErr(L"Character name not provided.");
        co_return;
    }

    if (roles.empty())
    {
        client.MessageErr(L"No roles provided.");
        co_return;
    }

    auto stackAllocatedCharacter = std::wstring(target);
    std::vector<std::string> stringRoles;
    std::ranges::transform(
        roles, std::back_inserter(stringRoles), [](const std::wstring_view& role) { return StringUtils::ToLower(StringUtils::wstos(role)); });

    B_ARR roleArray;
    for (auto role : stringRoles)
    {
        roleArray.append(role);
    }

    const auto updateAccountDoc = B_MDOC(B_KVP("$addToSet", B_MDOC(B_KVP("gameRoles", B_MDOC(B_KVP("$each", roleArray.view()))))));

    auto result = co_await AccountManager::UpdateAccount(stackAllocatedCharacter, updateAccountDoc, "adding roles");
    if (result.has_error())
    {
        client.MessageErr(result.error());
        co_return;
    }

    // Success, lets add roles to the credential map for this character, if they are online
    if (auto targetClient = FLHook::GetClientByName(stackAllocatedCharacter))
    {
        auto& credentials = FLHook::instance->credentialsMap[targetClient->id];
        for (std::string_view role : stringRoles)
        {
            credentials.insert(StringUtils::stows(role));
        }
    }

    client.Message(L"Successfully added role(s) to the account");
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::SetRoles(ClientId client, std::wstring_view target, std::vector<std::wstring_view> roles)
{
    if (target.empty())
    {
        client.MessageErr(L"Character name not provided.");
        co_return;
    }

    if (roles.empty())
    {
        client.MessageErr(L"No roles provided.");
        co_return;
    }

    auto stackAllocatedCharacter = std::wstring(target);
    std::vector<std::string> stringRoles;
    std::ranges::transform(
        roles, std::back_inserter(stringRoles), [](const std::wstring_view& role) { return StringUtils::ToLower(StringUtils::wstos(role)); });

    B_ARR roleArray;
    for (auto role : stringRoles)
    {
        roleArray.append(role);
    }

    const auto updateAccountDoc = B_MDOC(B_KVP("$set", B_MDOC(B_KVP("gameRoles", roleArray.view()))));

    auto result = co_await AccountManager::UpdateAccount(stackAllocatedCharacter, updateAccountDoc, "setting roles");
    if (result.has_error())
    {
        client.MessageErr(result.error());
        co_return;
    }

    // Success, lets set roles in the credential map for this character, if they are online
    if (auto targetClient = FLHook::GetClientByName(stackAllocatedCharacter))
    {
        std::unordered_set<std::wstring> roleSet;
        for (std::string_view role : stringRoles)
        {
            roleSet.insert(StringUtils::stows(role));
        }
        FLHook::instance->credentialsMap[targetClient->id] = roleSet;
    }

    client.Message(L"Successfully set role(s) on the account");
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::DeleteRoles(ClientId client, std::wstring_view target, std::vector<std::wstring_view> roles)
{
    if (target.empty())
    {
        client.MessageErr(L"Character name not provided.");
        co_return;
    }

    if (roles.empty())
    {
        client.MessageErr(L"No roles provided.");
        co_return;
    }

    auto stackAllocatedCharacter = std::wstring(target);
    std::vector<std::string> stringRoles;
    std::ranges::transform(
        roles, std::back_inserter(stringRoles), [](const std::wstring_view& role) { return StringUtils::ToLower(StringUtils::wstos(role)); });

    B_ARR roleArray;
    for (auto role : stringRoles)
    {
        roleArray.append(role);
    }

    const auto updateAccountDoc = B_MDOC(B_KVP("$pull", B_MDOC(B_KVP("gameRoles", B_MDOC(B_KVP("$in", roleArray.view()))))));

    auto result = co_await AccountManager::UpdateAccount(stackAllocatedCharacter, updateAccountDoc, "deleting roles");
    if (result.has_error())
    {
        client.MessageErr(result.error());
        co_return;
    }

    // Success, lets remove roles from the credential map for this character, if they are online
    if (auto targetClient = FLHook::GetClientByName(stackAllocatedCharacter))
    {
        auto& credentials = FLHook::instance->credentialsMap[targetClient->id];
        for (std::string_view role : stringRoles)
        {
            credentials.erase(StringUtils::stows(role));
        }
    }

    client.Message(L"Successfully removed roles.");
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::LoadPlugin(ClientId client, std::vector<std::wstring_view> pluginNames)
{
    if (pluginNames.empty())
    {
        client.Message(L"No plugin names provided. Use 'all' to load all plugins.");
        co_return;
    }

    std::wstring res;
    for (const auto p : pluginNames)
    {
        if (p == L"all")
        {
            PluginManager::i()->LoadAll(false);
            res = L"Loaded all possible plugins. See console for details.\n";
            break;
        }

        if (!PluginManager::i()->Load(p, false))
        {
            res += std::format(L"Plugin not loaded: {}. See console for details.\n", p);
        }
        else
        {
            res += std::format(L"Plugin loaded: {}.\n", p);
        }
    }

    client.Message(res.substr(0, res.length() - 1));

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::UnloadPlugin(ClientId client, std::vector<std::wstring_view> pluginNames)
{
    if (pluginNames.empty())
    {
        client.Message(L"No plugin names provided. Use 'all' to unload all plugins.");
        co_return;
    }

    std::wstring res;
    for (const auto p : pluginNames)
    {
        if (p == L"all")
        {
            PluginManager::i()->UnloadAll();
            res = L"Unloaded all possible plugins. See console for details.\n";
            break;
        }

        if (!PluginManager::i()->Unload(p))
        {
            res += std::format(L"Plugin not unloaded: {}. See console for details.\n", p);
        }
        else
        {
            res += std::format(L"Plugin unloaded: {}.\n", p);
        }
    }

    client.Message(res.substr(0, res.length() - 1));

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::ReloadPlugin(ClientId client, std::vector<std::wstring_view> pluginNames)
{
    std::vector<std::wstring> pluginFileNames;
    for (auto& pluginName : pluginNames)
    {
        const auto plugin = PluginManager::i()->GetPlugin(pluginName);
        if (plugin.expired())
        {
            continue;
        }

        std::array<wchar_t, MAX_PATH> path{};
        const auto size = GetModuleFileNameW(plugin.lock()->dll, path.data(), MAX_PATH);
        if (!size)
        {
            continue;
        }

        std::wstring dllName{ path.data(), size };
        pluginFileNames.emplace_back(dllName.substr(dllName.find_last_of('\\') + 1));
    }

    UnloadPlugin(client, pluginNames);

    std::vector<std::wstring_view> pluginPaths;
    std::ranges::transform(pluginFileNames, std::back_inserter(pluginPaths), [](const std::wstring_view pluginPath) { return pluginPath; });

    LoadPlugin(client, pluginPaths);

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::ListPlugins(ClientId client)
{
    if (PluginManager::i()->plugins.empty())
    {
        client.Message(L"No plugins are loaded");
        co_return;
    }

    std::wstring plugins;
    for (const auto& p : PluginManager::i()->plugins)
    {
        plugins += std::format(L"{} ({})\n", p->GetName(), p->GetShortName());
    }

    client.Message(plugins.substr(0, plugins.length() - 1));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::Chase(ClientId client, const ClientId target)
{
    if (!target.InSpace())
    {
        client.Message(L"Player not found or not in space");
        co_return;
    }

    auto [pos, orientation] = target.GetShip().Handle().GetPositionAndOrientation().Handle();

    pos.y += 100.0f;
    client.GetShip().Handle().Relocate(pos, orientation);

    client.Message(std::format(L"Jump to system={} x={:.0f} y={:.0f} z={:.0f}", target.GetSystemId().Handle().GetName().Handle(), pos.x, pos.y, pos.z));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::Beam(ClientId client, ClientId target, BaseId base)
{
    target.Beam(base).Handle();
    client.Message(std::format(L"{} beamed to {}", target.GetCharacterName().Handle(), base.GetName().Handle()));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::Pull(ClientId client, ClientId target)
{
    if (!target.InSpace())
    {
        client.Message(L"Player not found or not in space ");
        co_return;
    }

    if (!client.InSpace())
    {
        client.Message(L"You are currently not in space.");
        co_return;
    }

    const auto transform = target.GetShip().Handle().GetPositionAndOrientation().Handle();
    Vector pos = transform.first;
    Matrix orientation = transform.second;

    pos.y += 400;
    target.GetShip().Handle().Relocate(pos, orientation);

    client.Message(std::format(
        L"Player {} pulled to {} at x={:.0f} y={:.0f} z={:.0f}",
        target.GetCharacterName().Handle(), client.GetCharacterName().Handle(),
        pos.x, pos.y, pos.z));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::SetDamageType(ClientId client, const std::wstring_view newDamageType)
{
    static std::wstring usage = L"Sets what can be damaged on the server. Valid values are 'None', 'All', PvP, 'PvE'.";
    if (newDamageType.empty())
    {
        client.Message(usage);

        co_return;
    }

    const auto config = FLHook::GetConfig();
    const auto lower = StringUtils::ToLower(newDamageType);
    if (lower == L"none")
    {
        config->general.damageMode = DamageMode::None;
        Json::Save(config, "flhook.json");
        client.Message(L"Set damage mode to None. No player ship can take damage, but NPCs can still hurt each other.");
        co_return;
    }

    if (lower == L"all")
    {
        config->general.damageMode = DamageMode::All;
        Json::Save(config, "flhook.json");
        client.Message(L"Set damage mode to All. All ships can take damage.");
        co_return;
    }

    if (lower == L"pvp")
    {
        config->general.damageMode = DamageMode::PvP;
        Json::Save(config, "flhook.json");
        client.Message(L"Set damage mode to PvP. Players can hurt players, and NPCs can hurt NPCs, but they cannot hurt each other.");
        co_return;
    }

    if (lower == L"pve")
    {
        config->general.damageMode = DamageMode::PvE;
        Json::Save(config, "flhook.json");
        client.Message(L"Set damage mode to PvE. Players cannot hurt each other, but can hurt and be hurt by NPCs.");
        co_return;
    }

    client.Message(usage);

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::Move(ClientId client, ClientId target, const float x, const float y, const float z)
{
    const auto shipId = target.GetShip().Unwrap();
    if (!shipId)
    {
        client.Message(L"Target is docked. Unable to move.");
        co_return;
    }

    if (x == 0.0f && y == 0.0f && z == 0.0f)
    {
        shipId.Relocate({});
    }
    else
    {
        shipId.Relocate({{ x, y, z }});
    }
    client.Message(std::format(L"Moving target to location: {:0.0f}, {:0.0f}, {:0.0f}", x, y, z));
    co_return;
}

#include <cpptrace/basic.hpp>
concurrencpp::result<void> AdminCommandProcessor::Help(ClientId client, std::optional<int> optPage)
{
    constexpr int itemsPerPage = 20;
    const auto& pm = PluginManager::i();

    struct ModuleInfo
    {
            const AbstractAdminCommandProcessor* processor;
            const Plugin* plugin;
            int startingCommandIndex;
            int endingCommandIndex;
    };

    // list of pointers and their starting command index
    std::vector<ModuleInfo> processors;
    processors.reserve(pm->plugins.size() + 1);

    // Add core and set starting indexes
    int commandIndex = commands.size();
    int totalCommands = commands.size();
    // We set the index here to -1 so we have 0 later, easy way to ensure we have the right amount of items
    processors.emplace_back(this, nullptr, 0, commandIndex);

    for (const auto& plugin : pm->plugins)
    {
        if (const auto cmdProcessor = dynamic_cast<const AbstractAdminCommandProcessor*>(plugin.get()); cmdProcessor)
        {
            const auto& cmds = cmdProcessor->GetAdminCommands();
            totalCommands += cmds.size();
            processors.emplace_back(cmdProcessor, plugin.get(), commandIndex, commandIndex + cmds.size());
            commandIndex += cmds.size();
        }
    }

    // Divide the total commands against items per page, rounding up
    const auto div = std::div(totalCommands, itemsPerPage);
    const int totalPages = std::clamp(div.rem ? div.quot + 1 : div.quot, 1, 100);
    int page = optPage.value_or(0);
    if (page > totalPages)
    {
        page = totalPages;
    }
    else if (page < 1)
    {
        page = 1;
    }

    std::wstring response = std::format(L"Displaying help commands, page {} of {}", page, totalPages);
    for (const int startingIndex = commandIndex = itemsPerPage * (page - 1); const auto& info : processors)
    {
        if (commandIndex > startingIndex + itemsPerPage)
        {
            break;
        }

        if (info.endingCommandIndex < commandIndex)
        {
            continue;
        }

        for (auto& cmds = info.processor->GetAdminCommands();
             auto& cmd : cmds | std::ranges::views::drop(std::clamp(commandIndex - info.startingCommandIndex, 0, INT_MAX)))
        {
            if (++commandIndex > startingIndex + itemsPerPage)
            {
                break;
            }

            std::wstring_view description = std::get<2>(cmd);
            std::wstring_view usage = std::get<1>(cmd);

            response += std::format(L"\n{} - {}", usage, description);
        }
    }

    client.Message(response);
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::SetAccTransferCode(ClientId client, std::wstring_view characterNameView, std::wstring_view code)
{
    if (code.length() == 0)
    {
        (void)client.MessageErr(L"Code too small, set to none to clear.");
        co_return;
    }

    std::string stackAllocatedCharacterName = StringUtils::wstos(characterNameView);
    std::wstring stackAllocatedCode = std::wstring(code);

    THREAD_BACKGROUND;

    const auto config = FLHook::GetConfig();
    const auto dbClient = FLHook::GetDbClient();
    auto charactersCollection = dbClient->database(config->database.dbName).collection(config->database.charactersCollection);

    const auto findCharacterDoc = B_MDOC(B_KVP("characterName", stackAllocatedCharacterName));

    const auto characterResult = charactersCollection.find_one(findCharacterDoc.view());
    if (!characterResult.has_value())
    {
        THREAD_MAIN;
        client.MessageErr(L"Provided character name not found");
        co_return;
    }

    const auto findCharactersDoc = B_MDOC(B_KVP("accountId", characterResult->find("accountId")->get_string()));

    for (auto cursor = charactersCollection.find(findCharactersDoc.view()); const auto& doc : cursor)
    {
        std::string targetCharacter = std::string(doc.find("characterName")->get_string().value);
        if (targetCharacter.empty())
        {
            continue;
        }

        std::wstring targetWideCharacter = StringUtils::stows(targetCharacter);

        if (stackAllocatedCode == L"none")
        {
            if (AccountManager::ClearCharacterTransferCode(targetWideCharacter))
            {
                (void)client.Message(std::format(L"OK Transferchar code cleared on {}", targetWideCharacter));
            }
            else
            {
                (void)client.MessageErr(std::format(L"Database error encountered whilst clearing transferchar code on {}", targetWideCharacter));
            }
        }
        else
        {
            if (AccountManager::SetCharacterTransferCode(targetWideCharacter, stackAllocatedCode))
            {
                (void)client.Message(std::format(L"OK Transferchar code set to {} on {}", stackAllocatedCode, targetWideCharacter));
            }
            else
            {
                (void)client.MessageErr(std::format(L"Database error encountered whilst setting transferchar code on {}", targetWideCharacter));
            }
        }
    }

    THREAD_MAIN;
    co_return;
}