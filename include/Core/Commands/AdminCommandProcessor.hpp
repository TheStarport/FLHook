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
            { L"console", { magic_enum::enum_name(DefaultRoles::SuperAdmin) } }
        };

#define AddAdminCommand(str, func, context, requiredRole, usage, description)                                                                   \
    {                                                                                                                                           \
        str, ClassFunctionWrapper<decltype(&AdminCommandProcessor::func), &AdminCommandProcessor::func>::ProcessParam, AllowedContext::context, \
            magic_enum::enum_name(DefaultRoles::requiredRole), usage, description                                                               \
    }

        struct CommandInfo
        {
                std::vector<std::wstring_view> cmd;
                std::wstring (*func)(AdminCommandProcessor* cl, std::vector<std::wstring>& params);
                AllowedContext allowedContext;
                std::wstring_view requiredRole;
                std::wstring_view usage;
                std::wstring_view description;
        };

        std::wstring SetCash(std::wstring_view characterName, uint amount);
        std::wstring GetCash(std::wstring_view characterName);
        std::wstring KickPlayer(std::wstring_view characterName, std::wstring_view reason);
        std::wstring BanPlayer(std::wstring_view characterName);
        std::wstring TempbanPlayer(std::wstring_view characterName, uint durationInDays);
        std::wstring UnBanPlayer(std::wstring_view characterName);
        std::wstring GetClientId(std::wstring_view characterName);
        std::wstring KillPlayer(std::wstring_view characterName);
        std::wstring SetRep(std::wstring_view characterName, std::wstring_view repGroup, float value);
        std::wstring ResetRep(std::wstring_view characterName, std::wstring_view repGroup);
        std::wstring GetRep(std::wstring_view characterName, std::wstring_view repGroup);
        std::wstring MessagePlayer(std::wstring_view characterName, std::wstring_view text);
        std::wstring SendSystemMessage(std::wstring_view systemName, std::wstring_view text);
        std::wstring SendUniverseMessage(std::wstring_view text);
        std::wstring ListCargo(std::wstring_view characterName);
        std::wstring AddCargo(std::wstring_view characterName, std::wstring_view good, uint count, bool mission);
        std::wstring RenameChar(std::wstring_view characterName, std::wstring_view newName);
        std::wstring DeleteChar(std::wstring_view characterName);
        std::wstring GetPlayerInfo(std::wstring_view characterName);
        std::wstring AddRoles(std::wstring_view characterName, std::vector<std::wstring_view> roles);
        std::wstring DeleteRoles(std::wstring_view characterName, std::vector<std::wstring_view> roles);
        std::wstring SetRoles(std::wstring_view characterName, std::vector<std::wstring_view> roles);
        std::wstring LoadPlugin(std::vector<std::wstring_view> pluginNames);
        std::wstring UnloadPlugin(std::vector<std::wstring_view> pluginNames);
        std::wstring ReloadPlugin(std::vector<std::wstring_view> pluginNames);
        std::wstring ListPlugins();
        std::wstring Chase(std::wstring_view characterName);
        std::wstring Beam(std::wstring_view characterName, std::wstring_view baseName);
        std::wstring Pull(std::wstring_view characterName);
        std::wstring SetDamageType(std::wstring_view newDamageType);
        // std::wstring Move(const std::wstring& characterName, Vector position);

        const inline static std::array<CommandInfo, 30> commands = {
            { AddAdminCommand(Cmds(L"getcash"), GetCash, GameAndConsole, Cash, L".getcash <charname> <cash>", L"Gets the cash of the target cash"),
             AddAdminCommand(Cmds(L"setcash"), SetCash, GameAndConsole, Cash, L".setcash <charname> <cash>", L"Sets the cash of the target cash"),
             AddAdminCommand(Cmds(L"kick"), KickPlayer, GameAndConsole, Expel, L".kick <charname>", L"Kick the specified character from the server."),
             AddAdminCommand(Cmds(L"ban"), BanPlayer, GameAndConsole, Expel, L".ban <charname>", L"Ban the specified character from the server."),
             AddAdminCommand(Cmds(L"tempban"), TempbanPlayer, GameAndConsole, Expel, L".tempban <charname> <duration>",
             L"Bans the specified character for a specified duration."),
             AddAdminCommand(Cmds(L"unban"), UnBanPlayer, GameAndConsole, Expel, L".unban <charname>", L"Unbans the specified character from the server."),
             AddAdminCommand(Cmds(L"getclient"), GetClientId, GameAndConsole, Info, L".getclient <charname>", L"Get the client id of the specified character."),
             AddAdminCommand(Cmds(L"kill"), KillPlayer, GameAndConsole, Expel, L".kill <charname>", L"Kills the specified character if they are in space."),
             AddAdminCommand(Cmds(L"setrep"), SetRep, GameAndConsole, Reputation, L".setrep <charname> <repgroup> [value]",
             L"Set the rep value for the specified character for the specified group. Defaults to 0.0 if not specified."),
             AddAdminCommand(Cmds(L"resetrep"), ResetRep, GameAndConsole, Reputation, L".resetrep <charname> <repgroup>",
             L"Resets the reputation for the specified character and group to their default mpnewplayer value."),
             AddAdminCommand(Cmds(L"getrep"), GetRep, GameAndConsole, Reputation, L".getrep <charname> <repgroup>",
             L"Gets the rep value for the specified character and rep group."),
             AddAdminCommand(Cmds(L"msg"), MessagePlayer, GameAndConsole, Message, L".msg <charname> <message>", L"Send an admin message to the target player."),
             AddAdminCommand(Cmds(L"msgs"), SendSystemMessage, GameAndConsole, Message, L".msgs <system> <message>",
             L"Send a message to every player in a system."),
             AddAdminCommand(Cmds(L"msgu"), SendUniverseMessage, GameAndConsole, Message, L".msgu <message>", L"Send a message to every player on the server."),
             AddAdminCommand(Cmds(L"listcargo"), ListCargo, GameAndConsole, Cargo, L".listcargo <charname>", L"List the cargo of the specified player"),
             AddAdminCommand(Cmds(L"addcargo"), AddCargo, GameAndConsole, Cargo, L".addcargo <charname> <item> <count> [isMission]",
             L"Add cargo the specified player."),
             AddAdminCommand(Cmds(L"renamechar"), RenameChar, GameAndConsole, Character, L".renamechar <charname> <newcharanme>",
             L"Changes the name of the specified character to the new specified value."),
             AddAdminCommand(Cmds(L"deletechar"), DeleteChar, GameAndConsole, Character, L".deletechar <charname>",
             L"Permanently delete the specified character."),
             AddAdminCommand(Cmds(L"getplayerinfo"), GetPlayerInfo, GameAndConsole, Info, L".getplayerinfo <charname>",
             L"Returns data about the player's session information."),
             AddAdminCommand(Cmds(L"addroles"), AddRoles, GameAndConsole, SuperAdmin, L".addroles <charname> <roles...>",
             L"Add the specified admin roles to the user."),
             AddAdminCommand(Cmds(L"deleteroles"), DeleteRoles, GameAndConsole, SuperAdmin, L".deleteroles <charname> <roles...>",
             L"Removes the specified admin roles from the user."),
             AddAdminCommand(Cmds(L"setroles"), SetRoles, GameAndConsole, SuperAdmin, L".setroles <charname> [roles...]",
             L"Replaces the existing roles with the new specified roles. Providing none will clear the roles."),
             AddAdminCommand(Cmds(L"loadplugin"), LoadPlugin, GameAndConsole, Plugin, L".loadplugin <dllname>",
             L"Attempt to load the specified DLL from the plugins folder. Providing 'all' loads all plugins."),
             AddAdminCommand(Cmds(L"unloadplugin"), UnloadPlugin, GameAndConsole, Plugin, L".unloadplugin <plugin_shortname>",
             L"Unload the plugin by its short name. Providing 'all' unloads all plugins."),
             AddAdminCommand(Cmds(L"reloadplugin"), ReloadPlugin, GameAndConsole, Plugin, L".reloadplugin <plugin_shortname>", L"Unload then reload a plugin"),
             AddAdminCommand(Cmds(L"listplugins"), ListPlugins, GameAndConsole, Info, L".listplugins", L"List all plugins and their short names."),
             AddAdminCommand(Cmds(L"chase"), Chase, GameOnly, Movement, L".chase <charname>",
             L"Teleport the admin to the location of the specified character. Does not traverse systems."),
             AddAdminCommand(Cmds(L"beam"), Beam, GameAndConsole, Movement, L".beam <charname> <base>",
             L"Teleport the specified character to the specified base."),
             AddAdminCommand(Cmds(L"pull"), Pull, GameOnly, Movement, L".pull <charname>",
             L"Pulls the specified character to your location. Does not traverse systems."),
             AddAdminCommand(Cmds(L"damagemode"), SetDamageType, GameAndConsole, SuperAdmin, L".damagemode <all/none/pvp/pve>",
             L"Sets the source of allowed damage on the server.") }
  // AddAdminCommand(L"move", Move, GameOnly, Movement)
        };

