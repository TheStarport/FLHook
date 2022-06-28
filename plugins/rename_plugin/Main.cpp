// Rename plugin - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

namespace Plugins::Rename
{
	const auto global = std::make_unique<Global>();
	void LoadSettings()
	{
		global->config = Serializer::JsonToObject<Config>();
		global->tagList = Serializer::JsonToObject<TagList>();
	}

	void SaveSettings()
	{
		Serializer::SaveToJson(global->tagList);
	}

	bool CreateNewCharacter(SCreateCharacterInfo const& si, const uint& iClientID)
	{
		if (global->config.charNameTags)
		{
			// If this ship name starts with a restricted tag then the ship may only be created using rename and the faction password
			const std::wstring charName(si.wszCharname);
			if (const auto& tag = global->tagList.FindTagPartial(charName); tag != global->tagList.tags.end() && !tag->renamePassword.empty())
			{
				Server.CharacterInfoReq(iClientID, true);
				return true;
			}

			// If this ship name is too short, reject the request
			if (charName.size() < MinCharacterNameLength + 1)
			{
				Server.CharacterInfoReq(iClientID, true);
				return true;
			}
		}

		if (global->config.asciiCharNameOnly)
		{
			const std::wstring wscCharName(si.wszCharname);
			for (const wchar_t ch : wscCharName)
			{
				if (ch & 0xFF80)
					return true;
			}
		}

		return false;
	}

	// Update the tag list when a character is selected update the tag list to
	// indicate that this tag is in use. If a tag is not used after 60 days, remove
	// it.
	void CharacterSelect_AFTER([[maybe_unused]] std::string& szCharFilename, const uint& iClientID)
	{
		if (!global->config.charNameTags)
			return;

		const auto charName = HkGetCharacterNameById(iClientID);
		if (const auto& tag = global->tagList.FindTagPartial(charName); tag != global->tagList.tags.end() && !tag->renamePassword.empty())
		{
			tag->lastAccess = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}
	}

