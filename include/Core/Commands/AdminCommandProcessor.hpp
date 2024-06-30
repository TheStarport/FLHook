#pragma once

#include "AbstractAdminCommandProcessor.hpp"

class AdminCommandProcessor final : public Singleton<AdminCommandProcessor>, public AbstractAdminCommandProcessor
{
        std::wstring SetCash(ClientId characterName, uint amount);
        std::wstring GetCash(ClientId target);
        std::wstring KickPlayer(ClientId client, std::wstring_view reason);
        std::wstring BanPlayer(std::wstring_view characterName);
        std::wstring TempbanPlayer(std::wstring_view characterName, uint durationInDays);
        std::wstring UnBanPlayer(std::wstring_view characterName);
        std::wstring GetClientId(ClientId client);
        std::wstring KillPlayer(ClientId target);
        std::wstring SetRep(ClientId target, RepGroupId repGroup, float value);
        std::wstring ResetRep(ClientId target, RepGroupId repGroup);
        std::wstring GetRep(ClientId target, RepGroupId repGroup);
        std::wstring MessagePlayer(ClientId target, const std::wstring_view text);
        std::wstring SendSystemMessage(SystemId system, const std::wstring_view text);
        std::wstring SendUniverseMessage(std::wstring_view text);
        std::wstring ListCargo(const ClientId target);
        std::wstring AddCargo(ClientId target, GoodInfo* good, uint count, const bool mission);
        std::wstring RenameChar(ClientId target, std::wstring_view newName);
        std::wstring DeleteChar(ClientId target);
        std::wstring GetPlayerInfo(const ClientId target);
        std::wstring AddRoles(const std::wstring_view target, std::vector<std::wstring_view> roles);
        std::wstring DeleteRoles(std::wstring_view characterName, std::vector<std::wstring_view> roles);
        std::wstring SetRoles(std::wstring_view characterName, std::vector<std::wstring_view> roles);
        std::wstring LoadPlugin(std::vector<std::wstring_view> pluginNames);
        std::wstring UnloadPlugin(std::vector<std::wstring_view> pluginNames);
        std::wstring ReloadPlugin(std::vector<std::wstring_view> pluginNames);
        std::wstring ListPlugins();
        std::wstring Chase(const ClientId target);
        std::wstring Beam(ClientId target, BaseId base);
        std::wstring Pull(ClientId target);
        std::wstring SetDamageType(std::wstring_view newDamageType);
        std::wstring Move(ClientId target, float x, float y, float z);
        std::wstring Help(int page);

