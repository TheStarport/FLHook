#pragma once

#include <nlohmann/json.hpp>

class AdminCommandProcessor : public Singleton<AdminCommandProcessor>
{
  public:
	enum class DefaultRoles
	{
		SuperAdmin,
		Cash,
		Expel,
		Reputation,
		Info,
		Cargo,
		Message,
		Character,
		Plugin,
		Movement
	};

	enum class AllowedContext
	{
		Reset = 0,
		GameOnly = 1,
		ConsoleOnly = 2,
		ExternalOnly = 4,
		GameAndConsole = GameOnly | ConsoleOnly,
		GameAndExternal = GameOnly | ExternalOnly,
		ConsoleAndExternal = ConsoleOnly | ExternalOnly,
		All = GameOnly | ConsoleOnly | ExternalOnly,
	};

  private:
	// Current user, changes every command invocation.
	std::string_view currentUser = "";
	AllowedContext currentContext = AllowedContext::GameOnly;

	std::unordered_map<std::string, std::vector<std::string_view>> credentialsMap = {{
	    "console",
	    {magic_enum::enum_name(DefaultRoles::SuperAdmin)},
	}};

#define AddCommand(str, func, context, requiredRole)                                                                                     \
	{                                                                                                                                    \
		std::string_view(str), ClassFunctionWrapper<decltype(&AdminCommandProcessor::func), &AdminCommandProcessor::func>::ProcessParam, \
		    AllowedContext::context, magic_enum::enum_name(DefaultRoles::requiredRole)                                                   \
	}

#define ReturnType cpp::result<nlohmann::json, nlohmann::json>

	struct CommandInfo
	{
		std::string_view cmd;
		ReturnType (*func)(AdminCommandProcessor cl, const std::vector<std::string>& params);
		AllowedContext allowedContext;
		std::string_view requiredRole;
	};

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
	ReturnType WriteCharFile(std::string_view characterName, const std::wstring& data);
	ReturnType GetPlayerInfo(std::string_view characterName);
	ReturnType GetAllPlayerInfo();
	ReturnType GetGroupMembers(std::string_view characterName);
	ReturnType AddRoles(std::string_view characterName, const std::vector<std::wstring>& roles);
	ReturnType DeleteRoles(std::string_view characterName, const std::vector<std::wstring>& roles);
	ReturnType SetRoles(std::string_view characterName, const std::vector<std::wstring>& roles);
	ReturnType LoadPlugin(const std::vector<std::wstring>& pluginNames);
	ReturnType UnloadPlugin(const std::vector<std::wstring>& pluginNames);
	ReturnType ReloadPlugin(const std::vector<std::wstring>& pluginNames);
	ReturnType ListPlugins();
	ReturnType Chase(std::wstring_view characterName);
	ReturnType Beam(std::wstring_view characterName, const std::wstring& baseName);
	ReturnType Pull(const std::wstring& characterName);
	ReturnType Move(const std::wstring& characterName, Vector position);

#undef ReturnType

	constexpr inline static std::array<CommandInfo, 33> commands = {{
		AddCommand("getcash", GetCash, All, Cash),
	    AddCommand("setcash", SetCash, All, Cash),
	    AddCommand("kick", KickPlayer, All, Expel),
	    AddCommand("ban", BanPlayer, All, Expel),
	    AddCommand("tempban", TempbanPlayer, All, Expel),
	    AddCommand("unban", UnBanPlayer, All, Expel),
	    AddCommand("getclient", GetClientId, All, Info),
	    AddCommand("kill", KillPlayer, All, Expel),
	    AddCommand("setrep", SetRep, All, Reputation),
	    AddCommand("resetrep", ResetRep, All, Reputation),
	    AddCommand("getrep", GetRep, All, Reputation),
	    AddCommand("msg", MessagePlayer, All, Message),
	    AddCommand("msgs", SendSystemMessage, All, Message),
	    AddCommand("msgu", SendUniverseMessage, All, Message),
	    AddCommand("listcargo", ListCargo, All, Cargo),
	    AddCommand("addcargo", AddCargo, All, Cargo),
	    AddCommand("renamechar", RenameChar, All, Character),
	    AddCommand("deletechar", DeleteChar, All, Character),
	    AddCommand("writecharfile", WriteCharFile, ExternalOnly, Character),
	    AddCommand("getplayerinfo", GetPlayerInfo, All, Info),
	    AddCommand("getallplayerinfo", GetAllPlayerInfo, ExternalOnly, Info),
	    AddCommand("getgroupmembers", GetGroupMembers, ConsoleAndExternal, Info),
	    AddCommand("addroles", AddRoles, All, SuperAdmin),
	    AddCommand("deleteroles", DeleteRoles, All, SuperAdmin),
	    AddCommand("setroles", AddRoles, All, SuperAdmin),
	    AddCommand("loadplugin", LoadPlugin, All, Plugin),
	    AddCommand("unloadplugin", UnloadPlugin, All, Plugin),
	    AddCommand("reloadplugin", ReloadPlugin, All, Plugin),
	    AddCommand("listplugins", ListPlugins, All, Info ),
	    AddCommand("chase", Chase,GameOnly, Movement ),
	    AddCommand("beam", Beam, All,  Movement),
	    AddCommand("pull", Pull, All, Movement),
	    AddCommand("move", Move, GameOnly, Movement)
	}};

#undef AddCommand

	cpp::result<void, std::string_view> Validate(AllowedContext context, std::string_view requiredRole);

	template<int N>
	cpp::result<nlohmann::json, nlohmann::json> MatchCommand(
	    AdminCommandProcessor* processor, std::string_view cmd, const std::vector<std::string>& paramVector)
	{
		if (const CommandInfo command = std::get<N - 1>(commands); command.cmd == cmd)
		{
			if (const auto validation = Validate(command.allowedContext, command.requiredRole); validation.has_error())
			{
				return cpp::fail(nlohmann::json {{"err", validation.error()}});
			}

			return command.func(*processor, paramVector);
		}

		return MatchCommand<N - 1>(processor, cmd, paramVector);
	}

	template<>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	cpp::result<nlohmann::json, nlohmann::json> MatchCommand<0>(
	    AdminCommandProcessor* processor, std::string_view cmd, const std::vector<std::string>& paramVector)
	{
		// The original command was not found, we now search our plugins

		// No matching command was found.
		return cpp::fail(nlohmann::json {{"err", std::format("ERR: Command not found. ({})", cmd)}});
	}

	nlohmann::json::object_t GeneratePlayerInfoObj(const PlayerInfo& player);

  public:
	cpp::result<nlohmann::json, nlohmann::json> ProcessCommand(std::string_view commandString);
	void SetCurrentUser(std::string_view user, AllowedContext context);
};