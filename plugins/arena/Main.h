#pragma once
#include "../base/Main.h"

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Arena
{
	enum class ClientState
	{
		None,
		Transfer,
		Return
	};

	const std::wstring StrInfo1 = L"Please dock at nearest base";
	const std::wstring StrInfo2 = L"Cargo hold is not empty";

	//! Config data for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/arena.json"; }

		// Reflectable fields
		//! The command to be used to beam to the arena system. Defaults to "arena".
		std::string command = "arena";
		//! The base in the pvp system they should be beamed to.
		std::string targetBase;
		//! The pvp system they should be beamed to.
		std::string targetSystem;
		//! The system the commands should not work for.
		std::string restrictedSystem;

		// Non-reflectable fields
		uint targetBaseId;
		uint targetSystemId;
		uint restrictedSystemId;
		std::wstring wscCommand;
	};

	//! Global data for this plugin
	struct Global
	{
		std::array<ClientState, MaxClientId + 1> transferFlags;
		//! This plugin can communicate with the base plugin if loaded.
		BaseCommunicator* baseCommunicator = nullptr;
		//! Return code for the plugin
		ReturnCode returnCode = ReturnCode::Default;
		std::unique_ptr<Config> config = nullptr;
	};
} // namespace Plugins::Arena