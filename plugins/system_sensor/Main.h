#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::SystemSensor
{
	using NetworkId = uint;
	enum class Mode
	{
		Off = 0x00,
		JumpGate = 0x01,
		TradeLane = 0x02,
		Both = 0x1 | 0x2
	};

	struct Sensor final
	{
		Sensor() = delete;
		Sensor(const std::string& systemId, const std::string& equipId, const NetworkId networkId) : systemID(CreateID(systemId.c_str())), equipID(CreateID(equipId.c_str())), networkID(networkId)
		{
		}

		const SystemId systemId;
		const EquipId equipId;
		const NetworkId networkId;
	};

	//! Map of equipment and systems that have sensor networks
	struct ActiveNetwork
	{
		std::list<CARGO_INFO> lstLastScan;
		NetworkId iAvailableNetworkId = 0;
		NetworkId lastScanNetworkId = 0;
		bool inJumpGate = false;
		Mode mode = Mode::Off;
	};

	struct ReflectableSensor final : Reflectable
	{
		std::string systemId;
		std::string equipId;
		NetworkId networkId;
	};

	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/systemSensor.json"; }

		std::vector<ReflectableSensor> sensors;
	};

	struct Global
	{
		ReturnCode returnCode = ReturnCode::Default;
		std::map<ClientId, ActiveNetwork> networks;
		std::multimap<EquipId, Sensor> sensorEquip;
		std::multimap<SystemId, Sensor> sensorSystem;
	};
}