	void UserCmd_MakeTag(const uint& iClientID, const std::wstring_view& wscParam)
	{
		if (!global->config.charNameTags)
			return;

		const std::wstring usage = L"Usage: /maketag <tag> <master password> <description>";
		// Indicate an error if the command does not appear to be formatted
		// correctly and stop processing but tell FLHook that we processed the command.
		if (wscParam.empty())
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, usage);
			return;
		}

		uint iBaseID;
		pub::Player::GetBase(iClientID, iBaseID);
		if (!iBaseID)
		{
			PrintUserCmdText(iClientID, L"ERR Not in base");
			return;
		}

		std::wstring tag = GetParam(wscParam, ' ', 0);
		std::wstring pass = GetParam(wscParam, ' ', 1);
		std::wstring_view description = GetParamToEnd(wscParam, ' ', 2);

		if (tag.size() < MinCharacterNameLength)
		{
			PrintUserCmdText(iClientID, L"ERR Tag too short");
			PrintUserCmdText(iClientID, usage);
			return;
		}

		if (pass.empty())
		{
			PrintUserCmdText(iClientID, L"ERR Password not set");
			PrintUserCmdText(iClientID, usage);
			return;
		}

		if (description.empty())
		{
			PrintUserCmdText(iClientID, L"ERR Description not set");
			PrintUserCmdText(iClientID, usage);
			return;
		}

		// If this tag is in use then reject the request.
		for (const auto& i : global->tagList.tags)
		{
			if (tag.find(i.tag) == 0 || i.tag.find(tag) == 0)
			{
				PrintUserCmdText(iClientID, L"ERR Tag already exists or conflicts with existing tag");
				return;
			}
		}

		// Save character and exit if kicked on save.
		std::wstring charName = HkGetCharacterNameById(iClientID);
		HkSaveChar(charName);
		if (HkGetClientIdFromCharname(charName) == -1)
			return;

		int iCash;
		HkFunc(HkGetCash, iClientID, iCash);
		if (global->config.makeTagCost > 0 && iCash < global->config.makeTagCost)
		{
			PrintUserCmdText(iClientID, L"ERR Insufficient credits");
			return;
		}

		HkAddCash(charName, 0 - global->config.makeTagCost);

		TagData data;

		data.masterPassword = pass;
		data.renamePassword = L"";
		data.lastAccess = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		data.description = description;
		data.tag = tag;
		global->tagList.tags.emplace_back(data);

		PrintUserCmdText(iClientID, L"Created faction tag %s with master password %s", tag.c_str(), pass.c_str());
		AddLog(LogType::Normal, LogLevel::Info, L"Tag %s created by %s (%s)", tag.c_str(), charName.c_str(), HkGetAccountIDByClientID(iClientID).c_str());
		SaveSettings();
	}

	void UserCmd_DropTag(const uint& iClientID, const std::wstring_view& wscParam)
	{
		if (!global->config.charNameTags)
			return;

		// Indicate an error if the command does not appear to be formatted
		// correctly and stop processing but tell FLHook that we processed the
		// command.
		if (wscParam.empty())
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /droptag <tag> <master password>");
			return;
		}

		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
		std::wstring tag = GetParam(wscParam, ' ', 0);
		std::wstring pass = GetParam(wscParam, ' ', 1);

		// If this tag is in use then reject the request.
		if (const auto& data = global->tagList.FindTag(tag); data != global->tagList.tags.end() && pass == data->masterPassword)
		{
			global->tagList.tags.erase(
			    std::remove_if(global->tagList.tags.begin(), global->tagList.tags.end(), [tag](const TagData& tg) { return tg.tag == tag; }),
			    global->tagList.tags.end());
			SaveSettings();
			PrintUserCmdText(iClientID, L"OK Tag dropped");
			AddLog(LogType::Normal, LogLevel::Info, L"Tag %s dropped by %s (%s)", tag.c_str(), wscCharname.c_str(), HkGetAccountIDByClientID(iClientID).c_str());
			return;
		}

		PrintUserCmdText(iClientID, L"ERR tag or master password are invalid");
	}

	// Make tag password
	void UserCmd_SetTagPass(const uint& iClientID, const std::wstring_view& wscParam)
	{
		if (global->config.charNameTags)
		{
			// Indicate an error if the command does not appear to be formatted
			// correctly and stop processing but tell FLHook that we processed the command.
			if (wscParam.empty())
			{
				PrintUserCmdText(iClientID, L"ERR Invalid parameters");
				PrintUserCmdText(iClientID, L"Usage: /settagpass <tag> <master password> <rename password>");
				return;
			}

			const std::wstring tag = GetParam(wscParam, ' ', 0);
			const std::wstring masterPassword = GetParam(wscParam, ' ', 1);
			const std::wstring renamePassword = GetParam(wscParam, ' ', 2);

			// If this tag is in use then reject the request.
			if (const auto& data = global->tagList.FindTag(tag); data != global->tagList.tags.end())
			{
				if (masterPassword == data->masterPassword)
				{
					data->renamePassword = renamePassword;
					SaveSettings();
					PrintUserCmdText(iClientID, L"OK Created rename password %s for tag %s", renamePassword.c_str(), tag.c_str());
					return;
				}
			}
			PrintUserCmdText(iClientID, L"ERR tag or master password are invalid");
		}
	}

	void RenameTimer()
	{
		// Check for pending renames and execute them. We do this on a timer so that
		// the player is definitely not online when we do the rename.
		while (!global->pendingRenames.empty())
		{
			Rename o = global->pendingRenames.front();
			if (HkGetClientIdFromCharname(o.charName) != -1)
				return;

			global->pendingRenames.erase(global->pendingRenames.begin());

			CAccount* acc = HkGetAccountByCharname(o.charName);

			// Delete the character from the existing account, create a new
			// character with the same name in this account and then copy over it
			// with the save character file.
			try
			{
				if (!acc)
					throw "no acc";

				HkLockAccountAccess(acc, true);
				HkUnlockAccountAccess(acc);

				// Move the char file to a temporary one.
				if (!::MoveFileExA(o.sourceFile.c_str(), o.destFileTemp.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
					throw "move src to temp failed";

				// Decode the char file, update the char name and re-encode it.
				// Add a space to the value so the ini file line looks like "<key> =
				// <value>" otherwise Ioncross Server Operator can't decode the file
				// correctly
				flc_decode(o.destFileTemp.c_str(), o.destFileTemp.c_str());
				IniWriteW(o.destFileTemp, "Player", "Name", o.newCharName);
				if (!FLHookConfig::i()->general.disableCharfileEncryption)
				{
					flc_encode(o.destFileTemp.c_str(), o.destFileTemp.c_str());
				}

				// Create and delete the character
				HkDeleteCharacter(acc, o.charName);
				HkNewCharacter(acc, o.newCharName);

				// Move files around
				if (!::MoveFileExA(o.destFileTemp.c_str(), o.destFile.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
					throw "move failed";
				if (::PathFileExistsA(o.sourceFile.c_str()))
					throw "src still exists";
				if (!::PathFileExistsA(o.destFile.c_str()))
					throw "dest does not exist";

				// The rename worked. Log it and save the rename time.
				AddLog(LogType::Normal,
				    LogLevel::Info,
				    L"User rename %s to %s (%s)",
				    o.charName.c_str(),
				    o.newCharName.c_str(),
				    HkGetAccountID(acc).c_str());
			}
			catch (char* err)
			{
				AddLog(LogType::Normal,
				    LogLevel::Err,
				    L"User rename failed (%s) from %s to %s (%s)",
				    err,
				    o.charName.c_str(),
				    o.newCharName.c_str(),
				    HkGetAccountID(acc).c_str());
			}
		}

		while (global->pendingMoves.empty())
		{
			Move o = global->pendingMoves.front();
			if (HkGetClientIdFromCharname(o.destinationCharName) != -1)
				return;
			if (HkGetClientIdFromCharname(o.movingCharName) != -1)
				return;

			global->pendingMoves.erase(global->pendingMoves.begin());

			CAccount* acc = HkGetAccountByCharname(o.destinationCharName);
			CAccount* oldAcc = HkGetAccountByCharname(o.movingCharName);

			// Delete the character from the existing account, create a new
			// character with the same name in this account and then copy over it
			// with the save character file.
			try
			{
				HkLockAccountAccess(acc, true);
				HkUnlockAccountAccess(acc);

				HkLockAccountAccess(oldAcc, true);
				HkUnlockAccountAccess(oldAcc);

				// Move the char file to a temporary one.
				if (!::MoveFileExA(o.sourceFile.c_str(), o.destFileTemp.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
					throw "move src to temp failed";

				// Create and delete the character
				HkDeleteCharacter(oldAcc, o.movingCharName);
				HkNewCharacter(acc, o.movingCharName);

				// Move files around
				if (!::MoveFileExA(o.destFileTemp.c_str(), o.destFile.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
					throw "move failed";
				if (::PathFileExistsA(o.sourceFile.c_str()))
					throw "src still exists";
				if (!::PathFileExistsA(o.destFile.c_str()))
					throw "dest does not exist";

				// The move worked. Log it.
				AddLog(LogType::Normal, LogLevel::Info, L"Character %s moved from %s to %s", o.movingCharName.c_str(), HkGetAccountID(oldAcc).c_str(), HkGetAccountID(acc).c_str()); 
			}
			catch (char* err)
			{
				AddLog(LogType::Normal, LogLevel::Err, L"Character %s move failed (%s) from %s to %s", o.movingCharName.c_str(), err, HkGetAccountID(oldAcc).c_str(), HkGetAccountID(acc).c_str());
			}
		}
	}

	const std::vector<Timer> timers = {
		{ RenameTimer, 5 }
	};

	void UserCmd_RenameMe(const uint& iClientID, const std::wstring_view& wscParam)
	{
		HK_ERROR err;

		// Don't indicate an error if moving is disabled.
		if (!global->config.enableRenameMe)
			return;

		// Indicate an error if the command does not appear to be formatted
		// correctly and stop processing but tell FLHook that we processed the
		// command.
		if (wscParam.empty())
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /renameme <charname> [password]");
			return;
		}

		uint iBaseID;
		pub::Player::GetBase(iClientID, iBaseID);
		if (!iBaseID)
		{
			PrintUserCmdText(iClientID, L"ERR Not in base");
			return;
		}

		// If the new name contains spaces then flag this as an
		// error.
		std::wstring wscNewCharname = Trim(GetParam(wscParam, L' ', 0));
		if (wscNewCharname.find(L" ") != -1)
		{
			PrintUserCmdText(iClientID, L"ERR Space characters not allowed in name");
			return;
		}

		if (HkGetAccountByCharname(wscNewCharname))
		{
			PrintUserCmdText(iClientID, L"ERR Name already exists");
			return;
		}

		if (wscNewCharname.length() > 23)
		{
			PrintUserCmdText(iClientID, L"ERR Name to long");
			return;
		}

		if (wscNewCharname.length() < MinCharacterNameLength)
		{
			PrintUserCmdText(iClientID, L"ERR Name to short");
			return;
		}

		if (global->config.charNameTags)
		{
			std::wstring wscPassword = Trim(GetParam(wscParam, L' ', 1));

			for (const auto& i : global->tagList.tags)
			{
				if (wscNewCharname.find(i.tag) == 0 && !i.renamePassword.empty())
				{
					if (!wscPassword.length())
					{
						PrintUserCmdText(iClientID, L"ERR Name starts with an owned tag. Password is required.");
						return;
					}
					else if (wscPassword != i.masterPassword && wscPassword != i.renamePassword)
					{
						PrintUserCmdText(iClientID, L"ERR Name starts with an owned tag. Password is wrong.");
						return;
					}
					// Password is valid for owned tag.
					break;
				}
			}
		}

		// Get the character name for this connection.
		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		// Saving the characters forces an anti-cheat checks and fixes
		// up a multitude of other problems.
		HkSaveChar(wscCharname);
		if (!HkIsValidClientID(iClientID))
			return;

		// Read the current number of credits for the player
		// and check that the character has enough cash.
		int iCash = 0; 
		if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
			return;
		}
		if (global->config.renameCost > 0 && iCash < global->config.renameCost)
		{
			PrintUserCmdText(iClientID, L"ERR Insufficient credits");
			return;
		}

		// Read the last time a rename was done on this character
		std::wstring wscDir;
		if ((err = HkGetAccountDirName(wscCharname, wscDir)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
			return;
		}
		std::string scRenameFile = scAcctPath + wstos(wscDir) + "\\" + "rename.ini";
		int lastRenameTime = IniGetI(scRenameFile, "General", wstos(wscCharname), 0);

		// If a rename was done recently by this player then reject the request.
		// I know that time() returns time_t...shouldn't matter for a few years
		// yet.
		if ((lastRenameTime + 300) < static_cast<int>(time(nullptr)))
		{
			if ((lastRenameTime + global->config.renameTimeLimit) > static_cast<int>(time(nullptr)))
			{
				PrintUserCmdText(iClientID, L"ERR Rename time limit");
				return;
			}
		}

		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		std::string scAcctPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\";

		std::wstring wscSourceFile;
		if ((err = HkGetCharFileName(wscCharname, wscSourceFile)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
			return;
		}
		std::wstring wscDestFile;
		if ((err = HkGetCharFileName(wscNewCharname, wscDestFile)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
			return;
		}

		// Remove cash if we're charging for it.
		if (global->config.renameCost > 0)
			HkAddCash(wscCharname, 0 - global->config.renameCost);

		Rename o;
		o.charName = wscCharname;
		o.newCharName = wscNewCharname;
		o.sourceFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl";
		o.destFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscDestFile) + ".fl";
		o.destFileTemp = scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl.renaming";
		global->pendingRenames.emplace_back(o);

		HkKickReason(o.charName, L"Updating character, please wait 10 seconds before reconnecting");
		IniWrite(scRenameFile, "General", wstos(o.newCharName), std::to_string((int)time(0)));
	}

	/** Process a set the move char code command */
	void UserCmd_SetMoveCharCode(const uint& iClientID, const std::wstring_view& wscParam)
	{
		// Don't indicate an error if moving is disabled.
		if (!global->config.enableMoveChar)
			return;

		if (wscParam.empty())
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /set movecharcode <code>");
			return;
		}

		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
		std::string scFile = GetUserFilePath(wscCharname, "-movechar.ini");
		if (scFile.empty())
		{
			PrintUserCmdText(iClientID, L"ERR Character does not exist");
			return;
		}

		std::wstring wscCode = Trim(GetParam(wscParam, L' ', 0));
		if (wscCode == L"none")
		{
			IniWriteW(scFile, "Settings", "Code", L"");
			PrintUserCmdText(iClientID, L"OK Movechar code cleared");
		}
		else
		{
			IniWriteW(scFile, "Settings", "Code", wscCode);
			PrintUserCmdText(iClientID, L"OK Movechar code set to " + wscCode);
		}
		return;
	}

	static bool IsBanned(std::wstring charname)
	{
		char datapath[MAX_PATH];
		GetUserDataPath(datapath);

		std::wstring dir;
		HkGetAccountDirName(charname, dir);

		std::string banfile = std::string(datapath) + "\\Accts\\MultiPlayer\\" + wstos(dir) + "\\banned";

		// Prevent ships from banned accounts from being moved.
		FILE* f;
		fopen_s(&f, banfile.c_str(), "r");
		if (f)
		{
			fclose(f);
			return true;
		}
		return false;
	}

	/**
	 Move a character from a remote account into this one.
	*/
	void UserCmd_MoveChar(const uint& iClientID, const std::wstring_view& wscParam)
	{
		HK_ERROR err;

		// Don't indicate an error if moving is disabled.
		if (!global->config.enableMoveChar)
			return;

		// Indicate an error if the command does not appear to be formatted
		// correctly and stop processing but tell FLHook that we processed the
		// command.
		if (wscParam.empty())
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, L"Usage: /movechar <charname> <code>");
			return;
		}

		uint iBaseID;
		pub::Player::GetBase(iClientID, iBaseID);
		if (!iBaseID)
		{
			PrintUserCmdText(iClientID, L"ERR Not in base");
			return;
		}

		// Get the target account directory.
		std::wstring wscMovingCharname = Trim(GetParam(wscParam, L' ', 0));
		std::string scFile = GetUserFilePath(wscMovingCharname, "-movechar.ini");
		if (scFile.empty())
		{
			PrintUserCmdText(iClientID, L"ERR Character does not exist");
			return;
		}

		// Check the move char code.
		std::wstring wscCode = Trim(GetParam(wscParam, L' ', 1));
		std::wstring wscTargetCode = IniGetWS(scFile, "Settings", "Code", L"");
		if (!wscTargetCode.length() || wscTargetCode != wscCode)
		{
			PrintUserCmdText(iClientID, L"ERR Move character access denied");
			return;
		}

		// Prevent ships from banned accounts from being moved.
		if (IsBanned(wscMovingCharname))
		{
			PrintUserCmdText(iClientID, L"ERR not permitted");
			return;
		}

		std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		// Saving the characters forces an anti-cheat checks and fixes
		// up a multitude of other problems.
		HkSaveChar(wscCharname);
		HkSaveChar(wscMovingCharname);

		// Read the current number of credits for the player
		// and check that the character has enough cash.
		int iCash = 0;
		if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
			return;
		}
		if (global->config.moveCost > 0 && iCash < global->config.moveCost)
		{
			PrintUserCmdText(iClientID, L"ERR Insufficient credits");
			return;
		}

		// Check there is room in this account.
		if (CAccount* acc = Players.FindAccountFromClientID(iClientID); acc->iNumberOfCharacters >= 5)
		{
			PrintUserCmdText(iClientID, L"ERR Too many characters in account");
			return;
		}

		// Copy character file into this account with a temp name.
		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		std::string scAcctPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\";

		std::wstring wscDir;
		std::wstring wscSourceDir;
		std::wstring wscSourceFile;
		if ((err = HkGetAccountDirName(wscCharname, wscDir)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
			return;
		}
		if ((err = HkGetAccountDirName(wscMovingCharname, wscSourceDir)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
			return;
		}
		if ((err = HkGetCharFileName(wscMovingCharname, wscSourceFile)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
			return;
		}

		// Remove cash if we're charging for it.
		if (global->config.moveCost > 0)
			HkAddCash(wscCharname, 0 - global->config.moveCost);
		HkSaveChar(wscCharname);

		// Schedule the move
		Move o;
		o.destinationCharName = wscCharname;
		o.movingCharName = wscMovingCharname;
		o.sourceFile = scAcctPath + wstos(wscSourceDir) + "\\" + wstos(wscSourceFile) + ".fl";
		o.destFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl";
		o.destFileTemp = scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl.moving";
		global->pendingMoves.emplace_back(o);

		// Delete the move code
		::DeleteFileA(scFile.c_str());

		// Kick
		HkKickReason(o.destinationCharName, L"Moving character, please wait 10 seconds before reconnecting");
		HkKickReason(o.movingCharName, L"Moving character, please wait 10 seconds before reconnecting");
	}

	/// Set the move char code for all characters in the account
	void AdminCmd_SetAccMoveCode(CCmds* cmds, const std::wstring& wscCharname, const std::wstring& wscCode)
	{
		// Don't indicate an error if moving is disabled.
		if (!global->config.enableMoveChar)
			return;

		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		std::wstring wscDir;
		if (HkGetAccountDirName(wscCharname, wscDir) != HKE_OK)
		{
			cmds->Print(L"ERR Charname not found");
			return;
		}

		if (wscCode.length() == 0)
		{
			cmds->Print(L"ERR Code too small, set to none to clear.");
			return;
		}

		// Get the account path.
		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		std::string scPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\" + wstos(wscDir) + "\\*.fl";

		// Open the directory iterator.
		WIN32_FIND_DATA FindFileData;
		HANDLE hFileFind = FindFirstFile(scPath.c_str(), &FindFileData);
		if (hFileFind == INVALID_HANDLE_VALUE)
		{
			cmds->Print(L"ERR Account directory not found");
			return;
		}

		// Iterate it
		do
		{
			std::string scCharfile = FindFileData.cFileName;
			std::string scMoveCodeFile =
			    std::string(szDataPath) + "\\Accts\\MultiPlayer\\" + wstos(wscDir) + "\\" + scCharfile.substr(0, scCharfile.size() - 3) + "-movechar.ini";
			if (wscCode == L"none")
			{
				IniWriteW(scMoveCodeFile, "Settings", "Code", L"");
				cmds->Print(L"OK Movechar code cleared on " + stows(scCharfile) + L"\n");
			}
			else
			{
				IniWriteW(scMoveCodeFile, "Settings", "Code", wscCode);
				cmds->Print(L"OK Movechar code set to " + wscCode + L" on " + stows(scCharfile) + L"\n");
			}
		} while (FindNextFile(hFileFind, &FindFileData));
		FindClose(hFileFind);

		cmds->Print(L"OK");
	}

	/// Set the move char code for all characters in the account
	void AdminCmd_ShowTags(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		const uint curr_time = static_cast<uint>(time(nullptr));
		for (const auto& tag : global->tagList.tags)
		{
			int last_access = tag.lastAccess;
			int days = (curr_time - last_access) / (24 * 3600);
			cmds->Print(L"tag=%s master_password=%s rename_password=%s last_access=%u days description=%s\n", tag.tag.c_str(), tag.masterPassword.c_str(), tag.renamePassword.c_str(), days,
			    tag.description.c_str());
		}
		cmds->Print(L"OK");
	}

	void AdminCmd_AddTag(CCmds* cmds, const std::wstring& tag, const std::wstring& password, const std::wstring& description)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		if (tag.size() < 3)
		{
			cmds->Print(L"ERR Tag too short");
			return;
		}

		if (password.empty())
		{
			cmds->Print(L"ERR Password not set");
			return;
		}

		if (description.empty())
		{
			cmds->Print(L"ERR Description not set");
			return;
		}

		// If this tag is in use then reject the request.
		if (global->tagList.FindTagPartial(tag) == global->tagList.tags.end())
		{
			cmds->Print(L"ERR Tag already exists or conflicts with another tag\n");
			return;
		}

		TagData data;
		data.tag = tag;
		data.masterPassword = password;
		data.renamePassword = L"";
		data.lastAccess = static_cast<uint>(time(nullptr));
		data.description = description;
		cmds->Print(L"Created faction tag %s with master password %s", tag.c_str(), password.c_str());
		global->tagList.tags.emplace_back(data);
		SaveSettings();
	}

	void AdminCmd_DropTag(CCmds* cmds, const std::wstring& tag)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission");
			return;
		}

		if (global->tagList.FindTag(tag) != global->tagList.tags.end())
		{
			global->tagList.tags.erase(
			    std::remove_if(global->tagList.tags.begin(), global->tagList.tags.end(), [tag](const TagData& tg) { return tg.tag == tag; }),
			    global->tagList.tags.end());
			SaveSettings();
			cmds->Print(L"OK Tag dropped");
			return;
		}

		cmds->Print(L"ERR tag is invalid");
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/maketag", L"", UserCmd_MakeTag, L""),
	    CreateUserCommand(L"/droptag", L"", UserCmd_DropTag, L""),
	    CreateUserCommand(L"/settagpass", L"", UserCmd_SetTagPass, L""),
	    CreateUserCommand(L"/renameme", L"", UserCmd_RenameMe, L""),
	    CreateUserCommand(L"/movechar", L"", UserCmd_MoveChar, L""),
	    CreateUserCommand(L"/set movecharcode", L"", UserCmd_SetMoveCharCode, L""),
	}};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** Admin help command callback */
	void CmdHelp(CCmds* classptr)
	{
		classptr->Print(L"setaccmovecode <charname> <code>");
		classptr->Print(L"showtags");
		classptr->Print(L"addtag <tag> <password>");
		classptr->Print(L"droptag <tag> <password>");
	}

	bool ExecuteCommandString(CCmds* cmds, const std::wstring& wscCmd)
	{
		if (wscCmd == L"setaccmovecode")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_SetAccMoveCode(cmds, cmds->ArgCharname(1), cmds->ArgStr(2));
			return true;
		}
		else if (wscCmd == L"showtags")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_ShowTags(cmds);
			return true;
		}
		else if (wscCmd == L"addtag")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_AddTag(cmds, cmds->ArgStr(1), cmds->ArgStr(2), cmds->ArgStrToEnd(3));
			return true;
		}
		else if (wscCmd == L"droptag")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_DropTag(cmds, cmds->ArgStr(1));
			return true;
		}
		return true;
	}

	std::vector<TagData>::iterator TagList::FindTag(const std::wstring& tag)
	{
		return std::find_if(tags.begin(), tags.end(), [tag](const TagData& data) { return data.tag == tag; });
	}

	std::vector<TagData>::iterator TagList::FindTagPartial(const std::wstring& tag)
	{
		return std::find_if(tags.begin(), tags.end(), [tag](const TagData& data) { return data.tag.find(tag) || tag.find(data.tag); });
	}
} // namespace Plugins::Rename

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::Rename;
REFL_AUTO(type(TagData), field(tag), field(masterPassword), field(renamePassword), field(lastAccess), field(description));
REFL_AUTO(type(TagList), field(tags));
REFL_AUTO(type(Config), field(enableRenameMe), field(enableMoveChar), field(moveCost), field(renameCost), field(renameTimeLimit), field(charNameTags), field(asciiCharNameOnly), field(makeTagCost));

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Rename");
	pi->shortName("rename");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->commands(commands);
	pi->timers(timers);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__CreateNewCharacter, &CreateNewCharacter);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
}
