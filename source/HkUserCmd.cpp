#include "hook.h"

#define PRINT_ERROR() { for(uint i = 0; (i < sizeof(wscError)/sizeof(wstring)); i++) PrintUserCmdText(iClientID, L"%s", wscError[i].c_str()); return; }
#define PRINT_OK() PrintUserCmdText(iClientID, L"OK");
#define PRINT_DISABLED() PrintUserCmdText(iClientID, L"Command disabled");
#define GET_USERFILE(a) string a; { CAccount *acc = Players.FindAccountFromClientID(iClientID); wstring wscDir; HkGetAccountDirName(acc, wscDir); a = scAcctPath + wstos(wscDir) + "\\flhookuser.ini"; }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef void (*_UserCmdProc)(uint, wstring);

struct USERCMD
{
	wchar_t *wszCmd;
	_UserCmdProc proc;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void PrintUserCmdText(uint iClientID, wstring wscText, ...)
{
	wchar_t wszBuf[1024*8] = L"";
	va_list marker;
	va_start(marker, wscText);
	_vsnwprintf(wszBuf, sizeof(wszBuf)-1, wscText.c_str(), marker);

	wstring wscXML = L"<TRA data=\"" + set_wscUserCmdStyle + L"\" mask=\"-1\"/><TEXT>" + XMLText(wszBuf) + L"</TEXT>";
	HkFMsg(iClientID, wscXML);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetDieMsg(uint iClientID, wstring wscParam)
{
	if(!set_bUserCmdSetDieMsg)
	{
		PRINT_DISABLED();
		return;
	}

	wstring wscError[] = 
	{
		L"Error: Invalid parameters",
		L"Usage: /set diemsg <param>",
		L"<param>: all,system,self or none",
	};

	wstring wscDieMsg = ToLower(GetParam(wscParam, ' ', 0));;

	DIEMSGTYPE dieMsg;
	if(!wscDieMsg.compare(L"all"))
		dieMsg = DIEMSG_ALL;
	else if(!wscDieMsg.compare(L"system"))
		dieMsg = DIEMSG_SYSTEM;
	else if(!wscDieMsg.compare(L"none"))
		dieMsg = DIEMSG_NONE;
	else if(!wscDieMsg.compare(L"self"))
		dieMsg = DIEMSG_SELF;
	else 
		PRINT_ERROR();

	// save to ini
	GET_USERFILE(scUserFile);
	IniWrite(scUserFile, "settings", "DieMsg", itos(dieMsg));

	// save in ClientInfo
	ClientInfo[iClientID].dieMsg = dieMsg; 

	// send confirmation msg
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetDieMsgSize(uint iClientID, wstring wscParam)
{
	if(!set_bUserCmdSetDieMsgSize)
	{
		PRINT_DISABLED();
		return;
	}
	
	wstring wscError[] = 
	{
		L"Error: Invalid parameters",
		L"Usage: /set diemsgsize <size>",
		L"<size>: small, default",
	};

	wstring wscDieMsgSize = ToLower(GetParam(wscParam, ' ', 0));
//	wstring wscDieMsgStyle = ToLower(GetParam(wscParam, ' ', 1));

	CHATSIZE dieMsgSize;
	if(!wscDieMsgSize.compare(L"small"))
		dieMsgSize = CS_SMALL;
	else if(!wscDieMsgSize.compare(L"default"))
		dieMsgSize = CS_DEFAULT;
	else 
		PRINT_ERROR();

/*	CHATSTYLE dieMsgStyle;
	if(!wscDieMsgStyle.compare(L"default"))
		dieMsgStyle = CST_DEFAULT;
	else if(!wscDieMsgStyle.compare(L"bold"))
		dieMsgStyle = CST_BOLD;
	else if(!wscDieMsgStyle.compare(L"italic"))
		dieMsgStyle = CST_ITALIC;
	else if(!wscDieMsgStyle.compare(L"underline"))
		dieMsgStyle = CST_UNDERLINE;
	else 
		PRINT_ERROR(); */

	// save to ini
	GET_USERFILE(scUserFile);
	IniWrite(scUserFile, "Settings", "DieMsgSize", itos(dieMsgSize));
//	IniWrite(scUserFile, "Settings", "DieMsgStyle", itos(dieMsgStyle));

	// save in ClientInfo
	ClientInfo[iClientID].dieMsgSize = dieMsgSize; 
//	ClientInfo[iClientID].dieMsgStyle = dieMsgStyle;

	// send confirmation msg
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_SetChatFont(uint iClientID, wstring wscParam)
{
	if(!set_bUserCmdSetChatFont)
	{
		PRINT_DISABLED();
		return;
	}

	wstring wscError[] = 
	{
		L"Error: Invalid parameters",
		L"Usage: /set chatfont <size> <style>",
		L"<size>: small, default or big",
		L"<style>: default, bold, italic or underline",
	};

	wstring wscChatSize = ToLower(GetParam(wscParam, ' ', 0));
	wstring wscChatStyle = ToLower(GetParam(wscParam, ' ', 1));
	
	CHATSIZE chatSize;
	if(!wscChatSize.compare(L"small"))
		chatSize = CS_SMALL;
	else if(!wscChatSize.compare(L"default"))
		chatSize = CS_DEFAULT;
	else if(!wscChatSize.compare(L"big"))
		chatSize = CS_BIG;
	else 
		PRINT_ERROR();

	CHATSTYLE chatStyle;
	if(!wscChatStyle.compare(L"default"))
		chatStyle = CST_DEFAULT;
	else if(!wscChatStyle.compare(L"bold"))
		chatStyle = CST_BOLD;
	else if(!wscChatStyle.compare(L"italic"))
		chatStyle = CST_ITALIC;
	else if(!wscChatStyle.compare(L"underline"))
		chatStyle = CST_UNDERLINE;
	else 
		PRINT_ERROR();

	// save to ini
	GET_USERFILE(scUserFile);
	IniWrite(scUserFile, "settings", "ChatSize", itos(chatSize));
	IniWrite(scUserFile, "settings", "ChatStyle", itos(chatStyle));

	// save in ClientInfo
	ClientInfo[iClientID].chatSize = chatSize; 
	ClientInfo[iClientID].chatStyle = chatStyle; 

	// send confirmation msg
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Ignore(uint iClientID, wstring wscParam)
{
	if(!set_bUserCmdIgnore)
	{
		PRINT_DISABLED();
		return;
	}

	wstring wscError[] = 
	{
		L"Error: Invalid parameters",
		L"Usage: /ignore <charname> [<flags>]",
		L"<charname>: character name which should be ignored(case insensitive)",
		L"<flags>: combination of the following flags:",
		L" p - only affect private chat",
		L" i - <charname> may match partially",
		L"Examples:",
		L"\"/ignore SomeDude\" ignores all chatmessages from SomeDude",
		L"\"/ignore PlayerX p\" ignores all private-chatmessages from PlayerX",
		L"\"/ignore idiot i\" ignores all chatmessages from players whose charname contain \"idiot\" (e.g. \"[XYZ]IDIOT\", \"MrIdiot\", etc)",
		L"\"/ignore Fool pi\" ignores all private-chatmessages from players whose charname contain \"fool\"",
	};

	wstring wscAllowedFlags = L"pi";

	wstring wscCharname = GetParam(wscParam, ' ', 0);
	wstring wscFlags = ToLower(GetParam(wscParam, ' ', 1));

	if(!wscCharname.length())
		PRINT_ERROR();

	// check if flags are valid
	for(uint i = 0; (i < wscFlags.length()); i++)
	{
		if(wscAllowedFlags.find_first_of(wscFlags[i]) == -1)
			PRINT_ERROR();
	}

	if(ClientInfo[iClientID].lstIgnore.size() > set_iUserCmdMaxIgnoreList)
	{
		PrintUserCmdText(iClientID, L"Error: Too many entries in the ignore list, please delete an entry first!");
		return;
	}

	// save to ini
	GET_USERFILE(scUserFile);
	IniWriteW(scUserFile, "IgnoreList", itos((int)ClientInfo[iClientID].lstIgnore.size() + 1), (wscCharname + L" " + wscFlags));

	// save in ClientInfo
	IGNORE_INFO ii;
	ii.wscCharname = wscCharname;
	ii.wscFlags = wscFlags;
	ClientInfo[iClientID].lstIgnore.push_back(ii);

	// send confirmation msg
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IgnoreID(uint iClientID, wstring wscParam)
{
	if(!set_bUserCmdIgnore)
	{
		PRINT_DISABLED();
		return;
	}

	wstring wscError[] = 
	{
		L"Error: Invalid parameters",
		L"Usage: /ignoreid <client-id> [<flags>]",
		L"<client-id>: client-id of character which should be ignored",
		L"<flags>: if \"p\"(without quotation marks) then only affect private chat",
	};

	wstring wscClientID = GetParam(wscParam, ' ', 0);
	wstring wscFlags = ToLower(GetParam(wscParam, ' ', 1));

	if(!wscClientID.length())
		PRINT_ERROR();

	if(wscFlags.length() && wscFlags.compare(L"p") != 0)
		PRINT_ERROR();

	if(ClientInfo[iClientID].lstIgnore.size() > set_iUserCmdMaxIgnoreList)
	{
		PrintUserCmdText(iClientID, L"Error: Too many entries in the ignore list, please delete an entry first!");
		return;
	}

	uint iClientIDTarget = ToInt(wscClientID);
	if(!HkIsValidClientID(iClientIDTarget) || HkIsInCharSelectMenu(iClientIDTarget))
	{
		PrintUserCmdText(iClientID, L"Error: Invalid client-id");
		return;
	}

	wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientIDTarget);

	// save to ini
	GET_USERFILE(scUserFile);
	IniWriteW(scUserFile, "IgnoreList", itos((int)ClientInfo[iClientID].lstIgnore.size() + 1), (wscCharname + L" " + wscFlags));

	// save in ClientInfo
	IGNORE_INFO ii;
	ii.wscCharname = wscCharname;
	ii.wscFlags = wscFlags;
	ClientInfo[iClientID].lstIgnore.push_back(ii);

	// send confirmation msg
	PrintUserCmdText(iClientID, L"OK, \"%s\" added to ignore list", wscCharname.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IgnoreList(uint iClientID, wstring wscParam)
{
	if(!set_bUserCmdIgnore)
	{
		PRINT_DISABLED();
		return;
	}

	PrintUserCmdText(iClientID, L"ID | Charactername | Flags");
	int i = 1;
	foreach(ClientInfo[iClientID].lstIgnore, IGNORE_INFO, it)
	{
		PrintUserCmdText(iClientID, L"%.2u | %s | %s", i, it->wscCharname.c_str(), it->wscFlags.c_str());
		i++;
	}

	// send confirmation msg
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_DelIgnore(uint iClientID, wstring wscParam)
{
	if(!set_bUserCmdIgnore)
	{
		PRINT_DISABLED();
		return;
	}

	wstring wscError[] = 
	{
		L"Error: Invalid parameters",
		L"Usage: /delignore <id> [<id2> <id3> ...]",
		L"<id>: id of ignore-entry(see /ignorelist) or *(delete all)",
	};

	wstring wscID = GetParam(wscParam, ' ', 0);

	if(!wscID.length())
		PRINT_ERROR();

	GET_USERFILE(scUserFile);

	if(!wscID.compare(L"*"))
	{ // delete all
		IniDelSection(scUserFile, "IgnoreList");
		ClientInfo[iClientID].lstIgnore.clear();
		PRINT_OK();
		return;
	}

	list<uint> lstDelete;
	for(uint j = 1; wscID.length(); j++)
	{
		uint iID = _wtoi(wscID.c_str());
		if(!iID || (iID > ClientInfo[iClientID].lstIgnore.size()))
		{
			PrintUserCmdText(iClientID, L"Error: Invalid ID");
			return;
		}

		lstDelete.push_back(iID);
		wscID = GetParam(wscParam, ' ', j);
	}

	lstDelete.sort(greater<uint>());

	ClientInfo[iClientID].lstIgnore.reverse();
	foreach(lstDelete, uint, it)
	{
		uint iCurID = (uint)ClientInfo[iClientID].lstIgnore.size();
		foreach(ClientInfo[iClientID].lstIgnore, IGNORE_INFO, it2)
		{
			if(iCurID == (*it))
			{
				ClientInfo[iClientID].lstIgnore.erase(it2);
				break;
			}
			iCurID--;
		}
	}
	ClientInfo[iClientID].lstIgnore.reverse();

	// send confirmation msg
	IniDelSection(scUserFile, "IgnoreList");
	int i = 1;
	foreach(ClientInfo[iClientID].lstIgnore, IGNORE_INFO, it3)
	{
		IniWriteW(scUserFile, "IgnoreList", itos(i), ((*it3).wscCharname + L" " + (*it3).wscFlags));
		i++;
	}
	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_AutoBuy(uint iClientID, wstring wscParam)
{
	if(!set_bAutoBuy)
	{
		PRINT_DISABLED();
		return;
	}

	wstring wscError[] = 
	{
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

	wstring wscType = ToLower(GetParam(wscParam, ' ', 0));
	wstring wscSwitch = ToLower(GetParam(wscParam, ' ', 1));

	if(!wscType.compare(L"info"))
	{
		PrintUserCmdText(iClientID, L"Missiles: %s", ClientInfo[iClientID].bAutoBuyMissiles ? L"On" : L"Off");
		PrintUserCmdText(iClientID, L"Mine: %s", ClientInfo[iClientID].bAutoBuyMines ? L"On" : L"Off");
		PrintUserCmdText(iClientID, L"Torpedos: %s", ClientInfo[iClientID].bAutoBuyTorps ? L"On" : L"Off");
		PrintUserCmdText(iClientID, L"Cruise Disruptors: %s", ClientInfo[iClientID].bAutoBuyCD ? L"On" : L"Off");
		PrintUserCmdText(iClientID, L"Countermeasures: %s", ClientInfo[iClientID].bAutoBuyCM ? L"On" : L"Off");
		PrintUserCmdText(iClientID, L"Nanobots/Shield Batteries: %s", ClientInfo[iClientID].bAutoBuyReload ? L"On" : L"Off");
		return;
	}

	if(!wscType.length() || !wscSwitch.length() || ((wscSwitch.compare(L"on") != 0) && (wscSwitch.compare(L"off") != 0)))
		PRINT_ERROR();

	GET_USERFILE(scUserFile);

	wstring wscFilename;
	HkGetCharFileName(ARG_CLIENTID(iClientID), wscFilename);
	string scSection = "autobuy_" + wstos(wscFilename);

	bool bEnable = !wscSwitch.compare(L"on") ? true : false;
	if(!wscType.compare(L"all")) {
		ClientInfo[iClientID].bAutoBuyMissiles = bEnable;
		ClientInfo[iClientID].bAutoBuyMines = bEnable;
		ClientInfo[iClientID].bAutoBuyTorps = bEnable;
		ClientInfo[iClientID].bAutoBuyCD = bEnable;
		ClientInfo[iClientID].bAutoBuyCM = bEnable;		
		ClientInfo[iClientID].bAutoBuyReload = bEnable; 
		IniWrite(scUserFile, scSection, "missiles", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "mines", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "torps", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "cd", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "cm", bEnable ? "yes" : "no");
		IniWrite(scUserFile, scSection, "reload", bEnable ? "yes" : "no");
	} else if(!wscType.compare(L"missiles")) {
		ClientInfo[iClientID].bAutoBuyMissiles = bEnable;
		IniWrite(scUserFile, scSection, "missiles", bEnable ? "yes" : "no");
	} else if(!wscType.compare(L"mines")) {
		ClientInfo[iClientID].bAutoBuyMines = bEnable;
		IniWrite(scUserFile, scSection, "mines", bEnable ? "yes" : "no");
	} else if(!wscType.compare(L"torps")) {
		ClientInfo[iClientID].bAutoBuyTorps = bEnable;
		IniWrite(scUserFile, scSection, "torps", bEnable ? "yes" : "no");
	} else if(!wscType.compare(L"cd")) {
		ClientInfo[iClientID].bAutoBuyCD = bEnable;
		IniWrite(scUserFile, scSection, "cd", bEnable ? "yes" : "no");
	} else if(!wscType.compare(L"cm")) {
		ClientInfo[iClientID].bAutoBuyCM = bEnable;		
		IniWrite(scUserFile, scSection, "cm", bEnable ? "yes" : "no");
	} else if(!wscType.compare(L"reload")) {
		ClientInfo[iClientID].bAutoBuyReload = bEnable; 
		IniWrite(scUserFile, scSection, "reload", bEnable ? "yes" : "no");
	} else
		PRINT_ERROR();

	PRINT_OK();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_IDs(uint iClientID, wstring wscParam)
{
	wchar_t wszLine[128] = L"";
	list<HKPLAYERINFO> lstPlayers = HkGetPlayers();
	foreach(lstPlayers, HKPLAYERINFO, i)
	{
		wchar_t wszBuf[1024];
		swprintf(wszBuf, L"%s = %u | ", (*i).wscCharname.c_str(), (*i).iClientID);
		if((wcslen(wszBuf) + wcslen(wszLine)) >= sizeof(wszLine)/2)	{
			PrintUserCmdText(iClientID, L"%s", wszLine);
			wcscpy(wszLine, wszBuf);
		} else
			wcscat(wszLine, wszBuf);
	}

	if(wcslen(wszLine))
		PrintUserCmdText(iClientID, L"%s", wszLine);
	PrintUserCmdText(iClientID, L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_ID(uint iClientID, wstring wscParam)
{
	PrintUserCmdText(iClientID, L"Your client-id: %u", iClientID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_InviteID(uint iClientID, wstring wscParam)
{
	wstring wscError[] = 
	{
		L"Error: Invalid parameters",
		L"Usage: /i$ <client-id>",
	};

	wstring wscClientID = GetParam(wscParam, ' ', 0);

	if(!wscClientID.length())
		PRINT_ERROR();

	uint iClientIDTarget = ToInt(wscClientID);
	if(!HkIsValidClientID(iClientIDTarget) || HkIsInCharSelectMenu(iClientIDTarget))
	{
		PrintUserCmdText(iClientID, L"Error: Invalid client-id");
		return;
	}

	wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientIDTarget);

	wstring wscXML = L"<TEXT>/i " + XMLText(wscCharname) + L"</TEXT>";
	char szBuf[0xFFFF];
	uint iRet;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet)))
	{
		PrintUserCmdText(iClientID, L"Error: Could not encode XML");
		return;
	}

	CHAT_ID cID;
	cID.iID = iClientID;
	CHAT_ID cIDTo;
	cIDTo.iID = 0x00010001;
	Server.SubmitChat(cID, iRet, szBuf, cIDTo, -1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Credits(uint iClientID, wstring wscParam)
{
	PrintUserCmdText(iClientID, L"This server is running FLHook (v" VERSION);
	PrintUserCmdText(iClientID, L"Running plugins:");

	bool bRunning = false;
	foreach(lstPlugins,PLUGIN_DATA,it) {
		if(it->bPaused)
			continue;

		bRunning = true;
		PrintUserCmdText(iClientID, L"- %s", stows(it->sName).c_str());
	}
	if(!bRunning)
		PrintUserCmdText(iClientID, L"- none -");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Help(uint iClientID, wstring wscParam)
{
	if(!set_bUserCmdHelp)
	{
		PRINT_DISABLED();
		return;
	}

	PrintUserCmdText(iClientID, L"Available commands (type in chat window):");

	if(set_bUserCmdSetDieMsg)
		PrintUserCmdText(iClientID, L"/set diemsg");
	if(set_bUserCmdSetDieMsgSize)
		PrintUserCmdText(iClientID, L"/set diemsgsize");
	if(set_bUserCmdSetChatFont)
		PrintUserCmdText(iClientID, L"/set chatfont");
	if(set_bUserCmdIgnore) {
		PrintUserCmdText(iClientID, L"/ignorelist");
		PrintUserCmdText(iClientID, L"/ignore");
		PrintUserCmdText(iClientID, L"/delignore");
		PrintUserCmdText(iClientID, L"/ignoreid");
	}
	if(set_bAutoBuy)
		PrintUserCmdText(iClientID, L"/autobuy");
	
	PrintUserCmdText(iClientID, L"/ids");
	PrintUserCmdText(iClientID, L"/id");
	PrintUserCmdText(iClientID, L"/i$");
	PrintUserCmdText(iClientID, L"/invite$");
	PrintUserCmdText(iClientID, L"/credits");
	PrintUserCmdText(iClientID, L"/help");

	CALL_PLUGINS(PLUGIN_UserCmd_Help,(iClientID,wscParam));

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

USERCMD UserCmds[] =
{
	{ L"/set diemsg",			UserCmd_SetDieMsg},
	{ L"/set diemsgsize",		UserCmd_SetDieMsgSize},
	{ L"/set chatfont",			UserCmd_SetChatFont},
	{ L"/ignorelist",			UserCmd_IgnoreList},
	{ L"/delignore",			UserCmd_DelIgnore},
	{ L"/ignore",				UserCmd_Ignore},
	{ L"/ignoreid",				UserCmd_IgnoreID},
	{ L"/autobuy",				UserCmd_AutoBuy},
	{ L"/ids",					UserCmd_IDs},
	{ L"/id",					UserCmd_ID},
	{ L"/i$",					UserCmd_InviteID},
	{ L"/invite$",				UserCmd_InviteID},
	{ L"/credits",				UserCmd_Credits},
	{ L"/help",					UserCmd_Help},
};

bool UserCmd_Process(uint iClientID, wstring wscCmd)
{

	CALL_PLUGINS(PLUGIN_UserCmd_Process,(iClientID,wscCmd));
	if(bPluginReturn) 
		return reinterpret_cast<bool>(vPluginRet);


	wstring wscCmdLower = ToLower(wscCmd);

	for(uint i = 0; (i < sizeof(UserCmds)/sizeof(USERCMD)); i++)
	{
		if(wscCmdLower.find(ToLower(UserCmds[i].wszCmd)) == 0)
		{
			wstring wscParam = L"";
			if(wscCmd.length() > wcslen(UserCmds[i].wszCmd))
			{
				if(wscCmd[wcslen(UserCmds[i].wszCmd)] != ' ')
					continue;
				wscParam = wscCmd.substr(wcslen(UserCmds[i].wszCmd) + 1);
			}

			// addlog
			if(set_bLogUserCmds) {
				wstring wscCharname = (wchar_t*)Players.GetActiveCharacterName(iClientID);
				HkAddUserCmdLog("%s: %s",wstos(wscCharname).c_str(),wstos(wscCmd).c_str());
			}

			try {
				UserCmds[i].proc(iClientID, wscParam);
				if(set_bLogUserCmds)
					HkAddUserCmdLog("finished");
			} catch(...) {
				if(set_bLogUserCmds)
					HkAddUserCmdLog("exception");
			}

			return true;
		}
	}

	return false;
}
