#pragma once

#include <plugin.h>
#include "Tools/Serialization/Serializer.hpp"

namespace Plugins
{
	class Arena final : public Plugin
	{
		//! This plugin can communicate with the base plugin if loaded.
		enum class ClientState
		{
			None,
			Transfer,
			Return
		};
		const std::wstring dockErrorText = L"Please dock at nearest base";
		const std::wstring cargoErrorText = L"Cargo hold is not empty";
		//! Config data for this plugin
		struct Config final : Reflectable
		{
			std::string File() override { return "config/arena.json"; }

			// Reflectable fields
			//! The command to be used to beam to the arena system. Defaults to "arena".
			std::string command = "arena";
			//! The base in the pvp system they should be beamed to.
			std::string targetBase;
			//! The pvp system they should be beamed to.
			std::string targetSystem;
			//! The system the commands should not work for.
			std::string restrictedSystem;
		};
		Config config;
		// Non-reflectable fields
		uint targetBaseId = 0;
		uint targetSystemId = 0;
		uint restrictedSystemId = 0;
		std::wstring command = L"arena";

		std::array<ClientState, MaxClientId + 1> transferFlags = {};

		void ClearClientInfo(ClientId& client);
		void LoadSettings();
		void CharacterSelect([[maybe_unused]] const std::string& charFilename, ClientId& client);
		void PlayerLaunch_AFTER([[maybe_unused]] const uint& ship, ClientId& client);
		void UserCmd_Conn(ClientId& client, [[maybe_unused]] const std::wstring& param);
		void UserCmd_Return(ClientId& client, [[maybe_unused]] const std::wstring& param);

	public:
		explicit Arena(const PluginInfo& info);
	};
} // namespace Plugins