#pragma once

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
	std::wstring_view currentUser = L"";
	AllowedContext currentContext = AllowedContext::GameOnly;

	std::unordered_map<std::wstring, std::vector<std::wstring_view>> credentialsMap = {{
	    L"console",
	    { magic_enum::enum_name(DefaultRoles::SuperAdmin) },
	}};

#define AddCommand(str, func, context, requiredRole)                                                                                     \
	{                                                                                                                                    \
		std::wstring_view(str), ClassFunctionWrapper<decltype(&AdminCommandProcessor::func), &AdminCommandProcessor::func>::ProcessParam, \
		    AllowedContext::context, magic_enum::enum_name(DefaultRoles::requiredRole)                                                   \
	}

#define ReturnType cpp::result<nlohmann::json, nlohmann::json>

	struct CommandInfo
	{
		std::wstring_view cmd;
		ReturnType (*func)(AdminCommandProcessor cl, const std::vector<std::wstring>& params);
		AllowedContext allowedContext;
		std::wstring_view requiredRole;
	};

	ReturnType SetCash(std::wstring_view characterName, uint amount);
	ReturnType GetCash(std::wstring_view characterName);
	ReturnType KickPlayer(std::wstring_view characterName, std::wstring_view reason);
	ReturnType BanPlayer(std::wstring_view characterName);
	ReturnType TempbanPlayer(std::wstring_view characterName, uint duration);
	ReturnType UnBanPlayer(std::wstring_view characterName);
	ReturnType GetClientId(std::wstring_view characterName);
	ReturnType KillPlayer(std::wstring_view characterName);
	ReturnType SetRep(std::wstring_view characterName, const std::wstring& repGroup, float value);
	ReturnType ResetRep(std::wstring_view characterName, const std::wstring& repGroup);
	ReturnType GetRep(std::wstring_view characterName, const std::wstring& repGroup);
	ReturnType MessagePlayer(std::wstring_view characterName, const std::wstring& text);
	ReturnType SendSystemMessage(std::wstring_view systemName, const std::wstring& text);
	ReturnType SendUniverseMessage(std::wstring_view text);
	ReturnType ListCargo(std::wstring_view characterName);
	ReturnType AddCargo(std::wstring_view characterName, const std::wstring& good, uint count, bool mission);
	ReturnType RenameChar(std::wstring_view characterName, const std::wstring& newName);
	ReturnType DeleteChar(std::wstring_view characterName);
	ReturnType ReadCharFile(std::wstring_view characterName);
	ReturnType WriteCharFile(std::wstring_view characterName, const std::wstring& data);
	ReturnType GetPlayerInfo(std::wstring_view characterName);
	ReturnType GetAllPlayerInfo();
	ReturnType GetGroupMembers(std::wstring_view characterName);
	ReturnType AddRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles);
	ReturnType DeleteRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles);
	ReturnType SetRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles);
	ReturnType LoadPlugin(const std::vector<std::wstring_view>& pluginNames);
	ReturnType UnloadPlugin(const std::vector<std::wstring_view>& pluginNames);
	ReturnType ReloadPlugin(const std::vector<std::wstring_view>& pluginNames);
	ReturnType ListPlugins();
	ReturnType Chase(std::wstring_view characterName);
	ReturnType Beam(std::wstring_view characterName, const std::wstring& baseName);
	ReturnType Pull(const std::wstring& characterName);
	//ReturnType Move(const std::wstring& characterName, Vector position);

#undef ReturnType

	constexpr inline static std::array<CommandInfo, 32> commands = {{
		AddCommand(L"getcash", GetCash, All, Cash),
	    AddCommand(L"setcash", SetCash, All, Cash),
	    AddCommand(L"kick", KickPlayer, All, Expel),
	    AddCommand(L"ban", BanPlayer, All, Expel),
	    AddCommand(L"tempban", TempbanPlayer, All, Expel),
	    AddCommand(L"unban", UnBanPlayer, All, Expel),
	    AddCommand(L"getclient", GetClientId, All, Info),
	    AddCommand(L"kill", KillPlayer, All, Expel),
	    AddCommand(L"setrep", SetRep, All, Reputation),
	    AddCommand(L"resetrep", ResetRep, All, Reputation),
	    AddCommand(L"getrep", GetRep, All, Reputation),
	    AddCommand(L"msg", MessagePlayer, All, Message),
	    AddCommand(L"msgs", SendSystemMessage, All, Message),
	    AddCommand(L"msgu", SendUniverseMessage, All, Message),
	    AddCommand(L"listcargo", ListCargo, All, Cargo),
	    AddCommand(L"addcargo", AddCargo, All, Cargo),
	    AddCommand(L"renamechar", RenameChar, All, Character),
	    AddCommand(L"deletechar", DeleteChar, All, Character),
	    AddCommand(L"writecharfile", WriteCharFile, ExternalOnly, Character),
	    AddCommand(L"getplayerinfo", GetPlayerInfo, All, Info),
	    AddCommand(L"getallplayerinfo", GetAllPlayerInfo, ExternalOnly, Info),
	    AddCommand(L"getgroupmembers", GetGroupMembers, ConsoleAndExternal, Info),
	    AddCommand(L"addroles", AddRoles, All, SuperAdmin),
	    AddCommand(L"deleteroles", DeleteRoles, All, SuperAdmin),
	    AddCommand(L"setroles", AddRoles, All, SuperAdmin),
	    AddCommand(L"loadplugin", LoadPlugin, All, Plugin),
	    AddCommand(L"unloadplugin", UnloadPlugin, All, Plugin),
	    AddCommand(L"reloadplugin", ReloadPlugin, All, Plugin),
	    AddCommand(L"listplugins", ListPlugins, All, Info ),
	    AddCommand(L"chase", Chase, GameOnly, Movement ),
	    AddCommand(L"beam", Beam, All,  Movement),
	    AddCommand(L"pull", Pull, All, Movement),
	    //AddCommand(L"move", Move, GameOnly, Movement)
	}};

#undef AddCommand

	cpp::result<void, std::wstring_view> Validate(AllowedContext context, std::wstring_view requiredRole);

	template<int N>
	cpp::result<nlohmann::json, nlohmann::json> MatchCommand(
	    AdminCommandProcessor* processor, std::wstring_view cmd, const std::vector<std::wstring>& paramVector)
	{
		if (const CommandInfo command = std::get<N - 1>(commands); command.cmd == cmd)
		{
			if (const auto validation = Validate(command.allowedContext, command.requiredRole); validation.has_error())
			{
				return {cpp::fail(nlohmann::json {{"err", validation.error()}})};
			}

			return command.func(*processor, paramVector);
		}

		return MatchCommand<N - 1>(processor, cmd, paramVector);
	}

	template<>
	// ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
	cpp::result<nlohmann::json, nlohmann::json> MatchCommand<0>(
	    AdminCommandProcessor* processor, std::wstring_view cmd, const std::vector<std::wstring>& paramVector)
	{
		// The original command was not found, we now search our plugins

		// No matching command was found.
		return {cpp::fail(nlohmann::json {{"err", std::format(L"ERR: Command not found. ({})", cmd)}})};
	}

	nlohmann::json::object_t GeneratePlayerInfoObj(const PlayerInfo& player);

  public:
	cpp::result<nlohmann::json, nlohmann::json> ProcessCommand(std::wstring_view commandString);
	void SetCurrentUser(std::wstring_view user, AllowedContext context);
};