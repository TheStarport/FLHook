// Rename plugin - Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

void LoadSettings() {

    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\rename.cfg";

    set_bEnableRenameMe = IniGetB(scPluginCfgFile, "Rename", "EnableRename", false);
    set_bEnableMoveChar =
        IniGetB(scPluginCfgFile, "Rename", "EnableMoveChar", false);
    set_iRenameCost = IniGetI(scPluginCfgFile, "Rename", "RenameCost", 5000000);
    set_iRenameTimeLimit =
        IniGetI(scPluginCfgFile, "Rename", "RenameTimeLimit", 3600);
    set_iMoveCost = IniGetI(scPluginCfgFile, "Rename", "MoveCost", 5000000);
    set_bCharnameTags =
        IniGetB(scPluginCfgFile, "Rename", "CharnameTag", false);
    set_bAsciiCharnameOnly =
        IniGetB(scPluginCfgFile, "Rename", "AsciiCharnameOnly", true);
    set_iMakeTagCost =
        IniGetI(scPluginCfgFile, "Rename", "MakeTagCost", 50000000);

    char szDataPath[MAX_PATH];
    GetUserDataPath(szDataPath);
    std::string scPath =
        std::string(szDataPath) + "\\Accts\\MultiPlayer\\tags.ini";

    INI_Reader ini;
    if (ini.open(scPath.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("faction")) {
                std::wstring tag;
                while (ini.read_value()) {
                    if (ini.is_value("tag")) {
                        ini_get_wstring(ini, tag);
                        mapTagToPassword[tag].tag = tag;
                    } else if (ini.is_value("master_password")) {
                        std::wstring pass;
                        ini_get_wstring(ini, pass);
                        mapTagToPassword[tag].master_password = pass;
                    } else if (ini.is_value("rename_password")) {
                        std::wstring pass;
                        ini_get_wstring(ini, pass);
                        mapTagToPassword[tag].rename_password = pass;
                    } else if (ini.is_value("last_access")) {
                        mapTagToPassword[tag].last_access =
                            ini.get_value_int(0);
                    } else if (ini.is_value("description")) {
                        std::wstring description;
                        ini_get_wstring(ini, description);
                        mapTagToPassword[tag].description = description;
                    }
                }
            }
        }
        ini.close();
    }
}

void SaveSettings() {
    char szDataPath[MAX_PATH];
    GetUserDataPath(szDataPath);
    std::string scPath =
        std::string(szDataPath) + "\\Accts\\MultiPlayer\\tags.ini";

    FILE *file;
    fopen_s(&file, scPath.c_str(), "w");
    if (file) {
        for (std::map<std::wstring, TAG_DATA>::iterator i =
                 mapTagToPassword.begin();
             i != mapTagToPassword.end(); ++i) {
            fprintf(file, "[faction]\n");
            ini_write_wstring(file, "tag", i->second.tag);
            ini_write_wstring(file, "master_password",
                              i->second.master_password);
            ini_write_wstring(file, "rename_password",
                              i->second.rename_password);
            ini_write_wstring(file, "description", i->second.description);
            fprintf(file, "last_access = %u\n", i->second.last_access);
        }
        fclose(file);
    }
}

bool CreateNewCharacter(struct SCreateCharacterInfo const &si,
                        unsigned int iClientID) {
    if (set_bCharnameTags) {
        // If this ship name starts with a restricted tag then the ship may only
        // be created using rename and the faction password
        std::wstring wscCharname(si.wszCharname);
        for (std::map<std::wstring, TAG_DATA>::iterator i =
                 mapTagToPassword.begin();
             i != mapTagToPassword.end(); ++i) {
            if (wscCharname.find(i->second.tag) == 0 &&
                i->second.rename_password.size() != 0) {
                Server.CharacterInfoReq(iClientID, true);
                return true;
            }
        }

        // If this ship name is too short, reject the request
        if (wscCharname.size() < MIN_CHAR_TAG_LEN + 1) {
            Server.CharacterInfoReq(iClientID, true);
            return true;
        }
    }

    if (set_bAsciiCharnameOnly) {
        std::wstring wscCharname(si.wszCharname);
        for (uint i = 0; i < wscCharname.size(); i++) {
            wchar_t ch = wscCharname[i];
            if (ch & 0xFF80)
                return true;
        }
    }

    return false;
}

