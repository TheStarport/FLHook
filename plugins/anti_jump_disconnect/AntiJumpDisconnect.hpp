#pragma once

#include "API/API.hpp"

namespace Plugins
{
	class AntiJumpDisconnect final : public Plugin
	{
		void ClearClientInfo(ClientId& client);
		void KillIfInJumpTunnel(ClientId& client);
		void Disconnect(ClientId& client, [[maybe_unused]] const EFLConnection& state);
		void CharacterInfoReq(ClientId& client, [[maybe_unused]] const bool& param2);
		void JumpInComplete([[maybe_unused]] const SystemId& system, [[maybe_unused]] const ShipId& ship);
		void SystemSwitchOutComplete([[maybe_unused]] const ShipId& Ship, ClientId& client);
	public:
		explicit AntiJumpDisconnect(const PluginInfo& info);

		//! Map of client ids and whether they are in a gate or not
		std::map<uint, bool> mapInfo;
	};
} // namespace Plugins::AntiJumpDisconnect