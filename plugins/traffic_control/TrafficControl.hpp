#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

enum class Permissions
{
    LaneNet = 1 << 0,
    GateNet = 1 << 1,
    NoDock = 1 << 2,
    Scan = 1 << 3
};

inline Permissions operator|(Permissions p1, Permissions p2) { return static_cast<Permissions>(static_cast<int>(p1) | static_cast<int>(p2)); }

namespace Plugins
{

    class TrafficControlPlugin final : public Plugin, public AbstractUserCommandProcessor, public PacketInterface
    {
            using NetworkId = uint;
            struct Network final
            {
                    NetworkId networkId;
                    std::wstring networkName;
                    std::unordered_set<SystemId> networkSystems;
                    std::unordered_set<RepGroupId> nodockFactions;
            };

            struct NetworkData
            {
                    std::unordered_map<NetworkId, std::pair<Permissions, Network*>> availableNetworks;
                    Network* activeNetwork;
                    Permissions basePermissions;
                    Permissions currPermissions;
                    std::vector<std::wstring> scanCache;
                    uint64 timestamp;
                    uint64 nodockTimestamp;
            };

            struct Config final
            {
                    uint64 nodockDuration;
                    std::wstring nodockMessage;
                    std::unordered_map<NetworkId, Network> networks;
                    std::unordered_map<ClientId, Network*> subscribedClients;

                    std::unordered_map<uint, std::pair<Permissions, Network*>> equipAccesses;
                    std::unordered_map<uint, std::pair<Permissions, Network*>> IFFAccesses;
                    // TODO: Handle player tags
                    std::unordered_map<std::wstring, std::pair<Permissions, Network*>> tagsAccesses;
                    std::unordered_map<SystemId, std::unordered_set<Network*>> systemToNetworkMap;
            };

            struct SettingLoad
            {
                    NetworkId networkId;
                    Permissions perms;
                    std::string type;
                    std::string key;
            };

            struct ConfigLoad final
            {
                    uint64 nodockDuration = 60;
                    std::wstring nodockMessage = L"You've violated the law! Pay the court a fine or serve your sentence. Your stolen goods are now forfeit.";
                    std::vector<Network> networks;
                    std::vector<SettingLoad> accessList;
            };

            ConfigLoad configLoad;
            Config config;
            std::array<NetworkData, MaxClientId + 1> clientInfo;

            void ActivateNetwork(ClientId client, NetworkId networkId, const Permissions& permissions);

            bool OnSystemSwitchOutPacket(ClientId client, FLPACKET_SYSTEM_SWITCH_OUT& packet) override;

            void OnPlayerLaunchAfter(ClientId client, const ShipId& ship) override;
            void AddShipCargoSnapshot(ClientId client);
            void OnTradelaneStart(ClientId client, const XGoTradelane& tradelane) override;
            void OnClearClientInfo(ClientId client) override;
            std::optional<DOCK_HOST_RESPONSE> OnDockCall(const ShipId& shipId, const ObjectId& spaceId, int dockPortIndex,
                                                         DOCK_HOST_RESPONSE response) override;
            bool OnLoadSettings() override;

            Task UserCmdNetSwitch(ClientId client, std::wstring_view networkName);
            Task UserCmdNetList(ClientId client);
            Task UserCmdNet(ClientId client, std::wstring_view setting, bool newState);
            Task UserCmdNodockInfo(ClientId client);
            Task UserCmdNodock(ClientId client);

            const inline static std::array<CommandInfo<TrafficControlPlugin>, 5> commands = {
                {
                 AddCommand(TrafficControlPlugin, Cmds(L"/net switch"), UserCmdNetSwitch, L"/net switch [networkName]", L""),
                 AddCommand(TrafficControlPlugin, Cmds(L"/net list"), UserCmdNetList, L"/net list", L""),
                 AddCommand(TrafficControlPlugin, Cmds(L"/net"), UserCmdNet, L"/net <setting> <on|off>", L""),
                 AddCommand(TrafficControlPlugin, Cmds(L"/nodock info"), UserCmdNodockInfo, L"/nodock info", L""),
                 AddCommand(TrafficControlPlugin, Cmds(L"/nodock"), UserCmdNodock, L"/nodock", L""),
                 }
            }; // namespace Plugins

            SetupUserCommandHandler(TrafficControlPlugin, commands);

        public:
            explicit TrafficControlPlugin(const PluginInfo& info);
            ~TrafficControlPlugin() override;
    };
} // namespace Plugins
