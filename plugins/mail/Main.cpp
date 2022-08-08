/**
 * @date Feb, 2010
 * @author Cannon (Ported by Raikkonen)
 * @defgroup Mail Mail
 * @brief
 * Plugin to handle the sending/receiving of mail.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - mail - Sends mail to a character.
 * - maildel - Deletes the specified mail.
 *
 * @paragraph adminCmds Admin Commands
 * All commands are prefixed with '.' unless explicitly specified.
 * - mail - Sends mail to a character.
 *
 * @paragraph configuration Configuration
 * No configuration file is needed.
 *
 * @paragraph ipc IPC Interfaces Exposed
 * - SendMail - Can use this to send mail to a character.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin has no dependencies.
 */

#include "Main.h"

namespace Plugins::Mail
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/** @ingroup Mail
	 * @brief Return the number of messages.
	 */
	int MailCount(const std::wstring& wscCharname, const std::string& scExtension)
	{
		// Get the target player's message file.
		std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
		if (scFilePath.length() == 0)
			return 0;

		int iMsgs = 0;
		for (int iMsgSlot = 1; iMsgSlot < global->MAX_MAIL_MSGS; iMsgSlot++)
		{
			if (IniGetS(scFilePath, "Msgs", std::to_string(iMsgSlot), "").length() == 0)
				break;
			iMsgs++;
		}
		return iMsgs;
	}

	/** @ingroup Mail
	 * @brief Show five messages from the specified starting position.
	 */
	void MailShow(const std::wstring& wscCharname, const std::string& scExtension, int iFirstMsg)
	{
		// Make sure the character is logged in.
		uint iClientID = HkGetClientIdFromCharname(wscCharname);
		if (iClientID == -1)
			return;

		// Get the target player's message file.
		std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
		if (scFilePath.length() == 0)
			return;

		int iLastMsg = iFirstMsg;
		for (int iMsgSlot = iFirstMsg, iMsgCount = 0; iMsgSlot < global->MAX_MAIL_MSGS && iMsgCount < 5; iMsgSlot++, iMsgCount++)
		{
			std::wstring wscTmpMsg = IniGetWS(scFilePath, "Msgs", std::to_string(iMsgSlot), L"");
			if (wscTmpMsg.length() == 0)
				break;
			PrintUserCmdText(iClientID, L"#%02d %s", iMsgSlot, wscTmpMsg.c_str());
			IniWrite(scFilePath, "MsgsRead", std::to_string(iMsgSlot), "yes");
			iLastMsg = iMsgSlot;
		}
		PrintUserCmdText(iClientID, L"Viewing #%02d-#%02d of %02d messages", iFirstMsg, iLastMsg, MailCount(wscCharname, scExtension));
	}

	/** @ingroup Mail
	 * @brief Return the number of unread messages.
	 */
	int MailCountUnread(const std::wstring& wscCharname, const std::string& scExtension)
	{
		// Get the target player's message file.
		std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
		if (scFilePath.length() == 0)
			return 0;

		int iUnreadMsgs = 0;
		for (int iMsgSlot = 1; iMsgSlot < global->MAX_MAIL_MSGS; iMsgSlot++)
		{
			if (IniGetS(scFilePath, "Msgs", std::to_string(iMsgSlot), "").length() == 0)
				break;
			if (!IniGetB(scFilePath, "MsgsRead", std::to_string(iMsgSlot), false))
				iUnreadMsgs++;
		}
		return iUnreadMsgs;
	}

	/** @ingroup Mail
	 * @brief Check for new or unread messages. 
	 */
	void MailCheckLog(const std::wstring& wscCharname, const std::string& scExtension)
	{
		// Make sure the character is logged in.
		uint iClientID = HkGetClientIdFromCharname(wscCharname);
		if (iClientID == -1)
			return;

		// Get the target player's message file.
		std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
		if (scFilePath.length() == 0)
			return;

		// If there are unread messaging then inform the player
		int iUnreadMsgs = MailCountUnread(wscCharname, scExtension);
		if (iUnreadMsgs > 0)
		{
			PrintUserCmdText(iClientID, L"You have %d unread messages. Type /mail to see your messages", iUnreadMsgs);
		}
	}

	/** @ingroup Mail
	 * @brief Save a msg to disk so that we can inform the receiving character when they log in.
	 */
	bool __stdcall MailSend(const std::wstring& wscCharname, const std::string& scExtension, const std::wstring_view& wscMsg)
	{
		// Get the target player's message file.
		std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
		if (scFilePath.length() == 0)
			return false;

		// Move all mail up one slot starting at the end. We automatically
		// discard the oldest messages.
		for (int iMsgSlot = global->MAX_MAIL_MSGS - 1; iMsgSlot > 0; iMsgSlot--)
		{
			std::wstring wscTmpMsg = IniGetWS(scFilePath, "Msgs", std::to_string(iMsgSlot), L"");
			IniWriteW(scFilePath, "Msgs", std::to_string(iMsgSlot + 1), wscTmpMsg);

			bool bTmpRead = IniGetB(scFilePath, "MsgsRead", std::to_string(iMsgSlot), false);
			IniWrite(scFilePath, "MsgsRead", std::to_string(iMsgSlot + 1), (bTmpRead ? "yes" : "no"));
		}

		// Write message into the slot
		IniWriteW(scFilePath, "Msgs", "1", GetTimeString(FLHookConfig::i()->general.localTime) + L" " + ViewToWString(wscMsg));
		IniWrite(scFilePath, "MsgsRead", "1", "no");

		MailCommunicator::MailSent mail {wscCharname, wscMsg};
		global->communicator->Dispatch(static_cast<int>(MailCommunicator::MailEvent::MailSent), &mail);

		// Call the CheckLog function to prompt the target to check their mail if they are logged in
		MailCheckLog(wscCharname, global->MSG_LOG);

		return true;
	}

	/** @ingroup Mail
	 * @brief Delete a message.
	 */
	bool MailDel(const std::wstring& wscCharname, const std::string& scExtension, int iMsg)
	{
		// Get the target player's message file.
		std::string scFilePath = GetUserFilePath(wscCharname, scExtension);
		if (scFilePath.length() == 0)
			return false;

		// Move all mail down one slot starting at the deleted message to overwrite
		// it
		for (int iMsgSlot = iMsg; iMsgSlot < global->MAX_MAIL_MSGS; iMsgSlot++)
		{
			std::wstring wscTmpMsg = IniGetWS(scFilePath, "Msgs", std::to_string(iMsgSlot + 1), L"");
			IniWriteW(scFilePath, "Msgs", std::to_string(iMsgSlot), wscTmpMsg);

			bool bTmpRead = IniGetB(scFilePath, "MsgsRead", std::to_string(iMsgSlot + 1), false);
			IniWrite(scFilePath, "MsgsRead", std::to_string(iMsgSlot), (bTmpRead ? "yes" : "no"));
		}
		return true;
	}

	/** @ingroup Mail
	 * @brief Hook on PlayerLaunch. Calls MailCheckLog.
	 */
	void PlayerLaunch(uint& iShip, uint& iClientID) { MailCheckLog((const wchar_t*)Players.GetActiveCharacterName(iClientID), global->MSG_LOG); }

	/** @ingroup Mail
	 * @brief Hook on BaseEnter. Calls MailCheckLog.
	 */
	void BaseEnter(uint& iBaseID, uint& iClientID) { MailCheckLog((const wchar_t*)Players.GetActiveCharacterName(iClientID), global->MSG_LOG); }

	/** @ingroup Mail
	 * @brief MailCommunicator for inter-plugin communication.
	 */
	MailCommunicator::MailCommunicator(const std::string& plugin) : PluginCommunicator(plugin)
	{ // NOLINT(clang-diagnostic-shadow-field)
		this->SendMail = [](std::wstring character, std::wstring_view msg) {
			MailSend(character, global->MSG_LOG, msg);
		};
	}

	/** @ingroup Mail
	 * @brief Called when the player types "/mail". 
	 */
	void UserCmd_MailShow(const uint& iClientID, const std::wstring_view& wscParam)
	{
		int iNumberUnreadMsgs = MailCountUnread((const wchar_t*)Players.GetActiveCharacterName(iClientID), global->MSG_LOG);
		int iNumberMsgs = MailCount((const wchar_t*)Players.GetActiveCharacterName(iClientID), global->MSG_LOG);
		if (iNumberMsgs == 0)
		{
			PrintUserCmdText(iClientID, L"OK You have no messages");
			return;
		}

		int iFirstMsg = ToInt(ToLower(GetParam(wscParam, ' ', 0)));
		if (iFirstMsg == 0)
		{
			if (iNumberUnreadMsgs > 0)
				PrintUserCmdText(iClientID, L"OK You have %d unread messages", iNumberUnreadMsgs);
			else
				PrintUserCmdText(iClientID, L"OK You have %d messages", iNumberMsgs);
			PrintUserCmdText(iClientID,
			    L"Type /mail 1 to see first message or /mail "
			    L"<num> to see specified message");
			return;
		}

		if (iFirstMsg > iNumberMsgs)
		{
			PrintUserCmdText(iClientID, L"ERR Message does not exist");
			return;
		}

		MailShow((const wchar_t*)Players.GetActiveCharacterName(iClientID), global->MSG_LOG, iFirstMsg);
	}

	/** @ingroup Mail
	 * @brief Called when the player types "/maildel".
	 */
	void UserCmd_MailDel(const uint& iClientID, const std::wstring_view& wscParam)
	{
		if (wscParam.size() == 0)
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /maildel <msgnum>");
			return;
		}

		int iNumberMsgs = MailCount((const wchar_t*)Players.GetActiveCharacterName(iClientID), global->MSG_LOG);
		int iMsg = ToInt(ToLower(GetParam(wscParam, ' ', 0)));
		if (iMsg == 0 || iMsg > iNumberMsgs)
		{
			PrintUserCmdText(iClientID, L"ERR Message does not exist");
			return;
		}

		if (MailDel((const wchar_t*)Players.GetActiveCharacterName(iClientID), global->MSG_LOG, iMsg))
			PrintUserCmdText(iClientID, L"OK");
		else
			PrintUserCmdText(iClientID, L"ERR");
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/mail", L"{num}", UserCmd_MailShow, L"Shows any mail you have."),
	    CreateUserCommand(L"/maildel", L"<msgnum>", UserCmd_MailDel, L"Deletes mail from your inbox."),
	}};

	/** @ingroup Mail
	 * @brief Called when the admin types ".mail".
	 */
	void AdminCmd_SendMail(CCmds* cmds, const std::wstring& wscCharname, const std::wstring& wscMsg)
	{
		MailSend(wscCharname, global->MSG_LOG, cmds->GetAdminName() + L": " + wscMsg);
		cmds->Print(L"OK message saved to mailbox");
	}

	/** @ingroup Mail
	 * @brief Admin command processsing.
	 */
	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		if (wscCmd == L"mail")
		{
			global->returncode = ReturnCode::SkipAll;
			AdminCmd_SendMail(cmds, cmds->ArgStr(0), cmds->ArgStrToEnd(1));
			return true;
		}
		return false;
	}
} // namespace Plugins::Mail

using namespace Plugins::Mail;

DefaultDllMain()

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name(MailCommunicator::pluginName);
	pi->shortName("mail");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);

	// Register plugin for IPC
	global->communicator = new MailCommunicator(MailCommunicator::pluginName);
	PluginCommunicator::ExportPluginCommunicator(global->communicator);
}