// Update the tag list when a character is selected update the tag list to
// indicate that this tag is in use. If a tag is not used after 60 days, remove
// it.
void CharacterSelect_AFTER(struct CHARACTER_ID const &charId,
                           unsigned int iClientID) {
    if (set_bCharnameTags) {
        std::wstring wscCharname =
            (const wchar_t *)Players.GetActiveCharacterName(iClientID);
        for (std::map<std::wstring, TAG_DATA>::iterator i =
                 mapTagToPassword.begin();
             i != mapTagToPassword.end(); ++i) {
            if (wscCharname.find(i->second.tag) == 0) {
                i->second.last_access = (uint)time(0);
            }
        }
    }
}

void UserCmd_MakeTag(uint iClientID, const std::wstring &wscParam) {
    if (set_bCharnameTags) {
        std::wstring usage = L"Usage: /maketag <tag> <master password> <description>";
        // Indicate an error if the command does not appear to be formatted
        // correctly and stop processing but tell FLHook that we processed the
        // command.
        if (wscParam.size() == 0) {
            PrintUserCmdText(iClientID, L"ERR Invalid parameters");
            PrintUserCmdText(iClientID, usage);
            return;
        }

        uint iBaseID;
        pub::Player::GetBase(iClientID, iBaseID);
        if (!iBaseID) {
            PrintUserCmdText(iClientID, L"ERR Not in base");
            return;
        }

        std::wstring tag = GetParam(wscParam, ' ', 0);
        std::wstring pass = GetParam(wscParam, ' ', 1);
        std::wstring description = GetParamToEnd(wscParam, ' ', 2);

        if (tag.size() < MIN_CHAR_TAG_LEN) {
            PrintUserCmdText(iClientID, L"ERR Tag too short");
            PrintUserCmdText(iClientID, usage);
            return;
        }

        if (!pass.size()) {
            PrintUserCmdText(iClientID, L"ERR Password not set");
            PrintUserCmdText(iClientID, usage);
            return;
        }

        if (!description.size()) {
            PrintUserCmdText(iClientID, L"ERR Description not set");
            PrintUserCmdText(iClientID, usage);
            return;
        }

        // If this tag is in use then reject the request.
        for (std::map<std::wstring, TAG_DATA>::iterator i =
                 mapTagToPassword.begin();
             i != mapTagToPassword.end(); ++i) {
            if (tag.find(i->second.tag) == 0 || i->second.tag.find(tag) == 0) {
                PrintUserCmdText(
                    iClientID,
                    L"ERR Tag already exists or conflicts with existing tag");
                return;
            }
        }

        // Save character and exit if kicked on save.
        std::wstring wscCharname =
            (const wchar_t *)Players.GetActiveCharacterName(iClientID);
        HkSaveChar(wscCharname);
        if (HkGetClientIdFromCharname(wscCharname) == -1)
            return;

        int iCash;
        HK_ERROR err;
        if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK) {
            PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
            return;
        }
        if (set_iMakeTagCost > 0 && iCash < set_iMakeTagCost) {
            PrintUserCmdText(iClientID, L"ERR Insufficient credits");
            return;
        }

        HkAddCash(wscCharname, 0 - set_iMakeTagCost);

        // TODO: Try to check if any player is using this tag
        mapTagToPassword[tag].tag = tag;
        mapTagToPassword[tag].master_password = pass;
        mapTagToPassword[tag].rename_password = L"";
        mapTagToPassword[tag].last_access = (uint)time(0);
        mapTagToPassword[tag].description = description;

        PrintUserCmdText(iClientID,
                         L"Created faction tag %s with master password %s",
                         tag.c_str(), pass.c_str());
        AddLog("NOTICE: Tag %s created by %s (%s)", wstos(tag).c_str(),
               wstos(wscCharname).c_str(),
               wstos(HkGetAccountIDByClientID(iClientID)).c_str());
        SaveSettings();
    }
}

