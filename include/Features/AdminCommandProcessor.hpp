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
        std::wstring_view currentUser;
        AllowedContext currentContext = AllowedContext::GameOnly;

        std::unordered_map<std::wstring, std::vector<std::wstring_view>> credentialsMap = {
            {L"console", { magic_enum::enum_name(DefaultRoles::SuperAdmin) }}
        };

#define AddCommand(str, func, context, requiredRole)                                                                                      \
    {                                                                                                                                     \
        std::wstring_view(str), ClassFunctionWrapper<decltype(&AdminCommandProcessor::func), &AdminCommandProcessor::func>::ProcessParam, \
            AllowedContext::context, magic_enum::enum_name(DefaultRoles::requiredRole)                                                    \
    }

        struct CommandInfo
        {
                std::wstring_view cmd;
                std::wstring (*func)(AdminCommandProcessor cl, const std::vector<std::wstring>& params);
                AllowedContext allowedContext;
                std::wstring_view requiredRole;
        };

        std::wstring SetCash(std::wstring_view characterName, uint amount);
        std::wstring GetCash(std::wstring_view characterName);
        std::wstring KickPlayer(std::wstring_view characterName, std::wstring_view reason);
        std::wstring BanPlayer(std::wstring_view characterName);
        std::wstring TempbanPlayer(std::wstring_view characterName, uint durationInDays);
        std::wstring UnBanPlayer(std::wstring_view characterName);
        std::wstring GetClientId(std::wstring_view characterName);
        std::wstring KillPlayer(std::wstring_view characterName);
        std::wstring SetRep(std::wstring_view characterName, const std::wstring& repGroup, float value);
        std::wstring ResetRep(std::wstring_view characterName, const std::wstring& repGroup);
        std::wstring GetRep(std::wstring_view characterName, const std::wstring& repGroup);
        std::wstring MessagePlayer(std::wstring_view characterName, const std::wstring& text);
        std::wstring SendSystemMessage(std::wstring_view systemName, const std::wstring& text);
        std::wstring SendUniverseMessage(std::wstring_view text);
        std::wstring ListCargo(std::wstring_view characterName);
        std::wstring AddCargo(std::wstring_view characterName, const std::wstring& good, uint count, bool mission);
        std::wstring RenameChar(std::wstring_view characterName, const std::wstring& newName);
        std::wstring DeleteChar(std::wstring_view characterName);
        std::wstring GetPlayerInfo(std::wstring_view characterName);
        std::wstring AddRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles);
        std::wstring DeleteRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles);
        std::wstring SetRoles(std::wstring_view characterName, const std::vector<std::wstring_view>& roles);
        std::wstring LoadPlugin(const std::vector<std::wstring_view>& pluginNames);
        std::wstring UnloadPlugin(const std::vector<std::wstring_view>& pluginNames);
        std::wstring ReloadPlugin(const std::vector<std::wstring_view>& pluginNames);
        std::wstring ListPlugins();
        std::wstring Chase(std::wstring_view characterName);
        std::wstring Beam(std::wstring_view characterName, const std::wstring& baseName);
        std::wstring Pull(const std::wstring& characterName);
        std::wstring Move(const std::wstring& characterName, Vector position);

        constexpr inline static std::array<CommandInfo, 29> commands = {
            {AddCommand(L"getcash", GetCash, All, Cash),
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
             AddCommand(L"getplayerinfo", GetPlayerInfo, All, Info),
             AddCommand(L"addroles", AddRoles, All, SuperAdmin),
             AddCommand(L"deleteroles", DeleteRoles, All, SuperAdmin),
             AddCommand(L"setroles", AddRoles, All, SuperAdmin),
             AddCommand(L"loadplugin", LoadPlugin, All, Plugin),
             AddCommand(L"unloadplugin", UnloadPlugin, All, Plugin),
             AddCommand(L"reloadplugin", ReloadPlugin, All, Plugin),
             AddCommand(L"listplugins", ListPlugins, All, Info),
             AddCommand(L"chase", Chase, GameOnly, Movement),
             AddCommand(L"beam", Beam, All, Movement),
             AddCommand(L"pull", Pull, All, Movement)}
  // AddCommand(L"move", Move, GameOnly, Movement)
        };

#undef AddCommand

        cpp::result<void, std::wstring> Validate(AllowedContext context, std::wstring_view requiredRole);

        template <int N>
        std::wstring MatchCommand(AdminCommandProcessor* processor, const std::wstring_view cmd, const std::vector<std::wstring>& paramVector)
        {
            if (const CommandInfo command = std::get<N - 1>(commands); command.cmd == cmd)
            {
                if (const auto validation = Validate(command.allowedContext, command.requiredRole); validation.has_error())
                {
                    return validation.error();
                }

                return command.func(*processor, paramVector);
            }

            return MatchCommand<N - 1>(processor, cmd, paramVector);
        }

        template <>
        // ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
        std::wstring MatchCommand<0>(AdminCommandProcessor* processor, std::wstring_view cmd, const std::vector<std::wstring>& paramVector)
        {
            // The original command was not found, we now search our plugins

            // No matching command was found.
            return std::format(L"ERR: Command not found. ({})", cmd);
        }

    public:
        std::wstring ProcessCommand(std::wstring_view commandString);
        void SetCurrentUser(std::wstring_view user, AllowedContext context);
};
