// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppMemberFunctionMayBeStatic
#include "PCH.hpp"

#include "Core/Commands/AdminCommandProcessor.hpp"

#include "API/FLHook/AccountManager.hpp" // TODO: Swap account manager updates to use AccountId
#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/FLHook/InfocardManager.hpp"
#include "API/FLHook/TaskScheduler.hpp"
#include "Core/PluginManager.hpp"
#include "Defs/FLHookConfig.hpp"
#include "Defs/Database/MongoResult.hpp"

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

concurrencpp::result<void> AdminCommandProcessor::SetCash(ClientId client, CharacterId character, uint amount)
{
    const auto result = (co_await character.SetCash(static_cast<int>(amount))).Handle();

    THREAD_MAIN;

    switch (result)
    {
        case MongoResult::MatchButNoChange: client.Message(std::format(L"{} already has {} cash!", character.GetValue(), amount)); break;
        case MongoResult::Success: client.Message(std::format(L"{} cash set to {} credits", character.GetValue(), amount)); break;
        default: break;
    }

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::GetCash(ClientId client, CharacterId characterName)
{
    auto cash = (co_await characterName.GetCash()).Handle();
    THREAD_MAIN;

    client.ToastMessage(L"Cash", std::format(L"{} has {} credits", characterName, cash));
}

concurrencpp::result<void> AdminCommandProcessor::AddCash(const ClientId client, CharacterId characterName, int amount)
{
    if (amount == 0)
    {
        client.ToastMessage(L"Invalid Input", L"Invalid cash amount provided");
        co_return;
    }

    auto result = (co_await characterName.AddCash(amount)).Handle();
    THREAD_MAIN;
    switch (result)
    {
        case MongoResult::PerformedSynchronously:
        case MongoResult::Success: client.ToastMessage(L"Cash Added", std::format(L"{} given {} credits", characterName, amount)); break;
        default: break;
    }

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::KickPlayer(ClientId client, ClientId target, std::wstring_view reason)
{
    auto targetCharName = target.GetCharacterId().Handle().GetValue();
    if (reason.empty())
    {
        target.Kick().Handle();
    }
    else
    {
        target.Kick(reason).Handle();
    }

    client.ToastMessage(L"Player Kicked", std::format(L"{} has been successfully kicked. Reason: {}", targetCharName, reason));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::BanPlayer(ClientId client, CharacterId characterName)
{
    const auto account = co_await AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        client.MessageErr(std::format(L"Unable to find account from character: {}", characterName));
        co_return;
    }

    (void)account->Ban(0);
    client.Message(std::format(L"{} has been successfully banned.", characterName));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::TempbanPlayer(ClientId client, CharacterId characterName, uint durationInDays)
{
    const auto account = co_await AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        THREAD_MAIN;

        client.MessageErr(std::format(L"Unable to find account from character: {}", characterName));
        co_return;
    }

    co_await account->Ban(durationInDays);
    // TODO: Validate it was successful
    THREAD_MAIN;
    client.Message(std::format(L"{} has been successfully banned for {} days.", characterName, durationInDays));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::UnBanPlayer(ClientId client, CharacterId characterName)
{
    const auto account = co_await AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        THREAD_MAIN;
        client.Message(std::format(L"Unable to find account from character: {}", characterName));
        co_return;
    }

    account->UnBan();

    THREAD_MAIN;
    client.Message(std::format(L"{} has been successfully unbanned.", characterName));
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

    client.Message(std::format(L"{}'s reputation with {} set to {}", target.GetCharacterId().Handle(), repGroup.GetName().Handle(), value));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::GetRep(ClientId client, ClientId target, RepGroupId repGroup)
{
    const auto charRepId = target.GetReputation().Handle();
    const auto rep = charRepId.GetAttitudeTowardsFaction(repGroup).Handle();

    client.Message(std::format(L"{}'s reputation to {} is {}", target.GetCharacterId().Handle(), repGroup.GetName().Handle(), rep));
    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::MessagePlayer(ClientId client, ClientId target, const std::wstring_view text)
{
    target.Message(text).Handle();
    client.Message(std::format(L"Message sent to {} successfully. Contents: {}", target.GetCharacterId().Handle(), text));
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
    client.Message(std::format(L"{} units of {} has been added to {}'s cargo", count, im->GetInfoName(good->idsName), target.GetCharacterId().Handle()));

    co_return;
}

concurrencpp::result<void> AdminCommandProcessor::RenameChar(ClientId client, CharacterId targetCharacter, const std::wstring_view newName)
{
    const std::wstring newNameStr = newName.data();
    if (newNameStr.empty() || newNameStr == targetCharacter.GetValue())
    {
        (void)client.MessageErr(L"A new name, that is not the current one, must be provided.");
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

    if (co_await CharacterId::CharacterExists(newNameStr))
    {
        THREAD_MAIN;
        client.MessageErr(L"Character already exists.");
        co_return;
    }

    (co_await targetCharacter.Rename(newNameStr)).Handle();
    THREAD_MAIN;
}

concurrencpp::result<void> AdminCommandProcessor::DeleteChar(const ClientId client, const CharacterId characterName)
{
    // Kick the player if they are currently online
    for (auto& player : FLHook::Clients())
    {
        if (player.characterId == characterName)
        {
            player.id.Kick();
            break;
        }
    }

    auto result = (co_await characterName.Delete()).Handle();
    THREAD_MAIN;

    if (result == MongoResult::Success)
    {
        client.Message(std::format(L"{} has been successfully deleted", characterName));
        co_return;
    }

    client.Message(L"Failed to delete character");
}

concurrencpp::result<void> AdminCommandProcessor::GetPlayerInfo(ClientId client, const ClientId target)
{
    client.Message(std::format(L"Name: {}, Id: {}, IP: {}, Ping: {}, Base: {}, System: {}\n",
                               target.GetCharacterId().Unwrap(),
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

    if (res.empty())
    {
        client.Message(L"No plugins found with the specified names");
    }
    else
    {
        client.Message(res);
    }
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

concurrencpp::result<void> AdminCommandProcessor::Beam(const ClientId client, const ClientId target, StrToEnd baseStr)
{
    const auto base = BaseId(baseStr.end, true);
    if (!base)
    {
        client.ToastMessage(L"Failure", std::format(L"Could not find base '{}'", baseStr.end), ToastType::Error);
        co_return;
    }

    target.Beam(base).Handle();
    client.ToastMessage(L"Beam Successful", std::format(L"{} beamed to {}", target.GetCharacterId().Handle(), base.GetName().Handle()));
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
        L"Player {} pulled to {} at x={:.0f} y={:.0f} z={:.0f}", target.GetCharacterId().Handle(), client.GetCharacterId().Handle(), pos.x, pos.y, pos.z));
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
        shipId.Relocate({
            { x, y, z }
        });
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

concurrencpp::result<void> AdminCommandProcessor::SetCharacterTransferCode(ClientId client, CharacterId character, std::wstring_view code)
{
    if (code.length() == 0)
    {
        (void)client.MessageErr(L"Code too small, set to none to clear.");
        co_return;
    }

    auto stackAllocatedCode = std::wstring(code);
    const auto result = (co_await character.SetTransferCode(code)).Handle();

    THREAD_MAIN;

    if (result == MongoResult::Success)
    {
        client.ToastMessage(L"Code Set", std::format(L"Character transfer code has been set to {}", stackAllocatedCode));
    }
    else if (result == MongoResult::MatchButNoChange)
    {
        client.ToastMessage(L"Same Code", L"Character already had this code set.", ToastType::Warning);
    }
    else
    {
        client.ToastMessage(L"Failure", L"Failed to set character transfer code", ToastType::Error);
    }
}
