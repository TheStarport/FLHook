// AFK Plugin
// By Raikkonen

#include "Main.h"

namespace Plugins::Afk
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	// This text mimics the "New Player" messages
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

	// This command is called when a player types /afk
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

	// This command is called when a player types /back
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
	const std::vector<UserCommand> commands = {{
	    CreateUserCommand(L"/afk", L"", UserCmdAfk, L""),
	    CreateUserCommand(L"/back", L"", UserCmdBack, L""),
	}};

	// Hook on /help
	void UserCmdHelp(const uint& clientId, const std::wstring_view& wscParam)
	{
		PrintUserCmdText(clientId, L"/afk ");
		PrintUserCmdText(clientId,
		    L"Sets the player to AFK. If any other player messages "
		    L"directly, they will be told you are afk.");
		PrintUserCmdText(clientId, L"/back ");
		PrintUserCmdText(clientId, L"Turns off AFK for a the player.");
	}
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
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::IChat__SendChat, &HkCb_SendChat);
	pi->emplaceHook(HookedCall::IServerImpl__SubmitChat, &SubmitChat);
}