void UserCmd_DropTag(uint iClientID, const std::wstring &wscParam) {
    if (set_bCharnameTags) {
        // Indicate an error if the command does not appear to be formatted
        // correctly and stop processing but tell FLHook that we processed the
        // command.
        if (wscParam.size() == 0) {
            PrintUserCmdText(iClientID, L"ERR Invalid parameters");
            PrintUserCmdText(iClientID, L"Usage: /droptag <tag> <master password>");
            return;
        }

        std::wstring wscCharname =
            (const wchar_t *)Players.GetActiveCharacterName(iClientID);
        std::wstring tag = GetParam(wscParam, ' ', 0);
        std::wstring pass = GetParam(wscParam, ' ', 1);

        // If this tag is in use then reject the request.
        for (std::map<std::wstring, TAG_DATA>::iterator i =
                 mapTagToPassword.begin();
             i != mapTagToPassword.end(); ++i) {
            if (tag == i->second.tag && pass == i->second.master_password) {
                mapTagToPassword.erase(tag);
                SaveSettings();
                PrintUserCmdText(iClientID, L"OK Tag dropped");
                AddLog("NOTICE: Tag %s dropped by %s (%s)", wstos(tag).c_str(),
                       wstos(wscCharname).c_str(),
                       wstos(HkGetAccountIDByClientID(iClientID)).c_str());
                return;
            }
        }

        PrintUserCmdText(iClientID, L"ERR tag or master password are invalid");
    }
}

// Make tag password
void UserCmd_SetTagPass(uint iClientID, const std::wstring &wscParam) {
    if (set_bCharnameTags) {
        // Indicate an error if the command does not appear to be formatted
        // correctly and stop processing but tell FLHook that we processed the
        // command.
        if (wscParam.size() == 0) {
            PrintUserCmdText(iClientID, L"ERR Invalid parameters");
            PrintUserCmdText(iClientID, L"Usage: /settagpass <tag> <master password> <rename password>");
            return;
        }

        std::wstring tag = GetParam(wscParam, ' ', 0);
        std::wstring master_password = GetParam(wscParam, ' ', 1);
        std::wstring rename_password = GetParam(wscParam, ' ', 2);

        // If this tag is in use then reject the request.
        for (std::map<std::wstring, TAG_DATA>::iterator i =
                 mapTagToPassword.begin();
             i != mapTagToPassword.end(); ++i) {
            if (tag == i->second.tag &&
                master_password == i->second.master_password) {
                i->second.rename_password = rename_password;
                SaveSettings();
                PrintUserCmdText(iClientID,
                                 L"OK Created rename password %s for tag %s",
                                 rename_password.c_str(), tag.c_str());
                return;
            }
        }
        PrintUserCmdText(iClientID, L"ERR tag or master password are invalid");
    }
}


