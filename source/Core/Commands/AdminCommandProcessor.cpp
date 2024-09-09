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

std::optional<Task> AdminCommandProcessor::ProcessCommand(ClientId user, const AllowedContext currentContext, std::wstring_view cmd,
                                                          std::vector<std::wstring_view>& paramVector)
{
    for (auto& admin : PluginManager::i()->adminCommands)
    {
        const auto ptr = admin.lock();
        if (auto res = ptr->ProcessCommand(user, currentContext, cmd, paramVector); res.has_value())
        {
            return res;
        }
    }

    // If empty, command not found
    if (const auto result = MatchCommand<commands.size()>(this, user, currentContext, cmd, paramVector); result.has_value())
    {
        return result;
    }

    user.Message(std::format(L"ERR: Command not found. ({})", cmd));
    return std::nullopt;
}

std::optional<Task> AdminCommandProcessor::ProcessCommand(ClientId client, const AllowedContext currentContext, const std::wstring_view commandString)
{
    this->currentContext = currentContext;

    auto params = StringUtils::GetParams(commandString, ' ');

    if (const auto command = params.front(); command.length() < 2)
    {
        return std::nullopt;
    }

    constexpr auto consoleId = L"0"sv;
    std::vector paramsFiltered(params.begin(), params.end());
    paramsFiltered.insert(paramsFiltered.begin() + 1, consoleId); // Inject the 'client id'
    return ProcessCommand(client, currentContext, commandString, paramsFiltered);
}

