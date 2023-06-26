// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppMemberFunctionMayBeStatic
#include "PCH.hpp"

#include "Core/Commands/AdminCommandProcessor.hpp"
#include "Global.hpp"

std::wstring AdminCommandProcessor::ProcessCommand(std::wstring_view commandString)
{
    auto params = StringUtils::GetParams(commandString, ' ');

    auto command = params.front();

    std::vector<std::wstring> paramsFiltered(params.begin(), params.end());
    paramsFiltered.erase(paramsFiltered.begin()); // Remove the first item which is the command

    auto res = ProcessCommand(command, paramsFiltered);

    // After matching reset perms
    currentContext = AllowedContext::Reset;
    currentUser = L"";

    return res;
}

std::wstring AdminCommandProcessor::ProcessCommand(std::wstring_view cmd, std::vector<std::wstring>& paramVector)
{
    return MatchCommand<commands.size()>(this, cmd, paramVector);
}

void AdminCommandProcessor::SetCurrentUser(const std::wstring_view user, const AllowedContext context)
{
    currentUser = user;
    currentContext = context;
}

cpp::result<void, std::wstring> AdminCommandProcessor::Validate(const AllowedContext context, std::wstring_view requiredRole)
{
    using namespace magic_enum::bitwise_operators;
    const static std::wstring invalidPerms = L"ERR: No permission.";
    const static std::wstring invalidCommand = L"ERR: Command not found.";
    const static std::wstring_view superAdminRole = magic_enum::enum_name(DefaultRoles::SuperAdmin);

    // If the current context does not allow command
    if (static_cast<int>(currentContext & context) == 0)
    {
        return cpp::fail(invalidCommand);
    }

    const auto credentials = credentialsMap.find(currentUser.data());
    if (credentials == credentialsMap.end())
    {
        // Some how got here and not authenticated!
        return cpp::fail(invalidPerms);
    }

    if (std::ranges::find(credentials->second, requiredRole) == credentials->second.end() &&
        std::ranges::find(credentials->second, superAdminRole) == credentials->second.end())
    {
        return cpp::fail(invalidPerms);
    }

    // All good!
    return {};
}

std::wstring AdminCommandProcessor::SetCash(std::wstring_view characterName, uint amount)
{
    // Rights check here.
    const auto playerInitialCash = Hk::Player::GetCash(characterName).Handle();
    Hk::Player::AdjustCash(characterName, static_cast<int>(amount) - playerInitialCash).Handle();
    return std::format(L"{} cash set to {} credits", characterName, amount);
}

std::wstring AdminCommandProcessor::GetCash(std::wstring_view characterName)
{
    const auto res = Hk::Player::GetCash(characterName).Handle();
    return std::format(L"{} has been set {} credits.", characterName, res);
}

std::wstring AdminCommandProcessor::KickPlayer(std::wstring_view characterName, std::wstring_view reason)
{
    Hk::Player::KickReason(characterName, reason).Handle();
    return std::format(L"{} has been successfully kicked. Reason: {}", characterName, reason);
}

std::wstring AdminCommandProcessor::BanPlayer(std::wstring_view characterName)
{
    Hk::Player::Ban(characterName, true).Handle();
    return std::format(L"{} has been successfully banned.", characterName);
}

std::wstring AdminCommandProcessor::TempbanPlayer(std::wstring_view characterName, uint durationInDays) { return L"This command currently does nothing."; }

std::wstring AdminCommandProcessor::UnBanPlayer(std::wstring_view characterName)
{
    Hk::Player::Ban(characterName, false).Handle();
    return std::format(L"{} has been successfully unbanned.", characterName);
}

std::wstring AdminCommandProcessor::GetClientId(std::wstring_view characterName)
{
    const auto id = Hk::Client::GetClientIdFromCharName(characterName).Handle();
    return std::to_wstring(id);
}

std::wstring AdminCommandProcessor::KillPlayer(std::wstring_view characterName)
{
    Hk::Player::Kill(characterName).Handle();
    return std::format(L"{} successfully killed", characterName);
}

std::wstring AdminCommandProcessor::SetRep(std::wstring_view characterName, std::wstring_view repGroup, float value)
{
    Hk::Player::SetRep(characterName, repGroup, value).Handle();
    return std::format(L"{}'s reputation with {} set to {}", characterName, repGroup, value);
}

std::wstring AdminCommandProcessor::ResetRep(std::wstring_view characterName, std::wstring_view repGroup)
{
    Hk::Player::ResetRep(characterName).Handle();
    return std::format(L"{}'rep to {} reset", characterName, repGroup);
}

std::wstring AdminCommandProcessor::GetRep(std::wstring_view characterName, std::wstring_view repGroup)
{
    auto rep = Hk::Player::GetRep(characterName, repGroup).Handle();
    return std::format(L"{}'reputation to {} is {}", characterName, repGroup, rep);
}

std::wstring AdminCommandProcessor::MessagePlayer(std::wstring_view characterName, std::wstring_view text)
{
    Hk::Chat::Msg(characterName, text).Handle();
    return std::format(L"Message sent to {} successfully sent", characterName);
}

std::wstring AdminCommandProcessor::SendSystemMessage(std::wstring_view systemName, std::wstring_view text)
{
    Hk::Chat::MsgS(std::wstring(systemName), text).Handle();
    return std::format(L"Message successfully sent to {}", systemName);
}

std::wstring AdminCommandProcessor::SendUniverseMessage(std::wstring_view text)
{
    Hk::Chat::MsgU(text).Handle();
    return std::format(L"Message Sent to Server.");
}

