/**
 * @date August, 2022
 * @author w0dk4
 * @defgroup Condata Connection Data
 * @brief
 * This plugin is used to provide connection data e.g. ping
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - ping - This shows connection data to the player.
 *
 * @paragraph adminCmds Admin Commands
 * All commands are prefixed with '.' unless explicitly specified.
 * - getstats - Gets connection stats on all connected clients.
 * - kick <character> - Kicks the specified character.
 *
 * @paragraph configuration Configuration
 * No configuration file is needed.
 *
 * @paragraph ipc IPC Interfaces Exposed
 * - ReceiveData - See function documentation below
 * - ReceiveException - See function documentation below
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin uses the "Tempban" plugin.
 */

#include "Condata.h"
#include "Features/TempBan.hpp"

#define PRINT_ERROR()                                                        \
	{                                                                        \
		for (uint i = 0; (i < sizeof(wscError) / sizeof(std::wstring)); i++) \
			PrintUserCmdText(client, wscError[i]);                           \
		return;                                                              \
	}
#define PRINT_OK() PrintUserCmdText(client, L"OK");
#define PRINT_DISABLED() PrintUserCmdText(client, L"Command disabled");

namespace Plugins::ConData
{
	constexpr int KickTimer = 10;
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup Condata
	 * @brief Clears our connection data for the specified client.
	 */
	void ClearConData(ClientId client)
	{
		auto con = global->connections[client];
		con.averageLoss = 0;
		con.averagePing = 0;
		con.lastLoss = 0;
		con.lastPacketsDropped = 0;
		con.lastPacketsReceived = 0;
		con.lastPacketsSent = 0;
		con.pingFluctuation = 0;
		con.lossList.clear();
		con.pingList.clear();
		con.objUpdateIntervalsList.clear();
		con.lags = 0;
		con.lastObjUpdate = 0;
		con.lastObjTimestamp = 0;

		con.exception = false;
		con.exceptionReason = "";
	}

	/** @ingroup Condata
	 * @brief ClearClientInfo hook. Calls ClearConData().
	 */
	void ClearClientInfo(ClientId& client)
	{
		ClearConData(client);
	}

