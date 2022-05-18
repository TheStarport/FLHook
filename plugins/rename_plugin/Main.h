#pragma once

#include "Shlwapi.h"

#include <FLHook.h>
#include <plugin.h>

ReturnCode returncode = ReturnCode::Default;

// Enable Rename
bool set_bEnableRenameMe = false;

// Enable Moving of Characters
bool set_bEnableMoveChar = false;

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
	std::wstring tag;
	std::wstring master_password;
	std::wstring rename_password;
	uint last_access;
	std::wstring description;
};

std::map<std::wstring, TAG_DATA> mapTagToPassword;

struct RENAME
{
	std::wstring wscCharname;
	std::wstring wscNewCharname;

	std::string scSourceFile;
	std::string scDestFile;
	std::string scDestFileTemp;
};
std::list<RENAME> pendingRenames;

struct MOVE
{
	std::wstring wscDestinationCharname;
	std::wstring wscMovingCharname;

	std::string scSourceFile;
	std::string scDestFile;
	std::string scDestFileTemp;
};
std::list<MOVE> pendingMoves;

#define MIN_CHAR_TAG_LEN 3