        const inline static std::array<AdminCommandInfo<AdminCommandProcessor>, 32> commands = {
            { AddAdminCommand(AdminCommandProcessor, Cmds(L".getcash"), GetCash, GameAndConsole, Cash, L".getcash <charname> <cash>",
             L"Gets the cash of the target cash"),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".setcash"), SetCash, GameAndConsole, Cash, L".setcash <charname> <cash>",
             L"Sets the cash of the target cash"),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".kick"), KickPlayer, GameAndConsole, Expel, L".kick <charname>",
             L"Kick the specified character from the server."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".ban"), BanPlayer, GameAndConsole, Expel, L".ban <charname>",
             L"Ban the specified character from the server."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".tempban"), TempbanPlayer, GameAndConsole, Expel, L".tempban <charname> <duration>",
             L"Bans the specified character for a specified duration."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".unban"), UnBanPlayer, GameAndConsole, Expel, L".unban <charname>",
             L"Unbans the specified character from the server."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".getclient"), GetClientId, GameAndConsole, Info, L".getclient <charname>",
             L"Get the client id of the specified character."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".kill"), KillPlayer, GameAndConsole, Expel, L".kill <charname>",
             L"Kills the specified character if they are in space."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".setrep"), SetRep, GameAndConsole, Reputation, L".setrep <charname> <repgroup> [value]",
             L"Set the rep value for the specified character for the specified group. Defaults to 0.0 if not specified."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".resetrep"), ResetRep, GameAndConsole, Reputation, L".resetrep <charname> <repgroup>",
             L"Resets the reputation for the specified character and group to their default mpnewplayer value."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".getrep"), GetRep, GameAndConsole, Reputation, L".getrep <charname> <repgroup>",
             L"Gets the rep value for the specified character and rep group."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".msg"), MessagePlayer, GameAndConsole, Message, L".msg <charname> <message>",
             L"Send an admin message to the target player."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".msgs"), SendSystemMessage, GameAndConsole, Message, L".msgs <system> <message>",
             L"Send a message to every player in a system."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".msgu"), SendUniverseMessage, GameAndConsole, Message, L".msgu <message>",
             L"Send a message to every player on the server."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".listcargo"), ListCargo, GameAndConsole, Cargo, L".listcargo <charname>",
             L"List the cargo of the specified player"),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".addcargo"), AddCargo, GameAndConsole, Cargo, L".addcargo <charname> <item> <count> [isMission]",
             L"Add cargo the specified player."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".renamechar"), RenameChar, GameAndConsole, Character, L".renamechar <charname> <newcharanme>",
             L"Changes the name of the specified character to the new specified value."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".deletechar"), DeleteChar, GameAndConsole, Character, L".deletechar <charname>",
             L"Permanently delete the specified character."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".getplayerinfo"), GetPlayerInfo, GameAndConsole, Info, L".getplayerinfo <charname>",
             L"Returns data about the player's session information."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".addroles"), AddRoles, GameAndConsole, SuperAdmin, L".addroles <charname> <roles...>",
             L"Add the specified admin roles to the user."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".deleteroles"), DeleteRoles, GameAndConsole, SuperAdmin, L".deleteroles <charname> <roles...>",
             L"Removes the specified admin roles from the user."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".setroles"), SetRoles, GameAndConsole, SuperAdmin, L".setroles <charname> [roles...]",
             L"Replaces the existing roles with the new specified roles. Providing none will clear the roles."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".loadplugin"), LoadPlugin, GameAndConsole, Plugin, L".loadplugin <dllname>",
             L"Attempt to load the specified DLL from the plugins folder. Providing 'all' loads all plugins."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".unloadplugin"), UnloadPlugin, GameAndConsole, Plugin, L".unloadplugin <plugin_shortname>",
             L"Unload the plugin by its short name. Providing 'all' unloads all plugins."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".reloadplugin"), ReloadPlugin, GameAndConsole, Plugin, L".reloadplugin <plugin_shortname>",
             L"Unload then reload a plugin"),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".listplugins"), ListPlugins, GameAndConsole, Info, L".listplugins",
             L"List all plugins and their short names."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".chase"), Chase, GameOnly, Movement, L".chase <charname>",
             L"Teleport the admin to the location of the specified character. Does not traverse systems."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".beam"), Beam, GameAndConsole, Movement, L".beam <charname> <base>",
             L"Teleport the specified character to the specified base."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".pull"), Pull, GameOnly, Movement, L".pull <charname>",
             L"Pulls the specified character to your location. Does not traverse systems."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".damagemode"), SetDamageType, GameAndConsole, SuperAdmin, L".damagemode <all/none/pvp/pve>",
             L"Sets the source of allowed damage on the server."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".move"), Move, GameOnly, Movement, L".move <target> <x> <y> <z>",
             L"Moves the target to the destination location in space."),
             AddAdminCommand(AdminCommandProcessor, Cmds(L".help", L".?", L".h"), Help, GameAndConsole, Any, L".help [page]",
             L"Provides indepth help information") }
        };

        GetAdminCommandsFunc(commands);

        template <int N>
        std::wstring MatchCommand(AdminCommandProcessor* processor, std::wstring_view user, AllowedContext currentContext, std::wstring_view cmd,
                                  std::vector<std::wstring_view>& paramVector)
        {
            for (const auto& command = std::get<N - 1>(commands); auto& str : command.cmd)
            {
                if (cmd.starts_with(str))
                {
                    if (const auto validation = Validate(command.allowedContext, command.requiredRole); validation.has_error())
                    {
                        return validation.error();
                    }

                    paramVector.erase(paramVector.begin(), paramVector.begin() + std::clamp(std::ranges::count(str, L' '), 1, 5));
                    return command.func(processor, paramVector);
                }
            }

            return MatchCommand<N - 1>(processor, user, currentContext, cmd, paramVector);
        }

        template <>
        // ReSharper disable once CppExplicitSpecializationInNonNamespaceScope
        std::wstring MatchCommand<0>(AdminCommandProcessor* processor, std::wstring_view user, AllowedContext currentContext, std::wstring_view cmd,
                                     std::vector<std::wstring_view>& paramVector)
        {
            // No matching command was found
            return L"";
        }

        std::wstring ProcessCommand(const std::wstring_view user, const AllowedContext currentContext, std::wstring_view cmd,
                                    std::vector<std::wstring_view>& paramVector) override;

    public:
        std::wstring ProcessCommand(std::wstring_view user, AllowedContext currentContext, std::wstring_view commandString);
};
