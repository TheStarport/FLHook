#include "Mark.h"

namespace Plugins::Mark
{
	void UserCmd_MarkObj(ClientId& client, const std::wstring& Param)
	{
		auto target = Hk::Player::GetTarget(client);
		if (target.has_error())
		{
			client.Message(Hk::Err::ErrGetText(target.error()));
			return;
		}
		const char err = MarkObject(client, target.value());
		switch (err)
		{
			case 0:
				client.Message(L"OK");
				break;
			case 1:
				client.Message(L"Error: You must have something targeted to mark it.");
				break;
			case 2:
				client.Message(L"Error: You cannot mark cloaked ships.");
				break;
			case 3:
				client.Message(L"Error: Object is already marked.");
				break;
		}
	}

	void UserCmd_UnMarkObj(ClientId& client, const std::wstring& Param)
	{
		auto target = Hk::Player::GetTarget(client);
		if (target.has_error())
		{
			client.Message(Hk::Err::ErrGetText(target.error()));
			return;
		}
		const char err = UnMarkObject(client, target.value());
		switch (err)
		{
			case 0:
				client.Message(L"OK");
				break;
			case 1:
				client.Message(L"Error: You must have something targeted to unmark it.");
				break;
			case 2:
				client.Message(L"Error: Object is not marked.");
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
			client.Message(Hk::Err::ErrGetText(target.error()));
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

			uint clientShip = Hk::Player::GetShip(client).value();
			if (clientShip == target)
				continue;

			MarkObject(groupClient, target.value());
		}
	}

	void UserCmd_UnMarkObjGroup(ClientId& client, const std::wstring& param)
	{
		auto target = Hk::Player::GetTarget(client);
		char err = UnMarkObject(client, target.value());
		if (target.has_error())
		{
			client.Message(Hk::Err::ErrGetText(target.error()));
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

	void UserCmd_SetIgnoreGroupMark(ClientId& client, const std::wstring& paramStr)
	{

		const auto param = ViewToWString(paramStr);

		if (StringUtils::ToLower(param) == L"off")
		{
			global->Mark[client].IgnoreGroupMark = false;
			CAccount const* acc = Players.FindAccountFromClientID(client);
			const auto fileName = Hk::Client::GetCharFileName(client);
			const auto dir = Hk::Client::GetAccountDirName(acc);
			if (fileName.has_value())
			{
				std::string userFile = CoreGlobals::c()->accPath + StringUtils::wstos(dir) + "\\flhookuser.ini";
				std::string section = "general_" + StringUtils::wstos(fileName.value());
				IniWrite(userFile, section, "automarkenabled", "no");
				client.Message(L"Accepting marks from the group");
			}
		}
		else if (StringUtils::ToLower(param) == L"on")
		{
			global->Mark[client].IgnoreGroupMark = true;
			CAccount const* acc = Players.FindAccountFromClientID(client);
			const auto fileName = Hk::Client::GetCharFileName(client);
			const auto dir = Hk::Client::GetAccountDirName(acc);
			if (fileName.has_value())
			{
				std::string userFile = CoreGlobals::c()->accPath + StringUtils::wstos(dir) + "\\flhookuser.ini";
				std::string section = "general_" + StringUtils::wstos(fileName.value());
				IniWrite(userFile, section, "automarkenabled", "yes");
				client.Message(L"Ignoring marks from the group");
			}
		}
		else
		{
			PrintUserCmdText(client,
			    L"Error: Invalid parameters\n"
			    L"Usage: /ignoregroupmarks <on|off>");
		}
	}

	void UserCmd_AutoMark(ClientId& client, const std::wstring& param)
	{
		if (global->config->AutoMarkRadiusInM <= 0.0f) // automarking disabled
		{
			client.Message(L"Command disabled");
			return;
		}

		std::wstring enabled = StringUtils::ToLower(GetParam(param, ' ', 0));

		if (param.empty() || (enabled != L"on" && enabled != L"off"))
		{
			PrintUserCmdText(client,
			    L"Error: Invalid parameters\n"
			    L"Usage: /automark <on|off> [radius in KM]");
			return;
		}

		std::wstring radiusString = GetParam(param, ' ', 1);
		float radius = 0.0f;
		if (!radiusString.empty())
		{
			radius = ToFloat(radiusString);
		}

		// I think this section could be done better, but I can't think of it now..
		if (!global->Mark[client].MarkEverything)
		{
			if (!radiusString.empty())
				global->Mark[client].AutoMarkRadius = radius * 1000;
			if (enabled == L"on") // AutoMark is being enabled
			{
				global->Mark[client].MarkEverything = true;
				CAccount const* acc = Players.FindAccountFromClientID(client);
				const auto fileName = Hk::Client::GetCharFileName(client);
				const auto dir = Hk::Client::GetAccountDirName(acc);
				if (fileName.has_value())
				{
					std::string userFile = CoreGlobals::c()->accPath + StringUtils::wstos(dir) + "\\flhookuser.ini";
					std::string section = "general_" + StringUtils::wstos(fileName.value());
					IniWrite(userFile, section, "automark", "yes");
					if (!radiusString.empty())
						IniWrite(userFile, section, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				client.Message(std::format(L"Automarking turned on within a {:.2f} KM radius", global->Mark[client].AutoMarkRadius / 1000));
			}
			else if (!radiusString.empty())
			{
				CAccount const* acc = Players.FindAccountFromClientID(client);
				const auto fileName = Hk::Client::GetCharFileName(client);
				const auto dir = Hk::Client::GetAccountDirName(acc);
				if (fileName.has_value())
				{
					std::string userFile = CoreGlobals::c()->accPath + StringUtils::wstos(dir) + "\\flhookuser.ini";
					std::string section = "general_" + StringUtils::wstos(fileName.value());
					IniWrite(userFile, section, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				client.Message(std::format(L"Radius changed to {:.2f} KMs", radius));
			}
		}
		else
		{
			if (!radiusString.empty())
				global->Mark[client].AutoMarkRadius = radius * 1000;
			if (enabled == L"off") // AutoMark is being disabled
			{
				global->Mark[client].MarkEverything = false;
				CAccount const* acc = Players.FindAccountFromClientID(client);
				const auto fileName = Hk::Client::GetCharFileName(client);
				const auto dir = Hk::Client::GetAccountDirName(acc);
				if (fileName.has_value())
				{
					std::string userFile = CoreGlobals::c()->accPath + StringUtils::wstos(dir) + "\\flhookuser.ini";
					std::string section = "general_" + StringUtils::wstos(fileName.value());
					IniWrite(userFile, section, "automark", "no");
					if (!radiusString.empty())
						IniWrite(userFile, section, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				if (!radiusString.empty())
					client.Message(std::format(L"Automarking turned off; radius changed to {:.2f} KMs", global->Mark[client].AutoMarkRadius / 1000));
				else
					client.Message(L"Automarking turned off");
			}
			else if (!radiusString.empty())
			{
				CAccount const* acc = Players.FindAccountFromClientID(client);
				const auto fileName = Hk::Client::GetCharFileName(client);
				const auto dir = Hk::Client::GetAccountDirName(acc);
				if (fileName.has_value())
				{
					std::string userFile = CoreGlobals::c()->accPath + StringUtils::wstos(dir) + "\\flhookuser.ini";
					std::string section = "general_" + StringUtils::wstos(fileName.value());
					IniWrite(userFile, section, "automarkradius", std::to_string(global->Mark[client].AutoMarkRadius));
				}
				client.Message(std::format(L"Radius changed to {:.2f} KMs", radius));
			}
		}
	}
}