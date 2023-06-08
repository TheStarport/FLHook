// ReSharper disable CppMemberFunctionMayBeConst
// ReSharper disable CppMemberFunctionMayBeStatic
#include "PCH.hpp"
#include <ranges>
#include "Features/AdminCommandProcessor.hpp"

#include "Global.hpp"

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::ProcessCommand(std::wstring_view commandString)
{
	auto params = StringUtils::GetParams(commandString, ' ');

	auto command = params.front();

	std::vector<std::wstring> paramsFiltered(params.begin(), params.end());
	paramsFiltered.erase(paramsFiltered.begin()); // Remove the first item which is the command

	auto res = MatchCommand<commands.size()>(this, command, paramsFiltered);

	// After matching reset perms
	currentContext = AllowedContext::Reset;
	currentUser = L"";

	return res;
}

void AdminCommandProcessor::SetCurrentUser(const std::wstring_view user, const AllowedContext context)
{
	currentUser = user;
	currentContext = context;
}

cpp::result<void, std::wstring_view> AdminCommandProcessor::Validate(const AllowedContext context, std::wstring_view requiredRole)
{
	using namespace magic_enum::bitwise_operators;
	constexpr std::wstring_view invalidPerms = L"ERR: No permission.";
	constexpr std::wstring_view invalidCommand = L"ERR: Command not found.";
	static const std::wstring_view SuperAdminRole = magic_enum::enum_name(DefaultRoles::SuperAdmin);

	// If the current context does not allow command
	if (static_cast<int>(currentContext & context) == 0)
	{
		return {cpp::fail(invalidCommand)};
	}

	const auto credentials = credentialsMap.find(currentUser.data());
	if (credentials == credentialsMap.end())
	{
		// Some how got here and not authenticated!
		return {cpp::fail(invalidPerms)};
	}

	if (std::ranges::find(credentials->second, requiredRole) == credentials->second.end() &&
	    std::ranges::find(credentials->second, SuperAdminRole) == credentials->second.end())
	{
		return {cpp::fail(invalidPerms)};
	}

	// All good!
	return {};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::SetCash(std::wstring_view characterName, uint amount)
{
	// Rights check here.
	const auto playerInitialCash = Hk::Player::GetCash(characterName).Handle();
	const auto res = Hk::Player::AdjustCash(characterName, amount - playerInitialCash);
	return nlohmann::json {{"res", std::format(L"{} cash set to {} credits", characterName, amount)}, {"characterName", characterName}, {"amount", amount}};
};

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::GetCash(std::wstring_view characterName)
{
	// TODO: Rights Check.
	// TODO: Get HK functions to respect views.
	const auto res = Hk::Player::GetCash(characterName).Handle();
	return nlohmann::json {{"res", std::format(L"{} has been set {} credits.", characterName, res)}, {characterName, res}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::KickPlayer(std::wstring_view characterName, std::wstring_view reason)
{
	// Rights Check

	if (const auto res = Hk::Player::KickReason(characterName, reason); res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {
	    {"res", std::format(L"{} has been successfully kicked. Reason: {}", characterName, reason)}, {"KickedCharacter", characterName}, {"reason", reason}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::BanPlayer(std::wstring_view characterName)
{
	// Rights Check
	if (const auto res = Hk::Player::Ban(characterName, true); res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {{"res", std::format(L"{} has been successfully banned.", characterName)}, {"BannedCharacter", characterName}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::TempbanPlayer(std::wstring_view characterName, uint duration)
{
	if (const auto res = Hk::Player::Ban(characterName, true); res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {{"res", std::format(L"{} has been successfully banned. for duration of {}", characterName, duration)},
	    {"characterName", characterName},
	    {"duration", duration}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::UnBanPlayer(std::wstring_view characterName)
{
	if (const auto res = Hk::Player::Ban(characterName, false); res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {{"res", std::format(L"{} has been successfully unbanned.", characterName)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::GetClientId(std::wstring_view characterName)
{
	auto client = Hk::Client::GetClientIdFromCharName(characterName);
	if (client.has_error())
	{
		return nlohmann::json {{"err", client.error()}};
	}
	return nlohmann::json {{"res", client.value()}, {characterName, client.value()}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::KillPlayer(std::wstring_view characterName)
{
	const auto res = Hk::Player::Kill(characterName);
	if (res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {{"res", std::format(L"{} successfully killed", characterName)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::SetRep(std::wstring_view characterName, const std::wstring& repGroup, float value)
{
	const auto res = Hk::Player::SetRep(characterName, repGroup, value);
	if (res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}

	return nlohmann::json {{"res", std::format(L"{}'s reputation with {} set to {}", characterName, repGroup, value)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::ResetRep(std::wstring_view characterName, const std::wstring& repGroup)
{
	const auto res = Hk::Player::ResetRep(characterName);
	if (res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {{"res", std::format(L"{}'rep to {} reset", characterName, repGroup)}, {"repGroup", repGroup}, {"characterName", characterName}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::GetRep(std::wstring_view characterName, const std::wstring& repGroup)
{
	const auto res = Hk::Player::GetRep(characterName, repGroup);

	if (res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {{"res", std::format(L"{}'reputation to {} is {}", characterName, repGroup, res.value())},
	    {"repGroup", repGroup},
	    {"reputation", res.value()},
	    {"characterName", characterName}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::MessagePlayer(std::wstring_view characterName, const std::wstring& text)
{
	if (const auto res = Hk::Chat::Msg(characterName, text); res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {{"res", std::format(L"Message sent to {} successfully sent", characterName)}, {"receiver", characterName}, {"message", text}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::SendSystemMessage(std::wstring_view systemName, const std::wstring& text)
{
	if (const auto res = Hk::Chat::MsgS(std::wstring(systemName), text); res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}

	return nlohmann::json {{"res", std::format(L"Message successfully sent to {}", systemName)}, {"systemName", systemName}, {"message", text}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::SendUniverseMessage(std::wstring_view text)
{
	if (const auto res = Hk::Chat::MsgU(text); res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {{"res", std::format(L"Message Sent to Server.")}, {"message"}, text};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::ListCargo(std::wstring_view characterName)
{
	int holdSize = 0;
	auto cargo = Hk::Player::EnumCargo(characterName, holdSize);
	if (cargo.has_error())
	{
		return nlohmann::json {{"err", cargo.error()}};
	}
	std::wstring res;
	auto array = nlohmann::json::array();

	for (auto& item : cargo.value())
	{
		if (item.mounted)
		{
			continue;
		}
		auto obj = nlohmann::json::object();
		obj["id"] = item.id;
		obj["archId"] = item.archId;
		obj["count"] = item.count;
		obj["isMissionCargo"] = item.mission;
		array.emplace_back(obj);
		res += std::format(L"id={} archid={} count={} mission={} \n", item.id, item.archId, item.count, item.mission ? 1 : 0);
	}

	return nlohmann::json {{"res", res}, {"items", array}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::AddCargo(std::wstring_view characterName, const std::wstring& good, uint count, bool mission)
{
	const auto res = Hk::Player::AddCargo(characterName, good, count, mission);

	if (res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {{"res", std::format(L"{} units of {} has been added to {}'s cargo", count, good, characterName)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::RenameChar(std::wstring_view characterName, const std::wstring& newName)
{
	const auto res = Hk::Player::Rename(characterName, newName, false);
	if (res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}
	return nlohmann::json {{"res", std::format(L"{} has been renamed to {}", characterName, newName)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::DeleteChar(std::wstring_view characterName)
{
	const auto res = Hk::Player::Rename(characterName, L"", true);

	if (res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}

	return nlohmann::json {{"res", std::format(L"{} has been successfully deleted", characterName)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::ReadCharFile(std::wstring_view characterName)
{
	const auto res = Hk::Player::ReadCharFile(characterName);

	if (res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}

	std::wstring charFile;
	auto array = nlohmann::json::array();
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::WriteCharFile(std::wstring_view characterName, const std::wstring& data)
{
	const auto res = Hk::Player::WriteCharFile(characterName, data);
	if (res.has_error())
	{
		return nlohmann::json {{"err", magic_enum::enum_name(res.error())}};
	}

	return nlohmann::json {{"res", std::format(L"Char file of {}, saved", characterName)}};
}

nlohmann::json::object_t AdminCommandProcessor::GeneratePlayerInfoObj(const PlayerInfo& player)
{
	auto obj = nlohmann::json::object();
	obj["character"] = player.character;
	obj["clientId"] = player.client;
	obj["ipAddress"] = player.IP;
	obj["base"] = player.baseName;
	obj["ping"] = player.connectionInfo.roundTripLatencyMS;
	obj["system"] = player.systemName;
	obj["host"] = player.hostname;

	return obj;
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::GetPlayerInfo(std::wstring_view characterName)
{
	const auto res = Hk::Admin::GetPlayerInfo(characterName, false);
	if (res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	auto obj = GeneratePlayerInfoObj(res.value());

	obj["res"] = std::format(L"Name: {}, Id: {}, IP: {}, Host: {}, Ping: {}, Base: {}, System: {}\n",
	    res.value().character,
	    res.value().client,
	    res.value().IP,
	    res.value().hostname,
	    res.value().connectionInfo.roundTripLatencyMS,
	    res.value().baseName,
	    res.value().systemName);
	return obj;
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::GetAllPlayerInfo()
{
	auto arr = nlohmann::json::array();

	for (const auto& p : Hk::Admin::GetPlayers())
	{
		arr.emplace_back(GeneratePlayerInfoObj(p));
	}
	return arr;
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::GetGroupMembers(std::wstring_view characterName)
{
	const auto res = Hk::Player::GetGroupMembers(characterName);
	if (res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	auto arr = nlohmann::json::array();
	for (const auto m : res.value())
	{
		auto obj = nlohmann::json::object();

		obj["id"] = m.client;
		obj["charname"] = m.character;
		arr.emplace_back(obj);
	}
	arr["res"] = "characterData retrieved";

	return arr;
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::AddRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles)
{
	return {};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::SetRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles)
{
	return {};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::DeleteRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles)
{
	return {};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::LoadPlugin(const std::vector<std::wstring_view>& pluginNames)
{
	if (pluginNames.empty())
	{
		return nlohmann::json {{"err", "No plugins provided"}};
	}

	for (const auto p : pluginNames)
	{
		if (p == L"all")
		{
			PluginManager::i()->LoadAll(false);
			break;
		}

		PluginManager::i()->Load(p, false);
	}

	return nlohmann::json {{"res", "plugin(s) successfully loaded"}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::UnloadPlugin(const std::vector<std::wstring_view>& pluginNames)
{
	if (pluginNames.empty())
	{
		return nlohmann::json {{"err", "No plugins provided"}};
	}
	for (const auto p : pluginNames)
	{
		if (p == L"all")
		{
			PluginManager::i()->UnloadAll();
			break;
		}

		PluginManager::i()->Unload(p);
	}

	return nlohmann::json {{"res", "plugin(s) successfully unloaded"}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::ReloadPlugin(const std::vector<std::wstring_view>& pluginNames)
{
	if (pluginNames.empty())
	{
		return nlohmann::json {{"err", "No plugins provided"}};
	}
	for (const auto p : pluginNames)
	{
		if (p == L"all")
		{
			PluginManager::i()->UnloadAll();
			break;
		}

		PluginManager::i()->Unload(p);
	}

	for (const auto p : pluginNames)
	{
		if (p == L"all")
		{
			PluginManager::i()->LoadAll(false);
			break;
		}

		PluginManager::i()->Load(p, false);
	}
	return nlohmann::json {{"res", "plugin(s) successfully reloaded"}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::ListPlugins()
{
	if (PluginManager::i()->plugins.empty())
	{
		nlohmann::json {{"res", "No plugins are loaded"}};
	}
	auto arr = nlohmann::json::array();

	for (const auto& p : PluginManager::i()->plugins)
	{
		auto obj = nlohmann::json::object();
		obj["name"] = p->GetName();

		arr.emplace_back(obj);
	}

	arr["res"] = "Plugins retrieved";
	return arr;
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::Chase(std::wstring_view characterName)
{
	const auto admin = Hk::Admin::GetPlayerInfo(currentUser, false);
	if (admin.has_error())
	{
		return nlohmann::json {{"err", admin.error()}};
	}

	const auto target = Hk::Admin::GetPlayerInfo(characterName, false);
	if (target.has_error() || target.value().ship == 0)
	{
		return nlohmann::json {{"err", "Player not found or not in space"}};
	}
	Vector pos;
	Matrix orientation;

	pub::SpaceObj::GetLocation(target.value().ship, pos, orientation);

	pos.y += 100;
	Hk::Player::RelocateClient(target->client, pos, orientation);

	return nlohmann::json {{"res", std::format(L"Jump to system={} x={:.0f} y={:.0f} z={:.0f}", target.value().systemName, pos.x, pos.y, pos.z)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::Beam(std::wstring_view characterName, const std::wstring& baseName)
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
		return nlohmann::json {{"err", "Invalid Base Name"}};
	}
	const auto base = Hk::Solar::GetBaseByWildcard(baseName);
	if (base.has_error())
	{
		return nlohmann::json {{"err", base.error()}};
	}

	const auto res = Hk::Player::Beam(targetPlayer, base.value()->baseId);
	if (res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}

	return nlohmann::json {{"res", std::format(L"{} beamed to {}", targetPlayer, base.value()->baseId)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::Pull(const std::wstring& characterName)
{
	const auto admin = Hk::Admin::GetPlayerInfo(currentUser, false);
	if (admin.has_error())
	{
		return nlohmann::json {{"err", admin.error()}};
	}
	const auto target = Hk::Admin::GetPlayerInfo(characterName, false);
	if (target.has_error() || target.value().ship == 0)
	{
		return nlohmann::json {{"err", "Player not found or not in space"}};
	}
	Vector pos;
	Matrix orientation;
	pub::SpaceObj::GetLocation(target.value().ship, pos, orientation);
	pos.y += 400;

	Hk::Player::RelocateClient(target->client, pos, orientation);

	return nlohmann::json {{"res", std::format(L"player {} pulled to {} at {},{},{}", characterName, currentUser, pos.x, pos.y, pos.z)}};
}

// cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::Move(const std::wstring& characterName, Vector position)
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