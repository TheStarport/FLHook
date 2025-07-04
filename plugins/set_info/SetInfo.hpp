#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

#include <httplib.h>

namespace Plugins
{
    /**
     * @date July 2025
     * @author Aingar, ported by Laz
     * @brief
     * This plugin is used to teleport players to/from an arena system for the purpose of pvp.
     *
     * @par Player Commands
     * - arena (configurable) - This beams the player to the pvp system.
     * - return - This returns the player to their last docked base.
     *
     * @par Admin Commands
     * There are no admin commands in this plugin.
     *
     * @note All player commands are prefixed with '/', all admin commands are prefixed with a '.'
     */
    class SetInfoPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            std::unordered_map<std::wstring, std::wstring> pendingSaves;

            void OnPlayerLaunchAfter(ClientId client, const ShipId& ship) override;

            void OnClearClientInfo(ClientId client) override;

            void OnCharacterSave(ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document) override;

            /**
             * @brief Used to change the info of the triggering player
             */
            concurrencpp::result<void>UserCmdSetInfo(ClientId client, std::wstring_view newInfo);

            /**
             * @brief Used to show the current infocard of this ship
             */
            concurrencpp::result<void>UserCmdShowInfo(ClientId client);

            // clang-format off
            inline static const std::array<CommandInfo<SetInfoPlugin>, 2> commands = {
            {
                AddCommand(SetInfoPlugin, Cmds(L"/setinfo"), UserCmdSetInfo, L"/setinfo", L"Set an infocard for the current ship."),
                AddCommand(SetInfoPlugin, Cmds(L"/showinfo"), UserCmdShowInfo, L"/showinfo", L"Show the current infocard set for this ship.")}
            };
            SetupUserCommandHandler(SetInfoPlugin, commands);
            // clang-format on

        public:
            explicit SetInfoPlugin(const PluginInfo& info);
    };
} // namespace Plugins