void Timer() {
    // Check for pending renames and execute them. We do this on a timer so that
    // the player is definitely not online when we do the rename.
    while (pendingRenames.size()) {
        RENAME o = pendingRenames.front();
        if (HkGetClientIdFromCharname(o.wscCharname) != -1)
            return;

        pendingRenames.pop_front();

        CAccount *acc = HkGetAccountByCharname(o.wscCharname);

        // Delete the character from the existing account, create a new
        // character with the same name in this account and then copy over it
        // with the save character file.
        try {
            if (!acc)
                throw "no acc";

            HkLockAccountAccess(acc, true);
            HkUnlockAccountAccess(acc);

            // Move the char file to a temporary one.
            if (!::MoveFileExA(o.scSourceFile.c_str(), o.scDestFileTemp.c_str(),
                               MOVEFILE_REPLACE_EXISTING |
                                   MOVEFILE_WRITE_THROUGH))
                throw "move src to temp failed";

            // Decode the char file, update the char name and re-encode it.
            // Add a space to the value so the ini file line looks like "<key> =
            // <value>" otherwise Ioncross Server Operator can't decode the file
            // correctly
            flc_decode(o.scDestFileTemp.c_str(), o.scDestFileTemp.c_str());
            IniWriteW(o.scDestFileTemp, "Player", "Name", o.wscNewCharname);
            if (!set_bDisableCharfileEncryption) {
                flc_encode(o.scDestFileTemp.c_str(), o.scDestFileTemp.c_str());
            }

            // Create and delete the character
            HkDeleteCharacter(acc, o.wscCharname);
            HkNewCharacter(acc, o.wscNewCharname);

            // Move files around
            if (!::MoveFileExA(o.scDestFileTemp.c_str(), o.scDestFile.c_str(),
                               MOVEFILE_REPLACE_EXISTING |
                                   MOVEFILE_WRITE_THROUGH))
                throw "move failed";
            if (::PathFileExistsA(o.scSourceFile.c_str()))
                throw "src still exists";
            if (!::PathFileExistsA(o.scDestFile.c_str()))
                throw "dest does not exist";

            // The rename worked. Log it and save the rename time.
            AddLog("NOTICE: User rename %s to %s (%s)",
                   wstos(o.wscCharname).c_str(),
                   wstos(o.wscNewCharname).c_str(),
                   wstos(HkGetAccountID(acc)).c_str());
        } catch (char *err) {
            AddLog("ERROR: User rename failed (%s) from %s to %s (%s)", err,
                   wstos(o.wscCharname).c_str(),
                   wstos(o.wscNewCharname).c_str(),
                   wstos(HkGetAccountID(acc)).c_str());
        }
    }

    while (pendingMoves.size()) {
        MOVE o = pendingMoves.front();
        if (HkGetClientIdFromCharname(o.wscDestinationCharname) != -1)
            return;
        if (HkGetClientIdFromCharname(o.wscMovingCharname) != -1)
            return;

        pendingMoves.pop_front();

        CAccount *acc = HkGetAccountByCharname(o.wscDestinationCharname);
        CAccount *oldAcc = HkGetAccountByCharname(o.wscMovingCharname);

        // Delete the character from the existing account, create a new
        // character with the same name in this account and then copy over it
        // with the save character file.
        try {
            HkLockAccountAccess(acc, true);
            HkUnlockAccountAccess(acc);

            HkLockAccountAccess(oldAcc, true);
            HkUnlockAccountAccess(oldAcc);

            // Move the char file to a temporary one.
            if (!::MoveFileExA(o.scSourceFile.c_str(), o.scDestFileTemp.c_str(),
                               MOVEFILE_REPLACE_EXISTING |
                                   MOVEFILE_WRITE_THROUGH))
                throw "move src to temp failed";

            // Create and delete the character
            HkDeleteCharacter(oldAcc, o.wscMovingCharname);
            HkNewCharacter(acc, o.wscMovingCharname);

            // Move files around
            if (!::MoveFileExA(o.scDestFileTemp.c_str(), o.scDestFile.c_str(),
                               MOVEFILE_REPLACE_EXISTING |
                                   MOVEFILE_WRITE_THROUGH))
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

        } catch (char *err) {
            AddLog("ERROR: Character %s move failed (%s) from %s to %s",
                   wstos(o.wscMovingCharname).c_str(), err,
                   wstos(HkGetAccountID(oldAcc)).c_str(),
                   wstos(HkGetAccountID(acc)).c_str());
        }
    }
}

