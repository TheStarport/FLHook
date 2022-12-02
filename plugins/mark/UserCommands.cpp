#include "Main.h"

namespace Plugins::Mark
{
	void UserCmd_MarkObj(ClientId& client, const std::wstring_view& wscParam)
	{
		uint ship, iTargetShip;
		pub::Player::GetShip(client, ship);
		pub::SpaceObj::GetTarget(ship, iTargetShip);
		char err = MarkObject(client, iTargetShip);
		switch (err)
		{
			case 1:
				PrintUserCmdText(client, L"Error: You must have something targeted to mark it.");
				break;
			case 2:
				PrintUserCmdText(client, L"Error: You cannot mark cloaked ships.");
				break;
			case 3:
				PrintUserCmdText(client, L"Error: Object is already marked.");
			default:
				break;
		}
	}

	void UserCmd_UnMarkObj(ClientId& client, const std::wstring_view& wscParam)
	{
		uint ship, iTargetShip;
		pub::Player::GetShip(client, ship);
		pub::SpaceObj::GetTarget(ship, iTargetShip);
		char err = UnMarkObject(client, iTargetShip);
		switch (err)
		{
			case 0:
				// PRINT_OK()
				break;
			case 1:
				PrintUserCmdText(client, L"Error: You must have something targeted to unmark it.");
				break;
			case 2:
				PrintUserCmdText(client, L"Error: Object is not marked.");
			default:
				break;
		}
	}

	void UserCmd_UnMarkAllObj(ClientId& client, const std::wstring_view& wscParam)
	{
		UnMarkAllObjects(client);
	}

	void UserCmd_MarkObjGroup(ClientId& client, const std::wstring_view& wscParam)
	{
		uint ship, iTargetShip;
		pub::Player::GetShip(client, ship);
		pub::SpaceObj::GetTarget(ship, iTargetShip);
		if (!iTargetShip)
		{
			PrintUserCmdText(client, L"Error: You must have something targeted to mark it.");
			return;
		}
		std::list<GROUP_MEMBER> lstMembers;
		lstMembers.clear();
		std::wstring wsClientId = (wchar_t*)Players.GetActiveCharacterName(client);
		GetGroupMembers(wsClientId, lstMembers);
		for (auto& lstG : lstMembers)
		{
			if (global->Mark[lstG.client].IgnoreGroupMark)
				continue;
			uint iClientShip;
			pub::Player::GetShip(lstG.client, iClientShip);
			if (iClientShip == iTargetShip)
				continue;
			MarkObject(lstG.client, iTargetShip);
		}
	}

	void UserCmd_UnMarkObjGroup(ClientId& client, const std::wstring_view& wscParam)
	{
		uint ship, iTargetShip;
		pub::Player::GetShip(client, ship);
		pub::SpaceObj::GetTarget(ship, iTargetShip);
		if (!iTargetShip)
		{
			PrintUserCmdText(client, L"Error: You must have something targeted to mark it.");
			return;
		}
		std::list<GROUP_MEMBER> lstMembers;
		lstMembers.clear();
		std::wstring wsClientId = (wchar_t*)Players.GetActiveCharacterName(client);
		GetGroupMembers(wsClientId, lstMembers);
		for (auto& lstG : lstMembers)
		{
			UnMarkObject(lstG.client, iTargetShip);
		}
	}

	void UserCmd_SetIgnoreGroupMark(ClientId& client, const std::wstring_view& wscParam)
	{
		const std::wstring wscError[] = {
		    L"Error: Invalid parameters",
		    L"Usage: /ignoregroupmarks <on|off>",
		};

		const auto param = ViewToWString(wscParam);

		if (ToLower(param) == L"off")
		{
			global->Mark[client].IgnoreGroupMark = false;
			std::wstring dir, wscFilename;
			CAccount* acc = Players.FindAccountFromClientID(client);
			if (HKSUCCESS(GetCharFileName(client, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
			{
				std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
				std::string scSection = "general_" + wstos(wscFilename);
				IniWrite(scUserFile, scSection, "automarkenabled", "no");
				PrintUserCmdText(client, L"Accepting marks from the group");
			}
		}
		else if (ToLower(param) == L"on")
		{
			global->Mark[client].IgnoreGroupMark = true;
			std::wstring dir, wscFilename;
			CAccount* acc = Players.FindAccountFromClientID(client);
			if (HKSUCCESS(GetCharFileName(client, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
			{
				std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
				std::string scSection = "general_" + wstos(wscFilename);
				IniWrite(scUserFile, scSection, "automarkenabled", "yes");
				PrintUserCmdText(client, L"Ignoring marks from the group");
			}
		}
		else
		{
			PRINT_ERROR();
		}
	}

	void UserCmd_AutoMark(ClientId& client, const std::wstring_view& wscParam)
	{
		if (global->config->AutoMarkRadiusInM <= 0.0f) // automarking disabled
		{
			PrintUserCmdText(client, L"Command disabled");
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
		if (!global->Mark[client].MarkEverything)
		{
			if (wscRadius.length())
				global->Mark[client].AutoMarkRadius = fRadius * 1000;
			if (wscEnabled == L"on") // AutoMark is being enabled
			{
				global->Mark[client].MarkEverything = true;
				CAccount* acc = Players.FindAccountFromClientID(client);
				std::wstring dir, wscFilename;
				if (HKSUCCESS(GetCharFileName(client, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
				{
					std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
					std::string scSection = "general_" + wstos(wscFilename);
					IniWrite(scUserFile, scSection, "automark", "yes");
					if (wscRadius.length())
						IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				PrintUserCmdText(client, L"Automarking turned on within a %g KM radius", global->Mark[client].AutoMarkRadius / 1000);
			}
			else if (wscRadius.length())
			{
				CAccount* acc = Players.FindAccountFromClientID(client);
				std::wstring dir, wscFilename;
				if (HKSUCCESS(GetCharFileName(client, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
				{
					std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
					std::string scSection = "general_" + wstos(wscFilename);
					IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				PrintUserCmdText(client, L"Radius changed to %g KMs", fRadius);
			}
		}
		else
		{
			if (wscRadius.length())
				global->Mark[client].AutoMarkRadius = fRadius * 1000;
			if (wscEnabled == L"off") // AutoMark is being disabled
			{
				global->Mark[client].MarkEverything = false;
				CAccount* acc = Players.FindAccountFromClientID(client);
				std::wstring dir, wscFilename;
				if (HKSUCCESS(GetCharFileName(client, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
				{
					std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
					std::string scSection = "general_" + wstos(wscFilename);
					IniWrite(scUserFile, scSection, "automark", "no");
					if (wscRadius.length())
						IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				if (wscRadius.length())
					PrintUserCmdText(client, L"Automarking turned off; radius changed to %g KMs", global->Mark[client].AutoMarkRadius / 1000);
				else
					PrintUserCmdText(client, L"Automarking turned off");
			}
			else if (wscRadius.length())
			{
				CAccount* acc = Players.FindAccountFromClientID(client);
				std::wstring dir, wscFilename;
				if (HKSUCCESS(GetCharFileName(client, wscFilename)) && HKSUCCESS(GetAccountDirName(acc, dir)))
				{
					std::string scUserFile = scAcctPath + wstos(dir) + "\\flhookuser.ini";
					std::string scSection = "general_" + wstos(wscFilename);
					IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				PrintUserCmdText(client, L"Radius changed to %g KMs", fRadius);
			}
		}
	}
}