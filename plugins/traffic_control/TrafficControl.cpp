/**
 * @date Feb, 2010
 * @author Cannon, ported by Raikkonen
 * @defgroup SystemSensor System Sensor
 * @brief
 * The plugin allows players with proper equipment to see player traffic coming through
 * Trade Lanes and Jump Gates in the system, as well as being able to look up
 * their equipment and cargo at the time of using them.
 *
 * @paragraph cmds Player Commands
 * -net <all/jumponly/off> - if player has proper equipment, toggles his scanner between showing JG/TL transits,
 *   JG transits only, and disabling the feature
 * -shoan <name> - shows equipment and cargo carried by the specified player
 * -shoan$ <playerID> - same as above, but using player ID as paramenter, useful for to type difficult names
 *
 * @paragraph adminCmds Admin Commands
 * None
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "sensors": [
 *         {"systemId": "Li01",
 *          "equipId": "li_gun01_mark01",
 *          "networkId": 1}
 *          ]
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

#include "PCH.hpp"

#include "TrafficControl.hpp"

#include "API/FLHook/InfocardManager.hpp"
#include "API/FLHook/ResourceManager.hpp"

using namespace Plugins;
using namespace magic_enum::bitwise_operators;

void TrafficControlPlugin::ActivateNetwork(const ClientId client, const NetworkId networkId, const Permissions& permissions)
{
    if (auto clientData = clientInfo[client.GetValue()]; clientData.availableNetworks.contains(networkId))
    {
        auto& newNetwork = config.networks.at(networkId);
        config.subscribedClients[client] = &newNetwork;

        clientData.activeNetwork = &newNetwork;
        clientData.basePermissions = permissions;
        clientData.currPermissions = permissions;

        client.Message(std::format(L"Traffic monitoring active for {} region.", config.networks.at(networkId).networkName));
    }
    else
    {
        client.Message(L"Error: selected network not available!");
    }
}

bool TrafficControlPlugin::OnSystemSwitchOutPacket(const ClientId client, FLPACKET_SYSTEM_SWITCH_OUT& packet)
{
    if (const auto* activeNetwork = clientInfo[client.GetValue()].activeNetwork; activeNetwork)
    {
        const auto solar = FLHook::GetResourceManager()->Get<CSolar>(packet.jumpObjectId);
        if (solar.expired())
        {
            return true;
        }

        const auto targetSystem = SystemId(solar.lock()->jumpDestSystem);
        if (activeNetwork->networkSystems.contains(targetSystem))
        {
            return true;
        }

        client.ToastMessage(L"Leaving Network", std::format(L"You have left the are of {} monitoring network", activeNetwork->networkName));

        for (const auto& availableNetworks = clientInfo[client.GetValue()].availableNetworks;
             const auto [permissions, network] : availableNetworks | std::views::values)
        {
            if (network->networkSystems.contains(targetSystem))
            {
                ActivateNetwork(client, network->networkId, permissions);
                break;
            }
        }
    }
    return true;
}

void TrafficControlPlugin::OnPlayerLaunchAfter(const ClientId client, const ShipId& ship)
{
    auto& playerNetworks = clientInfo[client.GetValue()].availableNetworks;
    playerNetworks.clear();

    for (EquipDesc& equip : client.GetEquipCargo().Handle()->equip)
    {
        if (const auto findResult = config.equipAccesses.find(equip.archId.GetValue()); findResult != config.equipAccesses.end())
        {
            playerNetworks[findResult->second.second->networkId] = findResult->second;
        }
    }

    if (const auto findResult = config.IFFAccesses.find(client.GetReputation().Handle().GetAffiliation().Handle().GetValue());
        findResult != config.IFFAccesses.end())
    {
        playerNetworks[findResult->second.second->networkId] = findResult->second;
    }

    if (playerNetworks.empty())
    {
        return;
    }

    ActivateNetwork(client, playerNetworks.begin()->first, playerNetworks.begin()->second.first);
}

void TrafficControlPlugin::AddShipCargoSnapshot(ClientId client)
{
    auto& clientData = clientInfo[client.GetValue()];
    auto ship = client.GetShip().Handle();

    clientData.scanCache.clear();
    clientData.scanCache.emplace_back(std::format(L"Scan snapshot of {}:", client.GetCharacterId().Handle()));
    clientData.scanCache.emplace_back(std::format(L"Ship: {}", FLHook::GetInfocardManager()->GetInfoName(ship.GetArchetype().Handle()->idsName)));

    auto* shipEqManager = ship.GetEquipmentManager().Handle();
    CEquipTraverser tr(static_cast<int>(EquipmentClass::Cargo));
    CECargo* cargo;
    while (cargo = reinterpret_cast<CECargo*>(shipEqManager->Traverse(tr)))
    {
        clientData.scanCache.emplace_back(std::format(L"| {}x{}", FLHook::GetInfocardManager()->GetInfoName(cargo->archetype->idsInfo), cargo->count));
    }

    clientData.timestamp = TimeUtils::UnixTime<std::chrono::seconds>();
}

void TrafficControlPlugin::OnTradelaneStart(ClientId client, const XGoTradelane& tradelane)
{
    const auto networksToNotify = config.systemToNetworkMap.find(client.GetSystemId().Handle());
    if (networksToNotify == config.systemToNetworkMap.end())
    {
        return;
    }

    bool isFirstNotification = true;

    std::vector<std::wstring> message;
    for (const auto& notifiedClient : config.subscribedClients)
    {
        if (networksToNotify->second.contains(notifiedClient.second))
        {
            continue;
        }
        if (magic_enum::enum_flags_test(clientInfo[notifiedClient.first.GetValue()].currPermissions, Permissions::LaneNet))
        {
            SystemId clientSystem = notifiedClient.first.GetSystemId().Handle();
            notifiedClient.first.Message(
                std::format(L"{} has entered tradelane in {} system, sector {}",
                            client.GetCharacterId().Handle(),
                            clientSystem.GetName().Handle(),
                            clientSystem.PositionToSectorCoord(notifiedClient.first.GetShip().Handle().GetPositionAndOrientation().Handle().first).Handle()));
        }

        if (isFirstNotification && magic_enum::enum_flags_test(clientInfo[notifiedClient.first.GetValue()].currPermissions, Permissions::Scan))
        {
            isFirstNotification = false;
            AddShipCargoSnapshot(client);
        }
    }
}

void TrafficControlPlugin::OnClearClientInfo(const ClientId client)
{
    auto clientData = clientInfo[client.GetValue()];
    clientData.availableNetworks.clear();
    clientData.activeNetwork = nullptr;

    activePoliceSirens.erase(client);
}

std::optional<DOCK_HOST_RESPONSE> TrafficControlPlugin::OnDockCall(const ShipId& shipId, const ObjectId& spaceId, int dockPortIndex,
                                                                   DOCK_HOST_RESPONSE response)
{
    if (!shipId.IsPlayer())
    {
        return {};
    }

    const auto client = ClientId(shipId.GetPlayer().Unwrap());

    const auto& data = clientInfo[client.GetValue()];
    if (!data.nodockTimestamp)
    {
        return {};
    }

    const auto currTime = TimeUtils::UnixTime<std::chrono::seconds>();
    if (data.nodockTimestamp > currTime)
    {
        return {};
    }

    client.Message(L"Your dock access has been temporarily suspended");
    return DOCK_HOST_RESPONSE::DockDenied;
}

bool TrafficControlPlugin::OnLoadSettings()
{
    LoadJsonWithValidation(ConfigLoad, configLoad, "config/traffic_control.json");

    config.nodockDuration = configLoad.nodockDuration;
    config.nodockMessage = configLoad.nodockMessage;
    for (auto& network : configLoad.networks)
    {
        config.networks[network.networkId] = network;
    }

    for (auto& [networkId, perms, type, key] : configLoad.accessList)
    {
        if (type == "equip")
        {
            config.equipAccesses[CreateID(key.c_str())] = { perms, &config.networks.at(networkId) };
        }
        else if (type == "affiliation")
        {
            config.IFFAccesses[CreateID(key.c_str())] = { perms, &config.networks.at(networkId) };
        }
        else if (type == "tag")
        {
            config.tagsAccesses[StringUtils::stows(key)] = { perms, &config.networks.at(networkId) };
        }
    }

    for (auto network : config.networks | std::views::values)
    {
        for (auto system : network.networkSystems)
        {
            config.systemToNetworkMap[system].insert(&network);
        }
    }

    return true;
}

concurrencpp::result<void> TrafficControlPlugin::UserCmdNetSwitch(ClientId client, std::wstring_view networkName)
{
    auto clientData = clientInfo[client.GetValue()];
    if (clientData.availableNetworks.empty())
    {
        client.MessageErr(L"No networks available!");
        co_return;
    }

    for (auto& [permissions, network] : clientData.availableNetworks | std::views::values)
    {
        if (!StringUtils::CompareCaseInsensitive(networkName, std::wstring_view(network->networkName)))
        {
            continue;
        }

        ActivateNetwork(client, network->networkId, permissions);
        co_return;
    }

    client.MessageErr(L"Provided network doesn't exist or you don't have access");

    co_return;
}

concurrencpp::result<void> TrafficControlPlugin::UserCmdNetList(ClientId client)
{
    auto clientData = clientInfo[client.GetValue()];
    if (clientData.availableNetworks.empty())
    {
        client.MessageErr(L"No networks available for connection!");
        co_return;
    }

    for (const auto network : clientData.availableNetworks | std::views::values | std::views::values)
    {
        if (network == clientData.activeNetwork)
        {
            client.Message(std::format(L"{} - Active", network->networkName));
        }
        else
        {
            client.Message(std::format(L"{}", network->networkName));
        }
    }

    co_return;
}

concurrencpp::result<void> TrafficControlPlugin::UserCmdNet(ClientId client, const std::wstring_view setting, const bool newState)
{
    auto clientData = clientInfo[client.GetValue()];
    if (!clientData.activeNetwork)
    {
        client.MessageErr(L"No network connection!");
        co_return;
    }

    if (StringUtils::ToLower(setting) == L"lane")
    {
        if (!magic_enum::enum_flags_test(clientData.basePermissions, Permissions::LaneNet) && newState)
        {
            client.MessageErr(L"No lane monitoring permissions!");
            co_return;
        }

        if (newState)
        {
            clientData.basePermissions |= Permissions::LaneNet;
        }
        else
        {
            clientData.basePermissions &= Permissions::LaneNet;
        }
    }
    else if (StringUtils::ToLower(setting) == L"gate")
    {
        if (!magic_enum::enum_flags_test(clientData.basePermissions, Permissions::GateNet) && newState)
        {
            client.MessageErr(L"No gate monitoring permissions!");
            co_return;
        }

        if (newState)
        {
            clientData.basePermissions |= Permissions::GateNet;
        }
        else
        {
            clientData.basePermissions &= Permissions::GateNet;
        }
    }
    else
    {
        client.MessageErr(L"Incorrect parameters!");
    }

    co_return;
}

concurrencpp::result<void> TrafficControlPlugin::UserCmdNodockInfo(ClientId client)
{
    const auto network = clientInfo[client.GetValue()].activeNetwork;
    if (!network)
    {
        client.MessageErr(L"No network connection!");
        co_return;
    }

    if (network->nodockFactions.empty())
    {
        client.MessageErr(L"This network doesn't allow of dock restrictions!");
        co_return;
    }

    client.Message(L"Restrictable station affiliations:");
    for (auto& faction : network->nodockFactions)
    {
        client.Message(std::format(L"| {}", faction.GetName().Handle()));
    }

    co_return;
}

concurrencpp::result<void> TrafficControlPlugin::UserCmdNodock(ClientId client)
{
    const auto data = clientInfo[client.GetValue()];

    if (!data.activeNetwork)
    {
        client.MessageErr(L"No active traffic network!");
        co_return;
    }

    if (!magic_enum::enum_flags_test(data.currPermissions, Permissions::NoDock))
    {
        client.MessageErr(L"No permission for this action!");
        co_return;
    }

    const auto ship = client.GetShip().Handle();

    if (!ship)
    {
        client.MessageErr(L"Not in space!");
        co_return;
    }

    const auto target = ship.GetTarget().Handle();
    const auto targetPlayer = target.GetPlayer().Handle();

    targetPlayer.Message(config.nodockMessage);
    clientInfo[targetPlayer.GetValue()].nodockTimestamp = TimeUtils::UnixTime<std::chrono::seconds>() + config.nodockDuration;
    co_return;
}

concurrencpp::result<void> TrafficControlPlugin::UserCmdPoliceSiren(ClientId client)
{
    if (!config.policeFuse)
    {
        client.MessageErr(L"Traffic Control plugin not fully setup. Cannot activate siren.");
        co_return;
    }

    const auto ship = client.GetShip().Handle();
    if (activePoliceSirens.contains(client))
    {
        ship.ExtinguishFuse(config.policeFuse);
        activePoliceSirens.erase(client);
        client.ToastMessage(L"Siren Off", L"Police system successfully deactivated");
        co_return;
    }

    auto* shipEqManager = ship.GetEquipmentManager().Handle();
    CEquipTraverser tr;
    CEquip* equip;
    while ((equip = shipEqManager->Traverse(tr)))
    {
        if (!config.equipmentForPoliceFuse.contains(equip->EquipArch()->archId))
        {
            continue;
        }

        activePoliceSirens.insert(client);
        ship.IgniteFuse(config.policeFuse);
        client.ToastMessage(L"Siren On", L"Police system successfully activated");
        break;
    }

    co_return;
}

TrafficControlPlugin::TrafficControlPlugin(const PluginInfo& info) : Plugin(info) {}
TrafficControlPlugin::~TrafficControlPlugin() = default;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Traffic Control",
	    .shortName = L"traffic_control",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(TrafficControlPlugin);
