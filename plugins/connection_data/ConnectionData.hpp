#pragma once

#include "Core/Commands/AbstractAdminCommandProcessor.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

constexpr int LossInterval = 4;

namespace Plugins
{
    /**
     * @date unknown
     * @author Cannon (Ported in 2022)
     * @brief
     * Connection data analysis tool, provides players and the server with average latency, loss, latency fluct data.
     *
     * @par Configuration
     * @code
     * {
     *     "enableBountyHunt": true,
     *     "levelProtect": 0,
     *     "minimalHuntTime": 1,
     *     "maximumHuntTime": 240,
     *     "defaultHuntTime": 30
     * }
     * @endcode
     *
     * @par Player Commands
     * All commands are prefixed with '/' unless explicitly specified.
     * - bountyhunt <player> <amount> [timelimit] - Places a bounty on the specified player. When another player kills them, they gain <credits>.
     * - bountyhuntid <id> <amount> [timelimit] - Same as above but with an id instead of a player name. Use /ids
     *
     * @par Admin Commands
     * There are no admin commands in this plugin.
     *
     * @note All player commands are prefixed with '/'.
     * All admin commands are prefixed with a '.'.
     */
    class ConnectionDataPlugin final : public Plugin, public AbstractUserCommandProcessor, public AbstractAdminCommandProcessor
    {
            /// @brief Connection data struct
            struct ConnectionData
            {
                    // connection data
                    std::list<uint> lossList;
                    uint lastLoss = 0;
                    uint averageLoss = 0;
                    std::list<uint> pingList;
                    uint averagePing = 0;
                    //! Variation in minimal and maximum ping between client and server.
                    uint pingFluctuation = 0;
                    uint lastPacketsSent = 0;
                    uint lastPacketsReceived = 0;
                    uint lastPacketsDropped = 0;
                    uint lags = 0;
                    std::list<uint> objUpdateIntervalsList;
                    mstime lastObjUpdate = 0;
                    mstime lastObjTimestamp = 0;

                    // exception
                    bool exception = false;
                    std::string exceptionReason = "";
            };

            struct ConnectionDataException final
            {
                    ClientId client;
                    bool isException;
                    std::string reason;
            };

            struct Config final
            {
                    uint pingKick = 0;
                    uint pingKickFrame = 120;
                    uint fluctKick = 0;
                    uint lossKick = 0;
                    uint lossKickFrame = 120;
                    uint lagKick = 0;
                    uint lagDetectionFrame = 50;
                    uint lagDetectionMin = 50;
                    uint kickThreshold = 0;
                    bool allowPing = true;

                    int kickTimer = 10;
            };

            //! Global data for this plugin
            Config config;

            // Other fields
            ConnectionData connectionData[MaxClientId + 1];

            /// @brief Periodically checks and kicks players whose connections are too poor
            void TimerCheckKick();
            Task UserCmdPing(ClientId client);
            Task UserCmdPingTarget(ClientId client);
            Task AdminCmdGetStats(ClientId admin);
            /// @brief Runs once a second, updates the average ping data
            void TimerUpdatePingData();
            /// @brief Periodically updates the average packet loss data
            void TimerUpdateLossData();
            void PrintClientPing(ClientId clientToInform, ClientId clientToScan);
            void ClearConData(ClientId client);

            void OnClearClientInfo(ClientId client) override;
            bool OnLoadSettings() override;
            /// @brief Hook on client position packet update, used for speed hack detection.
            void OnSpObjectUpdateAfter(ClientId client, const SSPObjUpdateInfo& info) override;
            /// @brief Launch hook, resets player telemetry data.
            void OnPlayerLaunchAfter(ClientId client, const ShipId& ship) override;

            // clang-format off
            inline static const std::array<CommandInfo<ConnectionDataPlugin>, 2> commands =
            {
                {
                    AddCommand(ConnectionDataPlugin, Cmds(L"/ping"), UserCmdPing, L"/ping", L""),
                    AddCommand(ConnectionDataPlugin, Cmds(L"/pingtarget"), UserCmdPingTarget, L"/pingtarget", L""),
                }
            };

            inline static const std::array<AdminCommandInfo<ConnectionDataPlugin>, 1> adminCommands =
            {
                {
                    AddAdminCommand(ConnectionDataPlugin, Cmds(L".getstats"), AdminCmdGetStats, GameAndConsole, SuperAdmin, L".getstats", L""),
                }
            };
            // clang-format on

            SetupUserCommandHandler(ConnectionDataPlugin, commands);
            SetupAdminCommandHandler(ConnectionDataPlugin, adminCommands);

        public:
            explicit ConnectionDataPlugin(const PluginInfo& info);
    };
} // namespace Plugins
