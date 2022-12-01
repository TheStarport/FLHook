#include "Main.h"

namespace Plugins::Mark
{
	void UserCmd_MarkObj(const uint& iClientID, const std::wstring_view& wscParam)
	{
		uint iShip, iTargetShip;
		pub::Player::GetShip(iClientID, iShip);
		pub::SpaceObj::GetTarget(iShip, iTargetShip);
		char err = MarkObject(iClientID, iTargetShip);
		switch (err)
		{
			case 1:
				PrintUserCmdText(iClientID, L"Error: You must have something targeted to mark it.");
				break;
			case 2:
				PrintUserCmdText(iClientID, L"Error: You cannot mark cloaked ships.");
				break;
			case 3:
				PrintUserCmdText(iClientID, L"Error: Object is already marked.");
			default:
				break;
		}
	}

	void UserCmd_UnMarkObj(const uint& iClientID, const std::wstring_view& wscParam)
	{
		uint iShip, iTargetShip;
		pub::Player::GetShip(iClientID, iShip);
		pub::SpaceObj::GetTarget(iShip, iTargetShip);
		char err = UnMarkObject(iClientID, iTargetShip);
		switch (err)
		{
			case 0:
				// PRINT_OK()
				break;
			case 1:
				PrintUserCmdText(iClientID, L"Error: You must have something targeted to unmark it.");
				break;
			case 2:
				PrintUserCmdText(iClientID, L"Error: Object is not marked.");
			default:
				break;
		}
	}

	void UserCmd_UnMarkAllObj(const uint& iClientID, const std::wstring_view& wscParam)
	{
		UnMarkAllObjects(iClientID);
	}

	void UserCmd_MarkObjGroup(const uint& iClientID, const std::wstring_view& wscParam)
	{
		uint iShip, iTargetShip;
		pub::Player::GetShip(iClientID, iShip);
		pub::SpaceObj::GetTarget(iShip, iTargetShip);
		if (!iTargetShip)
		{
			PrintUserCmdText(iClientID, L"Error: You must have something targeted to mark it.");
			return;
		}
		std::list<GROUP_MEMBER> lstMembers;
		lstMembers.clear();
		std::wstring wsClientID = (wchar_t*)Players.GetActiveCharacterName(iClientID);
		GetGroupMembers(wsClientID, lstMembers);
		for (auto& lstG : lstMembers)
		{
			if (global->Mark[lstG.iClientID].IgnoreGroupMark)
				continue;
			uint iClientShip;
			pub::Player::GetShip(lstG.iClientID, iClientShip);
			if (iClientShip == iTargetShip)
				continue;
			MarkObject(lstG.iClientID, iTargetShip);
		}
	}

	void UserCmd_UnMarkObjGroup(const uint& iClientID, const std::wstring_view& wscParam)
	{
		uint iShip, iTargetShip;
		pub::Player::GetShip(iClientID, iShip);
		pub::SpaceObj::GetTarget(iShip, iTargetShip);
		if (!iTargetShip)
		{
			PrintUserCmdText(iClientID, L"Error: You must have something targeted to mark it.");
			return;
		}
		std::list<GROUP_MEMBER> lstMembers;
		lstMembers.clear();
		std::wstring wsClientID = (wchar_t*)Players.GetActiveCharacterName(iClientID);
		GetGroupMembers(wsClientID, lstMembers);
		for (auto& lstG : lstMembers)
		{
			UnMarkObject(lstG.iClientID, iTargetShip);
		}
	}

