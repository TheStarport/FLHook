// Message Plugin - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

namespace Plugins::Message
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** Load the msgs for specified client ID into memory. */
	static void LoadMsgs(uint iClientID)
	{
		if (!global->config->EnableSetMessage)
			return;

		// Load from disk the messages.
		for (int iMsgSlot = 0; iMsgSlot < INFO::NUMBER_OF_SLOTS; iMsgSlot++)
		{
			global->Info[iClientID].slot[iMsgSlot] = HkGetCharacterIniString(iClientID, L"msg." + std::to_wstring(iMsgSlot));
		}

		// Chat time settings.
		global->Info[iClientID].ShowChatTime = HkGetCharacterIniBool(iClientID, L"msg.chat_time");
	}

	/** Show the greeting banner to the specified player */
	static void ShowGreetingBanner(int iClientID)
	{
		if (!global->Info[iClientID].GreetingShown)
		{
			global->Info[iClientID].GreetingShown = true;
			for (auto& line : global->config->GreetingBannerLines)
			{
				if (line.find(L"<TRA") == 0)
					HkFMsg(iClientID, line);
				else
					PrintUserCmdText(iClientID, L"%s", line.c_str());
			}
		}
	}

	/** Show the special banner to all players. */
	static void ShowSpecialBanner()
	{
		struct PlayerData* pPD = 0;
		while (pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);
			for (auto& line : global->config->SpecialBannerLines)
			{
				if (line.find(L"<TRA") == 0)
					HkFMsg(iClientID, line);
				else
					PrintUserCmdText(iClientID, L"%s", line.c_str());
			}
		}
	}

	/** Show the next standard banner to all players. */
	static void ShowStandardBanner()
	{
		if (global->config->StandardBannerLines.size() == 0)
			return;

		static size_t iCurStandardBanner = 0;
		if (++iCurStandardBanner >= global->config->StandardBannerLines.size())
			iCurStandardBanner = 0;

		auto& lstStandardBannerSection = global->config->StandardBannerLines[iCurStandardBanner];

		struct PlayerData* pPD = 0;
		while (pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);

			for (auto& sec : lstStandardBannerSection)
			{
				if (sec.find(L"<TRA") == 0)
					HkFMsg(iClientID, sec);
				else
					PrintUserCmdText(iClientID, L"%s", sec.c_str());
			}
		}
	}

	/** Replace #t and #c tags with current target name and current ship location.
	Return false if tags cannot be replaced. */
	static bool ReplaceMessageTags(uint iClientID, INFO& clientData, std::wstring& wscMsg)
	{
		if (wscMsg.find(L"#t") != -1)
		{
			if (clientData.TargetClientID == -1)
			{
				PrintUserCmdText(iClientID, L"ERR Target not available");
				return false;
			}

			std::wstring wscTargetName = (const wchar_t*)Players.GetActiveCharacterName(clientData.TargetClientID);
			wscMsg = ReplaceStr(wscMsg, L"#t", wscTargetName);
		}

		if (wscMsg.find(L"#c") != -1)
		{
			std::wstring wscCurrLocation = GetLocation(iClientID);
			wscMsg = ReplaceStr(wscMsg, L"#c", wscCurrLocation.c_str());
		}

		return true;
	}

	std::wstring GetPresetMessage(uint iClientID, int iMsgSlot)
	{
		auto iter = global->Info.find(iClientID);
		if (iter == global->Info.end() || iter->second.slot[iMsgSlot].size() == 0)
		{
			PrintUserCmdText(iClientID, L"ERR No message defined");
			return L"";
		}

		// Replace the tag #t with name of the targeted player.
		std::wstring wscMsg = iter->second.slot[iMsgSlot];
		if (!ReplaceMessageTags(iClientID, iter->second, wscMsg))
			return L"";

		return wscMsg;
	}

	/** Send an preset message to the local system chat */
	void SendPresetLocalMessage(uint iClientID, int iMsgSlot)
	{
		if (!global->config->EnableSetMessage)
			return;

		if (iMsgSlot < 0 || iMsgSlot > 9)
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /ln (n=0-9)");
			return;
		}

		SendLocalSystemChat(iClientID, GetPresetMessage(iClientID, iMsgSlot));
	}

	/** Send a preset message to the last/current target. */
	void SendPresetToLastTarget(uint iClientID, int iMsgSlot)
	{
		if (!global->config->EnableSetMessage)
			return;

		UserCmd_SendToLastTarget(iClientID, GetPresetMessage(iClientID, iMsgSlot));
	}

	/** Send an preset message to the system chat */
	void SendPresetSystemMessage(uint iClientID, int iMsgSlot)
	{
		if (!global->config->EnableSetMessage)
			return;

		SendSystemChat(iClientID, GetPresetMessage(iClientID, iMsgSlot));
	}

	/** Send an preset message to the last PM sender */
	void SendPresetLastPMSender(uint iClientID, int iMsgSlot, std::wstring wscMsg)
	{
		if (!global->config->EnableSetMessage)
			return;

		UserCmd_ReplyToLastPMSender(iClientID, GetPresetMessage(iClientID, iMsgSlot));
	}

	/** Send an preset message to the group chat */
	void SendPresetGroupMessage(uint iClientID, int iMsgSlot)
	{
		if (!global->config->EnableSetMessage)
			return;

		SendGroupChat(iClientID, GetPresetMessage(iClientID, iMsgSlot));
	}

	/** Clean up when a client disconnects */
	void ClearClientInfo(uint& iClientID) { global->Info.erase(iClientID); }

	/**
	This function is called when the admin command rehash is called and when the
	module is loaded.
	*/
	void LoadSettings()
	{
		// For every active player load their msg settings.
		std::list<HKPLAYERINFO> players = HkGetPlayers();
		for (auto& p : players)
			LoadMsgs(p.iClientID);

		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);
	}

	/// On this timer display banners
	void Timer()
	{
		static int iSpecialBannerTimer = 0;
		static int iStandardBannerTimer = 0;

		if (++iSpecialBannerTimer > global->config->SpecialBannerTimeout)
		{
			iSpecialBannerTimer = 0;
			ShowSpecialBanner();
		}

		if (++iStandardBannerTimer > global->config->StandardBannerTimeout)
		{
			iStandardBannerTimer = 0;
			ShowStandardBanner();
		}
	}

	/// On client disconnect remove any references to this client.
	void DisConnect(uint& iClientID, enum EFLConnection& p2)
	{
		auto iter = global->Info.begin();
		while (iter != global->Info.end())
		{
			if (iter->second.LastPmClientID == iClientID)
				iter->second.LastPmClientID = -1;
			if (iter->second.TargetClientID == iClientID)
				iter->second.TargetClientID = -1;
			++iter;
		}
	}

	/// On client F1 or entry to char select menu.
	void CharacterInfoReq(unsigned int iClientID, bool p2)
	{
		auto iter = global->Info.begin();
		while (iter != global->Info.end())
		{
			if (iter->second.LastPmClientID == iClientID)
				iter->second.LastPmClientID = -1;
			if (iter->second.TargetClientID == iClientID)
				iter->second.TargetClientID = -1;
			++iter;
		}
	}

	/// On launch events and reload the msg cache for the client.
	void PlayerLaunch(uint& iShip, uint& iClientID)
	{
		LoadMsgs(iClientID);
		ShowGreetingBanner(iClientID);
	}

	/// On base entry events and reload the msg cache for the client.
	void BaseEnter(uint iBaseID, uint iClientID)
	{
		LoadMsgs(iClientID);
		ShowGreetingBanner(iClientID);
	}

	/// When a char selects a target and the target is a player ship then
	/// record the target's clientID. */
	void SetTarget(uint& uClientID, struct XSetTarget const& p2)
	{
		// The iSpaceID *appears* to represent a player ship ID when it is
		// targeted but this might not be the case. Also note that
		// HkGetClientIDByShip returns 0 on failure not -1.
		uint uTargetClientID = HkGetClientIDByShip(p2.iSpaceID);
		if (uTargetClientID)
		{
			auto iter = global->Info.find(uClientID);
			if (iter != global->Info.end())
			{
				iter->second.TargetClientID = uTargetClientID;
			}
		}
	}

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

		std::wstring wscChatMsg = ToLower(wszBuf);
		uint iClientID = cId;

		bool bIsGroup = (cIdTo == 0x10003 || !wscChatMsg.find(L"/g ") || !wscChatMsg.find(L"/group "));
		if (!bIsGroup)
		{
			// If a restricted word appears in the message take appropriate action.
			for (auto& word : global->config->SwearWords)
			{
				if (wscChatMsg.find(word) != -1)
				{
					PrintUserCmdText(iClientID, L"This is an automated message.");
					PrintUserCmdText(iClientID, L"Please do not swear or you may be sanctioned.");

					global->Info[iClientID].SwearWordWarnings++;
					if (global->Info[iClientID].SwearWordWarnings > 2)
					{
						std::wstring wscCharname = reinterpret_cast<const wchar_t*>(Players.GetActiveCharacterName(iClientID));
						AddLog(LogType::Kick, LogLevel::Info, L"Swearing tempban on %s (%s) reason='%s'", wscCharname.c_str(),
						    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str(), (wscChatMsg).c_str());

						if (global->tempBanCommunicator)
							global->tempBanCommunicator->TempBan(wscCharname, 10);

						HkDelayedKick(iClientID, 1);

						if (global->config->DisconnectSwearingInSpaceRange > 0.0f)
						{
							std::wstring wscMsg = global->config->DisconnectSwearingInSpaceMsg;
							wscMsg = ReplaceStr(wscMsg, L"%time", GetTimeString(FLHookConfig::i()->general.dieMsg));
							wscMsg = ReplaceStr(wscMsg, L"%player", wscCharname);
							PrintLocalUserCmdText(iClientID, wscMsg, global->config->DisconnectSwearingInSpaceRange);
						}
					}
					return true;
				}
			}
		}

		/// When a private chat message is sent from one client to another record
		/// who sent the message so that the receiver can reply using the /r command
		/// */
		if (iClientID < 0x10000 && cIdTo > 0 && cIdTo < 0x10000)
		{
			auto iter = global->Info.find(cIdTo);
			if (iter != global->Info.end())
			{
				iter->second.LastPmClientID = iClientID;
			}
		}
		return false;
	}

	void RedText(std::wstring wscXMLMsg, uint iSystemID)
	{
		char szBuf[0x1000];
		uint iRet;
		if (!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsg, szBuf, sizeof(szBuf), iRet)))
			return;

		// Send to all players in system
		struct PlayerData* pPD = 0;
		while (pPD = Players.traverse_active(pPD))
		{
			uint iClientID = HkGetClientIdFromPD(pPD);
			uint iClientSystemID = 0;
			pub::Player::GetSystem(iClientID, iClientSystemID);

			if (iSystemID == iClientSystemID)
				HkFMsgSendChat(iClientID, szBuf, iRet);
		}
	}

	/** When a chat message is sent to a client and this client has showchattime on
	insert the time on the line immediately before the chat message */
	bool SendChat(uint& iClientID, uint& iTo, uint& iSize, void** rdlReader)
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
			void* rdlReader2 = *rdlReader;
			rdl.extract_text_from_buffer((unsigned short*)wszBuf, sizeof(wszBuf), iRet1, (const char*)rdlReader2, iSize);
			std::wstring wscChatMsg = wszBuf;

			// Find the ': ' which indicates the end of the sending player name.
			size_t iTextStartPos = wscChatMsg.find(L": ");
			if (iTextStartPos != std::string::npos)
			{
				if ((wscChatMsg.find(L": /") == iTextStartPos && wscChatMsg.find(L": //") != iTextStartPos) || wscChatMsg.find(L": .") == iTextStartPos)
				{
					return true;
				}
			}
		}

		if (global->Info[iClientID].ShowChatTime)
		{
			// Send time with gray color (BEBEBE) in small text (90) above the chat
			// line.
			global->SendingTime = true;
			HkFMsg(iClientID, L"<TRA data=\"0xBEBEBE90\" mask=\"-1\"/><TEXT>" + XMLText(GetTimeString(FLHookConfig::i()->general.dieMsg)) + L"</TEXT>");
			global->SendingTime = false;
		}
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// USER COMMANDS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** Set an preset message */
	void UserCmd_SetMsg(uint iClientID, const std::wstring& wscParam)
	{
		if (!global->config->EnableSetMessage)
			return;

		int iMsgSlot = ToInt(GetParam(wscParam, ' ', 0));
		std::wstring wscMsg = GetParamToEnd(wscParam, ' ', 1);

		if (iMsgSlot < 0 || iMsgSlot > 9 || wscParam.size() == 0)
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /setmsg <n> <msg text>");
			return;
		}

		HkSetCharacterIni(iClientID, L"msg." + std::to_wstring(iMsgSlot), wscMsg);

		// Reload the character cache
		LoadMsgs(iClientID);
		PrintUserCmdText(iClientID, L"OK");
	}

	/** Show preset messages */
	void UserCmd_ShowMsgs(uint iClientID, const std::wstring& wscParam)
	{
		if (!global->config->EnableSetMessage)
			return;

		auto iter = global->Info.find(iClientID);
		if (iter == global->Info.end())
		{
			PrintUserCmdText(iClientID, L"ERR No messages");
			return;
		}

		for (int i = 0; i < INFO::NUMBER_OF_SLOTS; i++)
		{
			PrintUserCmdText(iClientID, L"%d: %s", i, iter->second.slot[i].c_str());
		}
		PrintUserCmdText(iClientID, L"OK");
	}

	// User Commands for /r0-9
	void UserCmd_RMsg0(uint iClientID, const std::wstring& wscParam) { SendPresetLastPMSender(iClientID, 0, wscParam); }

	void UserCmd_RMsg1(uint iClientID, const std::wstring& wscParam) { SendPresetLastPMSender(iClientID, 1, wscParam); }

	void UserCmd_RMsg2(uint iClientID, const std::wstring& wscParam) { SendPresetLastPMSender(iClientID, 2, wscParam); }

	void UserCmd_RMsg3(uint iClientID, const std::wstring& wscParam) { SendPresetLastPMSender(iClientID, 3, wscParam); }

	void UserCmd_RMsg4(uint iClientID, const std::wstring& wscParam) { SendPresetLastPMSender(iClientID, 4, wscParam); }

	void UserCmd_RMsg5(uint iClientID, const std::wstring& wscParam) { SendPresetLastPMSender(iClientID, 5, wscParam); }

	void UserCmd_RMsg6(uint iClientID, const std::wstring& wscParam) { SendPresetLastPMSender(iClientID, 6, wscParam); }

	void UserCmd_RMsg7(uint iClientID, const std::wstring& wscParam) { SendPresetLastPMSender(iClientID, 7, wscParam); }

	void UserCmd_RMsg8(uint iClientID, const std::wstring& wscParam) { SendPresetLastPMSender(iClientID, 8, wscParam); }

	void UserCmd_RMsg9(uint iClientID, const std::wstring& wscParam) { SendPresetLastPMSender(iClientID, 9, wscParam); }

	// User Commands for /s0-9
	void UserCmd_SMsg0(uint iClientID, const std::wstring& wscParam) { SendPresetSystemMessage(iClientID, 0); }

	void UserCmd_SMsg1(uint iClientID, const std::wstring& wscParam) { SendPresetSystemMessage(iClientID, 1); }

	void UserCmd_SMsg2(uint iClientID, const std::wstring& wscParam) { SendPresetSystemMessage(iClientID, 2); }

	void UserCmd_SMsg3(uint iClientID, const std::wstring& wscParam) { SendPresetSystemMessage(iClientID, 3); }

	void UserCmd_SMsg4(uint iClientID, const std::wstring& wscParam) { SendPresetSystemMessage(iClientID, 4); }

	void UserCmd_SMsg5(uint iClientID, const std::wstring& wscParam) { SendPresetSystemMessage(iClientID, 5); }

	void UserCmd_SMsg6(uint iClientID, const std::wstring& wscParam) { SendPresetSystemMessage(iClientID, 6); }

	void UserCmd_SMsg7(uint iClientID, const std::wstring& wscParam) { SendPresetSystemMessage(iClientID, 7); }

	void UserCmd_SMsg8(uint iClientID, const std::wstring& wscParam) { SendPresetSystemMessage(iClientID, 8); }

	void UserCmd_SMsg9(uint iClientID, const std::wstring& wscParam) { SendPresetSystemMessage(iClientID, 0); }

	// User Commands for /l0-9
	void UserCmd_LMsg0(uint iClientID, const std::wstring& wscParam) { SendPresetLocalMessage(iClientID, 0); }

	void UserCmd_LMsg1(uint iClientID, const std::wstring& wscParam) { SendPresetLocalMessage(iClientID, 1); }

	void UserCmd_LMsg2(uint iClientID, const std::wstring& wscParam) { SendPresetLocalMessage(iClientID, 2); }

	void UserCmd_LMsg3(uint iClientID, const std::wstring& wscParam) { SendPresetLocalMessage(iClientID, 3); }

	void UserCmd_LMsg4(uint iClientID, const std::wstring& wscParam) { SendPresetLocalMessage(iClientID, 4); }

	void UserCmd_LMsg5(uint iClientID, const std::wstring& wscParam) { SendPresetLocalMessage(iClientID, 5); }

	void UserCmd_LMsg6(uint iClientID, const std::wstring& wscParam) { SendPresetLocalMessage(iClientID, 6); }

	void UserCmd_LMsg7(uint iClientID, const std::wstring& wscParam) { SendPresetLocalMessage(iClientID, 7); }

	void UserCmd_LMsg8(uint iClientID, const std::wstring& wscParam) { SendPresetLocalMessage(iClientID, 8); }

	void UserCmd_LMsg9(uint iClientID, const std::wstring& wscParam) { SendPresetLocalMessage(iClientID, 9); }

	// User Commands for /g0-9
	void UserCmd_GMsg0(uint iClientID, const std::wstring& wscParam) { SendPresetGroupMessage(iClientID, 0); }

	void UserCmd_GMsg1(uint iClientID, const std::wstring& wscParam) { SendPresetGroupMessage(iClientID, 1); }

	void UserCmd_GMsg2(uint iClientID, const std::wstring& wscParam) { SendPresetGroupMessage(iClientID, 2); }

	void UserCmd_GMsg3(uint iClientID, const std::wstring& wscParam) { SendPresetGroupMessage(iClientID, 3); }

	void UserCmd_GMsg4(uint iClientID, const std::wstring& wscParam) { SendPresetGroupMessage(iClientID, 4); }

	void UserCmd_GMsg5(uint iClientID, const std::wstring& wscParam) { SendPresetGroupMessage(iClientID, 5); }

	void UserCmd_GMsg6(uint iClientID, const std::wstring& wscParam) { SendPresetGroupMessage(iClientID, 6); }

	void UserCmd_GMsg7(uint iClientID, const std::wstring& wscParam) { SendPresetGroupMessage(iClientID, 7); }

	void UserCmd_GMsg8(uint iClientID, const std::wstring& wscParam) { SendPresetGroupMessage(iClientID, 8); }

	void UserCmd_GMsg9(uint iClientID, const std::wstring& wscParam) { SendPresetGroupMessage(iClientID, 9); }

	void UserCmd_TMsg0(uint iClientID, const std::wstring& wscParam) { SendPresetToLastTarget(iClientID, 0); }

	void UserCmd_TMsg1(uint iClientID, const std::wstring& wscParam) { SendPresetToLastTarget(iClientID, 1); }

	void UserCmd_TMsg2(uint iClientID, const std::wstring& wscParam) { SendPresetToLastTarget(iClientID, 2); }

	void UserCmd_TMsg3(uint iClientID, const std::wstring& wscParam) { SendPresetToLastTarget(iClientID, 3); }

	void UserCmd_TMsg4(uint iClientID, const std::wstring& wscParam) { SendPresetToLastTarget(iClientID, 4); }

	void UserCmd_TMsg5(uint iClientID, const std::wstring& wscParam) { SendPresetToLastTarget(iClientID, 5); }

	void UserCmd_TMsg6(uint iClientID, const std::wstring& wscParam) { SendPresetToLastTarget(iClientID, 6); }

	void UserCmd_TMsg7(uint iClientID, const std::wstring& wscParam) { SendPresetToLastTarget(iClientID, 7); }

	void UserCmd_TMsg8(uint iClientID, const std::wstring& wscParam) { SendPresetToLastTarget(iClientID, 8); }

	void UserCmd_TMsg9(uint iClientID, const std::wstring& wscParam) { SendPresetToLastTarget(iClientID, 9); }

	/** Send an message to the last person that PM'd this client. */
	void UserCmd_ReplyToLastPMSender(uint iClientID, const std::wstring& wscParam)
	{
		auto iter = global->Info.find(iClientID);
		if (iter == global->Info.end())
		{
			// There's no way for this to happen! yeah right.
			PrintUserCmdText(iClientID, L"ERR No message defined");
			return;
		}

		std::wstring wscMsg = GetParamToEnd(wscParam, ' ', 0);

		if (iter->second.LastPmClientID == -1)
		{
			PrintUserCmdText(iClientID, L"ERR PM sender not available");
			return;
		}

		global->Info[iter->second.LastPmClientID].LastPmClientID = iClientID;
		SendPrivateChat(iClientID, iter->second.LastPmClientID, wscMsg);
	}

	/** Shows the sender of the last PM and the last char targeted */
	void UserCmd_ShowLastPMSender(uint iClientID, const std::wstring& wscParam)
	{
		auto iter = global->Info.find(iClientID);
		if (iter == global->Info.end())
		{
			// There's no way for this to happen! yeah right.
			PrintUserCmdText(iClientID, L"ERR No message defined");
			return;
		}

		std::wstring wscSenderCharname = L"<not available>" + std::to_wstring(iter->second.LastPmClientID);
		if (iter->second.LastPmClientID != -1 && HkIsValidClientID(iter->second.LastPmClientID))
			wscSenderCharname = (const wchar_t*)Players.GetActiveCharacterName(iter->second.LastPmClientID);

		std::wstring wscTargetCharname = L"<not available>" + std::to_wstring(iter->second.TargetClientID);
		if (iter->second.TargetClientID != -1 && HkIsValidClientID(iter->second.TargetClientID))
			wscTargetCharname = (const wchar_t*)Players.GetActiveCharacterName(iter->second.TargetClientID);

		PrintUserCmdText(iClientID, L"OK sender=" + wscSenderCharname + L" target=" + wscTargetCharname);
	}

	/** Send a message to the last/current target. */
	void UserCmd_SendToLastTarget(uint iClientID, const std::wstring& wscParam)
	{
		auto iter = global->Info.find(iClientID);
		if (iter == global->Info.end())
		{
			// There's no way for this to happen! yeah right.
			PrintUserCmdText(iClientID, L"ERR No message defined");
			return;
		}

		std::wstring wscMsg = GetParamToEnd(wscParam, ' ', 0);

		if (iter->second.TargetClientID == -1)
		{
			PrintUserCmdText(iClientID, L"ERR PM target not available");
			return;
		}

		global->Info[iter->second.TargetClientID].LastPmClientID = iClientID;
		SendPrivateChat(iClientID, iter->second.TargetClientID, wscMsg);
	}

	/** Send a private message to the specified charname. If the player is offline
	the message will be delivery when they next login. */
	void UserCmd_PrivateMsg(uint iClientID, const std::wstring& wscParam)
	{
		std::wstring usage = L"Usage: /privatemsg <charname> <messsage> or /pm ...";
		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
		const std::wstring& wscTargetCharname = GetParam(wscParam, ' ', 0);
		const std::wstring& wscMsg = GetParamToEnd(wscParam, ' ', 1);

		if (wscCharname.size() == 0 || wscMsg.size() == 0)
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, usage);
			return;
		}

		if (!HkGetAccountByCharname(wscTargetCharname))
		{
			PrintUserCmdText(iClientID, L"ERR charname does not exist");
			return;
		}

		uint iToClientID = HkGetClientIdFromCharname(wscTargetCharname);
		if (iToClientID == -1)
		{
			if (global->mailCommunicator)
			{
				global->mailCommunicator->SendMail(wscTargetCharname, wscMsg);
				PrintUserCmdText(iClientID, L"OK message saved to mailbox");
			}
			else
			{
				PrintUserCmdText(iClientID, L"ERR: Player offline");
			}
		}
		else
		{
			global->Info[iToClientID].LastPmClientID = iClientID;
			SendPrivateChat(iClientID, iToClientID, wscMsg);
		}
	}

	/** Send a private message to the specified clientid. */
	void UserCmd_PrivateMsgID(uint iClientID, const std::wstring& wscParam)
	{
		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
		const std::wstring& wscClientID = GetParam(wscParam, ' ', 0);
		const std::wstring& wscMsg = GetParamToEnd(wscParam, ' ', 1);

		uint iToClientID = ToInt(wscClientID);
		if (!HkIsValidClientID(iToClientID) || HkIsInCharSelectMenu(iToClientID))
		{
			PrintUserCmdText(iClientID, L"ERR Invalid client-id");
			return;
		}

		global->Info[iToClientID].LastPmClientID = iClientID;
		SendPrivateChat(iClientID, iToClientID, wscMsg);
	}

	/** Send a message to all players with a particular prefix. */
	void UserCmd_FactionMsg(uint iClientID, const std::wstring& wscParam)
	{
		std::wstring wscSender = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
		const std::wstring& wscCharnamePrefix = GetParam(wscParam, ' ', 0);
		const std::wstring& wscMsg = GetParamToEnd(wscParam, ' ', 1);

		if (wscCharnamePrefix.size() < 3 || wscMsg.size() == 0)
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /factionmsg <tag> <message> or /fm ...");
			return;
		}

		bool bSenderReceived = false;
		bool bMsgSent = false;
		for (auto& player : HkGetPlayers())
		{
			if (ToLower(player.wscCharname).find(ToLower(wscCharnamePrefix)) == std::string::npos)
				continue;

			if (player.iClientID == iClientID)
				bSenderReceived = true;

			FormatSendChat(player.iClientID, wscSender, wscMsg, L"FF7BFF");
			bMsgSent = true;
		}
		if (!bSenderReceived)
			FormatSendChat(iClientID, wscSender, wscMsg, L"FF7BFF");

		if (bMsgSent == false)
			PrintUserCmdText(iClientID, L"ERR No chars found");
	}

	/** Send a faction invite message to all players with a particular prefix. */
	void UserCmd_FactionInvite(uint iClientID, const std::wstring& wscParam)
	{
		const std::wstring& wscCharnamePrefix = GetParam(wscParam, ' ', 0);

		bool msgSent = false;

		if (wscCharnamePrefix.size() < 3)
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /factioninvite <tag> or /fi ...");
			return;
		}

		for (auto& player : HkGetPlayers())
		{
			if (ToLower(player.wscCharname).find(ToLower(wscCharnamePrefix)) == std::string::npos)
				continue;
			if (player.iClientID == iClientID)
				continue;

			std::wstring wscMsg = L"/i " + player.wscCharname;

			uint iRet;
			char szBuf[1024];
			if (HK_ERROR err; (err = HkFMsgEncodeXML(wscMsg, szBuf, sizeof(szBuf), iRet)) != HKE_OK)
			{
				PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
				return;
			}

			struct CHAT_ID cId = {iClientID};
			struct CHAT_ID cIdTo = {0x10001};

			Server.SubmitChat(cId, iRet, szBuf, cIdTo, -1);
			msgSent = true;
		}

		if (msgSent == false)
			PrintUserCmdText(iClientID, L"ERR No chars found");
	}

	void UserCmd_SetChatTime(uint iClientID, const std::wstring& wscParam)
	{
		std::wstring wscParam1 = ToLower(GetParam(wscParam, ' ', 0));
		bool bShowChatTime = false;
		if (!wscParam1.compare(L"on"))
			bShowChatTime = true;
		else if (!wscParam1.compare(L"off"))
			bShowChatTime = false;
		else
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /set chattime [on|off]");
		}

		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		HkSetCharacterIni(iClientID, L"msg.chat_time", bShowChatTime ? L"true" : L"false");

		// Update the client cache.
		auto iter = global->Info.find(iClientID);
		if (iter != global->Info.end())
			iter->second.ShowChatTime = bShowChatTime;

		// Send confirmation msg
		PrintUserCmdText(iClientID, L"OK");
	}

	void UserCmd_Time(uint iClientID, const std::wstring& wscParam)
	{
		// Send time with gray color (BEBEBE) in small text (90) above the chat line.
		PrintUserCmdText(iClientID, GetTimeString(FLHookConfig::i()->general.localTime));
	}

	/** Print out custom help overriding flhook built in help */
	void UserCmd_CustomHelp(uint iClientID, const std::wstring& wscParam)
	{
		if (global->config->CustomHelp)
		{
			// Print any custom help strings
			for (auto& [key, value] : global->config->HelpLines)
			{
				std::string scHelp = key;
				if (value.size() > 0)
				{
					scHelp += "=";
					scHelp += value;
				}
				PrintUserCmdText(iClientID, stows(scHelp));
			}
		}
	}

	/** Me command allow players to type "/me powers his engines" which would print:
	 * "Trent powers his engines" */
	void UserCmd_Me(uint iClientID, const std::wstring& wscParam)
	{
		if (global->config->EnableMe)
		{
			std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
			uint iSystemID;
			pub::Player::GetSystem(iClientID, iSystemID);

			// Encode message using the death message style (red text).
			std::wstring wscXMLMsg = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.deathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
			wscXMLMsg += charname + L" ";
			wscXMLMsg += XMLText(GetParamToEnd(wscParam, ' ', 0));
			wscXMLMsg += L"</TEXT>";

			RedText(wscXMLMsg, iSystemID);
		}
		else
		{
			PrintUserCmdText(iClientID, L"Command not enabled.");
		}
	}

	/** Do command allow players to type "/do Nomad fighters detected" which would
	 * print: "Nomad fighters detected" in the standard red text */
	void UserCmd_Do(uint iClientID, const std::wstring& wscParam)
	{
		if (global->config->EnableDo)
		{
			uint iSystemID;
			pub::Player::GetSystem(iClientID, iSystemID);

			// Encode message using the death message style (red text).
			std::wstring wscXMLMsg = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.deathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
			wscXMLMsg += XMLText(GetParamToEnd(wscParam, ' ', 0));
			wscXMLMsg += L"</TEXT>";

			RedText(wscXMLMsg, iSystemID);
		}
		else
		{
			PrintUserCmdText(iClientID, L"Command not enabled.");
		}
	}

	// Client command processing
