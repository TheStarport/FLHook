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

#define AddAdminCommand(str, func, context, requiredRole)                                                                                 \
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
            {AddAdminCommand(L"getcash", GetCash, GameAndConsole, Cash),
             AddAdminCommand(L"setcash", SetCash, GameAndConsole, Cash),
             AddAdminCommand(L"kick", KickPlayer, GameAndConsole, Expel),
             AddAdminCommand(L"ban", BanPlayer, GameAndConsole, Expel),
             AddAdminCommand(L"tempban", TempbanPlayer, GameAndConsole, Expel),
             AddAdminCommand(L"unban", UnBanPlayer, GameAndConsole, Expel),
             AddAdminCommand(L"getclient", GetClientId, GameAndConsole, Info),
             AddAdminCommand(L"kill", KillPlayer, GameAndConsole, Expel),
             AddAdminCommand(L"setrep", SetRep, GameAndConsole, Reputation),
             AddAdminCommand(L"resetrep", ResetRep, GameAndConsole, Reputation),
             AddAdminCommand(L"getrep", GetRep, GameAndConsole, Reputation),
             AddAdminCommand(L"msg", MessagePlayer, GameAndConsole, Message),
             AddAdminCommand(L"msgs", SendSystemMessage, GameAndConsole, Message),
             AddAdminCommand(L"msgu", SendUniverseMessage, GameAndConsole, Message),
             AddAdminCommand(L"listcargo", ListCargo, GameAndConsole, Cargo),
             AddAdminCommand(L"addcargo", AddCargo, GameAndConsole, Cargo),
             AddAdminCommand(L"renamechar", RenameChar, GameAndConsole, Character),
             AddAdminCommand(L"deletechar", DeleteChar, GameAndConsole, Character),
             AddAdminCommand(L"getplayerinfo", GetPlayerInfo, GameAndConsole, Info),
             AddAdminCommand(L"addroles", AddRoles, GameAndConsole, SuperAdmin),
             AddAdminCommand(L"deleteroles", DeleteRoles, GameAndConsole, SuperAdmin),
             AddAdminCommand(L"setroles", SetRoles, GameAndConsole, SuperAdmin),
             AddAdminCommand(L"loadplugin", LoadPlugin, GameAndConsole, Plugin),
             AddAdminCommand(L"unloadplugin", UnloadPlugin, GameAndConsole, Plugin),
             AddAdminCommand(L"reloadplugin", ReloadPlugin, GameAndConsole, Plugin),
             AddAdminCommand(L"listplugins", ListPlugins, GameAndConsole, Info),
             AddAdminCommand(L"chase", Chase, GameOnly, Movement),
             AddAdminCommand(L"beam", Beam, GameAndConsole, Movement),
             AddAdminCommand(L"pull", Pull, GameAndConsole, Movement)}
  // AddAdminCommand(L"move", Move, GameOnly, Movement)
        };

#undef AddAdminCommand

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
