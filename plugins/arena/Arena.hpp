#pragma once

#include "API/FLHook/Plugin.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

#include <httplib.h>

namespace Plugins
{
    /**
     * @date August, 2022
     * @author MadHunter (Ported by Raikkonen 2022)
     * @brief
     * This plugin is used to teleport players to/from an arena system for the purpose of pvp.
     *
     * @par Configuration
     * The configuration file is generated at config/arena.json. An example configuration is as follows:
     * @code
     * {
     *     "restrictedSystems": [ "Li01", "Li02" ],
     *     "targetBase": "Li02_01_Base",
     *     "targetSystem": "Li02"
     * }
     * @endcode
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
    class ArenaPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            enum class TransferFlag
            {
                None,
                Transfer,
                Return
            };

            const std::wstring dockErrorText = L"Please dock at nearest base";
            const std::wstring cargoErrorText = L"Cargo hold is not empty";

            struct Config final
            {
                    BaseId targetBase;
                    SystemId targetSystem;
                    std::vector<SystemId> restrictedSystems;
            };

            Config config;

            struct ArenaClientData
            {
                    TransferFlag flag;
                    BaseId returnBase;
                    std::vector<FLCargo> storedCargo;
            };

            std::unordered_map<ClientId, ArenaClientData> clientData;

            /**
             * @brief Used to switch to the arena system
             */
            concurrencpp::result<void> UserCmdArena(ClientId client);

            /**
             * @brief Used to return from the arena system.
             */
            concurrencpp::result<void> UserCmdReturn(ClientId client);

            // clang-format off
            inline static const std::array<CommandInfo<ArenaPlugin>, 2> commands =
            {
                {
                    AddCommand(ArenaPlugin, Cmds(L"/arena", L"/conn"), UserCmdArena, L"/arena", L"Sends you to the designated arena system."),
                    AddCommand(ArenaPlugin, Cmds(L"/return"), UserCmdReturn, L"/return", L"Returns you from the arena system to where you last docked.")
                }
            };
            SetupUserCommandHandler(ArenaPlugin, commands);
            // clang-format on

            void OnClearClientInfo(ClientId client) override;
            bool OnLoadSettings() override;

            void RestoreCargo(const ClientId client);
            void StoreCargo(const ClientId client);

            /**
             * @brief Hook on CharacterSelect. Sets their transfer flag to "None".
             */
            void OnCharacterSelectAfter(ClientId client) override;

            /**
             * @brief Hook on PlayerLaunch. If their transfer flags are set appropriately,
             * redirect the undock to either the arena base or the return point
             */
            void OnPlayerLaunchAfter(ClientId client, [[maybe_unused]] const ShipId& ship) override;

            /**
             * @brief Hook on OnCharacterSave. Adds the return base to the character's database document.
             */
            void OnCharacterSave(ClientId client, std::wstring_view charName, B_DOC& document) override;

            /**
             * @brief This returns the return base id that is stored in the client's save file.
             */
            static BaseId ReadArenaDataForClient(ClientId client);

            /**
             * @brief Hook on HttpServerRegister. If enabled, registers the following routes:
             * @par GET '/plugins/arena/usage' - Reports the players currently within the arena
             */
            void OnHttpServerRegister(std::shared_ptr<httplib::Server> httpServer) override;
            httplib::StatusCode GetCurrentArenaUsers(const httplib::Request& request, httplib::Response& response);

        public:
            explicit ArenaPlugin(const PluginInfo& info);
    };
} // namespace Plugins
