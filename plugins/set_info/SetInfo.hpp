#pragma once

#include "API/FLHook/Plugin.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

#include <httplib.h>

namespace Plugins
{
    /**
     * @date July 2025
     * @author Aingar, ported by Laz
     * @brief
     * Allows a player to set a custom infocard that displays within the F9 menu. Requires FLUF integration
     *
     * @par Player Commands
     * - setinfo - Update the current player infocard
     * - showinfo - Display the current player infocard
     *
     * @par Admin Commands
     * There are no admin commands in this plugin.
     *
     * @note All player commands are prefixed with '/', all admin commands are prefixed with a '.'
     */
    class SetInfoPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            struct PlayerSetInfo
            {
                    bool initialised = false;
                    bool pulledInfos = false;
                    bool changedSinceLastLaunch = true;
                    std::string infocard;
            };

            using SetInfoFlufPayload = std::unordered_map<uint, std::string>;

            std::array<PlayerSetInfo, MaxClientId + 1> playersInfo;

            void PropagatePlayerInfo(ClientId client) const;
            void FetchPlayerInfo(ClientId client);
            void InitializePlayerInfo(ClientId client);

            void OnPlayerLaunchAfter(ClientId client, const ShipId& ship) override;

            void OnClearClientInfo(ClientId client) override;

            void OnCharacterSave(ClientId client, std::wstring_view charName, B_DOC& document) override;

            /**
             * @brief Used to change the info of the triggering player
             */
            concurrencpp::result<void> UserCmdSetInfo(ClientId client, std::wstring_view newInfo);

            /**
             * @brief Used to show the current infocard of this ship
             */
            concurrencpp::result<void> UserCmdShowInfo(ClientId client);

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