std::wstring AdminCommandProcessor::ListCargo(std::wstring_view characterName)
{
    int holdSize = 0;
    auto cargo = Hk::Player::EnumCargo(characterName, holdSize).Handle();
    std::wstring res;

    for (auto& item : cargo)
    {
        if (item.mounted)
        {
            continue;
        }

        res += std::format(L"id={} archid={} count={} mission={} \n", item.id, item.archId, item.count, item.mission ? 1 : 0);
    }

    return res;
}

std::wstring AdminCommandProcessor::AddCargo(std::wstring_view characterName, std::wstring_view good, uint count, bool mission)
{
    Hk::Player::AddCargo(characterName, good, count, mission).Handle();
    return std::format(L"{} units of {} has been added to {}'s cargo", count, good, characterName);
}

std::wstring AdminCommandProcessor::RenameChar(std::wstring_view characterName, std::wstring_view newName)
{
    Hk::Player::Rename(characterName, newName, false).Handle();
    return std::format(L"{} has been renamed to {}", characterName, newName);
}

std::wstring AdminCommandProcessor::DeleteChar(std::wstring_view characterName)
{
    Hk::Player::Rename(characterName, L"", true).Handle();
    return std::format(L"{} has been successfully deleted", characterName);
}

std::wstring AdminCommandProcessor::GetPlayerInfo(std::wstring_view characterName)
{
    const auto res = Hk::Admin::GetPlayerInfo(characterName, false).Handle();
    return std::format(L"Name: {}, Id: {}, IP: {}, Host: {}, Ping: {}, Base: {}, System: {}\n",
                       res.character,
                       res.client,
                       res.IP,
                       res.hostname,
                       res.connectionInfo.roundTripLatencyMS,
                       res.baseName,
                       res.systemName);
}

std::wstring AdminCommandProcessor::AddRoles(std::wstring_view characterName, std::vector<std::wstring_view> roles)
{
    Hk::Admin::AddRoles(characterName, roles).Handle();
    return std::format(L"Successfully added {} roles", roles.size());
}

std::wstring AdminCommandProcessor::SetRoles(std::wstring_view characterName, std::vector<std::wstring_view> roles)
{
    Hk::Admin::AddRoles(characterName, roles).Handle();
    return L"Successfully set roles.{} roles";
}

std::wstring AdminCommandProcessor::DeleteRoles(std::wstring_view characterName, std::vector<std::wstring_view> roles)
{
    Hk::Admin::AddRoles(characterName, roles).Handle();
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

std::wstring AdminCommandProcessor::Chase(std::wstring_view characterName)
{
    const auto admin = Hk::Admin::GetPlayerInfo(currentUser, false).Handle();
    const auto target = Hk::Admin::GetPlayerInfo(characterName, false).Handle();
    if (target.ship == 0)
    {
        return L"Player not found or not in space";
    }
    Vector pos;
    Matrix orientation;

    pub::SpaceObj::GetLocation(target.ship, pos, orientation);

    pos.y += 100.0f;
    Hk::Player::RelocateClient(admin.client, pos, orientation);

    return std::format(
        L"Jump to system={} x={:.0f} y={:.0f} z={:.0f}", target.systemName, static_cast<float>(pos.x), static_cast<float>(pos.y), static_cast<float>(pos.z));
}

std::wstring AdminCommandProcessor::Beam(std::wstring_view characterName, std::wstring_view baseName)
{
    std::wstring targetPlayer;

    if (characterName == L"me")
    {
        targetPlayer = currentUser;
    }
    else
    {
        targetPlayer = characterName;
    }

    if (StringUtils::Trim(baseName).empty())
    {
        return L"Invalid Base Name";
    }

    const auto base = Hk::Solar::GetBaseByWildcard(baseName).Handle();

    Hk::Player::Beam(targetPlayer, base->baseId).Handle();
    return std::format(L"{} beamed to {}", targetPlayer, base->baseId);
}

std::wstring AdminCommandProcessor::Pull(std::wstring_view characterName)
{
    const auto admin = Hk::Admin::GetPlayerInfo(currentUser, false).Handle();
    const auto target = Hk::Admin::GetPlayerInfo(characterName, false).Handle();
    if (target.ship == 0)
    {
        return L"Player not found or not in space ";
    }

    Vector pos;
    Matrix orientation;
    pub::SpaceObj::GetLocation(target.ship, pos, orientation);
    pos.y += 400;

    Hk::Player::RelocateClient(target.client, pos, orientation);

    return std::format(L"player {} pulled to {} at x={:.0f} y={:.0f} z={:.0f}",
                       characterName,
                       currentUser,
                       static_cast<float>(pos.x),
                       static_cast<float>(pos.y),
                       static_cast<float>(pos.z));
}

// std::wstring AdminCommandProcessor::Move(std::wstring_view characterName, Vector position)
//{
//	std::wstring targetPlayer;
//
//	if (characterName == L"me")
//	{
//		targetPlayer = currentUser;
//	}
//	else
//	{
//		targetPlayer = characterName;
//	}
//
//	const auto target = Hk::Admin::GetPlayerInfo(targetPlayer, false);
//	if (target.has_error() || target.value().ship == 0)
//	{
//		return nlohmann::json {{"err", "Player not found or not in space"}};
//	}
//	Vector pos;
//	Matrix orientation;
//
//	pub::SpaceObj::GetLocation(target.value().ship, pos,orientation);
//	pos = position;
//	Hk::Player::RelocateClient(target.value().client, pos, orientation);
//	return nlohmann::json {{"res", std::format(L"player {} moved to {},{},{}", characterName, pos.x, pos.y, pos.z)}};
// }
