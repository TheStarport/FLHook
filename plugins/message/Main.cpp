/**
 * @date Feb, 2010
 * @author Cannon (Ported by Raikkonen)
 * @defgroup Message Message
 * @brief Handles different types of messages, banners and faction invitations.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - setmsg <n> <msg text> - Sets a preset message.
 * - showmsgs - Show your preset messages.
 * - {0-9} - Send preset message from slot 0-9.
 * - l{0-9} - Send preset message to local chat from slot 0-9.
 * - g{0-9} - Send preset message to group chat from slot 0-9.
 * - t{0-9} - Send preset message to last target from slot 0-9.
 * - target <message> - Send a message to previous/current target.
 * - reply <message> - Send a message to the last person to PM you.
 * - privatemsg <charname> <message> - Send private message to the specified player.
 * - factionmsg <tag> <message> - Send a message to the specified faction tag.
 * - factioninvite <tag> - Send a group invite to online members of the specified tag.
 * - lastpm - Shows the send of the last PM received.
 * - set chattime <on/off> - Enable time stamps on chat.
 * - me - Prints your name plus other text to system. Eg, /me thinks would say Bob thinks in system chat.
 * - do - Prints red text to system chat.
 * - time - Prints the current server time.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "CustomHelp": false,
 *     "DisconnectSwearingInSpaceMsg": "%player has been kicked for swearing",
 *     "DisconnectSwearingInSpaceRange": 5000.0,
 *     "EnableDo": true,
 *     "EnableMe": true,
 *     "EnableSetMessage": false,
 *     "GreetingBannerLines": ["<TRA data="0xDA70D608" mask="-1"/><TEXT>Welcome to the server.</TEXT>"],
 *     "SpecialBannerLines": ["<TRA data="0x40B60000" mask="-1"/><TEXT>This is a special banner.</TEXT>"],
 *     "SpecialBannerTimeout": 5,
 *     "StandardBannerLines": ["<TRA data="0x40B60000" mask="-1"/><TEXT>Here is a standard banner.</TEXT>"],
 *     "StandardBannerTimeout": 60,
 *     "SuppressMistypedCommands": true,
 *     "SwearWords": [""]
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * - Mail
 * - Tempban
 */

#include "Main.h"

