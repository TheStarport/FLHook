// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <float.h>
#include <FLHook.h>
#include <plugin.h>
#include <math.h>
#include <list>
#include <set>

#include "PluginUtilities.h"
#include "Main.h"

#include <FLCoreServer.h>
#include <FLCoreCommon.h>

#include "Shlwapi.h"

#define MIN_CHAR_TAG_LEN 3

namespace Rename
{
	static void ini_write_wstring(FILE *file, const string &parmname, const wstring &in)
	{
		fprintf(file, "%s=", parmname.c_str()); 
		for (int i = 0; i < (int)in.size(); i++)
		{
			UINT v1 = in[i] >> 8;
			UINT v2 = in[i] & 0xFF;
			fprintf(file, "%02x%02x", v1, v2); 
		}
		fprintf(file, "\n");
	}


	static void ini_get_wstring(INI_Reader &ini, wstring &wscValue)
	{
		string scValue = ini.get_value_string();
		wscValue = L"";
		long lHiByte;
		long lLoByte;
		while(sscanf(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
		{
			scValue = scValue.substr(4);
			wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
			wscValue.append(1, wChar);
		}
	}

	// Cost of the rename in credits
	int set_iMoveCost = 0;

	// Cost of the rename in credits
	int set_iRenameCost = 0;

	// Rename is not allowed if attempted within the rename time limit (in seconds)
	int set_iRenameTimeLimit = 0;

	// True if charname tags are supported
	bool set_bCharnameTags = false;

	// True if ascii only tags are supported
	bool set_bAsciiCharnameOnly = false;

	// The tag making cost
	int set_iMakeTagCost = 50000000;

	struct TAG_DATA
	{
		wstring tag;
		wstring master_password;
		wstring rename_password;
		uint last_access;
		wstring description;
	};

	std::map<wstring, TAG_DATA> mapTagToPassword;

	void LoadSettings(const string &scPluginCfgFile)
	{
		set_iRenameCost = IniGetI(scPluginCfgFile, "Rename", "RenameCost", 5000000);
		set_iRenameTimeLimit = IniGetI(scPluginCfgFile, "Rename", "RenameTimeLimit", 3600);
		set_iMoveCost = IniGetI(scPluginCfgFile, "Rename", "MoveCost", 5000000);
		set_bCharnameTags = IniGetB(scPluginCfgFile, "Rename", "CharnameTag", false);
		set_bAsciiCharnameOnly = IniGetB(scPluginCfgFile, "Rename", "AsciiCharnameOnly", true);
		set_iMakeTagCost = IniGetI(scPluginCfgFile, "Rename", "MakeTagCost", 50000000);

		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		string scPath = string(szDataPath) + "\\Accts\\MultiPlayer\\tags.ini";

		INI_Reader ini;
		if (ini.open(scPath.c_str(), false))
		{
			while (ini.read_header())
			{
				if (ini.is_header("faction"))
				{
					wstring tag;
					while (ini.read_value())
					{
						if (ini.is_value("tag"))
						{
							ini_get_wstring(ini, tag);
							mapTagToPassword[tag].tag = tag;
						}
						else if (ini.is_value("master_password"))
						{
							wstring pass;
							ini_get_wstring(ini, pass);
							mapTagToPassword[tag].master_password = pass;
						}
						else if (ini.is_value("rename_password"))
						{
							wstring pass;
							ini_get_wstring(ini, pass);
							mapTagToPassword[tag].rename_password = pass;
						}
						else if (ini.is_value("last_access"))
						{
							mapTagToPassword[tag].last_access = ini.get_value_int(0);
						}
						else if (ini.is_value("description"))
						{
							wstring description;
							ini_get_wstring(ini, description);
							mapTagToPassword[tag].description = description;
						}
					}
				}
			}
			ini.close();
		}
	}

	void SaveSettings()
	{
		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		string scPath = string(szDataPath) + "\\Accts\\MultiPlayer\\tags.ini";

		FILE *file = fopen(scPath.c_str(), "w");
		if (file)
		{
			for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
			{
				fprintf(file, "[faction]\n");
				ini_write_wstring(file, "tag", i->second.tag);
				ini_write_wstring(file, "master_password", i->second.master_password);
				ini_write_wstring(file, "rename_password", i->second.rename_password);
				ini_write_wstring(file, "description", i->second.description);
				fprintf(file, "last_access = %u\n", i->second.last_access);
			}
			fclose(file);
		}
	}

	bool CreateNewCharacter(struct SCreateCharacterInfo const &si, unsigned int iClientID)
	{
		if (set_bCharnameTags)
		{
			// If this ship name starts with a restricted tag then the ship may only be
			// created using rename and the faction password
			wstring wscCharname(si.wszCharname);
			for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
			{
				if (wscCharname.find(i->second.tag)==0
					&& i->second.rename_password.size()!=0)
				{
					Server.CharacterInfoReq(iClientID, true);
					return true;
				}
			}

			// If this ship name is too short, reject the request
			if (wscCharname.size() < MIN_CHAR_TAG_LEN + 1)
			{
				Server.CharacterInfoReq(iClientID, true);
				return true;
			}
		}

		if (set_bAsciiCharnameOnly)
		{
			wstring wscCharname(si.wszCharname);
			for (uint i=0; i<wscCharname.size(); i++)
			{
				wchar_t ch = wscCharname[i];
				if (ch&0xFF80)
					return true;
			}
		}

		return false;
	}

	// Update the tag list when a character is selected update the tag list to indicate that this tag
	// is in use. If a tag is not used after 60 days, remove it.
	void CharacterSelect_AFTER(struct CHARACTER_ID const &charId, unsigned int iClientID)
	{
		if (set_bCharnameTags)
		{
			wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
			for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
			{
				if (wscCharname.find(i->second.tag)==0)
				{
					i->second.last_access = (uint)time(0);
				}
			}
		}
	}

	bool UserCmd_MakeTag(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		if (set_bCharnameTags)
		{
			// Indicate an error if the command does not appear to be formatted correctly 
			// and stop processing but tell FLHook that we processed the command.
			if (wscParam.size()==0)
			{
				PrintUserCmdText(iClientID, L"ERR Invalid parameters");
				PrintUserCmdText(iClientID, usage);
				return true;
			}

			uint iBaseID;
			pub::Player::GetBase(iClientID, iBaseID);
			if (!iBaseID)
			{
				PrintUserCmdText(iClientID, L"ERR Not in base");
				return true;
			}

			wstring tag = GetParam(wscParam, ' ', 0);
			wstring pass = GetParam(wscParam, ' ', 1);
			wstring description = GetParamToEnd(wscParam, ' ', 2);

			if (tag.size() < MIN_CHAR_TAG_LEN)
			{
				PrintUserCmdText(iClientID, L"ERR Tag too short");
				PrintUserCmdText(iClientID, usage);
				return true;
			}

			if (!pass.size())
			{
				PrintUserCmdText(iClientID, L"ERR Password not set");
				PrintUserCmdText(iClientID, usage);
				return true;
			}

			if (!description.size())
			{
				PrintUserCmdText(iClientID, L"ERR Description not set");
				PrintUserCmdText(iClientID, usage);
				return true;
			}

			// If this tag is in use then reject the request.
			for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
			{
				if (tag.find(i->second.tag)==0 || i->second.tag.find(tag)==0)
				{
					PrintUserCmdText(iClientID, L"ERR Tag already exists or conflicts with existing tag");
					return true;
				}
			}

			// Save character and exit if kicked on save.
			wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
			HkSaveChar(wscCharname);
			if (HkGetClientIdFromCharname(wscCharname)==-1)
				return false;

			int iCash;
			HK_ERROR err;
			if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK)
			{
				PrintUserCmdText(iClientID, L"ERR "+HkErrGetText(err));
				return true;
			}
			if (set_iMakeTagCost>0 && iCash<set_iMakeTagCost)
			{
				PrintUserCmdText(iClientID, L"ERR Insufficient credits");
				return true;
			}

			HkAddCash(wscCharname, 0-set_iMakeTagCost);

			// TODO: Try to check if any player is using this tag
			mapTagToPassword[tag].tag = tag;
			mapTagToPassword[tag].master_password = pass;
			mapTagToPassword[tag].rename_password = L"";
			mapTagToPassword[tag].last_access = (uint)time(0);
			mapTagToPassword[tag].description = description;

			PrintUserCmdText(iClientID, L"Created faction tag %s with master password %s", tag.c_str(), pass.c_str());
			AddLog("NOTICE: Tag %s created by %s (%s)", wstos(tag).c_str(), wstos(wscCharname).c_str(), wstos(HkGetAccountIDByClientID(iClientID)).c_str());
			SaveSettings();
			return true;
		}
		return false;
	}

	bool UserCmd_DropTag(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		if (set_bCharnameTags)
		{
			// Indicate an error if the command does not appear to be formatted correctly 
			// and stop processing but tell FLHook that we processed the command.
			if (wscParam.size()==0)
			{
				PrintUserCmdText(iClientID, L"ERR Invalid parameters");
				PrintUserCmdText(iClientID, usage);
				return true;
			}

			wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
			wstring tag = GetParam(wscParam, ' ', 0);
			wstring pass = GetParam(wscParam, ' ', 1);

			// If this tag is in use then reject the request.
			for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
			{
				if (tag == i->second.tag && pass == i->second.master_password)
				{
					mapTagToPassword.erase(tag);
					SaveSettings();
					PrintUserCmdText(iClientID, L"OK Tag dropped");
					AddLog("NOTICE: Tag %s dropped by %s (%s)", wstos(tag).c_str(), wstos(wscCharname).c_str(), wstos(HkGetAccountIDByClientID(iClientID)).c_str());
					return true;
				}
			}

			PrintUserCmdText(iClientID, L"ERR tag or master password are invalid");
			return true;
		}
		return false;
	}

	// Make tag password
	bool UserCmd_SetTagPass(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		if (set_bCharnameTags)
		{
			// Indicate an error if the command does not appear to be formatted correctly 
			// and stop processing but tell FLHook that we processed the command.
			if (wscParam.size()==0)
			{
				PrintUserCmdText(iClientID, L"ERR Invalid parameters");
				PrintUserCmdText(iClientID, usage);
				return true;
			}

			wstring tag = GetParam(wscParam, ' ', 0);
			wstring master_password = GetParam(wscParam, ' ', 1);
			wstring rename_password = GetParam(wscParam, ' ', 2);

			// If this tag is in use then reject the request.
			for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
			{
				if (tag == i->second.tag && master_password == i->second.master_password)
				{
					i->second.rename_password = rename_password;
					SaveSettings();
					PrintUserCmdText(iClientID, L"OK Created rename password %s for tag %s", rename_password.c_str(), tag.c_str());
					return true;
				}
			}
			PrintUserCmdText(iClientID, L"ERR tag or master password are invalid");
			return true;
		}
		return false;
	}

	struct RENAME
	{
		wstring wscCharname;
		wstring wscNewCharname;

		string scSourceFile;
		string scDestFile;
		string scDestFileTemp;
	};
	std::list<RENAME> pendingRenames;

	struct MOVE
	{
		wstring wscDestinationCharname;
		wstring wscMovingCharname;

		string scSourceFile;
		string scDestFile;
		string scDestFileTemp;
	};
	std::list<MOVE> pendingMoves;

	void Timer()
	{
		// Every 100 seconds expire unused tags and save the tag database
		/* uint curr_time = (uint)time(0);
		if (curr_time % 100)
		{
			for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
			{
				if (i->second.last_access < (curr_time - (3600 * 24 * 30)))
				{
					mapTagToPassword.erase(i);
					break;
				}
			}
			SaveSettings();
		} */

		// Check for pending renames and execute them. We do this on a timer so that the
		// player is definitely not online when we do the rename.
		while (pendingRenames.size())
		{
			RENAME o = pendingRenames.front();
			if (HkGetClientIdFromCharname(o.wscCharname)!=-1)
				return;
			
			pendingRenames.pop_front();

			CAccount *acc = HkGetAccountByCharname(o.wscCharname);
			
			// Delete the character from the existing account, create a new character with the
			// same name in this account and then copy over it with the save character file.
			try
			{
				if (!acc)
					throw "no acc";

				HkLockAccountAccess(acc, true);
				HkUnlockAccountAccess(acc);

				// Move the char file to a temporary one.
				if (!::MoveFileExA(o.scSourceFile.c_str(), o.scDestFileTemp.c_str(),
					MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
					throw "move src to temp failed";
			
				// Decode the char file, update the char name and re-encode it.
				// Add a space to the value so the ini file line looks like "<key> = <value>"
				// otherwise Ioncross Server Operator can't decode the file correctly
				flc_decode(o.scDestFileTemp.c_str(), o.scDestFileTemp.c_str());
				IniWriteW(o.scDestFileTemp, "Player", "Name", o.wscNewCharname);
				if (!set_bDisableCharfileEncryption)
				{
					flc_encode(o.scDestFileTemp.c_str(), o.scDestFileTemp.c_str());
				}

				// Create and delete the character
				HkDeleteCharacter(acc, o.wscCharname);
				HkNewCharacter(acc, o.wscNewCharname);

				// Move files around
				if (!::MoveFileExA(o.scDestFileTemp.c_str(), o.scDestFile.c_str(),
					MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
					throw "move failed";
				if (::PathFileExistsA(o.scSourceFile.c_str()))
					throw "src still exists";
				if (!::PathFileExistsA(o.scDestFile.c_str()))
					throw "dest does not exist";

				// The rename worked. Log it and save the rename time.
				AddLog("NOTICE: User rename %s to %s (%s)",wstos(o.wscCharname).c_str(),wstos(o.wscNewCharname).c_str(), wstos(HkGetAccountID(acc)).c_str());
			}
			catch (char *err)
			{
				AddLog("ERROR: User rename failed (%s) from %s to %s (%s)", err, wstos(o.wscCharname).c_str(),wstos(o.wscNewCharname).c_str(), wstos(HkGetAccountID(acc)).c_str());
			}
		}

		while (pendingMoves.size())
		{
			MOVE o = pendingMoves.front();
			if (HkGetClientIdFromCharname(o.wscDestinationCharname)!=-1)
				return;
			if (HkGetClientIdFromCharname(o.wscMovingCharname)!=-1)
				return;
			
			pendingMoves.pop_front();

			CAccount *acc = HkGetAccountByCharname(o.wscDestinationCharname);
			CAccount *oldAcc = HkGetAccountByCharname(o.wscMovingCharname);

			// Delete the character from the existing account, create a new character with the
			// same name in this account and then copy over it with the save character file.
			try
			{
				HkLockAccountAccess(acc, true);
				HkUnlockAccountAccess(acc);

				HkLockAccountAccess(oldAcc, true);
				HkUnlockAccountAccess(oldAcc);

				// Move the char file to a temporary one.
				if (!::MoveFileExA(o.scSourceFile.c_str(), o.scDestFileTemp.c_str(),
					MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
					throw "move src to temp failed";

				// Create and delete the character
				HkDeleteCharacter(oldAcc, o.wscMovingCharname);
				HkNewCharacter(acc, o.wscMovingCharname);

				// Move files around
				if (!::MoveFileExA(o.scDestFileTemp.c_str(), o.scDestFile.c_str(),
					MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
					throw "move failed";
				if (::PathFileExistsA(o.scSourceFile.c_str()))
					throw "src still exists";
				if (!::PathFileExistsA(o.scDestFile.c_str()))
					throw "dest does not exist";

				// The move worked. Log it.
				AddLog("NOTICE: Character %s moved from %s to %s",
					wstos(o.wscMovingCharname).c_str(),
					wstos(HkGetAccountID(oldAcc)).c_str(),
					wstos(HkGetAccountID(acc)).c_str());

			}
			catch (char *err)
			{
				AddLog("ERROR: Character %s move failed (%s) from %s to %s",
					wstos(o.wscMovingCharname).c_str(), err,
					wstos(HkGetAccountID(oldAcc)).c_str(),
					wstos(HkGetAccountID(acc)).c_str());
			}
		}
	}

	bool UserCmd_RenameMe(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		HK_ERROR err;

		// Don't indicate an error if moving is disabled.
		if (!set_bEnableRenameMe)
			return false;

		// Indicate an error if the command does not appear to be formatted correctly 
		// and stop processing but tell FLHook that we processed the command.
		if (wscParam.size()==0)
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, usage);
			return true;
		}

		uint iBaseID;
		pub::Player::GetBase(iClientID, iBaseID);
		if (!iBaseID)
		{
			PrintUserCmdText(iClientID, L"ERR Not in base");
			return true;
		}

		// If the new name contains spaces then flag this as an
		// error.
		wstring wscNewCharname = Trim(GetParam(wscParam, L' ', 0));
		if (wscNewCharname.find(L" ")!=-1)
		{
			PrintUserCmdText(iClientID, L"ERR Space characters not allowed in name");
			return true;
		}

		if (HkGetAccountByCharname(wscNewCharname))
		{
			PrintUserCmdText(iClientID, L"ERR Name already exists");	
			return true;
		}

		if (wscNewCharname.length() > 23)
		{
			PrintUserCmdText(iClientID, L"ERR Name to long");	
			return true;
		}
		
		if (wscNewCharname.length() < MIN_CHAR_TAG_LEN)
		{
			PrintUserCmdText(iClientID, L"ERR Name to short");	
			return true;
		}

		if (set_bCharnameTags)
		{
			wstring wscPassword = Trim(GetParam(wscParam, L' ', 1));

			for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
			{
				if (wscNewCharname.find(i->first)==0
					&& i->second.rename_password.size() != 0)
				{
					if (!wscPassword.length())
					{
						PrintUserCmdText(iClientID, L"ERR Name starts with an owned tag. Password is required.");	
						return true;
					}
					else if (wscPassword != i->second.master_password
						&& wscPassword != i->second.rename_password)
					{
						PrintUserCmdText(iClientID, L"ERR Name starts with an owned tag. Password is wrong.");	
						return true;
					}
					// Password is valid for owned tag.
					break;
				}
			}
		}

		// Get the character name for this connection.
		wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		// Saving the characters forces an anti-cheat checks and fixes 
		// up a multitude of other problems.
		HkSaveChar(wscCharname);
		if (!HkIsValidClientID(iClientID))
			return true;

		// Read the current number of credits for the player
		// and check that the character has enough cash.
		int iCash = 0;
		if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR "+HkErrGetText(err));
			return true;
		}
		if (set_iRenameCost>0 && iCash<set_iRenameCost)
		{
			PrintUserCmdText(iClientID, L"ERR Insufficient credits");
			return true;
		}

		// Read the last time a rename was done on this character
		wstring wscDir;
		if ((err = HkGetAccountDirName(wscCharname, wscDir))!=HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR "+HkErrGetText(err));
			return true;
		}
		string scRenameFile  = scAcctPath + wstos(wscDir) + "\\" + "rename.ini";
		int lastRenameTime = IniGetI(scRenameFile, "General", wstos(wscCharname), 0);

		// If a rename was done recently by this player then reject the request.
		// I know that time() returns time_t...shouldn't matter for a few years
		// yet.
		if ((lastRenameTime + 300) < (int)time(0))
		{
			if ((lastRenameTime + set_iRenameTimeLimit) > (int)time(0))
			{
				PrintUserCmdText(iClientID, L"ERR Rename time limit");
				return true;
			}
		}

		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		string scAcctPath = string(szDataPath) + "\\Accts\\MultiPlayer\\";

		wstring wscSourceFile;
		if ((err = HkGetCharFileName(wscCharname, wscSourceFile))!=HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR "+HkErrGetText(err));
			return true;
		}
		wstring wscDestFile;
		if ((err = HkGetCharFileName(wscNewCharname, wscDestFile))!=HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR "+HkErrGetText(err));
			return true;
		}

		// Remove cash if we're charging for it.
		if (set_iRenameCost>0)
			HkAddCash(wscCharname, 0-set_iRenameCost);


		RENAME o;
		o.wscCharname = wscCharname;
		o.wscNewCharname = wscNewCharname;
		o.scSourceFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl";
		o.scDestFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscDestFile) + ".fl";
		o.scDestFileTemp = scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl.renaming";
		pendingRenames.push_back(o);
		
		HkKickReason(o.wscCharname, L"Updating character, please wait 10 seconds before reconnecting");
		IniWrite(scRenameFile, "General", wstos(o.wscNewCharname), itos((int)time(0)));
		return true;
	}

	/** Process a set the move char code command */
	bool Rename::UserCmd_SetMoveCharCode(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		// Don't indicate an error if moving is disabled.
		if (!set_bEnableMoveChar)
			return false;

		if (wscParam.size()==0)
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, usage);
			return true;
		}

		wstring wscCharname = (const wchar_t*) Players.GetActiveCharacterName(iClientID);
		string scFile;
		if (!GetUserFilePath(scFile, wscCharname, "-movechar.ini"))
		{
			PrintUserCmdText(iClientID, L"ERR Character does not exist");
			return true;
		}

		wstring wscCode = Trim(GetParam(wscParam, L' ', 0));
		if (wscCode==L"none")
		{
			IniWriteW(scFile, "Settings", "Code", L"");
			PrintUserCmdText(iClientID, L"OK Movechar code cleared");
		}
		else
		{
			IniWriteW(scFile, "Settings", "Code", wscCode);
			PrintUserCmdText(iClientID, L"OK Movechar code set to "+wscCode);
		}
		return true;
	}

	static bool IsBanned(wstring charname)
	{
		char datapath[MAX_PATH];
		GetUserDataPath(datapath);

		wstring dir;
		HkGetAccountDirName(charname, dir);

		string banfile = string(datapath) + "\\Accts\\MultiPlayer\\" + wstos(dir) + "\\banned";

		// Prevent ships from banned accounts from being moved.
		FILE *f = fopen(banfile.c_str(), "r");
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
	bool Rename::UserCmd_MoveChar(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage)
	{
		HK_ERROR err;

		// Don't indicate an error if moving is disabled.
		if (!set_bEnableMoveChar)
			return false;

		// Indicate an error if the command does not appear to be formatted correctly 
		// and stop processing but tell FLHook that we processed the command.
		if (wscParam.size()==0)
		{
			PrintUserCmdText(iClientID, L"ERR Invalid parameters");
			PrintUserCmdText(iClientID, usage);
			return true;
		}

		uint iBaseID;
		pub::Player::GetBase(iClientID, iBaseID);
		if (!iBaseID)
		{
			PrintUserCmdText(iClientID, L"ERR Not in base");
			return true;
		}

		// Get the target account directory.
		string scFile;
		wstring wscMovingCharname = Trim(GetParam(wscParam, L' ', 0));
		if (!GetUserFilePath(scFile, wscMovingCharname, "-movechar.ini"))
		{
			PrintUserCmdText(iClientID, L"ERR Character does not exist");
			return true;
		}
		
		// Check the move char code.
		wstring wscCode = Trim(GetParam(wscParam, L' ', 1));
		wstring wscTargetCode = IniGetWS(scFile, "Settings", "Code", L"");
		if (!wscTargetCode.length() || wscTargetCode!=wscCode)
		{
			PrintUserCmdText(iClientID, L"ERR Move character access denied");
			return true;
		}

		// Prevent ships from banned accounts from being moved.
		if (IsBanned(wscMovingCharname))
		{
			PrintUserCmdText(iClientID, L"ERR not permitted");
			return true;
		}

		wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		// Saving the characters forces an anti-cheat checks and fixes 
		// up a multitude of other problems.
		HkSaveChar(wscCharname);
		HkSaveChar(wscMovingCharname);

		// Read the current number of credits for the player
		// and check that the character has enough cash.
		int iCash = 0;
		if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR "+HkErrGetText(err));
			return true;
		}
		if (set_iMoveCost>0 && iCash<set_iMoveCost)
		{
			PrintUserCmdText(iClientID, L"ERR Insufficient credits");
			return true;
		}

		// Check there is room in this account.
		CAccount *acc=Players.FindAccountFromClientID(iClientID);
		if (acc->iNumberOfCharacters >= 5)
		{
			PrintUserCmdText(iClientID, L"ERR Too many characters in account");
			return true;
		}

		// Copy character file into this account with a temp name.
		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		string scAcctPath = string(szDataPath) + "\\Accts\\MultiPlayer\\";

		wstring wscDir;
		wstring wscSourceDir;
		wstring wscSourceFile;
		if ((err = HkGetAccountDirName(wscCharname, wscDir))!=HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR "+HkErrGetText(err));
			return true;
		}
		if ((err = HkGetAccountDirName(wscMovingCharname, wscSourceDir))!=HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR "+HkErrGetText(err));
			return true;
		}
		if ((err = HkGetCharFileName(wscMovingCharname, wscSourceFile))!=HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR "+HkErrGetText(err));
			return true;
		}

		// Remove cash if we're charging for it.
		if (set_iMoveCost>0)
			HkAddCash(wscCharname, 0-set_iMoveCost);
		HkSaveChar(wscCharname);
		
		// Schedule the move
		MOVE o;
		o.wscDestinationCharname = wscCharname;
		o.wscMovingCharname = wscMovingCharname;
		o.scSourceFile = scAcctPath + wstos(wscSourceDir) + "\\" + wstos(wscSourceFile) + ".fl";
		o.scDestFile = scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl";
		o.scDestFileTemp = scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl.moving";
		pendingMoves.push_back(o);

		// Delete the move code
		::DeleteFileA(scFile.c_str());

		// Kick
		HkKickReason(o.wscDestinationCharname, L"Moving character, please wait 10 seconds before reconnecting");
		HkKickReason(o.wscMovingCharname, L"Moving character, please wait 10 seconds before reconnecting");
		return true;
	}

	/// Set the move char code for all characters in the account
	void Rename::AdminCmd_SetAccMoveCode(CCmds* cmds, const wstring &wscCharname, const wstring &wscCode)
	{
		// Don't indicate an error if moving is disabled.
		if (!set_bEnableMoveChar)
			return;

		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission\n");
			return;
		}

		wstring wscDir;
		if (HkGetAccountDirName(wscCharname, wscDir)!=HKE_OK)
		{
			cmds->Print(L"ERR Charname not found\n");
			return;
		}

		if (wscCode.length()==0)
		{
			cmds->Print(L"ERR Code too small, set to none to clear.\n");
			return;
		}

		// Get the account path.
		char szDataPath[MAX_PATH];
		GetUserDataPath(szDataPath);
		string scPath = string(szDataPath) + "\\Accts\\MultiPlayer\\" + wstos(wscDir) + "\\*.fl";

		// Open the directory iterator.
		WIN32_FIND_DATA FindFileData; 
		HANDLE hFileFind = FindFirstFile(scPath.c_str(), &FindFileData);
		if (hFileFind==INVALID_HANDLE_VALUE)
		{
			cmds->Print(L"ERR Account directory not found\n");
			return;
		}

		// Iterate it
		do
		{
			string scCharfile = FindFileData.cFileName;
			string scMoveCodeFile = string(szDataPath) + "\\Accts\\MultiPlayer\\" + wstos(wscDir) + "\\"
				+ scCharfile.substr(0,scCharfile.size()-3) + "-movechar.ini";
			if (wscCode==L"none")
			{
				IniWriteW(scMoveCodeFile, "Settings", "Code", L"");
				cmds->Print(L"OK Movechar code cleared on "+stows(scCharfile)+L"\n");
			}
			else
			{
				IniWriteW(scMoveCodeFile, "Settings", "Code", wscCode);
				cmds->Print(L"OK Movechar code set to "+wscCode +L" on "+stows(scCharfile)+L"\n");
			}
		}
		while (FindNextFile(hFileFind, &FindFileData));
		FindClose(hFileFind); 

		cmds->Print(L"OK\n");
	}

	/// Set the move char code for all characters in the account
	void AdminCmd_ShowTags(CCmds* cmds)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission\n");
			return;
		}

		uint curr_time = (uint)time(0);
		for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
		{
			int last_access = i->second.last_access;
			int days = (curr_time - last_access) / (24 * 3600);
			cmds->Print(L"tag=%s master_password=%s rename_password=%s last_access=%u days description=%s\n",
				i->second.tag.c_str(), i->second.master_password.c_str(), i->second.rename_password.c_str(), days, i->second.description.c_str());
		}
		cmds->Print(L"OK\n");
	}

