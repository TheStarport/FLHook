#include "Global.hpp"

#define PRINT_ERROR()                                                        \
	{                                                                        \
		for (uint i = 0; (i < sizeof(wscError) / sizeof(std::wstring)); i++) \
			PrintUserCmdText(clientId, L"%s", wscError[i].c_str());         \
		return;                                                              \
	}
#define PRINT_OK() PrintUserCmdText(clientId, L"OK");
#define PRINT_DISABLED() PrintUserCmdText(clientId, L"Command disabled");
#define GET_USERFILE(a)                                             \
	std::string a;                                                  \
	{                                                               \
		CAccount* acc = Players.FindAccountFromClientID(clientId); \
		std::wstring dir = Hk::Client::GetAccountDirName(acc);                           \
		a = scAcctPath + wstos(dir) + "\\flhookuser.ini";        \
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintUserCmdText(uint clientId, std::wstring text, ...)
{
	wchar_t wszBuf[1024 * 8] = L"";
	va_list marker;
	va_start(marker, text);
	_vsnwprintf_s(wszBuf, sizeof wszBuf - 1, text.c_str(), marker);

	std::wstring wscXML = L"<TRA data=\"" + FLHookConfig::i()->msgStyle.userCmdStyle + L"\" mask=\"-1\"/><TEXT>" + XMLText(wszBuf) + L"</TEXT>";
	Hk::Message::FMsg(clientId, wscXML);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Print message to all ships within the specific number of meters of the
/// player.
void PrintLocalUserCmdText(uint clientId, const std::wstring& wscMsg, float fDistance)
{
	uint iShip;
	pub::Player::GetShip(clientId, iShip);

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(iShip, pos, rot);

	uint iSystem;
	pub::Player::GetSystem(clientId, iSystem);

	// For all players in system...
	struct PlayerData* playerDb = 0;
	while (playerDb = Players.traverse_active(playerDb))
	{
		// Get the this player's current system and location in the system.
		uint clientId2 = playerDb->iOnlineID;
		uint iSystem2 = 0;
		pub::Player::GetSystem(clientId2, iSystem2);
		if (iSystem != iSystem2)
			continue;

		uint iShip2;
		pub::Player::GetShip(clientId2, iShip2);

		Vector pos2;
		Matrix rot2;
		pub::SpaceObj::GetLocation(iShip2, pos2, rot2);

		// Is player within the specified range of the sending char.
		if (Hk::Math::Distance3D(pos, pos2) > fDistance)
			continue;

		PrintUserCmdText(clientId2, wscMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetDieMsg(const uint& clientId, const std::wstring_view& wscParam)
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
	ClientInfo[clientId].dieMsg = dieMsg;

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetDieMsgSize(const uint& clientId, const std::wstring_view& wscParam)
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
	ClientInfo[clientId].dieMsgSize = dieMsgSize;

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetChatFont(const uint& clientId, const std::wstring_view& wscParam)
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
	ClientInfo[clientId].chatSize = chatSize;
	ClientInfo[clientId].chatStyle = chatStyle;

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Ignore(const uint& clientId, const std::wstring_view& wscParam)
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

	std::wstring character = GetParam(wscParam, ' ', 0);
	std::wstring wscFlags = ToLower(GetParam(wscParam, ' ', 1));

	if (!character.length())
		PRINT_ERROR()

	// check if flags are valid
	for (uint i = 0; i < wscFlags.length(); i++)
	{
		if (wscAllowedFlags.find_first_of(wscFlags[i]) == -1)
			PRINT_ERROR()
	}

	if (ClientInfo[clientId].lstIgnore.size() > FLHookConfig::i()->userCommands.userCmdMaxIgnoreList)
	{
		PrintUserCmdText(clientId, L"Error: Too many entries in the ignore list, please delete an entry first!");
		return;
	}

	// save to ini
	GET_USERFILE(scUserFile)
	IniWriteW(scUserFile, "IgnoreList", std::to_string((int)ClientInfo[clientId].lstIgnore.size() + 1), character + L" " + wscFlags);

	// save in ClientInfo
	IGNORE_INFO ii;
	ii.character = character;
	ii.wscFlags = wscFlags;
	ClientInfo[clientId].lstIgnore.push_back(ii);

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IgnoreID(const uint& clientId, const std::wstring_view& wscParam)
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

	if (ClientInfo[clientId].lstIgnore.size() > FLHookConfig::i()->userCommands.userCmdMaxIgnoreList)
	{
		PrintUserCmdText(clientId, L"Error: Too many entries in the ignore list, please delete an entry first!");
		return;
	}

	uint clientIdTarget = ToInt(wscClientID);
	if (!Hk::Client::IsValidClientID(clientIdTarget) || Hk::Client::IsInCharSelectMenu(clientIdTarget))
	{
		PrintUserCmdText(clientId, L"Error: Invalid client-id");
		return;
	}

	std::wstring character = (wchar_t*)Players.GetActiveCharacterName(clientIdTarget);

	// save to ini
	GET_USERFILE(scUserFile)
	IniWriteW(scUserFile, "IgnoreList", std::to_string((int)ClientInfo[clientId].lstIgnore.size() + 1), character + L" " + wscFlags);

	// save in ClientInfo
	IGNORE_INFO ii;
	ii.character = character;
	ii.wscFlags = wscFlags;
	ClientInfo[clientId].lstIgnore.push_back(ii);

	// send confirmation msg
	PrintUserCmdText(clientId, L"OK, \"%s\" added to ignore list", character.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IgnoreList(const uint& clientId, const std::wstring_view& wscParam)
{
	if (!FLHookConfig::i()->userCommands.userCmdIgnore)
	{
		PRINT_DISABLED()
		return;
	}

	PrintUserCmdText(clientId, L"ID | Charactername | Flags");
	int i = 1;
	for (auto& ignore : ClientInfo[clientId].lstIgnore)
	{
		PrintUserCmdText(clientId, L"%.2u | %s | %s", i, ignore.character.c_str(), ignore.wscFlags.c_str());
		i++;
	}

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_DelIgnore(const uint& clientId, const std::wstring_view& wscParam)
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
		ClientInfo[clientId].lstIgnore.clear();
		PRINT_OK()
		return;
	}

	std::list<uint> lstDelete;
	for (uint j = 1; wscID.length(); j++)
	{
		uint iID = ToInt(wscID.c_str());
		if (!iID || iID > ClientInfo[clientId].lstIgnore.size())
		{
			PrintUserCmdText(clientId, L"Error: Invalid ID");
			return;
		}

		lstDelete.push_back(iID);
		wscID = GetParam(wscParam, ' ', j);
	}

	lstDelete.sort(std::greater<uint>());

	ClientInfo[clientId].lstIgnore.reverse();
	for (auto& del : lstDelete)
	{
		uint iCurID = (uint)ClientInfo[clientId].lstIgnore.size();
		for (auto ignoreIt = ClientInfo[clientId].lstIgnore.begin(); ignoreIt != ClientInfo[clientId].lstIgnore.end(); ++ignoreIt)
		{
			if (iCurID == del)
			{
				ClientInfo[clientId].lstIgnore.erase(ignoreIt);
				break;
			}
			iCurID--;
		}
	}
	ClientInfo[clientId].lstIgnore.reverse();

	// send confirmation msg
	IniDelSection(scUserFile, "IgnoreList");
	int i = 1;
	for (auto& ignore : ClientInfo[clientId].lstIgnore)
	{
		IniWriteW(scUserFile, "IgnoreList", std::to_string(i), ignore.character + L" " + ignore.wscFlags);
		i++;
	}
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_AutoBuy(const uint& clientId, const std::wstring_view& wscParam)
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
		PrintUserCmdText(clientId, L"Missiles: %s", ClientInfo[clientId].bAutoBuyMissiles ? L"On" : L"Off");
		PrintUserCmdText(clientId, L"Mine: %s", ClientInfo[clientId].bAutoBuyMines ? L"On" : L"Off");
		PrintUserCmdText(clientId, L"Torpedos: %s", ClientInfo[clientId].bAutoBuyTorps ? L"On" : L"Off");
		PrintUserCmdText(clientId, L"Cruise Disruptors: %s", ClientInfo[clientId].bAutoBuyCD ? L"On" : L"Off");
		PrintUserCmdText(clientId, L"Countermeasures: %s", ClientInfo[clientId].bAutoBuyCM ? L"On" : L"Off");
		PrintUserCmdText(clientId, L"Nanobots/Shield Batteries: %s", ClientInfo[clientId].bAutoBuyReload ? L"On" : L"Off");
		return;
	}

	if (!wscType.length() || !wscSwitch.length() || wscSwitch.compare(L"on") != 0 && wscSwitch.compare(L"off") != 0)
		PRINT_ERROR()

	GET_USERFILE(scUserFile)

	const auto fileName = Hk::Client::GetCharFileName(clientId);
	std::string scSection = "autobuy_" + wstos(fileName.value());

	bool bEnable = !wscSwitch.compare(L"on") ? true : false;
	if (!wscType.compare(L"all"))
	{
		ClientInfo[clientId].bAutoBuyMissiles = bEnable;
		ClientInfo[clientId].bAutoBuyMines = bEnable;
		ClientInfo[clientId].bAutoBuyTorps = bEnable;
		ClientInfo[clientId].bAutoBuyCD = bEnable;
		ClientInfo[clientId].bAutoBuyCM = bEnable;
		ClientInfo[clientId].bAutoBuyReload = bEnable;
		IniWrite(scUserFile, scSection, "missiles", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "mines", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "torps", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "cd", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "cm", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "reload", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"missiles"))
	{
		ClientInfo[clientId].bAutoBuyMissiles = bEnable;
		IniWrite(scUserFile, scSection, "missiles", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"mines"))
	{
		ClientInfo[clientId].bAutoBuyMines = bEnable;
		IniWrite(scUserFile, scSection, "mines", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"torps"))
	{
		ClientInfo[clientId].bAutoBuyTorps = bEnable;
		IniWrite(scUserFile, scSection, "torps", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"cd"))
	{
		ClientInfo[clientId].bAutoBuyCD = bEnable;
		IniWrite(scUserFile, scSection, "cd", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"cm"))
	{
		ClientInfo[clientId].bAutoBuyCM = bEnable;
		IniWrite(scUserFile, scSection, "cm", bEnable ? "yes" : "no");
	}
	else if (!wscType.compare(L"reload"))
	{
		ClientInfo[clientId].bAutoBuyReload = bEnable;
		IniWrite(scUserFile, scSection, "reload", bEnable ? "yes" : "no");
	}
	else
		PRINT_ERROR()

	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IDs(const uint& clientId, const std::wstring_view& wscParam)
{
	wchar_t wszLine[128] = L"";
	for (auto& player : Hk::Admin::GetPlayers())
	{
		wchar_t wszBuf[1024];
		swprintf_s(wszBuf, L"%s = %u | ", player.character.c_str(), player.clientId);
		if (wcslen(wszBuf) + wcslen(wszLine) >= sizeof wszLine / 2)
		{
			PrintUserCmdText(clientId, L"%s", wszLine);
			wcscpy_s(wszLine, wszBuf);
		}
		else
			wcscat_s(wszLine, wszBuf);
	}

	if (wcslen(wszLine))
		PrintUserCmdText(clientId, L"%s", wszLine);
	PrintUserCmdText(clientId, L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_ID(const uint& clientId, const std::wstring_view& wscParam)
{
	PrintUserCmdText(clientId, L"Your client-id: %u", clientId);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_InviteID(const uint& clientId, const std::wstring_view& wscParam)
{
	std::wstring wscError[] = {
	    L"Error: Invalid parameters",
	    L"Usage: /i$ <client-id>",
	};

	std::wstring wscClientID = GetParam(wscParam, ' ', 0);

	if (!wscClientID.length())
		PRINT_ERROR()

	uint clientIdTarget = ToInt(wscClientID);
	if (!Hk::Client::IsValidClientID(clientIdTarget) || Hk::Client::IsInCharSelectMenu(clientIdTarget))
	{
		PrintUserCmdText(clientId, L"Error: Invalid client-id");
		return;
	}

	std::wstring character = (wchar_t*)Players.GetActiveCharacterName(clientIdTarget);

	std::wstring wscXML = L"<TEXT>/i " + XMLText(character) + L"</TEXT>";
	char szBuf[0xFFFF];
	uint iRet;
	if (!Hk::Message::FMsgEncodeXML(wscXML, szBuf, sizeof szBuf, iRet).has_error())
	{
		PrintUserCmdText(clientId, L"Error: Could not encode XML");
		return;
	}

	CHAT_ID cID;
	cID.iID = clientId;
	CHAT_ID cIDTo;
	cIDTo.iID = 0x00010001;
	Server.SubmitChat(cID, iRet, szBuf, cIDTo, -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Credits(const uint& clientId, const std::wstring_view& wscParam)
{
	PrintUserCmdText(clientId, L"This server is running FLHook v" + VersionInformation);
	PrintUserCmdText(clientId, L"Running plugins:");

	bool bRunning = false;
	for (const auto& plugin : PluginManager::ir())
	{
		if (plugin.paused)
			continue;

		bRunning = true;
		PrintUserCmdText(clientId, L"- %s", stows(plugin.name).c_str());
	}
	if (!bRunning)
		PrintUserCmdText(clientId, L"- none -");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Help(const uint& clientId, const std::wstring_view& paramView);
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

void UserCmd_Help(const uint& clientId, const std::wstring_view& paramView)
{
	if (const auto* config = FLHookConfig::c(); !config->userCommands.userCmdHelp)
	{
		PRINT_DISABLED()
		return;
	}

	const auto& plugins = PluginManager::ir();
	if (paramView.empty() || paramView.find_first_not_of(L' ') == std::string::npos)
	{
		PrintUserCmdText(clientId, L"The following command modules are available to you. Use /help <module> [command] for detailed information.");
		PrintUserCmdText(clientId, L"core");
		for (const auto& plugin : plugins)
		{
			if (plugin.commands.empty())
				continue;

			PrintUserCmdText(clientId, ToLower(stows(plugin.shortName)));
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
				PrintUserCmdText(clientId, i.command);
			}
		}
		else if (const auto& userCommand =
		             std::find_if(UserCmds.begin(), UserCmds.end(), [&cmd](const UserCommand& userCmd) { return GetCommand(cmd, userCmd); });
		         userCommand != UserCmds.end())
		{
			PrintUserCmdText(clientId, userCommand->usage);
			PrintUserCmdText(clientId, userCommand->description);
		}
		else
		{
			PrintUserCmdText(clientId, L"Command '%s' not found within module 'Core'", cmd.c_str());
		}
		return;
	}

	const auto& plugin =
	    std::find_if(plugins.begin(), plugins.end(), [&mod](const PluginData& plug) { return ToLower(stows(plug.shortName)) == ToLower(mod); });

	if (plugin == plugins.end())
	{
		PrintUserCmdText(clientId, L"Command module not found.");
		return;
	}

	if (cmd.empty())
	{
		for (const auto& command : plugin->commands)
		{
			PrintUserCmdText(clientId, command.command);
		}
	}
	else if (const auto& userCommand =
	             std::find_if(plugin->commands.begin(), plugin->commands.end(), [&cmd](const UserCommand& userCmd) { return GetCommand(cmd, userCmd); });
	         userCommand != plugin->commands.end())
	{
		PrintUserCmdText(clientId, userCommand->usage);
		PrintUserCmdText(clientId, userCommand->description);
	}
	else
	{
		PrintUserCmdText(clientId, L"Command '%s' not found within module '%s'", cmd.c_str(), stows(plugin->shortName).c_str());
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

			std::wstring character = (wchar_t*)Players.GetActiveCharacterName(clientId);
			AddLog(LogType::UserLogCmds, LogLevel::Info, L"%s: %s", character.c_str(), cmd.c_str());

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

bool UserCmd_Process(uint clientId, const std::wstring& wscCmd)
{
	if (const auto& config = FLHookConfig::c(); config->general.echoCommands)
	{
		std::wstring lower = ToLower(GetParam(wscCmd, ' ', 0));
		if ((lower.find(L'/') == 0 || lower.find(L'.') == 0) &&
		    !(lower == L"/l" || lower == L"/local" || lower == L"/s" || lower == L"/system" || lower == L"/g" || lower == L"/group" || lower == L"/t" ||
		        lower == L"/target" || lower == L"/r" || lower == L"/reply" || lower.find(L"//") == 0 || lower.find(L'*') == lower.length() - 1))
		{
			const std::wstring wscXML = L"<TRA data=\"" + config->msgStyle.msgEchoStyle + L"\" mask=\"-1\"/><TEXT>" + XMLText(wscCmd) + L"</TEXT>";
			Hk::Message::FMsg(clientId, wscXML);
		}
	}

	auto [pluginRet, pluginSkip] = CallPluginsBefore<bool>(HookedCall::FLHook__UserCommand__Process, clientId, wscCmd);
	if (pluginSkip)
		return pluginRet;

	const auto& plugins = PluginManager::ir();
	for (const PluginData& i : plugins)
	{
		if (ProcessPluginCommand(clientId, wscCmd, i.commands))
			return true;
	}

	// In-built commands
	return ProcessPluginCommand(clientId, wscCmd, UserCmds);
}
