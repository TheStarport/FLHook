/**
 * @date Feb, 2010
 * @author Cannon, ported by Raikkonen
 * @defgroup SystemSensor System Sensor
 * @brief
 * The plugin allows players with proper equipment to see player traffic coming through
 * Trade Lanes and Jump Gates in the system, as well as being able to look up
 * their equipment and cargo at the time of using them.
 *
 * @paragraph cmds Player Commands
 * -net <all/jumponly/off> - if player has proper equipment, toggles his scanner between showing JG/TL transits,
 *   JG transits only, and disabling the feature
 * -shoan <name> - shows equipment and cargo carried by the specified player
 * -shoan$ <playerID> - same as above, but using player ID as paramenter, useful for to type difficult names
 *
 * @paragraph adminCmds Admin Commands
 * None
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "sensors": [
 *         {"systemId": "Li01",
 *          "equipId": "li_gun01_mark01",
 *          "networkId": 1}
 *          ]
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

#include "SystemSensor.h"
#include <Tools/Serialization/Attributes.hpp>

namespace Plugins::SystemSensor
{
	const auto global = std::make_unique<Global>();
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		std::ranges::for_each(config.sensors, [](const ReflectableSensor& sensor) {
			Sensor s = {sensor.systemId, sensor.equipId, sensor.networkId};
			global->sensorEquip.insert(std::multimap<EquipId, Sensor>::value_type(CreateID(sensor.equipId.c_str()), s));
			global->sensorSystem.insert(std::multimap<SystemId, Sensor>::value_type(CreateID(sensor.systemId.c_str()), s));
		});
	}

	void UserCmd_Net(ClientId& client, const std::wstring& param)
	{
		const std::wstring mode = ToLower(GetParam(param, ' ', 0));
		if (mode.empty())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /net [all|jumponly|off]");
			return;
		}

		if (!global->networks[client].availableNetworkId)
		{
			PrintUserCmdText(client, L"ERR Sensor network monitoring is not available");
			global->networks[client].mode = Mode::Off;
			return;
		}

		if (mode == L"all")
		{
			PrintUserCmdText(client, L"OK Sensor network monitoring all traffic");
			global->networks[client].mode = Mode::Both;
		}
		else if (mode == L"jumponly")
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

	void UserCmd_Shoan(ClientId& client, const std::wstring& param)
	{
		const std::wstring targetCharname = GetParam(param, ' ', 0);

		if (targetCharname.empty())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage:");
			PrintUserCmdText(client, L"/shoan <charname>");
			PrintUserCmdText(client, L"/shoan$ <playerID>");
			return;
		}

		const auto targetClientId = Hk::Client::GetClientIdFromCharName(targetCharname);
		if (targetClientId.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(targetClientId.error()));
			return;
		}

		const auto& [targetSensorClientId, targetSensor] = *global->networks.find(targetClientId.value());
		if (!targetSensorClientId || !global->networks[client].availableNetworkId || !targetSensor.lastScanNetworkId ||
		    global->networks[client].availableNetworkId != targetSensor.lastScanNetworkId)
		{
			PrintUserCmdText(client, L"ERR Scan data not available");
			return;
		}

		std::wstring eqList;
		for (auto const& ci : targetSensor.lastScanList)
		{
			std::string hardpoint = ci.hardpoint.value;
			if (!hardpoint.empty())
			{
				Archetype::Equipment* eq = Archetype::GetEquipment(ci.iArchId);
				if (eq && eq->iIdsName)
				{
					std::wstring result;
					switch (Hk::Client::GetEqType(eq))
					{
						case ET_GUN:
						case ET_MISSILE:
						case ET_CD:
						case ET_CM:
						case ET_TORPEDO:
						case ET_OTHER:
							if (eqList.length())
								eqList += L",";
							result = Hk::Message::GetWStringFromIdS(eq->iIdsName);
							eqList += result;
							break;
						default:
							break;
					}
				}
			}
		}
		PrintUserCmdText(client, eqList);
		PrintUserCmdText(client, L"OK");
	}

	void UserCmd_ShoanID(ClientId& client, const std::wstring& param)
	{
		ClientId client2 = ToInt(GetParam(param, ' ', 0));

		std::wstring targetCharname = L"";

		if (Hk::Client::IsValidClientID(client2))
			targetCharname = (wchar_t*)Players.GetActiveCharacterName(client2);

		UserCmd_Shoan(client, targetCharname);
	}

	void ClearClientInfo(ClientId& client)
	{
		global->networks.erase(client);
	}

	static void EnableSensorAccess(ClientId client)
	{
		// Retrieve the location and cargo list.
		int holdSize;
		const auto cargo = Hk::Player::EnumCargo(client, holdSize);

		SystemId systemId = Hk::Player::GetSystem(client).value();

		// If this is ship has the right equipment and is in the right system then
		// enable access.
		uint availableNetworkId = 0;
		for (auto& ci : cargo.value())
		{
			if (ci.bMounted)
			{
				auto start = global->sensorEquip.lower_bound(ci.iArchId);
				auto end = global->sensorEquip.upper_bound(ci.iArchId);
				while (start != end)
				{
					if (start->second.systemId == systemId)
					{
						availableNetworkId = start->second.networkId;
						break;
					}
					++start;
				}
			}
		}

		if (availableNetworkId != global->networks[client].availableNetworkId)
		{
			global->networks[client].availableNetworkId = availableNetworkId;
			if (availableNetworkId)
				PrintUserCmdText(client,
				    L"Connection to tradelane sensor network "
				    L"established. Type /net access network.");
			else
				PrintUserCmdText(client, L"Connection to tradelane sensor network lost.");
		}
	}

	void PlayerLaunch([[maybe_unused]] const uint& ship, ClientId& client)
	{
		EnableSensorAccess(client);
	}

	static void DumpSensorAccess(ClientId client, const std::wstring& type, Mode mode)
	{
		SystemId systemId = Hk::Player::GetSystem(client).value();

		// Find the sensor network for this system.
		const auto siter = global->sensorSystem.lower_bound(systemId);
		if (const auto send = global->sensorSystem.upper_bound(systemId); siter == send)
			return;

		if (global->networks.contains(client))
		{
			ClearClientInfo(client);
		}

		// Record the ship's cargo.
		int holdSize;
		global->networks[client].lastScanList = Hk::Player::EnumCargo(client, holdSize).value();
		global->networks[client].lastScanNetworkId = siter->second.networkId;

		// Notify any players connected to the the sensor network that this ship is
		// in
		for (const auto& [playerId, sensor] : global->networks)
		{
			if (sensor.availableNetworkId == siter->second.networkId)
			{
				const Universe::ISystem* system = Universe::get_system(systemId);
				if (system && magic_enum::enum_integer(sensor.mode & mode))
				{
					std::wstring sysName = Hk::Message::GetWStringFromIdS(system->strid_name);
					const auto location = Hk::Solar::GetLocation(client, IdType::Client);
					const auto playerSystem = Hk::Player::GetSystem(client);
					const Vector& position = location.value().first;
					const std::wstring curLocation = Hk::Math::VectorToSectorCoord<std::wstring>(playerSystem.value(), position);
					PrintUserCmdText(
					    playerId, std::format(L"{}[${}] {} at {} {}", Hk::Client::GetCharacterNameByID(client).value(), client, type, sysName, curLocation));
				}
			}
		}
	}

	// Record jump type.
	int Dock_Call(unsigned int const& ship, unsigned int const& dockTarget, const int& cancel, const enum DOCK_HOST_RESPONSE& response)
	{
		if (const auto client = Hk::Client::GetClientIdByShip(ship); client.has_value() && (response == PROCEED_DOCK || response == DOCK) && cancel >= 0)
		{
			auto spaceObjType = Hk::Solar::GetType(dockTarget);
			if (spaceObjType.has_error())
			{
				Console::ConWarn(wstos(Hk::Err::ErrGetText(spaceObjType.error())));
			}

			global->networks[client.value()].inJumpGate = spaceObjType.value() == OBJ_JUMP_GATE;
		}

		return 0;
	}

	void JumpInComplete([[maybe_unused]] SystemId& system, [[maybe_unused]] ShipId& ship, ClientId& client)
	{
		EnableSensorAccess(client);
		if (global->networks[client].inJumpGate)
		{
			global->networks[client].inJumpGate = false;
			DumpSensorAccess(client, L"exited jumpgate", Mode::JumpGate);
		}
	}

	void GoTradelane(ClientId& client, [[maybe_unused]] struct XGoTradelane const& xgt)
	{
		DumpSensorAccess(client, L"entered tradelane", Mode::TradeLane);
	}

	void StopTradelane(ClientId& client, [[maybe_unused]] const uint& p1, [[maybe_unused]] const uint& p2, [[maybe_unused]] const uint& p3)
	{
		DumpSensorAccess(client, L"exited tradelane", Mode::TradeLane);
	}
	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/shoan", L"<name>", UserCmd_Shoan, L"Shows equipment and cargo carried by the specified player."),
	    CreateUserCommand(L"/shoan$", L"<playerID>", UserCmd_ShoanID, L"Same as /shoan, but using player ID as paramenter, useful for to type difficult names"),
	    CreateUserCommand(L"/net", L"<all/jumponly/off>", UserCmd_Net, L"Toggles your scanner between off, jump gates only, and both tradelanes and jump gates"),
	}};
} // namespace Plugins::SystemSensor

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::SystemSensor;

REFL_AUTO(type(ReflectableSensor), field(equipId), field(systemId), field(networkId));
REFL_AUTO(type(Config), field(sensors, AttrNotEmpty<std::vector<ReflectableSensor>> {}));

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("System Sensor");
	pi->shortName("system_sensor");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::IEngine__DockCall, &Dock_Call);
	pi->emplaceHook(HookedCall::IServerImpl__GoTradelane, &GoTradelane);
	pi->emplaceHook(HookedCall::IServerImpl__StopTradelane, &StopTradelane);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__JumpInComplete, &JumpInComplete);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}