	void AdminCmd_AddTag(CCmds* cmds, const wstring &tag, const wstring &password, const wstring &description)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission\n");
			return;
		}

		if (tag.size() < 3)
		{
			cmds->Print(L"ERR Tag too short\n");
			return;
		}

		if (!password.size())
		{
			cmds->Print(L"ERR Password not set\n");
			return;
		}

		if (!description.size())
		{
			cmds->Print(L"ERR Description not set\n");
			return;
		}

		// If this tag is in use then reject the request.
		for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
		{
			if (tag.find(i->second.tag)==0 || i->second.tag.find(tag)==0)
			{
				cmds->Print(L"ERR Tag already exists or conflicts with another tag\n");
				return;
			}
		}

		mapTagToPassword[tag].tag = tag;
		mapTagToPassword[tag].master_password = password;
		mapTagToPassword[tag].rename_password = L"";
		mapTagToPassword[tag].last_access = (uint)time(0);
		mapTagToPassword[tag].description = description;
		cmds->Print(L"Created faction tag %s with master password %s\n", tag.c_str(), password.c_str());
		SaveSettings();
	}

	void AdminCmd_DropTag(CCmds* cmds, const wstring &tag)
	{
		if (!(cmds->rights & RIGHT_SUPERADMIN))
		{
			cmds->Print(L"ERR No permission\n");
			return;
		}

		// If this tag is in use then reject the request.
		for (std::map<wstring, TAG_DATA>::iterator i = mapTagToPassword.begin(); i != mapTagToPassword.end(); ++i)
		{
			if (tag == i->second.tag)
			{
				mapTagToPassword.erase(tag);
				SaveSettings();
				cmds->Print(L"OK Tag dropped\n");
				return;
			}
		}

		cmds->Print(L"ERR tag is invalid\n");
		return;
	}
}