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

#include "Message.h"
#include "Features/Mail.hpp"
#include "Features/TempBan.hpp"

namespace Plugins::Message
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	long GetNumberFromCmd(const std::wstring& param)
	{
		const auto numStr = param[2];
		wchar_t* _;
		return std::wcstol(&numStr, &_, 10);
	}

	/** @ingroup Message
	 * @brief Load the msgs for specified client Id into memory.
	 */
	static void LoadMsgs(ClientId client)
	{
		// Load from disk the messages.
		for (int msgSlot = 0; msgSlot < NumberOfSlots; msgSlot++)
		{
			global->info[client].slots[msgSlot] = Hk::Ini::GetCharacterIniString(client, L"msg." + std::to_wstring(msgSlot));
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
				Hk::Chat::FMsg(client, line);
			else
				client.Message(line);
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
		PlayerData* playerData = nullptr;
		while ((playerData = Players.traverse_active(playerData)))
		{
			ClientId client = playerData->onlineId;
			for (const auto& line : global->config->specialBannerLines)
			{
				if (line.find(L"<TRA") == 0)
					Hk::Chat::FMsg(client, line);
				else
					client.Message(line);
			}
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

		PlayerData* playerData = nullptr;
		while ((playerData = Players.traverse_active(playerData)))
		{
			ClientId client = playerData->onlineId;

			if (global->config->standardBannerLines[curStandardBanner].find(L"<TRA") == 0)
				Hk::Chat::FMsg(client, global->config->standardBannerLines[curStandardBanner]);
			else
				client.Message(global->config->standardBannerLines[curStandardBanner].c_str());
		}
	}

	/** @ingroup Message
	 * @brief Replace #t and #c tags with current target name and current ship location. Return false if tags cannot be replaced.
	 */
	static bool ReplaceMessageTags(ClientId client, const ClientInfo& clientData, std::wstring& msg)
	{
		if (msg.find(L"#t") != -1)
		{
			if (clientData.targetClientId == UINT_MAX)
			{
				client.Message(L"ERR Target not available");
				return false;
			}

			const auto targetName = clientData.targetClientId.GetCharacterName();
			msg = StringUtils::ReplaceStr(msg, L"#t", targetName.value());
		}

		if (msg.find(L"#c") != -1)
		{
			const auto location = Hk::Solar::GetLocation(client, IdType::Client);
			const auto system = Hk::Player::GetSystem(client);
			const Vector& position = location.value().first;
			if (location.has_value() && system.has_value())
			{
				const std::wstring curLocation = Hk::Math::VectorToSectorCoord<std::wstring>(system.value(), position);
				msg = StringUtils::ReplaceStr(msg, L"#c", curLocation);
			}
			else
			{
				client.Message(L"ERR Target not available");
				return false;
			}
		}

		return true;
	}

	/** @ingroup Message
	 * @brief Returns a string with the preset message
	 */
	std::wstring GetPresetMessage(ClientId client, int msgSlot)
	{
		const auto iter = global->info.find(client);
		if (iter == global->info.end() || iter->second.slots[msgSlot].empty())
		{
			client.Message(L"ERR No message defined");
			return L"";
		}

		// Replace the tag #t with name of the targeted player.
		std::wstring msg = iter->second.slots[msgSlot];
		if (!ReplaceMessageTags(client, iter->second, msg))
			return L"";

		return msg;
	}

	/** @ingroup Message
	 * @brief Send an preset message to the local system chat
	 */
	void SendPresetLocalMessage(ClientId client, int msgSlot)
	{
		if (!global->config->enableSetMessage)
		{
			client.Message(L"Set commands disabled");
			return;
		}
		if (msgSlot < 0 || msgSlot > 9)
		{
			client.Message(L"ERR Invalid parameters");
			client.Message(L"Usage: /ln (n=0-9)");
			return;
		}

		Hk::Chat::SendLocalSystemChat(client, GetPresetMessage(client, msgSlot));
	}

	/** @ingroup Message
	 * @brief Send a preset message to the last/current target.
	 */
	void SendPresetToLastTarget(ClientId client, int msgSlot)
	{
		if (!global->config->enableSetMessage)
		{
			client.Message(L"Set commands disabled");
			return;
		}
		UserCmd_SendToLastTarget(client, GetPresetMessage(client, msgSlot));
	}

	/** @ingroup Message
	 * @brief Send an preset message to the system chat
	 */
	void SendPresetSystemMessage(ClientId client, int msgSlot)
	{
		if (!global->config->enableSetMessage)
		{
			client.Message(L"Set commands disabled");
			return;
		}
		Hk::Chat::SendSystemChat(client, GetPresetMessage(client, msgSlot));
	}

	/** @ingroup Message
	 * @brief Send an preset message to the last PM sender
	 */
	void SendPresetLastPMSender(ClientId& client, int msgSlot)
	{
		if (!global->config->enableSetMessage)
		{
			client.Message(L"Set commands disabled");
			return;
		}

		UserCmd_ReplyToLastPMSender(client, GetPresetMessage(client, msgSlot));
	}

	/** @ingroup Message
	 * @brief Send an preset message to the group chat
	 */
	void SendPresetGroupMessage(ClientId& client, int msgSlot)
	{
		if (!global->config->enableSetMessage)
		{
			client.Message(L"Set commands disabled");
			return;
		}

		Hk::Chat::SendGroupChat(client, GetPresetMessage(client, msgSlot));
	}

	/** @ingroup Message
	 * @brief Clean up when a client disconnects
	 */
	void ClearClientInfo(ClientId& client)
	{
		global->info.erase(client);
	}

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
	void DisConnect(ClientId& client, const EFLConnection [[maybe_unused]])
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
	void SetTarget(const ClientId& uClientId, XSetTarget const& p2)
	{
		// The spaceId *appears* to represent a player ship Id when it is
		// targeted but this might not be the case. Also note that
		// GetClientIdByShip returns 0 on failure not -1.
		const auto targetClientId = Hk::Client::GetClientIdByShip(p2.spaceId);
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
		wchar_t wBuf[1024];
		uint ret1;
		rdl.extract_text_from_buffer((unsigned short*)wBuf, sizeof(wBuf), ret1, (const char*)*rdlReader, msgSize);

		const std::wstring chatMsg = StringUtils::ToLower(wBuf);

		if (const bool isGroup = (cIdTo == 0x10003 || !chatMsg.find(L"/g ") || !chatMsg.find(L"/group ")); !isGroup)
		{
			// If a restricted word appears in the message take appropriate action.
			for (const auto& word : global->config->swearWords)
			{
				if (chatMsg.find(word) != -1)
				{
					client.Message(L"This is an automated message.");
					client.Message(L"Please do not swear or you may be sanctioned.");

					global->info[client].swearWordWarnings++;
					if (global->info[client].swearWordWarnings > 2)
					{
						const std::wstring charname = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(client));
						Logger::i()->Log(
						    LogLevel::Info,
						    StringUtils::wstos(std::format(L"Swearing tempban on {} ({}) reason='{}'",
						        charname,
						        Hk::Client::GetAccountID(Hk::Client::GetAccountByCharName(charname).value()).value(),
						        chatMsg.c_str())));

						TempBanManager::i()->AddTempBan(client,
						    global->config->swearingTempBanDuration,
						    std::format(L"Swearing tempban for {} minutes.", global->config->swearingTempBanDuration));

						if (global->config->disconnectSwearingInSpaceRange > 0.0f)
						{
							std::wstring msg = global->config->disconnectSwearingInSpaceMsg;
							msg = StringUtils::ReplaceStr(msg, L"%time", GetTimeString(FLHook::GetConfig()->messages.dieMsg));
							msg = StringUtils::ReplaceStr(msg, L"%player", charname);
							PrintLocalUserCmdText(client, msg, global->config->disconnectSwearingInSpaceRange);
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
	void RedText(const std::wstring& XMLMsg, uint systemId)
	{
		char buf[0x1000];
		uint retVal;
		if (const auto err = Hk::Chat::FMsgEncodeXML(XMLMsg, buf, sizeof(buf), retVal); err.has_error())
			return;

		// Send to all players in system
		PlayerData* playerData = nullptr;
		while ((playerData = Players.traverse_active(playerData)))
		{
			ClientId client = playerData->onlineId;

			if (SystemId clientSystemId = Hk::Player::GetSystem(client).value(); systemId == clientSystemId)
				Hk::Chat::FMsgSendChat(client, buf, retVal);
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
			wchar_t buf[1024];
			uint ret1;
			const void* rdlReader2 = *rdlReader;
			rdl.extract_text_from_buffer((unsigned short*)buf, sizeof(buf), ret1, (const char*)rdlReader2, msgSize);
			std::wstring chatMsg = buf;

			// Find the ': ' which indicates the end of the sending player name.
			const size_t textStartPos = chatMsg.find(L": ");
			if (textStartPos != std::string::npos &&
			    ((chatMsg.find(L": /") == textStartPos && chatMsg.find(L": //") != textStartPos) || chatMsg.find(L": .") == textStartPos))
			{
				return true;
			}
		}

		if (global->info[client].showChatTime)
		{
			// Send time with gray color (BEBEBE) in small text (90) above the chat
			// line.
			global->sendingTime = true;
			Hk::Chat::FMsg(
			    client, L"<TRA data=\"0xBEBEBE90\" mask=\"-1\"/><TEXT>" + StringUtils::XmlText(GetTimeString(FLHook::GetConfig()->messages.dieMsg)) + L"</TEXT>");
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
			client.Message(L"Set commands disabled");
			return;
		}

		const int msgSlot = StringUtils::Cast<int>(GetParam(param, ' ', 0));
		const std::wstring msg = GetParamToEnd(ViewToWString(param), ' ', 1);

		if (msgSlot < 0 || msgSlot > 9 || msg.empty())
		{
			client.Message(L"ERR Invalid parameters");
			client.Message(L"Usage: /setmsg <n> <msg text>");
			return;
		}

		Hk::Ini::SetCharacterIni(client, L"msg." + std::to_wstring(msgSlot), ViewToWString(msg));

		// Update the character cache
		global->info[client].slots[msgSlot] = msg;
		client.Message(L"OK");
	}

	/** @ingroup Message
	 * @brief Show preset messages
	 */
	void UserCmd_ShowMsgs(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		if (!global->config->enableSetMessage)
		{
			client.Message(L"Set commands disabled");
			return;
		}
		const auto iter = global->info.find(client);
		if (iter == global->info.end())
		{
			client.Message(L"ERR No messages");
			return;
		}

		for (int i = 0; i < NumberOfSlots; i++)
		{
			client.Message(std::format(L"{}: {}", i, iter->second.slots[i]));
		}
		client.Message(L"OK");
	}

	/** @ingroup Message
	 * @brief User Commands for /0-9
	 */
	void UserCmd_Msg(ClientId& client, const std::wstring& param)
	{
		const auto numStr = param[1];
		wchar_t* _;
		const long num = std::wcstol(&numStr, &_, 10);
		if (FLHook::GetConfig()->messages.defaultLocalChat)
		{
			SendPresetLocalMessage(client, num);
		}
		else
		{
			SendPresetSystemMessage(client, num);
		}
	}

	/** @ingroup Message
	 * @brief User Commands for /r0-9
	 */
	void UserCmd_RMsg(ClientId& client, const std::wstring& param)
	{
		const long num = GetNumberFromCmd(param);
		SendPresetLastPMSender(client, num);
	}

	/** @ingroup Message
	 * @brief User Commands for /s0-9
	 */
	void UserCmd_SMsg(ClientId& client, const std::wstring& param)
	{
		const long num = GetNumberFromCmd(param);
		SendPresetSystemMessage(client, num);
	}

	/** @ingroup Message
	 * @brief User Commands for /l0-9
	 */
	void UserCmd_LMsg(ClientId& client, const std::wstring& param)
	{
		const long num = GetNumberFromCmd(param);
		SendPresetLocalMessage(client, num);
	}

	/** @ingroup Message
	 * @brief User Commands for /g0-9
	 */
	void UserCmd_GMsg(ClientId& client, const std::wstring& param)
	{
		const long num = GetNumberFromCmd(param);
		SendPresetGroupMessage(client, num);
	}

	/** @ingroup Message
	 * @brief User Commands for /t0-9
	 */
	void UserCmd_TMsg(ClientId& client, const std::wstring& param)
	{
		const long num = GetNumberFromCmd(param);
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
			client.Message(L"ERR No message defined");
			return;
		}

		const std::wstring msg = GetParamToEnd(param, ' ', 0);

		if (iter->second.lastPmClientId == UINT_MAX)
		{
			client.Message(L"ERR PM sender not available");
			return;
		}

		global->info[iter->second.lastPmClientId].lastPmClientId = client;
		Hk::Chat::SendPrivateChat(client, iter->second.lastPmClientId, ViewToWString(msg));
	}

	/** @ingroup Message
	 * @brief Shows the sender of the last PM and the last char targeted
	 */
	void UserCmd_ShowLastPMSender(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		const auto iter = global->info.find(client);
		if (iter == global->info.end())
		{
			// There's no way for this to happen! yeah right.
			client.Message(L"ERR No message defined");
			return;
		}

		std::wstring senderCharname = L"<not available>" + std::to_wstring(iter->second.lastPmClientId);
		if (iter->second.lastPmClientId != -1 && Hk::Client::IsValidClientID(iter->second.lastPmClientId))
			senderCharname = (const wchar_t*)Players.GetActiveCharacterName(iter->second.lastPmClientId);

		std::wstring targetCharname = L"<not available>" + std::to_wstring(iter->second.targetClientId);
		if (iter->second.targetClientId != -1 && Hk::Client::IsValidClientID(iter->second.targetClientId))
			targetCharname = (const wchar_t*)Players.GetActiveCharacterName(iter->second.targetClientId);

		client.Message(L"OK sender=" + senderCharname + L" target=" + targetCharname);
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
			client.Message(L"ERR No message defined");
			return;
		}

		const std::wstring msg = GetParamToEnd(param, ' ', 0);

		if (iter->second.targetClientId == UINT_MAX)
		{
			client.Message(L"ERR PM target not available");
			return;
		}

		global->info[iter->second.targetClientId].lastPmClientId = client;
		Hk::Chat::SendPrivateChat(client, iter->second.targetClientId, ViewToWString(msg));
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
			client.Message(L"ERR Invalid parameters");
			client.Message(usage);
			return;
		}

		if (!Hk::Client::GetAccountByCharName(targetCharname))
		{
			client.Message(L"ERR charname does not exist");
			return;
		}

		const auto clientId = Hk::Client::GetClientIdFromCharName(targetCharname);
		if (clientId.has_error())
		{
			MailManager::MailItem item;
			item.author = StringUtils::wstos(charname);
			item.subject = "Private Message";
			item.body = StringUtils::wstos(msg);
			MailManager::i()->SendNewMail(targetCharname, item);
			MailManager::i()->SendMailNotification(targetCharname);
			client.Message(L"OK message saved to mailbox");
		}
		else
		{
			global->info[clientId.value()].lastPmClientId = client;
			Hk::Chat::SendPrivateChat(client, clientId.value(), ViewToWString(msg));
		}
	}

	/** @ingroup Message
	 * @brief Send a private message to the specified clientid.
	 */
	void UserCmd_PrivateMsgID(ClientId& client, const std::wstring& param)
	{
		std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
		const std::wstring& clientId = GetParam(param, ' ', 0);
		const std::wstring msg = GetParamToEnd(param, ' ', 1);

		const uint toClientId = StringUtils::Cast<int>(clientId);
		if (!Hk::Client::IsValidClientID(toClientId) || Hk::Client::IsInCharSelectMenu(toClientId))
		{
			client.Message(L"ERR Invalid client-id");
			return;
		}

		global->info[toClientId].lastPmClientId = client;
		Hk::Chat::SendPrivateChat(client, toClientId, ViewToWString(msg));
	}

	/** @ingroup Message
	 * @brief Send a message to all players with a particular prefix.
	 */
	void UserCmd_FactionMsg(ClientId& client, const std::wstring& param)
	{
		const std::wstring sender = (const wchar_t*)Players.GetActiveCharacterName(client);
		const std::wstring& charnamePrefix = GetParam(param, ' ', 0);
		const std::wstring& msg = GetParamToEnd(param, ' ', 1);

		if (charnamePrefix.size() < 3 || msg.empty())
		{
			client.Message(L"ERR Invalid parameters");
			client.Message(L"Usage: /factionmsg <tag> <message> or /fm ...");
			return;
		}

		bool senderReceived = false;
		bool msgSent = false;
		for (const auto& player : Hk::Admin::GetPlayers())
		{
			if (StringUtils::ToLower(player.character).find(StringUtils::ToLower(charnamePrefix)) == std::string::npos)
				continue;

			if (player.client == client)
				senderReceived = true;

			Hk::Chat::FormatSendChat(player.client, sender, ViewToWString(msg), L"FF7BFF");
			msgSent = true;
		}
		if (!senderReceived)
			Hk::Chat::FormatSendChat(client, sender, ViewToWString(msg), L"FF7BFF");

		if (msgSent == false)
			client.Message(L"ERR No chars found");
	}

	/** @ingroup Message
	 * @brief User command for enabling the chat timestamps.
	 */
	void UserCmd_SetChatTime(ClientId& client, const std::wstring& param)
	{
		const std::wstring param1 = StringUtils::ToLower(GetParam(param, ' ', 0));
		bool showChatTime = false;
		if (!param1.compare(L"on"))
			showChatTime = true;
		else if (!param1.compare(L"off"))
			showChatTime = false;
		else
		{
			client.Message(L"ERR Invalid parameters");
			client.Message(L"Usage: /set chattime [on|off]");
		}

		std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);

		Hk::Ini::SetCharacterIni(client, L"msg.chat_time", showChatTime ? L"true" : L"false");

		// Update the client cache.
		if (const auto iter = global->info.find(client); iter != global->info.end())
			iter->second.showChatTime = showChatTime;

		// Send confirmation msg
		client.Message(L"OK");
	}

	/** @ingroup Message
	 * @brief Prints the current server time.
	 */
	void UserCmd_Time(ClientId& client, [[maybe_unused]] const std::wstring& param)
	{
		// Send time with gray color (BEBEBE) in small text (90) above the chat line.
		client.Message(GetTimeString(FLHook::GetConfig()->general.localTime));
	}

	/** @ingroup Message
	 * @brief Me command allow players to type "/me powers his engines" which would print: "Trent powers his engines"
	 */
	void UserCmd_Me(ClientId& client, const std::wstring& param)
	{
		if (global->config->enableMe)
		{
			const std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
			SystemId systemId = Hk::Player::GetSystem(client).value();

			// Encode message using the death message style (red text).
			std::wstring XMLMsg = L"<TRA data=\"" + FLHook::GetConfig()->messages.msgStyle.deathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
			XMLMsg += charname + L" ";
			XMLMsg += StringUtils::XmlText(ViewToWString(GetParamToEnd(param, ' ', 0)));
			XMLMsg += L"</TEXT>";

			RedText(XMLMsg, systemId);
		}
		else
		{
			client.Message(L"Command not enabled.");
		}
	}

	/** @ingroup Message
	 * @brief Do command allow players to type "/do Nomad fighters detected" which would print: "Nomad fighters detected" in the standard red text
	 */
	void UserCmd_Do(ClientId& client, const std::wstring& param)
	{
		if (global->config->enableDo)
		{
			SystemId systemId = Hk::Player::GetSystem(client).value();

			// Encode message using the death message style (red text).
			std::wstring XMLMsg = L"<TRA data=\"" + FLHook::GetConfig()->messages.msgStyle.deathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
			XMLMsg += StringUtils::XmlText(ViewToWString(GetParamToEnd(ViewToWString(param), ' ', 0)));
			XMLMsg += L"</TEXT>";

			RedText(XMLMsg, systemId);
		}
		else
		{
			client.Message(L"Command not enabled.");
		}
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/setmsg", L"<n> <msg text>", UserCmd_SetMsg, L"Sets a preset message."),
	    CreateUserCommand(L"/showmsgs", L"", UserCmd_ShowMsgs, L"Show your preset messages."),
	    CreateUserCommand(CmdArr({L"/0", L"/1", L"/2", L"/3", L"/4", L"/5", L"/6", L"/7", L"/8", L"/9"}), L"/<msgNumber>", UserCmd_Msg,
	        L"Send preset message from slot [0-9]."),
	    CreateUserCommand(CmdArr({L"/s0", L"/s1", L"/s2", L"/s3", L"/s4", L"/s5", L"/s6", L"/s7", L"/s8", L"/s9"}), L"/s<msgNumber>", UserCmd_SMsg,
	        L"Send preset message to system chat from slot [0-9]."),
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
    field(disconnectSwearingInSpaceMsg), field(disconnectSwearingInSpaceRange), field(swearWords), field(swearingTempBanDuration))

DefaultDllMainSettings(LoadSettings);

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Message");
	pi->shortName("message");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::V04);
	pi->versionMinor(PluginMinorVersion::V00);
	pi->emplaceHook(HookedCall::IServerImpl__SubmitChat, &SubmitChat);
	pi->emplaceHook(HookedCall::IChat__SendChat, &SendChat);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__SetTarget, &SetTarget);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &PlayerLogin, HookStep::After);
}