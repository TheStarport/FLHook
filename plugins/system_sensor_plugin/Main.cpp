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

	void UserCmd_Net(const uint& iClientID, const std::wstring_view& wscParam)
	{
		const std::wstring wscMode = ToLower(GetParam(wscParam, ' ', 0));
		if (wscMode.empty())
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /net [all|jumponly|off]");
			return;
		}

		if (!global->networks[iClientID].iAvailableNetworkID)
		{
			PrintUserCmdText(iClientID, L"ERR Sensor network monitoring is not available");
			global->networks[iClientID].mode = Mode::Off;
			return;
		}

		if (wscMode == L"all")
		{
			PrintUserCmdText(iClientID, L"OK Sensor network monitoring all traffic");
			global->networks[iClientID].mode = Mode::Both;
		}
		else if (wscMode == L"jumponly")
		{
			PrintUserCmdText(iClientID, L"OK Sensor network monitoring jumpgate traffic only");
			global->networks[iClientID].mode = Mode::JumpGate;
		}
		else
		{
			PrintUserCmdText(iClientID, L"OK Sensor network monitoring disabled");
			global->networks[iClientID].mode = Mode::Off;
		}
		return;
	}

	void UserCmd_ShowScan(const uint& iClientID, const std::wstring_view& wscParam)
	{
		std::wstring wscTargetCharname = GetParam(wscParam, ' ', 0);

		if (wscTargetCharname.size() == 0)
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /showscan <charname>");
			return;
		}

		const uint iTargetClientID = HkGetClientIDFromArg(wscTargetCharname);
		if (iTargetClientID == -1)
		{
			PrintUserCmdText(iClientID, L"ERR Target not found");
			return;
		}

		auto iterTargetClientID = global->networks.find(iTargetClientID);
		if (iterTargetClientID == global->networks.end() || !global->networks[iClientID].iAvailableNetworkID || !iterTargetClientID->second.lastScanNetworkId ||
		    global->networks[iClientID].iAvailableNetworkID != iterTargetClientID->second.lastScanNetworkId)
		{
			PrintUserCmdText(iClientID, L"ERR Scan data not available");
			return;
		}

		std::wstring wscEqList;
		for (auto& ci : iterTargetClientID->second.lstLastScan)
		{
			std::string scHardpoint = ci.hardpoint.value;
			if (scHardpoint.length())
			{
				Archetype::Equipment* eq = Archetype::GetEquipment(ci.iArchID);
				if (eq && eq->iIdsName)
				{
					std::wstring wscResult;
					switch (HkGetEqType(eq))
					{
						case ET_GUN:
						case ET_MISSILE:
						case ET_CD:
						case ET_CM:
						case ET_TORPEDO:
						case ET_OTHER:
							if (wscEqList.length())
								wscEqList += L",";
							wscResult = HkGetWStringFromIDS(eq->iIdsName);
							wscEqList += wscResult;
							break;
						default:
							break;
					}
				}
			}
		}
		PrintUserCmdText(iClientID, L"%s", wscEqList.c_str());
		PrintUserCmdText(iClientID, L"OK");
	}

	void UserCmd_ShowScanID(const uint& iClientID, const std::wstring_view& wscParam)
	{
		const uint iClientID2 = ToInt(GetParam(wscParam, ' ', 0));

		std::wstring wscTargetCharname = L"";

		if (HkIsValidClientID(iClientID2))
			wscTargetCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID2);

		UserCmd_ShowScan(iClientID, wscTargetCharname);
	}

	void ClearClientInfo(uint& iClientID) { global->networks.erase(iClientID); }

	static void EnableSensorAccess(uint iClientID)
	{
		// Retrieve the location and cargo list.
		int iHoldSize;
		std::list<CARGO_INFO> lstCargo;
		HkEnumCargo((const wchar_t*)Players.GetActiveCharacterName(iClientID), lstCargo, iHoldSize);

		unsigned int iSystemID;
		pub::Player::GetSystem(iClientID, iSystemID);

		// If this is ship has the right equipment and is in the right system then
		// enable access.
		uint iAvailableNetworkID = 0;
		for (auto& ci : lstCargo)
		{
			if (ci.bMounted)
			{
				auto start = global->sensorEquip.lower_bound(ci.iArchID);
				auto end = global->sensorEquip.upper_bound(ci.iArchID);
				while (start != end)
				{
					if (start->second.systemId == iSystemID)
					{
						iAvailableNetworkID = start->second.networkId;
						break;
					}
					++start;
				}
			}
		}

		if (iAvailableNetworkID != global->networks[iClientID].iAvailableNetworkID)
		{
			global->networks[iClientID].iAvailableNetworkID = iAvailableNetworkID;
			if (iAvailableNetworkID)
				PrintUserCmdText(iClientID,
				    L"Connection to tradelane sensor network "
				    L"established. Type /net access network.");
			else
				PrintUserCmdText(iClientID, L"Connection to tradelane sensor network lost.");
		}
	}

	void PlayerLaunch(uint& iShip, uint& iClientID) { EnableSensorAccess(iClientID); }

	static void DumpSensorAccess(uint iClientID, const std::wstring& wscType, Mode mode)
	{
		unsigned int iSystemID;
		pub::Player::GetSystem(iClientID, iSystemID);

		// Find the sensor network for this system.
		auto siter = global->sensorSystem.lower_bound(iSystemID);
		auto send = global->sensorSystem.upper_bound(iSystemID);
		if (siter == send)
			return;

		if (global->networks.find(iClientID) == global->networks.end())
		{
			ClearClientInfo(iClientID);
		}

		// Record the ship's cargo.
		int iHoldSize;
		HkEnumCargo(iClientID, global->networks[iClientID].lstLastScan, iHoldSize);
		global->networks[iClientID].lastScanNetworkId = siter->second.networkId;

		// Notify any players connected to the the sensor network that this ship is
		// in
		auto iter = global->networks.begin();
		const auto pend = global->networks.end();
		while (iter != pend)
		{
			if (iter->second.iAvailableNetworkID == siter->second.networkId)
			{
				const Universe::ISystem* iSys = Universe::get_system(iSystemID);
				if (iSys && enum_integer(iter->second.mode & mode))
				{
					std::wstring wscSysName = HkGetWStringFromIDS(iSys->strid_name);
					PrintUserCmdText(iter->first,
					    L"%s[$%u] %s at %s %s",
						Players.GetActiveCharacterName(iClientID),
						iClientID,
						wscType.c_str(),
						wscSysName.c_str(),
						GetLocation(iClientID).c_str());
				}
			}
			++iter;
		}
	}

	// Record jump type.
	void Dock_Call(unsigned int const& iShip, unsigned int const& iDockTarget, int& iCancel, enum DOCK_HOST_RESPONSE& response)
	{
		uint iClientID = HkGetClientIDByShip(iShip);
		if (iClientID && (response == PROCEED_DOCK || response == DOCK) && !iCancel)
		{
			uint iTypeID;
			pub::SpaceObj::GetType(iDockTarget, iTypeID);
			if (iTypeID == OBJ_JUMP_GATE)
			{
				global->networks[iClientID].inJumpGate = true;
			}
			else
			{
				global->networks[iClientID].inJumpGate = false;
			}
		}
	}

	void JumpInComplete(SystemId& iSystem, ShipId& iShip, ClientId& iClientID)
	{
		EnableSensorAccess(iClientID);
		if (global->networks[iClientID].inJumpGate)
		{
			global->networks[iClientID].inJumpGate = false;
			DumpSensorAccess(iClientID, L"exited jumpgate", Mode::JumpGate);
		}
	}

	void GoTradelane(uint& iClientID, struct XGoTradelane const& xgt) { DumpSensorAccess(iClientID, L"entered tradelane", Mode::TradeLane); }

	void StopTradelane(uint& iClientID, uint& p1, uint& p2, uint& p3) { DumpSensorAccess(iClientID, L"exited tradelane", Mode::TradeLane); }
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