#include "Global.hpp"
#include "Features/Mail.hpp"

#define PRINT_ERROR()                                                     \
	{                                                                     \
		for (uint i = 0; (i < sizeof(error) / sizeof(std::wstring)); i++) \
			PrintUserCmdText(client, std::format(L"{}", error[i]));       \
		return;                                                           \
	}
#define PRINT_OK() PrintUserCmdText(client, L"OK");
#define PRINT_DISABLED() PrintUserCmdText(client, L"Command disabled");
#define GET_USERFILE(a)                                                  \
	std::string a;                                                       \
	{                                                                    \
		CAccount* acc = Players.FindAccountFromClientID(client);         \
		std::wstring dir = Hk::Client::GetAccountDirName(acc);           \
		a = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini"; \
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintUserCmdText(ClientId client, const std::wstring& text)
{
	auto newLineChar = text.find(L"\n");
	if (newLineChar == std::wstring::npos)
	{
		std::wstring xml = std::format(L"<TRA data=\"{}\" mask=\"-1\"/><TEXT>{}</TEXT>", FLHookConfig::i()->messages.msgStyle.userCmdStyle, XMLText(text));
		Hk::Message::FMsg(client, xml);
	}
	else
	{
		//Split text into two strings, one from the beginning to the character before newLineChar, and one after newLineChar till the end.
		//It will then recursively call itself for each new line char until the original text is all displayed. 
		PrintUserCmdText(client, text.substr(0, newLineChar - 1));
		PrintUserCmdText(client, text.substr(newLineChar + 1, std::wstring::npos));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Print message to all ships within the specific number of meters of the
/// player.
void PrintLocalUserCmdText(ClientId client, const std::wstring& wscMsg, float fDistance)
{
	uint ship;
	pub::Player::GetShip(client, ship);

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(ship, pos, rot);

	uint iSystem;
	pub::Player::GetSystem(client, iSystem);

	// For all players in system...
	PlayerData* playerDb = nullptr;
	while ((playerDb = Players.traverse_active(playerDb)))
	{
		// Get the this player's current system and location in the system.
		ClientId client2 = playerDb->iOnlineId;
		uint iSystem2 = 0;
		pub::Player::GetSystem(client2, iSystem2);
		if (iSystem != iSystem2)
			continue;

		uint ship2;
		pub::Player::GetShip(client2, ship2);

		Vector pos2;
		Matrix rot2;
		pub::SpaceObj::GetLocation(ship2, pos2, rot2);

		// Is player within the specified range of the sending char.
		if (Hk::Math::Distance3D(pos, pos2) > fDistance)
			continue;

		PrintUserCmdText(client2, wscMsg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetDieMsg(ClientId& client, const std::wstring& wscParam)
{
	if (!FLHookConfig::i()->messages.dieMsg)
	{
		PRINT_DISABLED()
		return;
	}

	std::wstring error[] = {
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
	ClientInfo[client].dieMsg = dieMsg;

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetDieMsgSize(ClientId& client, const std::wstring& param)
{
	if (!FLHookConfig::i()->userCommands.userCmdSetDieMsgSize)
	{
		PRINT_DISABLED()
		return;
	}

	std::wstring error[] = {
	    L"Error: Invalid parameters",
	    L"Usage: /set diemsgsize <size>",
	    L"<size>: small, default",
	};

	std::wstring wscDieMsgSize = ToLower(GetParam(param, ' ', 0));

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
	ClientInfo[client].dieMsgSize = dieMsgSize;

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetChatFont(ClientId& client, const std::wstring& wscParam)
{
	if (!FLHookConfig::i()->userCommands.userCmdSetChatFont)
	{
		PRINT_DISABLED()
		return;
	}

	std::wstring error[] = {
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
	ClientInfo[client].chatSize = chatSize;
	ClientInfo[client].chatStyle = chatStyle;

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Ignore(ClientId& client, const std::wstring& param)
{
	if (!FLHookConfig::i()->userCommands.userCmdIgnore)
	{
		PRINT_DISABLED()
		return;
	}

	std::wstring error[] = {
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
	    L"charname contain \"idiot\" (e.g. \"[XYZ]IdIOT\", \"MrIdiot\", etc)",
	    L"\"/ignore Fool pi\" ignores all private-chatmessages from players "
	    L"whose charname contain \"fool\"",
	};

	std::wstring allowedFlags = L"pi";

	std::wstring character = GetParam(param, ' ', 0);
	std::wstring flags = ToLower(GetParam(param, ' ', 1));

	if (!character.length())
		PRINT_ERROR()

	// check if flags are valid
	for (auto flag : flags)
	{
		if (allowedFlags.find_first_of(flag) == -1)
			PRINT_ERROR()
	}

	if (ClientInfo[client].lstIgnore.size() > FLHookConfig::i()->userCommands.userCmdMaxIgnoreList)
	{
		PrintUserCmdText(client, L"Error: Too many entries in the ignore list, please delete an entry first!");
		return;
	}

	// save to ini
	GET_USERFILE(scUserFile)
	IniWriteW(scUserFile, "IgnoreList", std::to_string((int)ClientInfo[client].lstIgnore.size() + 1), character + L" " + flags);

	// save in ClientInfo
	IGNORE_INFO ii;
	ii.character = character;
	ii.wscFlags = flags;
	ClientInfo[client].lstIgnore.push_back(ii);

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IgnoreID(ClientId& client, const std::wstring& param)
{
	if (!FLHookConfig::i()->userCommands.userCmdIgnore)
	{
		PRINT_DISABLED()
		return;
	}

	std::wstring error[] = {
	    L"Error: Invalid parameters",
	    L"Usage: /ignoreid <client-id> [<flags>]",
	    L"<client-id>: client-id of character which should be ignored",
	    L"<flags>: if \"p\"(without quotation marks) then only affect private "
	    L"chat",
	};

	std::wstring wscClientId = GetParam(param, ' ', 0);
	std::wstring wscFlags = ToLower(GetParam(param, ' ', 1));

	if (!wscClientId.length())
		PRINT_ERROR()

	if (wscFlags.length() && wscFlags.compare(L"p") != 0)
		PRINT_ERROR()

	if (ClientInfo[client].lstIgnore.size() > FLHookConfig::i()->userCommands.userCmdMaxIgnoreList)
	{
		PrintUserCmdText(client, L"Error: Too many entries in the ignore list, please delete an entry first!");
		return;
	}

	ClientId clientTarget = ToInt(wscClientId);
	if (!Hk::Client::IsValidClientID(clientTarget) || Hk::Client::IsInCharSelectMenu(clientTarget))
	{
		PrintUserCmdText(client, L"Error: Invalid client-id");
		return;
	}

	std::wstring character = Hk::Client::GetCharacterNameByID(clientTarget).value();

	// save to ini
	GET_USERFILE(scUserFile)
	IniWriteW(scUserFile, "IgnoreList", std::to_string((int)ClientInfo[client].lstIgnore.size() + 1), character + L" " + wscFlags);

	// save in ClientInfo
	IGNORE_INFO ii;
	ii.character = character;
	ii.wscFlags = wscFlags;
	ClientInfo[client].lstIgnore.push_back(ii);

	// send confirmation msg
	PrintUserCmdText(client, std::format(L"OK, \"{}\" added to ignore list", character));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IgnoreList(ClientId& client, [[maybe_unused]] const std::wstring& param)
{
	if (!FLHookConfig::i()->userCommands.userCmdIgnore)
	{
		PRINT_DISABLED()
		return;
	}

	PrintUserCmdText(client, L"Id | Charactername | Flags");
	int i = 1;
	for (auto& ignore : ClientInfo[client].lstIgnore)
	{
		PrintUserCmdText(client, std::format(L"{} | %s | %s", i, ignore.character.c_str(), ignore.wscFlags));
		i++;
	}

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_DelIgnore(ClientId& client, const std::wstring& param)
{
	if (!FLHookConfig::i()->userCommands.userCmdIgnore)
	{
		PRINT_DISABLED()
		return;
	}

	std::wstring error[] = {
	    L"Error: Invalid parameters",
	    L"Usage: /delignore <id> [<id2> <id3> ...]",
	    L"<id>: id of ignore-entry(see /ignorelist) or *(delete all)",
	};

	std::wstring wscId = GetParam(param, ' ', 0);

	if (wscId.empty())
		PRINT_ERROR()

	GET_USERFILE(scUserFile)

	if (!wscId.compare(L"*"))
	{ // delete all
		IniDelSection(scUserFile, "IgnoreList");
		ClientInfo[client].lstIgnore.clear();
		PRINT_OK()
		return;
	}

	std::list<uint> lstDelete;
	for (uint j = 1; !wscId.empty(); j++)
	{
		uint iId = ToInt(wscId.c_str());
		if (!iId || iId > ClientInfo[client].lstIgnore.size())
		{
			PrintUserCmdText(client, L"Error: Invalid Id");
			return;
		}

		lstDelete.push_back(iId);
		wscId = GetParam(param, ' ', j);
	}

	lstDelete.sort(std::greater<uint>());

	ClientInfo[client].lstIgnore.reverse();
	for (const auto& del : lstDelete)
	{
		uint iCurId = ClientInfo[client].lstIgnore.size();
		for (auto ignoreIt = ClientInfo[client].lstIgnore.begin(); ignoreIt != ClientInfo[client].lstIgnore.end(); ++ignoreIt)
		{
			if (iCurId == del)
			{
				ClientInfo[client].lstIgnore.erase(ignoreIt);
				break;
			}
			iCurId--;
		}
	}
	ClientInfo[client].lstIgnore.reverse();

	// send confirmation msg
	IniDelSection(scUserFile, "IgnoreList");
	int i = 1;
	for (const auto& ignore : ClientInfo[client].lstIgnore)
	{
		IniWriteW(scUserFile, "IgnoreList", std::to_string(i), ignore.character + L" " + ignore.wscFlags);
		i++;
	}
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Ids(ClientId& client, [[maybe_unused]] const std::wstring& wscParam)
{
	for (auto& player : Hk::Admin::GetPlayers())
	{
		PrintUserCmdText(client, std::format(L"{} = {} | ", player.character, player.client));
	}
	PrintUserCmdText(client, L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_ID(ClientId& client, [[maybe_unused]] const std::wstring& wscParam)
{
	PrintUserCmdText(client, std::format(L"Your client-id: {}", client));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_InviteID(ClientId& client, const std::wstring& param)
{
	std::wstring error[] = {
	    L"Error: Invalid parameters",
	    L"Usage: /i$ <client-id>",
	};

	std::wstring wscClientId = GetParam(param, ' ', 0);

	if (!wscClientId.length())
		PRINT_ERROR()

	ClientId clientTarget = ToInt(wscClientId);
	if (!Hk::Client::IsValidClientID(clientTarget) || Hk::Client::IsInCharSelectMenu(clientTarget))
	{
		PrintUserCmdText(client, L"Error: Invalid client-id");
		return;
	}

	std::wstring character = Hk::Client::GetCharacterNameByID(clientTarget).value();

	std::wstring wscXML = L"<TEXT>/i " + XMLText(character) + L"</TEXT>";
	char szBuf[0xFFFF];
	uint iRet;
	if (Hk::Message::FMsgEncodeXML(wscXML, szBuf, sizeof szBuf, iRet).has_error())
	{
		PrintUserCmdText(client, L"Error: Could not encode XML");
		return;
	}

	CHAT_ID cId;
	cId.iId = client;
	CHAT_ID cIdTo;
	cIdTo.iId = 0x00010001;
	Server.SubmitChat(cId, iRet, szBuf, cIdTo, -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Credits(ClientId& client, [[maybe_unused]] const std::wstring& param)
{
	PrintUserCmdText(client, L"This server is running FLHook v" + VersionInformation);
	PrintUserCmdText(client, L"Running plugins:");

	bool bRunning = false;
	for (const auto& plugin : PluginManager::ir())
	{
		if (plugin->paused)
			continue;

		bRunning = true;
		PrintUserCmdText(client, std::format(L"- {}", stows(plugin->name)));
	}
	if (!bRunning)
		PrintUserCmdText(client, L"- none -");
}

void UserCmdDelMail(ClientId& client, const std::wstring& param)
{
	// /maildel <id/all> [readonly]
	const auto str = GetParam(param, ' ', 0);
	if (str == L"all")
	{
		const auto count = MailManager::i()->PurgeAllMail(client, GetParam(param, ' ', 1) == L"readonly");
		if (count.has_error())
		{
			PrintUserCmdText(client, std::format(L"Error deleting mail: {}", stows(count.error())));
			return;
		}

		PrintUserCmdText(client, std::format(L"Deleted {} mail", count.value()));
	}
	else
	{
		const auto index = ToInt64(str);
		if (const auto err = MailManager::i()->DeleteMail(client, index); err.has_error())
		{
			PrintUserCmdText(client, std::format(L"Error deleting mail: {}", stows(err.error())));
			return;
		}

		PrintUserCmdText(client, L"Mail deleted");
	}
}

void UserCmdReadMail(ClientId& client, const std::wstring& param)
{
	const auto index = ToInt64(GetParam(param, ' ', 0));
	if (index <= 0)
	{
		PrintUserCmdText(client, L"Id was not provided or was invalid.");
		return;
	}

	const auto mail = MailManager::i()->GetMailById(client, index);
	if (mail.has_error())
	{
		PrintUserCmdText(client, std::format(L"Error retreiving mail: {}", stows(mail.error())));
		return;
	}

	const auto& item = mail.value();
	PrintUserCmdText(client, std::format(L"From: {}", stows(item.author)));
	PrintUserCmdText(client, std::format(L"Subject: {}", stows(item.subject)));
	PrintUserCmdText(client, std::format(L"Date: {:%F %T}", UnixToSysTime(item.timestamp)));
	PrintUserCmdText(client, stows(item.body));
}

void UserCmdListMail(ClientId& client, const std::wstring& param)
{
	const auto page = ToInt(GetParam(param, ' ', 0));
	if (page <= 0)
	{
		PrintUserCmdText(client, L"Page was not provided or was invalid.");
		return;
	}

	const bool unreadOnly = GetParam(param, ' ', 1) == L"unread";

	const auto mail = MailManager::i()->GetMail(client, unreadOnly, page);
	if (mail.has_error())
	{
		PrintUserCmdText(client, std::format(L"Error retreiving mail: {}", stows(mail.error())));
		return;
	}

	const auto& mailList = mail.value();
	if (mailList.empty())
	{
		PrintUserCmdText(client, L"You have no mail.");
		return;
	}

	PrintUserCmdText(client, std::format(L"Printing mail of page {}", mailList.size()));
	for (const auto& item : mailList)
	{
		// |    Id.) Subject (unread) - Author - Time
		PrintUserCmdText(client,
		    stows(std::format(
		        "|    {}.) {} {}- {} - {:%F %T}", item.id, item.subject, item.unread ? "(unread) " : "", item.author, UnixToSysTime(item.timestamp))));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Help(ClientId& client, const std::wstring& paramView);
UserCommand CreateUserCommand(const std::variant<std::wstring, std::vector<std::wstring>>& command, const std::wstring& usage,
    const std::variant<UserCmdProc, UserCmdProcWithParam>& proc, const std::wstring& description)
{
	return {command, usage, proc, description};
}

std::vector<std::wstring> CmdArr(std::initializer_list<std::wstring> cmds)
{
	return cmds;
}

const std::wstring helpUsage = L"/help [module] [command]";
const std::wstring helpDescription =
    L"/help, /h, or /? will list all command modules, commands within a specific module, or information on a specific command.";
const std::vector UserCmds = {{
    CreateUserCommand(L"/set diemsg", L"", UserCmd_SetDieMsg, L""),
    CreateUserCommand(L"/set diemsgsize", L"", UserCmd_SetDieMsgSize, L""),
    CreateUserCommand(L"/set chatfont", L"", UserCmd_SetChatFont, L""),
    CreateUserCommand(L"/ignorelist", L"", UserCmd_IgnoreList, L""),
    CreateUserCommand(L"/delignore", L"", UserCmd_DelIgnore, L""),
    CreateUserCommand(L"/ignore", L"", UserCmd_Ignore, L""),
    CreateUserCommand(L"/ignoreid", L"", UserCmd_IgnoreID, L""),
    CreateUserCommand(L"/ids", L"", UserCmd_Ids, L""),
    CreateUserCommand(L"/id", L"", UserCmd_ID, L""),
    CreateUserCommand(L"/i$", L"", UserCmd_InviteID, L""),
    CreateUserCommand(L"/invite$", L"", UserCmd_InviteID, L""),
    CreateUserCommand(L"/credits", L"", UserCmd_Credits, L""),
    CreateUserCommand(L"/maildel", L"/maildel <id/all> [readonly]", UserCmdDelMail,
        L"Delete the specified mail, or if all is provided delete all mail. If all is specified with the param of readonly, unread mail will be preserved."),
    CreateUserCommand(L"/mailread", L"/mailread <id>", UserCmdReadMail, L"Read the specified mail."),
    CreateUserCommand(L"/maillist", L"/maillist <page> [unread]", UserCmdListMail,
        L"List the mail items on the specified page. If unread is specified, only mail that hasn't been read will be listed."),
    CreateUserCommand(CmdArr({L"/help", L"/h", L"/?"}), helpUsage, UserCmd_Help, helpDescription),
}};

bool GetCommand(const std::wstring& cmd, const UserCommand& userCmd)
{
	const auto isMatch = [&cmd](const std::wstring& subCmd) {
		if (cmd.rfind(L'/') == 0)
		{
			return subCmd == cmd;
		}

		auto trimmed = subCmd;
		trimmed.erase(0, 1);
		return trimmed == cmd;
	};

	if (userCmd.command.index() == 0)
	{
		return isMatch(std::get<std::wstring>(userCmd.command));
	}
	else
	{
		const auto& arr = std::get<std::vector<std::wstring>>(userCmd.command);
		return std::ranges::any_of(arr, isMatch);
	}
}

void UserCmd_Help(ClientId& client, const std::wstring& paramView)
{
	if (const auto* config = FLHookConfig::c(); !config->userCommands.userCmdHelp)
	{
		PRINT_DISABLED()
		return;
	}

	const auto& plugins = PluginManager::ir();
	if (paramView.find(L' ') == std::string::npos)
	{
		PrintUserCmdText(client, L"The following command modules are available to you. Use /help <module> [command] for detailed information.");
		PrintUserCmdText(client, L"core");
		for (const auto& plugin : plugins)
		{
			if (!plugin->commands || plugin->commands->empty())
				continue;

			PrintUserCmdText(client, ToLower(stows(plugin->shortName)));
		}
		return;
	}

	const auto mod = GetParam(paramView, L' ', 1);
	const auto cmd = ToLower(Trim(GetParam(paramView, L' ', 2)));

	if (mod == L"core")
	{
		if (cmd.empty())
		{
			for (const auto& i : UserCmds)
			{
				if (i.command.index() == 0)
					PrintUserCmdText(client, std::get<std::wstring>(i.command));
				else
					PrintUserCmdText(client, i.usage);
			}
		}
		else if (const auto& userCommand = std::ranges::find_if(UserCmds, [&cmd](const UserCommand& userCmd) { return GetCommand(cmd, userCmd); });
		         userCommand != UserCmds.end())
		{
			PrintUserCmdText(client, userCommand->usage);
			PrintUserCmdText(client, userCommand->description);
		}
		else
		{
			PrintUserCmdText(client, std::format(L"Command '{}' not found within module 'Core'", cmd.c_str()));
		}
		return;
	}

	const auto& pluginIterator =
	    std::ranges::find_if(plugins, [&mod](const std::shared_ptr<PluginData> plug) { return ToLower(stows(plug->shortName)) == ToLower(mod); });

	if (pluginIterator == plugins.end())
	{
		PrintUserCmdText(client, L"Command module not found.");
		return;
	}

	const auto plugin = *pluginIterator;

	if (plugin->commands)
	{
		if (cmd.empty())
		{
			for (const auto& command : *plugin->commands)
			{
				if (command.command.index() == 0)
					PrintUserCmdText(client, std::get<std::wstring>(command.command));
				else
					PrintUserCmdText(client, command.usage);
			}
		}
		else if (const auto& userCommand = std::ranges::find_if(*plugin->commands, [&cmd](const UserCommand& userCmd) { return GetCommand(cmd, userCmd); });
		         userCommand != plugin->commands->end())
		{
			PrintUserCmdText(client, userCommand->usage);
			PrintUserCmdText(client, userCommand->description);
		}
		else
		{
			PrintUserCmdText(client, std::format(L"Command '{}' not found within module '{}'", cmd, stows(plugin->shortName)));
		}
	}
	else
	{
		PrintUserCmdText(client, std::format(L"Module '{}' does not have commands.", stows(plugin->shortName)));
	}
}

bool ProcessPluginCommand(ClientId& client, const std::wstring& originalCmdString, const std::vector<UserCommand>& commands)
{
	const std::wstring inputLower = ToLower(originalCmdString);
	for (const auto& cmdObj : commands)
	{
		std::optional<std::wstring> param;
		if (cmdObj.command.index() == 0)
		{
			const auto& subCmd = std::get<std::wstring>(cmdObj.command);

			// No command match
			if (inputLower.rfind(subCmd, 0) != 0)
			{
				continue;
			}

			// If the length of the input is greater than the command, check the last character and assert its not a space
			if (originalCmdString.length() > subCmd.length())
			{
				if (originalCmdString[subCmd.length()] != ' ')
				{
					continue;
				}
				param = originalCmdString.substr(subCmd.length() + 1);
			}
			else
			{
				param = L"";
			}
		}
		else
		{
			for (const auto& subCmd : std::get<std::vector<std::wstring>>(cmdObj.command))
			{
				if (inputLower.rfind(subCmd, 0) != 0 || (originalCmdString.length() > subCmd.length() && originalCmdString[subCmd.length()] != ' '))
				{
					continue;
				}

				param = originalCmdString;
			}
		}
		if (param.has_value())
		{
			std::wstring character = (wchar_t*)Players.GetActiveCharacterName(client);
			AddLog(LogType::UserLogCmds, LogLevel::Info, wstos(std::format(L"{}: {}", character.c_str(), originalCmdString.c_str())));

			try
			{
				if (cmdObj.proc.index() == 0)
				{
					std::get<UserCmdProc>(cmdObj.proc)(client);
				}
				else
				{
					std::get<UserCmdProcWithParam>(cmdObj.proc)(client, param.value());
				}

				AddLog(LogType::UserLogCmds, LogLevel::Info, "finished");
			}
			catch (std::exception const& ex)
			{
				AddLog(LogType::UserLogCmds, LogLevel::Err, std::format("exception {}", ex.what()));
			}

			return true;
		}
	}
	return false;
};

bool UserCmd_Process(ClientId client, const std::wstring& wscCmd)
{
	if (const auto& config = FLHookConfig::c(); config->messages.echoCommands)
	{
		std::wstring lower = ToLower(GetParam(wscCmd, ' ', 0));
		if ((lower.find(L'/') == 0 || lower.find(L'.') == 0) &&
		    !(lower == L"/l" || lower == L"/local" || lower == L"/s" || lower == L"/system" || lower == L"/g" || lower == L"/group" || lower == L"/t" ||
		        lower == L"/target" || lower == L"/r" || lower == L"/reply" || lower.find(L"//") == 0 || lower.find(L'*') == lower.length() - 1))
		{
			const std::wstring wscXML = L"<TRA data=\"" + config->messages.msgStyle.msgEchoStyle + L"\" mask=\"-1\"/><TEXT>" + XMLText(wscCmd) + L"</TEXT>";
			Hk::Message::FMsg(client, wscXML);
		}
	}

	auto [pluginRet, pluginSkip] = CallPluginsBefore<bool>(HookedCall::FLHook__UserCommand__Process, client, wscCmd);
	if (pluginSkip)
		return pluginRet;

	for (const auto& plugins = PluginManager::ir(); const std::shared_ptr<PluginData> i : plugins)
	{
		if (i->commands && ProcessPluginCommand(client, wscCmd, *i->commands))
		{
			return true;
		}
	}

	// In-built commands
	return ProcessPluginCommand(client, wscCmd, UserCmds);
}
