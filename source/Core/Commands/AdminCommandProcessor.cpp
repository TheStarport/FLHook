// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppMemberFunctionMayBeStatic
#include "PCH.hpp"

#include "Core/Commands/AdminCommandProcessor.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/Database.hpp"
#include "API/FLHook/TaskScheduler.hpp"

// TODO: General, a lot of these functions are agnostic about whether or not the player is online and thus has a clientId, so along with the player database
// rework a lot of these functions need to be reworked to account for that.

std::wstring AdminCommandProcessor::ProcessCommand(const std::wstring_view user, const AllowedContext currentContext, std::wstring_view cmd,
                                                   std::vector<std::wstring>& paramVector)
{
    // If empty, command not found
    if (const auto result = MatchCommand<commands.size()>(this, user, currentContext, cmd, paramVector); !result.empty())
    {
        return result;
    }

    for (auto& admin : PluginManager::i()->adminCommands)
    {
        const auto ptr = admin.lock();
        if (auto res = ptr->ProcessCommand(user, currentContext, cmd, paramVector); !res.empty())
        {
            return res;
        }
    }

    return std::format(L"ERR: Command not found. ({})", cmd);
}

std::wstring AdminCommandProcessor::ProcessCommand(const std::wstring_view user, const AllowedContext currentContext, const std::wstring_view commandString)
{
    currentUser = user;
    this->currentContext = currentContext;

    auto params = StringUtils::GetParams(commandString, ' ');

    const auto command = params.front();
    if (command.length() < 2)
    {
        return L"";
    }

    std::vector<std::wstring> paramsFiltered(params.begin(), params.end());
    paramsFiltered.erase(paramsFiltered.begin()); // Remove the first item which is the command

    return ProcessCommand(user, currentContext, command, paramsFiltered);
}

std::wstring AdminCommandProcessor::SetCash(std::wstring_view characterName,
                                            uint amount) // TODO: Need to implement functionality here for offline chars as well.
{
    // Rights check here.
    const auto account = AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        // TODO: fix the error handling
        return L"Error lol";
    }
    account->SetCash(characterName, amount).Handle();

    return std::format(L"{} cash set to {} credits", characterName, amount);
}

std::wstring AdminCommandProcessor::GetCash(std::wstring_view characterName)
{
    // const auto res = AccountId(characterName).GetCash(characterName).Handle();
    // return std::format(L"{} has been set {} credits.", characterName, res);
    return L"Not Implemented";
}

std::wstring AdminCommandProcessor::KickPlayer(std::wstring_view characterName, std::wstring_view reason)
{
    const auto client = ClientId(characterName);
    client.Kick(reason).Handle();
    return std::format(L"{} has been successfully kicked. Reason: {}", characterName, reason);
}

std::wstring AdminCommandProcessor::BanPlayer(std::wstring_view characterName)
{
    const auto account = AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        // TODO: fix the error handling
        return L"Error lol";
    }
    (void)account->Ban(0);
    return std::format(L"{} has been successfully banned.", characterName);
}

std::wstring AdminCommandProcessor::TempbanPlayer(std::wstring_view characterName, uint durationInDays) { return L"This command currently does nothing."; }

std::wstring AdminCommandProcessor::UnBanPlayer(std::wstring_view characterName)
{
    const auto account = AccountId::GetAccountFromCharacterName(characterName);
    if (!account.has_value())
    {
        // TODO: fix the error handling
        return L"Error lol";
    }
    (void)account->UnBan();
    return std::format(L"{} has been successfully unbanned.", characterName);
}

std::wstring AdminCommandProcessor::GetClientId(const std::wstring_view characterName) { return std::to_wstring(ClientId(characterName).GetValue()); }

std::wstring AdminCommandProcessor::KillPlayer(std::wstring_view characterName)
{
    ClientId(characterName).GetShipId().Handle().Destroy();
    return std::format(L"{} successfully killed", characterName);
}

std::wstring AdminCommandProcessor::SetRep(std::wstring_view characterName, std::wstring_view repGroup, float value)
{
    const auto client = ClientId(characterName);
    const auto repId = client.GetReputation().Handle();
    repId.SetAttitudeTowardsRepGroupId(RepGroupId(repGroup), value).Handle();

    return std::format(L"{}'s reputation with {} set to {}", characterName, repGroup, value);
}

std::wstring AdminCommandProcessor::ResetRep(std::wstring_view characterName, std::wstring_view repGroup)
{
    // TODO: Implement as part of character database rework due to removal of iniUtils.

    return std::format(L"{}'rep to {} reset", characterName, repGroup);
}

std::wstring AdminCommandProcessor::GetRep(std::wstring_view characterName, std::wstring_view repGroup)
{
    const auto charRepId = ClientId(characterName).GetReputation().Handle();
    const auto repGroupId = RepGroupId(repGroup);
    const auto rep = repGroupId.GetAttitudeTowardsRepId(charRepId).Handle();

    return std::format(L"{}'reputation to {} is {}", characterName, repGroup, rep);
}

