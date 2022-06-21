// includes
#include "header.h"

#define PRINT_ERROR()                                                        \
	{                                                                        \
		for (uint i = 0; (i < sizeof(wscError) / sizeof(std::wstring)); i++) \
			PrintUserCmdText(iClientID, wscError[i]);                        \
		return;                                                              \
	}
#define PRINT_OK()       PrintUserCmdText(iClientID, L"OK");
#define PRINT_DISABLED() PrintUserCmdText(iClientID, L"Command disabled");

/** @defgroup MiscCommands Misc Commands (plugin) */

namespace Plugins::ConData
{
	constexpr int KickTimer = 10;
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ClearConData(uint iClientID)
	{
		auto con = global->connections[iClientID];
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

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ClearClientInfo(uint& iClientID) { ClearConData(iClientID); }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UserCmdHelp(uint& iClientID, const std::wstring& wscParam)
	{
		if (global->config->allowPing)
		{
			PrintUserCmdText(iClientID, L"/ping");
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void HkTimerCheckKick()
	{
		if (g_iServerLoad > global->config->kickThreshold)
		{
			// for all players
			struct PlayerData* pPD = nullptr;
			while (pPD = Players.traverse_active(pPD))
			{
				const uint iClientID = HkGetClientIdFromPD(pPD);
				if (iClientID < 1 || iClientID > MaxClientId)
					continue;

				auto con = global->connections[iClientID];

				if (global->config->lossKick && con.iAverageLoss > global->config->lossKick)
				{
					con.lstLoss.clear();
					HkAddKickLog(iClientID, L"High loss");
					HkMsgAndKick(iClientID, L"High loss", KickTimer);
					// call tempban plugin
					if (global->tempBanCommunicator)
					{
						std::wstring wscCharname = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(iClientID));
						global->tempBanCommunicator->TempBan(wscCharname, 60);
					}
				}

				if (global->config->pingKick)
				{ // check if ping is too high
					if (con.iAveragePing > (global->config->pingKick))
					{
						con.lstPing.clear();
						HkAddKickLog(iClientID, L"High ping");
						HkMsgAndKick(iClientID, L"High ping", KickTimer);
						// call tempban plugin
						if (global->tempBanCommunicator)
						{
							std::wstring wscCharname = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(iClientID));
							global->tempBanCommunicator->TempBan(wscCharname, 60);
						}
					}
				}

				if (global->config->fluctKick)
				{ // check if ping fluct is too high
					if (con.iPingFluctuation > (global->config->fluctKick))
					{
						con.lstPing.clear();
						HkAddKickLog(iClientID, L"High fluct");
						HkMsgAndKick(iClientID, L"High ping fluctuation", KickTimer);
						// call tempban plugin
						if (global->tempBanCommunicator)
						{
							std::wstring wscCharname = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(iClientID));
							global->tempBanCommunicator->TempBan(wscCharname, 60);
						}
					}
				}

				if (global->config->lagKick)
				{ // check if lag is too high
					if (con.iLags > (global->config->lagKick))
					{
						con.lstObjUpdateIntervalls.clear();

						HkAddKickLog(iClientID, L"High Lag");
						HkMsgAndKick(iClientID, L"High Lag", KickTimer);
						// call tempban plugin
						if (global->tempBanCommunicator)
						{
							std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
							global->tempBanCommunicator->TempBan(wscCharname, 60);
						}
					}
				}
			}
		}

		// Are there accounts connected with client IDs greater than max player
		// count? If so, kick them as FLServer is buggy and will use high client IDs
		// but will not allow character selection on them.
		for (int iClientID = Players.GetMaxPlayerCount() + 1; iClientID <= MaxClientId; iClientID++)
		{
			if (Players[iClientID].iOnlineID)
			{
				if (CAccount* acc = Players.FindAccountFromClientID(iClientID))
				{
					acc->ForceLogout();
					Players.logout(iClientID);
				}
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/**************************************************************************************************************
	Update average ping data
	**************************************************************************************************************/

	void TimerUpdatePingData()
	{
		// for all players
		struct PlayerData* pPD = nullptr;
		while (pPD = Players.traverse_active(pPD))
		{
			const uint iClientID = HkGetClientIdFromPD(pPD);
			if (iClientID < 1 || iClientID > MaxClientId)
				continue;

			if (ClientInfo[iClientID].tmF1TimeDisconnect)
				continue;

			DPN_CONNECTION_INFO ci;
			if (HkGetConnectionStats(iClientID, ci) != HKE_OK)
				continue;

			auto con = global->connections[iClientID];

			///////////////////////////////////////////////////////////////
			// update ping data
			if (con.lstPing.size() >= global->config->pingKickFrame)
			{
				// calculate average ping and ping fluctuation
				unsigned int iLastPing = 0;
				con.iAveragePing = 0;
				con.iPingFluctuation = 0;
				for (const auto& ping : con.lstPing)
				{
					con.iAveragePing += ping;
					if (iLastPing != 0)
					{
						con.iPingFluctuation += static_cast<uint>(sqrt(pow(static_cast<float>(ping) - static_cast<float>(iLastPing), 2)));
					}
					iLastPing = ping;
				}

				con.iPingFluctuation /= con.lstPing.size();
				con.iAveragePing /= con.lstPing.size();
			}

			// remove old pingdata
			while (con.lstPing.size() >= global->config->pingKickFrame)
				con.lstPing.pop_back();

			con.lstPing.push_front(ci.dwRoundTripLatencyMS);
		}
	}

	/**************************************************************************************************************
	Update average loss data
	**************************************************************************************************************/

	void TimerUpdateLossData()
	{
		// for all players
		float fLossPercentage;
		struct PlayerData* pPD = 0;
		while (pPD = Players.traverse_active(pPD))
		{
			const uint iClientID = HkGetClientIdFromPD(pPD);
			if (iClientID < 1 || iClientID > MaxClientId)
				continue;

			if (ClientInfo[iClientID].tmF1TimeDisconnect)
				continue;

			DPN_CONNECTION_INFO ci;
			if (HkGetConnectionStats(iClientID, ci) != HKE_OK)
				continue;

			auto con = global->connections[iClientID];

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
			const uint iNewDrops = (ci.dwPacketsRetried + ci.dwPacketsDropped) - con.iLastPacketsDropped;

			// % of Packets Lost = Drops / (sent+received) * 100
			if (const uint iNewSent = (ci.dwPacketsSentGuaranteed + ci.dwPacketsSentNonGuaranteed) - con.iLastPacketsSent; iNewSent > 0) // division by zero check
				fLossPercentage = static_cast<float>(iNewDrops) / static_cast<float>(iNewSent) * 100.0f;
			else
				fLossPercentage = 0.0;

			if (fLossPercentage > 100)
				fLossPercentage = 100;

			// add last loss to List lstLoss and put current value into iLastLoss
			con.lstLoss.push_front(con.iLastLoss);
			con.iLastLoss = static_cast<uint>(fLossPercentage);

			// Fill new ClientInfo-variables with current values
			con.iLastPacketsSent = ci.dwPacketsSentGuaranteed + ci.dwPacketsSentNonGuaranteed;
			con.iLastPacketsDropped = ci.dwPacketsRetried + ci.dwPacketsDropped;
		}
	}

	int __stdcall Update()
	{
		static bool bFirstTime = true;
		if (bFirstTime)
		{
			bFirstTime = false;
			// check for logged in players and reset their connection data
			struct PlayerData* pPD = 0;
			while (pPD = Players.traverse_active(pPD))
			{
				const uint iClientID = pPD->iOnlineID;
				if (iClientID < 1 || iClientID > MaxClientId)
					continue;

				ClearConData(HkGetClientIdFromPD(pPD));
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

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void __stdcall PlayerLaunch(uint& iShip, uint& iClientID) { global->connections[iClientID].tmLastObjUpdate = 0; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void __stdcall SPObjUpdate(struct SSPObjUpdateInfo const& ui, uint& iClientID)
	{
		// lag detection
		if (const IObjInspectImpl* ins = HkGetInspect(iClientID); !ins)
			return; // ??? 8[

		const mstime tmNow = timeInMS();
		const auto tmTimestamp = static_cast<mstime>(ui.fTimestamp * 1000);

		auto con = global->connections[iClientID];

		if (global->config->lagDetectionFrame && con.tmLastObjUpdate && (HkGetEngineState(iClientID) != ES_TRADELANE) && (ui.cState != 7))
		{
			const auto iTimeDiff = static_cast<uint>(tmNow - con.tmLastObjUpdate);
			const auto iTimestampDiff = static_cast<uint>(tmTimestamp - con.tmLastObjTimestamp);
			auto iDiff = static_cast<int>(sqrt(pow(static_cast<long double>(static_cast<int>(iTimeDiff) - static_cast<int>(iTimestampDiff)), 2)));
			iDiff -= g_iServerLoad;
			if (iDiff < 0)
				iDiff = 0;

			uint iPerc;
			if (iTimestampDiff != 0)
				iPerc = static_cast<uint>(static_cast<float>(iDiff) / static_cast<float>(iTimestampDiff) * 100.0f);
			else
				iPerc = 0;

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

			con.lstObjUpdateIntervalls.push_back(iPerc);
		}

		con.tmLastObjUpdate = tmNow;
		con.tmLastObjTimestamp = tmTimestamp;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UserCmdPing(uint iClientID, const std::wstring& wscParam)
	{
		if (!global->config->allowPing)
		{
			PRINT_DISABLED();
			return;
		}

		std::wstring wscTargetPlayer = GetParam(wscParam, ' ', 0);
		uint iClientIDTarget = iClientID;

		// If they have a target selected, and that target is a player, get their target's ping instead
		uint iShip = 0;
		pub::Player::GetShip(iClientID, iShip);
		if (iShip)
		{
			uint iTarget = 0;
			pub::SpaceObj::GetTarget(iShip, iTarget);

			if (iTarget)
			{
				uint id = HkGetClientIDByShip(iTarget);
				if (HkIsValidClientID(iClientIDTarget))
					iClientIDTarget = id;
			}
		}

		auto con = global->connections[iClientIDTarget];

		std::wstring Response;

		Response += L"Ping: ";
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
		PrintUserCmdText(iClientID, Response);
	}
	
	const std::array<USERCMD, 1> UserCmds = {{
	    {L"/ping", UserCmdPing},
	}};

	void ReceiveException(ConnectionDataException exc)
	{
		global->connections[exc.iClientID].bException = exc.bException;
		global->connections[exc.iClientID].sExceptionReason = exc.sReason;
		if (!global->connections[exc.iClientID].bException)
			ClearConData(exc.iClientID);
	}

	void ReceiveConnectionData(ConnectionData cd)
	{
		cd.iAverageLoss = global->connections[cd.iClientID].iAverageLoss;
		cd.iAveragePing = global->connections[cd.iClientID].iAveragePing;
		cd.iLags = global->connections[cd.iClientID].iLags;
		cd.iPingFluctuation = global->connections[cd.iClientID].iPingFluctuation;
	}

	bool ExecuteCommandString(CCmds* classptr, const std::wstring& wscCmd)
	{
		if (wscCmd == L"getstats")
		{
			struct PlayerData* pPD = 0;
			while (pPD = Players.traverse_active(pPD))
			{
				uint iClientID = HkGetClientIdFromPD(pPD);
				if (HkIsInCharSelectMenu(iClientID))
					continue;

				CDPClientProxy* cdpClient = g_cClientProxyArray[iClientID - 1];
				if (!cdpClient)
					continue;

				auto con = global->connections[iClientID];

				int saturation = static_cast<int>(cdpClient->GetLinkSaturation() * 100);
				int txqueue = cdpClient->GetSendQSize();
				classptr->Print(L"charname=%s clientid=%u loss=%u lag=%u pingfluct=%u "
				                L"saturation=%u txqueue=%u\n",
				    Players.GetActiveCharacterName(iClientID), iClientID, con.iAverageLoss, con.iLags, con.iPingFluctuation, saturation, txqueue);
			}
			classptr->Print(L"OK");
			global->returncode = ReturnCode::SkipAll;
			return true;
		}
		else if (wscCmd == L"kick")
		{
			// Find by charname. If this fails, fall through to default behaviour.
			CAccount* acc = HkGetAccountByCharname(classptr->ArgCharname(1));
			if (!acc)
				return false;

			// Logout.
			global->returncode = ReturnCode::SkipAll;
			acc->ForceLogout();
			classptr->Print(L"OK");

			// If the client is still active then force the disconnect.
			const uint iClientID = HkGetClientIdFromAccount(acc);
			if (iClientID != -1)
			{
				classptr->Print(L"Forcing logout on iClientID=%d", iClientID);
				Players.logout(iClientID);
			}
			return true;
		}

		return false;
	}

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
	}
} // namespace Plugins::ConData

using namespace Plugins::ConData;

bool ProcessUserCmds(uint& clientId, const std::wstring& param)
{
	return DefaultUserCommandHandling(clientId, param, UserCmds, global->returncode);
}

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name(ConDataCommunicator::pluginName);
	pi->shortName("condata");
	pi->mayPause(false);
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &HkTimerCheckKick);
	pi->emplaceHook(HookedCall::IServerImpl__Update, &Update);
	pi->emplaceHook(HookedCall::IServerImpl__SPObjUpdate, &SPObjUpdate);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &ProcessUserCmds);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmdHelp);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);

	// Register plugin for IPC
	global->communicator = new ConDataCommunicator(ConDataCommunicator::pluginName);
	PluginCommunicator::ExportPluginCommunicator(global->communicator);
	global->tempBanCommunicator = static_cast<Plugins::Tempban::TempBanCommunicator*>(PluginCommunicator::ImportPluginCommunicator(Plugins::Tempban::TempBanCommunicator::pluginName));
}