void UserCmd_RenameMe(uint iClientID, const std::wstring &wscParam) {
    HK_ERROR err;

    // Don't indicate an error if moving is disabled.
    if (!set_bEnableRenameMe)
        return;

    // Indicate an error if the command does not appear to be formatted
    // correctly and stop processing but tell FLHook that we processed the
    // command.
    if (wscParam.size() == 0) {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        PrintUserCmdText(iClientID, L"Usage: /renameme <charname> [password]");
        return;
    }

    uint iBaseID;
    pub::Player::GetBase(iClientID, iBaseID);
    if (!iBaseID) {
        PrintUserCmdText(iClientID, L"ERR Not in base");
        return;
    }

    // If the new name contains spaces then flag this as an
    // error.
    std::wstring wscNewCharname = Trim(GetParam(wscParam, L' ', 0));
    if (wscNewCharname.find(L" ") != -1) {
        PrintUserCmdText(iClientID,
                         L"ERR Space characters not allowed in name");
        return;
    }

    if (HkGetAccountByCharname(wscNewCharname)) {
        PrintUserCmdText(iClientID, L"ERR Name already exists");
        return;
    }

    if (wscNewCharname.length() > 23) {
        PrintUserCmdText(iClientID, L"ERR Name to long");
        return;
    }

    if (wscNewCharname.length() < MIN_CHAR_TAG_LEN) {
        PrintUserCmdText(iClientID, L"ERR Name to short");
        return;
    }

    if (set_bCharnameTags) {
        std::wstring wscPassword = Trim(GetParam(wscParam, L' ', 1));

        for (std::map<std::wstring, TAG_DATA>::iterator i =
                 mapTagToPassword.begin();
             i != mapTagToPassword.end(); ++i) {
            if (wscNewCharname.find(i->first) == 0 &&
                i->second.rename_password.size() != 0) {
                if (!wscPassword.length()) {
                    PrintUserCmdText(iClientID,
                                     L"ERR Name starts with an owned tag. "
                                     L"Password is required.");
                    return;
                } else if (wscPassword != i->second.master_password &&
                           wscPassword != i->second.rename_password) {
                    PrintUserCmdText(iClientID,
                                     L"ERR Name starts with an owned tag. "
                                     L"Password is wrong.");
                    return;
                }
                // Password is valid for owned tag.
                break;
            }
        }
    }

    // Get the character name for this connection.
    std::wstring wscCharname =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);

    // Saving the characters forces an anti-cheat checks and fixes
    // up a multitude of other problems.
    HkSaveChar(wscCharname);
    if (!HkIsValidClientID(iClientID))
        return;

    // Read the current number of credits for the player
    // and check that the character has enough cash.
    int iCash = 0;
    if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
        return;
    }
    if (set_iRenameCost > 0 && iCash < set_iRenameCost) {
        PrintUserCmdText(iClientID, L"ERR Insufficient credits");
        return;
    }

    // Read the last time a rename was done on this character
    std::wstring wscDir;
    if ((err = HkGetAccountDirName(wscCharname, wscDir)) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
        return;
    }
    std::string scRenameFile = scAcctPath + wstos(wscDir) + "\\" + "rename.ini";
    int lastRenameTime =
        IniGetI(scRenameFile, "General", wstos(wscCharname), 0);

    // If a rename was done recently by this player then reject the request.
    // I know that time() returns time_t...shouldn't matter for a few years
    // yet.
    if ((lastRenameTime + 300) < (int)time(0)) {
        if ((lastRenameTime + set_iRenameTimeLimit) > (int)time(0)) {
            PrintUserCmdText(iClientID, L"ERR Rename time limit");
            return;
        }
    }

    char szDataPath[MAX_PATH];
    GetUserDataPath(szDataPath);
    std::string scAcctPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\";

    std::wstring wscSourceFile;
    if ((err = HkGetCharFileName(wscCharname, wscSourceFile)) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
        return;
    }
    std::wstring wscDestFile;
    if ((err = HkGetCharFileName(wscNewCharname, wscDestFile)) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
        return;
    }

    // Remove cash if we're charging for it.
    if (set_iRenameCost > 0)
        HkAddCash(wscCharname, 0 - set_iRenameCost);

    RENAME o;
    o.wscCharname = wscCharname;
    o.wscNewCharname = wscNewCharname;
    o.scSourceFile =
        scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl";
    o.scDestFile =
        scAcctPath + wstos(wscDir) + "\\" + wstos(wscDestFile) + ".fl";
    o.scDestFileTemp = scAcctPath + wstos(wscDir) + "\\" +
                       wstos(wscSourceFile) + ".fl.renaming";
    pendingRenames.push_back(o);

    HkKickReason(
        o.wscCharname,
        L"Updating character, please wait 10 seconds before reconnecting");
    IniWrite(scRenameFile, "General", wstos(o.wscNewCharname),
             std::to_string((int)time(0)));
}

