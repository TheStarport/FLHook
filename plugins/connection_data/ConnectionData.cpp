// TODO: PENDING REBUILD
// TODO: MOVE ALL FUNCTIONALITY TO CORE AND STORE IN DATABASE

#include "PCH.hpp"

#include "ConnectionData.hpp"

#include "API/FLHook/ClientList.hpp"
#include "FLCore/FLCoreDALib.h"

namespace Plugins
{
    /** @ingroup Condata
     * @brief Clears our connection data for the specified client.
     */
    void ConnectionDataPlugin::ClearConData(ClientId client) { connectionData[client.GetValue()] = ConnectionData(); }

    void ConnectionDataPlugin::OnClearClientInfo(ClientId client) { ClearConData(client); }

    /** @ingroup Condata
     * @brief Hook on TimerCheckKick. Checks clients's connectionData against a threshold and kicks them if they are above it.
     */
    void ConnectionDataPlugin::TimerCheckKick()
    {
        if (FLHook::GetServerLoadInMs() > config.kickThreshold)
        {
            // for all players
            PlayerData* playerData = nullptr;
            while ((playerData = Players.traverse_active(playerData)))
            {
                ClientId client = ClientId(playerData->clientId);
                if (client.GetValue() < 1 || client.GetValue() > MaxClientId)
                {
                    continue;
                }

                auto& con = connectionData[client.GetValue()];

                if (config.lossKick && con.averageLoss > config.lossKick)
                {
                    con.lossList.clear();
                    client.Kick(L"High loss", 5);
                }

                if (config.pingKick && con.averagePing > (config.pingKick))
                {
                    con.pingList.clear();
                    client.Kick(L"High ping", 5);
                }

                if (config.fluctKick && con.pingFluctuation > (config.fluctKick))
                {
                    con.pingList.clear();
                    client.Kick(L"High ping fluctuation", 5);
                }

                if (config.lagKick && con.lags > (config.lagKick))
                {
                    con.objUpdateIntervalsList.clear();

                    client.Kick(L"High lag", 5);
                }
            }
        }

        // Are there accounts connected with client Ids greater than max player
        // count? If so, kick them as FLServer is buggy and will use high client Ids
        // but will not allow character selection on them.
        for (uint client = Players.GetMaxPlayerCount() + 1; client <= MaxClientId; client++)
        {
            if (Players[client].clientId)
            {
                if (CAccount* acc = Players.FindAccountFromClientID(client))
                {
                    acc->ForceLogout();
                    Players.logout(client);
                }
            }
        }
    }

    /** @ingroup Condata
     * @brief Update average ping data.
     */
    void ConnectionDataPlugin::TimerUpdatePingData()
    {
        // for all players
        PlayerData* playerData = nullptr;
        while ((playerData = Players.traverse_active(playerData)))
        {
            ClientId client = ClientId(playerData->clientId);
            auto connectionInfo = client.GetConnectionData();
            if (client.GetValue() < 1 || client.GetValue() > MaxClientId || client.GetData().f1Time || connectionInfo.Raw().has_error())
            {
                continue;
            }

            auto& con = connectionData[client.GetValue()];

            ///////////////////////////////////////////////////////////////
            // update ping data
            if (con.pingList.size() >= config.pingKickFrame)
            {
                // calculate average ping and ping fluctuation
                unsigned int lastPing = 0;
                con.averagePing = 0;
                con.pingFluctuation = 0;
                for (const auto& ping : con.pingList)
                {
                    con.averagePing += ping;
                    if (lastPing != 0)
                    {
                        con.pingFluctuation += static_cast<uint>(sqrt(pow(static_cast<float>(ping) - static_cast<float>(lastPing), 2)));
                    }
                    lastPing = ping;
                }

                con.pingFluctuation /= con.pingList.size();
                con.averagePing /= con.pingList.size();
            }

            // remove old pingdata
            while (con.pingList.size() >= config.pingKickFrame)
            {
                con.pingList.pop_back();
            }

            con.pingList.push_front(connectionInfo.Handle().roundTripLatencyMS);
        }
    }

