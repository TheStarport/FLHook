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
	 * @brief This text mimics the "New Player" style of messages. This is legacy and will eventually be moved into core.
	 */
	bool RedText(uint clientID, const std::wstring& message, const std::wstring& message2)
	{
		const std::wstring characterName = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientID));
		uint systemID;
		pub::Player::GetSystem(clientID, systemID);

		std::wstring xmlMsg = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.deathMsgStyleSys + L"\" mask=\"-1\"/> <TEXT>";
		xmlMsg += XMLText(message);
		xmlMsg += characterName;
		xmlMsg += XMLText(message2);
		xmlMsg += L"</TEXT>";

		char buffer[0x1000];
		uint size;
		if (!HKHKSUCCESS(HkFMsgEncodeXML(xmlMsg, buffer, sizeof(buffer), size)))
			return false;

		// Send to all players in system
		struct PlayerData* playerData = nullptr;
		while (playerData = Players.traverse_active(playerData))
		{
			uint playerClientID = HkGetClientIdFromPD(playerData);
			uint clientSystemID = 0;
			pub::Player::GetSystem(playerClientID, clientSystemID);

			if (systemID == clientSystemID)
				HkFMsgSendChat(playerClientID, buffer, size);
		}
		return true;
	}

	/** @ingroup AwayFromKeyboard
	 * @brief This command is called when a player types /afk. It prints a message in red text to nearby players saying they are afk. It will also let anyone who messages them know too.
	 */
	void UserCmdAfk(const uint& clientID, const std::wstring_view& wscParam)
	{
		global->awayClients.emplace_back(clientID);
		RedText(clientID, L"", L" is now away from keyboard.");
		PrintUserCmdText(clientID, L"Use the /back command to stop sending automatic replies to PMs.");
	}

	// This function welcomes the player back and removes their afk status
	void Back(const uint clientId)
	{
		if (const auto it = global->awayClients.begin(); std::find(it, global->awayClients.end(), clientId) != global->awayClients.end())
		{
			global->awayClients.erase(it);
			std::wstring message = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(clientId));
			RedText(clientId, L"Welcome back ", L".");
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
	void DisConnect_AFTER(uint& clientId)
	{
		if (const auto it = global->awayClients.begin(); std::find(it, global->awayClients.end(), clientId) != global->awayClients.end())
		{
			global->awayClients.erase(it);
		}
	}

	// Hook on chat being sent (This gets called twice with the iClientID and to
	// swapped
	void __stdcall HkCb_SendChat(uint& clientId, uint& to, uint& size, void** rdl)
	{
		if (const auto it = global->awayClients.begin();
		    HkIsValidClientID(to) && std::find(it, global->awayClients.end(), clientId) != global->awayClients.end())
				PrintUserCmdText(to, L"This user is away from keyboard.");
	}

	// Hooks on chat being submitted
	void __stdcall SubmitChat(uint& clientId, unsigned long& lP1, void const** rdlReader, uint& to, int& iP2)
	{
		if (const auto it = global->awayClients.begin();
		    HkIsValidClientID(clientId) && std::find(it, global->awayClients.end(), clientId) != global->awayClients.end())
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
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::IChat__SendChat, &HkCb_SendChat);
	pi->emplaceHook(HookedCall::IServerImpl__SubmitChat, &SubmitChat);
}
