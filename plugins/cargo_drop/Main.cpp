/**
 * @date Feb 2010
 * @author Cannon (Ported by Raikkonen 2022)
 * @defgroup CargoDrop Cargo Drop
 * @brief
 * The "Cargo Drop" plugin handles consequences to a player who disconnects whilst in space.
 *
 * @paragraph cmds Player Commands
 * There are no player commands in this plugin.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *   "cargoDropContainer": "lootcrate_ast_loot_metal",
 *   "disconnectMsg": "%player is attempting to engage cloaking device",
 *   "disconnectingPlayersRange": 5000.0,
 *   "hullDrop1NickName": "commodity_super_alloys",
 *   "hullDrop2NickName": "commodity_engine_components",
 *   "hullDropFactor": 0.1,
 *   "killDisconnectingPlayers": true,
 *   "lootDisconnectingPlayers": true,
 *   "noLootItems": [],
 *   "reportDisconnectingPlayers": true
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * None.
 */

// Includes
#include "Main.h"

namespace Plugins::CargoDrop
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->cargoDropContainerId = CreateID(config.cargoDropContainer.c_str());
		global->hullDrop1NickNameId = CreateID(config.hullDrop1NickName.c_str());
		global->hullDrop2NickNameId = CreateID(config.hullDrop2NickName.c_str());

		for (const auto& noLootItem : config.noLootItems)
			global->noLootItemsIds.push_back(CreateID(noLootItem.c_str()));

		global->config = std::make_unique<Config>(config);
	}

	/** @ingroup CargoDrop
	 * @brief Timer that checks if a player has disconnected and punished them if so.
	 */
	void OneSecondTimer()
	{
		// Disconnecting while interacting checks.
		for (auto& [clientId, snd] : global->info)
		{
			// If selecting a character or invalid, do nothing.
			if (!IsValidClientID(clientId) || IsInCharSelectMenu(clientId))
				continue;

			// If not in space, do nothing
			uint shipId;
			pub::Player::GetShip(clientId, shipId);
			if (!shipId)
				continue;

			if (ClientInfo[clientId].tmF1Time || ClientInfo[clientId].tmF1TimeDisconnect)
			{
				std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));

				// Drain the ship's shields.
				pub::SpaceObj::DrainShields(shipId);

				// Simulate an obj update to stop the ship in space.
				SSPObjUpdateInfo updateInfo{};
				snd.lastTimestamp += 1.0;
				updateInfo.fTimestamp = static_cast<float>(snd.lastTimestamp);
				updateInfo.cState = 0;
				updateInfo.fThrottle = 0;
				updateInfo.vPos = snd.lastPosition;
				updateInfo.vDir = snd.lastDirection;
				Server.SPObjUpdate(updateInfo, clientId);

				if (!snd.f1DisconnectProcessed)
				{
					snd.f1DisconnectProcessed = true;

					// Send disconnect report to all ships in scanner range.
					if (global->config->reportDisconnectingPlayers)
					{
						std::wstring msg = stows(global->config->disconnectMsg);
						msg = ReplaceStr(msg, L"%time", GetTimeString(FLHookConfig::i()->general.localTime));
						msg = ReplaceStr(msg, L"%player", characterName);
						PrintLocalUserCmdText(clientId, msg, global->config->disconnectingPlayersRange);
					}

					// Drop the player's cargo.
					if (global->config->lootDisconnectingPlayers && IsInRange(clientId, global->config->disconnectingPlayersRange))
					{
						uint systemID = 0;
						pub::Player::GetSystem(clientId, systemID);
						Vector vLoc = {0.0f, 0.0f, 0.0f};
						Matrix mRot = {0.0f, 0.0f, 0.0f};
						pub::SpaceObj::GetLocation(shipId, vLoc, mRot);
						vLoc.x += 30.0f;

						std::list<CARGO_INFO> cargo;
						if (int iRemainingHoldSize = 0; EnumCargo(characterName, cargo, iRemainingHoldSize) == E_OK)
						{
							for (const auto& [iID, iCount, iArchID, fStatus, bMission, bMounted, hardpoint] : cargo)
							{
								if (!bMounted &&
								    std::find(global->noLootItemsIds.begin(), global->noLootItemsIds.end(), iArchID) != global->noLootItemsIds.end())
								{
									RemoveCargo(characterName, iID, iCount);
									Server.MineAsteroid(systemID, vLoc, global->cargoDropContainerId, iArchID, iCount, clientId);
								}
							}
						}
						SaveChar(characterName);
					}

					// Kill if other ships are in scanner range.
					if (global->config->killDisconnectingPlayers && IsInRange(clientId, global->config->disconnectingPlayersRange))
					{
						pub::SpaceObj::SetRelativeHealth(shipId, 0.0f);
					}
				}
			}
		}
	}

	/** @ingroup CargoDrop
	 * @brief Hook for ship destruction. It's easier to hook this than the PlayerDeath one. Drop a percentage of cargo + some loot representing ship bits.
	 */
	void SendDeathMsg(const std::wstring& message, uint& system, uint& clientIDVictim, uint& clientIDKiller)
	{
		// If player ship loot dropping is enabled then check for a loot drop.
		if (global->config->hullDropFactor == 0.0f)
			return;

		std::list<CARGO_INFO> cargo;
		int iRemainingHoldSize;
		if (EnumCargo(reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientIDVictim)), cargo, iRemainingHoldSize) != E_OK)
			return;

		int shipSizeEstimate = iRemainingHoldSize;
		for (const auto& [iID, iCount, iArchID, fStatus, bMission, bMounted, hardpoint] : cargo)
		{
			if (!bMounted)
			{
				shipSizeEstimate += iCount;
			}
		}

		if (const auto iHullDrop = static_cast<int>(global->config->hullDropFactor * shipSizeEstimate); iHullDrop > 0)
		{
			uint shipId;
			pub::Player::GetShip(clientIDVictim, shipId);
			Vector location{};
			Matrix rotation{};
			pub::SpaceObj::GetLocation(shipId, location, rotation);
			location.x += 30.0f;

			if (FLHookConfig::i()->general.debugMode)
				Console::ConInfo(L"NOTICE: Cargo drop in system %08x at %f,%f,%f "
				                 L"for ship size of iShipSizeEst=%d iHullDrop=%d\n",
				    system, location.x, location.y, location.z, shipSizeEstimate, iHullDrop);

			Server.MineAsteroid(system, location, global->cargoDropContainerId, global->hullDrop1NickNameId, iHullDrop, clientIDVictim);
			Server.MineAsteroid(system, location, global->cargoDropContainerId, global->hullDrop2NickNameId, static_cast
				<int>(0.5 * iHullDrop), clientIDVictim);
		}
	}

	/** @ingroup CargoDrop
	 * @brief Clear our variables so that we can recycle clientIds without confusion.
	 */
	void ClearClientInfo(uint& clientId) { global->info.erase(clientId); }

	/** @ingroup CargoDrop
	 * @brief Hook on SPObjUpdate, used to get the timestamp from the player. Used to figure out if the player has disconnected in the timer.
	 */
	void SPObjUpdate(struct SSPObjUpdateInfo const& ui, uint& clientId)
	{
		global->info[clientId].lastTimestamp = ui.fTimestamp;
		global->info[clientId].lastPosition = ui.vPos;
		global->info[clientId].lastDirection = ui.vDir;
	}
} // namespace Plugins::CargoDrop

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::CargoDrop;
REFL_AUTO(type(Config), field(reportDisconnectingPlayers), field(killDisconnectingPlayers), field(lootDisconnectingPlayers), field(disconnectingPlayersRange),
    field(hullDropFactor), field(disconnectMsg), field(cargoDropContainer), field(hullDrop1NickName), field(hullDrop2NickName), field(noLootItems))

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Cargo Drop");
	pi->shortName("cargo_drop");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &OneSecondTimer);
	pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMsg);
	pi->emplaceHook(HookedCall::IServerImpl__SPObjUpdate, &SPObjUpdate);
}