    /** @ingroup Condata
     * @brief Update average loss data.
     */
    void ConnectionDataPlugin::TimerUpdateLossData()
    {
        // for all players
        float lossPercentage;
        PlayerData* playerData = nullptr;
        while ((playerData = Players.traverse_active(playerData)))
        {
            ClientId client = ClientId(playerData->clientId);
            if (client.GetValue() < 1 || client.GetValue() > MaxClientId)
            {
                continue;
            }

            if (client.GetData().f1Time)
            {
                continue;
            }
            auto connectionInfo = client.GetConnectionData();

            if (connectionInfo.Raw().has_error())
            {
                continue;
            }
            const auto& connInfo = connectionInfo.Handle();

            auto& con = connectionData[client.GetValue()];

            ///////////////////////////////////////////////////////////////
            // update loss data
            if (con.lossList.size() >= (config.lossKickFrame / LossInterval))
            {
                // calculate average loss
                con.averageLoss = 0;
                for (const auto& loss : con.lossList)
                {
                    con.averageLoss += loss;
                }

                con.averageLoss /= con.lossList.size();
            }

            // remove old lossdata
            while (con.lossList.size() >= (config.lossKickFrame / LossInterval))
            {
                con.lossList.pop_back();
            }

            // sum of Drops = Drops guaranteed + drops non-guaranteed
            const uint newDrops = (connInfo.packetsRetried + connInfo.packetsDropped) - con.lastPacketsDropped;

            // % of Packets Lost = Drops / (sent+received) * 100
            if (const uint newSent = (connInfo.packetsSentGuaranteed + connInfo.packetsSentNonGuaranteed) - con.lastPacketsSent;
                newSent > 0) // division by zero check
            {
                lossPercentage = static_cast<float>(newDrops) / static_cast<float>(newSent) * 100.0f;
            }
            else
            {
                lossPercentage = 0.0;
            }

            if (lossPercentage > 100)
            {
                lossPercentage = 100;
            }

            // add last loss to List lossList and put current value into lastLoss
            con.lossList.push_front(con.lastLoss);
            con.lastLoss = static_cast<uint>(lossPercentage);

            // Fill new ClientInfo-variables with current values
            con.lastPacketsSent = connInfo.packetsSentGuaranteed + connInfo.packetsSentNonGuaranteed;
            con.lastPacketsDropped = connInfo.packetsRetried + connInfo.packetsDropped;
        }
    }

    /** @ingroup Condata
     * @brief Hook on PlayerLaunch. Sets lastObjUpdate to 0.
     */
    void ConnectionDataPlugin::OnPlayerLaunchAfter(ClientId client, const ShipId& ship) { connectionData[client.GetValue()].lastObjUpdate = 0; }

    /** @ingroup Condata
     * @brief Hook on SPObjUpdate. Updates timestamps for lag detection.
     */
    void ConnectionDataPlugin::OnSpObjectUpdateAfter(ClientId client, const SSPObjUpdateInfo& info)
    {
        // lag detection
        if (!client.InSpace())
        {
            return; // ??? 8[
        }

        const mstime timeNow = TimeUtils::UnixTime<std::chrono::milliseconds>();
        const auto timestamp = static_cast<mstime>(info.timestamp * 1000);

        auto& con = connectionData[client.GetValue()];

        if (config.lagDetectionFrame && con.lastObjUpdate && (client.GetEngineState() != EngineState::Tradelane) && (info.state != 7))
        {
            const auto timeDiff = static_cast<uint>(timeNow - con.lastObjUpdate);
            const auto timestampDiff = static_cast<uint>(timestamp - con.lastObjTimestamp);
            auto diff = static_cast<int>(sqrt(pow(static_cast<long double>(static_cast<int>(timeDiff) - static_cast<int>(timestampDiff)), 2)));
            diff -= static_cast<int>(FLHook::GetServerLoadInMs());
            if (diff < 0)
            {
                diff = 0;
            }

            uint perc;
            if (timestampDiff != 0)
            {
                perc = static_cast<uint>(static_cast<float>(diff) / static_cast<float>(timestampDiff) * 100.0f);
            }
            else
            {
                perc = 0;
            }

            if (con.objUpdateIntervalsList.size() >= config.lagDetectionFrame)
            {
                uint lags = 0;
                for (const auto& iv : con.objUpdateIntervalsList)
                {
                    if (iv > config.lagDetectionMin)
                    {
                        lags++;
                    }
                }

                con.lags = (lags * 100) / config.lagDetectionFrame;
                while (con.objUpdateIntervalsList.size() >= config.lagDetectionFrame)
                {
                    con.objUpdateIntervalsList.pop_front();
                }
            }

            con.objUpdateIntervalsList.push_back(perc);
        }

        con.lastObjUpdate = timeNow;
        con.lastObjTimestamp = timestamp;
    }

