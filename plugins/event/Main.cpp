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
 *         "Example Nickname": {
 *             "base": "li01_01_base",
 *             "current_amount": 0,
 *             "item": "commodity_gold",
 *             "nickname": "Example",
 *             "required_amount": 99
 *         }
 *     },
 *     "NpcMissions": {
 *         "Example Nickname": {
 *             "current_amount": 0,
 *             "nickname": "Example",
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

#include "Main.h"

namespace Plugins::Event
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{
		global->CargoMissions.clear();
		global->NpcMissions.clear();

		auto config = Serializer::JsonToObject<Config>();
		
		for (auto& cargo : config.CargoMissions)
		{
			if (cargo.first == "Example")
				continue;

			CARGO_MISSION cargo_mission;
			cargo_mission.nickname = cargo.second.nickname;
			cargo_mission.base = CreateID(cargo.second.base.c_str());
			cargo_mission.item = CreateID(cargo.second.item.c_str());
			cargo_mission.required_amount = cargo.second.required_amount;
			cargo_mission.current_amount = cargo.second.current_amount;
			global->CargoMissions.push_back(cargo_mission);
		}

		for (auto& mission : config.NpcMissions)
		{
			if (mission.first == "Example")
				continue;

			NPC_MISSION npc_mission;
			npc_mission.nickname = mission.second.nickname;
			npc_mission.system = CreateID(mission.second.system.c_str());
			npc_mission.sector = mission.second.sector;
			pub::Reputation::GetReputationGroup(npc_mission.reputation, mission.second.reputation.c_str());
			npc_mission.required_amount = mission.second.required_amount;
			npc_mission.current_amount = mission.second.current_amount;
			global->NpcMissions.push_back(npc_mission);
		}

		global->config = std::make_unique<Config>(config);

		Console::ConInfo(L"CargoMissionSettings loaded [%d]", global->CargoMissions.size());
		Console::ConInfo(L"NpcMissionSettings loaded [%d]", global->NpcMissions.size());
	}

	/** @ingroup Event
	 * @brief Save mission status every 100 seconds.
	 */
	void SaveMissionStatus()
	{
		std::ofstream out("flhook_plugins/event.json");

		nlohmann::json jExport;

		for (auto& mission : global->CargoMissions)
		{
			nlohmann::json jMission;
			jMission["nickname"] = mission.nickname;
			jMission["base"] = mission.base;
			jMission["item"] = mission.item;
			jMission["current_amount"] = mission.current_amount;
			jMission["required_amount"] = mission.required_amount;
			jExport["CargoMissions"].push_back(jMission);
		}

		for (auto& mission : global->NpcMissions)
		{
			nlohmann::json jMission;
			jMission["nickname"] = mission.nickname;
			jMission["system"] = mission.system;
			jMission["reputation"] = mission.reputation;
			jMission["sector"] = mission.sector;
			jMission["current_amount"] = mission.current_amount;
			jMission["required_amount"] = mission.required_amount;
			jExport["NpcMissions"].push_back(jMission);
		}

		out << jExport;
		out.close();

	}

	const std::vector<Timer> timers = {
		{SaveMissionStatus, 100}
	};

	/** @ingroup Event
	 * @brief Hook on ShipDestroyed to see if an NPC mission needs to be updated.
	 */
	void __stdcall ShipDestroyed(DamageList** _dmg, const DWORD** ecx, uint& iKill)
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
				if (Affiliation == mission.reputation && System == mission.system && mission.sector.length() && mission.sector == sector &&
				    mission.current_amount < mission.required_amount)
				{
					mission.current_amount++;
					int needed = mission.required_amount = mission.current_amount;
					// TODO: Print Mission text here in red text once we integrate that function into core
				}
			}
		}
	}

	/** @ingroup Event
	 * @brief Hook on GFGoodBuy to see if a cargo mission needs to be updated.
	 */
	void __stdcall GFGoodBuy(struct SGFGoodBuyInfo const& gbi, ClientId& client)
	{
		uint Base;
		pub::Player::GetBase(client, Base);

		for (auto& mission : global->CargoMissions)
		{
			if (mission.base == Base && mission.item == gbi.iGoodId)
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
	void __stdcall GFGoodSell(const struct SGFGoodSellInfo& gsi, ClientId& client)
	{
		uint Base;
		pub::Player::GetBase(client, Base);

		for (auto& mission : global->CargoMissions)
		{
			if (mission.base == Base && mission.item == gsi.iArchId && mission.current_amount < mission.required_amount)
			{
				int needed = mission.required_amount - mission.current_amount;
				if (needed > gsi.iCount)
				{
					mission.current_amount += gsi.iCount;
					needed = mission.required_amount - mission.current_amount;
					PrintUserCmdText(client, L"%d units remaining to complete mission objective", needed);
				}
				else
				{
					PrintUserCmdText(client, L"Mission objective completed", needed);
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

DefaultDllMainSettings(LoadSettings)

/** Functions to hook */
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Event");
	pi->shortName("event");
	pi->mayUnload(true);
	pi->timers(timers);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &GFGoodBuy);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodSell, &GFGoodSell);
}
