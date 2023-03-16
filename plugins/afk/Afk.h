#pragma once

// Includes
#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::Afk
{
	class AfkPlugin final : public Plugin
	{
		std::vector<uint> awayClients;

		/** @ingroup AwayFromKeyboard
		 * @brief This command is called when a player types /afk. It prints a message in red text to nearby players saying they are afk. It will also let
		 * anyone who messages them know too.
		 */
		void UserCmdAfk(ClientId& client);

		/** @ingroup AwayFromKeyboard
		 * @brief This command is called when a player types /back. It removes the afk status and welcomes the player back.
		 * who messages them know too.
		 */
		void UserCmdBack(ClientId& client);

	  public:
		explicit AfkPlugin(const PluginInfo& info);

		void ClearClientInfo(ClientId& client);
		void SendChat(ClientId& client, ClientId& targetClient, [[maybe_unused]] const uint& size, [[maybe_unused]] void** rdl);
		void SubmitChat(ClientId& client, [[maybe_unused]] const unsigned long& lP1, [[maybe_unused]] void const** rdlReader, [[maybe_unused]] ClientId& to,
		    [[maybe_unused]] const int& dunno);
	};
} // namespace Plugins::Afk