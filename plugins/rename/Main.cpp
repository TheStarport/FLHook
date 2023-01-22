// Rename plugin - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"
#include "Features/Mail.hpp"

namespace Plugins::Rename
{
	const auto global = std::make_unique<Global>();
	void LoadSettings()
	{
		global->tagList = Serializer::JsonToObject<TagList>();
		global->config = std::make_unique<Config>(Serializer::JsonToObject<Config>());
	}

	void SaveSettings()
	{
		Serializer::SaveToJson(global->tagList);
	}

	bool CreateNewCharacter(SCreateCharacterInfo const& si, ClientId& client)
	{
		if (global->config->enableTagProtection)
		{
			// If this ship name starts with a restricted tag then the ship may only be created using rename and the faction password
			const std::wstring charName(si.wszCharname);
			if (const auto& tag = global->tagList.FindTagPartial(charName); tag != global->tagList.tags.end() && !tag->renamePassword.empty())
			{
				Server.CharacterInfoReq(client, true);
				return true;
			}

			// If this ship name is too short, reject the request
			if (charName.size() < MinCharacterNameLength + 1)
			{
				Server.CharacterInfoReq(client, true);
				return true;
			}
		}

		if (global->config->asciiCharNameOnly)
		{
			const std::wstring charname(si.wszCharname);
			for (const wchar_t ch : charname)
			{
				if (ch & 0xFF80)
					return true;
			}
		}

		return false;
	}

	bool DeleteCharacter(const std::string& charFilename, ClientId& client)
	{
		const auto dir = Hk::Client::GetAccountDirName(Hk::Client::GetAccountByClientID(client));
		const std::string accDir = CoreGlobals::c()->accPath + wstos(dir) + "\\";
		const std::string renameIniDir = accDir + "rename.ini";
		const std::string charDir = accDir + charFilename.c_str();
		if (!std::filesystem::exists(renameIniDir))
			return true;

		std::wstring charName = IniGetWS(charDir, "Player", "Name", L"");

		IniDelete(renameIniDir, "General", wstos(charName));

		return true;
	}

	// Update the tag list when a character is selected update the tag list to
	// indicate that this tag is in use. If a tag is not used after 60 days, remove
	// it.
	void CharacterSelect_AFTER([[maybe_unused]] const std::string& charFilename, ClientId& client)
	{
		if (!global->config->enableTagProtection)
			return;

		const auto charName = Hk::Client::GetCharacterNameByID(client);
		if (const auto& tag = global->tagList.FindTagPartial(charName.value()); tag != global->tagList.tags.end() && !tag->renamePassword.empty())
		{
			tag->lastAccess = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		}
	}