namespace Plugins::Message
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup Message
	 * @brief Load the msgs for specified client Id into memory.
	 */
	static void LoadMsgs(ClientId client)
	{
		if (!global->config->EnableSetMessage)
			return;

		// Load from disk the messages.
		for (int iMsgSlot = 0; iMsgSlot < numberOfSlots; iMsgSlot++)
		{
			global->Info[client].slot[iMsgSlot] = Hk::Ini::GetCharacterIniString(client, L"msg." + std::to_wstring(iMsgSlot));
		}

		// Chat time settings.
		global->Info[client].showChatTime = Hk::Ini::GetCharacterIniBool(client, L"msg.chat_time");
	}

	/** @ingroup Message
	 * @brief Show the greeting banner to the specified player.
	 */
	static void ShowGreetingBanner(int client)
	{
		if (!global->Info[client].greetingShown)
		{
			global->Info[client].greetingShown = true;
			for (auto& line : global->config->GreetingBannerLines)
			{
				if (line.find(L"<TRA") == 0)
					Hk::Message::FMsg(client, line);
				else
					PrintUserCmdText(client, L"%s", line.c_str());
			}
		}
	}

	/** @ingroup Message
	 * @brief Show the special banner to all players.
	 */
	static void ShowSpecialBanner()
	{
		struct PlayerData* playerData = 0;
		while (playerData = Players.traverse_active(playerData))
		{
			ClientId client = playerData->iOnlineId;
			for (auto& line : global->config->SpecialBannerLines)
			{
				if (line.find(L"<TRA") == 0)
					Hk::Message::FMsg(client, line);
				else
					PrintUserCmdText(client, L"%s", line.c_str());
			}
		}
	}

	/** @ingroup Message
	 * @brief Show the next standard banner to all players.
	 */
	static void ShowStandardBanner()
	{
		if (global->config->StandardBannerLines.empty())
			return;

		static size_t iCurStandardBanner = 0;
		if (++iCurStandardBanner >= global->config->StandardBannerLines.size())
			iCurStandardBanner = 0;

		struct PlayerData* playerData = nullptr;
		while (playerData = Players.traverse_active(playerData))
		{
			ClientId client = playerData->iOnlineId;

			if (global->config->StandardBannerLines[iCurStandardBanner].find(L"<TRA") == 0)
				Hk::Message::FMsg(client, global->config->StandardBannerLines[iCurStandardBanner]);
			else
				PrintUserCmdText(client, L"%s", global->config->StandardBannerLines[iCurStandardBanner].c_str());
		}
	}

	/** @ingroup Message
	 * @brief Replace #t and #c tags with current target name and current ship location. Return false if tags cannot be replaced.
	 */
	static bool ReplaceMessageTags(ClientId client, ClientInfo& clientData, std::wstring& wscMsg)
	{
		if (wscMsg.find(L"#t") != -1)
		{
			if (clientData.targetClientId == -1)
			{
				PrintUserCmdText(client, L"ERR Target not available");
				return false;
			}

			const auto targetName = Hk::Client::GetCharacterNameByID(clientData.targetClientId);
			wscMsg = ReplaceStr(wscMsg, L"#t", targetName.value());
		}

		if (wscMsg.find(L"#c") != -1)
		{
			const std::wstring curLocation = Hk::Player::GetLocation(client);
			wscMsg = ReplaceStr(wscMsg, L"#c", curLocation);
		}

		return true;
	}

	/** @ingroup Message
	 * @brief Returns a string with the preset message
	 */
	std::wstring GetPresetMessage(ClientId client, int iMsgSlot)
	{
		const auto iter = global->Info.find(client);
		if (iter == global->Info.end() || iter->second.slot[iMsgSlot].empty())
		{
			PrintUserCmdText(client, L"ERR No message defined");
			return L"";
		}

		// Replace the tag #t with name of the targeted player.
		std::wstring wscMsg = iter->second.slot[iMsgSlot];
		if (!ReplaceMessageTags(client, iter->second, wscMsg))
			return L"";

		return wscMsg;
	}

	/** @ingroup Message
	 * @brief Send an preset message to the local system chat
	 */
	void SendPresetLocalMessage(ClientId client, int iMsgSlot)
	{
		if (!global->config->EnableSetMessage)
			return;

		if (iMsgSlot < 0 || iMsgSlot > 9)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /ln (n=0-9)");
			return;
		}

		Hk::Message::SendLocalSystemChat(client, GetPresetMessage(client, iMsgSlot));
	}

	/** @ingroup Message
	 * @brief Send a preset message to the last/current target.
	 */
	void SendPresetToLastTarget(ClientId client, int iMsgSlot)
	{
		if (!global->config->EnableSetMessage)
			return;

		UserCmd_SendToLastTarget(client, GetPresetMessage(client, iMsgSlot));
	}

	/** @ingroup Message
	 * @brief Send an preset message to the system chat 
	 */
	void SendPresetSystemMessage(ClientId client, int iMsgSlot)
	{
		if (!global->config->EnableSetMessage)
			return;

		Hk::Message::SendSystemChat(client, GetPresetMessage(client, iMsgSlot));
	}

	/** @ingroup Message
	 * @brief Send an preset message to the last PM sender
	 */
	void SendPresetLastPMSender(ClientId& client, int iMsgSlot, const std::wstring& wscMsg)
	{
		if (!global->config->EnableSetMessage)
			return;

		UserCmd_ReplyToLastPMSender(client, GetPresetMessage(client, iMsgSlot));
	}

	/** @ingroup Message
	 * @brief Send an preset message to the group chat
	 */
	void SendPresetGroupMessage(ClientId& client, int iMsgSlot)
	{
		if (!global->config->EnableSetMessage)
			return;

		Hk::Message::SendGroupChat(client, GetPresetMessage(client, iMsgSlot));
	}

	/** @ingroup Message
	 * @brief Clean up when a client disconnects
	 */
	void ClearClientInfo(ClientId& client) { global->Info.erase(client); }

	/** @ingroup Message
	 * @brief This function is called when the admin command rehash is called and when the module is loaded.
	 */
	void LoadSettings()
	{
		// For every active player load their msg settings.
		const std::list<PLAYERINFO> players = Hk::Admin::GetPlayers();
		for (auto& p : players)
			LoadMsgs(p.client);

		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

		// Load our communicators
		global->mailCommunicator = static_cast<Mail::MailCommunicator*>(PluginCommunicator::ImportPluginCommunicator(Mail::MailCommunicator::pluginName));
		global->tempBanCommunicator =
		    static_cast<Tempban::TempBanCommunicator*>(PluginCommunicator::ImportPluginCommunicator(Tempban::TempBanCommunicator::pluginName));
	}

	/** @ingroup Message
	 * @brief On this timer display banners
	 */
	void OneSecondTimer()
	{
		if (static int iSpecialBannerTimer = 0; ++iSpecialBannerTimer > global->config->SpecialBannerTimeout)
		{
			iSpecialBannerTimer = 0;
			ShowSpecialBanner();
		}

		if (static int iStandardBannerTimer = 0; ++iStandardBannerTimer > global->config->StandardBannerTimeout)
		{
			iStandardBannerTimer = 0;
			ShowStandardBanner();
		}
	}

	/** @ingroup Message
	 * @brief On client disconnect remove any references to this client.
	 */
	void DisConnect(ClientId& client, enum EFLConnection& p2)
	{
		auto iter = global->Info.begin();
		while (iter != global->Info.end())
		{
			if (iter->second.lastPmClientId == client)
				iter->second.lastPmClientId = -1;
			if (iter->second.targetClientId == client)
				iter->second.targetClientId = -1;
			++iter;
		}
	}

	/** @ingroup Message
	 * @brief On client F1 or entry to char select menu.
	 */
	void CharacterInfoReq(unsigned int client, bool p2)
	{
		auto iter = global->Info.begin();
		while (iter != global->Info.end())
		{
			if (iter->second.lastPmClientId == client)
				iter->second.lastPmClientId = -1;
			if (iter->second.targetClientId == client)
				iter->second.targetClientId = -1;
			++iter;
		}
	}

	/** @ingroup Message
	 * @brief On launch events and reload the msg cache for the client.
	 */
	void PlayerLaunch(uint& ship, ClientId& client)
	{
		LoadMsgs(client);
		ShowGreetingBanner(client);
	}

	/** @ingroup Message
	 * @brief On base entry events and reload the msg cache for the client.
	 */
	void BaseEnter(uint iBaseId, ClientId client)
	{
		LoadMsgs(client);
		ShowGreetingBanner(client);
	}

	/** @ingroup Message
	 * @brief When a char selects a target and the target is a player ship then record the target's client.
	 */
	void SetTarget(uint& uClientId, struct XSetTarget const& p2)
	{
		// The iSpaceId *appears* to represent a player ship Id when it is
		// targeted but this might not be the case. Also note that
		// GetClientIdByShip returns 0 on failure not -1.
		const auto targetClientId = Hk::Client::GetClientIdByShip(p2.iSpaceId);
		if (targetClientId.has_value())
		{
			const auto iter = global->Info.find(uClientId);
			if (iter != global->Info.end())
			{
				iter->second.targetClientId = targetClientId.value();
			}
		}
	}

	/** @ingroup Message
	 * @brief Hook on SubmitChat. Suppresses swearing. Records the last user to PM.
	 */
	bool SubmitChat(uint& cId, unsigned long& iSize, const void** rdlReader, uint& cIdTo, int& p2)
	{
		// Ignore group join/leave commands
		if (cIdTo == 0x10004)
			return false;

		// Extract text from rdlReader
		BinaryRDLReader rdl;
		wchar_t wszBuf[1024];
		uint iRet1;
		rdl.extract_text_from_buffer((unsigned short*)wszBuf, sizeof(wszBuf), iRet1, (const char*)*rdlReader, iSize);

		const std::wstring wscChatMsg = ToLower(wszBuf);
		ClientId client = cId;

		const bool bIsGroup = (cIdTo == 0x10003 || !wscChatMsg.find(L"/g ") || !wscChatMsg.find(L"/group "));
		if (!bIsGroup)
		{
			// If a restricted word appears in the message take appropriate action.
			for (auto& word : global->config->SwearWords)
			{
				if (wscChatMsg.find(word) != -1)
				{
					PrintUserCmdText(client, L"This is an automated message.");
					PrintUserCmdText(client, L"Please do not swear or you may be sanctioned.");

					global->Info[client].swearWordWarnings++;
					if (global->Info[client].swearWordWarnings > 2)
					{
						const std::wstring wscCharname = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));
						AddLog(LogType::Kick, LogLevel::Info, wstos(fmt::format(L"Swearing tempban on {} ({}) reason='{}'", wscCharname, 
						    Hk::Client::GetAccountID(Hk::Client::GetAccountByCharName(wscCharname).value()).value(), wscChatMsg.c_str())));

						if (global->tempBanCommunicator)
							global->tempBanCommunicator->TempBan(wscCharname, 10);

						Hk::Player::DelayedKick(client, 1);

						if (global->config->DisconnectSwearingInSpaceRange > 0.0f)
						{
							std::wstring wscMsg = global->config->DisconnectSwearingInSpaceMsg;
							wscMsg = ReplaceStr(wscMsg, L"%time", GetTimeString(FLHookConfig::i()->general.dieMsg));
							wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
							PrintLocalUserCmdText(client, wscMsg, global->config->DisconnectSwearingInSpaceRange);
						}
					}
					return true;
				}
			}
		}

		/// When a private chat message is sent from one client to another record
		/// who sent the message so that the receiver can reply using the /r command
		/// */
		if (client < 0x10000 && cIdTo > 0 && cIdTo < 0x10000)
		{
			const auto iter = global->Info.find(cIdTo);
			if (iter != global->Info.end())
			{
				iter->second.lastPmClientId = client;
			}
		}
		return false;
	}

	/** @ingroup Message
	 * @brief Prints RedText in the style of New Player messages.
	 */
	void RedText(std::wstring wscXMLMsg, uint iSystemId)
	{
		char szBuf[0x1000];
		uint iRet;
		const auto err = Hk::Message::FMsgEncodeXML(wscXMLMsg, szBuf, sizeof(szBuf), iRet);
		if (err.has_error())
			return;

		// Send to all players in system
		struct PlayerData* playerData = 0;
		while (playerData = Players.traverse_active(playerData))
		{
			ClientId client = playerData->iOnlineId;
			SystemId iClientSystemId = Hk::Player::GetSystem(client).value();

			if (iSystemId == iClientSystemId)
				Hk::Message::FMsgSendChat(client, szBuf, iRet);
		}
	}

	/** @ingroup Message
	 * @brief When a chat message is sent to a client and this client has showchattime on insert the time on the line immediately before the chat message
	 */
	bool SendChat(ClientId& client, uint& iTo, uint& iSize, void** rdlReader)
	{
		// Return immediately if the chat line is the time.
		if (global->SendingTime)
			return false;

		// Ignore group messages (I don't know if they ever get here
		if (iTo == 0x10004)
			return false;

		if (global->config->SuppressMistypedCommands)
		{
			// Extract text from rdlReader
			BinaryRDLReader rdl;
			wchar_t wszBuf[1024];
			uint iRet1;
			const void* rdlReader2 = *rdlReader;
			rdl.extract_text_from_buffer((unsigned short*)wszBuf, sizeof(wszBuf), iRet1, (const char*)rdlReader2, iSize);
			std::wstring wscChatMsg = wszBuf;

			// Find the ': ' which indicates the end of the sending player name.
			const size_t iTextStartPos = wscChatMsg.find(L": ");
			if (iTextStartPos != std::string::npos)
			{
				if ((wscChatMsg.find(L": /") == iTextStartPos && wscChatMsg.find(L": //") != iTextStartPos) || wscChatMsg.find(L": .") == iTextStartPos)
				{
					return true;
				}
			}
		}

		if (global->Info[client].showChatTime)
		{
			// Send time with gray color (BEBEBE) in small text (90) above the chat
			// line.
			global->SendingTime = true;
			Hk::Message::FMsg(client, L"<TRA data=\"0xBEBEBE90\" mask=\"-1\"/><TEXT>" + XMLText(GetTimeString(FLHookConfig::i()->general.dieMsg)) + L"</TEXT>");
			global->SendingTime = false;
		}
		return false;
	}

	/** @ingroup Message
	 * @brief Set a preset message
	 */
	void UserCmd_SetMsg(ClientId& client, const std::wstring& wscParam)
	{
		if (!global->config->EnableSetMessage)
			return;

		const int iMsgSlot = ToInt(GetParam(wscParam, ' ', 0));
		const std::wstring wscMsg = GetParamToEnd(ViewToWString(wscParam), ' ', 1);

		if (iMsgSlot < 0 || iMsgSlot > 9 || wscParam.size() == 0)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /setmsg <n> <msg text>");
			return;
		}

		Hk::Ini::SetCharacterIni(client, L"msg." + std::to_wstring(iMsgSlot), ViewToWString(wscMsg));

		// Reload the character cache
		LoadMsgs(client);
		PrintUserCmdText(client, L"OK");
	}

	/** @ingroup Message
	 * @brief Show preset messages
	 */
	void UserCmd_ShowMsgs(ClientId& client, const std::wstring& wscParam)
	{
		if (!global->config->EnableSetMessage)
			return;

		const auto iter = global->Info.find(client);
		if (iter == global->Info.end())
		{
			PrintUserCmdText(client, L"ERR No messages");
			return;
		}

		for (int i = 0; i < numberOfSlots; i++)
		{
			PrintUserCmdText(client, L"%d: %s", i, iter->second.slot[i].c_str());
		}
		PrintUserCmdText(client, L"OK");
	}

	/** @ingroup Message
	 * @brief User Commands for /r0-9
	 */
	void UserCmd_RMsg0(ClientId& client, const std::wstring& wscParam) { SendPresetLastPMSender(client, 0, wscParam); }

	void UserCmd_RMsg1(ClientId& client, const std::wstring& wscParam) { SendPresetLastPMSender(client, 1, wscParam); }

	void UserCmd_RMsg2(ClientId& client, const std::wstring& wscParam) { SendPresetLastPMSender(client, 2, wscParam); }

	void UserCmd_RMsg3(ClientId& client, const std::wstring& wscParam) { SendPresetLastPMSender(client, 3, wscParam); }

	void UserCmd_RMsg4(ClientId& client, const std::wstring& wscParam) { SendPresetLastPMSender(client, 4, wscParam); }

	void UserCmd_RMsg5(ClientId& client, const std::wstring& wscParam) { SendPresetLastPMSender(client, 5, wscParam); }

	void UserCmd_RMsg6(ClientId& client, const std::wstring& wscParam) { SendPresetLastPMSender(client, 6, wscParam); }

	void UserCmd_RMsg7(ClientId& client, const std::wstring& wscParam) { SendPresetLastPMSender(client, 7, wscParam); }

	void UserCmd_RMsg8(ClientId& client, const std::wstring& wscParam) { SendPresetLastPMSender(client, 8, wscParam); }

	void UserCmd_RMsg9(ClientId& client, const std::wstring& wscParam) { SendPresetLastPMSender(client, 9, wscParam); }

	/** @ingroup Message
	 * @brief User Commands for /s0-9
	 */
	void UserCmd_SMsg0(ClientId& client, const std::wstring& wscParam) { SendPresetSystemMessage(client, 0); }

	void UserCmd_SMsg1(ClientId& client, const std::wstring& wscParam) { SendPresetSystemMessage(client, 1); }

	void UserCmd_SMsg2(ClientId& client, const std::wstring& wscParam) { SendPresetSystemMessage(client, 2); }

	void UserCmd_SMsg3(ClientId& client, const std::wstring& wscParam) { SendPresetSystemMessage(client, 3); }

	void UserCmd_SMsg4(ClientId& client, const std::wstring& wscParam) { SendPresetSystemMessage(client, 4); }

	void UserCmd_SMsg5(ClientId& client, const std::wstring& wscParam) { SendPresetSystemMessage(client, 5); }

	void UserCmd_SMsg6(ClientId& client, const std::wstring& wscParam) { SendPresetSystemMessage(client, 6); }

	void UserCmd_SMsg7(ClientId& client, const std::wstring& wscParam) { SendPresetSystemMessage(client, 7); }

	void UserCmd_SMsg8(ClientId& client, const std::wstring& wscParam) { SendPresetSystemMessage(client, 8); }

	void UserCmd_SMsg9(ClientId& client, const std::wstring& wscParam) { SendPresetSystemMessage(client, 9); }

	/** @ingroup Message
	 * @brief User Commands for /l0-9
	 */
	void UserCmd_LMsg0(ClientId& client, const std::wstring& wscParam) { SendPresetLocalMessage(client, 0); }

	void UserCmd_LMsg1(ClientId& client, const std::wstring& wscParam) { SendPresetLocalMessage(client, 1); }

	void UserCmd_LMsg2(ClientId& client, const std::wstring& wscParam) { SendPresetLocalMessage(client, 2); }

	void UserCmd_LMsg3(ClientId& client, const std::wstring& wscParam) { SendPresetLocalMessage(client, 3); }

	void UserCmd_LMsg4(ClientId& client, const std::wstring& wscParam) { SendPresetLocalMessage(client, 4); }

	void UserCmd_LMsg5(ClientId& client, const std::wstring& wscParam) { SendPresetLocalMessage(client, 5); }

	void UserCmd_LMsg6(ClientId& client, const std::wstring& wscParam) { SendPresetLocalMessage(client, 6); }

	void UserCmd_LMsg7(ClientId& client, const std::wstring& wscParam) { SendPresetLocalMessage(client, 7); }

	void UserCmd_LMsg8(ClientId& client, const std::wstring& wscParam) { SendPresetLocalMessage(client, 8); }

	void UserCmd_LMsg9(ClientId& client, const std::wstring& wscParam) { SendPresetLocalMessage(client, 9); }

	/** @ingroup Message
	 * @brief User Commands for /g0-9
	 */
	void UserCmd_GMsg0(ClientId& client, const std::wstring& wscParam) { SendPresetGroupMessage(client, 0); }

	void UserCmd_GMsg1(ClientId& client, const std::wstring& wscParam) { SendPresetGroupMessage(client, 1); }

	void UserCmd_GMsg2(ClientId& client, const std::wstring& wscParam) { SendPresetGroupMessage(client, 2); }

	void UserCmd_GMsg3(ClientId& client, const std::wstring& wscParam) { SendPresetGroupMessage(client, 3); }

	void UserCmd_GMsg4(ClientId& client, const std::wstring& wscParam) { SendPresetGroupMessage(client, 4); }

	void UserCmd_GMsg5(ClientId& client, const std::wstring& wscParam) { SendPresetGroupMessage(client, 5); }

	void UserCmd_GMsg6(ClientId& client, const std::wstring& wscParam) { SendPresetGroupMessage(client, 6); }

	void UserCmd_GMsg7(ClientId& client, const std::wstring& wscParam) { SendPresetGroupMessage(client, 7); }

	void UserCmd_GMsg8(ClientId& client, const std::wstring& wscParam) { SendPresetGroupMessage(client, 8); }

	void UserCmd_GMsg9(ClientId& client, const std::wstring& wscParam) { SendPresetGroupMessage(client, 9); }

	/** @ingroup Message
	 * @brief User Commands for /t0-9
	 */
	void UserCmd_TMsg0(ClientId& client, const std::wstring& wscParam) { SendPresetToLastTarget(client, 0); }

	void UserCmd_TMsg1(ClientId& client, const std::wstring& wscParam) { SendPresetToLastTarget(client, 1); }

	void UserCmd_TMsg2(ClientId& client, const std::wstring& wscParam) { SendPresetToLastTarget(client, 2); }

	void UserCmd_TMsg3(ClientId& client, const std::wstring& wscParam) { SendPresetToLastTarget(client, 3); }

	void UserCmd_TMsg4(ClientId& client, const std::wstring& wscParam) { SendPresetToLastTarget(client, 4); }

	void UserCmd_TMsg5(ClientId& client, const std::wstring& wscParam) { SendPresetToLastTarget(client, 5); }

	void UserCmd_TMsg6(ClientId& client, const std::wstring& wscParam) { SendPresetToLastTarget(client, 6); }

	void UserCmd_TMsg7(ClientId& client, const std::wstring& wscParam) { SendPresetToLastTarget(client, 7); }

	void UserCmd_TMsg8(ClientId& client, const std::wstring& wscParam) { SendPresetToLastTarget(client, 8); }

	void UserCmd_TMsg9(ClientId& client, const std::wstring& wscParam) { SendPresetToLastTarget(client, 9); }

	/** @ingroup Message
	 * @brief Send an message to the last person that PM'd this client.
	 */
	void UserCmd_ReplyToLastPMSender(ClientId& client, const std::wstring& wscParam)
	{
		const auto iter = global->Info.find(client);
		if (iter == global->Info.end())
		{
			// There's no way for this to happen! yeah right.
			PrintUserCmdText(client, L"ERR No message defined");
			return;
		}

		const std::wstring wscMsg = GetParamToEnd(wscParam, ' ', 0);

		if (iter->second.lastPmClientId == -1)
		{
			PrintUserCmdText(client, L"ERR PM sender not available");
			return;
		}

		global->Info[iter->second.lastPmClientId].lastPmClientId = client;
		Hk::Message::SendPrivateChat(client, iter->second.lastPmClientId, ViewToWString(wscMsg));
	}

	/** @ingroup Message
	 * @brief Shows the sender of the last PM and the last char targeted
	 */
	void UserCmd_ShowLastPMSender(ClientId& client, const std::wstring& wscParam)
	{
		const auto iter = global->Info.find(client);
		if (iter == global->Info.end())
		{
			// There's no way for this to happen! yeah right.
			PrintUserCmdText(client, L"ERR No message defined");
			return;
		}

		std::wstring wscSenderCharname = L"<not available>" + std::to_wstring(iter->second.lastPmClientId);
		if (iter->second.lastPmClientId != -1 && Hk::Client::IsValidClientID(iter->second.lastPmClientId))
			wscSenderCharname = (const wchar_t*)Players.GetActiveCharacterName(iter->second.lastPmClientId);

		std::wstring wscTargetCharname = L"<not available>" + std::to_wstring(iter->second.targetClientId);
		if (iter->second.targetClientId != -1 && Hk::Client::IsValidClientID(iter->second.targetClientId))
			wscTargetCharname = (const wchar_t*)Players.GetActiveCharacterName(iter->second.targetClientId);

		PrintUserCmdText(client, L"OK sender=" + wscSenderCharname + L" target=" + wscTargetCharname);
	}

	/** @ingroup Message
	 * @brief Send a message to the last/current target. 
	 */
	void UserCmd_SendToLastTarget(ClientId& client, const std::wstring& wscParam)
	{
		const auto iter = global->Info.find(client);
		if (iter == global->Info.end())
		{
			// There's no way for this to happen! yeah right.
			PrintUserCmdText(client, L"ERR No message defined");
			return;
		}

		const std::wstring wscMsg = GetParamToEnd(wscParam, ' ', 0);

		if (iter->second.targetClientId == -1)
		{
			PrintUserCmdText(client, L"ERR PM target not available");
			return;
		}

		global->Info[iter->second.targetClientId].lastPmClientId = client;
		Hk::Message::SendPrivateChat(client, iter->second.targetClientId, ViewToWString(wscMsg));
	}

	/** @ingroup Message
	 * @brief Send a private message to the specified charname. If the player is offline the message will be delivery when they next login.
	 */
	void UserCmd_PrivateMsg(ClientId& client, const std::wstring& wscParam)
	{
		const std::wstring usage = L"Usage: /privatemsg <charname> <messsage> or /pm ...";
		const std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(client);
		const std::wstring& wscTargetCharname = GetParam(wscParam, ' ', 0);
		const std::wstring wscMsg = GetParamToEnd(wscParam, ' ', 1);

		if (wscCharname.size() == 0 || wscMsg.size() == 0)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, usage);
			return;
		}

		if (!Hk::Client::GetAccountByCharName(wscTargetCharname))
		{
			PrintUserCmdText(client, L"ERR charname does not exist");
			return;
		}

		const auto clientId = Hk::Client::GetClientIdFromCharName(wscTargetCharname);
		if (clientId.has_error())
		{
			if (global->mailCommunicator)
			{
				global->mailCommunicator->SendMail(wscTargetCharname, wscMsg);
				PrintUserCmdText(client, L"OK message saved to mailbox");
			}
			else
			{
				PrintUserCmdText(client, L"ERR: Player offline");
			}
		}
		else
		{
			global->Info[clientId.value()].lastPmClientId = client;
			Hk::Message::SendPrivateChat(client, clientId.value(), ViewToWString(wscMsg));
		}
	}

	/** @ingroup Message
	 * @brief Send a private message to the specified clientid.
	 */
	void UserCmd_PrivateMsgID(ClientId& client, const std::wstring& wscParam)
	{
		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(client);
		const std::wstring& wscClientId = GetParam(wscParam, ' ', 0);
		const std::wstring wscMsg = GetParamToEnd(wscParam, ' ', 1);

		uint iToClientId = ToInt(wscClientId);
		if (!Hk::Client::IsValidClientID(iToClientId) || Hk::Client::IsInCharSelectMenu(iToClientId))
		{
			PrintUserCmdText(client, L"ERR Invalid client-id");
			return;
		}

		global->Info[iToClientId].lastPmClientId = client;
		Hk::Message::SendPrivateChat(client, iToClientId, ViewToWString(wscMsg));
	}

	/** @ingroup Message
	 * @brief Send a message to all players with a particular prefix.
	 */
	void UserCmd_FactionMsg(ClientId& client, const std::wstring& wscParam)
	{
		std::wstring wscSender = (const wchar_t*)Players.GetActiveCharacterName(client);
		const std::wstring& wscCharnamePrefix = GetParam(wscParam, ' ', 0);
		const std::wstring& wscMsg = GetParamToEnd(wscParam, ' ', 1);

		if (wscCharnamePrefix.size() < 3 || wscMsg.size() == 0)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /factionmsg <tag> <message> or /fm ...");
			return;
		}

		bool bSenderReceived = false;
		bool bMsgSent = false;
		for (auto& player : Hk::Admin::GetPlayers())
		{
			if (ToLower(player.character).find(ToLower(wscCharnamePrefix)) == std::string::npos)
				continue;

			if (player.client == client)
				bSenderReceived = true;

			Hk::Message::FormatSendChat(player.client, wscSender, ViewToWString(wscMsg), L"FF7BFF");
			bMsgSent = true;
		}
		if (!bSenderReceived)
			Hk::Message::FormatSendChat(client, wscSender, ViewToWString(wscMsg), L"FF7BFF");

		if (bMsgSent == false)
			PrintUserCmdText(client, L"ERR No chars found");
	}

	/** @ingroup Message
	 * @brief Send a faction invite message to all players with a particular prefix.
	 */
	void UserCmd_FactionInvite(ClientId& client, const std::wstring& wscParam)
	{
		const std::wstring& wscCharnamePrefix = GetParam(wscParam, ' ', 0);

		bool msgSent = false;

		if (wscCharnamePrefix.size() < 3)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /factioninvite <tag> or /fi ...");
			return;
		}

		for (auto& player : Hk::Admin::GetPlayers())
		{
			if (ToLower(player.character).find(ToLower(wscCharnamePrefix)) == std::string::npos)
				continue;

			if (player.client == client)
				continue;

			std::wstring wscMsg = L"/i " + player.character;

			uint iRet;
			char szBuf[1024];
			if (const auto err = Hk::Message::FMsgEncodeXML(wscMsg, szBuf, sizeof(szBuf), iRet); err.has_error())
			{
				PrintUserCmdText(client, L"ERR " + Hk::Err::ErrGetText(err.error()));
				return;
			}

			const struct CHAT_ID cId = {client};
			const struct CHAT_ID cIdTo = {0x10001};

			Server.SubmitChat(cId, iRet, szBuf, cIdTo, -1);
			msgSent = true;
		}

		if (msgSent == false)
			PrintUserCmdText(client, L"ERR No chars found");
	}

	/** @ingroup Message
	 * @brief User command for enabling the chat timestamps.
	 */
	void UserCmd_SetChatTime(ClientId& client, const std::wstring& wscParam)
	{
		const std::wstring wscParam1 = ToLower(GetParam(wscParam, ' ', 0));
		bool bShowChatTime = false;
		if (!wscParam1.compare(L"on"))
			bShowChatTime = true;
		else if (!wscParam1.compare(L"off"))
			bShowChatTime = false;
		else
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /set chattime [on|off]");
		}

		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(client);

		Hk::Ini::SetCharacterIni(client, L"msg.chat_time", bShowChatTime ? L"true" : L"false");

		// Update the client cache.
		const auto iter = global->Info.find(client);
		if (iter != global->Info.end())
			iter->second.showChatTime = bShowChatTime;

		// Send confirmation msg
		PrintUserCmdText(client, L"OK");
	}

	/** @ingroup Message
	 * @brief Prints the current server time.
	 */
	void UserCmd_Time(ClientId& client, const std::wstring& wscParam)
	{
		// Send time with gray color (BEBEBE) in small text (90) above the chat line.
		PrintUserCmdText(client, GetTimeString(FLHookConfig::i()->general.localTime));
	}

	/** @ingroup Message
	 * @brief Me command allow players to type "/me powers his engines" which would print: "Trent powers his engines"
	 */
	void UserCmd_Me(ClientId& client, const std::wstring& wscParam)
	{
		if (global->config->EnableMe)
		{
			const std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
			SystemId iSystemId = Hk::Player::GetSystem(client).value();

			// Encode message using the death message style (red text).
			std::wstring wscXMLMsg = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.deathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
			wscXMLMsg += charname + L" ";
			wscXMLMsg += XMLText(ViewToWString(GetParamToEnd(wscParam, ' ', 0)));
			wscXMLMsg += L"</TEXT>";

			RedText(wscXMLMsg, iSystemId);
		}
		else
		{
			PrintUserCmdText(client, L"Command not enabled.");
		}
	}

	/** @ingroup Message
	 * @brief Do command allow players to type "/do Nomad fighters detected" which would print: "Nomad fighters detected" in the standard red text 
	 */
	void UserCmd_Do(ClientId& client, const std::wstring& wscParam)
	{
		if (global->config->EnableDo)
		{
			SystemId iSystemId = Hk::Player::GetSystem(client).value();

			// Encode message using the death message style (red text).
			std::wstring wscXMLMsg = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.deathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
			wscXMLMsg += XMLText(ViewToWString(GetParamToEnd(ViewToWString(wscParam), ' ', 0)));
			wscXMLMsg += L"</TEXT>";

			RedText(wscXMLMsg, iSystemId);
		}
		else
		{
			PrintUserCmdText(client, L"Command not enabled.");
		}
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/setmsg", L"<n> <msg text>", UserCmd_SetMsg, L"Sets a preset message."),
	    CreateUserCommand(L"/showmsgs", L"", UserCmd_ShowMsgs, L"Show your preset messages."),
	    CreateUserCommand(L"/0", L"", UserCmd_SMsg0, L"Send preset message from slot 0."),
	    CreateUserCommand(L"/1", L"", UserCmd_SMsg1, L""),
	    CreateUserCommand(L"/2", L"", UserCmd_SMsg2, L""),
	    CreateUserCommand(L"/3", L"", UserCmd_SMsg3, L""),
	    CreateUserCommand(L"/4", L"", UserCmd_SMsg4, L""),
	    CreateUserCommand(L"/5", L"", UserCmd_SMsg5, L""),
	    CreateUserCommand(L"/6", L"", UserCmd_SMsg6, L""),
	    CreateUserCommand(L"/7", L"", UserCmd_SMsg7, L""),
	    CreateUserCommand(L"/8", L"", UserCmd_SMsg8, L""),
	    CreateUserCommand(L"/9", L"", UserCmd_SMsg9, L""),
	    CreateUserCommand(L"/l0", L"", UserCmd_LMsg0, L"Send preset message to local chat from slot 0."),
	    CreateUserCommand(L"/l1", L"", UserCmd_LMsg1, L""),
	    CreateUserCommand(L"/l2", L"", UserCmd_LMsg2, L""),
	    CreateUserCommand(L"/l3", L"", UserCmd_LMsg3, L""),
	    CreateUserCommand(L"/l4", L"", UserCmd_LMsg4, L""),
	    CreateUserCommand(L"/l5", L"", UserCmd_LMsg5, L""),
	    CreateUserCommand(L"/l6", L"", UserCmd_LMsg6, L""),
	    CreateUserCommand(L"/l7", L"", UserCmd_LMsg7, L""),
	    CreateUserCommand(L"/l8", L"", UserCmd_LMsg8, L""),
	    CreateUserCommand(L"/l9", L"", UserCmd_LMsg9, L""),
	    CreateUserCommand(L"/g0", L"", UserCmd_GMsg0, L"Send preset message to group chat from slot 0."),
	    CreateUserCommand(L"/g1", L"", UserCmd_GMsg1, L""),
	    CreateUserCommand(L"/g2", L"", UserCmd_GMsg2, L""),
	    CreateUserCommand(L"/g3", L"", UserCmd_GMsg3, L""),
	    CreateUserCommand(L"/g4", L"", UserCmd_GMsg4, L""),
	    CreateUserCommand(L"/g5", L"", UserCmd_GMsg5, L""),
	    CreateUserCommand(L"/g6", L"", UserCmd_GMsg6, L""),
	    CreateUserCommand(L"/g7", L"", UserCmd_GMsg7, L""),
	    CreateUserCommand(L"/g8", L"", UserCmd_GMsg8, L""),
	    CreateUserCommand(L"/g9", L"", UserCmd_GMsg9, L""),
	    CreateUserCommand(L"/t0", L"", UserCmd_TMsg0, L"Send preset message to last target from slot 0."),
	    CreateUserCommand(L"/t1", L"", UserCmd_TMsg1, L""),
	    CreateUserCommand(L"/t2", L"", UserCmd_TMsg2, L""),
	    CreateUserCommand(L"/t3", L"", UserCmd_TMsg3, L""),
	    CreateUserCommand(L"/t4", L"", UserCmd_TMsg4, L""),
	    CreateUserCommand(L"/t5", L"", UserCmd_TMsg5, L""),
	    CreateUserCommand(L"/t6", L"", UserCmd_TMsg6, L""),
	    CreateUserCommand(L"/t7", L"", UserCmd_TMsg7, L""),
	    CreateUserCommand(L"/t8", L"", UserCmd_TMsg8, L""),
	    CreateUserCommand(L"/t9", L"", UserCmd_TMsg9, L""),
	    CreateUserCommand(L"/target", L"<message>", UserCmd_SendToLastTarget, L"Send a message to previous/current target."),
	    CreateUserCommand(L"/t", L"<message>", UserCmd_SendToLastTarget, L"Shortcut for /target."),
	    CreateUserCommand(L"/reply", L"<message>", UserCmd_ReplyToLastPMSender, L"Send a message to the last person to PM you."),
	    CreateUserCommand(L"/r", L"<message>", UserCmd_ReplyToLastPMSender, L"Shortcut for /reply."),
	    CreateUserCommand(L"/privatemsg$", L"<clientid> <message>", UserCmd_PrivateMsgID, L"Send private message to the specified client id."),
	    CreateUserCommand(L"/pm$", L"<clientid> <message>", UserCmd_PrivateMsgID, L"Shortcut for /privatemsg$."),
	    CreateUserCommand(L"/privatemsg", L"<charname> <message>", UserCmd_PrivateMsg, L"Send private message to the specified character name."),
	    CreateUserCommand(L"/pm", L"<charname> <message>", UserCmd_PrivateMsg, L"Shortcut for /privatemsg."),
	    CreateUserCommand(L"/factionmsg", L"<tag> <message>", UserCmd_FactionMsg, L"Send a message to the specified faction tag."),
	    CreateUserCommand(L"/fm", L"<tag> <message>", UserCmd_FactionMsg, L"Shortcut for /factionmsg."),
	    CreateUserCommand(L"/factioninvite", L"<name>", UserCmd_FactionInvite, L"Send a group invite to online members of the specified tag."),
	    CreateUserCommand(L"/fi", L"<name>", UserCmd_FactionInvite, L"Shortcut for /factioninvite."),
	    CreateUserCommand(L"/lastpm", L"", UserCmd_ShowLastPMSender, L"Shows the send of the last PM received."),
	    CreateUserCommand(L"/set chattime", L"<on|off>", UserCmd_SetChatTime, L"Enable time stamps on chat."),
	    CreateUserCommand(L"/me", L"<message>", UserCmd_Me, L"Prints your name plus other text to system. Eg, /me thinks would say Bob thinks in system chat."),
	    CreateUserCommand(L"/do", L"<message>", UserCmd_Do, L"Prints red text to system chat."),
	    CreateUserCommand(L"/time", L"", UserCmd_Time, L"Prints the current server time."),
	}};

} // namespace Plugins::Message

using namespace Plugins::Message;
REFL_AUTO(type(Config), field(GreetingBannerLines), field(SpecialBannerLines), field(StandardBannerLines), field(SpecialBannerTimeout),
    field(StandardBannerTimeout), field(CustomHelp), field(SuppressMistypedCommands), field(EnableSetMessage), field(EnableMe), field(EnableDo),
    field(DisconnectSwearingInSpaceMsg), field(DisconnectSwearingInSpaceRange), field(SwearWords))

DefaultDllMainSettings(LoadSettings)

    // Functions to hook
    extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Message");
	pi->shortName("message");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__SubmitChat, &SubmitChat);
	pi->emplaceHook(HookedCall::IChat__SendChat, &SendChat);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &OneSecondTimer);
	pi->emplaceHook(HookedCall::IServerImpl__SetTarget, &SetTarget);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
}