/** Process a set the move char code command */
void UserCmd_SetMoveCharCode(uint iClientID, const std::wstring &wscParam) {
    // Don't indicate an error if moving is disabled.
    if (!set_bEnableMoveChar)
        return;

    if (wscParam.size() == 0) {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        PrintUserCmdText(iClientID, L"Usage: /set movecharcode <code>");
        return;
    }

    std::wstring wscCharname =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);
    std::string scFile = GetUserFilePath(wscCharname, "-movechar.ini");
    if (scFile.empty()) {
        PrintUserCmdText(iClientID, L"ERR Character does not exist");
        return;
    }

    std::wstring wscCode = Trim(GetParam(wscParam, L' ', 0));
    if (wscCode == L"none") {
        IniWriteW(scFile, "Settings", "Code", L"");
        PrintUserCmdText(iClientID, L"OK Movechar code cleared");
    } else {
        IniWriteW(scFile, "Settings", "Code", wscCode);
        PrintUserCmdText(iClientID, L"OK Movechar code set to " + wscCode);
    }
    return;
}

static bool IsBanned(std::wstring charname) {
    char datapath[MAX_PATH];
    GetUserDataPath(datapath);

    std::wstring dir;
    HkGetAccountDirName(charname, dir);

    std::string banfile = std::string(datapath) + "\\Accts\\MultiPlayer\\" +
                          wstos(dir) + "\\banned";

    // Prevent ships from banned accounts from being moved.
    FILE *f;
    fopen_s(&f, banfile.c_str(), "r");
    if (f) {
        fclose(f);
        return true;
    }
    return false;
}

/**
 Move a character from a remote account into this one.
*/
void UserCmd_MoveChar(uint iClientID, const std::wstring &wscParam) {
    HK_ERROR err;

    // Don't indicate an error if moving is disabled.
    if (!set_bEnableMoveChar)
        return;

    // Indicate an error if the command does not appear to be formatted
    // correctly and stop processing but tell FLHook that we processed the
    // command.
    if (wscParam.size() == 0) {
        PrintUserCmdText(iClientID, L"ERR Invalid parameters");
        PrintUserCmdText(iClientID, L"Usage: /movechar <charname> <code>");
        return;
    }

    uint iBaseID;
    pub::Player::GetBase(iClientID, iBaseID);
    if (!iBaseID) {
        PrintUserCmdText(iClientID, L"ERR Not in base");
        return;
    }

    // Get the target account directory.
    std::wstring wscMovingCharname = Trim(GetParam(wscParam, L' ', 0));
    std::string scFile = GetUserFilePath(wscMovingCharname, "-movechar.ini");
    if (scFile.empty()) {
        PrintUserCmdText(iClientID, L"ERR Character does not exist");
        return;
    }

    // Check the move char code.
    std::wstring wscCode = Trim(GetParam(wscParam, L' ', 1));
    std::wstring wscTargetCode = IniGetWS(scFile, "Settings", "Code", L"");
    if (!wscTargetCode.length() || wscTargetCode != wscCode) {
        PrintUserCmdText(iClientID, L"ERR Move character access denied");
        return;
    }

    // Prevent ships from banned accounts from being moved.
    if (IsBanned(wscMovingCharname)) {
        PrintUserCmdText(iClientID, L"ERR not permitted");
        return;
    }

    std::wstring wscCharname =
        (const wchar_t *)Players.GetActiveCharacterName(iClientID);

    // Saving the characters forces an anti-cheat checks and fixes
    // up a multitude of other problems.
    HkSaveChar(wscCharname);
    HkSaveChar(wscMovingCharname);

    // Read the current number of credits for the player
    // and check that the character has enough cash.
    int iCash = 0;
    if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
        return;
    }
    if (set_iMoveCost > 0 && iCash < set_iMoveCost) {
        PrintUserCmdText(iClientID, L"ERR Insufficient credits");
        return;
    }

    // Check there is room in this account.
    CAccount *acc = Players.FindAccountFromClientID(iClientID);
    if (acc->iNumberOfCharacters >= 5) {
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
    if ((err = HkGetAccountDirName(wscCharname, wscDir)) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
        return;
    }
    if ((err = HkGetAccountDirName(wscMovingCharname, wscSourceDir)) !=
        HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
        return;
    }
    if ((err = HkGetCharFileName(wscMovingCharname, wscSourceFile)) != HKE_OK) {
        PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
        return;
    }

    // Remove cash if we're charging for it.
    if (set_iMoveCost > 0)
        HkAddCash(wscCharname, 0 - set_iMoveCost);
    HkSaveChar(wscCharname);

    // Schedule the move
    MOVE o;
    o.wscDestinationCharname = wscCharname;
    o.wscMovingCharname = wscMovingCharname;
    o.scSourceFile =
        scAcctPath + wstos(wscSourceDir) + "\\" + wstos(wscSourceFile) + ".fl";
    o.scDestFile =
        scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl";
    o.scDestFileTemp =
        scAcctPath + wstos(wscDir) + "\\" + wstos(wscSourceFile) + ".fl.moving";
    pendingMoves.push_back(o);

    // Delete the move code
    ::DeleteFileA(scFile.c_str());

    // Kick
    HkKickReason(
        o.wscDestinationCharname,
        L"Moving character, please wait 10 seconds before reconnecting");
    HkKickReason(
        o.wscMovingCharname,
        L"Moving character, please wait 10 seconds before reconnecting");
}