Task AdminCommandProcessor::SetCash(ClientId client, ClientId characterName, uint amount)
{
    // TODO: Implement
    client.Message(std::format(L"{} cash set to {} credits", characterName, amount));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::GetCash(ClientId client, ClientId target)
{
    // TODO: Implement get cash
    // const auto res = AccountId(characterName).GetCash(characterName).Handle();
    // client.Message(std::format(L"{} has been set {} credits.", characterName, res));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::KickPlayer(ClientId client, ClientId target, std::wstring_view reason)
{
    if (reason.empty())
    {
        target.Kick().Handle();
    }
    else
    {
        target.Kick(reason).Handle();
    }

    client.Message(std::format(L"{} has been successfully kicked. Reason: {}", target, reason));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::BanPlayer(ClientId client, std::wstring_view characterName)
{
    const auto account = AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        client.Message(std::format(L"Unable to find account from character: {}", characterName));
        co_return TaskStatus::Finished;
    }

    (void)account->Ban(0);
    client.Message(std::format(L"{} has been successfully banned.", characterName));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::TempbanPlayer(ClientId client, std::wstring_view characterName, uint durationInDays)
{
    client.Message(L"This command currently does nothing.");
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::UnBanPlayer(ClientId client, std::wstring_view characterName)
{
    const auto account = AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        client.Message(std::format(L"Unable to find account from character: {}", characterName));
        co_return TaskStatus::Finished;
    }
    (void)account->UnBan();
    client.Message(std::format(L"{} has been successfully unbanned.", characterName));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::GetClientId(ClientId client, ClientId target)
{
    client.Message(std::to_wstring(target.GetValue()));

    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::KillPlayer(ClientId client, ClientId target)
{
    target.GetShipId().Handle().Destroy();
    client.Message(std::format(L"{} successfully killed", target));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::SetRep(ClientId client, ClientId target, RepGroupId repGroup, float value)
{
    const auto repId = target.GetReputation().Handle();
    repId.SetAttitudeTowardsRepGroupId(repGroup, value).Handle();

    client.Message(std::format(L"{}'s reputation with {} set to {}", target, repGroup, value));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::ResetRep(ClientId client, ClientId target, RepGroupId repGroup)
{
    // TODO: finish implementing this. Reset to server default

    client.Message(std::format(L"{}'rep to {} reset", target, repGroup));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::GetRep(ClientId client, ClientId target, RepGroupId repGroup)
{
    const auto charRepId = target.GetReputation().Handle();
    const auto rep = repGroup.GetAttitudeTowardsRepId(charRepId).Handle();

    client.Message(std::format(L"{}'reputation to {} is {}", target, repGroup, rep));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::MessagePlayer(ClientId client, ClientId target, const std::wstring_view text)
{
    target.Message(text).Handle();
    client.Message(std::format(L"Message sent to {} successfully sent. Contents: {}", target, text));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::SendSystemMessage(ClientId client, SystemId system, const std::wstring_view text)
{
    system.Message(text).Handle();
    client.Message(std::format(L"Message successfully sent to {}", system));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::SendUniverseMessage(ClientId client, const std::wstring_view text)
{
    FLHook::MessageUniverse(text).Handle();
    client.Message(std::format(L"Message Sent to Server."));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::ListCargo(ClientId client, const ClientId target)
{
    const auto cargo = target.GetEquipCargo().Handle();
    std::wstring res;

    for (auto& item : *cargo)
    {
        if (item.mounted)
        {
            continue;
        }

        res += std::format(L"id={} archid={} count={} mission={} \n", item.id, item.archId, item.count, item.mission ? 1 : 0);
    }

    client.Message(res);

    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::AddCargo(ClientId client, ClientId target, GoodInfo* good, uint count, const bool mission)
{
    target.GetShipId().Handle().AddCargo(good->goodId, count, mission).Handle();

    const auto& im = FLHook::GetInfocardManager();
    client.Message(std::format(L"{} units of {} has been added to {}'s cargo", count, im.GetInfocard(good->idsName), target.GetCharacterName().Handle()));

    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::RenameChar(ClientId client, ClientId target, std::wstring_view newName)
{
    // TODO: Rename is to be reimplemented
    client.Message(std::format(L"{} has been renamed to {}", target.GetCharacterName().Handle(), newName));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::DeleteChar(const ClientId client, const std::wstring_view characterName)
{
    using bsoncxx::builder::basic::make_document;
    using bsoncxx::builder::basic::kvp;

    // Kick the player if they are currently online
    for (auto& player : FLHook::Clients())
    {
        if (player.characterName == characterName)
        {
            player.id.Kick();
            break;
        }
    }

    co_yield TaskStatus::DatabaseAwait;

    const auto& config = FLHook::GetConfig();
    auto db = FLHook::GetDbClient();
    auto accountCollection = Database::GetCollection(db, config.database.accountsCollection);
    auto charactersCollection = Database::GetCollection(db, config.database.charactersCollection);

    auto transaction = db->start_session();
    transaction.start_transaction();

    // TODO: Handle soft delete
    const auto filter = make_document(kvp("characterName", StringUtils::wstos(characterName)));

    try
    {
        if (const auto doc = charactersCollection.find_one_and_delete(filter.view()); doc.has_value())
        {
            bsoncxx::oid characterId = doc->find("_id")->get_oid().value;
            auto accountId = doc->find("accountId")->get_string().value;

            const auto accountFilter = make_document(kvp("_id", accountId));
            const auto deleteCharacter = make_document(kvp("$pull", make_document(kvp("characters", characterId))));
            accountCollection.update_one(accountFilter.view(), deleteCharacter.view());

            transaction.commit_transaction();

            co_yield TaskStatus::FLHookAwait;

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
            co_yield TaskStatus::FLHookAwait;
            client.Message(L"ERR: Character name not found.");
        }
    }
    catch (const mongocxx::exception& ex)
    {
        transaction.abort_transaction();
    }

    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::GetPlayerInfo(ClientId client, const ClientId target)
{
    client.Message(std::format(L"Name: {}, Id: {}, IP: {}, Ping: {}, Base: {}, System: {}\n",
                               target.GetCharacterName().Unwrap(),
                               target.GetValue(),
                               target.GetPlayerIp().Unwrap(),
                               target.GetLatency().Unwrap(),
                               target.GetCurrentBase().Handle().GetName().Unwrap(),
                               target.GetSystemId().Handle().GetName().Unwrap()));

    co_return TaskStatus::Finished;
}

bool AddRole(std::string characterName, std::vector<std::string> roles, std::shared_ptr<void> taskData)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    const auto responseMessage = std::static_pointer_cast<std::wstring>(taskData);

    const auto& config = FLHook::GetConfig();
    const auto dbClient = FLHook::GetDatabase().AcquireClient();
    auto accountCollection = dbClient->database(config.database.dbName).collection(config.database.accountsCollection);
    auto charactersCollection = dbClient->database(config.database.dbName).collection(config.database.charactersCollection);

    const auto findCharacterDoc = make_document(kvp("characterName", characterName));

    const auto characterResult = charactersCollection.find_one(findCharacterDoc.view());
    if (!characterResult.has_value())
    {
        responseMessage->assign(L"ERR: Provided character name not found");
        return true;
    }

    bsoncxx::builder::basic::array roleArray;
    for (auto& role : roles)
    {
        roleArray.append(role);
    }

    const auto findAccountDoc = make_document(kvp("_id", characterResult->find("accountId")->get_string()));
    const auto updateAccountDoc = make_document(kvp("$addToSet", make_document(kvp("gameRoles", make_document(kvp("$each", roleArray.view()))))));
    if (const auto updateResponse = accountCollection.update_one(findAccountDoc.view(), updateAccountDoc.view()); updateResponse->modified_count() != 1)
    {
        responseMessage->assign(L"ERR: Unable to add any role. Account was either invalid or already contained role(s).");
        return true;
    }

    responseMessage->assign(L"Successfully added role(s) to character");
    return true;
}

void AddRoleCallback(std::wstring currentUser, std::shared_ptr<void> taskData)
{
    const auto responseMessage = std::wstring_view(std::static_pointer_cast<std::wstring>(taskData)->data());
    const bool isErr = responseMessage.starts_with(L"ERR: ");
    if (currentUser == ConsoleName)
    {
        isErr ? Logger::Err(responseMessage.substr(5)) : Logger::Info(responseMessage);
        return;
    }

    for (ClientData& client : FLHook::Clients())
    {
        if (currentUser == client.characterName)
        {
            (void)client.id.Message(responseMessage);
            return;
        }
    }
}

Task AdminCommandProcessor::AddRoles(ClientId client, const std::wstring_view target, std::vector<std::wstring_view> roles)
{
    if (target.empty())
    {
        client.Message(L"ERR: Character name not provided.");
        co_return TaskStatus::Finished;
    }

    if (roles.empty())
    {
        client.Message(L"ERR: No roles provided.");
        co_return TaskStatus::Finished;
    }

    for (const ClientData& client : FLHook::Clients())
    {
        // They are online
        if (target == client.characterName)
        {
            (void)client.id.Kick();
            break;
        }
    }

    auto character = StringUtils::wstos(target);
    std::vector<std::string> stringRoles;
    std::ranges::transform(roles, std::back_inserter(stringRoles), [](const std::wstring_view& role) { return StringUtils::wstos(role); });

    TaskScheduler::ScheduleWithCallback<std::wstring>(std::bind(AddRole, character, stringRoles, std::placeholders::_1),
                                                      std::bind(AddRoleCallback, std::wstring(client.GetCharacterName().Handle()), std::placeholders::_1));

    client.Message(L"OK");
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::SetRoles(ClientId client, std::wstring_view target, std::vector<std::wstring_view> roles)
{
    // TODO: pending Character Database rework
    client.Message(L"Successfully set roles.{} roles");
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::DeleteRoles(ClientId client, std::wstring_view target, std::vector<std::wstring_view> roles)
{
    // TODO: pending Character Database rework
    client.Message(L"Successfully removed roles.");
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::LoadPlugin(ClientId client, std::vector<std::wstring_view> pluginNames)
{
    if (pluginNames.empty())
    {
        client.Message(L"No plugin names provided. Use 'all' to load all plugins.");
        co_return TaskStatus::Finished;
    }

    std::wstring res;
    for (const auto p : pluginNames)
    {
        if (p == L"all")
        {
            PluginManager::i()->LoadAll(false);
            res = L"Loaded all possible plugins. See console for details.";
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

    client.Message(res);

    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::UnloadPlugin(ClientId client, std::vector<std::wstring_view> pluginNames)
{
    if (pluginNames.empty())
    {
        client.Message(L"No plugin names provided. Use 'all' to unload all plugins.");
        co_return TaskStatus::Finished;
    }

    std::wstring res;
    for (const auto p : pluginNames)
    {
        if (p == L"all")
        {
            PluginManager::i()->UnloadAll();
            res = L"Unloaded all possible plugins. See console for details.";
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

    client.Message(res);

    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::ReloadPlugin(ClientId client, std::vector<std::wstring_view> pluginNames)
{
    UnloadPlugin(client, pluginNames);
    LoadPlugin(client, pluginNames);

    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::ListPlugins(ClientId client)
{
    auto builder = FLHook::GetResourceManager().NewBuilder();
    builder.WithNpc(L"fc_c_co_fighter_d8")
        .WithReputation(L"fc_x_grp")
        .WithLevel(90)
        .WithRandomName()
        .WithPosition(client.GetShipId().Handle().GetPositionAndOrientation().Handle().first)
        .WithSystem(client.GetSystemId().Handle().GetValue())
        .Spawn();

    if (PluginManager::i()->plugins.empty())
    {
        client.Message(L"No plugins are loaded");
        co_return TaskStatus::Finished;
    }

    std::wstring plugins;
    for (const auto& p : PluginManager::i()->plugins)
    {
        plugins += std::format(L"{} ({})\n", p->GetName(), p->GetShortName());
    }

    client.Message(plugins);
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::Chase(ClientId client, const ClientId target)
{
    if (!target.InSpace())
    {
        client.Message(L"Player not found or not in space");
        co_return TaskStatus::Finished;
    }

    auto [pos, orientation] = target.GetShipId().Handle().GetPositionAndOrientation().Handle();

    pos.y += 100.0f;
    client.GetShipId().Handle().Relocate(pos, orientation);

    client.Message(std::format(L"Jump to system={} x={:.0f} y={:.0f} z={:.0f}", target.GetSystemId().Handle().GetName().Handle(), pos.x, pos.y, pos.z));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::Beam(ClientId client, ClientId target, BaseId base)
{
    target.Beam(base).Handle();
    client.Message(std::format(L"{} beamed to {}", target.GetCharacterName().Handle(), base.GetName().Handle()));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::Pull(ClientId client, ClientId target)
{
    if (!target.InSpace())
    {
        client.Message(L"Player not found or not in space ");
        co_return TaskStatus::Finished;
    }

    if (!client.InSpace())
    {
        client.Message(L"You are currently not in space.");
        co_return TaskStatus::Finished;
    }

    const auto transform = target.GetShipId().Handle().GetPositionAndOrientation().Handle();
    Vector pos = transform.first;
    Matrix orientation = transform.second;

    pos.y += 400;
    target.GetShipId().Handle().Relocate(pos, orientation);

    client.Message(std::format(L"player {} pulled to {} at x={:.0f} y={:.0f} z={:.0f}", target.GetCharacterName().Handle(), client, pos.x, pos.y, pos.z));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::SetDamageType(ClientId client, const std::wstring_view newDamageType)
{
    static std::wstring usage = L"Sets what can be damaged on the server. Valid values are 'None', 'All', PvP, 'PvE'.";
    if (newDamageType.empty())
    {
        client.Message(usage);

        co_return TaskStatus::Finished;
    }

    auto& config = FLHook::GetConfig();
    const auto lower = StringUtils::ToLower(newDamageType);
    if (lower == L"none")
    {
        config.general.damageMode = DamageMode::None;
        Json::Save(config, "flhook.json");
        client.Message(L"Set damage mode to None. No player ship can take damage, but NPCs can still hurt each other.");
        co_return TaskStatus::Finished;
    }

    if (lower == L"all")
    {
        config.general.damageMode = DamageMode::All;
        Json::Save(config, "flhook.json");
        client.Message(L"Set damage mode to All. All ships can take damage.");
        co_return TaskStatus::Finished;
    }

    if (lower == L"pvp")
    {
        config.general.damageMode = DamageMode::PvP;
        Json::Save(config, "flhook.json");
        client.Message(L"Set damage mode to PvP. Players can hurt players, and NPCs can hurt NPCs, but they cannot hurt each other.");
        co_return TaskStatus::Finished;
    }

    if (lower == L"pve")
    {
        config.general.damageMode = DamageMode::PvE;
        Json::Save(config, "flhook.json");
        client.Message(L"Set damage mode to PvE. Players cannot hurt each other, but can hurt and be hurt by NPCs.");
        co_return TaskStatus::Finished;
    }

    client.Message(usage);

    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::Move(ClientId client, ClientId target, const float x, const float y, const float z)
{
    if (x == 0.0f || z == 0.0f)
    {
        client.Message(L"X or Z coordinates were 0. Suppressing to prevent accidental sun-teleporting. If this was intentional, try '0.1 0 0.1' instead.");
        co_return TaskStatus::Finished;
    }

    const auto shipId = target.GetShipId().Unwrap();
    if (!shipId)
    {
        client.Message(L"Target is docked. Unable to move.");
        co_return TaskStatus::Finished;
    }

    shipId.Relocate({ x, y, z });
    client.Message(std::format(L"Moving target to location: {:0f}, {:0f}, {:0f}", x, y, z));
    co_return TaskStatus::Finished;
}

Task AdminCommandProcessor::Help(ClientId client, int page)
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
    co_return TaskStatus::Finished;
}