	void UserCmd_SetIgnoreGroupMark(const uint& iClientID, const std::wstring_view& wscParam)
	{
		const std::wstring wscError[] = {
		    L"Error: Invalid parameters",
		    L"Usage: /ignoregroupmarks <on|off>",
		};

		const auto param = ViewToWString(wscParam);

		if (ToLower(param) == L"off")
		{
			global->Mark[iClientID].IgnoreGroupMark = false;
			std::wstring dir, wscFilename;
			CAccount* acc = Players.FindAccountFromClientID(iClientID);
			if (HKSUCCESS(GetCharFileName(iClientID, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
			{
				std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
				std::string scSection = "general_" + wstos(wscFilename);
				IniWrite(scUserFile, scSection, "automarkenabled", "no");
				PrintUserCmdText(iClientID, L"Accepting marks from the group");
			}
		}
		else if (ToLower(param) == L"on")
		{
			global->Mark[iClientID].IgnoreGroupMark = true;
			std::wstring dir, wscFilename;
			CAccount* acc = Players.FindAccountFromClientID(iClientID);
			if (HKSUCCESS(GetCharFileName(iClientID, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
			{
				std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
				std::string scSection = "general_" + wstos(wscFilename);
				IniWrite(scUserFile, scSection, "automarkenabled", "yes");
				PrintUserCmdText(iClientID, L"Ignoring marks from the group");
			}
		}
		else
		{
			PRINT_ERROR();
		}
	}

	void UserCmd_AutoMark(const uint& iClientID, const std::wstring_view& wscParam)
	{
		if (global->config->AutoMarkRadiusInM <= 0.0f) // automarking disabled
		{
			PrintUserCmdText(iClientID, L"Command disabled");
			return;
		}

		std::wstring wscError[] = {
		    L"Error: Invalid parameters",
		    L"Usage: /automark <on|off> [radius in KM]",
		};

		std::wstring wscEnabled = ToLower(GetParam(wscParam, ' ', 0));

		if (!wscParam.length() || (wscEnabled != L"on" && wscEnabled != L"off"))
		{
			PRINT_ERROR();
			return;
		}

		std::wstring wscRadius = GetParam(wscParam, ' ', 1);
		float fRadius = 0.0f;
		if (wscRadius.length())
		{
			fRadius = ToFloat(wscRadius);
		}

		// I think this section could be done better, but I can't think of it now..
		if (!global->Mark[iClientID].MarkEverything)
		{
			if (wscRadius.length())
				global->Mark[iClientID].AutoMarkRadius = fRadius * 1000;
			if (wscEnabled == L"on") // AutoMark is being enabled
			{
				global->Mark[iClientID].MarkEverything = true;
				CAccount* acc = Players.FindAccountFromClientID(iClientID);
				std::wstring dir, wscFilename;
				if (HKSUCCESS(GetCharFileName(iClientID, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
				{
					std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
					std::string scSection = "general_" + wstos(wscFilename);
					IniWrite(scUserFile, scSection, "automark", "yes");
					if (wscRadius.length())
						IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[iClientID].AutoMarkRadius));
				}
				PrintUserCmdText(iClientID, L"Automarking turned on within a %g KM radius", global->Mark[iClientID].AutoMarkRadius / 1000);
			}
			else if (wscRadius.length())
			{
				CAccount* acc = Players.FindAccountFromClientID(iClientID);
				std::wstring dir, wscFilename;
				if (HKSUCCESS(GetCharFileName(iClientID, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
				{
					std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
					std::string scSection = "general_" + wstos(wscFilename);
					IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[iClientID].AutoMarkRadius));
				}
				PrintUserCmdText(iClientID, L"Radius changed to %g KMs", fRadius);
			}
		}
		else
		{
			if (wscRadius.length())
				global->Mark[iClientID].AutoMarkRadius = fRadius * 1000;
			if (wscEnabled == L"off") // AutoMark is being disabled
			{
				global->Mark[iClientID].MarkEverything = false;
				CAccount* acc = Players.FindAccountFromClientID(iClientID);
				std::wstring dir, wscFilename;
				if (HKSUCCESS(GetCharFileName(iClientID, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
				{
					std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
					std::string scSection = "general_" + wstos(wscFilename);
					IniWrite(scUserFile, scSection, "automark", "no");
					if (wscRadius.length())
						IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[iClientID].AutoMarkRadius));
				}
				if (wscRadius.length())
					PrintUserCmdText(iClientID, L"Automarking turned off; radius changed to %g KMs", global->Mark[iClientID].AutoMarkRadius / 1000);
				else
					PrintUserCmdText(iClientID, L"Automarking turned off");
			}
			else if (wscRadius.length())
			{
				CAccount* acc = Players.FindAccountFromClientID(iClientID);
				std::wstring dir, wscFilename;
				if (HKSUCCESS(GetCharFileName(iClientID, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
				{
					std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
					std::string scSection = "general_" + wstos(wscFilename);
					IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[iClientID].AutoMarkRadius));
				}
				PrintUserCmdText(iClientID, L"Radius changed to %g KMs", fRadius);
			}
		}
	}
}