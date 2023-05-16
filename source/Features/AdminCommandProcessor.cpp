#include "PCH.hpp"
#include <ranges>
#include "Features/AdminCommandProcessor.hpp"

#include <nlohmann/json.hpp>

#include "Helpers/Player.hpp"
#include "Tools/Utils.hpp"

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
	return nlohmann::json {{"res", std::format("{} has been set {} credits.", characterName, res.value())}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::KickPlayer(std::string_view characterName, std::string_view reason)
{
	// Rights Check

	if (const auto res = Hk::Player::KickReason(stows(ViewToString(characterName)), stows(ViewToString(reason))); res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {{"res", std::format("{} has been successfully kicked. Reason: {}", characterName, reason)}};
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::BanPlayer(std::string_view characterName)
{
	// Rights Check
	if (const auto res = Hk::Player::Ban(stows(ViewToString(characterName)), true); res.has_error())
	{
		return nlohmann::json {{"err", res.error()}};
	}
	return nlohmann::json {{"res", std::format("{} has been successfully banned.", characterName)}};
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
	return nlohmann::json {{"res", std::format("{} cash set to {} credits", characterName, amount)}};
}
