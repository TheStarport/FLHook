#include "Mark.h"

namespace Plugins::Mark
{
	void UserCmd_MarkObj(ClientId& client, const std::wstring& Param)
	{
		auto target = Hk::Player::GetTarget(client);
		if (target.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(target.error()));
			return;
		}
		const char err = MarkObject(client, target.value());
		switch (err)
		{
			case 0:
				PrintUserCmdText(client, L"OK");
				break;
			case 1:
				PrintUserCmdText(client, L"Error: You must have something targeted to mark it.");
				break;
			case 2:
				PrintUserCmdText(client, L"Error: You cannot mark cloaked ships.");
				break;
			case 3:
				PrintUserCmdText(client, L"Error: Object is already marked.");
				break;
		}
	}

	void UserCmd_UnMarkObj(ClientId& client, const std::wstring& Param)
	{
		auto target = Hk::Player::GetTarget(client);
		if (target.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(target.error()));
			return;
		}
		const char err = UnMarkObject(client, target.value());
		switch (err)
		{
			case 0:
				PrintUserCmdText(client, L"OK");
				break;
			case 1:
				PrintUserCmdText(client, L"Error: You must have something targeted to unmark it.");
				break;
			case 2:
				PrintUserCmdText(client, L"Error: Object is not marked.");
				break;
		}
	}

	void UserCmd_UnMarkAllObj(ClientId& client, const std::wstring& Param)
	{
		UnMarkAllObjects(client);
	}

	void UserCmd_MarkObjGroup(ClientId& client, const std::wstring& Param)
	{
		auto target = Hk::Player::GetTarget(client);
		char err = MarkObject(client, target.value());
		if (target.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(target.error()));
			return;
		}

		const auto Members = Hk::Player::GetGroupMembers(client);

		if (Members.has_error())
		{
			return;
		}

		for (const auto& [groupClient, _] : Members.value())
		{
			if (global->Mark[groupClient].IgnoreGroupMark)
				continue;

			uint iClientShip = Hk::Player::GetShip(client).value();
			if (iClientShip == target)
				continue;

			MarkObject(groupClient, target.value());
		}
	}

	void UserCmd_UnMarkObjGroup(ClientId& client, const std::wstring& Param)
	{
		auto target = Hk::Player::GetTarget(client);
		char err = UnMarkObject(client, target.value());
		if (target.has_error())
		{
			PrintUserCmdText(client, Hk::Err::ErrGetText(target.error()));
			return;
		}
		const auto Members = Hk::Player::GetGroupMembers(client);
		if (Members.has_error())
		{
			return;
		}

		for (const auto& [groupClient, _] : Members.value())
		{
			UnMarkObject(groupClient, target.value());
		}
	}

	void UserCmd_SetIgnoreGroupMark(ClientId& client, const std::wstring& Param)
	{
		const std::wstring Error[] = {
		    L"Error: Invalid parameters",
		    L"Usage: /ignoregroupmarks <on|off>",
		};

		const auto param = ViewToWString(Param);

		if (ToLower(param) == L"off")
		{
			global->Mark[client].IgnoreGroupMark = false;
			CAccount const* acc = Players.FindAccountFromClientID(client);
			const auto fileName = Hk::Client::GetCharFileName(client);
			const auto dir = Hk::Client::GetAccountDirName(acc);
			if (fileName.has_value())
			{
				std::string UserFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini";
				std::string Section = "general_" + wstos(fileName.value());
				IniWrite(scUserFile, scSection, "automarkenabled", "no");
				PrintUserCmdText(client, L"Accepting marks from the group");
			}
		}
		else if (ToLower(param) == L"on")
		{
			global->Mark[client].IgnoreGroupMark = true;
			CAccount const* acc = Players.FindAccountFromClientID(client);
			const auto fileName = Hk::Client::GetCharFileName(client);
			const auto dir = Hk::Client::GetAccountDirName(acc);
			if (fileName.has_value())
			{
				std::string UserFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini";
				std::string Section = "general_" + wstos(fileName.value());
				IniWrite(scUserFile, scSection, "automarkenabled", "yes");
				PrintUserCmdText(client, L"Ignoring marks from the group");
			}
		}
		else
		{
			PRINT_ERROR();
		}
	}

	void UserCmd_AutoMark(ClientId& client, const std::wstring& Param)
	{
		if (global->config->AutoMarkRadiusInM <= 0.0f) // automarking disabled
		{
			PrintUserCmdText(client, L"Command disabled");
			return;
		}

		std::wstring Error[] = {
		    L"Error: Invalid parameters",
		    L"Usage: /automark <on|off> [radius in KM]",
		};

		std::wstring Enabled = ToLower(GetParam(Param, ' ', 0));

		if (!Param.length() || (Enabled != L"on" && Enabled != L"off"))
		{
			PRINT_ERROR();
			return;
		}

		std::wstring Radius = GetParam(Param, ' ', 1);
		float fRadius = 0.0f;
		if (Radius.length())
		{
			fRadius = ToFloat(Radius);
		}

		// I think this section could be done better, but I can't think of it now..
		if (!global->Mark[client].MarkEverything)
		{
			if (Radius.length())
				global->Mark[client].AutoMarkRadius = fRadius * 1000;
			if (Enabled == L"on") // AutoMark is being enabled
			{
				global->Mark[client].MarkEverything = true;
				CAccount const* acc = Players.FindAccountFromClientID(client);
				const auto fileName = Hk::Client::GetCharFileName(client);
				const auto dir = Hk::Client::GetAccountDirName(acc);
				if (fileName.has_value())
				{
					std::string UserFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini";
					std::string Section = "general_" + wstos(fileName.value());
					IniWrite(scUserFile, scSection, "automark", "yes");
					if (Radius.length())
						IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				PrintUserCmdText(client, std::format(L"Automarking turned on within a {:.2f} KM radius", global->Mark[client].AutoMarkRadius / 1000));
			}
			else if (Radius.length())
			{
				CAccount const* acc = Players.FindAccountFromClientID(client);
				const auto fileName = Hk::Client::GetCharFileName(client);
				const auto dir = Hk::Client::GetAccountDirName(acc);
				if (fileName.has_value())
				{
					std::string UserFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini";
					std::string Section = "general_" + wstos(fileName.value());
					IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				PrintUserCmdText(client, std::format(L"Radius changed to {:.2f} KMs", fRadius));
			}
		}
		else
		{
			if (Radius.length())
				global->Mark[client].AutoMarkRadius = fRadius * 1000;
			if (Enabled == L"off") // AutoMark is being disabled
			{
				global->Mark[client].MarkEverything = false;
				CAccount const* acc = Players.FindAccountFromClientID(client);
				const auto fileName = Hk::Client::GetCharFileName(client);
				const auto dir = Hk::Client::GetAccountDirName(acc);
				if (fileName.has_value())
				{
					std::string UserFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini";
					std::string Section = "general_" + wstos(fileName.value());
					IniWrite(scUserFile, scSection, "automark", "no");
					if (Radius.length())
						IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				if (Radius.length())
					PrintUserCmdText(client, std::format(L"Automarking turned off; radius changed to {:.2f} KMs", global->Mark[client].AutoMarkRadius / 1000));
				else
					PrintUserCmdText(client, L"Automarking turned off");
			}
			else if (Radius.length())
			{
				CAccount const* acc = Players.FindAccountFromClientID(client);
				const auto fileName = Hk::Client::GetCharFileName(client);
				const auto dir = Hk::Client::GetAccountDirName(acc);
				if (fileName.has_value())
				{
					std::string UserFile = CoreGlobals::c()->accPath + wstos(dir) + "\\flhookuser.ini";
					std::string Section = "general_" + wstos(fileName.value());
					IniWrite(scUserFile, scSection, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				PrintUserCmdText(client, std::format(L"Radius changed to {:.2f} KMs", fRadius));
			}
		}
	}
}