    void ConnectionDataPlugin::PrintClientPing(ClientId clientToInform, ClientId clientToScan)
    {
        const auto& con = connectionData[clientToScan.GetValue()];

        std::wstring response = L"Ping";
        response += L": ";
        if (con.pingList.size() < config.pingKickFrame)
        {
            response += L"n/a Fluct: n/a ";
        }
        else
        {
            response += std::to_wstring(con.averagePing);
            response += L"ms ";
            if (config.pingKick > 0)
            {
                response += L"(Max: ";
                response += std::to_wstring(config.pingKick);
                response += L"ms) ";
            }
            response += L"Fluct: ";
            response += std::to_wstring(con.pingFluctuation);
            response += L"ms ";
            if (config.fluctKick > 0)
            {
                response += L"(Max: ";
                response += std::to_wstring(config.fluctKick);
                response += L"ms) ";
            }
        }

        response += L"Loss: ";
        if (con.lossList.size() < (config.lossKickFrame / LossInterval))
        {
            response += L"n/a ";
        }
        else
        {
            response += std::to_wstring(con.averageLoss);
            response += L"% ";
            if (config.lossKick > 0)
            {
                response += L"(Max: ";
                response += std::to_wstring(config.lossKick);
                response += L"%) ";
            }
        }

        response += L"Lag: ";
        if (con.objUpdateIntervalsList.size() < config.lagDetectionFrame)
        {
            response += L"n/a";
        }
        else
        {
            response += std::to_wstring(con.lags).c_str();
            response += L"% ";
            if (config.lagKick > 0)
            {
                response += L"(Max: ";
                response += std::to_wstring(config.lagKick);
                response += L"%)";
            }
        }

        // Send the message to the user
        clientToInform.Message(response);
    }

    /** @ingroup Condata
     * @brief Gets called when the player types /ping
     */
    concurrencpp::result<void> ConnectionDataPlugin::UserCmdPing(ClientId client)
    {
        if (!config.allowPing)
        {
            (void)client.MessageErr(L"Command disabled");
            co_return;
        }

        PrintClientPing(client, client);

        co_return;
    }

    concurrencpp::result<void> ConnectionDataPlugin::UserCmdPingTarget(ClientId client)
    {
        if (!config.allowPing)
        {
            (void)client.MessageErr(L"Command disabled");
            co_return;
        }
        const ShipId ship = client.GetShip().Handle();
        const auto target = ship.GetTarget().Handle();

        const auto targetClient = target.GetPlayer().Unwrap();
        if (!targetClient)
        {
            (void)client.MessageErr(L"Target not a player");
            co_return;
        }

        PrintClientPing(client, targetClient);
        co_return;
    }

    /** @ingroup Condata
     * @brief Process admin commands.
     */
    concurrencpp::result<void> ConnectionDataPlugin::AdminCmdGetStats(ClientId admin)
    {
        PlayerData* playerData = nullptr;
        while ((playerData = Players.traverse_active(playerData)))
        {
            ClientId client = ClientId(playerData->clientId);
            if (client.InCharacterSelect())
            {
                continue;
            }

            CDPClientProxy* cdpClient = FLHook::GetClientProxyArray()[client.GetValue() - 1];
            if (!cdpClient)
            {
                continue;
            }

            auto& con = connectionData[client.GetValue()];
            auto saturation = static_cast<int>(cdpClient->GetLinkSaturation() * 100);
            int txqueue = cdpClient->GetSendQSize();
            admin.Message(std::format(L"charname={} clientid={} loss={} lag={} pingfluct={} saturation={} txqueue={}\n",
                                      client.GetCharacterId().Handle(),
                                      client,
                                      con.averageLoss,
                                      con.lags,
                                      con.pingFluctuation,
                                      saturation,
                                      txqueue));
        }

        co_return;
    }

    bool ConnectionDataPlugin::OnLoadSettings()
    {
        if (const auto conf = Json::Load<Config>("config/connectiondata.json"); !conf.second.has_value())
        {
            Json::Save(config, "config/connectiondata.json");
        }
        else
        {
            config = conf.second.value();
        }

        return true;
    }

    ConnectionDataPlugin::ConnectionDataPlugin(const PluginInfo& info) : Plugin(info)
    {
        AddTimer([this] { TimerCheckKick(); }, 1000);
        AddTimer([this] { TimerUpdatePingData(); }, 1000);
        AddTimer([this] { TimerUpdateLossData(); }, LossInterval);
    }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"ConData",
	    .shortName = L"condata",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(ConnectionDataPlugin);
