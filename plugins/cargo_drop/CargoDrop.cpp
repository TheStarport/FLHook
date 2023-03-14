/**
 * @date Feb 2010
 * @author Cannon (Ported by Raikkonen 2022)
 * @defgroup CargoDrop Cargo Drop
 * @brief
 * The "Cargo Drop" plugin handles consequences to a player who disconnects whi in space.
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
#include "CargoDrop.h"

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
	void DisconnectCheck()
	{
		// Disconnecting while interacting checks.
		for (auto& [client, snd] : global->info)
		{
			// If selecting a character or invalid, do nothing.
			if (!Hk::Client::IsValidClientID(client) || Hk::Client::IsInCharSelectMenu(client))
				continue;

			// If not in space, do nothing
			auto ship = Hk::Player::GetShip(client);
			if (ship.has_error())
				continue;

			if (ClientInfo[client].tmF1Time || ClientInfo[client].tmF1TimeDisconnect)
			{
				std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));

				// Drain the ship's shields.
				pub::SpaceObj::DrainShields(ship.value());

				// Simulate an obj update to stop the ship in space.
				SSPObjUpdateInfo updateInfo {};
				snd.lastTimestamp += 1.0;
				updateInfo.timestamp = static_cast<float>(snd.lastTimestamp);
				updateInfo.state = 0;
				updateInfo.throttle = 0;
				updateInfo.pos = snd.lastPosition;
				updateInfo.dir = snd.lastDirection;
				Server.SPObjUpdate(updateInfo, client);

				if (!snd.f1DisconnectProcessed)
				{
					snd.f1DisconnectProcessed = true;

					// Send disconnect report to all ships in scanner range.
					if (global->config->reportDisconnectingPlayers)
					{
						std::wstring msg = stows(global->config->disconnectMsg);
						msg = ReplaceStr(msg, L"%time", GetTimeString(FLHookConfig::i()->general.localTime));
						msg = ReplaceStr(msg, L"%player", characterName);
						PrintLocalUserCmdText(client, msg, global->config->disconnectingPlayersRange);
					}

					// Drop the player's cargo.
					if (global->config->lootDisconnectingPlayers && Hk::Player::IsInRange(client, global->config->disconnectingPlayersRange))
					{
						const auto system = Hk::Player::GetSystem(client);
						auto [position, _] = Hk::Solar::GetLocation(ship.value(), IdType::Ship).value();
						position.x += 30.0f;

						int remainingHoldSize = 0;
						if (const auto cargo = Hk::Player::EnumCargo(client, remainingHoldSize); cargo.has_value())
						{
							for (const auto& [id, count, archId, status, mission, mounted, hardpoint] : cargo.value())
							{
								if (!mounted && std::ranges::find(global->noLootItemsIds, archId) == global->noLootItemsIds.end())
								{
									Hk::Player::RemoveCargo(characterName, static_cast<ushort>(id), count);
									Server.MineAsteroid(system.value(), position, global->cargoDropContainerId, archId, count, client);
								}
							}
						}
						Hk::Player::SaveChar(characterName);
					}

					// Kill if other ships are in scanner range.
					if (global->config->killDisconnectingPlayers && Hk::Player::IsInRange(client, global->config->disconnectingPlayersRange))
					{
						pub::SpaceObj::SetRelativeHealth(ship.value(), 0.0f);
					}
				}
			}
		}
	}

	const std::vector<Timer> timers = {{DisconnectCheck, 1}};

	/** @ingroup CargoDrop
	 * @brief Hook for ship destruction. It's easier to hook this than the PlayerDeath one. Drop a percentage of cargo + some loot representing ship bits.
	 */
	void SendDeathMsg([[maybe_unused]] const std::wstring& message, const SystemId& system, ClientId& clientVictim, ClientId& clientKiller)
	{
		// If player ship loot dropping is enabled then check for a loot drop.
		if (global->config->hullDropFactor == 0.0f)
			return;

		int remainingHoldSize;
		auto cargo = Hk::Player::EnumCargo(clientVictim, remainingHoldSize);
		if (cargo.has_error())
			return;

		const uint ship = Hk::Player::GetShip(clientVictim).value();
		auto [position, _] = Hk::Solar::GetLocation(ship, IdType::Ship).value();
		position.x += 30.0f;

		int shipSizeEstimate = remainingHoldSize;
		if (global->config->enablePlayerCargoDropOnDeath)
		{
			for (const auto& [iId, count, archId, status, mission, mounted, hardpoint] : cargo.value())
			{
				if (!mounted)
				{
					shipSizeEstimate += count;
					if (!mission && std::ranges::find(global->noLootItemsIds, archId) == global->noLootItemsIds.end())
						Server.MineAsteroid(
						    system, position, global->cargoDropContainerId, archId, std::min(count, global->config->maxPlayerCargoDropCount), clientKiller);
				}
			}
		}
		if (const auto hullDrop = static_cast<int>(global->config->hullDropFactor * static_cast<float>(shipSizeEstimate)); hullDrop > 0)
		{
			if (FLHookConfig::i()->general.debugMode)
				Logger::i()->Log(LogLevel::Info, std::format("Cargo drop in system {:#X} at {:.2f}, {:.2f}, {:.2f} for ship size of shipSizeEst={} iHullDrop={}\n",
				    system,
				    position.x,
				    position.y,
				    position.z,
				    shipSizeEstimate,
				    hullDrop));

			Server.MineAsteroid(system, position, global->cargoDropContainerId, global->hullDrop1NickNameId, hullDrop, clientKiller);
			Server.MineAsteroid(system, position, global->cargoDropContainerId, global->hullDrop2NickNameId, static_cast<int>(0.5 * hullDrop), clientKiller);
		}
	}

	/** @ingroup CargoDrop
	 * @brief Clear our variables so that we can recycle clients without confusion.
	 */
	void ClearClientInfo(ClientId& client)
	{
		global->info.erase(client);
	}

	/** @ingroup CargoDrop
	 * @brief Hook on SPObjUpdate, used to get the timestamp from the player. Used to figure out if the player has disconnected in the timer.
	 */
	void SPObjUpdate(SSPObjUpdateInfo const& ui, ClientId& client)
	{
		global->info[client].lastTimestamp = ui.timestamp;
		global->info[client].lastPosition = ui.pos;
		global->info[client].lastDirection = ui.dir;
	}
} // namespace Plugins::CargoDrop

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::CargoDrop;
REFL_AUTO(type(Config), field(reportDisconnectingPlayers), field(killDisconnectingPlayers), field(lootDisconnectingPlayers), field(disconnectingPlayersRange),
    field(hullDropFactor), field(disconnectMsg), field(cargoDropContainer), field(hullDrop1NickName), field(hullDrop2NickName), field(noLootItems),
    field(enablePlayerCargoDropOnDeath), field(maxPlayerCargoDropCount))

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Cargo Drop");
	pi->shortName("cargo_drop");
	pi->mayUnload(true);
	pi->timers(&timers);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__SendDeathMessage, &SendDeathMsg);
	pi->emplaceHook(HookedCall::IServerImpl__SPObjUpdate, &SPObjUpdate);
}