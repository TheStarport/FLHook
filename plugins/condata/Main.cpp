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

#include "Main.h"

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
		con.iAverageLoss = 0;
		con.iAveragePing = 0;
		con.iLastLoss = 0;
		con.iLastPacketsDropped = 0;
		con.iLastPacketsReceived = 0;
		con.iLastPacketsSent = 0;
		con.iPingFluctuation = 0;
		con.lstLoss.clear();
		con.lstPing.clear();
		con.lstObjUpdateIntervalls.clear();
		con.iLags = 0;
		con.tmLastObjUpdate = 0;
		con.tmLastObjTimestamp = 0;

		con.bException = false;
		con.sExceptionReason = "";
	}

	/** @ingroup Condata
	 * @brief ClearClientInfo hook. Calls ClearConData().
	 */
	void ClearClientInfo(ClientId& client) { ClearConData(client); }

	/** @ingroup Condata
	 * @brief Hook on TimerCheckKick. Checks clients's connections against a threshold and kicks them if they are above it.
	 */
	void TimerCheckKick()
	{
		if (g_iServerLoad > global->config->kickThreshold)
		{
			// for all players
			struct PlayerData* playerData = nullptr;
			while (playerData = Players.traverse_active(playerData))
			{
				ClientId client = playerData->iOnlineId ;
				if (client < 1 || client > MaxClientId)
					continue;

				auto con = global->connections[client];

				if (global->config->lossKick && con.iAverageLoss > global->config->lossKick)
				{
					con.lstLoss.clear();
					AddKickLog(client, "High loss");
					Hk::Player::MsgAndKick(client, L"High loss", KickTimer);
					// call tempban plugin
					if (global->tempBanCommunicator)
					{
						const auto charName = Hk::Client::GetCharacterNameByID(client);
						global->tempBanCommunicator->TempBan(charName.value(), 60);
					}
				}

				if (global->config->pingKick)
				{ // check if ping is too high
					if (con.iAveragePing > (global->config->pingKick))
					{
						con.lstPing.clear();
						AddKickLog(client, "High ping");
						Hk::Player::MsgAndKick(client, L"High ping", KickTimer);
						// call tempban plugin
						if (global->tempBanCommunicator)
						{
							const auto charName = Hk::Client::GetCharacterNameByID(client);
							global->tempBanCommunicator->TempBan(charName.value(), 60);
						}
					}
				}

				if (global->config->fluctKick)
				{ // check if ping fluct is too high
					if (con.iPingFluctuation > (global->config->fluctKick))
					{
						con.lstPing.clear();
						AddKickLog(client, "High fluct");
						Hk::Player::MsgAndKick(client, L"High ping fluctuation", KickTimer);
						// call tempban plugin
						if (global->tempBanCommunicator)
						{
							const auto charName = Hk::Client::GetCharacterNameByID(client);
							global->tempBanCommunicator->TempBan(charName.value(), 60);
						}
					}
				}

				if (global->config->lagKick)
				{ // check if lag is too high
					if (con.iLags > (global->config->lagKick))
					{
						con.lstObjUpdateIntervalls.clear();

						AddKickLog(client, "High Lag");
						Hk::Player::MsgAndKick(client, L"High Lag", KickTimer);
						// call tempban plugin
						if (global->tempBanCommunicator)
						{
							const auto charName = Hk::Client::GetCharacterNameByID(client);
							global->tempBanCommunicator->TempBan(charName.value(), 60);
						}
					}
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
		PlayerData* playerData = nullptr;
		while (playerData = Players.traverse_active(playerData))
		{
			ClientId client = playerData->iOnlineId;
			if (client < 1 || client > MaxClientId)
				continue;

			if (ClientInfo[client].tmF1TimeDisconnect)
				continue;

			const auto connectionInfo = Hk::Admin::GetConnectionStats(client);
			if (connectionInfo.has_error())
				continue;

			auto& con = global->connections[client];

			///////////////////////////////////////////////////////////////
			// update ping data
			if (con.lstPing.size() >= global->config->pingKickFrame)
			{
				// calculate average ping and ping fluctuation
				unsigned int lastPing = 0;
				con.iAveragePing = 0;
				con.iPingFluctuation = 0;
				for (const auto& ping : con.lstPing)
				{
					con.iAveragePing += ping;
					if (lastPing != 0)
					{
						con.iPingFluctuation += static_cast<uint>(sqrt(pow(static_cast<float>(ping) - static_cast<float>(lastPing), 2)));
					}
					lastPing = ping;
				}

				con.iPingFluctuation /= con.lstPing.size();
				con.iAveragePing /= con.lstPing.size();
			}

			// remove old pingdata
			while (con.lstPing.size() >= global->config->pingKickFrame)
				con.lstPing.pop_back();

			con.lstPing.push_front(connectionInfo->dwRoundTripLatencyMS);
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
		while (playerData = Players.traverse_active(playerData))
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
			if (con.lstLoss.size() >= (global->config->lossKickFrame / (LossInterval / 1000)))
			{
				// calculate average loss
				con.iAverageLoss = 0;
				for (const auto& loss : con.lstLoss)
					con.iAverageLoss += loss;

				con.iAverageLoss /= con.lstLoss.size();
			}

			// remove old lossdata
			while (con.lstLoss.size() >= (global->config->lossKickFrame / (LossInterval / 1000)))
				con.lstLoss.pop_back();

			// sum of Drops = Drops guaranteed + drops non-guaranteed
			const uint newDrops = (connInfo.dwPacketsRetried + connInfo.dwPacketsDropped) - con.iLastPacketsDropped;

			// % of Packets Lost = Drops / (sent+received) * 100
			if (const uint newSent = (connInfo.dwPacketsSentGuaranteed + connInfo.dwPacketsSentNonGuaranteed) - con.iLastPacketsSent;
			    newSent > 0) // division by zero check
				lossPercentage = static_cast<float>(newDrops) / static_cast<float>(newSent) * 100.0f;
			else
				lossPercentage = 0.0;

			if (lossPercentage > 100)
				lossPercentage = 100;

			// add last loss to List lstLoss and put current value into iLastLoss
			con.lstLoss.push_front(con.iLastLoss);
			con.iLastLoss = static_cast<uint>(lossPercentage);

			// Fill new ClientInfo-variables with current values
			con.iLastPacketsSent = connInfo.dwPacketsSentGuaranteed + connInfo.dwPacketsSentNonGuaranteed;
			con.iLastPacketsDropped = connInfo.dwPacketsRetried + connInfo.dwPacketsDropped;
		}
	}

	/** @ingroup Condata
	 * @brief If this is the first tick of the plugin being loaded, reset any connection data. Call any timers also.
	 */
	int __stdcall Update()
	{
		static bool firstTime = true;
		if (firstTime)
		{
			firstTime = false;
			// check for logged in players and reset their connection data
			struct PlayerData* playerData = 0;
			while (playerData = Players.traverse_active(playerData))
			{
				ClientId client = playerData->iOnlineId;
				if (client < 1 || client > MaxClientId)
					continue;

				ClearConData(playerData->iOnlineId);
			}
		}

		// call timers
		for (auto& [proc, tmIntervallMS, tmLastCall] : global->timers)
		{
			if ((timeInMS() - tmLastCall) >= tmIntervallMS)
			{
				tmLastCall = timeInMS();
				proc();
			}
		}

		return 0; // it doesnt matter what we return here since we have set the
		          // return code to "DEFAULT_RETURNCODE", so FLHook will just ignore
		          // it
	}

	/** @ingroup Condata
	 * @brief Hook on PlayerLaunch. Sets tmLastObjUpdate to 0.
	 */
	void __stdcall PlayerLaunch(ShipId& ship, ClientId& client) { global->connections[client].tmLastObjUpdate = 0; }

	/** @ingroup Condata
	 * @brief Hook on SPObjUpdate. Updates timestamps for lag detection.
	 */
	void __stdcall SPObjUpdate(struct SSPObjUpdateInfo const& ui, ClientId& client)
	{
		// lag detection
		if (const auto ins = Hk::Client::GetInspect(client); ins.has_error())
			return; // ??? 8[

		const mstime tmNow = timeInMS();
		const auto tmTimestamp = static_cast<mstime>(ui.fTimestamp * 1000);

		auto& con = global->connections[client];

		if (global->config->lagDetectionFrame && con.tmLastObjUpdate && (Hk::Client::GetEngineState(client) != ES_TRADELANE) && (ui.cState != 7))
		{
			const auto iTimeDiff = static_cast<uint>(tmNow - con.tmLastObjUpdate);
			const auto iTimestampDiff = static_cast<uint>(tmTimestamp - con.tmLastObjTimestamp);
			auto iDiff = static_cast<int>(sqrt(pow(static_cast<long double>(static_cast<int>(iTimeDiff) - static_cast<int>(iTimestampDiff)), 2)));
			iDiff -= g_iServerLoad;
			if (iDiff < 0)
				iDiff = 0;

			uint perc;
			if (iTimestampDiff != 0)
				perc = static_cast<uint>(static_cast<float>(iDiff) / static_cast<float>(iTimestampDiff) * 100.0f);
			else
				perc = 0;

			if (con.lstObjUpdateIntervalls.size() >= global->config->lagDetectionFrame)
			{
				uint iLags = 0;
				for (const auto& iv : con.lstObjUpdateIntervalls)
				{
					if (iv > global->config->lagDetectionMin)
						iLags++;
				}

				con.iLags = (iLags * 100) / global->config->lagDetectionFrame;
				while (con.lstObjUpdateIntervalls.size() >= global->config->lagDetectionFrame)
					con.lstObjUpdateIntervalls.pop_front();
			}

			con.lstObjUpdateIntervalls.push_back(perc);
		}

		con.tmLastObjUpdate = tmNow;
		con.tmLastObjTimestamp = tmTimestamp;
	}

	/** @ingroup Condata
	 * @brief Gets called when the player types /ping
	 */
	void UserCmdPing(ClientId& client, const std::wstring& param)
	{
		if (!global->config->allowPing)
		{
			PRINT_DISABLED();
			return;
		}

		uint clientTarget = client;

		// If they have a target selected, and that target is a player, get their target's ping instead
		
		
		auto ship = Hk::Player::GetShip(client);
		auto iTarget = Hk::Player::GetTarget(ship.value());
		if (iTarget.has_value())
		{
			const auto id = Hk::Client::GetClientIdByShip(iTarget.value());
			if (Hk::Client::IsValidClientID(clientTarget))
				clientTarget = id.value();
		}

		auto& con = global->connections[clientTarget];

		std::wstring Response = L"Ping";
		if (iTarget.has_value())
			Response += L" (target)";

		Response += L" :";
		if (con.lstPing.size() < global->config->pingKickFrame)
			Response += L"n/a Fluct: n/a ";
		else
		{
			Response += std::to_wstring(con.iAveragePing);
			Response += L"ms ";
			if (global->config->pingKick > 0)
			{
				Response += L"(Max: ";
				Response += std::to_wstring(global->config->pingKick);
				Response += L"ms) ";
			}
			Response += L"Fluct: ";
			Response += std::to_wstring(con.iPingFluctuation);
			Response += L"ms ";
			if (global->config->fluctKick > 0)
			{
				Response += L"(Max: ";
				Response += std::to_wstring(global->config->fluctKick);
				Response += L"ms) ";
			}
		}

		Response += L"Loss: ";
		if (con.lstLoss.size() < (global->config->lossKickFrame / (LossInterval / 1000)))
			Response += L"n/a ";
		else
		{
			Response += std::to_wstring(con.iAverageLoss);
			Response += L"%% ";
			if (global->config->lossKick > 0)
			{
				Response += L"(Max: ";
				Response += std::to_wstring(global->config->lossKick);
				Response += L"%%) ";
			}
		}

		Response += L"Lag: ";
		if (con.lstObjUpdateIntervalls.size() < global->config->lagDetectionFrame)
			Response += L"n/a";
		else
		{
			Response += std::to_wstring(con.iLags).c_str();
			Response += L"%% ";
			if (global->config->lagKick > 0)
			{
				Response += L"(Max: ";
				Response += std::to_wstring(global->config->lagKick);
				Response += L"%%)";
			}
		}

		// Send the message to the user
		PrintUserCmdText(client, Response);
	}

	const std::vector commands = {{
	    CreateUserCommand(L"/ping", L"", UserCmdPing, L""),
	}};

	/** @ingroup Condata
	 * @brief Receive Exception from inter-plugin communication.
	 */
	void ReceiveException(ConnectionDataException exc)
	{
		global->connections[exc.client].bException = exc.bException;
		global->connections[exc.client].sExceptionReason = exc.sReason;
		if (!global->connections[exc.client].bException)
			ClearConData(exc.client);
	}

	/** @ingroup Condata
	 * @brief Receive Connection data from inter-plugin communication.
	 */
	void ReceiveConnectionData(ConnectionData cd)
	{
		cd.iAverageLoss = global->connections[cd.client].iAverageLoss;
		cd.iAveragePing = global->connections[cd.client].iAveragePing;
		cd.iLags = global->connections[cd.client].iLags;
		cd.iPingFluctuation = global->connections[cd.client].iPingFluctuation;
	}

	/** @ingroup Condata
	 * @brief Process admin commands.
	 */
	bool ExecuteCommandString(CCmds* classptr, const std::wstring& command)
	{
		if (command == L"getstats")
		{
			struct PlayerData* playerData = 0;
			while (playerData = Players.traverse_active(playerData))
			{
				ClientId client = playerData->iOnlineId;
				if (Hk::Client::IsInCharSelectMenu(client))
					continue;

				CDPClientProxy* cdpClient = g_cClientProxyArray[client - 1];
				if (!cdpClient)
					continue;

				auto con = global->connections[client];

				int saturation = static_cast<int>(cdpClient->GetLinkSaturation() * 100);
				int txqueue = cdpClient->GetSendQSize();
				classptr->Print(L"charname=%s clientid=%u loss=%u lag=%u pingfluct=%u "
				                L"saturation=%u txqueue=%u\n",
				    Players.GetActiveCharacterName(client),
				    client,
				    con.iAverageLoss,
				    con.iLags,
				    con.iPingFluctuation,
				    saturation,
				    txqueue);
			}
			classptr->Print(L"OK");
			global->returncode = ReturnCode::SkipAll;
			return true;
		}
		else if (command == L"kick")
		{
			// Find by charname. If this fails, fall through to default behaviour.
			const auto acc = Hk::Client::GetAccountByCharName(classptr->ArgCharname(1));
			if (!acc)
				return false;

			// Logout.
			global->returncode = ReturnCode::SkipAll;
			acc.value()->ForceLogout();
			classptr->Print(L"OK");

			// If the client is still active then force the disconnect.
			const auto client = Hk::Client::GetClientIdFromAccount(acc.value());
			if (client != -1)
			{
				classptr->Print(L"Forcing logout on client=%d", client);
				Players.logout(client.value());
			}
			return true;
		}

		return false;
	}

	/** @ingroup Condata
	 * @brief Exposes functions for inter-plugin communication.
	 */
	ConDataCommunicator::ConDataCommunicator(std::string plug) : PluginCommunicator(plug)
	{
		this->ReceiveData = ReceiveData;
		this->ReceiveException = ReceiveException;
	}

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);
		global->timers = {
		    {TimerUpdatePingData, 1000, 0},
		    {TimerUpdateLossData, LossInterval, 0},
		};

		global->tempBanCommunicator = static_cast<Plugins::Tempban::TempBanCommunicator*>(
		    PluginCommunicator::ImportPluginCommunicator(Plugins::Tempban::TempBanCommunicator::pluginName));
	}
} // namespace Plugins::ConData

using namespace Plugins::ConData;

DefaultDllMainSettings(LoadSettings)

    extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name(ConDataCommunicator::pluginName);
	pi->shortName("condata");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &TimerCheckKick);
	pi->emplaceHook(HookedCall::IServerImpl__Update, &Update);
	pi->emplaceHook(HookedCall::IServerImpl__SPObjUpdate, &SPObjUpdate);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);

	// Register plugin for IPC
	global->communicator = new ConDataCommunicator(ConDataCommunicator::pluginName);
	PluginCommunicator::ExportPluginCommunicator(global->communicator);
}