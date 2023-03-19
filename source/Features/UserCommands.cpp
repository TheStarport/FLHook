#include "PCH.hpp"

#include <Features/Logger.hpp>

#include "Global.hpp"
#include "plugin.h"
#include "Defs/FLHookConfig.hpp"
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
	if (const auto newLineChar = text.find(L"\n"); newLineChar == std::wstring::npos)
	{
		const std::wstring xml = std::format(L"<TRA data=\"{}\" mask=\"-1\"/><TEXT>{}</TEXT>", FLHookConfig::i()->messages.msgStyle.userCmdStyle, XMLText(text));
		Hk::Message::FMsg(client, xml);
	}
	else
	{
		// Split text into two strings, one from the beginning to the character before newLineChar, and one after newLineChar till the end.
		// It will then recursively call itself for each new line char until the original text is all displayed.
		PrintUserCmdText(client, text.substr(0, newLineChar));
		PrintUserCmdText(client, text.substr(newLineChar + 1, std::wstring::npos));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Print message to all ships within the specific number of meters of the
/// player.
void PrintLocalUserCmdText(ClientId client, const std::wstring& msg, float distance)
{
	uint ship;
	pub::Player::GetShip(client, ship);

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(ship, pos, rot);

	uint system;
	pub::Player::GetSystem(client, system);

	// For all players in system...
	PlayerData* playerDb = nullptr;
	while ((playerDb = Players.traverse_active(playerDb)))
	{
		// Get the this player's current system and location in the system.
		ClientId client2 = playerDb->onlineId;
		uint system2 = 0;
		pub::Player::GetSystem(client2, system2);
		if (system != system2)
			continue;

		uint ship2;
		pub::Player::GetShip(client2, ship2);

		Vector pos2;
		Matrix rot2;
		pub::SpaceObj::GetLocation(ship2, pos2, rot2);

		// Is player within the specified range of the sending char.
		if (Hk::Math::Distance3D(pos, pos2) > distance)
			continue;

		PrintUserCmdText(client2, msg);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetDieMsg(ClientId& client, const std::wstring& param)
{
	if (!FLHookConfig::i()->messages.dieMsg)
	{
		PRINT_DISABLED()
		return;
	}

	static const std::wstring errorMsg = L"Error: Invalid parameters\n"
		L"Usage: /set diemsg <param>\n"
		L"<param>: all,system,self or none";

	const std::wstring dieMsgParam = ToLower(GetParam(param, ' ', 0));

	DIEMSGTYPE dieMsg;
	if (dieMsgParam == L"all")
		dieMsg = DiemsgAll;
	else if (dieMsgParam == L"system")
		dieMsg = DIEMSG_SYSTEM;
	else if (dieMsgParam == L"none")
		dieMsg = DIEMSG_NONE;
	else if (dieMsgParam == L"self")
		dieMsg = DIEMSG_SELF;
	else
	{
		PrintUserCmdText(client, errorMsg);
		return;
	}

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

	static const std::wstring errorMsg = L"Error: Invalid parameters\n"
		L"Usage: /set diemsgsize <size>\n"
		L"<size>: small, default";

	const std::wstring dieMsgSizeParam = ToLower(GetParam(param, ' ', 0));

	CHATSIZE dieMsgSize;
	if (!dieMsgSizeParam.compare(L"small"))
		dieMsgSize = CS_SMALL;
	else if (!dieMsgSizeParam.compare(L"default"))
		dieMsgSize = CS_DEFAULT;
	else
	{
		PrintUserCmdText(client, errorMsg);
		return;
	}

	// save to ini
	GET_USERFILE(scUserFile)
	IniWrite(scUserFile, "Settings", "DieMsgSize", std::to_string(dieMsgSize));

	// save in ClientInfo
	ClientInfo[client].dieMsgSize = dieMsgSize;

	// send confirmation msg
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetChatFont(ClientId& client, const std::wstring& param)
{
	if (!FLHookConfig::i()->userCommands.userCmdSetChatFont)
	{
		PRINT_DISABLED()
		return;
	}

	static const std::wstring errorMsg = L"Error: Invalid parameters\n"
		L"Usage: /set chatfont <size> <style>\n"
		L"<size>: small, default or big\n"
		L"<style>: default, bold, italic or underline";

	const std::wstring chatSizeParam = ToLower(GetParam(param, ' ', 0));
	const std::wstring chatStyleParam = ToLower(GetParam(param, ' ', 1));

	CHATSIZE chatSize;
	if (!chatSizeParam.compare(L"small"))
		chatSize = CS_SMALL;
	else if (!chatSizeParam.compare(L"default"))
		chatSize = CS_DEFAULT;
	else if (!chatSizeParam.compare(L"big"))
		chatSize = CS_BIG;
	else
	{
		PrintUserCmdText(client, errorMsg);
		return;
	}

	CHATSTYLE chatStyle;
	if (!chatStyleParam.compare(L"default"))
		chatStyle = CST_DEFAULT;
	else if (!chatStyleParam.compare(L"bold"))
		chatStyle = CST_BOLD;
	else if (!chatStyleParam.compare(L"italic"))
		chatStyle = CST_ITALIC;
	else if (!chatStyleParam.compare(L"underline"))
		chatStyle = CST_UNDERLINE;
	else
	{
		PrintUserCmdText(client, errorMsg);
		return;
	}

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

	static const std::wstring errorMsg = L"Error: Invalid parameters\n"
		L"Usage: /ignore <charname> [<flags>]\n"
		L"<charname>: character name which should be ignored(case insensitive)\n"
		L"<flags>: combination of the following flags:\n"
		L" p - only affect private chat\n"
		L" i - <charname> may match partially\n"
		L"Examples:\n"
		L"\"/ignore SomeDude\" ignores all chatmessages from SomeDude\n"
		L"\"/ignore PlayerX p\" ignores all private-chatmessages from PlayerX\n"
		L"\"/ignore idiot i\" ignores all chatmessages from players whose \n"
		L"charname contain \"idiot\" (e.g. \"[XYZ]IdIOT\", \"MrIdiot\", etc)\n"
		L"\"/ignore Fool pi\" ignores all private-chatmessages from players \n"
		L"whose charname contain \"fool\"";

	const std::wstring allowedFlags = L"pi";

	const std::wstring character = GetParam(param, ' ', 0);
	const std::wstring flags = ToLower(GetParam(param, ' ', 1));

	if (character.empty())
	{
		PrintUserCmdText(client, errorMsg);
		return;
	}

	// check if flags are valid
	for (const auto flag : flags)
	{
		if (allowedFlags.find_first_of(flag) == -1)
		{
			PrintUserCmdText(client, errorMsg);
			return;
		}
	}

	if (ClientInfo[client].ignoreInfoList.size() > FLHookConfig::i()->userCommands.userCmdMaxIgnoreList)
	{
		PrintUserCmdText(client, L"Error: Too many entries in the ignore list, please delete an entry first!");
		return;
	}

	// save to ini
	GET_USERFILE(scUserFile)
	IniWriteW(scUserFile, "IgnoreList", std::to_string(static_cast<int>(ClientInfo[client].ignoreInfoList.size()) + 1), character + L" " + flags);

	// save in ClientInfo
	IgnoreInfo ii;
	ii.character = character;
	ii.flags = flags;
	ClientInfo[client].ignoreInfoList.push_back(ii);

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

	static const std::wstring errorMsg = L"Error: Invalid parameters\n"
		L"Usage: /ignoreid <client-id> [<flags>]\n"
		L"<client-id>: client-id of character which should be ignored\n"
		L"<flags>: if \"p\"(without quotation marks) then only affect private\n"
		L"chat";

	const std::wstring clientId = GetParam(param, ' ', 0);
	const std::wstring flags = ToLower(GetParam(param, ' ', 1));

	if (!clientId.length() || !flags.empty() && flags.compare(L"p") != 0)
	{
		PrintUserCmdText(client, errorMsg);
		return;
	}

	if (ClientInfo[client].ignoreInfoList.size() > FLHookConfig::i()->userCommands.userCmdMaxIgnoreList)
	{
		PrintUserCmdText(client, L"Error: Too many entries in the ignore list, please delete an entry first!");
		return;
	}

	ClientId clientTarget = ToInt(clientId);
	if (!Hk::Client::IsValidClientID(clientTarget) || Hk::Client::IsInCharSelectMenu(clientTarget))
	{
		PrintUserCmdText(client, L"Error: Invalid client-id");
		return;
	}

	std::wstring character = Hk::Client::GetCharacterNameByID(clientTarget).value();

	// save to ini
	GET_USERFILE(scUserFile)
	IniWriteW(scUserFile, "IgnoreList", std::to_string(static_cast<int>(ClientInfo[client].ignoreInfoList.size()) + 1), character + L" " + flags);

	// save in ClientInfo
	IgnoreInfo ii;
	ii.character = character;
	ii.flags = flags;
	ClientInfo[client].ignoreInfoList.push_back(ii);

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

	PrintUserCmdText(client, L"Id | Charactername | flags");
	int i = 1;
	for (auto& ignore : ClientInfo[client].ignoreInfoList)
	{
		PrintUserCmdText(client, std::format(L"{} | %s | %s", i, ignore.character.c_str(), ignore.flags));
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

	static const std::wstring errorMsg = L"Error: Invalid parameters\n"
		L"Usage: /delignore <id> [<id2> <id3> ...]\n"
		L"<id>: id of ignore-entry(see /ignorelist) or *(delete all)";

	std::wstring idToDelete = GetParam(param, ' ', 0);

	if (idToDelete.empty())
	{
		PrintUserCmdText(client, errorMsg);
		return;
	}

	GET_USERFILE(scUserFile)

	if (!idToDelete.compare(L"*"))
	{
		// delete all
		IniDelSection(scUserFile, "IgnoreList");
		ClientInfo[client].ignoreInfoList.clear();
		PRINT_OK()
		return;
	}

	std::list<uint> Delete;
	for (uint j = 1; !idToDelete.empty(); j++)
	{
		uint id = ToInt(idToDelete.c_str());
		if (!id || id > ClientInfo[client].ignoreInfoList.size())
		{
			PrintUserCmdText(client, L"Error: Invalid Id");
			return;
		}

		Delete.push_back(id);
		idToDelete = GetParam(param, ' ', j);
	}

	Delete.sort(std::greater<uint>());

	ClientInfo[client].ignoreInfoList.reverse();
	for (const auto& del : Delete)
	{
		uint currId = ClientInfo[client].ignoreInfoList.size();
		for (auto ignoreIt = ClientInfo[client].ignoreInfoList.begin(); ignoreIt != ClientInfo[client].ignoreInfoList.end(); ++ignoreIt)
		{
			if (currId == del)
			{
				ClientInfo[client].ignoreInfoList.erase(ignoreIt);
				break;
			}
			currId--;
		}
	}
	ClientInfo[client].ignoreInfoList.reverse();

	// send confirmation msg
	IniDelSection(scUserFile, "IgnoreList");
	int i = 1;
	for (const auto& ignore : ClientInfo[client].ignoreInfoList)
	{
		IniWriteW(scUserFile, "IgnoreList", std::to_string(i), ignore.character + L" " + ignore.flags);
		i++;
	}
	PRINT_OK()
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Ids(ClientId& client, [[maybe_unused]] const std::wstring& param)
{
	for (auto& player : Hk::Admin::GetPlayers())
	{
		PrintUserCmdText(client, std::format(L"{} = {} | ", player.character, player.client));
	}
	PrintUserCmdText(client, L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_ID(ClientId& client, [[maybe_unused]] const std::wstring& param)
{
	PrintUserCmdText(client, std::format(L"Your client-id: {}", client));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Invite_Player(ClientId& client, const std::wstring& characterName)
{
	const std::wstring XML = L"<TEXT>/i " + XMLText(characterName) + L"</TEXT>";
	char buf[0xFFFF];
	uint retVal;
	if (Hk::Message::FMsgEncodeXML(XML, buf, sizeof buf, retVal).has_error())
	{
		PrintUserCmdText(client, L"Error: Could not encode XML");
		return;
	}

	CHAT_ID chatId;
	chatId.id = client;
	CHAT_ID chatIdTo;
	chatIdTo.id = 0x00010001;
	Server.SubmitChat(chatId, retVal, buf, chatIdTo, -1);
}

void UserCmd_Invite(ClientId& client, const std::wstring& param)
{
	if (!param.empty())
	{
		const auto characterName = GetParam(param, ' ', 0);
		auto inviteeId = Hk::Client::GetClientIdFromCharName(characterName);
		if (inviteeId.has_value() && !Hk::Client::IsInCharSelectMenu(inviteeId.value()))
		{
			Invite_Player(client, characterName);
		}
	}
	else
	{
		auto targetClientId = Hk::Player::GetTargetClientID(client);
		if (targetClientId.has_value())
		{
			Invite_Player(client, Hk::Client::GetCharacterNameByID(targetClientId.value()).value());
		}
	}
}

void UserCmd_InviteID(ClientId& client, const std::wstring& param)
{
	const std::wstring invitedClientId = GetParam(param, ' ', 0);

	if (invitedClientId.empty())
	{
		PrintUserCmdText(client, L"Error: Invalid parameters\nUsage: /i$ <client-id>");
		return;
	}

	ClientId clientTarget = ToInt(invitedClientId);
	if (!Hk::Client::IsValidClientID(clientTarget) || Hk::Client::IsInCharSelectMenu(clientTarget))
	{
		PrintUserCmdText(client, L"Error: Invalid client-id");
		return;
	}

	Invite_Player(client, Hk::Client::GetCharacterNameByID(clientTarget).value());
}

void UserCmd_FactionInvite(ClientId& client, const std::wstring& param)
{
	const std::wstring& charnamePrefix = ToLower(GetParam(param, ' ', 0));

	bool msgSent = false;

	if (charnamePrefix.size() < 3)
	{
		PrintUserCmdText(client, L"ERR Invalid parameters");
		PrintUserCmdText(client, L"Usage: /factioninvite <tag> or /fi ...");
		return;
	}

	for (const auto& player : Hk::Admin::GetPlayers())
	{
		if (ToLower(player.character).find(charnamePrefix) == std::string::npos)
			continue;

		if (player.client == client)
			continue;

		Invite_Player(client, player.character);
		msgSent = true;
	}

	if (msgSent == false)
		PrintUserCmdText(client, L"ERR No chars found");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_FLHookInfo(ClientId& client, [[maybe_unused]] const std::wstring& param)
{
	PrintUserCmdText(client, L"This server is running FLHook v" + VersionInformation);
	PrintUserCmdText(client, L"Running plugins:");

	bool running = false;
	for (const auto& plugin : PluginManager::ir())
	{
		if (plugin->paused)
			continue;

		running = true;
		PrintUserCmdText(client, std::format(L"- {}", stows(plugin->name)));
	}
	if (!running)
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
				"|    {}.) {} {}- {} - {:%F %T}",
				item.id,
				item.subject,
				item.unread ? "(unread) " : "",
				item.author,
				UnixToSysTime(item.timestamp))));
	}
}

void UserCmdGiveCash(ClientId& client, const std::wstring& param)
{
	const auto targetPlayer = Hk::Client::GetClientIdFromCharName(GetParam(param, ' ', 0));
	const auto cash = MultiplyUIntBySuffix(GetParam(param, ' ', 1));
	const auto clientCash = Hk::Player::GetCash(client);

	if (targetPlayer.has_error())
	{
		PrintUserCmdText(client, Hk::Err::ErrGetText(targetPlayer.error()));
		return;
	}

	if (client == targetPlayer.value())
	{
		PrintUserCmdText(client, L"Not sure this really accomplishes much, (Don't give cash to yourself.)");
		return;
	}

	if (clientCash.has_error())
	{
		PrintUserCmdText(client, Hk::Err::ErrGetText(clientCash.error()));
		return;
	}

	if (cash == 0)
	{
		PrintUserCmdText(client, std::format(L"Err: Invalid cash amount."));
		return;
	}

	if (clientCash.value() < cash)
	{
		PrintUserCmdText(client, std::format(L"Err: You do not have enough cash, you only have {}, and are trying to give {}.", clientCash.value(), cash));
		return;
	}

	const auto removal = Hk::Player::RemoveCash(client, cash);

	if (removal.has_error())
	{
		PrintUserCmdText(client, Hk::Err::ErrGetText(removal.error()));
		return;
	}

	const auto addition = Hk::Player::AddCash(targetPlayer.value(), cash);

	if (addition.has_error())
	{
		PrintUserCmdText(client, Hk::Err::ErrGetText(addition.error()));
		return;
	}

	Hk::Player::SaveChar(client);
	Hk::Player::SaveChar(targetPlayer.value());
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
const std::vector UserCmds = {{CreateUserCommand(L"/set diemsg", L"<all/system/self/none>", UserCmd_SetDieMsg, L""),
                               CreateUserCommand(L"/set diemsgsize", L"<small/default>", UserCmd_SetDieMsgSize, L"Sets the text size of death messages."),
                               CreateUserCommand(L"/set chatfont",
	                               L"<small/default/big> <default/bold/italic/underline>",
	                               UserCmd_SetChatFont,
	                               L"Sets the font of chat."),
                               CreateUserCommand(L"/ignorelist", L"", UserCmd_IgnoreList, L"Lists currently ignored players."),
                               CreateUserCommand(L"/delignore",
	                               L"<id/*> [<id2> <id3...]",
	                               UserCmd_DelIgnore,
	                               L"Removes specified entries form ignore list, '*' clears the whole list."),
                               CreateUserCommand(L"/ignore",
	                               L"<name> [flags]",
	                               UserCmd_Ignore,
	                               L"Suppresses chat messages from the specified player. Flag 'p' only suppresses private chat, 'i' allows for partial match."),
                               CreateUserCommand(
	                               L"/ignoreid",
	                               L"<client-id> [flags]",
	                               UserCmd_IgnoreID,
	                               L"Suppresses chat messages from the specified player. Flag 'p' only suppresses private chat."),
                               CreateUserCommand(L"/ids", L"", UserCmd_Ids, L"List all player IDs."),
                               CreateUserCommand(L"/id", L"", UserCmd_ID, L"Lists your player ID."),
                               CreateUserCommand(L"/invite",
	                               L"[name]",
	                               UserCmd_Invite,
	                               L"Sends a group invite to a player with the specified name, or target, if no name is provided."),
                               CreateUserCommand(L"/invite$", L"<id>", UserCmd_InviteID, L"Sends a group invite to a player with the specified ID."),
                               CreateUserCommand(L"/i", L"[name]", UserCmd_Invite, L"Shortcut for /invite"),
                               CreateUserCommand(L"/i$", L"<id>", UserCmd_InviteID, L"Shortcut for /invite$"),
                               CreateUserCommand(L"/factioninvite",
	                               L"<name>",
	                               UserCmd_FactionInvite,
	                               L"Send a group invite to online members of the specified tag."),
                               CreateUserCommand(L"/fi", L"<name>", UserCmd_FactionInvite, L"Shortcut for /factioninvite."),
                               CreateUserCommand(L"/flhookinfo", L"", UserCmd_FLHookInfo, L""),
                               CreateUserCommand(L"/maildel",
	                               L"/maildel <id/all> [readonly]",
	                               UserCmdDelMail,
	                               L"Delete the specified mail, or if all is provided delete all mail. If all is specified with the param of readonly, unread mail will be preserved."),
                               CreateUserCommand(L"/mailread", L"/mailread <id>", UserCmdReadMail, L"Read the specified mail."),
                               CreateUserCommand(L"/maillist",
	                               L"/maillist <page> [unread]",
	                               UserCmdListMail,
	                               L"List the mail items on the specified page. If unread is specified, only mail that hasn't been read will be listed."),
                               CreateUserCommand(CmdArr({L"/help", L"/h", L"/?"}), helpUsage, UserCmd_Help, helpDescription),
                               CreateUserCommand(L"/givecash",
	                               L"<player> <amount>",
	                               UserCmdGiveCash,
	                               L"Gives specified amount of cash to a target player, target must be online.")}};

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
	const auto& arr = std::get<std::vector<std::wstring>>(userCmd.command);
	return std::ranges::any_of(arr, isMatch);
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

bool ProcessPluginCommand(ClientId& client, const std::wstring& originalCmdString, 
	std::vector<UserCommand>::const_iterator commandStart, const std::vector<UserCommand>::const_iterator commandEnd)
{
	const std::wstring inputLower = ToLower(originalCmdString);
	while (commandStart++ != commandEnd)
	{
		std::optional<std::wstring> param;
		if (commandStart->command.index() == 0)
		{
			const auto& subCmd = std::get<std::wstring>(commandStart->command);

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
			for (const auto& subCmd : std::get<std::vector<std::wstring>>(commandStart->command))
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
			const std::wstring character = (wchar_t*)Players.GetActiveCharacterName(client);
			Logger::i()->Log(LogLevel::Info, wstos(std::format(L"{}: {}", character.c_str(), originalCmdString.c_str())));

			try
			{
				if (commandStart->proc.index() == 0)
				{
					std::get<UserCmdProc>(commandStart->proc)(client);
				}
				else
				{
					std::get<UserCmdProcWithParam>(commandStart->proc)(client, param.value());
				}

				Logger::i()->Log(LogLevel::Info, "finished");
			}
			catch (const std::exception& ex)
			{
				Logger::i()->Log(LogLevel::Err, std::format("exception {}", ex.what()));
			}

			return true;
		}
	}
	return false;
};

bool UserCmdProcess(ClientId client, const std::wstring& cmd)
{
	if (auto [pluginRet, pluginSkip] = CallPluginsBefore<bool>(HookedCall::FLHook__UserCommand__Process, client, cmd); pluginSkip)
		return pluginRet;

	for (const auto& plugins = PluginManager::ir(); const std::weak_ptr<Plugin> i : plugins)
	{
		if (const auto [begin, end] = i.lock()->GetCommands(); begin != end && ProcessPluginCommand(client, cmd, begin, end))
		{
			return true;
		}
	}

	// In-built commands
	return ProcessPluginCommand(client, cmd, UserCmds.begin(), UserCmds.end());
}

