/**
 * @date June, 2022
 * @author Cannon (Ported by Raikkonen)
 * @defgroup Event Event
 * @brief
 * This plugin is used to create NPC and Cargo missions. For example, a base needs to be sold a certain amount of a commodity.
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
 *     "CargoMissions": {
 *         "Example": {
 *             "base": "li01_01_base",
 *             "current_amount": 0,
 *             "item": "commodity_gold",
 *             "required_amount": 99
 *         }
 *     },
 *     "NpcMissions": {
 *         "Example": {
 *             "current_amount": 0,
 *             "reputation": "li_n_grp",
 *             "required_amount": 99,
 *             "sector": "D1",
 *             "system": "li01"
 *         }
 *     }
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no dependencies.
 */

#include "Event.h"

namespace Plugins::Event
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		global->CargoMissions.clear();
		global->NpcMissions.clear();

		auto config = Serializer::JsonToObject<Config>();

		for (const auto& [missionName, cargoMission] : config.CargoMissions)
		{
			if (missionName == "Example")
				continue;

			CARGO_MISSION cargo_mission;
			cargo_mission.nickname = missionName;
			cargo_mission.base = CreateID(cargoMission.base.c_str());
			global->nicknameToNameMap[cargo_mission.base] = cargoMission.base;
			cargo_mission.item = CreateID(cargoMission.item.c_str());
			global->nicknameToNameMap[cargo_mission.item] = cargoMission.item;
			cargo_mission.required_amount = cargoMission.required_amount;
			cargo_mission.current_amount = cargoMission.current_amount;
			global->CargoMissions.push_back(cargo_mission);
		}

		for (const auto& [missionName, mission] : config.NpcMissions)
		{
			if (missionName == "Example")
				continue;

			NPC_MISSION npc_mission;
			npc_mission.nickname = missionName;
			npc_mission.system = CreateID(mission.system.c_str());
			global->nicknameToNameMap[npc_mission.system] = mission.system;
			npc_mission.sector = mission.sector;
			pub::Reputation::GetReputationGroup(npc_mission.reputation, mission.reputation.c_str());
			global->nicknameToNameMap[npc_mission.reputation] = mission.reputation;
			npc_mission.required_amount = mission.required_amount;
			npc_mission.current_amount = mission.current_amount;
			global->NpcMissions.push_back(npc_mission);
		}

		global->config = std::make_unique<Config>(config);

		Console::ConInfo(std::format("CargoMissionSettings loaded [{}]", global->CargoMissions.size()));
		Console::ConInfo(std::format("NpcMissionSettings loaded [{}]", global->NpcMissions.size()));
	}

	/** @ingroup Event
	 * @brief Save mission status every 100 seconds.
	 */
	void SaveMissionStatus()
	{
		if (global->CargoMissions.empty() && global->NpcMissions.empty())
			return;
		std::ofstream out("config/event.json");

		nlohmann::json jExport;

		for (auto& mission : global->CargoMissions)
		{
			nlohmann::json jsonMission = {{"base", global->nicknameToNameMap[mission.base]},
			    {"item", global->nicknameToNameMap[mission.item]},
			    {"current_amount", mission.current_amount},
			    {"required_amount", mission.required_amount}};
			jExport["CargoMissions"][mission.nickname] = jsonMission;
		}

		for (auto& mission : global->NpcMissions)
		{
			nlohmann::json jsonMission = {{"system", global->nicknameToNameMap[mission.system]},
			    {"reputation", global->nicknameToNameMap[mission.reputation]},
			    {"sector", mission.sector},
			    {"current_amount", mission.current_amount},
			    {"required_amount", mission.required_amount}};
			jExport["NpcMissions"][mission.nickname] = jsonMission;
		}
		out << std::setw(4) << jExport;
		out.close();
	}

	const std::vector<Timer> timers = {{SaveMissionStatus, 100}};

	/** @ingroup Event
	 * @brief Hook on ShipDestroyed to see if an NPC mission needs to be updated.
	 */
	void ShipDestroyed([[maybe_unused]] DamageList** _dmg, const DWORD** ecx, const uint& iKill)
	{
		if (iKill)
		{
			const CShip* cShip = Hk::Player::CShipFromShipDestroyed(ecx);

			int Reputation;
			pub::SpaceObj::GetRep(cShip->get_id(), Reputation);

			uint Affiliation;
			pub::Reputation::GetAffiliation(Reputation, Affiliation);

			SystemId System = Hk::Solar::GetSystemBySpaceId(cShip->get_id()).value();

			const Vector position = cShip->get_position();
			const std::string sector = Hk::Math::VectorToSectorCoord<std::string>(System, position);

			for (auto& mission : global->NpcMissions)
			{
				if (Affiliation == mission.reputation && System == mission.system && (mission.sector.empty() || mission.sector == sector) &&
				    mission.current_amount < mission.required_amount)
				{
					mission.current_amount++;
				}
			}
		}
	}

	/** @ingroup Event
	 * @brief Hook on GFGoodBuy to see if a cargo mission needs to be updated.
	 */
	void GFGoodBuy(struct SGFGoodBuyInfo const& gbi, ClientId& client)
	{
		auto base = Hk::Player::GetCurrentBase(client);

		for (auto& mission : global->CargoMissions)
		{
			if (mission.base == base.value() && mission.item == gbi.iGoodId)
			{
				mission.current_amount -= gbi.iCount;
				if (mission.current_amount < 0)
					mission.current_amount = 0;
			}
		}
	}

	/** @ingroup Event
	 * @brief Hook on GFGoodSell to see if a cargo mission needs to be updated.
	 */
	void GFGoodSell(const struct SGFGoodSellInfo& gsi, ClientId& client)
	{
		auto base = Hk::Player::GetCurrentBase(client);

		for (auto& mission : global->CargoMissions)
		{
			if (mission.base == base.value() && mission.item == gsi.iArchId && mission.current_amount < mission.required_amount)
			{
				int needed = mission.required_amount - mission.current_amount;
				if (needed > gsi.iCount)
				{
					mission.current_amount += gsi.iCount;
					needed = mission.required_amount - mission.current_amount;
					PrintUserCmdText(client, std::format(L"{} units remaining to complete mission objective", needed));
				}
				else
				{
					PrintUserCmdText(client, L"Mission objective completed");
				}
			}
		}
	}
} // namespace Plugins::Event

using namespace Plugins::Event;

REFL_AUTO(type(Config::CARGO_MISSION_REFLECTABLE), field(nickname), field(base), field(item), field(required_amount), field(current_amount))
REFL_AUTO(
    type(Config::NPC_MISSION_REFLECTABLE), field(nickname), field(system), field(sector), field(reputation), field(required_amount), field(current_amount))
REFL_AUTO(type(Config), field(CargoMissions), field(NpcMissions))

DefaultDllMainSettings(LoadSettings);

/** Functions to hook */
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Event");
	pi->shortName("event");
	pi->mayUnload(true);
	pi->timers(&timers);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__Startup, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &GFGoodBuy);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodSell, &GFGoodSell);
}
