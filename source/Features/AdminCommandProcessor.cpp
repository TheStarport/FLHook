#include "PCH.hpp"
#include <ranges>
#include "Features/AdminCommandProcessor.hpp"
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
	}
}

cpp::result<nlohmann::json, nlohmann::json> AdminCommandProcessor::SetCash(std::string_view characterName, uint amount)
{
}