/// Set the move char code for all characters in the account
void AdminCmd_SetAccMoveCode(CCmds *cmds,
                                     const std::wstring &wscCharname,
                                     const std::wstring &wscCode) {
    // Don't indicate an error if moving is disabled.
    if (!set_bEnableMoveChar)
        return;

    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    std::wstring wscDir;
    if (HkGetAccountDirName(wscCharname, wscDir) != HKE_OK) {
        cmds->Print(L"ERR Charname not found\n");
        return;
    }

    if (wscCode.length() == 0) {
        cmds->Print(L"ERR Code too small, set to none to clear.\n");
        return;
    }

    // Get the account path.
    char szDataPath[MAX_PATH];
    GetUserDataPath(szDataPath);
    std::string scPath = std::string(szDataPath) + "\\Accts\\MultiPlayer\\" +
                         wstos(wscDir) + "\\*.fl";

    // Open the directory iterator.
    WIN32_FIND_DATA FindFileData;
    HANDLE hFileFind = FindFirstFile(scPath.c_str(), &FindFileData);
    if (hFileFind == INVALID_HANDLE_VALUE) {
        cmds->Print(L"ERR Account directory not found\n");
        return;
    }

    // Iterate it
    do {
        std::string scCharfile = FindFileData.cFileName;
        std::string scMoveCodeFile =
            std::string(szDataPath) + "\\Accts\\MultiPlayer\\" + wstos(wscDir) +
            "\\" + scCharfile.substr(0, scCharfile.size() - 3) +
            "-movechar.ini";
        if (wscCode == L"none") {
            IniWriteW(scMoveCodeFile, "Settings", "Code", L"");
            cmds->Print(L"OK Movechar code cleared on " + stows(scCharfile) +
                        L"\n");
        } else {
            IniWriteW(scMoveCodeFile, "Settings", "Code", wscCode);
            cmds->Print(L"OK Movechar code set to " + wscCode + L" on " +
                        stows(scCharfile) + L"\n");
        }
    } while (FindNextFile(hFileFind, &FindFileData));
    FindClose(hFileFind);

    cmds->Print(L"OK\n");
}

/// Set the move char code for all characters in the account
void AdminCmd_ShowTags(CCmds *cmds) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    uint curr_time = (uint)time(0);
    for (std::map<std::wstring, TAG_DATA>::iterator i =
             mapTagToPassword.begin();
         i != mapTagToPassword.end(); ++i) {
        int last_access = i->second.last_access;
        int days = (curr_time - last_access) / (24 * 3600);
        cmds->Print(
            L"tag=%s master_password=%s rename_password=%s last_access=%u "
            L"days description=%s\n",
            i->second.tag.c_str(), i->second.master_password.c_str(),
            i->second.rename_password.c_str(), days,
            i->second.description.c_str());
    }
    cmds->Print(L"OK\n");
}