const std::array<USERCMD, 62> UserCmds = {{
    {L"/setmsg", UserCmd_SetMsg},
    {L"/showmsgs", UserCmd_ShowMsgs},
    {L"/0", UserCmd_SMsg0},
    {L"/1", UserCmd_SMsg1},
    {L"/2", UserCmd_SMsg2},
    {L"/3", UserCmd_SMsg3},
    {L"/4", UserCmd_SMsg4},
    {L"/5", UserCmd_SMsg5},
    {L"/6", UserCmd_SMsg6},
    {L"/7", UserCmd_SMsg7},
    {L"/8", UserCmd_SMsg8},
    {L"/9", UserCmd_SMsg9},
    {L"/l0", UserCmd_LMsg0},
    {L"/l1", UserCmd_LMsg1},
    {L"/l2", UserCmd_LMsg2},
    {L"/l3", UserCmd_LMsg3},
    {L"/l4", UserCmd_LMsg4},
    {L"/l5", UserCmd_LMsg5},
    {L"/l6", UserCmd_LMsg6},
    {L"/l7", UserCmd_LMsg7},
    {L"/l8", UserCmd_LMsg8},
    {L"/l9", UserCmd_LMsg9},
    {L"/g0", UserCmd_GMsg0},
    {L"/g1", UserCmd_GMsg1},
    {L"/g2", UserCmd_GMsg2},
    {L"/g3", UserCmd_GMsg3},
    {L"/g4", UserCmd_GMsg4},
    {L"/g5", UserCmd_GMsg5},
    {L"/g6", UserCmd_GMsg6},
    {L"/g7", UserCmd_GMsg7},
    {L"/g8", UserCmd_GMsg8},
    {L"/g9", UserCmd_GMsg9},
    {L"/t0", UserCmd_TMsg0},
    {L"/t1", UserCmd_TMsg1},
    {L"/t2", UserCmd_TMsg2},
    {L"/t3", UserCmd_TMsg3},
    {L"/t4", UserCmd_TMsg4},
    {L"/t5", UserCmd_TMsg5},
    {L"/t6", UserCmd_TMsg6},
    {L"/t7", UserCmd_TMsg7},
    {L"/t8", UserCmd_TMsg8},
    {L"/t9", UserCmd_TMsg9},
    {L"/target", UserCmd_SendToLastTarget},
    {L"/t", UserCmd_SendToLastTarget},
    {L"/reply", UserCmd_ReplyToLastPMSender},
    {L"/r", UserCmd_ReplyToLastPMSender},
    {L"/privatemsg$", UserCmd_PrivateMsgID},
    {L"/pm$", UserCmd_PrivateMsgID},
    {L"/privatemsg", UserCmd_PrivateMsg},
    {L"/pm", UserCmd_PrivateMsg},
    {L"/factionmsg", UserCmd_FactionMsg},
    {L"/fm", UserCmd_FactionMsg},
    {L"/factioninvite", UserCmd_FactionInvite},
    {L"/fi", UserCmd_FactionInvite},
    {L"/lastpm", UserCmd_ShowLastPMSender},
    {L"/set chattime", UserCmd_SetChatTime},
    {L"/help", UserCmd_CustomHelp},
    {L"/h", UserCmd_CustomHelp},
    {L"/?", UserCmd_CustomHelp},
    {L"/me", UserCmd_Me},
    {L"/do", UserCmd_Do},
    {L"/time", UserCmd_Time},
}};

	// Process user input
	bool UserCmd_Process(uint& iClientID, const std::wstring& wscCmd)
	{
	// Echo the command back to the sender's console but only if it starts with
	// / or .
	std::wstring wscCmdLineLower = ToLower(wscCmd);
	if (global->config->EchoCommandsBackToUser)
	{
		std::wstring wscCmd = GetParam(wscCmdLineLower, ' ', 0);
		if (wscCmd.find(L"/") == 0 || wscCmd.find(L".") == 0)
			{
				if (!(wscCmd == L"/l" || wscCmd == L"/local" || wscCmd == L"/s" || wscCmd == L"/system" || wscCmd == L"/g" || wscCmd == L"/group" ||
				        wscCmd == L"/t" || wscCmd == L"/target" || wscCmd == L"/r" || wscCmd == L"/reply" || wscCmd.find(L"//") == 0 ||
				        wscCmd.find(L"*") == (wscCmd.length() - 1)))
				{
					std::wstring wscXML =
					    L"<TRA data=\"" + global->config->CommandEchoStyle + L"\" mask=\"-1\"/><TEXT>" + XMLText(wscCmdLineLower) + L"</TEXT>";
					HkFMsg(iClientID, wscXML);
				}
			}
		}
		return DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, global->returncode);
	}

	// Hook on /help
	void UserCmd_Help(uint& iClientID, const std::wstring& wscParam)
	{	
	for (auto& uc : UserCmds)
		PrintUserCmdText(iClientID, uc.cmd);
	}
} // namespace Plugins::Message

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::Message;
REFL_AUTO(type(Config), field(HelpLines), field(GreetingBannerLines), field(SpecialBannerLines), field(StandardBannerLines), field(SpecialBannerTimeout),
    field(StandardBannerTimeout), field(CustomHelp), field(EchoCommandsBackToUser), field(SuppressMistypedCommands), field(EnableSetMessage), field(EnableMe),
    field(EnableDo), field(CommandEchoStyle), field(DisconnectSwearingInSpaceMsg), field(DisconnectSwearingInSpaceRange), field(SwearWords))

DefaultDllMainSettings(LoadSettings)

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Message");
	pi->shortName("message");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
	pi->emplaceHook(HookedCall::IServerImpl__SubmitChat, &SubmitChat);
	pi->emplaceHook(HookedCall::IChat__SendChat, &SendChat);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__TimerNPCAndF1Check, &Timer);
	pi->emplaceHook(HookedCall::IServerImpl__SetTarget, &SetTarget);
	pi->emplaceHook(HookedCall::IServerImpl__DisConnect, &DisConnect);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);

	// Load our communicators
	global->mailCommunicator =
	    static_cast<Plugins::Mail::MailCommunicator*>(PluginCommunicator::ImportPluginCommunicator(Plugins::Mail::MailCommunicator::pluginName));
	global->tempBanCommunicator =
	    static_cast<Plugins::Tempban::TempBanCommunicator*>(PluginCommunicator::ImportPluginCommunicator(Plugins::Tempban::TempBanCommunicator::pluginName));
}