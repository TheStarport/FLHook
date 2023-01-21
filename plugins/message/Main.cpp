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
#include "Features/Mail.hpp"

namespace Plugins::Message
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	long GetNumberFromCmd(const std::wstring& param)
	{
		const auto numStr = param[1];
		wchar_t* _;
		return std::wcstol(&numStr, &_, 10);
	}

	/** @ingroup Message
	 * @brief Load the msgs for specified client Id into memory.
	 */
	static void LoadMsgs(ClientId client)
	{
		// Load from disk the messages.
		for (int iMsgSlot = 0; iMsgSlot < numberOfSlots; iMsgSlot++)
		{
			global->info[client].slot[iMsgSlot] = Hk::Ini::GetCharacterIniString(client, L"msg." + std::to_wstring(iMsgSlot));
		}

		// Chat time settings.
		global->info[client].showChatTime = Hk::Ini::GetCharacterIniBool(client, L"msg.chat_time");
	}

	/** @ingroup Message
	 * @brief Show the greeting banner to the specified player.
	 */
	static void ShowGreetingBanner(int client)
	{
		for (const auto& line : global->config->greetingBannerLines)
		{
			if (line.find(L"<TRA") == 0)
				Hk::Message::FMsg(client, line);
			else
				PrintUserCmdText(client, line);
		}
	}

	/** @ingroup Message
	 * @brief Load the custom message templates and show the greeting banner to the specified player.
	 */
	static void PlayerLogin([[maybe_unused]] const std::string_view& charFilename, ClientId& client)
	{ 
		LoadMsgs(client);
		ShowGreetingBanner(client);
	}

	/** @ingroup Message
	 * @brief Show the special banner to all players.
	 */
	static void ShowSpecialBanner()
	{
		struct PlayerData* playerData = Players.traverse_active(nullptr);
		while (playerData)
		{
			ClientId client = playerData->iOnlineId;
			for (const auto& line : global->config->specialBannerLines)
			{
				if (line.find(L"<TRA") == 0)
					Hk::Message::FMsg(client, line);
				else
					PrintUserCmdText(client, line);
			}
			playerData = Players.traverse_active(playerData);
		}
	}

	/** @ingroup Message
	 * @brief Show the next standard banner to all players.
	 */
	static void ShowStandardBanner()
	{
		if (global->config->standardBannerLines.empty())
			return;

		static size_t curStandardBanner = 0;
		if (++curStandardBanner >= global->config->standardBannerLines.size())
			curStandardBanner = 0;

		struct PlayerData* playerData = Players.traverse_active(nullptr);
		while (playerData)
		{
			ClientId client = playerData->iOnlineId;

			if (global->config->standardBannerLines[curStandardBanner].find(L"<TRA") == 0)
				Hk::Message::FMsg(client, global->config->standardBannerLines[curStandardBanner]);
			else
				PrintUserCmdText(client, global->config->standardBannerLines[curStandardBanner].c_str());

			playerData = Players.traverse_active(playerData);
		}
	}

	/** @ingroup Message
	 * @brief Replace #t and #c tags with current target name and current ship location. Return false if tags cannot be replaced.
	 */
	static bool ReplaceMessageTags(ClientId client, const ClientInfo& clientData, std::wstring& wscMsg)
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
			const auto location = Hk::Solar::GetLocation(client, IdType::Client);
			const auto system = Hk::Player::GetSystem(client);
			const Vector& position = location.value().first;
			if (location.has_value() && system.has_value())
			{
				const std::wstring curLocation = Hk::Math::VectorToSectorCoord<std::wstring>(system.value(), position);
				wscMsg = ReplaceStr(wscMsg, L"#c", curLocation);
			}
			else
			{
				PrintUserCmdText(client, L"ERR Target not available");
				return false;
			}
		}

		return true;
	}

	/** @ingroup Message
	 * @brief Returns a string with the preset message
	 */
	std::wstring GetPresetMessage(ClientId client, int iMsgSlot)
	{
		const auto iter = global->info.find(client);
		if (iter == global->info.end() || iter->second.slot[iMsgSlot].empty())
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
		if (!global->config->enableSetMessage)
		{
			PrintUserCmdText(client, L"Set commands disabled");
			return;
		}
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
		if (!global->config->enableSetMessage)
		{
			PrintUserCmdText(client, L"Set commands disabled");
			return;
		}
		UserCmd_SendToLastTarget(client, GetPresetMessage(client, iMsgSlot));
	}

	/** @ingroup Message
	 * @brief Send an preset message to the system chat
	 */
	void SendPresetSystemMessage(ClientId client, int iMsgSlot)
	{
		if (!global->config->enableSetMessage)
		{
			PrintUserCmdText(client, L"Set commands disabled");
			return;
		}
		Hk::Message::SendSystemChat(client, GetPresetMessage(client, iMsgSlot));
	}

	/** @ingroup Message
	 * @brief Send an preset message to the last PM sender
	 */
	void SendPresetLastPMSender(ClientId& client, int msgSlot)
	{
		if (!global->config->enableSetMessage)
		{
			PrintUserCmdText(client, L"Set commands disabled");
			return;
		}

		UserCmd_ReplyToLastPMSender(client, GetPresetMessage(client, msgSlot));
	}

	/** @ingroup Message
	 * @brief Send an preset message to the group chat
	 */
	void SendPresetGroupMessage(ClientId& client, int iMsgSlot)
	{
		if (!global->config->enableSetMessage)
		{
			PrintUserCmdText(client, L"Set commands disabled");
			return;
		}

		Hk::Message::SendGroupChat(client, GetPresetMessage(client, iMsgSlot));
	}

	/** @ingroup Message
	 * @brief Clean up when a client disconnects
	 */
	void ClearClientInfo(ClientId& client) { global->info.erase(client); }

	/** @ingroup Message
	 * @brief This function is called when the admin command rehash is called and when the module is loaded.
	 */
	void LoadSettings()
	{
		// For every active player load their msg settings.
		for (const auto& p : Hk::Admin::GetPlayers())
			LoadMsgs(p.client);

		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

		// Load our communicators
		global->tempBanCommunicator =
		    static_cast<Tempban::TempBanCommunicator*>(PluginCommunicator::ImportPluginCommunicator(Tempban::TempBanCommunicator::pluginName));
	}

	/** @ingroup Message
	 * @brief On this timer display banners
	 */
	void OneSecondTimer()
	{
		if (static int specialBannerTimer = 0; ++specialBannerTimer > global->config->specialBannerTimeout)
		{
			specialBannerTimer = 0;
			ShowSpecialBanner();
		}

		if (static int standardBannerTimer = 0; ++standardBannerTimer > global->config->standardBannerTimeout)
		{
			standardBannerTimer = 0;
			ShowStandardBanner();
		}
	}

	const std::vector<Timer> timers = {{OneSecondTimer, 1}};

	/** @ingroup Message
	 * @brief On client disconnect remove any references to this client.
	 */
	void DisConnect(ClientId& client, const enum EFLConnection [[maybe_unused]])
	{
		auto iter = global->info.begin();
		while (iter != global->info.end())
		{
			if (iter->second.lastPmClientId == client)
				iter->second.lastPmClientId = -1;
			if (iter->second.targetClientId == client)
				iter->second.targetClientId = -1;
			++iter;
		}
		global->info.erase(client);
	}

	/** @ingroup Message
	 * @brief On client F1 or entry to char select menu.
	 */
	void CharacterInfoReq(unsigned int client, bool [[maybe_unused]])
	{
		auto iter = global->info.begin();
		while (iter != global->info.end())
		{
			if (iter->second.lastPmClientId == client)
				iter->second.lastPmClientId = -1;
			if (iter->second.targetClientId == client)
				iter->second.targetClientId = -1;
			++iter;
		}
	}

	/** @ingroup Message
	 * @brief When a char selects a target and the target is a player ship then record the target's client.
	 */
	void SetTarget(const ClientId& uClientId, struct XSetTarget const& p2)
	{
		// The iSpaceId *appears* to represent a player ship Id when it is
		// targeted but this might not be the case. Also note that
		// GetClientIdByShip returns 0 on failure not -1.
		const auto targetClientId = Hk::Client::GetClientIdByShip(p2.iSpaceId);
		if (targetClientId.has_value())
		{
			const auto iter = global->info.find(uClientId);
			if (iter != global->info.end())
			{
				iter->second.targetClientId = targetClientId.value();
			}
		}
	}

	/** @ingroup Message
	 * @brief Hook on SubmitChat. Suppresses swearing. Records the last user to PM.
	 */
	bool SubmitChat(const ClientId& client, const unsigned long& msgSize, const void** rdlReader, const uint& cIdTo, const int [[maybe_unused]])
	{
		// Ignore group join/leave commands
		if (cIdTo == 0x10004)
			return false;

		// Extract text from rdlReader
		BinaryRDLReader rdl;
		wchar_t wszBuf[1024];
		uint iRet1;
		rdl.extract_text_from_buffer((unsigned short*)wszBuf, sizeof(wszBuf), iRet1, (const char*)*rdlReader, msgSize);

		const std::wstring wscChatMsg = ToLower(wszBuf);

		if (const bool isGroup = (cIdTo == 0x10003 || !wscChatMsg.find(L"/g ") || !wscChatMsg.find(L"/group ")); !isGroup)
		{
			// If a restricted word appears in the message take appropriate action.
			for (const auto& word : global->config->swearWords)
			{
				if (wscChatMsg.find(word) != -1)
				{
					PrintUserCmdText(client, L"This is an automated message.");
					PrintUserCmdText(client, L"Please do not swear or you may be sanctioned.");

					global->info[client].swearWordWarnings++;
					if (global->info[client].swearWordWarnings > 2)
					{
						const std::wstring wscCharname = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));
						AddLog(LogType::Kick,
						    LogLevel::Info,
						    wstos(std::format(L"Swearing tempban on {} ({}) reason='{}'",
						        wscCharname,
						        Hk::Client::GetAccountID(Hk::Client::GetAccountByCharName(wscCharname).value()).value(),
						        wscChatMsg.c_str())));

						if (global->tempBanCommunicator)
							global->tempBanCommunicator->TempBan(wscCharname, 10);

						Hk::Player::DelayedKick(client, 1);

						if (global->config->disconnectSwearingInSpaceRange > 0.0f)
						{
							std::wstring wscMsg = global->config->disconnectSwearingInSpaceMsg;
							wscMsg = ReplaceStr(wscMsg, L"%time", GetTimeString(FLHookConfig::i()->general.dieMsg));
							wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
							PrintLocalUserCmdText(client, wscMsg, global->config->disconnectSwearingInSpaceRange);
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
			const auto iter = global->info.find(cIdTo);
			if (iter != global->info.end())
			{
				iter->second.lastPmClientId = client;
			}
		}
		return false;
	}

	/** @ingroup Message
	 * @brief Prints RedText in the style of New Player messages.
	 */
	void RedText(const std::wstring& wscXMLMsg, uint iSystemId)
	{
		char szBuf[0x1000];
		uint iRet;
		if (const auto err = Hk::Message::FMsgEncodeXML(wscXMLMsg, szBuf, sizeof(szBuf), iRet); err.has_error())
			return;

		// Send to all players in system
		PlayerData* playerData = Players.traverse_active(nullptr);
		while (playerData)
		{
			ClientId client = playerData->iOnlineId;

			if (SystemId iClientSystemId = Hk::Player::GetSystem(client).value(); iSystemId == iClientSystemId)
				Hk::Message::FMsgSendChat(client, szBuf, iRet);

			playerData = Players.traverse_active(playerData);
		}
	}

	/** @ingroup Message
	 * @brief When a chat message is sent to a client and this client has showchattime on insert the time on the line immediately before the chat message
	 */
	bool SendChat(ClientId& client, const uint& msgTarget, const uint& msgSize, void** rdlReader)
	{
		// Return immediately if the chat line is the time.
		if (global->sendingTime)
			return false;

		// Ignore group messages (I don't know if they ever get here
		if (msgTarget == 0x10004)
			return false;

		if (global->config->suppressMistypedCommands)
		{
			// Extract text from rdlReader
			BinaryRDLReader rdl;
			wchar_t wszBuf[1024];
			uint iRet1;
			const void* rdlReader2 = *rdlReader;
			rdl.extract_text_from_buffer((unsigned short*)wszBuf, sizeof(wszBuf), iRet1, (const char*)rdlReader2, msgSize);
			std::wstring wscChatMsg = wszBuf;

			// Find the ': ' which indicates the end of the sending player name.
			const size_t iTextStartPos = wscChatMsg.find(L": ");
			if (iTextStartPos != std::string::npos
			&& ((wscChatMsg.find(L": /") == iTextStartPos && wscChatMsg.find(L": //") != iTextStartPos) || wscChatMsg.find(L": .") == iTextStartPos))
			{
				return true;
			}
		}

		if (global->info[client].showChatTime)
		{
			// Send time with gray color (BEBEBE) in small text (90) above the chat
			// line.
			global->sendingTime = true;
			Hk::Message::FMsg(client, L"<TRA data=\"0xBEBEBE90\" mask=\"-1\"/><TEXT>" + XMLText(GetTimeString(FLHookConfig::i()->general.dieMsg)) + L"</TEXT>");
			global->sendingTime = false;
		}
		return false;
	}

	/** @ingroup Message
	 * @brief Set a preset message
	 */
	void UserCmd_SetMsg(ClientId& client, const std::wstring& param)
	{
		if (!global->config->enableSetMessage)
		{
			PrintUserCmdText(client, L"Set commands disabled");
			return;
		}

		const int iMsgSlot = ToInt(GetParam(param, ' ', 0));
		const std::wstring wscMsg = GetParamToEnd(ViewToWString(param), ' ', 1);

		if (iMsgSlot < 0 || iMsgSlot > 9 || param.size() == 0)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /setmsg <n> <msg text>");
			return;
		}

		Hk::Ini::SetCharacterIni(client, L"msg." + std::to_wstring(iMsgSlot), ViewToWString(wscMsg));

		// Update the character cache
		global->info[client].slot[iMsgSlot] = wscMsg;
		PrintUserCmdText(client, L"OK");
	}

	/** @ingroup Message
	 * @brief Show preset messages
	 */
	void UserCmd_ShowMsgs(ClientId& client, const std::wstring& param)
	{
		if (!global->config->enableSetMessage)
		{
			PrintUserCmdText(client, L"Set commands disabled");
			return;
		}
		const auto iter = global->info.find(client);
		if (iter == global->info.end())
		{
			PrintUserCmdText(client, L"ERR No messages");
			return;
		}

		for (int i = 0; i < numberOfSlots; i++)
		{
			PrintUserCmdText(client, std::format(L"{}: {}", i, iter->second.slot[i]));
		}
		PrintUserCmdText(client, L"OK");
	}

	/** @ingroup Message
	 * @brief User Commands for /r0-9
	 */
	void UserCmd_RMsg(ClientId& client, const std::wstring& param)
	{
		long num = GetNumberFromCmd(param);
		SendPresetLastPMSender(client, num);
	}

	/** @ingroup Message
	 * @brief User Commands for /s0-9
	 */
	void UserCmd_SMsg(ClientId& client, const std::wstring& param)
	{
		long num = GetNumberFromCmd(param);
		SendPresetSystemMessage(client, num);
	}

	/** @ingroup Message
	 * @brief User Commands for /l0-9
	 */
	void UserCmd_LMsg(ClientId& client, const std::wstring& param)
	{
		long num = GetNumberFromCmd(param);
		SendPresetLocalMessage(client, num);
	}

	/** @ingroup Message
	 * @brief User Commands for /g0-9
	 */
	void UserCmd_GMsg(ClientId& client, const std::wstring& param)
	{
		long num = GetNumberFromCmd(param);
		SendPresetGroupMessage(client, num);
	}

	/** @ingroup Message
	 * @brief User Commands for /t0-9
	 */
	void UserCmd_TMsg(ClientId& client, const std::wstring& param)
	{
		long num = GetNumberFromCmd(param);
		SendPresetToLastTarget(client, num);
	}

	/** @ingroup Message
	 * @brief Send an message to the last person that PM'd this client.
	 */
	void UserCmd_ReplyToLastPMSender(ClientId& client, const std::wstring& param)
	{
		const auto iter = global->info.find(client);
		if (iter == global->info.end())
		{
			// There's no way for this to happen! yeah right.
			PrintUserCmdText(client, L"ERR No message defined");
			return;
		}

		const std::wstring wscMsg = GetParamToEnd(param, ' ', 0);

		if (iter->second.lastPmClientId == -1)
		{
			PrintUserCmdText(client, L"ERR PM sender not available");
			return;
		}

		global->info[iter->second.lastPmClientId].lastPmClientId = client;
		Hk::Message::SendPrivateChat(client, iter->second.lastPmClientId, ViewToWString(wscMsg));
	}

	/** @ingroup Message
	 * @brief Shows the sender of the last PM and the last char targeted
	 */
	void UserCmd_ShowLastPMSender(ClientId& client, const std::wstring& param)
	{
		const auto iter = global->info.find(client);
		if (iter == global->info.end())
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
	void UserCmd_SendToLastTarget(ClientId& client, const std::wstring& param)
	{
		const auto iter = global->info.find(client);
		if (iter == global->info.end())
		{
			// There's no way for this to happen! yeah right.
			PrintUserCmdText(client, L"ERR No message defined");
			return;
		}

		const std::wstring wscMsg = GetParamToEnd(param, ' ', 0);

		if (iter->second.targetClientId == -1)
		{
			PrintUserCmdText(client, L"ERR PM target not available");
			return;
		}

		global->info[iter->second.targetClientId].lastPmClientId = client;
		Hk::Message::SendPrivateChat(client, iter->second.targetClientId, ViewToWString(wscMsg));
	}

	/** @ingroup Message
	 * @brief Send a private message to the specified charname. If the player is offline the message will be delivery when they next login.
	 */
	void UserCmd_PrivateMsg(ClientId& client, const std::wstring& param)
	{
		const std::wstring usage = L"Usage: /privatemsg <charname> <messsage> or /pm ...";
		const std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
		const std::wstring& targetCharname = GetParam(param, ' ', 0);
		const std::wstring msg = GetParamToEnd(param, ' ', 1);

		if (charname.size() == 0 || msg.size() == 0)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, usage);
			return;
		}

		if (!Hk::Client::GetAccountByCharName(targetCharname))
		{
			PrintUserCmdText(client, L"ERR charname does not exist");
			return;
		}

		const auto clientId = Hk::Client::GetClientIdFromCharName(targetCharname);
		if (clientId.has_error())
		{
			MailManager::MailItem item;
			item.author = wstos(charname);
			item.subject = "Private Message";
			item.body = wstos(msg);
			MailManager::i()->SendNewMail(targetCharname, item);
			MailManager::i()->SendMailNotification(targetCharname);
			PrintUserCmdText(client, L"OK message saved to mailbox");
		}
		else
		{
			global->info[clientId.value()].lastPmClientId = client;
			Hk::Message::SendPrivateChat(client, clientId.value(), ViewToWString(msg));
		}
	}

	/** @ingroup Message
	 * @brief Send a private message to the specified clientid.
	 */
	void UserCmd_PrivateMsgID(ClientId& client, const std::wstring& param)
	{
		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(client);
		const std::wstring& wscClientId = GetParam(param, ' ', 0);
		const std::wstring wscMsg = GetParamToEnd(param, ' ', 1);

		uint iToClientId = ToInt(wscClientId);
		if (!Hk::Client::IsValidClientID(iToClientId) || Hk::Client::IsInCharSelectMenu(iToClientId))
		{
			PrintUserCmdText(client, L"ERR Invalid client-id");
			return;
		}

		global->info[iToClientId].lastPmClientId = client;
		Hk::Message::SendPrivateChat(client, iToClientId, ViewToWString(wscMsg));
	}

	/** @ingroup Message
	 * @brief Send a message to all players with a particular prefix.
	 */
	void UserCmd_FactionMsg(ClientId& client, const std::wstring& param)
	{
		std::wstring wscSender = (const wchar_t*)Players.GetActiveCharacterName(client);
		const std::wstring& wscCharnamePrefix = GetParam(param, ' ', 0);
		const std::wstring& wscMsg = GetParamToEnd(param, ' ', 1);

		if (wscCharnamePrefix.size() < 3 || wscMsg.size() == 0)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /factionmsg <tag> <message> or /fm ...");
			return;
		}

		bool bSenderReceived = false;
		bool bMsgSent = false;
		for (const auto& player : Hk::Admin::GetPlayers())
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
	void UserCmd_FactionInvite(ClientId& client, const std::wstring& param)
	{
		const std::wstring& wscCharnamePrefix = GetParam(param, ' ', 0);

		bool msgSent = false;

		if (wscCharnamePrefix.size() < 3)
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /factioninvite <tag> or /fi ...");
			return;
		}

		for (const auto& player : Hk::Admin::GetPlayers())
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
	void UserCmd_SetChatTime(ClientId& client, const std::wstring& param)
	{
		const std::wstring param1 = ToLower(GetParam(param, ' ', 0));
		bool bShowChatTime = false;
		if (!param1.compare(L"on"))
			bShowChatTime = true;
		else if (!param1.compare(L"off"))
			bShowChatTime = false;
		else
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /set chattime [on|off]");
		}

		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(client);

		Hk::Ini::SetCharacterIni(client, L"msg.chat_time", bShowChatTime ? L"true" : L"false");

		// Update the client cache.
		if (const auto iter = global->info.find(client); iter != global->info.end())
			iter->second.showChatTime = bShowChatTime;

		// Send confirmation msg
		PrintUserCmdText(client, L"OK");
	}

	/** @ingroup Message
	 * @brief Prints the current server time.
	 */
	void UserCmd_Time(ClientId& client, const std::wstring& param)
	{
		// Send time with gray color (BEBEBE) in small text (90) above the chat line.
		PrintUserCmdText(client, GetTimeString(FLHookConfig::i()->general.localTime));
	}

	/** @ingroup Message
	 * @brief Me command allow players to type "/me powers his engines" which would print: "Trent powers his engines"
	 */
	void UserCmd_Me(ClientId& client, const std::wstring& param)
	{
		if (global->config->enableMe)
		{
			const std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
			SystemId iSystemId = Hk::Player::GetSystem(client).value();

			// Encode message using the death message style (red text).
			std::wstring wscXMLMsg = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.deathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
			wscXMLMsg += charname + L" ";
			wscXMLMsg += XMLText(ViewToWString(GetParamToEnd(param, ' ', 0)));
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
	void UserCmd_Do(ClientId& client, const std::wstring& param)
	{
		if (global->config->enableDo)
		{
			SystemId iSystemId = Hk::Player::GetSystem(client).value();

			// Encode message using the death message style (red text).
			std::wstring wscXMLMsg = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.deathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
			wscXMLMsg += XMLText(ViewToWString(GetParamToEnd(ViewToWString(param), ' ', 0)));
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
	    CreateUserCommand(
	        CmdArr({L"/0", L"/1", L"/2", L"/3", L"/4", L"/5", L"/6", L"/7", L"/8", L"/9"}), L"/<msgNumber>", UserCmd_SMsg, L"Send preset message from slot [0-9]."),
	    CreateUserCommand(CmdArr({L"/l0", L"/l1", L"/l2", L"/l3", L"/l4", L"/l5", L"/l6", L"/l7", L"/l8", L"/l9"}), L"/l<msgNumber>", UserCmd_LMsg,
	        L"Send preset message to local chat from slot [0-9]."),
	    CreateUserCommand(CmdArr({L"/r0", L"/r1", L"/r2", L"/r3", L"/r4", L"/r5", L"/r6", L"/r7", L"/r8", L"/r9"}), L"/r<msgNumber>", UserCmd_RMsg,
	        L"Send preset message to local chat from slot [0-9]."),
	    CreateUserCommand(CmdArr({L"/g0", L"/g1", L"/g2", L"/g3", L"/g4", L"/g5", L"/g6", L"/g7", L"/g8", L"/g9"}), L"/g<msgNumber>", UserCmd_GMsg,
	        L"Send preset message to group chat from slot [0-9]."),
	    CreateUserCommand(CmdArr({L"/t0", L"/t1", L"/t2", L"/t3", L"/t4", L"/t5", L"/t6", L"/t7", L"/t8", L"/t9"}), L"/t<msgNumber>", UserCmd_TMsg,
	        L"Send preset message to target from slot [0-9]."),
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
REFL_AUTO(type(Config), field(greetingBannerLines), field(specialBannerLines), field(standardBannerLines), field(specialBannerTimeout),
    field(standardBannerTimeout), field(customHelp), field(suppressMistypedCommands), field(enableSetMessage), field(enableMe), field(enableDo),
    field(disconnectSwearingInSpaceMsg), field(disconnectSwearingInSpaceRange), field(swearWords))

DefaultDllMainSettings(LoadSettings);

    // Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Message");
	pi->shortName("message");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__SubmitChat, &SubmitChat);
	pi->emplaceHook(HookedCall::IChat__SendChat, &SendChat);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__SetTarget, &SetTarget);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &PlayerLogin, HookStep::After);
}