void AdminCmd_AddTag(CCmds *cmds, const std::wstring &tag,
                     const std::wstring &password,
                     const std::wstring &description) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    if (tag.size() < 3) {
        cmds->Print(L"ERR Tag too short\n");
        return;
    }

    if (!password.size()) {
        cmds->Print(L"ERR Password not set\n");
        return;
    }

    if (!description.size()) {
        cmds->Print(L"ERR Description not set\n");
        return;
    }

    // If this tag is in use then reject the request.
    for (std::map<std::wstring, TAG_DATA>::iterator i =
             mapTagToPassword.begin();
         i != mapTagToPassword.end(); ++i) {
        if (tag.find(i->second.tag) == 0 || i->second.tag.find(tag) == 0) {
            cmds->Print(
                L"ERR Tag already exists or conflicts with another tag\n");
            return;
        }
    }

    mapTagToPassword[tag].tag = tag;
    mapTagToPassword[tag].master_password = password;
    mapTagToPassword[tag].rename_password = L"";
    mapTagToPassword[tag].last_access = (uint)time(0);
    mapTagToPassword[tag].description = description;
    cmds->Print(L"Created faction tag %s with master password %s\n",
                tag.c_str(), password.c_str());
    SaveSettings();
}

void AdminCmd_DropTag(CCmds *cmds, const std::wstring &tag) {
    if (!(cmds->rights & RIGHT_SUPERADMIN)) {
        cmds->Print(L"ERR No permission\n");
        return;
    }

    // If this tag is in use then reject the request.
    for (std::map<std::wstring, TAG_DATA>::iterator i =
             mapTagToPassword.begin();
         i != mapTagToPassword.end(); ++i) {
        if (tag == i->second.tag) {
            mapTagToPassword.erase(tag);
            SaveSettings();
            cmds->Print(L"OK Tag dropped\n");
            return;
        }
    }

    cmds->Print(L"ERR tag is invalid\n");
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Client command processing
USERCMD UserCmds[] = {
    {L"/maketag", UserCmd_MakeTag},
    {L"/droptag", UserCmd_DropTag},
    {L"/settagpass", UserCmd_SetTagPass},
    {L"/renameme", UserCmd_RenameMe},
    {L"/movechar", UserCmd_MoveChar},
    {L"/set movecharcode", UserCmd_SetMoveCharCode},
};

// Process user input
bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd) {
    DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

// Hook on /help
EXPORT void UserCmd_Help(uint iClientID, const std::wstring &wscParam) {

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Admin help command callback */
void CmdHelp(CCmds *classptr) {
classptr->Print(L"setaccmovecode <charname> <code>\n");
    classptr->Print(L"showtags\n");
    classptr->Print(L"addtag <tag> <password>\n");
    classptr->Print(L"droptag <tag> <password>\n");
}

bool ExecuteCommandString(CCmds *cmds, const std::wstring &wscCmd) {
    if (IS_CMD("setaccmovecode")) {
        returncode = ReturnCode::SkipAll;
        AdminCmd_SetAccMoveCode(cmds, cmds->ArgCharname(1),
                                        cmds->ArgStr(2));
        return true;
    } else if (IS_CMD("showtags")) {
        returncode = ReturnCode::SkipAll;
        AdminCmd_ShowTags(cmds);
        return true;
    } else if (IS_CMD("addtag")) {
        returncode = ReturnCode::SkipAll;
        AdminCmd_AddTag(cmds, cmds->ArgStr(1), cmds->ArgStr(2),
                                cmds->ArgStrToEnd(3));
        return true;
    } else if (IS_CMD("droptag")) {
        returncode = ReturnCode::SkipAll;
        AdminCmd_DropTag(cmds, cmds->ArgStr(1));
        return true;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo *pi) {
    pi->name("Rename");
    pi->shortName("rename");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect_AFTER, HookStep::After);
    pi->emplaceHook(HookedCall::IServerImpl__CreateNewCharacter, &CreateNewCharacter);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
    pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &Timer);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
    pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
    pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
    pi->emplaceHook(HookedCall::FLHook__AdminCommand__Help, &UserCmd_Help);
}