#undef AddAdminCommand

        GetCommandsFunc(commands);

        cpp::result<void, std::wstring> Validate(AllowedContext context, std::wstring_view requiredRole);

        template <int N>
        std::wstring MatchCommand(AdminCommandProcessor* processor, std::wstring_view cmd, std::vector<std::wstring>& paramVector)
        {
            for (const CommandInfo& command = std::get<N - 1>(commands); auto& str : command.cmd)
            {
                if (str == cmd)
                {
                    if (const auto validation = Validate(command.allowedContext, command.requiredRole); validation.has_error())
                    {
                        return validation.error();
                    }

                    return command.func(processor, paramVector);
                }
            }

            return MatchCommand<N - 1>(processor, cmd, paramVector);
        }

        template <>
        // ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
        std::wstring MatchCommand<0>(AdminCommandProcessor* processor, std::wstring_view cmd, std::vector<std::wstring>& paramVector)
        {
            // The original command was not found, we now search our plugins

            // No matching command was found.
            return std::format(L"ERR: Command not found. ({})", cmd);
        }

    public:
        std::wstring ProcessCommand(std::wstring_view commandString);
        std::wstring ProcessCommand(std::wstring_view cmd, std::vector<std::wstring>& paramVector) override;
        void SetCurrentUser(std::wstring_view user, AllowedContext context);
};