std::wstring AdminCommandProcessor::MessagePlayer(std::wstring_view characterName, const std::wstring_view text)
{
    ClientId(characterName).Message(text).Handle();
    return std::format(L"Message sent to {} successfully sent", characterName);
}

std::wstring AdminCommandProcessor::SendSystemMessage(std::wstring_view systemName, const std::wstring_view text)
{
    SystemId(systemName).Message(text).Handle();
    return std::format(L"Message successfully sent to {}", systemName);
}

std::wstring AdminCommandProcessor::SendUniverseMessage(const std::wstring_view text)
{
    FLHook::MessageUniverse(text).Handle();
    return std::format(L"Message Sent to Server.");
}

std::wstring AdminCommandProcessor::ListCargo(const std::wstring_view characterName)
{
    const auto cargo = ClientId(characterName).GetEquipCargo().Handle();
    std::wstring res;

    for (auto& item : *cargo)
    {
        if (item.mounted)
        {
            continue;
        }

        res += std::format(L"id={} archid={} count={} mission={} \n", item.id, item.archId, item.count, item.mission ? 1 : 0);
    }

    return res;
}

std::wstring AdminCommandProcessor::AddCargo(std::wstring_view characterName, std::wstring_view good, uint count, const bool mission)
{
    const auto goodId = CreateID(StringUtils::wstos(std::wstring(good)).c_str());

    ClientId(characterName).GetShipId().Handle().AddCargo(goodId, count, mission).Handle();
    return std::format(L"{} units of {} has been added to {}'s cargo", count, good, characterName);
}

std::wstring AdminCommandProcessor::RenameChar(std::wstring_view characterName, std::wstring_view newName)
{
    // TODO: Rename is to be reimplemented
    return std::format(L"{} has been renamed to {}", characterName, newName);
}

std::wstring AdminCommandProcessor::DeleteChar(std::wstring_view characterName)
{
    // TODO: pending Character Database rework
    return std::format(L"{} has been successfully deleted", characterName);
}

std::wstring AdminCommandProcessor::GetPlayerInfo(const std::wstring_view characterName)
{
    auto res = ClientId(characterName);

    return std::format(L"Name: {}, Id: {}, IP: {}, Ping: {}, Base: {}, System: {}\n",
                       res.GetCharacterName().Unwrap(),
                       res.GetValue(),
                       res.GetPlayerIp().Unwrap(),
                       res.GetLatency().Unwrap(),
                       res.GetCurrentBase().Handle().GetName().Unwrap(),
                       res.GetSystemId().Handle().GetName().Unwrap());
}

