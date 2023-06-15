#pragma once

#include "AbstractAdminCommandProcessor.hpp"

class AdminCommandProcessor final : public Singleton<AdminCommandProcessor>, public AbstractAdminCommandProcessor
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
            GameAndConsole = GameOnly | ConsoleOnly,
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
        // std::wstring Move(const std::wstring& characterName, Vector position);

        constexpr inline static std::array<CommandInfo, 29> commands = {
            {AddCommand(L"getcash", GetCash, GameAndConsole, Cash),
             AddCommand(L"setcash", SetCash, GameAndConsole, Cash),
             AddCommand(L"kick", KickPlayer, GameAndConsole, Expel),
             AddCommand(L"ban", BanPlayer, GameAndConsole, Expel),
             AddCommand(L"tempban", TempbanPlayer, GameAndConsole, Expel),
             AddCommand(L"unban", UnBanPlayer, GameAndConsole, Expel),
             AddCommand(L"getclient", GetClientId, GameAndConsole, Info),
             AddCommand(L"kill", KillPlayer, GameAndConsole, Expel),
             AddCommand(L"setrep", SetRep, GameAndConsole, Reputation),
             AddCommand(L"resetrep", ResetRep, GameAndConsole, Reputation),
             AddCommand(L"getrep", GetRep, GameAndConsole, Reputation),
             AddCommand(L"msg", MessagePlayer, GameAndConsole, Message),
             AddCommand(L"msgs", SendSystemMessage, GameAndConsole, Message),
             AddCommand(L"msgu", SendUniverseMessage, GameAndConsole, Message),
             AddCommand(L"listcargo", ListCargo, GameAndConsole, Cargo),
             AddCommand(L"addcargo", AddCargo, GameAndConsole, Cargo),
             AddCommand(L"renamechar", RenameChar, GameAndConsole, Character),
             AddCommand(L"deletechar", DeleteChar, GameAndConsole, Character),
             AddCommand(L"getplayerinfo", GetPlayerInfo, GameAndConsole, Info),
             AddCommand(L"addroles", AddRoles, GameAndConsole, SuperAdmin),
             AddCommand(L"deleteroles", DeleteRoles, GameAndConsole, SuperAdmin),
             AddCommand(L"setroles", AddRoles, GameAndConsole, SuperAdmin),
             AddCommand(L"loadplugin", LoadPlugin, GameAndConsole, Plugin),
             AddCommand(L"unloadplugin", UnloadPlugin, GameAndConsole, Plugin),
             AddCommand(L"reloadplugin", ReloadPlugin, GameAndConsole, Plugin),
             AddCommand(L"listplugins", ListPlugins, GameAndConsole, Info),
             AddCommand(L"chase", Chase, GameOnly, Movement),
             AddCommand(L"beam", Beam, GameAndConsole, Movement),
             AddCommand(L"pull", Pull, GameAndConsole, Movement)}
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
        std::wstring ProcessCommand(std::wstring_view cmd, const std::vector<std::wstring>& paramVector) override;
        void SetCurrentUser(std::wstring_view user, AllowedContext context);
};
