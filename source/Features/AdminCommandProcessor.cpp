#include "PCH.hpp"
#include <ranges>
#include "Features/AdminCommandProcessor.hpp"
#include <nlohmann/json.hpp>
#include "Helpers/Client.hpp"
#include "Helpers/Player.hpp"
#include "Helpers/Chat.hpp"

void AdminCommandProcessor::ProcessCommand(std::string_view commandString)
{
	auto fullCommand = commandString | std::ranges::views::split(' ') |
	    std::ranges::views::transform([](auto&& rng) { return std::string_view(&*rng.begin(), std::ranges::distance(rng)); });

	const auto command = fullCommand.front();

	std::vector<std::string> params(fullCommand.begin(), fullCommand.end());
	params.erase(params.begin()); // Remove the first item which is the command

	MatchCommand<commands.size()>(this, command, params);
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::GetCash(std::string_view characterName)
{
	// TODO: Rights Check.
	// TODO: Get HK functions to respect views.

	const auto res = Hk::Player::GetCash(stows(ViewToString(characterName)));
	if (res.has_error())
	{
		return cpp::fail(nlohmann::json {{"err", res.error()}});
	}
	return nlohmann::json {{"res", std::format("{} has been set {} credits.", characterName, res.value()), {characterName, res.value}}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::KickPlayer(std::string_view characterName, std::string_view reason)
{
	// Rights Check

	if (const auto res = Hk::Player::KickReason(stows(ViewToString(characterName)), stows(ViewToString(reason))); res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {
	    {"res", std::format("{} has been successfully kicked. Reason: {}", characterName, reason)}, {"KickedCharacter", characterName}, {"reason", reason}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::BanPlayer(std::string_view characterName)
{
	// Rights Check
	if (const auto res = Hk::Player::Ban(stows(ViewToString(characterName)), true); res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {{"res", std::format("{} has been successfully banned.", characterName)}, {"BannedCharacter", characterName}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::TempbanPlayer(std::string_view characterName, uint duration)
{
	if (const auto res = Hk::Player::Ban(stows(ViewToString(characterName)), true); res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {{"res", std::format("{} has been successfully banned. for duration of {}", characterName, duration)},
	    {"characterName", characterName},
	    {"duration", duration}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::UnBanPlayer(std::string_view characterName)
{
	if (const auto res = Hk::Player::Ban(stows(ViewToString(characterName)), false); res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {{"res", std::format("{} has been successfully unbanned.", characterName)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::GetClientId(std::string_view characterName)
{
	auto client = Hk::Client::GetClientIdFromCharName(stows(ViewToString(characterName)));
	if (client.has_error())
	{
		return nlohmann::json {{"err", client.error()}};
	}
	return nlohmann::json {{"res", client.value()}, {characterName, client.value()}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::KillPlayer(std::string_view characterName)
{
	const auto res = Hk::Player::Kill(stows(ViewToString(characterName)));
	if (res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {{"res", std::format("{} successfully killed", characterName)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::SetRep(std::string_view characterName, const std::wstring& repGroup, float value)
{
	const auto res = Hk::Player::SetRep(stows(ViewToString(characterName)), repGroup, value);
	if (res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}

	return nlohmann::json {{"res", std::format("{}'s reputation with {} set to {}", characterName, wstos(repGroup), value)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::ResetRep(std::string_view characterName, const std::wstring& repGroup)
{
	const auto res = Hk::Player::ResetRep(stows(ViewToString(characterName)));
	if (res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {
	    {"res", std::format("{}'rep to {} reset", characterName, wstos(repGroup))}, {"repGroup", repGroup}, {"characterName", characterName}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::GetRep(std::string_view characterName, const std::wstring& repGroup)
{
	const auto res = Hk::Player::GetRep(stows(ViewToString(characterName)), repGroup);

	if (res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {{"res", std::format("{}'reputation to {} is {}", characterName, wstos(repGroup), res.value())},
	    {"repGroup", repGroup},
	    {"reputation", res.value()},
	    {"characterName", characterName}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::MessagePlayer(std::string_view characterName, const std::wstring& text)
{
	if (const auto res = Hk::Chat::Msg(stows(ViewToString(characterName)), text); res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {{"res", std::format("Message sent to {} successfully sent", characterName)}, {"receiver", characterName}, {"message", text}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::SendSystemMessage(const std::wstring& systemName, const std::wstring& text)
{
	if (const auto res = Hk::Chat::MsgS(systemName, text); res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {{"res", std::format("Message successfully sent to {}", systemName)}, {"systemName", systemName}, {"message", text}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::SendUniverseMessage(const std::wstring text)
{
	if (const auto res = Hk::Chat::MsgU(text); res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {{"res", std::format("Message Sent to Server.")}, {"message"}, text};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::SetCash(std::string_view characterName, uint amount)
{
	// Rights check here.
	const auto playerInitialCash = Hk::Player::GetCash(stows(ViewToString(characterName)));

	if (playerInitialCash.has_error())
	{
		return cpp::fail(nlohmann::json {{"err", playerInitialCash.error()}});
	}
	const auto res = Hk::Player::AdjustCash(stows(ViewToString(characterName)), amount - playerInitialCash.value());
	if (res.has_error())
	{
		return cpp::fail(nlohmann::json {{"err", res.error()}});
	}
	return nlohmann::json {{"res", std::format("{} cash set to {} credits", characterName, amount)}, {"characterName", characterName}, {"amount", amount}};
};