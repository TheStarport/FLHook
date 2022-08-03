﻿/**
 Event Plugin for FLHook-Plugin
 by Cannon.

June 2022 - Ported by Raikkonen
*/

// includes
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
			if (cargo.first == "Example Nickname")
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
			if (mission.first == "Example Nickname")
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

	// Save mission status every 100 seconds
	void HkTimerCheckKick()
	{
		if ((time(0) % 100) == 0)
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
	}

	void __stdcall ShipDestroyed(DamageList** _dmg, const DWORD** ecx, uint& iKill)
	{
		if (iKill)
		{
			const CShip* cShip = HkCShipFromShipDestroyed(ecx);

			int Reputation;
			pub::SpaceObj::GetRep(cShip->get_id(), Reputation);

			uint Affiliation;
			pub::Reputation::GetAffiliation(Reputation, Affiliation);

			uint System;
			pub::SpaceObj::GetSystem(cShip->get_id(), System);

			const Vector position = cShip->get_position();
			const std::string sector = VectorToSectorCoord<std::string>(System, position);

			for (auto& mission : global->NpcMissions)
			{
				if (Affiliation == mission.reputation && System == mission.system && mission.sector.length() && mission.sector == sector &&
				    mission.current_amount < mission.required_amount)
				{
					mission.current_amount++;
					int needed = mission.required_amount = mission.current_amount;
					// Print Mission text here in red text once we integrate that function into core
				}
			}
		}
	}

	void __stdcall GFGoodBuy(struct SGFGoodBuyInfo const& gbi, uint& iClientID)
	{
		uint Base;
		pub::Player::GetBase(iClientID, Base);

		for (auto& mission : global->CargoMissions)
		{
			if (mission.base == Base && mission.item == gbi.iGoodID)
			{
				mission.current_amount -= gbi.iCount;
				if (mission.current_amount < 0)
					mission.current_amount = 0;
			}
		}
	}

	void __stdcall GFGoodSell(const struct SGFGoodSellInfo& gsi, uint& iClientID)
	{
		uint Base;
		pub::Player::GetBase(iClientID, Base);

		for (auto& mission : global->CargoMissions)
		{
			if (mission.base == Base && mission.item == gsi.iArchID && mission.current_amount < mission.required_amount)
			{
				int needed = mission.required_amount - mission.current_amount;
				if (needed > gsi.iCount)
				{
					mission.current_amount += gsi.iCount;
					needed = mission.required_amount - mission.current_amount;
					PrintUserCmdText(iClientID, L"%d units remaining to complete mission objective", needed);
				}
				else
				{
					PrintUserCmdText(iClientID, L"Mission objective completed", needed);
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
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &HkTimerCheckKick);
	pi->emplaceHook(HookedCall::IEngine__ShipDestroyed, &ShipDestroyed);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &GFGoodBuy);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodSell, &GFGoodSell);
}