#include "hook.h"

#define PRINT_ERROR()                                                        \
	{                                                                        \
		for (uint i = 0; (i < sizeof(wscError) / sizeof(std::wstring)); i++) \
			PrintUserCmdText(iClientID, L"%s", wscError[i].c_str());         \
		return;                                                              \
	}
#define PRINT_OK()       PrintUserCmdText(iClientID, L"OK");
#define PRINT_DISABLED() PrintUserCmdText(iClientID, L"Command disabled");
#define GET_USERFILE(a)                                             \
	std::string a;                                                  \
	{                                                               \
		CAccount* acc = Players.FindAccountFromClientID(iClientID); \
		std::wstring wscDir;                                        \
		HkGetAccountDirName(acc, wscDir);                           \
		a = scAcctPath + wstos(wscDir) + "\\flhookuser.ini";        \
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*_UserCmdProc)(uint, const std::wstring&);

struct USERCMD
{
	wchar_t* wszCmd;
	_UserCmdProc proc;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintUserCmdText(uint iClientID, std::wstring wscText, ...)
{
	wchar_t wszBuf[1024 * 8] = L"";
	va_list marker;
	va_start(marker, wscText);
	_vsnwprintf_s(wszBuf, sizeof wszBuf - 1, wscText.c_str(), marker);

	std::wstring wscXML =
	    L"<TRA data=\"" + set_wscUserCmdStyle + L"\" mask=\"-1\"/><TEXT>" + XMLText(wszBuf) + L"</TEXT>";
	HkFMsg(iClientID, wscXML);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Print message to all ships within the specific number of meters of the
/// player.
void PrintLocalUserCmdText(uint iClientID, const std::wstring& wscMsg, float fDistance)
{
	uint iShip;
	pub::Player::GetShip(iClientID, iShip);

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(iShip, pos, rot);

	uint iSystem;
	pub::Player::GetSystem(iClientID, iSystem);

	// For all players in system...
	struct PlayerData* pPD = 0;
	while (pPD = Players.traverse_active(pPD))
	{
		// Get the this player's current system and location in the system.
		uint iClientID2 = HkGetClientIdFromPD(pPD);
		uint iSystem2 = 0;
		pub::Player::GetSystem(iClientID2, iSystem2);
		if (iSystem != iSystem2)
			continue;

		uint iShip2;
		pub::Player::GetShip(iClientID2, iShip2);

		Vector pos2;
		Matrix rot2;
		pub::SpaceObj::GetLocation(iShip2, pos2, rot2);

		// Is player within the specified range of the sending char.
		if (HkDistance3D(pos, pos2) > fDistance)
			continue;

		PrintUserCmdText(iClientID2, wscMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetDieMsg(uint iClientID, const std::wstring& wscParam)
{
	if (!set_bUserCmdSetDieMsg)
	{
		PRINT_DISABLED();
		return;
	}

	std::wstring wscError[] = {
		L"Error: Invalid parameters",
		L"Usage: /set diemsg <param>",
		L"<param>: all,system,self or none",
	};

	std::wstring wscDieMsg = ToLower(GetParam(wscParam, ' ', 0));
	;

	DIEMSGTYPE dieMsg;
	if (!wscDieMsg.compare(L"all"))
		dieMsg = DIEMSG_ALL;
	else if (!wscDieMsg.compare(L"system"))
		dieMsg = DIEMSG_SYSTEM;
	else if (!wscDieMsg.compare(L"none"))
		dieMsg = DIEMSG_NONE;
	else if (!wscDieMsg.compare(L"self"))
		dieMsg = DIEMSG_SELF;
	else
		PRINT_ERROR();

	// save to ini
	GET_USERFILE(scUserFile);
	IniWrite(scUserFile, "settings", "DieMsg", std::to_string(dieMsg));

	// save in ClientInfo
	ClientInfo[iClientID].dieMsg = dieMsg;

	// send confirmation msg
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetDieMsgSize(uint iClientID, const std::wstring& wscParam)
{
	if (!set_bUserCmdSetDieMsgSize)
	{
		PRINT_DISABLED();
		return;
	}

	std::wstring wscError[] = {
		L"Error: Invalid parameters",
		L"Usage: /set diemsgsize <size>",
		L"<size>: small, default",
	};

	std::wstring wscDieMsgSize = ToLower(GetParam(wscParam, ' ', 0));
	//	std::wstring wscDieMsgStyle = ToLower(GetParam(wscParam, ' ', 1));

	CHATSIZE dieMsgSize;
	if (!wscDieMsgSize.compare(L"small"))
		dieMsgSize = CS_SMALL;
	else if (!wscDieMsgSize.compare(L"default"))
		dieMsgSize = CS_DEFAULT;
	else
		PRINT_ERROR();

	/*	CHATSTYLE dieMsgStyle;
	        if(!wscDieMsgStyle.compare(L"default"))
	                dieMsgStyle = CST_DEFAULT;
	        else if(!wscDieMsgStyle.compare(L"bold"))
	                dieMsgStyle = CST_BOLD;
	        else if(!wscDieMsgStyle.compare(L"italic"))
	                dieMsgStyle = CST_ITALIC;
	        else if(!wscDieMsgStyle.compare(L"underline"))
	                dieMsgStyle = CST_UNDERLINE;
	        else
	                PRINT_ERROR(); */

	// save to ini
	GET_USERFILE(scUserFile);
	IniWrite(scUserFile, "Settings", "DieMsgSize", std::to_string(dieMsgSize));
	//	IniWrite(scUserFile, "Settings", "DieMsgStyle",
	// std::to_string(dieMsgStyle));

	// save in ClientInfo
	ClientInfo[iClientID].dieMsgSize = dieMsgSize;
	//	ClientInfo[iClientID].dieMsgStyle = dieMsgStyle;

	// send confirmation msg
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetChatFont(uint iClientID, const std::wstring& wscParam)
{
	if (!set_bUserCmdSetChatFont)
	{
		PRINT_DISABLED();
		return;
	}

	std::wstring wscError[] = {
		L"Error: Invalid parameters",
		L"Usage: /set chatfont <size> <style>",
		L"<size>: small, default or big",
		L"<style>: default, bold, italic or underline",
	};

	std::wstring wscChatSize = ToLower(GetParam(wscParam, ' ', 0));
	std::wstring wscChatStyle = ToLower(GetParam(wscParam, ' ', 1));

	CHATSIZE chatSize;
	if (!wscChatSize.compare(L"small"))
		chatSize = CS_SMALL;
	else if (!wscChatSize.compare(L"default"))
		chatSize = CS_DEFAULT;
	else if (!wscChatSize.compare(L"big"))
		chatSize = CS_BIG;
	else
		PRINT_ERROR();

	CHATSTYLE chatStyle;
	if (!wscChatStyle.compare(L"default"))
		chatStyle = CST_DEFAULT;
	else if (!wscChatStyle.compare(L"bold"))
		chatStyle = CST_BOLD;
	else if (!wscChatStyle.compare(L"italic"))
		chatStyle = CST_ITALIC;
	else if (!wscChatStyle.compare(L"underline"))
		chatStyle = CST_UNDERLINE;
	else
		PRINT_ERROR();

	// save to ini
	GET_USERFILE(scUserFile);
	IniWrite(scUserFile, "settings", "ChatSize", std::to_string(chatSize));
	IniWrite(scUserFile, "settings", "ChatStyle", std::to_string(chatStyle));

	// save in ClientInfo
	ClientInfo[iClientID].chatSize = chatSize;
	ClientInfo[iClientID].chatStyle = chatStyle;

	// send confirmation msg
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Ignore(uint iClientID, const std::wstring& wscParam)
{
	if (!set_bUserCmdIgnore)
	{
		PRINT_DISABLED();
		return;
	}

	std::wstring wscError[] = {
		L"Error: Invalid parameters",
		L"Usage: /ignore <charname> [<flags>]",
		L"<charname>: character name which should be ignored(case insensitive)",
		L"<flags>: combination of the following flags:",
		L" p - only affect private chat",
		L" i - <charname> may match partially",
		L"Examples:",
		L"\"/ignore SomeDude\" ignores all chatmessages from SomeDude",
		L"\"/ignore PlayerX p\" ignores all private-chatmessages from PlayerX",
		L"\"/ignore idiot i\" ignores all chatmessages from players whose "
		L"charname contain \"idiot\" (e.g. \"[XYZ]IDIOT\", \"MrIdiot\", etc)",
		L"\"/ignore Fool pi\" ignores all private-chatmessages from players "
		L"whose charname contain \"fool\"",
	};

	std::wstring wscAllowedFlags = L"pi";

	std::wstring wscCharname = GetParam(wscParam, ' ', 0);
	std::wstring wscFlags = ToLower(GetParam(wscParam, ' ', 1));

	if (!wscCharname.length())
		PRINT_ERROR();

	// check if flags are valid
	for (uint i = 0; i < wscFlags.length(); i++)
	{
		if (wscAllowedFlags.find_first_of(wscFlags[i]) == -1)
			PRINT_ERROR();
	}

	if (ClientInfo[iClientID].lstIgnore.size() > set_iUserCmdMaxIgnoreList)
	{
		PrintUserCmdText(
		    iClientID,
		    L"Error: Too many entries in the ignore list, "
		    L"please delete an entry first!");
		return;
	}

	// save to ini
	GET_USERFILE(scUserFile);
	IniWriteW(
	    scUserFile, "IgnoreList", std::to_string((int)ClientInfo[iClientID].lstIgnore.size() + 1),
	    wscCharname + L" " + wscFlags);

	// save in ClientInfo
	IGNORE_INFO ii;
	ii.wscCharname = wscCharname;
	ii.wscFlags = wscFlags;
	ClientInfo[iClientID].lstIgnore.push_back(ii);

	// send confirmation msg
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IgnoreID(uint iClientID, const std::wstring& wscParam)
{
	if (!set_bUserCmdIgnore)
	{
		PRINT_DISABLED();
		return;
	}

	std::wstring wscError[] = {
		L"Error: Invalid parameters",
		L"Usage: /ignoreid <client-id> [<flags>]",
		L"<client-id>: client-id of character which should be ignored",
		L"<flags>: if \"p\"(without quotation marks) then only affect private "
		L"chat",
	};

	std::wstring wscClientID = GetParam(wscParam, ' ', 0);
	std::wstring wscFlags = ToLower(GetParam(wscParam, ' ', 1));

	if (!wscClientID.length())
		PRINT_ERROR();

	if (wscFlags.length() && wscFlags.compare(L"p") != 0)
		PRINT_ERROR();

	if (ClientInfo[iClientID].lstIgnore.size() > set_iUserCmdMaxIgnoreList)
	{
		PrintUserCmdText(
		    iClientID,
		    L"Error: Too many entries in the ignore list, "
		    L"please delete an entry first!");
		return;
	}

	uint iClientIDTarget = ToInt(wscClientID);
	if (!HkIsValidClientID(iClientIDTarget) || HkIsInCharSelectMenu(iClientIDTarget))
	{
		PrintUserCmdText(iClientID, L"Error: Invalid client-id");
		return;
	}

	std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientIDTarget);

	// save to ini
	GET_USERFILE(scUserFile);
	IniWriteW(
	    scUserFile, "IgnoreList", std::to_string((int)ClientInfo[iClientID].lstIgnore.size() + 1),
	    wscCharname + L" " + wscFlags);

	// save in ClientInfo
	IGNORE_INFO ii;
	ii.wscCharname = wscCharname;
	ii.wscFlags = wscFlags;
	ClientInfo[iClientID].lstIgnore.push_back(ii);

	// send confirmation msg
	PrintUserCmdText(iClientID, L"OK, \"%s\" added to ignore list", wscCharname.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IgnoreList(uint iClientID, const std::wstring& wscParam)
{
	if (!set_bUserCmdIgnore)
	{
		PRINT_DISABLED();
		return;
	}

	PrintUserCmdText(iClientID, L"ID | Charactername | Flags");
	int i = 1;
	for (auto& ignore : ClientInfo[iClientID].lstIgnore)
	{
		PrintUserCmdText(iClientID, L"%.2u | %s | %s", i, ignore.wscCharname.c_str(), ignore.wscFlags.c_str());
		i++;
	}

	// send confirmation msg
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_DelIgnore(uint iClientID, const std::wstring& wscParam)
{
	if (!set_bUserCmdIgnore)
	{
		PRINT_DISABLED();
		return;
	}

	std::wstring wscError[] = {
		L"Error: Invalid parameters",
		L"Usage: /delignore <id> [<id2> <id3> ...]",
		L"<id>: id of ignore-entry(see /ignorelist) or *(delete all)",
	};

	std::wstring wscID = GetParam(wscParam, ' ', 0);

	if (!wscID.length())
		PRINT_ERROR();

	GET_USERFILE(scUserFile);

	if (!wscID.compare(L"*"))
	{ // delete all
		IniDelSection(scUserFile, "IgnoreList");
		ClientInfo[iClientID].lstIgnore.clear();
		PRINT_OK();
		return;
	}

	std::list<uint> lstDelete;
	for (uint j = 1; wscID.length(); j++)
	{
		uint iID = ToInt(wscID.c_str());
		if (!iID || iID > ClientInfo[iClientID].lstIgnore.size())
		{
			PrintUserCmdText(iClientID, L"Error: Invalid ID");
			return;
		}

		lstDelete.push_back(iID);
		wscID = GetParam(wscParam, ' ', j);
	}

	lstDelete.sort(std::greater<uint>());

	ClientInfo[iClientID].lstIgnore.reverse();
	for (auto& del : lstDelete)
	{
		uint iCurID = (uint)ClientInfo[iClientID].lstIgnore.size();
		for (auto ignoreIt = ClientInfo[iClientID].lstIgnore.begin(); ignoreIt != ClientInfo[iClientID].lstIgnore.end();
		     ++ignoreIt)
		{
			if (iCurID == del)
			{
				ClientInfo[iClientID].lstIgnore.erase(ignoreIt);
				break;
			}
			iCurID--;
		}
	}
	ClientInfo[iClientID].lstIgnore.reverse();

	// send confirmation msg
	IniDelSection(scUserFile, "IgnoreList");
	int i = 1;
	for (auto& ignore : ClientInfo[iClientID].lstIgnore)
	{
		IniWriteW(scUserFile, "IgnoreList", std::to_string(i), ignore.wscCharname + L" " + ignore.wscFlags);
		i++;
	}
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_AutoBuy(uint iClientID, const std::wstring& wscParam)
{
	if (!set_bAutoBuy)
	{
		PRINT_DISABLED();
		return;
	}

	std::wstring wscError[] = {
		L"Error: Invalid parameters",
		L"Usage: /autobuy <param> [<on/off>]",
		L"<Param>:",
		L"   info - display current autobuy-settings",
		L"   missiles - enable/disable autobuy for missiles",
		L"   torps - enable/disable autobuy for torpedos",
		L"   mines - enable/disable autobuy for mines",
		L"   cd - enable/disable autobuy for cruise disruptors",
		L"   cm - enable/disable autobuy for countermeasures",
		L"   reload - enable/disable autobuy for nanobots/shield batteries",
		L"   all: enable/disable autobuy for all of the above",
		L"Examples:",
		L"\"/autobuy missiles on\" enable autobuy for missiles",
		L"\"/autobuy all off\" completely disable autobuy",
		L"\"/autobuy info\" show autobuy info",
	};

	std::wstring wscType = ToLower(GetParam(wscParam, ' ', 0));
	std::wstring wscSwitch = ToLower(GetParam(wscParam, ' ', 1));

	if (!wscType.compare(L"info"))
	{
		PrintUserCmdText(iClientID, L"Missiles: %s", ClientInfo[iClientID].bAutoBuyMissiles ? L"On" : L"Off");
		PrintUserCmdText(iClientID, L"Mine: %s", ClientInfo[iClientID].bAutoBuyMines ? L"On" : L"Off");
		PrintUserCmdText(iClientID, L"Torpedos: %s", ClientInfo[iClientID].bAutoBuyTorps ? L"On" : L"Off");
		PrintUserCmdText(iClientID, L"Cruise Disruptors: %s", ClientInfo[iClientID].bAutoBuyCD ? L"On" : L"Off");
		PrintUserCmdText(iClientID, L"Countermeasures: %s", ClientInfo[iClientID].bAutoBuyCM ? L"On" : L"Off");
		PrintUserCmdText(
		    iClientID, L"Nanobots/Shield Batteries: %s", ClientInfo[iClientID].bAutoBuyReload ? L"On" : L"Off");
		return;
	}

	if (!wscType.length() || !wscSwitch.length() || wscSwitch.compare(L"on") != 0 && wscSwitch.compare(L"off") != 0)
		PRINT_ERROR();

	GET_USERFILE(scUserFile);

	std::wstring wscFilename;
	HkGetCharFileName(iClientID, wscFilename);
	std::string scSection = "autobuy_" + wstos(wscFilename);

	bool bEnable = !wscSwitch.compare(L"on") ? true : false;
	if (!wscType.compare(L"all"))
	{
		ClientInfo[iClientID].bAutoBuyMissiles = bEnable;
		ClientInfo[iClientID].bAutoBuyMines = bEnable;
		ClientInfo[iClientID].bAutoBuyTorps = bEnable;
		ClientInfo[iClientID].bAutoBuyCD = bEnable;
		ClientInfo[iClientID].bAutoBuyCM = bEnable;
		ClientInfo[iClientID].bAutoBuyReload = bEnable;
		IniWrite(scUserFile, scSection, "missiles", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "mines", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "torps", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "cd", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "cm", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "reload", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"missiles"))
	{
		ClientInfo[iClientID].bAutoBuyMissiles = bEnable;
		IniWrite(scUserFile, scSection, "missiles", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"mines"))
	{
		ClientInfo[iClientID].bAutoBuyMines = bEnable;
		IniWrite(scUserFile, scSection, "mines", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"torps"))
	{
		ClientInfo[iClientID].bAutoBuyTorps = bEnable;
		IniWrite(scUserFile, scSection, "torps", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"cd"))
	{
		ClientInfo[iClientID].bAutoBuyCD = bEnable;
		IniWrite(scUserFile, scSection, "cd", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"cm"))
	{
		ClientInfo[iClientID].bAutoBuyCM = bEnable;
		IniWrite(scUserFile, scSection, "cm", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"reload"))
	{
		ClientInfo[iClientID].bAutoBuyReload = bEnable;
		IniWrite(scUserFile, scSection, "reload", bEnable ? "yes" : "no");
	}
	else
		PRINT_ERROR();

	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IDs(uint iClientID, const std::wstring& wscParam)
{
	wchar_t wszLine[128] = L"";
	for (auto& player : HkGetPlayers())
	{
		wchar_t wszBuf[1024];
		swprintf_s(wszBuf, L"%s = %u | ", player.wscCharname.c_str(), player.iClientID);
		if (wcslen(wszBuf) + wcslen(wszLine) >= sizeof wszLine / 2)
		{
			PrintUserCmdText(iClientID, L"%s", wszLine);
			wcscpy_s(wszLine, wszBuf);
		}
		else
			wcscat_s(wszLine, wszBuf);
	}

	if (wcslen(wszLine))
		PrintUserCmdText(iClientID, L"%s", wszLine);
	PrintUserCmdText(iClientID, L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_ID(uint iClientID, const std::wstring& wscParam)
{
	PrintUserCmdText(iClientID, L"Your client-id: %u", iClientID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_InviteID(uint iClientID, const std::wstring& wscParam)
{
	std::wstring wscError[] = {
		L"Error: Invalid parameters",
		L"Usage: /i$ <client-id>",
	};

	std::wstring wscClientID = GetParam(wscParam, ' ', 0);

	if (!wscClientID.length())
		PRINT_ERROR();

	uint iClientIDTarget = ToInt(wscClientID);
	if (!HkIsValidClientID(iClientIDTarget) || HkIsInCharSelectMenu(iClientIDTarget))
	{
		PrintUserCmdText(iClientID, L"Error: Invalid client-id");
		return;
	}

	std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientIDTarget);

	std::wstring wscXML = L"<TEXT>/i " + XMLText(wscCharname) + L"</TEXT>";
	char szBuf[0xFFFF];
	uint iRet;
	if (!HKHKSUCCESS(HkFMsgEncodeXML(wscXML, szBuf, sizeof szBuf, iRet)))
	{
		PrintUserCmdText(iClientID, L"Error: Could not encode XML");
		return;
	}

	CHAT_ID cID;
	cID.iID = iClientID;
	CHAT_ID cIDTo;
	cIDTo.iID = 0x00010001;
	Server.SubmitChat(cID, iRet, szBuf, cIDTo, -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Credits(uint iClientID, const std::wstring& wscParam)
{
	PrintUserCmdText(iClientID, L"This server is running FLHook v" + VersionInformation);
	PrintUserCmdText(iClientID, L"Running plugins:");

	bool bRunning = false;
	for (const auto& plugin : PluginManager::ir())
	{
		if (plugin.paused)
			continue;

		bRunning = true;
		PrintUserCmdText(iClientID, L"- %s", stows(plugin.name).c_str());
	}
	if (!bRunning)
		PrintUserCmdText(iClientID, L"- none -");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Help(uint iClientID, const std::wstring& wscParam)
{
	if (!set_bUserCmdHelp)
	{
		PRINT_DISABLED();
		return;
	}

	bool singleCommandHelp = wscParam.length() > 1;

	if (!singleCommandHelp)
		PrintUserCmdText(
		    iClientID,
		    L"The following commands are available to you. Use /help "
		    L"<command> for detailed information.");

	std::wstring boldHelp = set_wscUserCmdStyle.substr(0, set_wscUserCmdStyle.length() - 1) + L"1";
	std::wstring normal = set_wscUserCmdStyle;

	for (auto& he : lstHelpEntries)
	{
		if (he.fnIsDisplayed(iClientID))
		{
			if (singleCommandHelp)
			{
				if (he.wszCommand == wscParam)
				{
					set_wscUserCmdStyle = boldHelp;
					PrintUserCmdText(iClientID, he.wszCommand + L" " + he.wszArguments);
					set_wscUserCmdStyle = normal;
					int pos = 0;
					while (pos != std::wstring::npos)
					{
						int nextPos = he.wszLongHelp.find('\n', pos + 1);
						PrintUserCmdText(iClientID, L"â€‚â€‚" + he.wszLongHelp.substr(pos, nextPos - pos));
						pos = nextPos;
					}
					return;
				}
			}
			else
			{
				set_wscUserCmdStyle = boldHelp;
				PrintUserCmdText(iClientID, he.wszCommand + L" " + he.wszArguments);
				set_wscUserCmdStyle = normal;
				PrintUserCmdText(iClientID, L"â€‚â€‚" + he.wszShortHelp);
			}
		}
	}

	if (singleCommandHelp)
	{
		set_wscUserCmdStyle = boldHelp;
		PrintUserCmdText(iClientID, wscParam);
		set_wscUserCmdStyle = normal;

		if (!UserCmd_Process(iClientID, wscParam))
			PrintUserCmdText(iClientID, L"No help found for specified command.");
	}
	else
	{
		CallPluginsAfter(HookedCall::FLHook__UserCommand__Help, iClientID, wscParam);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

USERCMD UserCmds[] = {
	{ L"/set diemsg", UserCmd_SetDieMsg },
	{ L"/set diemsgsize", UserCmd_SetDieMsgSize },
	{ L"/set chatfont", UserCmd_SetChatFont },
	{ L"/ignorelist", UserCmd_IgnoreList },
	{ L"/delignore", UserCmd_DelIgnore },
	{ L"/ignore", UserCmd_Ignore },
	{ L"/ignoreid", UserCmd_IgnoreID },
	{ L"/autobuy", UserCmd_AutoBuy },
	{ L"/ids", UserCmd_IDs },
	{ L"/id", UserCmd_ID },
	{ L"/i$", UserCmd_InviteID },
	{ L"/invite$", UserCmd_InviteID },
	{ L"/credits", UserCmd_Credits },
	{ L"/help", UserCmd_Help },
};

bool UserCmd_Process(uint iClientID, const std::wstring& wscCmd)
{
	auto [pluginRet, pluginSkip] = CallPluginsBefore<bool>(HookedCall::FLHook__UserCommand__Process, iClientID, wscCmd);
	if (pluginSkip)
		return pluginRet;

	std::wstring wscCmdLower = ToLower(wscCmd);

	for (uint i = 0; i < sizeof UserCmds / sizeof(USERCMD); i++)
	{
		if (wscCmdLower.find(UserCmds[i].wszCmd) == 0)
		{
			std::wstring wscParam = L"";
			if (wscCmd.length() > wcslen(UserCmds[i].wszCmd))
			{
				if (wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
					continue;
				wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
			}

			// addlog
			if (set_bLogUserCmds)
			{
				std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
				AddLog(UserLogCmds, L"%s: %s", wscCharname.c_str(), wscCmd.c_str());
			}

			try
			{
				UserCmds[i].proc(iClientID, wscParam);
				if (set_bLogUserCmds)
					AddLog(UserLogCmds, L"finished");
			}
			catch (...)
			{
				if (set_bLogUserCmds)
					AddLog(UserLogCmds, L"exception");
			}

			return true;
		}
	}

	return false;
}
