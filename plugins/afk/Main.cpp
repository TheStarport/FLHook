/**
 * @date August, 2022
 * @author Raikkonen
 * @defgroup AwayFromKeyboard Away from Keyboard
 * @brief
 * The AFK plugin allows you to set yourself as Away from Keyboard. 
 * This will notify other players if they try and speak to you, that you are not at your desk.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - afk - Sets your status to Away from Keyboard. Other players will notified if they try to speak to you.
 * - back - Removes the AFK status.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * No configuration file is needed.
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

#include "Main.h"

namespace Plugins::Afk
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup AwayFromKeyboard
	 * @brief This command is called when a player types /afk. It prints a message in red text to nearby players saying they are afk. It will also let anyone who messages them know too.
	 */
	void UserCmdAfk(const uint& clientId, const std::wstring_view& wscParam)
	{
		global->awayClients.emplace_back(clientId);
		const std::wstring playerName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));
		const auto message = Hk::Message::FormatMsg(MessageColor::Red, MessageFormat::Normal,playerName + L" is now away from keyboard.");

		const auto systemId = Hk::Player::GetSystem(clientId);

		if (systemId.has_error())
		{
			PrintUserCmdText(clientId, Hk::Err::ErrGetText(systemId.error()));
			return;
		}
		const auto _ = Hk::Message::FMsgS(systemId.value(), message);

		PrintUserCmdText(clientId, L"Use the /back command to stop sending automatic replies to PMs.");
	}

	// This function welcomes the player back and removes their afk status
	void Back(const ClientId clientId)
	{
		if (const auto it = global->awayClients.begin(); std::find(it, global->awayClients.end(), clientId) != global->awayClients.end())
		{
			const auto systemId = Hk::Player::GetSystem(clientId);

			if (systemId.has_error())
			{
				PrintUserCmdText(clientId, Hk::Err::ErrGetText(systemId.error()));
				return;
			}
			global->awayClients.erase(it);
			const std::wstring playerName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));
			const auto message = Hk::Message::FormatMsg(MessageColor::Red, MessageFormat::Normal, playerName + L" has returned");
			const auto _ = Hk::Message::FMsgS(systemId.value(), message);
			return;
		}
		PrintUserCmdText(clientId, L"You are not marked as AFK. To do this, use the /afk command.");
	}

	/** @ingroup AwayFromKeyboard
	 * @brief This command is called when a player types /back. It removes the afk status and welcomes the player back.
	 * who messages them know too.
	 */
	void UserCmdBack(const uint& clientId, const std::wstring_view& wscParam)
	{
		Back(clientId);
	}

	// Clean up when a client disconnects
	void ClearClientInfo(const ClientId& clientId)
	{
		if (const auto it = global->awayClients.begin(); std::find(it, global->awayClients.end(), clientId) != global->awayClients.end())
		{
			global->awayClients.erase(it);
		}
	}

	// Hook on chat being sent (This gets called twice with the iClientID and to
	// swapped
	void __stdcall Cb_SendChat(ClientId& clientId, ClientId& to, uint& size, void** rdl)
	{
		if (const auto it = global->awayClients.begin();
			Hk::Client::IsValidClientID(to) && std::find(it, global->awayClients.end(), clientId) != global->awayClients.end())
				PrintUserCmdText(to, L"This user is away from keyboard.");
	}

	// Hooks on chat being submitted
	void __stdcall SubmitChat(const ClientId& clientId, unsigned long& lP1, void const** rdlReader, const ClientId& to, int& iP2)
	{
		if (const auto it = global->awayClients.begin();
			Hk::Client::IsValidClientID(clientId) && std::find(it, global->awayClients.end(), clientId) != global->awayClients.end())
			Back(clientId);
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/afk", L"", UserCmdAfk, L"Sets your status to ""Away from Keyboard"". Other players will notified if they try to speak to you."),
	    CreateUserCommand(L"/back", L"", UserCmdBack, L"Removes the AFK status."),
	}};
} // namespace Plugins::AFK

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::Afk;

DefaultDllMain()
    // Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("AFK");
	pi->shortName("afk");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::IChat__SendChat, &Cb_SendChat);
	pi->emplaceHook(HookedCall::IServerImpl__SubmitChat, &SubmitChat);
}
