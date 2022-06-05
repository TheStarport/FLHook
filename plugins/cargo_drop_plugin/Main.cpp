// Cargo Drop Plugin by Cannon
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

namespace Plugins::CargoDrop
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->CargoDropContainerId = CreateID(config.CargoDropContainer.c_str());
		global->HullDrop1NickNameId = CreateID(config.HullDrop1NickName.c_str());
		global->HullDrop2NickNameId = CreateID(config.HullDrop2NickName.c_str());

		for (const auto& no_loot_item : config.NoLootItems)
			global->NoLootItemsIds.push_back(CreateID(no_loot_item.c_str()));

		global->config = std::make_unique<Config>(config);
	}

	void Timer()
	{
		// Disconnecting while interacting checks.
		for (auto& iter : global->mapInfo)
		{
			int iClientID = iter.first;

			// If selecting a character or invalid, do nothing.
			if (!HkIsValidClientID(iClientID) || HkIsInCharSelectMenu(iClientID))
				continue;

			// If not in space, do nothing
			uint iShip;
			pub::Player::GetShip(iClientID, iShip);
			if (!iShip)
				continue;

			if (ClientInfo[iClientID].tmF1Time || ClientInfo[iClientID].tmF1TimeDisconnect)
			{
				std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

				// Drain the ship's shields.
				pub::SpaceObj::DrainShields(iShip);

				// Simulate an obj update to stop the ship in space.
				SSPObjUpdateInfo ui;
				iter.second.dLastTimestamp += 1.0;
				ui.fTimestamp = (float)iter.second.dLastTimestamp;
				ui.cState = 0;
				ui.fThrottle = 0;
				ui.vPos = iter.second.vLastPosition;
				ui.vDir = iter.second.vLastDir;
				Server.SPObjUpdate(ui, iClientID);

				if (!iter.second.bF1DisconnectProcessed)
				{
					iter.second.bF1DisconnectProcessed = true;

					// Send disconnect report to all ships in scanner range.
					if (global->config->ReportDisconnectingPlayers)
					{
						std::wstring wscMsg = stows(global->config->DisconnectMsg);
						wscMsg = ReplaceStr(wscMsg, L"%time", GetTimeString(FLHookConfig::i()->general.localTime));
						wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
						PrintLocalUserCmdText(iClientID, wscMsg, global->config->DisconnectingPlayersRange);
					}

					// Drop the player's cargo.
					if (global->config->LootDisconnectingPlayers && IsInRange(iClientID, global->config->DisconnectingPlayersRange))
					{
						uint iSystem = 0;
						pub::Player::GetSystem(iClientID, iSystem);
						uint iShip = 0;
						pub::Player::GetShip(iClientID, iShip);
						Vector vLoc = {0.0f, 0.0f, 0.0f};
						Matrix mRot = {0.0f, 0.0f, 0.0f};
						pub::SpaceObj::GetLocation(iShip, vLoc, mRot);
						vLoc.x += 30.0;

						std::list<CARGO_INFO> lstCargo;
						int iRemainingHoldSize = 0;
						if (HkEnumCargo(wscCharname, lstCargo, iRemainingHoldSize) == HKE_OK)
						{
							for (auto& item : lstCargo)
							{
								if (!item.bMounted &&
								    std::find(global->NoLootItemsIds.begin(), global->NoLootItemsIds.end(), item.iArchID) != global->NoLootItemsIds.end())
								{
									HkRemoveCargo(wscCharname, item.iID, item.iCount);
									Server.MineAsteroid(iSystem, vLoc, global->CargoDropContainerId, item.iArchID, item.iCount, iClientID);
								}
							}
						}
						HkSaveChar(wscCharname);
					}

					// Kill if other ships are in scanner range.
					if (global->config->KillDisconnectingPlayers && IsInRange(iClientID, global->config->DisconnectingPlayersRange))
					{
						uint iShip = 0;
						pub::Player::GetShip(iClientID, iShip);
						pub::SpaceObj::SetRelativeHealth(iShip, 0.0f);
					}
				}
			}
		}
	}

	/// Hook for ship distruction. It's easier to hook this than the PlayerDeath
	/// one. Drop a percentage of cargo + some loot representing ship bits.
	void SendDeathMsg(const std::wstring& wscMsg, uint& iSystem, uint& iClientIDVictim, uint& iClientIDKiller)
	{
		// If player ship loot dropping is enabled then check for a loot drop.
		if (global->config->HullDropFactor == 0.0f)
			return;

		std::list<CARGO_INFO> lstCargo;
		int iRemainingHoldSize;
		if (HkEnumCargo((const wchar_t*)Players.GetActiveCharacterName(iClientIDVictim), lstCargo, iRemainingHoldSize) != HKE_OK)
			return;

		int iShipSizeEst = iRemainingHoldSize;
		for (auto& item : lstCargo)
		{
			if (!(item.bMounted))
			{
				iShipSizeEst += item.iCount;
			}
		}

		int iHullDrop = (int)(global->config->HullDropFactor * (float)iShipSizeEst);
		if (iHullDrop > 0)
		{
			uint iShip;
			pub::Player::GetShip(iClientIDVictim, iShip);
			Vector myLocation;
			Matrix myRot;
			pub::SpaceObj::GetLocation(iShip, myLocation, myRot);
			myLocation.x += 30.0;

			if (FLHookConfig::i()->general.debugMode)
				Console::ConInfo(L"NOTICE: player control cargo drop in system %08x at %f,%f,%f "
				                 L"for ship size of iShipSizeEst=%d iHullDrop=%d\n",
				    iSystem, myLocation.x, myLocation.y, myLocation.z, iShipSizeEst, iHullDrop);

			Server.MineAsteroid(iSystem, myLocation, global->CargoDropContainerId, global->HullDrop1NickNameId, iHullDrop, iClientIDVictim);
			Server.MineAsteroid(iSystem, myLocation, global->CargoDropContainerId, global->HullDrop2NickNameId, (int)(0.5 * (float)iHullDrop), iClientIDVictim);
		}
	}

	void ClearClientInfo(uint& iClientID) { global->mapInfo.erase(iClientID); }

	void SPObjUpdate(struct SSPObjUpdateInfo const& ui, uint& iClientID)
	{
		global->mapInfo[iClientID].dLastTimestamp = ui.fTimestamp;
		global->mapInfo[iClientID].vLastPosition = ui.vPos;
		global->mapInfo[iClientID].vLastDir = ui.vDir;
	}
} // namespace Plugins::CargoDrop

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::CargoDrop;
REFL_AUTO(type(Config), field(ReportDisconnectingPlayers), field(KillDisconnectingPlayers), field(LootDisconnectingPlayers), field(DisconnectingPlayersRange),
    field(HullDropFactor), field(DisconnectMsg), field(CargoDropContainer), field(HullDrop1NickName), field(HullDrop2NickName), field(NoLootItems))

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH)
		LoadSettings();
	return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Cargo Drop Plugin by Cannon");
	pi->shortName("cargo_drop");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &Timer);
	pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMsg);
	pi->emplaceHook(HookedCall::IServerImpl__SPObjUpdate, &SPObjUpdate);
}