	/** @ingroup Condata
	 * @brief Hook on TimerCheckKick. Checks clients's connections against a threshold and kicks them if they are above it.
	 */
	void TimerCheckKick()
	{
		if (CoreGlobals::c()->serverLoadInMs > global->config->kickThreshold)
		{
			// for all players
			struct PlayerData* playerData = nullptr;
			while ((playerData = Players.traverse_active(playerData)))
			{
				ClientId client = playerData->iOnlineId;
				if (client < 1 || client > MaxClientId)
					continue;

				auto& con = global->connections[client];

				if (global->config->lossKick && con.averageLoss > global->config->lossKick)
				{
					con.lossList.clear();
					AddKickLog(client, "High loss");
					TempBanManager::i()->AddTempBan(client, 60, L"High loss");
				}

				if (global->config->pingKick && con.averagePing > (global->config->pingKick))
				{
					con.pingList.clear();
					AddKickLog(client, "High ping");
					TempBanManager::i()->AddTempBan(client, 60, L"High ping");
				}

				if (global->config->fluctKick && con.pingFluctuation > (global->config->fluctKick))
				{
					con.pingList.clear();
					AddKickLog(client, "High fluct");
					TempBanManager::i()->AddTempBan(client, 60, L"High ping fluctuation");
				}

				if (global->config->lagKick && con.lags > (global->config->lagKick))
				{
					con.objUpdateIntervalsList.clear();

					AddKickLog(client, "High Lag");
					TempBanManager::i()->AddTempBan(client, 60, L"High lag");
				}
			}
		}

		// Are there accounts connected with client Ids greater than max player
		// count? If so, kick them as FLServer is buggy and will use high client Ids
		// but will not allow character selection on them.
		for (uint client = Players.GetMaxPlayerCount() + 1; client <= MaxClientId; client++)
		{
			if (Players[client].iOnlineId)
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
	void TimerUpdatePingData()
	{
		// for all players
		struct PlayerData* playerData = nullptr;
		while ((playerData = Players.traverse_active(playerData)))
		{
			ClientId client = playerData->iOnlineId;
			const auto connectionInfo = Hk::Admin::GetConnectionStats(client);
			if (client < 1 || client > MaxClientId || ClientInfo[client].tmF1TimeDisconnect || connectionInfo.has_error())
				continue;

			auto& con = global->connections[client];

			///////////////////////////////////////////////////////////////
			// update ping data
			if (con.pingList.size() >= global->config->pingKickFrame)
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
			while (con.pingList.size() >= global->config->pingKickFrame)
				con.pingList.pop_back();

			con.pingList.push_front(connectionInfo->dwRoundTripLatencyMS);
		}
	}

	/** @ingroup Condata
	 * @brief Update average loss data.
	 */
	void TimerUpdateLossData()
	{
		// for all players
		float lossPercentage;
		PlayerData* playerData = nullptr;
		while ((playerData = Players.traverse_active(playerData)))
		{
			ClientId client = playerData->iOnlineId;
			if (client < 1 || client > MaxClientId)
				continue;

			if (ClientInfo[client].tmF1TimeDisconnect)
				continue;

			const auto connectionInfo = Hk::Admin::GetConnectionStats(client);
			if (connectionInfo.has_error())
				continue;
			const auto& connInfo = connectionInfo.value();

			auto& con = global->connections[client];

			///////////////////////////////////////////////////////////////
			// update loss data
			if (con.lossList.size() >= (global->config->lossKickFrame / LossInterval))
			{
				// calculate average loss
				con.averageLoss = 0;
				for (const auto& loss : con.lossList)
					con.averageLoss += loss;

				con.averageLoss /= con.lossList.size();
			}

			// remove old lossdata
			while (con.lossList.size() >= (global->config->lossKickFrame / LossInterval))
				con.lossList.pop_back();

			// sum of Drops = Drops guaranteed + drops non-guaranteed
			const uint newDrops = (connInfo.dwPacketsRetried + connInfo.dwPacketsDropped) - con.lastPacketsDropped;

			// % of Packets Lost = Drops / (sent+received) * 100
			if (const uint newSent = (connInfo.dwPacketsSentGuaranteed + connInfo.dwPacketsSentNonGuaranteed) - con.lastPacketsSent;
			    newSent > 0) // division by zero check
				lossPercentage = static_cast<float>(newDrops) / static_cast<float>(newSent) * 100.0f;
			else
				lossPercentage = 0.0;

			if (lossPercentage > 100)
				lossPercentage = 100;

			// add last loss to List lossList and put current value into lastLoss
			con.lossList.push_front(con.lastLoss);
			con.lastLoss = static_cast<uint>(lossPercentage);

			// Fill new ClientInfo-variables with current values
			con.lastPacketsSent = connInfo.dwPacketsSentGuaranteed + connInfo.dwPacketsSentNonGuaranteed;
			con.lastPacketsDropped = connInfo.dwPacketsRetried + connInfo.dwPacketsDropped;
		}
	}

	/** @ingroup Condata
	 * @brief Hook on PlayerLaunch. Sets lastObjUpdate to 0.
	 */
	void PlayerLaunch([[maybe_unused]] ShipId& ship, ClientId& client)
	{
		global->connections[client].lastObjUpdate = 0;
	}

	/** @ingroup Condata
	 * @brief Hook on SPObjUpdate. Updates timestamps for lag detection.
	 */
	void SPObjUpdate(struct SSPObjUpdateInfo const& ui, ClientId& client)
	{
		// lag detection
		if (const auto ins = Hk::Client::GetInspect(client); ins.has_error())
			return; // ??? 8[

		const mstime timeNow = Hk::Time::GetUnixMiliseconds();
		const auto timestamp = static_cast<mstime>(ui.fTimestamp * 1000);

		auto& con = global->connections[client];

		if (global->config->lagDetectionFrame && con.lastObjUpdate && (Hk::Client::GetEngineState(client) != ES_TRADELANE) && (ui.cState != 7))
		{
			const auto timeDiff = static_cast<uint>(timeNow - con.lastObjUpdate);
			const auto timestampDiff = static_cast<uint>(timestamp - con.lastObjTimestamp);
			auto diff = static_cast<int>(sqrt(pow(static_cast<long double>(static_cast<int>(timeDiff) - static_cast<int>(timestampDiff)), 2)));
			diff -= CoreGlobals::c()->serverLoadInMs;
			if (diff < 0)
				diff = 0;

			uint perc;
			if (timestampDiff != 0)
				perc = static_cast<uint>(static_cast<float>(diff) / static_cast<float>(timestampDiff) * 100.0f);
			else
				perc = 0;

			if (con.objUpdateIntervalsList.size() >= global->config->lagDetectionFrame)
			{
				uint lags = 0;
				for (const auto& iv : con.objUpdateIntervalsList)
				{
					if (iv > global->config->lagDetectionMin)
						lags++;
				}

				con.lags = (lags * 100) / global->config->lagDetectionFrame;
				while (con.objUpdateIntervalsList.size() >= global->config->lagDetectionFrame)
					con.objUpdateIntervalsList.pop_front();
			}

			con.objUpdateIntervalsList.push_back(perc);
		}

		con.lastObjUpdate = timeNow;
		con.lastObjTimestamp = timestamp;
	}

	/** @ingroup Condata
	 * @brief Gets called when the player types /ping
	 */
	void UserCmdPing(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		if (!global->config->allowPing)
		{
			PRINT_DISABLED();
			return;
		}

		uint clientTarget = client;

		std::wstring response = L"Ping";

		// If they have a target selected, and that target is a player, get their target's ping instead
		if (auto target = Hk::Player::GetTarget(client); target.has_value())
		{
			const auto id = Hk::Client::GetClientIdByShip(target.value());
			if (id.has_value() && Hk::Client::IsValidClientID(id.value()))
			{
				clientTarget = id.value();
				response += L" (target)";
			}
		}

		const auto& con = global->connections[clientTarget];

		response += L": ";
		if (con.pingList.size() < global->config->pingKickFrame)
			response += L"n/a Fluct: n/a ";
		else
		{
			response += std::to_wstring(con.averagePing);
			response += L"ms ";
			if (global->config->pingKick > 0)
			{
				response += L"(Max: ";
				response += std::to_wstring(global->config->pingKick);
				response += L"ms) ";
			}
			response += L"Fluct: ";
			response += std::to_wstring(con.pingFluctuation);
			response += L"ms ";
			if (global->config->fluctKick > 0)
			{
				response += L"(Max: ";
				response += std::to_wstring(global->config->fluctKick);
				response += L"ms) ";
			}
		}

		response += L"Loss: ";
		if (con.lossList.size() < (global->config->lossKickFrame / LossInterval))
			response += L"n/a ";
		else
		{
			response += std::to_wstring(con.averageLoss);
			response += L"% ";
			if (global->config->lossKick > 0)
			{
				response += L"(Max: ";
				response += std::to_wstring(global->config->lossKick);
				response += L"%) ";
			}
		}

		response += L"Lag: ";
		if (con.objUpdateIntervalsList.size() < global->config->lagDetectionFrame)
			response += L"n/a";
		else
		{
			response += std::to_wstring(con.lags).c_str();
			response += L"% ";
			if (global->config->lagKick > 0)
			{
				response += L"(Max: ";
				response += std::to_wstring(global->config->lagKick);
				response += L"%)";
			}
		}

		// Send the message to the user
		PrintUserCmdText(client, response);
	}

	const std::vector commands = {{
	    CreateUserCommand(L"/ping", L"", UserCmdPing, L" Gets the ping of your current target, if you have no player as a target it will return your ping."),
	}};

	/** @ingroup Condata
	 * @brief Receive Exception from inter-plugin communication.
	 */
	void ReceiveExceptionData(const ConnectionDataException& exc)
	{
		global->connections[exc.client].exception = exc.isException;
		global->connections[exc.client].exceptionReason = exc.reason;
		if (!global->connections[exc.client].exception)
			ClearConData(exc.client);
	}

	/** @ingroup Condata
	 * @brief Receive Connection data from inter-plugin communication.
	 */
	void ReceiveConnectionData(ConnectionData& cd)
	{
		cd.averageLoss = global->connections[cd.client].averageLoss;
		cd.averagePing = global->connections[cd.client].averagePing;
		cd.lags = global->connections[cd.client].lags;
		cd.pingFluctuation = global->connections[cd.client].pingFluctuation;
	}

	/** @ingroup Condata
	 * @brief Process admin commands.
	 */
	bool ExecuteCommandString(CCmds* classptr, const std::wstring& command)
	{
		if (command == L"getstats")
		{
			struct PlayerData* playerData = nullptr;
			while ((playerData = Players.traverse_active(playerData)))
			{
				ClientId client = playerData->iOnlineId;
				if (Hk::Client::IsInCharSelectMenu(client))
					continue;

				CDPClientProxy* cdpClient = clientProxyArray[client - 1];
				if (!cdpClient)
					continue;

				auto& con = global->connections[client];

				auto saturation = static_cast<int>(cdpClient->GetLinkSaturation() * 100);
				int txqueue = cdpClient->GetSendQSize();
				classptr->Print(wstos(std::format(L"charname={} clientid={} loss={} lag={} pingfluct={} saturation={} txqueue={}\n",
				    Hk::Client::GetCharacterNameByID(client).value(),
				    client,
				    con.averageLoss,
				    con.lags,
				    con.pingFluctuation,
				    saturation,
				    txqueue)));
			}
			classptr->Print("OK");
			global->returncode = ReturnCode::SkipAll;
			return true;
		}
		else if (command == L"kick")
		{
			// Find by charname. If this fails, fall through to default behaviour.
			const auto acc = Hk::Client::GetAccountByCharName(classptr->ArgCharname(1));
			if (acc.has_error())
				return false;

			// Logout.
			global->returncode = ReturnCode::SkipAll;
			acc.value()->ForceLogout();
			classptr->Print("OK");

			// If the client is still active then force the disconnect.
			if (const auto client = Hk::Client::GetClientIdFromAccount(acc.value()); client.has_value())
			{
				classptr->Print(std::format("Forcing logout on client={}", client.value()));
				Players.logout(client.value());
			}
			return true;
		}

		return false;
	}

	/** @ingroup Condata
	 * @brief Exposes functions for inter-plugin communication.
	 */
	ConDataCommunicator::ConDataCommunicator(const std::string& plug) : PluginCommunicator(plug)
	{
		this->ReceiveData = ReceiveConnectionData;
		this->ReceiveException = ReceiveExceptionData;
	}

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

		// check for logged in players and reset their connection data
		struct PlayerData* playerData = nullptr;
		while ((playerData = Players.traverse_active(playerData)))
		{
			if (ClientId client = playerData->iOnlineId; client < 1 || client > MaxClientId)
				continue;

			ClearConData(playerData->iOnlineId);
		}
	}

	const std::vector<Timer> timers = {
	    {TimerUpdatePingData, 1, 0},
	    {TimerUpdateLossData, LossInterval, 0},
	};

} // namespace Plugins::ConData

using namespace Plugins::ConData;

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name(ConDataCommunicator::pluginName);
	pi->shortName("condata");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->timers(&timers);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &TimerCheckKick);
	pi->emplaceHook(HookedCall::IServerImpl__SPObjUpdate, &SPObjUpdate);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);

	// Register plugin for IPC
	global->communicator = new ConDataCommunicator(ConDataCommunicator::pluginName);
	PluginCommunicator::ExportPluginCommunicator(global->communicator);
}