	void UserCmd_MakeTag(ClientId& client, const std::wstring& param)
	{
		if (!global->config->enableTagProtection)
		{
			PrintUserCmdText(client, L"Command disabled");
			return;
		}

		const std::wstring usage = L"Usage: /maketag <tag> <master password> <description>";
		// Indicate an error if the command does not appear to be formatted
		// correctly and stop processing but tell FLHook that we processed the command.
		if (param.empty())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, usage);
			return;
		}

		if (auto base = Hk::Player::GetCurrentBase(client); base.has_error())
		{
			PrintUserCmdText(client, L"ERR Not in base");
			return;
		}

		std::wstring tag = GetParam(param, ' ', 0);
		std::wstring pass = GetParam(param, ' ', 1);
		std::wstring description = GetParamToEnd(param, ' ', 2);

		if (tag.size() < MinCharacterNameLength)
		{
			PrintUserCmdText(client, L"ERR Tag too short");
			PrintUserCmdText(client, usage);
			return;
		}

		if (pass.empty())
		{
			PrintUserCmdText(client, L"ERR Password not set");
			PrintUserCmdText(client, usage);
			return;
		}

		if (description.empty())
		{
			PrintUserCmdText(client, L"ERR Description not set");
			PrintUserCmdText(client, usage);
			return;
		}

		// If this tag is in use then reject the request.
		for (const auto& i : global->tagList.tags)
		{
			if (tag.find(i.tag) == 0 || i.tag.find(tag) == 0)
			{
				PrintUserCmdText(client, L"ERR Tag already exists or conflicts with existing tag");
				return;
			}
		}

		// Save character and exit if kicked on save.
		const auto charName = Hk::Client::GetCharacterNameByID(client);
		Hk::Player::SaveChar(charName.value());
		if (Hk::Client::GetClientIdFromCharName(charName.value()) == -1)
			return;

		if (const auto iCash = Hk::Player::GetCash(client); global->config->makeTagCost > 0 && iCash < global->config->makeTagCost)
		{
			PrintUserCmdText(client, L"ERR Insufficient credits");
			return;
		}

		Hk::Player::RemoveCash(client, global->config->makeTagCost);

		TagData data;

		data.masterPassword = pass;
		data.renamePassword = L"";
		data.lastAccess = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		data.description = description;
		data.tag = tag;
		global->tagList.tags.emplace_back(data);

		PrintUserCmdText(client, std::format(L"Created faction tag {} with master password {}", tag, pass));
		AddLog(LogType::Normal, LogLevel::Info, wstos(std::format(L"Tag {} created by {} ({})", tag.c_str(), charName.value().c_str(), Hk::Client::GetAccountIdByClientID(client).c_str())));
		SaveSettings();
	}

	void UserCmd_DropTag(ClientId& client, const std::wstring& param)
	{
		if (!global->config->enableTagProtection)
		{
			PrintUserCmdText(client, L"Command disabled");
			return;
		}

		// Indicate an error if the command does not appear to be formatted
		// correctly and stop processing but tell FLHook that we processed the
		// command.
		if (param.empty())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /droptag <tag> <master password>");
			return;
		}

		std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
		std::wstring tag = GetParam(param, ' ', 0);
		std::wstring pass = GetParam(param, ' ', 1);

		// If this tag is in use then reject the request.
		if (const auto& data = global->tagList.FindTag(tag); data != global->tagList.tags.end() && pass == data->masterPassword)
		{	
			const auto [first, last] = std::ranges::remove_if(global->tagList.tags, [&tag](const TagData& tg) { return tg.tag == tag; });
			global->tagList.tags.erase(first, last);
			SaveSettings();
			PrintUserCmdText(client, L"OK Tag dropped");
			AddLog(LogType::Normal, LogLevel::Info, wstos(std::format(L"Tag {} dropped by {} ({})", tag.c_str(), charname.c_str(), Hk::Client::GetAccountIdByClientID(client).c_str())));
			return;
		}

		PrintUserCmdText(client, L"ERR tag or master password are invalid");
	}

	// Make tag password
	void UserCmd_SetTagPass(ClientId& client, const std::wstring& param)
	{
		if (global->config->enableTagProtection)
		{
			// Indicate an error if the command does not appear to be formatted
			// correctly and stop processing but tell FLHook that we processed the command.
			if (param.empty())
			{
				PrintUserCmdText(client, L"ERR Invalid parameters");
				PrintUserCmdText(client, L"Usage: /settagpass <tag> <master password> <rename password>");
				return;
			}

			const std::wstring tag = GetParam(param, ' ', 0);
			const std::wstring masterPassword = GetParam(param, ' ', 1);
			const std::wstring renamePassword = GetParam(param, ' ', 2);

			// If this tag is in use then reject the request.
			if (const auto& data = global->tagList.FindTag(tag); data != global->tagList.tags.end() && masterPassword == data->masterPassword)
			{
				data->renamePassword = renamePassword;
				SaveSettings();
				PrintUserCmdText(client, std::format(L"OK Created rename password {} for tag {}", renamePassword, tag));
				return;
			}
			PrintUserCmdText(client, L"ERR tag or master password are invalid");
		}
		else
		{
			PrintUserCmdText(client, L"Command disabled");
		}
	}

	void RenameTimer()
	{
		// Check for pending renames and execute them. We do this on a timer so that
		// the player is definitely not online when we do the rename.
		while (!global->pendingRenames.empty())
		{
			Rename o = global->pendingRenames.front();
			if (Hk::Client::GetClientIdFromCharName(o.charName).has_value())
				return;

			global->pendingRenames.erase(global->pendingRenames.begin());

			CAccount* acc = Hk::Client::GetAccountByCharName(o.charName).value();

			// Delete the character from the existing account, create a new
			// character with the same name in this account and then copy over it
			// with the save character file.
			try
			{
				if (!acc)
					throw "no acc";

				Hk::Client::LockAccountAccess(acc, true);
				Hk::Client::UnlockAccountAccess(acc);

				// Move files around
				if (!::MoveFileExA(o.sourceFile.c_str(), o.destFile.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
					throw "move failed";
				if (std::filesystem::exists(o.sourceFile.c_str()))
					throw "src still exists";
				if (!std::filesystem::exists(o.destFile.c_str()))
					throw "dest does not exist";


				// Decode the char file, update the char name and re-encode it.
				// Add a space to the value so the ini file line looks like "<key> =
				// <value>" otherwise Ioncross Server Operator can't decode the file
				// correctly
				FlcDecodeFile(o.destFile.c_str(), o.destFile.c_str());
				IniWriteW(o.destFile, "Player", "Name", o.newCharName);
				if (!FLHookConfig::i()->general.disableCharfileEncryption)
				{
					FlcEncodeFile(o.destFile.c_str(), o.destFile.c_str());
				}

				// Update any mail references this character had before
				MailManager::i()->UpdateCharacterName(wstos(o.charName), wstos(o.newCharName));

				// The rename worked. Log it and save the rename time.
				AddLog(LogType::Normal,
				    LogLevel::Info,
				    wstos(std::format(L"User rename {} to {} ({})",
				    o.charName.c_str(),
				    o.newCharName.c_str(),
				    Hk::Client::GetAccountID(acc).value().c_str())));
			}
			catch (char* err)
			{
				AddLog(LogType::Normal,
				    LogLevel::Err,
				    wstos(std::format(L"User rename failed ({}) from {} to {} ({})",
				    stows(err),
				    o.charName.c_str(),
				    o.newCharName.c_str(),
				    Hk::Client::GetAccountID(acc).value().c_str())));
			}
		}

		while (!global->pendingMoves.empty())
		{
			Move o = global->pendingMoves.front();
			if (Hk::Client::GetClientIdFromCharName(o.destinationCharName).has_value()
			||	Hk::Client::GetClientIdFromCharName(o.movingCharName).has_value())
				return;

			global->pendingMoves.erase(global->pendingMoves.begin());

			CAccount* acc = Hk::Client::GetAccountByCharName(o.destinationCharName).value();
			CAccount* oldAcc = Hk::Client::GetAccountByCharName(o.movingCharName).value();

			// Delete the character from the existing account, create a new
			// character with the same name in this account and then copy over it
			// with the save character file.
			try
			{
				Hk::Client::LockAccountAccess(acc, true);
				Hk::Client::UnlockAccountAccess(acc);

				Hk::Client::LockAccountAccess(oldAcc, true);
				Hk::Client::UnlockAccountAccess(oldAcc);

				// Move files around
				if (!::MoveFileExA(o.sourceFile.c_str(), o.destFile.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
					throw "move failed";
				if (std::filesystem::exists(o.sourceFile.c_str()))
					throw "src still exists";
				if (!std::filesystem::exists(o.destFile.c_str()))
					throw "dest does not exist";

				std::string oldAccDir = CoreGlobals::c()->accPath + wstos(Hk::Client::GetAccountDirName(oldAcc));

				if (std::wstring oldCharRenameLimit = IniGetWS(oldAccDir + "\\rename.ini", "General", wstos(o.movingCharName), L""); 
					!oldCharRenameLimit.empty())
				{
					std::string newAccDir = CoreGlobals::c()->accPath + wstos(Hk::Client::GetAccountDirName(acc));
					IniWriteW(newAccDir + "\\rename.ini", "General", wstos(o.movingCharName), oldCharRenameLimit);
					IniDelete(oldAccDir + "\\rename.ini", "General", wstos(o.movingCharName));
				}

				// The move worked. Log it.
				AddLog(LogType::Normal,
				    LogLevel::Info,
				    wstos(std::format(L"Character {} moved from {} to {}",
				    o.movingCharName.c_str(),
				    Hk::Client::GetAccountID(oldAcc).value().c_str(),
				    Hk::Client::GetAccountID(acc).value().c_str()))); 
			}
			catch (char* err)
			{
				AddLog(LogType::Normal,
				    LogLevel::Err,
				    wstos(std::format(L"Character {} move failed ({}) from {} to {}",
				    o.movingCharName,
				    stows(std::string(err)),
				    Hk::Client::GetAccountID(oldAcc).value().c_str(),
				    Hk::Client::GetAccountID(acc).value().c_str())));
			}
		}
	}

	const std::vector<Timer> timers = {
		{ RenameTimer, 5 }
	};

	void UserCmd_Rename(ClientId& client, const std::wstring& param)
	{

		if (!global->config->enableRename)
		{
			PrintUserCmdText(client, L"Command disabled");
			return;
		}

		// Indicate an error if the command does not appear to be formatted
		// correctly and stop processing but tell FLHook that we processed the
		// command.
		if (param.empty())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /renameme <charname> [password]");
			return;
		}

		if (auto base = Hk::Player::GetCurrentBase(client); base.has_error())
		{
			PrintUserCmdText(client, L"ERR Not in base");
			return;
		}

		// If the new name contains spaces then flag this as an
		// error.
		std::wstring newCharName = Trim(GetParam(param, L' ', 0));
		if (newCharName.find(L" ") != -1)
		{
			PrintUserCmdText(client, L"ERR Space characters not allowed in name");
			return;
		}

		if (Hk::Client::GetAccountByCharName(newCharName).has_value())
		{
			PrintUserCmdText(client, L"ERR Name already exists");
			return;
		}

		if (newCharName.length() > 23)
		{
			PrintUserCmdText(client, L"ERR Name too long");
			return;
		}

		if (newCharName.length() < MinCharacterNameLength)
		{
			PrintUserCmdText(client, L"ERR Name too short");
			return;
		}

		if (global->config->enableTagProtection)
		{
			std::wstring password = Trim(GetParam(param, L' ', 1));

			for (const auto& i : global->tagList.tags)
			{
				if (newCharName.find(i.tag) == 0 && !i.renamePassword.empty())
				{
					if (!password.length())
					{
						PrintUserCmdText(client, L"ERR Name starts with an owned tag. Password is required.");
						return;
					}
					else if (password != i.masterPassword && password != i.renamePassword)
					{
						PrintUserCmdText(client, L"ERR Name starts with an owned tag. Password is wrong.");
						return;
					}
					// Password is valid for owned tag.
					break;
				}
			}
		}

		// Get the character name for this connection.
		std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);

		// Saving the characters forces an anti-cheat checks and fixes
		// up a multitude of other problems.
		Hk::Player::SaveChar(charname);
		if (!Hk::Client::IsValidClientID(client))
			return;

		// Read the current number of credits for the player
		// and check that the character has enough cash.
		const auto cash = Hk::Player::GetCash(charname);
		if (cash.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(cash.error()));
			return;
		}
		if (global->config->renameCost > 0 && cash < global->config->renameCost)
		{
			PrintUserCmdText(client, L"ERR Insufficient credits");
			return;
		}

		// Read the last time a rename was done on this character
		const auto dir = Hk::Client::GetAccountDirName(Hk::Client::GetAccountByClientID(client));
		std::string renameFile = CoreGlobals::c()->accPath + wstos(dir) + "\\" + "rename.ini";

		// If a rename was done recently by this player then reject the request.
		// I know that time() returns time_t...shouldn't matter for a few years
		// yet.
		if (uint lastRenameTime = IniGetI(renameFile, "General", wstos(charname), 0); (lastRenameTime + 300) < Hk::Time::GetUnix()
		&& (lastRenameTime + global->config->renameTimeLimit) > Hk::Time::GetUnix())
		{
			PrintUserCmdText(client, L"ERR Rename time limit");
			return;
		}

		std::string accPath = CoreGlobals::c()->accPath;

		const auto sourceFile = Hk::Client::GetCharFileName(charname);
		if (sourceFile.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(sourceFile.error()));
			return;
		}

		const auto destFile = Hk::Client::GetCharFileName(newCharName, true);
		if (destFile.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(destFile.error()));
			return;
		}

		// Remove cash if we're charging for it.
		if (global->config->renameCost > 0)
			Hk::Player::RemoveCash(charname, global->config->renameCost);

		Rename o;
		o.charName = charname;
		o.newCharName = newCharName;
		o.sourceFile = accPath + wstos(dir) + "\\" + wstos(sourceFile.value()) + ".fl";
		o.destFile = accPath + wstos(dir) + "\\" + wstos(destFile.value()) + ".fl";
		global->pendingRenames.emplace_back(o);

		Hk::Player::KickReason(o.charName, L"Updating character, please wait 10 seconds before reconnecting");
		IniWrite(renameFile, "General", wstos(o.newCharName), std::to_string(Hk::Time::GetUnix()));
	}

	/** Process a set the move char code command */
	void UserCmd_SetMoveCharCode(ClientId& client, const std::wstring& param)
	{
		if (!global->config->enableMoveChar)
		{
			PrintUserCmdText(client, L"Command disabled");
			return;
		}

		if (param.empty())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /setmovecode <code>");
			return;
		}

		std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);
		std::string scFile = GetUserFilePath(charname, "-movechar.ini");
		if (scFile.empty())
		{
			PrintUserCmdText(client, L"ERR Character does not exist");
			return;
		}

		if (std::wstring code = Trim(GetParam(param, L' ', 0)); code == L"none")
		{
			IniWriteW(scFile, "Settings", "Code", L"");
			PrintUserCmdText(client, L"OK Movechar code cleared");
		}
		else
		{
			IniWriteW(scFile, "Settings", "Code", code);
			PrintUserCmdText(client, L"OK Movechar code set to " + code);
		}
		return;
	}

	static bool IsBanned(const std::wstring& charname)
	{
		std::wstring dir = Hk::Client::GetAccountDirName(Hk::Client::GetAccountByCharName(charname).value());

		std::string banfile = CoreGlobals::c()->accPath + wstos(dir) + "\\banned";

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
	void UserCmd_MoveChar(ClientId& client, const std::wstring& param)
	{

		if (!global->config->enableMoveChar)
		{
			PrintUserCmdText(client, L"Command disabled");
			return;
		}

		// Indicate an error if the command does not appear to be formatted
		// correctly and stop processing but tell FLHook that we processed the
		// command.
		if (param.empty())
		{
			PrintUserCmdText(client, L"ERR Invalid parameters");
			PrintUserCmdText(client, L"Usage: /movechar <charname> <code>");
			return;
		}

		if (auto base = Hk::Player::GetCurrentBase(client); base.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(base.error()));
			return;
		}

		// Get the target account directory.
		std::wstring movingCharName = Trim(GetParam(param, L' ', 0));
		std::string scFile = GetUserFilePath(movingCharName, "-movechar.ini");
		if (scFile.empty())
		{
			PrintUserCmdText(client, L"ERR Character does not exist");
			return;
		}

		// Check the move char code.
		std::wstring code = Trim(GetParam(param, L' ', 1));
		if (std::wstring targetCode = IniGetWS(scFile, "Settings", "Code", L""); targetCode != code)
		{
			PrintUserCmdText(client, L"ERR Move character access denied");
			return;
		}

		// Prevent ships from banned accounts from being moved.
		if (IsBanned(movingCharName))
		{
			PrintUserCmdText(client, L"ERR not permitted");
			return;
		}

		std::wstring charname = (const wchar_t*)Players.GetActiveCharacterName(client);

		// Saving the characters forces an anti-cheat checks and fixes
		// up a multitude of other problems.
		Hk::Player::SaveChar(charname);
		Hk::Player::SaveChar(movingCharName);

		// Read the current number of credits for the player
		// and check that the character has enough cash.
		const auto cash = Hk::Player::GetCash(charname);
		if (cash.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(cash.error()));
			return;
		}

		if (global->config->moveCost > 0 && cash.value() < global->config->moveCost)
		{
			PrintUserCmdText(client, L"ERR Insufficient credits");
			return;
		}

		// Check there is room in this account.
		CAccount const* acc = Players.FindAccountFromClientID(client);
		if (acc && acc->iNumberOfCharacters >= 5)
		{
			PrintUserCmdText(client, L"ERR Too many characters in account");
			return;
		}

		// Copy character file into this account with a temp name.
		std::string acctPath = CoreGlobals::c()->accPath;

		const auto sourceAcc = Hk::Client::GetAccountByCharName(movingCharName);

		std::wstring dir = Hk::Client::GetAccountDirName(acc);
		std::wstring sourceDir = Hk::Client::GetAccountDirName(sourceAcc.value());
		const auto sourceFile = Hk::Client::GetCharFileName(movingCharName);
		if (sourceFile.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(sourceFile.error()));
			return;
		}

		// Remove cash if we're charging for it.
		if (global->config->moveCost > 0)
		{
			Hk::Player::RemoveCash(charname, global->config->moveCost);
		}

		Hk::Player::SaveChar(charname);

		// Schedule the move
		Move o;
		o.destinationCharName = charname;
		o.movingCharName = movingCharName;
		o.sourceFile = acctPath + wstos(sourceDir) + "\\" + wstos(sourceFile.value()) + ".fl";
		o.destFile = acctPath + wstos(dir) + "\\" + wstos(sourceFile.value()) + ".fl";
		global->pendingMoves.emplace_back(o);

		// Delete the move code
		::DeleteFileA(scFile.c_str());

		// Kick
		Hk::Player::KickReason(o.destinationCharName, L"Moving character, please wait 10 seconds before reconnecting");
		Hk::Player::KickReason(o.movingCharName, L"Moving character, please wait 10 seconds before reconnecting");
	}

	/// Set the move char code for all characters in the account
	void AdminCmd_SetAccMoveCode(CCmds* cmds, const std::wstring& charname, const std::wstring& code)
	{
		// Don't indicate an error if moving is disabled.
		if (!global->config->enableMoveChar)
			return;

		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print("ERR No permission");
			return;
		}

		
		const auto acc = Hk::Client::GetAccountByCharName(charname);
		
		if (acc.has_error())
		{
			cmds->Print("ERR Charname not found");
			return;
		}

		if (code.length() == 0)
		{
			cmds->Print("ERR Code too small, set to none to clear.");
			return;
		}

		std::wstring dir = Hk::Client::GetAccountDirName(acc.value());

		// Get the account path.
		std::string path = CoreGlobals::c()->accPath + "\\*.fl";

		// Open the directory iterator.
		WIN32_FIND_DATA FindFileData;
		HANDLE hFileFind = FindFirstFile(path.c_str(), &FindFileData);
		if (hFileFind == INVALID_HANDLE_VALUE)
		{
			cmds->Print("ERR Account directory not found");
			return;
		}

		// Iterate it
		do
		{
			std::string charFile = FindFileData.cFileName;
			std::string moveCodeFile = CoreGlobals::c()->accPath + wstos(dir) + "\\" + charFile.substr(0, charFile.size() - 3) + "-movechar.ini";
			if (code == L"none")
			{
				IniWriteW(moveCodeFile, "Settings", "Code", L"");
				cmds->Print(std::format("OK Movechar code cleared on {} \n", charFile));
			}
			else
			{
				IniWriteW(moveCodeFile, "Settings", "Code", code);
				cmds->Print(std::format("OK Movechar code set to {} on {} \n", wstos(code), charFile));
			}
		} while (FindNextFile(hFileFind, &FindFileData));
		FindClose(hFileFind);

		cmds->Print("OK");
	}

	/// Set the move char code for all characters in the account
	void AdminCmd_ShowTags(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print("ERR No permission");
			return;
		}

		const auto curr_time = Hk::Time::GetUnix();
		for (const auto& tag : global->tagList.tags)
		{
			auto last_access = static_cast<int>(tag.lastAccess);
			int days = (curr_time - last_access) / (24 * 3600);
			cmds->Print(wstos(std::format(L"tag={} master_password={} rename_password={} last_access={} days description={}\n", tag.tag, tag.masterPassword, tag.renamePassword, days,
			    tag.description)));
		}
		cmds->Print("OK");
	}

	void AdminCmd_AddTag(CCmds* cmds, const std::wstring& tag, const std::wstring& password, [[maybe_unused]] const std::wstring& description)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print("ERR No permission");
			return;
		}

		if (tag.size() < 3)
		{
			cmds->Print("ERR Tag too short");
			return;
		}

		if (password.empty())
		{
			cmds->Print("ERR Password not set");
			return;
		}

		if (description.empty())
		{
			cmds->Print("ERR Description not set");
			return;
		}

		// If this tag is in use then reject the request.
		if (global->tagList.FindTagPartial(tag) == global->tagList.tags.end())
		{
			cmds->Print("ERR Tag already exists or conflicts with another tag\n");
			return;
		}

		TagData data;
		data.tag = tag;
		data.masterPassword = password;
		data.renamePassword = L"";
		data.lastAccess = Hk::Time::GetUnix();
		data.description = description;
		cmds->Print(wstos(std::format(L"Created faction tag {} with master password {}", tag, password)));
		global->tagList.tags.emplace_back(data);
		SaveSettings();
	}

	void AdminCmd_DropTag(CCmds* cmds, const std::wstring& tag)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print("ERR No permission");
			return;
		}

		if (global->tagList.FindTag(tag) != global->tagList.tags.end())
		{
			auto [first, last] = std::ranges::remove_if(global->tagList.tags, [&tag](const TagData& tg) { return tg.tag == tag; });

			global->tagList.tags.erase(first, last);
			SaveSettings();
			cmds->Print("OK Tag dropped");
			return;
		}

		cmds->Print("ERR tag is invalid");
		return;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/maketag", L"<tag> <master password> <description>", UserCmd_MakeTag, L"Creates a faction tag and prevents others from creating said tag without a password."),
	    CreateUserCommand(L"/droptag", L"<tag> <master password>", UserCmd_DropTag, L"Deletes a faction tag"),
	    CreateUserCommand(L"/tagpass", L"<tag> <master password> <rename password>", UserCmd_SetTagPass, L"Set the passwords. Master is to manage the tag. Rename is the password to give to people who you wish to use the tag with the /rename command."),
	    CreateUserCommand(L"/rename", L"<charname> [password]", UserCmd_Rename, L"Renames the current character"),
	    CreateUserCommand(L"/movechar", L"<charname> <code>", UserCmd_MoveChar, L"Move a character from a remote account into this one"),
	    CreateUserCommand(L"/movecode", L"<code>", UserCmd_SetMoveCharCode, L"Set the password for this account if you wish to move it's characters to another account"),
	}};

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/** Admin help command callback */
	void CmdHelp(CCmds* classptr)
	{
		classptr->Print("setaccmovecode <charname> <code>");
		classptr->Print("showtags");
		classptr->Print("addtag <tag> <password>");
		classptr->Print("droptag <tag> <password>");
	}

	bool ExecuteCommandString(CCmds* cmds, const std::wstring& cmd)
	{
		if (cmd == L"setaccmovecode")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_SetAccMoveCode(cmds, cmds->ArgCharname(1), cmds->ArgStr(2));
			return true;
		}
		if (cmd == L"showtags")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_ShowTags(cmds);
			return true;
		}
		if (cmd == L"addtag")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_AddTag(cmds, cmds->ArgStr(1), cmds->ArgStr(2), cmds->ArgStrToEnd(3));
			return true;
		}
		if (cmd == L"droptag")
		{
			global->returnCode = ReturnCode::SkipAll;
			AdminCmd_DropTag(cmds, cmds->ArgStr(1));
		}
		return true;
	}

	std::vector<TagData>::iterator TagList::FindTag(const std::wstring& tag)
	{
		return std::ranges::find_if(tags, [&tag](const TagData& data) { return data.tag == tag; });
	}

	std::vector<TagData>::iterator TagList::FindTagPartial(const std::wstring& tag)
	{
		return std::ranges::find_if(tags, [&tag](const TagData& data) { return data.tag.find(tag) || tag.find(data.tag); });
	}
} // namespace Plugins::Rename

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::Rename;
REFL_AUTO(type(TagData), field(tag), field(masterPassword), field(renamePassword), field(lastAccess), field(description));
REFL_AUTO(type(TagList), field(tags));
REFL_AUTO(type(Config), field(enableRename), field(enableMoveChar), field(moveCost), field(renameCost), field(renameTimeLimit), field(enableTagProtection), field(asciiCharNameOnly), field(makeTagCost));

DefaultDllMainSettings(LoadSettings);

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Rename");
	pi->shortName("rename");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->timers(&timers);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__CreateNewCharacter, &CreateNewCharacter);
	pi->emplaceHook(HookedCall::IServerImpl__DestroyCharacter, &DeleteCharacter);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
}
