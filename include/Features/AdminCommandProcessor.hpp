#pragma once

#include <nlohmann/json.hpp>

class AdminCommandProcessor
{
	std::unordered_map<std::string, std::vector<std::string_view>> credentialsMap;

#define AddCommand(str, func) { std::string_view(str), wrapper<decltype(&AdminCommandProcessor::func), &AdminCommandProcessor::func>::ProcessParam }

	cpp::result<nlohmann::json, nlohmann::json> SetCash(std::string_view characterName, uint amount);
	cpp::result<nlohmann::json, nlohmann::json> GetCash(std::string_view characterName);
	cpp::result<nlohmann::json, nlohmann::json> KickPlayer(std::string_view characterName, std::string_view reason);
	cpp::result<nlohmann::json, nlohmann::json> BanPlayer(std::string_view characterName);
	cpp::result<nlohmann::json, nlohmann::json> TempbanPlayer(std::string_view characterName, uint duration);
	cpp::result<nlohmann::json, nlohmann::json> UnBanPlayer(std::string_view characterName);
	cpp::result<nlohmann::json, nlohmann::json> GetClientId(std::string_view characterName);
	cpp::result<nlohmann::json, nlohmann::json> KillPlayer(std::string_view characterName);
	cpp::result<nlohmann::json, nlohmann::json> SetRep(std::string_view characterName, const std::wstring& repGroup, float value);
	cpp::result<nlohmann::json, nlohmann::json> ResetRep(std::string_view characterName, const std::wstring& repGroup);
	cpp::result<nlohmann::json, nlohmann::json> GetRep(std::string_view characterName, const std::wstring& repGroup);
	cpp::result<nlohmann::json, nlohmann::json> MessagePlayer(std::string_view characterName, const std::wstring& text);
	cpp::result<nlohmann::json, nlohmann::json> SendSystemMessage(const std::wstring& systemName, const std::wstring& text);
	cpp::result<nlohmann::json, nlohmann::json> SendUniverseMessage(const std::wstring text);

	const inline static std::array<std::pair<std::string_view, cpp::result<nlohmann::json, nlohmann::json> (*)(AdminCommandProcessor cl, const std::vector<std::string>& params)>, 14> commands = 
	{
	    {
	        AddCommand("getcash", GetCash),
	        AddCommand("setcash", SetCash),
	        AddCommand("kick", KickPlayer),
	        AddCommand("ban", BanPlayer),
	        AddCommand("tempban", TempbanPlayer),
	        AddCommand("unban", UnBanPlayer),
	        AddCommand("getclient", GetClientId),
	        AddCommand("kill", KillPlayer),
	        AddCommand("setrep", SetRep),
	        AddCommand("resetrep", ResetRep),
	        AddCommand("getrep", GetRep),
	        AddCommand("msg", MessagePlayer),
	        AddCommand("msgs", SendSystemMessage),
	        AddCommand("msgu", SendUniverseMessage),
	    }
	};	    

#undef AddCommand

	template<int N>
	cpp::result<nlohmann::json, nlohmann::json> MatchCommand(AdminCommandProcessor* processor, std::string_view cmd, const std::vector<std::string>& paramVector)
	{
		if (const auto command = std::get<N - 1>(commands); command.first == cmd)
		{
			return command.second(*processor, paramVector);
		}
		return MatchCommand<N - 1>(processor, cmd, paramVector);
	}

	template<>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	cpp::result<nlohmann::json, nlohmann::json> MatchCommand<0>(AdminCommandProcessor* processor, std::string_view cmd, const std::vector<std::string>& paramVector)
	{
		// The original command was not found, we now search our plugins

		// No matching command was found.
		return cpp::fail(nlohmann::json {"err", "Command not found."});
	}

public:
	void ProcessCommand(std::string_view commandString);
};