#pragma once

#include "Core/Commands/AbstractAdminCommandProcessor.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

constexpr int LossInterval = 4;

namespace Plugins
{
    class ConnectionDataPlugin final : public Plugin, public AbstractUserCommandProcessor, public AbstractAdminCommandProcessor
    {
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

            //! The struct that holds client info for this plugin
            struct MiscClientInfo final
            {
                    bool lightsOn = false;
                    bool shieldsDown = false;
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

            void OnClearClientInfo(ClientId client) override;
            void TimerCheckKick();
            Task UserCmdPing(ClientId client);
            Task UserCmdPingTarget(ClientId client);
            Task AdminCmdGetStats(ClientId admin);
            bool OnLoadSettings() override;
            void OnSpObjectUpdateAfter(ClientId client, const SSPObjUpdateInfo& info) override;
            void PrintClientPing(ClientId clientToInform, ClientId clientToScan);
            void TimerUpdatePingData();
            void TimerUpdateLossData();
            void OnPlayerLaunchAfter(ClientId client, const ShipId& ship) override;
            void ClearConData(ClientId client);

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

            std::vector<std::shared_ptr<Timer>> timers;

            // Admin Commands
            // Timer Method

        public:
            explicit ConnectionDataPlugin(const PluginInfo& info);
            ~ConnectionDataPlugin();
    };
} // namespace Plugins
