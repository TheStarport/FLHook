// System Sensor - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.
//
// This file includes code that was not written by me but I can't find
// the original author (I know they posted on the-starport.net about it)

#include "Main.h"

namespace Plugins::SystemSensor
{
	const auto global = std::make_unique<Global>();
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		std::for_each(config.sensors.begin(), config.sensors.end(), [](const ReflectableSensor& sensor) {
			Sensor s = {sensor.systemId, sensor.equipId, sensor.networkId};
			global->sensorEquip.insert(std::multimap<EquipId, Sensor>::value_type(CreateID(sensor.equipId.c_str()), s));
			global->sensorEquip.insert(std::multimap<SystemId, Sensor>::value_type(CreateID(sensor.systemId.c_str()), s));
		});
	}

	void UserCmd_Net(ClientId& client, const std::wstring_view& wscParam)
	{
		const std::wstring wscMode = ToLower(GetParam(wscParam, ' ', 0));
		if (wscMode.empty())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /net [all|jumponly|off]");
			return;
		}

		if (!global->networks[client].iAvailableNetworkId)
		{
			PrintUserCmdText(client, L"ERR Sensor network monitoring is not available");
			global->networks[client].mode = Mode::Off;
			return;
		}

		if (wscMode == L"all")
		{
			PrintUserCmdText(client, L"OK Sensor network monitoring all traffic");
			global->networks[client].mode = Mode::Both;
		}
		else if (wscMode == L"jumponly")
		{
			PrintUserCmdText(client, L"OK Sensor network monitoring jumpgate traffic only");
			global->networks[client].mode = Mode::JumpGate;
		}
		else
		{
			PrintUserCmdText(client, L"OK Sensor network monitoring disabled");
			global->networks[client].mode = Mode::Off;
		}
		return;
	}

	void UserCmd_ShowScan(ClientId& client, const std::wstring_view& wscParam)
	{
		std::wstring wscTargetCharname = GetParam(wscParam, ' ', 0);

		if (wscTargetCharname.size() == 0)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /showscan <charname>");
			return;
		}

		const auto iTargetClientId = Hk::Client::GetClientIdFromCharName(wscTargetCharname);
		if (iTargetClientId.value() == -1)
		{
			PrintUserCmdText(client, L"ERR Target not found");
			return;
		}

		auto iterTargetClientId = global->networks.find(iTargetClientId.value());
		if (iterTargetClientId == global->networks.end() || !global->networks[client].iAvailableNetworkId || !iterTargetClientId->second.lastScanNetworkId ||
		    global->networks[client].iAvailableNetworkId != iterTargetClientId->second.lastScanNetworkId)
		{
			PrintUserCmdText(client, L"ERR Scan data not available");
			return;
		}

		std::wstring wscEqList;
		for (auto& ci : iterTargetClientId->second.lstLastScan)
		{
			std::string scHardpoint = ci.hardpoint.value;
			if (scHardpoint.length())
			{
				Archetype::Equipment* eq = Archetype::GetEquipment(ci.iArchId);
				if (eq && eq->iIdsName)
				{
					std::wstring wscResult;
					switch (Hk::Client::GetEqType(eq))
					{
						case ET_GUN:
						case ET_MISSILE:
						case ET_CD:
						case ET_CM:
						case ET_TORPEDO:
						case ET_OTHER:
							if (wscEqList.length())
								wscEqList += L",";
							wscResult = Hk::Message::GetWStringFromIdS(eq->iIdsName);
							wscEqList += wscResult;
							break;
						default:
							break;
					}
				}
			}
		}
		PrintUserCmdText(client, L"%s", wscEqList.c_str());
		PrintUserCmdText(client, L"OK");
	}

	void UserCmd_ShowScanID(ClientId& client, const std::wstring_view& wscParam)
	{
		ClientId client2 = ToInt(GetParam(wscParam, ' ', 0));

		std::wstring wscTargetCharname = L"";

		if (Hk::Client::IsValidClientID(client2))
			wscTargetCharname = (wchar_t*)Players.GetActiveCharacterName(client2);

		UserCmd_ShowScan(client, wscTargetCharname);
	}

	void ClearClientInfo(ClientId& client) { global->networks.erase(client); }

	static void EnableSensorAccess(ClientId client)
	{
		// Retrieve the location and cargo list.
		int iHoldSize;
		const auto cargo = Hk::Player::EnumCargo(client, iHoldSize);

		unsigned int iSystemId;
		pub::Player::GetSystem(client, iSystemId);

		// If this is ship has the right equipment and is in the right system then
		// enable access.
		uint iAvailableNetworkId = 0;
		for (auto& ci : cargo.value())
		{
			if (ci.bMounted)
			{
				auto start = global->sensorEquip.lower_bound(ci.iArchId);
				auto end = global->sensorEquip.upper_bound(ci.iArchId);
				while (start != end)
				{
					if (start->second.systemId == iSystemId)
					{
						iAvailableNetworkId = start->second.networkId;
						break;
					}
					++start;
				}
			}
		}

		if (iAvailableNetworkId != global->networks[client].iAvailableNetworkId)
		{
			global->networks[client].iAvailableNetworkId = iAvailableNetworkId;
			if (iAvailableNetworkId)
				PrintUserCmdText(client,
				    L"Connection to tradelane sensor network "
				    L"established. Type /net access network.");
			else
				PrintUserCmdText(client, L"Connection to tradelane sensor network lost.");
		}
	}

	void PlayerLaunch(uint& ship, ClientId& client) { EnableSensorAccess(client); }

	static void DumpSensorAccess(ClientId client, const std::wstring& wscType, Mode mode)
	{
		unsigned int iSystemId;
		pub::Player::GetSystem(client, iSystemId);

		// Find the sensor network for this system.
		auto siter = global->sensorSystem.lower_bound(iSystemId);
		auto send = global->sensorSystem.upper_bound(iSystemId);
		if (siter == send)
			return;

		if (global->networks.find(client) == global->networks.end())
		{
			ClearClientInfo(client);
		}

		// Record the ship's cargo.
		int iHoldSize;
		global->networks[client].lstLastScan = Hk::Player::EnumCargo(client, iHoldSize).value();
		global->networks[client].lastScanNetworkId = siter->second.networkId;

		// Notify any players connected to the the sensor network that this ship is
		// in
		auto iter = global->networks.begin();
		const auto pend = global->networks.end();
		while (iter != pend)
		{
			if (iter->second.iAvailableNetworkId == siter->second.networkId)
			{
				const Universe::ISystem* iSys = Universe::get_system(iSystemId);
				if (iSys && enum_integer(iter->second.mode & mode))
				{
					std::wstring wscSysName = Hk::Message::GetWStringFromIdS(iSys->strid_name);
					PrintUserCmdText(iter->first,
					    L"%s[$%u] %s at %s %s",
						Players.GetActiveCharacterName(client),
						client,
						wscType.c_str(),
						wscSysName.c_str(),
						Hk::Player::GetLocation(client).c_str());
				}
			}
			++iter;
		}
	}

	// Record jump type.
	void Dock_Call(unsigned int const& ship, unsigned int const& iDockTarget, int& iCancel, enum DOCK_HOST_RESPONSE& response)
	{
		const auto client = Hk::Client::GetClientIdByShip(ship);
		if (client && (response == PROCEED_DOCK || response == DOCK) && !iCancel)
		{
			uint iTypeId;
			pub::SpaceObj::GetType(iDockTarget, iTypeId);
			if (iTypeId == OBJ_JUMP_GATE)
			{
				global->networks[client.value()].inJumpGate = true;
			}
			else
			{
				global->networks[client.value()].inJumpGate = false;
			}
		}
	}

	void JumpInComplete(SystemId& iSystem, ShipId& ship, ClientId& client)
	{
		EnableSensorAccess(client);
		if (global->networks[client].inJumpGate)
		{
			global->networks[client].inJumpGate = false;
			DumpSensorAccess(client, L"exited jumpgate", Mode::JumpGate);
		}
	}

	void GoTradelane(ClientId& client, struct XGoTradelane const& xgt) { DumpSensorAccess(client, L"entered tradelane", Mode::TradeLane); }

	void StopTradelane(ClientId& client, uint& p1, uint& p2, uint& p3) { DumpSensorAccess(client, L"exited tradelane", Mode::TradeLane); }
	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/showscan", L"", UserCmd_ShowScan, L""),
	    CreateUserCommand(L"/showscan$", L"", UserCmd_ShowScan, L""),
	    CreateUserCommand(L"/net", L"", UserCmd_Net, L""),
	}};
} // namespace Plugins::SystemSensor

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::SystemSensor;

REFL_AUTO(type(ReflectableSensor), field(equipId), field(systemId), field(networkId));
REFL_AUTO(type(Config), field(sensors));

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("System Sensor");
	pi->shortName("system_sensor");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::IEngine__DockCall, &Dock_Call);
	pi->emplaceHook(HookedCall::IServerImpl__GoTradelane, &GoTradelane);
	pi->emplaceHook(HookedCall::IServerImpl__StopTradelane, &StopTradelane);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__JumpInComplete, &JumpInComplete);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}