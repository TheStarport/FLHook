// Money Manager Plugin by Cannon
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include <FLHook.h>
#include <plugin.h>

ReturnCode returncode = ReturnCode::Default;

// The minimum transfer amount.
static int set_iMinTransfer = 0;

// Transfers are not allowed to/from chars in this system.
static uint set_iBlockedSystem = 0;

// Enable in dock cash cheat detection
static bool set_bCheatDetection = false;

// Prohibit transfers if the character has not been online for at least this
// time
static int set_iMinTime = 0;

/**
It checks character's givecash history and prints out any received cash
messages. Also fixes the money fix list, we can do this because this plugin is
called before the money fix list is accessed.
*/
static void CheckTransferLog(uint iClientID)
{
	std::wstring wscCharname = ToLower((const wchar_t*)Players.GetActiveCharacterName(iClientID));

	std::string logFile = GetUserFilePath(wscCharname, "-givecashlog.txt");
	if (logFile.empty())
		return;

	FILE* f;
	fopen_s(&f, logFile.c_str(), "r");
	if (!f)
		return;

	// A fixed length buffer be a little dangerous, but char name lengths are
	// fixed to about 30ish characters so this should be okay, and in the worst
	// case we will catch the exception.
	try
	{
		char buf[1000];
		while (fgets(buf, 1000, f) != NULL)
		{
			std::string scValue = buf;
			std::wstring msg = L"";
			uint lHiByte;
			uint lLoByte;
			while (scValue.length() > 3 && sscanf_s(scValue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2)
			{
				scValue = scValue.substr(4);
				msg.append(1, (wchar_t)((lHiByte << 8) | lLoByte));
			}
			PrintUserCmdText(iClientID, L"%s", msg.c_str());
		}
	}
	catch (...)
	{
	}
	// Always close the file and remove the givecash log.
	fclose(f);
	remove(logFile.c_str());
}

/**
Save a transfer to disk so that we can inform the receiving character
when they log in. The log is recorded in ascii hex to support wide
char sets.
*/
static void LogTransfer(std::wstring wscToCharname, std::wstring msg)
{
	std::string logFile = GetUserFilePath(wscToCharname, "-givecashlog.txt");
	if (logFile.empty())
		return;
	FILE* f;
	fopen_s(&f, logFile.c_str(), "at");
	if (!f)
		return;

	try
	{
		for (uint i = 0; (i < msg.length()); i++)
		{
			char cHiByte = msg[i] >> 8;
			char cLoByte = msg[i] & 0xFF;
			fprintf(f, "%02X%02X", ((uint)cHiByte) & 0xFF, ((uint)cLoByte) & 0xFF);
		}
		fprintf(f, "\n");
	}
	catch (...)
	{
	}
	fclose(f);
	return;
}

/** Return return if this char is in the blocked system */
static bool InBlockedSystem(const std::wstring& wscCharname)
{
	// An optimisation if we have no blocked systems.
	if (set_iBlockedSystem == 0)
		return false;

	// If the char is logged in we can check in memory.
	uint iClientID = HkGetClientIdFromCharname(wscCharname);
	if (iClientID != -1)
	{
		uint iSystem = 0;
		pub::Player::GetSystem(iClientID, iSystem);
		if (iSystem == set_iBlockedSystem)
			return true;
		return false;
	}

	// Have to check the charfile.
	std::wstring wscSystemNick;
	if (HkFLIniGet(wscCharname, L"system", wscSystemNick) != HKE_OK)
		return false;

	uint iSystem = 0;
	pub::GetSystemID(iSystem, wstos(wscSystemNick).c_str());
	if (iSystem == set_iBlockedSystem)
		return true;
	return false;
}

void LoadSettings()
{
	// The path to the configuration file.
	char szCurDir[MAX_PATH];
	GetCurrentDirectory(sizeof(szCurDir), szCurDir);
	std::string scPluginCfgFile = std::string(szCurDir) + "\\flhook_plugins\\cash_manager.cfg";

	set_iMinTransfer = IniGetI(scPluginCfgFile, "General", "MinTransfer", 1);
	set_iMinTime = IniGetI(scPluginCfgFile, "General", "MinTime", 0);
	set_bCheatDetection = IniGetB(scPluginCfgFile, "General", "CheatDetection", true);
	set_iBlockedSystem = CreateID(IniGetS(scPluginCfgFile, "General", "BlockedSystem", "").c_str());
}

/// Check for cash transfer while this char was offline whenever they
/// enter or leave a base.
void PlayerLaunch(uint& iShip, uint& iClientID)
{
	CheckTransferLog(iClientID);
}

/// Check for cash transfer while this char was offline whenever they
/// enter or leave a base. */
void BaseEnter(uint& iBaseID, uint& iClientID)
{
	CheckTransferLog(iClientID);
}

/** Process a give cash command */
void UserCmd_GiveCash(uint iClientID, const std::wstring& wscParam)
{
	// The last error.
	HK_ERROR err;

	// Get the current character name
	std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

	// Get the parameters from the user command.
	std::wstring wscTargetCharname = GetParam(wscParam, L' ', 0);
	std::wstring wscCash = GetParam(wscParam, L' ', 1);
	std::wstring wscAnon = GetParam(wscParam, L' ', 2);
	wscCash = ReplaceStr(wscCash, L".", L"");
	wscCash = ReplaceStr(wscCash, L",", L"");
	wscCash = ReplaceStr(wscCash, L"$", L"");
	int cash = ToInt(wscCash);
	if ((!wscTargetCharname.length() || cash <= 0) || (wscAnon.size() && wscAnon != L"anon"))
	{
		PrintUserCmdText(iClientID, L"ERR Invalid parameters");
		PrintUserCmdText(iClientID, L"Usage: /givecash <charname> <cash> [anon]");
		return;
	}

	bool bAnon = false;
	if (wscAnon == L"anon")
		bAnon = true;

	if (HkGetAccountByCharname(wscTargetCharname) == 0)
	{
		PrintUserCmdText(iClientID, L"ERR char does not exist");
		return;
	}

	int secs = 0;
	HkGetOnlineTime(wscCharname, secs);
	if (secs < set_iMinTime)
	{
		PrintUserCmdText(iClientID, L"ERR insufficient time online");
		return;
	}

	if (InBlockedSystem(wscCharname) || InBlockedSystem(wscTargetCharname))
	{
		PrintUserCmdText(iClientID, L"ERR cash transfer blocked");
		return;
	}

	// Read the current number of credits for the player
	// and check that the character has enough cash.
	int iCash = 0;
	if ((err = HkGetCash(wscCharname, iCash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}
	if (cash < set_iMinTransfer || cash < 0)
	{
		PrintUserCmdText(
		    iClientID, L"ERR Transfer too small, minimum transfer " + ToMoneyStr(set_iMinTransfer) + L" credits");
		return;
	}
	if (iCash < cash)
	{
		PrintUserCmdText(iClientID, L"ERR Insufficient credits");
		return;
	}

	// Prevent target ship from becoming corrupt.
	float fTargetValue = 0.0f;
	if (HKGetShipValue(wscTargetCharname, fTargetValue) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}
	if ((fTargetValue + cash) > 2000000000.0f)
	{
		PrintUserCmdText(iClientID, L"ERR Transfer will exceed credit limit");
		return;
	}

	// Calculate the new cash
	int iExpectedCash = 0;
	if ((err = HkGetCash(wscTargetCharname, iExpectedCash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR Get cash failed err=" + HkErrGetText(err));
		return;
	}
	iExpectedCash += cash;

	// Do an anticheat check on the receiving character first.
	uint targetClientId = HkGetClientIdFromCharname(wscTargetCharname);
	if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId))
	{
		if (HkAntiCheat(targetClientId) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR Transfer failed");
			AddLog(
			    Cheater,
			    L"NOTICE: Possible cheating when sending %s credits from %s "
			    "(%s) "
			    "to %s (%s)",
			    ToMoneyStr(cash).c_str(), wscCharname.c_str(),
			    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str(), wscTargetCharname.c_str(),
			    HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str());
			return;
		}
		HkSaveChar(targetClientId);
	}

	if (targetClientId != -1)
	{
		if (ClientInfo[iClientID].iTradePartner || ClientInfo[targetClientId].iTradePartner)
		{
			PrintUserCmdText(iClientID, L"ERR Trade window open");
			AddLog(
			    Normal,
			    L"NOTICE: Trade window open when sending %s credits from %s "
			    "(%s) "
			    "to %s (%s) %u %u",
			    ToMoneyStr(cash).c_str(), wscCharname.c_str(),
			    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str(), wscTargetCharname.c_str(),
			    HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str(), iClientID, targetClientId);
			return;
		}
	}

	// Remove cash from current character and save it checking that the
	// save completes before allowing the cash to be added to the target ship.
	if ((err = HkAddCash(wscCharname, 0 - cash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR Remove cash failed err=" + HkErrGetText(err));
		return;
	}

	if (HkAntiCheat(iClientID) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR Transfer failed");
		AddLog(
		    Cheater,
		    L"NOTICE: Possible cheating when sending %s credits from %s (%s) to "
		    "%s (%s)",
		    ToMoneyStr(cash).c_str(), wscCharname.c_str(), HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str(),
		    wscTargetCharname.c_str(), HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str());
		return;
	}
	HkSaveChar(iClientID);

	// Add cash to target character
	if ((err = HkAddCash(wscTargetCharname, cash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR Add cash failed err=" + HkErrGetText(err));
		return;
	}

	targetClientId = HkGetClientIdFromCharname(wscTargetCharname);
	if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId))
	{
		if (HkAntiCheat(targetClientId) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR Transfer failed");
			AddLog(
			    Cheater,
			    L"NOTICE: Possible cheating when sending %s credits from %s "
			    "(%s) "
			    "to %s (%s)",
			    ToMoneyStr(cash).c_str(), wscCharname.c_str(),
			    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str(), wscTargetCharname.c_str(),
			    HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str());
			return;
		}
		HkSaveChar(targetClientId);
	}

	// Check that receiving character has the correct amount of cash.
	int iCurrCash;
	if ((err = HkGetCash(wscTargetCharname, iCurrCash)) != HKE_OK || iCurrCash != iExpectedCash)
	{
		AddLog(
		    Error,
		    L"Cash transfer error when sending %s credits from %s (%s) "
		    "to "
		    "%s (%s) current %s credits expected %s credits ",
		    ToMoneyStr(cash).c_str(), wscCharname.c_str(), HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str(),
		    wscTargetCharname.c_str(), HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str(),
		    ToMoneyStr(iCurrCash).c_str(), ToMoneyStr(iExpectedCash).c_str());
		PrintUserCmdText(iClientID, L"ERR Transfer failed");
		return;
	}

	// If the target player is online then send them a message saying
	// telling them that they've received the cash.
	std::wstring msg =
	    L"You have received " + ToMoneyStr(cash) + L" credits from " + ((bAnon) ? L"anonymous" : wscCharname);
	if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId))
	{
		PrintUserCmdText(targetClientId, L"%s", msg.c_str());
	}
	// Otherwise we assume that the character is offline so we record an entry
	// in the character's givecash.ini. When they come online we inform them
	// of the transfer. The ini is cleared when ever the character logs in.
	else
	{
		std::wstring msg =
		    L"You have received " + ToMoneyStr(cash) + L" credits from " + ((bAnon) ? L"anonymous" : wscCharname);
		LogTransfer(wscTargetCharname, msg);
	}

	AddLog(
	    Normal, L"NOTICE: Send %s credits from %s (%s) to %s (%s)", ToMoneyStr(cash).c_str(), wscCharname.c_str(),
	    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str(), wscTargetCharname.c_str(),
	    HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str());

	// A friendly message explaining the transfer.
	msg = L"You have sent " + ToMoneyStr(cash) + L" credits to " + wscTargetCharname;
	if (bAnon)
		msg += L" anonymously";
	PrintUserCmdText(iClientID, L"%s", msg.c_str());
	return;
}

/** Process a set cash code command */
void UserCmd_SetCashCode(uint iClientID, const std::wstring& wscParam)
{
	std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);
	std::string scFile = GetUserFilePath(wscCharname, "-givecash.ini");
	if (scFile.empty())
		return;

	std::wstring wscCode = GetParam(wscParam, L' ', 0);

	if (!wscCode.size())
	{
		PrintUserCmdText(iClientID, L"ERR Invalid parameters");
		PrintUserCmdText(iClientID, L"Usage: /set cashcode <code>");
	}
	else if (wscCode == L"none")
	{
		IniWriteW(scFile, "Settings", "Code", L"");
		PrintUserCmdText(iClientID, L"OK Account code cleared");
	}
	else
	{
		IniWriteW(scFile, "Settings", "Code", wscCode);
		PrintUserCmdText(iClientID, L"OK Account code set to " + wscCode);
	}
	return;
}

/** Process a show cash command **/
void UserCmd_ShowCash(uint iClientID, const std::wstring& wscParam)
{
	// The last error.
	HK_ERROR err;

	// Get the current character name
	std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

	// Get the parameters from the user command.
	std::wstring wscTargetCharname = GetParam(wscParam, L' ', 0);
	std::wstring wscCode = GetParam(wscParam, L' ', 1);

	if (!wscTargetCharname.length() || !wscCode.length())
	{
		PrintUserCmdText(iClientID, L"ERR Invalid parameters");
		PrintUserCmdText(iClientID, L"Usage: /showcash <charname> <code>");
		return;
	}

	CAccount* acc = HkGetAccountByCharname(wscTargetCharname);
	if (acc == 0)
	{
		PrintUserCmdText(iClientID, L"ERR char does not exist");
		return;
	}

	std::string scFile = GetUserFilePath(wscTargetCharname, "-givecash.ini");
	if (scFile.empty())
		return;

	std::wstring wscTargetCode = IniGetWS(scFile, "Settings", "Code", L"");
	if (!wscTargetCode.length() || wscTargetCode != wscCode)
	{
		PrintUserCmdText(iClientID, L"ERR cash account access denied");
		return;
	}

	int iCash = 0;
	if ((err = HkGetCash(wscTargetCharname, iCash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}

	PrintUserCmdText(iClientID, L"OK Account " + wscTargetCharname + L" has " + ToMoneyStr(iCash) + L" credits");
	return;
}

/** Process a draw cash command **/
void UserCmd_DrawCash(uint iClientID, const std::wstring& wscParam)
{
	// The last error.
	HK_ERROR err;

	// Get the current character name
	std::wstring wscCharname = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

	// Get the parameters from the user command.
	std::wstring wscTargetCharname = GetParam(wscParam, L' ', 0);
	std::wstring wscCode = GetParam(wscParam, L' ', 1);
	std::wstring wscCash = GetParam(wscParam, L' ', 2);
	wscCash = ReplaceStr(wscCash, L".", L"");
	wscCash = ReplaceStr(wscCash, L",", L"");
	wscCash = ReplaceStr(wscCash, L"$", L"");
	int cash = ToInt(wscCash);
	if (!wscTargetCharname.length() || !wscCode.length() || cash <= 0)
	{
		PrintUserCmdText(iClientID, L"ERR Invalid parameters");
		PrintUserCmdText(iClientID, L"Usage: /drawcash <charname> <code> <cash>");
		return;
	}

	CAccount* iTargetAcc = HkGetAccountByCharname(wscTargetCharname);
	if (iTargetAcc == 0)
	{
		PrintUserCmdText(iClientID, L"ERR char does not exist");
		return;
	}

	int secs = 0;
	HkGetOnlineTime(wscTargetCharname, secs);
	if (secs < set_iMinTime)
	{
		PrintUserCmdText(iClientID, L"ERR insufficient time online");
		return;
	}

	if (InBlockedSystem(wscCharname) || InBlockedSystem(wscTargetCharname))
	{
		PrintUserCmdText(iClientID, L"ERR cash transfer blocked");
		return;
	}

	std::string scFile = GetUserFilePath(wscTargetCharname, "-givecash.ini");
	if (scFile.empty())
		return;

	std::wstring wscTargetCode = IniGetWS(scFile, "Settings", "Code", L"");
	if (!wscTargetCode.length() || wscTargetCode != wscCode)
	{
		PrintUserCmdText(iClientID, L"ERR cash account access denied");
		return;
	}

	if (cash < set_iMinTransfer || cash < 0)
	{
		PrintUserCmdText(
		    iClientID, L"ERR Transfer too small, minimum transfer " + ToMoneyStr(set_iMinTransfer) + L" credits");
		return;
	}

	int tCash = 0;
	if ((err = HkGetCash(wscTargetCharname, tCash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}
	if (tCash < cash)
	{
		PrintUserCmdText(iClientID, L"ERR Insufficient credits");
		return;
	}

	// Check the adding this cash to this player will not
	// exceed the maximum ship value.
	float fTargetValue = 0.0f;
	if (HKGetShipValue(wscCharname, fTargetValue) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}
	if ((fTargetValue + cash) > 2000000000.0f)
	{
		PrintUserCmdText(iClientID, L"ERR Transfer will exceed credit limit");
		return;
	}

	// Calculate the new cash
	int iExpectedCash = 0;
	if ((err = HkGetCash(wscCharname, iExpectedCash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}
	iExpectedCash += cash;

	// Do an anticheat check on the receiving ship first.
	if (HkAntiCheat(iClientID) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR Transfer failed");
		AddLog(
		    Cheater,
		    L"NOTICE: Possible cheating when drawing %s credits from %s (%s) to "
		    "%s (%s)",
		    ToMoneyStr(cash).c_str(), wscTargetCharname.c_str(),
		    HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str(), wscCharname.c_str(),
		    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str());
		return;
	}
	HkSaveChar(iClientID);

	uint targetClientId = HkGetClientIdFromCharname(wscTargetCharname);
	if (targetClientId != -1)
	{
		if (ClientInfo[iClientID].iTradePartner || ClientInfo[targetClientId].iTradePartner)
		{
			PrintUserCmdText(iClientID, L"ERR Trade window open");
			AddLog(
			    Normal,
			    L"NOTICE: Trade window open when drawing %s credits from %s "
			    "(%s) "
			    "to %s (%s) %u %u",
			    ToMoneyStr(cash).c_str(), wscTargetCharname.c_str(),
			    HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str(), wscCharname.c_str(),
			    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str(), iClientID, targetClientId);
			return;
		}
	}

	// Remove cash from target character
	if ((err = HkAddCash(wscTargetCharname, 0 - cash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}

	if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId))
	{
		if (HkAntiCheat(targetClientId) != HKE_OK)
		{
			PrintUserCmdText(iClientID, L"ERR Transfer failed");
			AddLog(
			    Cheater,
			    L"NOTICE: Possible cheating when drawing %s credits from %s "
			    "(%s) to "
			    "%s (%s)",
			    ToMoneyStr(cash).c_str(), wscTargetCharname.c_str(),
			    HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str(), wscCharname.c_str(),
			    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str());
			return;
		}
		HkSaveChar(targetClientId);
	}

	// Add cash to this player
	if ((err = HkAddCash(wscCharname, cash)) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR " + HkErrGetText(err));
		return;
	}

	if (HkAntiCheat(iClientID) != HKE_OK)
	{
		PrintUserCmdText(iClientID, L"ERR Transfer failed");
		AddLog(
		    Cheater,
		    L"NOTICE: Possible cheating when drawing %s credits from %s (%s) to "
		    "%s (%s)",
		    ToMoneyStr(cash).c_str(), wscTargetCharname.c_str(),
		    HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str(), wscCharname.c_str(),
		    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str());
		return;
	}
	HkSaveChar(iClientID);

	// Check that receiving player has the correct amount of cash.
	int iCurrCash;
	if ((err = HkGetCash(wscCharname, iCurrCash)) != HKE_OK || iCurrCash != iExpectedCash)
	{
		AddLog(
		    Error,
		    L"Cash transfer error when drawing %s credits from %s (%s) "
		    "to "
		    "%s (%s) current %s credits expected %s credits ",
		    ToMoneyStr(cash).c_str(), wscTargetCharname.c_str(),
		    HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str(), wscCharname.c_str(),
		    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str(), ToMoneyStr(iCurrCash).c_str(),
		    ToMoneyStr(iExpectedCash).c_str());
		PrintUserCmdText(iClientID, L"ERR Transfer failed");
	}

	// If the target player is online then send them a message saying
	// telling them that they've received transfered cash.
	std::wstring msg = L"You have transferred " + ToMoneyStr(cash) + L" credits to " + wscCharname;
	if (targetClientId != -1 && !HkIsInCharSelectMenu(targetClientId))
	{
		PrintUserCmdText(targetClientId, L"%s", msg.c_str());
	}
	// Otherwise we assume that the character is offline so we record an entry
	// in the character's givecash.ini. When they come online we inform them
	// of the transfer. The ini is cleared when ever the character logs in.
	else
	{
		LogTransfer(wscTargetCharname, msg);
	}

	AddLog(
	    Normal, L"NOTICE: Draw %s credits from %s (%s) to %s (%s)", ToMoneyStr(cash).c_str(), wscTargetCharname.c_str(),
	    HkGetAccountID(HkGetAccountByCharname(wscTargetCharname)).c_str(), wscCharname.c_str(),
	    HkGetAccountID(HkGetAccountByCharname(wscCharname)).c_str());

	// A friendly message explaining the transfer.
	msg =
	    GetTimeString(set_bLocalTime) + L": You have drawn " + ToMoneyStr(cash) + L" credits from " + wscTargetCharname;
	PrintUserCmdText(iClientID, L"%s", msg.c_str());
	return;
}

// Client command processing
USERCMD UserCmds[] = { { L"/givecash", UserCmd_GiveCash },
	                   { L"/sendcash", UserCmd_GiveCash },
	                   { L"/set cashcode", UserCmd_SetCashCode },
	                   { L"/showcash", UserCmd_ShowCash },
	                   { L"/drawcash", UserCmd_DrawCash } };

// Process user input
bool UserCmd_Process(uint& iClientID, const std::wstring& wscCmd)
{
	DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, returncode);
}

// Hook on /help
EXPORT void UserCmd_Help(uint& iClientID, const std::wstring& wscParam)
{
	for (auto& uc : UserCmds)
	{
		PrintUserCmdText(iClientID, uc.wszCmd);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Cash Manager Plugin by Cannon");
	pi->shortName("cash_manager");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
}
