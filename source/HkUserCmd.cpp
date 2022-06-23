#include "Global.hpp"

#define PRINT_ERROR()                                                        \
	{                                                                        \
		for (uint i = 0; (i < sizeof(wscError) / sizeof(std::wstring)); i++) \
			PrintUserCmdText(iClientID, L"%s", wscError[i].c_str());         \
		return;                                                              \
	}
#define PRINT_OK() PrintUserCmdText(iClientID, L"OK");
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

void PrintUserCmdText(uint iClientID, std::wstring wscText, ...)
{
	wchar_t wszBuf[1024 * 8] = L"";
	va_list marker;
	va_start(marker, wscText);
	_vsnwprintf_s(wszBuf, sizeof wszBuf - 1, wscText.c_str(), marker);

	std::wstring wscXML = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.userCmdStyle + L"\" mask=\"-1\"/><TEXT>" + XMLText(wszBuf) + L"</TEXT>";
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

void UserCmd_SetDieMsg(const uint& iClientID, const std::wstring_view& wscParam)
{
	if (!FLHookConfig::i()->general.dieMsg)
	{
		PRINT_DISABLED()
		return;
	}

	std::wstring wscError[] = {
	    L"Error: Invalid parameters",
	    L"Usage: /set diemsg <param>",
	    L"<param>: all,system,self or none",
	};

	std::wstring wscDieMsg = ToLower(GetParam(wscParam, ' ', 0));

	DIEMSGTYPE dieMsg;
	if (wscDieMsg == L"all")
		dieMsg = DIEMSG_ALL;
	else if (wscDieMsg == L"system")
		dieMsg = DIEMSG_SYSTEM;
	else if (wscDieMsg == L"none")
		dieMsg = DIEMSG_NONE;
	else if (wscDieMsg == L"self")
		dieMsg = DIEMSG_SELF;
	else
		PRINT_ERROR()

	// save to ini
	GET_USERFILE(scUserFile)
	IniWrite(scUserFile, "settings", "DieMsg", std::to_string(dieMsg));

	// save in ClientInfo
	ClientInfo[iClientID].dieMsg = dieMsg;

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetDieMsgSize(const uint& iClientID, const std::wstring_view& wscParam)
{
	if (!FLHookConfig::i()->userCommands.userCmdSetDieMsgSize)
	{
		PRINT_DISABLED()
		return;
	}

	std::wstring wscError[] = {
	    L"Error: Invalid parameters",
	    L"Usage: /set diemsgsize <size>",
	    L"<size>: small, default",
	};

	std::wstring wscDieMsgSize = ToLower(GetParam(wscParam, ' ', 0));

	CHATSIZE dieMsgSize;
	if (!wscDieMsgSize.compare(L"small"))
		dieMsgSize = CS_SMALL;
	else if (!wscDieMsgSize.compare(L"default"))
		dieMsgSize = CS_DEFAULT;
	else
		PRINT_ERROR()

	// save to ini
	GET_USERFILE(scUserFile)
	IniWrite(scUserFile, "Settings", "DieMsgSize", std::to_string(dieMsgSize));

	// save in ClientInfo
	ClientInfo[iClientID].dieMsgSize = dieMsgSize;

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetChatFont(const uint& iClientID, const std::wstring_view& wscParam)
{
	if (!FLHookConfig::i()->userCommands.userCmdSetChatFont)
	{
		PRINT_DISABLED()
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
		PRINT_ERROR()

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
		PRINT_ERROR()

	// save to ini
	GET_USERFILE(scUserFile)
	IniWrite(scUserFile, "settings", "ChatSize", std::to_string(chatSize));
	IniWrite(scUserFile, "settings", "ChatStyle", std::to_string(chatStyle));

	// save in ClientInfo
	ClientInfo[iClientID].chatSize = chatSize;
	ClientInfo[iClientID].chatStyle = chatStyle;

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Ignore(const uint& iClientID, const std::wstring_view& wscParam)
{
	if (!FLHookConfig::i()->userCommands.userCmdIgnore)
	{
		PRINT_DISABLED()
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
		PRINT_ERROR()

	// check if flags are valid
	for (uint i = 0; i < wscFlags.length(); i++)
	{
		if (wscAllowedFlags.find_first_of(wscFlags[i]) == -1)
			PRINT_ERROR()
	}

	if (ClientInfo[iClientID].lstIgnore.size() > FLHookConfig::i()->userCommands.userCmdMaxIgnoreList)
	{
		PrintUserCmdText(iClientID, L"Error: Too many entries in the ignore list, please delete an entry first!");
		return;
	}

	// save to ini
	GET_USERFILE(scUserFile)
	IniWriteW(scUserFile, "IgnoreList", std::to_string((int)ClientInfo[iClientID].lstIgnore.size() + 1), wscCharname + L" " + wscFlags);

	// save in ClientInfo
	IGNORE_INFO ii;
	ii.wscCharname = wscCharname;
	ii.wscFlags = wscFlags;
	ClientInfo[iClientID].lstIgnore.push_back(ii);

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IgnoreID(const uint& iClientID, const std::wstring_view& wscParam)
{
	if (!FLHookConfig::i()->userCommands.userCmdIgnore)
	{
		PRINT_DISABLED()
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
		PRINT_ERROR()

	if (wscFlags.length() && wscFlags.compare(L"p") != 0)
		PRINT_ERROR()

	if (ClientInfo[iClientID].lstIgnore.size() > FLHookConfig::i()->userCommands.userCmdMaxIgnoreList)
	{
		PrintUserCmdText(iClientID, L"Error: Too many entries in the ignore list, please delete an entry first!");
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
	GET_USERFILE(scUserFile)
	IniWriteW(scUserFile, "IgnoreList", std::to_string((int)ClientInfo[iClientID].lstIgnore.size() + 1), wscCharname + L" " + wscFlags);

	// save in ClientInfo
	IGNORE_INFO ii;
	ii.wscCharname = wscCharname;
	ii.wscFlags = wscFlags;
	ClientInfo[iClientID].lstIgnore.push_back(ii);

	// send confirmation msg
	PrintUserCmdText(iClientID, L"OK, \"%s\" added to ignore list", wscCharname.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IgnoreList(const uint& iClientID, const std::wstring_view& wscParam)
{
	if (!FLHookConfig::i()->userCommands.userCmdIgnore)
	{
		PRINT_DISABLED()
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
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_DelIgnore(const uint& iClientID, const std::wstring_view& wscParam)
{
	if (!FLHookConfig::i()->userCommands.userCmdIgnore)
	{
		PRINT_DISABLED()
		return;
	}

	std::wstring wscError[] = {
	    L"Error: Invalid parameters",
	    L"Usage: /delignore <id> [<id2> <id3> ...]",
	    L"<id>: id of ignore-entry(see /ignorelist) or *(delete all)",
	};

	std::wstring wscID = GetParam(wscParam, ' ', 0);

	if (!wscID.length())
		PRINT_ERROR()

	GET_USERFILE(scUserFile)

	if (!wscID.compare(L"*"))
	{ // delete all
		IniDelSection(scUserFile, "IgnoreList");
		ClientInfo[iClientID].lstIgnore.clear();
		PRINT_OK()
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
		for (auto ignoreIt = ClientInfo[iClientID].lstIgnore.begin(); ignoreIt != ClientInfo[iClientID].lstIgnore.end(); ++ignoreIt)
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
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_AutoBuy(const uint& iClientID, const std::wstring_view& wscParam)
{
	if (!FLHookConfig::i()->general.autobuy)
	{
		PRINT_DISABLED()
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
		PrintUserCmdText(iClientID, L"Nanobots/Shield Batteries: %s", ClientInfo[iClientID].bAutoBuyReload ? L"On" : L"Off");
		return;
	}

	if (!wscType.length() || !wscSwitch.length() || wscSwitch.compare(L"on") != 0 && wscSwitch.compare(L"off") != 0)
		PRINT_ERROR()

	GET_USERFILE(scUserFile)

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
		PRINT_ERROR()

	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IDs(const uint& iClientID, const std::wstring_view& wscParam)
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

void UserCmd_ID(const uint& iClientID, const std::wstring_view& wscParam)
{
	PrintUserCmdText(iClientID, L"Your client-id: %u", iClientID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_InviteID(const uint& iClientID, const std::wstring_view& wscParam)
{
	std::wstring wscError[] = {
	    L"Error: Invalid parameters",
	    L"Usage: /i$ <client-id>",
	};

	std::wstring wscClientID = GetParam(wscParam, ' ', 0);

	if (!wscClientID.length())
		PRINT_ERROR()

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

void UserCmd_Credits(const uint& iClientID, const std::wstring_view& wscParam)
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

void UserCmd_Help(const uint& iClientID, const std::wstring_view& paramView);
UserCommand CreateUserCommand(const std::wstring& command, const std::wstring& usage, const UserCmdProc& proc, const std::wstring& description)
{
	return {command, command + L" " + usage, proc, description};
}

const std::wstring helpUsage = L"[module] [command]";
const std::wstring helpDescription = L"/help, /h, or /? will list all command modules, commands within a specific module, or information on a specific command.";
const std::vector UserCmds = {{
    CreateUserCommand(L"/set diemsg", L"", UserCmd_SetDieMsg, L""),
    CreateUserCommand(L"/set diemsgsize", L"", UserCmd_SetDieMsgSize, L""),
    CreateUserCommand(L"/set chatfont", L"", UserCmd_SetChatFont, L""),
    CreateUserCommand(L"/ignorelist", L"", UserCmd_IgnoreList, L""),
    CreateUserCommand(L"/delignore", L"", UserCmd_DelIgnore, L""),
    CreateUserCommand(L"/ignore", L"", UserCmd_Ignore, L""),
    CreateUserCommand(L"/ignoreid", L"", UserCmd_IgnoreID, L""),
    CreateUserCommand(L"/autobuy", L"", UserCmd_AutoBuy, L""),
    CreateUserCommand(L"/ids", L"", UserCmd_IDs, L""),
    CreateUserCommand(L"/id", L"", UserCmd_ID, L""),
    CreateUserCommand(L"/i$", L"", UserCmd_InviteID, L""),
    CreateUserCommand(L"/invite$", L"", UserCmd_InviteID, L""),
    CreateUserCommand(L"/credits", L"", UserCmd_Credits, L""),
    CreateUserCommand(L"/help", helpUsage, UserCmd_Help, helpDescription),
    CreateUserCommand(L"/h", helpUsage, UserCmd_Help, helpDescription),
    CreateUserCommand(L"/?", helpUsage, UserCmd_Help, helpDescription),
}};

bool GetCommand(const std::wstring& cmd, const UserCommand& userCmd)
{
	if (cmd.rfind(L'/') == 0)
	{
		return userCmd.command == cmd;
	}

	auto trimmed = userCmd.command;
	trimmed.erase(0, 1);
	return trimmed == cmd;
}

void UserCmd_Help(const uint& iClientID, const std::wstring_view& paramView)
{
	if (const auto* config = FLHookConfig::c(); !config->userCommands.userCmdHelp)
	{
		PRINT_DISABLED()
		return;
	}

	const auto& plugins = PluginManager::ir();
	if (paramView.empty() || paramView.find_first_not_of(L' ') == std::string::npos)
	{
		PrintUserCmdText(iClientID, L"The following command modules are available to you. Use /help <module> [command] for detailed information.");
		PrintUserCmdText(iClientID, L"core");
		for (const auto& plugin : plugins)
		{
			if (plugin.commands.empty())
				continue;

			PrintUserCmdText(iClientID, ToLower(stows(plugin.shortName)));
		}
		return;
	}

	const auto mod = GetParam(paramView, L' ', 0);
	const auto cmd = ToLower(Trim(GetParam(paramView, L' ', 1)));

	if (mod == L"core")
	{
		if (cmd.empty())
		{
			for (const auto& i : UserCmds)
			{
				PrintUserCmdText(iClientID, i.command);
			}
		}
		else if (const auto& userCommand =
		             std::find_if(UserCmds.begin(), UserCmds.end(), [&cmd](const UserCommand& userCmd) { return GetCommand(cmd, userCmd); });
		         userCommand != UserCmds.end())
		{
			PrintUserCmdText(iClientID, userCommand->usage);
			PrintUserCmdText(iClientID, userCommand->description);
		}
		else
		{
			PrintUserCmdText(iClientID, L"Command '%s' not found within module 'Core'", cmd.c_str());
		}
		return;
	}

	const auto& plugin =
	    std::find_if(plugins.begin(), plugins.end(), [&mod](const PluginData& plug) { return ToLower(stows(plug.shortName)) == ToLower(mod); });

	if (plugin == plugins.end())
	{
		PrintUserCmdText(iClientID, L"Command module not found.");
		return;
	}

	if (cmd.empty())
	{
		for (const auto& command : plugin->commands)
		{
			PrintUserCmdText(iClientID, command.command);
		}
	}
	else if (const auto& userCommand =
	             std::find_if(plugin->commands.begin(), plugin->commands.end(), [&cmd](const UserCommand& userCmd) { return GetCommand(cmd, userCmd); });
	         userCommand != plugin->commands.end())
	{
		PrintUserCmdText(iClientID, userCommand->usage);
		PrintUserCmdText(iClientID, userCommand->description);
	}
	else
	{
		PrintUserCmdText(iClientID, L"Command '%s' not found within module '%s'", cmd.c_str(), stows(plugin->shortName).c_str());
	}
}

bool ProcessPluginCommand(const uint& clientId, const std::wstring& cmd, const std::vector<UserCommand>& commands)
{
	const std::wstring cmdLower = ToLower(cmd);
	for (const auto& command : commands)
	{
		if (cmdLower.find(command.command) == 0)
		{
			std::wstring wscParam;
			if (cmd.length() > command.command.length())
			{
				if (cmd[command.command.length()] != ' ')
				{
					continue;
				}
				wscParam = cmd.substr(command.command.length() + 1);
			}

			std::wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(clientId);
			AddLog(LogType::UserLogCmds, LogLevel::Info, L"%s: %s", wscCharname.c_str(), cmd.c_str());

			try
			{
				const auto view = std::wstring_view(wscParam);
				command.proc(clientId, view);
				AddLog(LogType::UserLogCmds, LogLevel::Info, L"finished");
			}
			catch (std::exception const& ex)
			{
				AddLog(LogType::UserLogCmds, LogLevel::Info, L"exception %s", stows(ex.what()).c_str());
			}

			return true;
		}
	}
	return false;
};

bool UserCmd_Process(uint iClientID, const std::wstring& wscCmd)
{
	if (const auto& config = FLHookConfig::c(); config->general.echoCommands)
	{
		std::wstring lower = ToLower(GetParam(wscCmd, ' ', 0));
		if ((lower.find(L'/') == 0 || lower.find(L'.') == 0) &&
		    !(lower == L"/l" || lower == L"/local" || lower == L"/s" || lower == L"/system" || lower == L"/g" || lower == L"/group" || lower == L"/t" ||
		        lower == L"/target" || lower == L"/r" || lower == L"/reply" || lower.find(L"//") == 0 || lower.find(L'*') == lower.length() - 1))
		{
			const std::wstring wscXML = L"<TRA data=\"" + config->msgStyle.msgEchoStyle + L"\" mask=\"-1\"/><TEXT>" + XMLText(wscCmd) + L"</TEXT>";
			HkFMsg(iClientID, wscXML);
		}
	}

	auto [pluginRet, pluginSkip] = CallPluginsBefore<bool>(HookedCall::FLHook__UserCommand__Process, iClientID, wscCmd);
	if (pluginSkip)
		return pluginRet;

	const auto& plugins = PluginManager::ir();
	for (const PluginData& i : plugins)
	{
		if (ProcessPluginCommand(iClientID, wscCmd, i.commands))
			return true;
	}

	// In-built commands
	return ProcessPluginCommand(iClientID, wscCmd, UserCmds);
}