bool AddRole(std::string characterName, std::vector<std::string> roles, std::shared_ptr<void> taskData)
{
    using bsoncxx::builder::basic::kvp;
    using bsoncxx::builder::basic::make_document;

    const auto responseMessage = std::static_pointer_cast<std::wstring>(taskData);

    const auto& config = FLHook::GetConfig();
    const auto dbClient = FLHook::GetDatabase().AcquireClient();
    auto accountCollection = dbClient->database(config.databaseConfig.dbName).collection(config.databaseConfig.accountsCollection);

    const auto findCharacterDoc = make_document(kvp("characterName", characterName));

    const auto characterResult = accountCollection.find_one(findCharacterDoc.view());
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
    if (currentUser == L"console")
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

std::wstring AdminCommandProcessor::AddRoles(const std::wstring_view characterName, std::vector<std::wstring_view> roles)
{
    if (characterName.empty())
    {
        return L"ERR: Character name not provided.";
    }

    if (roles.empty())
    {
        return L"ERR: No roles provided.";
    }

    for (const ClientData& client : FLHook::Clients())
    {
        // They are online
        if (characterName == client.characterName)
        {
            (void)client.id.Kick();
            break;
        }
    }

    auto character = StringUtils::wstos(characterName);
    std::vector<std::string> stringRoles;
    std::ranges::transform(roles, std::back_inserter(stringRoles), [](const std::wstring_view& role) { return StringUtils::wstos(role); });

    TaskScheduler::ScheduleWithCallback<std::wstring>(std::bind(AddRole, character, stringRoles, std::placeholders::_1),
                                                      std::bind(AddRoleCallback, std::wstring(currentUser), std::placeholders::_1));

    return L"OK";
}

std::wstring AdminCommandProcessor::SetRoles(std::wstring_view characterName, std::vector<std::wstring_view> roles)
{
    // TODO: pending Character Database rework
    return L"Successfully set roles.{} roles";
}

std::wstring AdminCommandProcessor::DeleteRoles(std::wstring_view characterName, std::vector<std::wstring_view> roles)
{
    // TODO: pending Character Database rework
    return L"Successfully removed roles.";
}

std::wstring AdminCommandProcessor::LoadPlugin(std::vector<std::wstring_view> pluginNames)
{
    if (pluginNames.empty())
    {
        return L"No plugin names provided";
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

    return res;
}

std::wstring AdminCommandProcessor::UnloadPlugin(std::vector<std::wstring_view> pluginNames)
{
    if (pluginNames.empty())
    {
        return L"No plugin names provided";
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

    return res;
}

std::wstring AdminCommandProcessor::ReloadPlugin(std::vector<std::wstring_view> pluginNames)
{
    std::wstring response = std::format(L"{}\n", UnloadPlugin(pluginNames));
    response += LoadPlugin(pluginNames);
    return response;
}

std::wstring AdminCommandProcessor::ListPlugins()
{
    if (PluginManager::i()->plugins.empty())
    {
        return L"No plugins are loaded";
    }

    std::wstring plugins;
    for (const auto& p : PluginManager::i()->plugins)
    {
        plugins += std::format(L"{} ({})", p->GetName(), p->GetShortName());
    }

    return plugins;
}

std::wstring AdminCommandProcessor::Chase(const std::wstring_view characterName)
{
    const auto admin = ClientId(currentUser);
    const auto target = ClientId(characterName);
    if (!target.InSpace())
    {
        return L"Player not found or not in space";
    }

    const auto transform = target.GetShipId().Handle().GetPositionAndOrientation().Handle();

    Vector pos = transform.first;
    Matrix orientation = transform.second;

    pos.y += 100.0f;
    admin.GetShipId().Handle().Relocate(pos, orientation);

    return std::format(L"Jump to system={} x={:.0f} y={:.0f} z={:.0f}", target.GetSystemId().Handle().GetName().Handle(), pos.x, pos.y, pos.z);
}

std::wstring AdminCommandProcessor::Beam(ClientId target, BaseId base)
{
    target.Beam(base).Handle();
    return std::format(L"{} beamed to {}", target.GetCharacterName().Handle(), base.GetName().Handle());
}

std::wstring AdminCommandProcessor::Pull(std::wstring_view characterName)
{
    const auto admin = ClientId(currentUser);
    const auto target = ClientId(characterName);

    if (!target.InSpace())
    {
        return L"Player not found or not in space ";
    }
    if (!admin.InSpace())
    {
        return L"You are currently not in space.";
    }

    const auto transform = target.GetShipId().Handle().GetPositionAndOrientation().Handle();
    Vector pos = transform.first;
    Matrix orientation = transform.second;

    pos.y += 400;
    target.GetShipId().Handle().Relocate(pos, orientation);

    return std::format(L"player {} pulled to {} at x={:.0f} y={:.0f} z={:.0f}", characterName, currentUser, pos.x, pos.y, pos.z);
}

std::wstring AdminCommandProcessor::SetDamageType(const std::wstring_view newDamageType)
{
    static std::wstring usage = L"Sets what can be damaged on the server. Valid values are 'None', 'All', PvP, 'PvE'.";
    if (newDamageType.empty())
    {
        return usage;
    }

    auto& config = FLHook::GetConfig();
    const auto lower = StringUtils::ToLower(newDamageType);
    if (lower == L"none")
    {
        config.general.damageMode = DamageMode::None;
        Json::Save(config, "flhook.json");
        return L"Set damage mode to None. No player ship can take damage, but NPCs can still hurt each other.";
    }

    if (lower == L"all")
    {
        config.general.damageMode = DamageMode::All;
        Json::Save(config, "flhook.json");
        return L"Set damage mode to All. All ships can take damage.";
    }

    if (lower == L"pvp")
    {
        config.general.damageMode = DamageMode::PvP;
        Json::Save(config, "flhook.json");
        return L"Set damage mode to PvP. Players can hurt players, and NPCs can hurt NPCs, but they cannot hurt each other.";
    }

    if (lower == L"pve")
    {
        config.general.damageMode = DamageMode::PvE;
        Json::Save(config, "flhook.json");
        return L"Set damage mode to PvE. Players cannot hurt each other, but can hurt and be hurt by NPCs.";
    }

    return usage;
}

std::wstring AdminCommandProcessor::Move(ClientId target, const float x, const float y, const float z)
{
    if (x == 0.0f || z == 0.0f)
    {
        return L"X or Z coordinates were 0. Suppressing to prevent accidental sun-teleporting. If this was intentional, try '0.1 0 0.1' instead.";
    }

    const auto shipId = target.GetShipId().Unwrap();
    if (!shipId)
    {
        return L"Target is docked. Unable to move.";
    }

    shipId.Relocate({ x, y, z });
    return std::format(L"Moving target to location: {:0f}, {:0f}, {:0f}", x, y, z);
}