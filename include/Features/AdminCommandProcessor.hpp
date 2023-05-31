#pragma once

#include <nlohmann/json.hpp>

class AdminCommandProcessor
{
	std::unordered_map<std::string, std::vector<std::string_view>> credentialsMap;

#define AddCommand(str, func) { std::string_view(str), ClassFunctionWrapper<decltype(&AdminCommandProcessor::func), &AdminCommandProcessor::func>::ProcessParam }

	#define ReturnType cpp::result<nlohmann::json, nlohmann::json>

	ReturnType SetCash(std::string_view characterName, uint amount);
	ReturnType GetCash(std::string_view characterName);
	ReturnType KickPlayer(std::string_view characterName, std::string_view reason);
	ReturnType BanPlayer(std::string_view characterName);
	ReturnType TempbanPlayer(std::string_view characterName, uint duration);
	ReturnType UnBanPlayer(std::string_view characterName);
	ReturnType GetClientId(std::string_view characterName);
	ReturnType KillPlayer(std::string_view characterName);
	ReturnType SetRep(std::string_view characterName, const std::wstring& repGroup, float value);
	ReturnType ResetRep(std::string_view characterName, const std::wstring& repGroup);
	ReturnType GetRep(std::string_view characterName, const std::wstring& repGroup);
	ReturnType MessagePlayer(std::string_view characterName, const std::wstring& text);
	ReturnType SendSystemMessage(std::string_view systemName, const std::wstring& text);
	ReturnType SendUniverseMessage(std::wstring_view text);
	ReturnType ListCargo(std::string_view characterName);
	ReturnType AddCargo(std::string_view characterName, const std::wstring& good, uint count, bool mission);
	ReturnType RenameChar(std::string_view characterName, const std::wstring& newName);
	ReturnType DeleteChar(std::string_view characterName);
	ReturnType ReadCharFile(std::string_view characterName);
	ReturnType WriteCharFile(std::string_view characterName,const std::wstring& data);
	ReturnType GetPlayerInfo(std::string_view characterName);

#undef ReturnType

	const inline static std::array<
	    std::pair<std::string_view, cpp::result<nlohmann::json, nlohmann::json> (*)(AdminCommandProcessor cl, const std::vector<std::string>& params)>, 20>
	    commands = {{AddCommand("getcash", GetCash),
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
	        AddCommand("listcargo", ListCargo),
	        AddCommand("addcargo", AddCargo),
	        AddCommand("renamechar", RenameChar),
	        AddCommand("deletechar", DeleteChar),
	        AddCommand("writecharfile", WriteCharFile),
	        AddCommand("getplayerinfo", GetPlayerInfo)
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