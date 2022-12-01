#include "Global.hpp"

#define RIGHT_CHECK(a)               \
	if (!(this->rights & a))         \
	{                                \
		Print(L"ERR No permission"); \
		return;                      \
	}
#define RIGHT_CHECK_SUPERADMIN()             \
	if (!(this->rights == RIGHT_SUPERADMIN)) \
	{                                        \
		Print(L"ERR No permission");         \
		return;                              \
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetCash(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_CASH);

	const auto res = Hk::Player::GetCash(player);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"cash=%d\nOK\n", res.value());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetCash(const std::variant<uint, std::wstring>& player, int iAmount)
{
	RIGHT_CHECK(RIGHT_CASH);

	const auto res = Hk::Player::GetCash(player);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Hk::Player::AddCash(player, iAmount - res.value());
	CmdGetCash(player);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdAddCash(const std::variant<uint, std::wstring>& player, int amount)
{
	RIGHT_CHECK(RIGHT_CASH);

	if (const auto res = Hk::Player::AddCash(player, amount); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	CmdGetCash(player);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKick(const std::variant<uint, std::wstring>& player, const std::wstring& reason)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if (const auto res = Hk::Player::KickReason(player, reason); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdBan(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if (const auto res = Hk::Player::Ban(player, true); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"Player banned");
	CmdKick(player, L"Player banned");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnban(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_KICKBAN);

	if (const auto res = Hk::Player::Ban(player, false); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetClientId(const std::wstring& player)
{
	RIGHT_CHECK(RIGHT_OTHER);

	auto client = Hk::Client::GetClientIdFromCharName(player);
	if (client.has_error())
	{
		PrintError(client.error());
		return;
	}

	Print(L"clientid=%uOK\n", client);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdKill(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_BEAMKILL);

	if (const auto res = Hk::Player::Kill(player); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdResetRep(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_REPUTATION);

	if (const auto res = Hk::Player::ResetRep(player); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetRep(const std::variant<uint, std::wstring>& player, const std::wstring& repGroup, float value)
{
	RIGHT_CHECK(RIGHT_REPUTATION);

	if (const auto res = Hk::Player::SetRep(player, repGroup, value); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetRep(const std::variant<uint, std::wstring>& player, const std::wstring& repGroup)
{
	RIGHT_CHECK(RIGHT_REPUTATION);

	const auto res = Hk::Player::GetRep(player, repGroup);
	if ( res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"feelings=%f", res.value());
	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsg(const std::variant<uint, std::wstring>& player, const std::wstring& text)
{
	RIGHT_CHECK(RIGHT_MSG);

	if (const auto res = Hk::Message::Msg(player, text); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsgS(const std::wstring& system, const std::wstring& text)
{
	RIGHT_CHECK(RIGHT_MSG);

	if (const auto res = Hk::Message::MsgS(system, text); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMsgU(const std::wstring& text)
{
	RIGHT_CHECK(RIGHT_MSG);

	if (const auto res = Hk::Message::MsgU(text); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsg(const std::variant<uint, std::wstring>& player, const std::wstring& xml)
{
	RIGHT_CHECK(RIGHT_MSG);

	if (const auto res = Hk::Message::FMsg(player, xml); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsgS(const std::wstring& system, const std::wstring& xml)
{
	RIGHT_CHECK(RIGHT_MSG);

	if (const auto res = Hk::Message::FMsgS(system, xml); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsgU(const std::wstring& xml)
{
	RIGHT_CHECK(RIGHT_MSG);

	if (const auto res = Hk::Message::FMsgU(xml); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdEnumCargo(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_CARGO);

	int holdSize = 0;
	auto cargo = Hk::Player::EnumCargo(player, holdSize);
	if (cargo.has_error())
	{
		PrintError(cargo.error());	
	}

	Print(L"remainingholdsize=%d", holdSize);
	for (auto& item : cargo.value())
	{
		if (!item.bMounted)
			Print(L"id=%u archid=%u count=%d mission=%u", item.iID, item.iArchID, item.iCount, item.bMission ? 1 : 0);
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRemoveCargo(const std::variant<uint, std::wstring>& player, uint id, uint count)
{
	RIGHT_CHECK(RIGHT_CARGO);

	if (const auto res = Hk::Player::RemoveCargo(player, id, count); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdAddCargo(const std::variant<uint, std::wstring>& player, const std::wstring& good, uint count, bool mission)
{
	RIGHT_CHECK(RIGHT_CARGO);

	if (const auto res = Hk::Player::AddCargo(player, good, count, mission); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRename(const std::variant<uint, std::wstring>& player, const std::wstring& newName)
{
	RIGHT_CHECK(RIGHT_CHARACTERS);

	if (const auto res = Hk::Player::Rename(player, newName, false); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdDeleteChar(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_CHARACTERS);

	if (const auto res = Hk::Player::Rename(player, L"", true); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdReadCharFile(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_CHARACTERS);

	const auto res = Hk::Player::ReadCharFile(player); 
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	for (const auto& line : res.value())
	{
		Print(L"%s", line.c_str());
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdWriteCharFile(const std::variant<uint, std::wstring>& player, const std::wstring& data)
{
	RIGHT_CHECK(RIGHT_CHARACTERS);

	if (const auto res = Hk::Player::WriteCharFile(player, data); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::PrintPlayerInfo(PLAYERINFO& pi)
{
	RIGHT_CHECK(RIGHT_OTHER);

	Print(L"charname=%s clientid=%u ip=%s host=%s ping=%u base=%s system=%s", pi.character.c_str(), pi.clientId,
	    pi.wscIP.c_str(), pi.wscHostname.c_str(), pi.connectionInfo.dwRoundTripLatencyMS, pi.wscBase.c_str(), pi.wscSystem.c_str());
}

void CCmds::CmdGetPlayerInfo(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_OTHER);

	const auto res = Hk::Admin::GetPlayerInfo(player, false);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	auto info = res.value();
	PrintPlayerInfo(info);
}

void CCmds::CmdGetPlayers()
{
	RIGHT_CHECK(RIGHT_OTHER);

	for (auto& p : Hk::Admin::GetPlayers())
		PrintPlayerInfo(p);

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::XPrintPlayerInfo(const PLAYERINFO& pi)
{
	RIGHT_CHECK(RIGHT_OTHER);

	Print(L"Name: %s, ID: %u, IP: %s, Host: %s, Ping: %u, Base: %s, System: %s\n", pi.character.c_str(), pi.clientId,
	    pi.wscIP.c_str(), pi.wscHostname.c_str(), pi.connectionInfo.dwRoundTripLatencyMS, pi.wscBase.c_str(), pi.wscSystem.c_str());
}

void CCmds::CmdXGetPlayerInfo(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_OTHER);

	const auto res = Hk::Admin::GetPlayerInfo(player, false);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	XPrintPlayerInfo(res.value());
}

void CCmds::CmdXGetPlayers()
{
	RIGHT_CHECK(RIGHT_OTHER);

	for (auto& p : Hk::Admin::GetPlayers())
		XPrintPlayerInfo(p);

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetPlayerIDs()
{
	RIGHT_CHECK(RIGHT_OTHER);

	wchar_t wszLine[128] = L"";
	for (auto& p : Hk::Admin::GetPlayers())
	{
		wchar_t wszBuf[1024];
		swprintf_s(wszBuf, L"%s = %u | ", p.character.c_str(), p.clientId);
		if ((wcslen(wszBuf) + wcslen(wszLine)) >= sizeof(wszLine) / 2)
		{
			Print(L"%s", wszLine);
			wcscpy_s(wszLine, wszBuf);
		}
		else
			wcscat_s(wszLine, wszBuf);
	}

	if (wcslen(wszLine))
		Print(L"%s", wszLine);
	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAccountDirName(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_OTHER);

	const auto acc = Hk::Client::GetAccountByCharName(std::get<std::wstring>(player));
	if (acc.has_error())
	{
		PrintError(acc.error());
		return;
	}

	auto dir = Hk::Client::GetAccountDirName(acc.value());

	Print(L"dirname=%s\nOK", dir.c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetCharFileName(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_OTHER);

	const auto res = Hk::Client::GetCharFileName(player);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"filename=%s", res.value());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdIsOnServer(std::wstring player)
{
	RIGHT_CHECK(RIGHT_OTHER);

	const auto res = Hk::Client::GetAccountByCharName(player);
	if ( res.has_error())
	{
		PrintError(res.error());
		return;
	}

	const auto id = Hk::Client::GetClientIdFromAccount(res.value());
	if (id.has_error())
		Print(L"onserver=noOK\n");
	else
		Print(L"onserver=yesOK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMoneyFixList()
{
	RIGHT_CHECK(RIGHT_OTHER);

	struct PlayerData* playerDb = 0;
	while (playerDb = Players.traverse_active(playerDb))
	{
		uint clientId = playerDb->iOnlineID;

		if (ClientInfo[clientId].lstMoneyFix.size())
			Print(L"id=%u", clientId);
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdServerInfo()
{
	RIGHT_CHECK(RIGHT_OTHER);

	// calculate uptime
	FILETIME ftCreation;
	FILETIME ft;
	GetProcessTimes(GetCurrentProcess(), &ftCreation, &ft, &ft, &ft);
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILETIME ftNow;
	SystemTimeToFileTime(&st, &ftNow);
	__int64 iTimeCreation = (((__int64)ftCreation.dwHighDateTime) << 32) + ftCreation.dwLowDateTime;
	__int64 iTimeNow = (((__int64)ftNow.dwHighDateTime) << 32) + ftNow.dwLowDateTime;

	uint iUptime = (uint)((iTimeNow - iTimeCreation) / 10000000);
	uint iDays = (iUptime / (60 * 60 * 24));
	iUptime %= (60 * 60 * 24);
	uint iHours = (iUptime / (60 * 60));
	iUptime %= (60 * 60);
	uint iMinutes = (iUptime / 60);
	iUptime %= (60);
	uint iSeconds = iUptime;
	wchar_t wszUptime[16];
	swprintf_s(wszUptime, L"%.1u:%.2u:%.2u:%.2u", iDays, iHours, iMinutes, iSeconds);

	// print
	Print(L"serverload=%d npcspawn=%s uptime=%sOK\n", g_iServerLoad, g_bNPCDisabled ? L"disabled" : L"enabled", wszUptime);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetGroupMembers(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_OTHER);

	const auto res = Hk::Player::GetGroupMembers(player);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"groupsize=%d", res.value().size());
	for (auto& m : res.value())
		Print(L"id=%d charname=%s", m.clientId, m.character.c_str());
	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSaveChar(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_OTHER);

	if (const auto res = Hk::Player::SaveChar(player); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetReservedSlot(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK(RIGHT_SETTINGS);

	const auto res = Hk::Player::GetReservedSlot(player); 
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"reservedslot=%sOK\n", res.value() ? L"yes" : L"no");
	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetReservedSlot(const std::variant<uint, std::wstring>& player, int reservedSlot)
{
	RIGHT_CHECK(RIGHT_SETTINGS);

	if (const auto res = Hk::Player::SetReservedSlot(player, reservedSlot ? true : false); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetAdmin(const std::variant<uint, std::wstring>& player, const std::wstring& rights)
{
	RIGHT_CHECK_SUPERADMIN();

	if (const auto res = Hk::Admin::SetAdmin(player, rights); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAdmin(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK_SUPERADMIN();

	const auto res = Hk::Admin::GetAdmin(player);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"rights=%s\nOK\n", res.value().c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdDelAdmin(const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK_SUPERADMIN();

	if (const auto res = Hk::Admin::DelAdmin(player); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdLoadPlugins()
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	PluginManager::i()->loadAll(false, this);
	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdLoadPlugin(const std::wstring& wscPlugin)
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	PluginManager::i()->load(wscPlugin, this, false);
	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdListPlugins()
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	for (const auto& data : PluginManager::ir())
		Print(L"%s (%s) - %s", stows(data.name).c_str(), stows(data.shortName).c_str(), !data.paused ? L"running" : L"paused");

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnloadPlugin(const std::wstring& wscPlugin)
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	if (const auto res = PluginManager::i()->unload(wstos(wscPlugin)); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRehash()
{
	RIGHT_CHECK(RIGHT_SETTINGS);

	LoadSettings();
	CallPluginsAfter(HookedCall::FLHook__LoadSettings);

	HookRehashed();
	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Chase a player. Only works in system as you'd need a client hook to do across system */
void CCmds::CmdChase(std::wstring adminName, const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK_SUPERADMIN();

	const auto res = Hk::Admin::GetPlayerInfo(adminName, false);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	const auto target = Hk::Admin::GetPlayerInfo(player, false);
	if (target.has_error() || target.value().iShip == 0)
	{
		Print(L"ERR Player not found or not in space");
		return;
	}

	Vector pos;
	Matrix ornt;
	pub::SpaceObj::GetLocation(target.value().iShip, pos, ornt);
	pos.y += 100;

	Print(L"Jump to system=%s x=%0.0f y=%0.0f z=%0.0f", target.value().wscSystem.c_str(), pos.x, pos.y, pos.z);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Beam admin to a base. Works across systems but needs improvement of the path
 * selection algorithm */
void CCmds::CmdBeam(const std::variant<uint, std::wstring>& player, const std::wstring& targetBaseName)
{
	RIGHT_CHECK(RIGHT_BEAMKILL);

	// Fall back to default flhook .beam command
	try
	{
		const auto res = Hk::Player::Beam(player, targetBaseName);
		if (res.has_error())
		{
			PrintError(res.error());
			return;
		}
		else
		{

			const auto res = Hk::Admin::GetPlayerInfo(player, false);
			if (res.has_error())
			{
				PrintError(res.error());
				return;
			}

			if (res.value().iShip == 0)
			{
				Print(L"ERR Player not in space");
				return;
			}

			// Search for an exact match at the start of the name
			const struct Universe::IBase* baseinfo = Universe::GetFirstBase();
			while (baseinfo)
			{
				std::wstring basename = Hk::Message::GetWStringFromIDS(baseinfo->baseIdS);
				if (ToLower(basename).find(ToLower(targetBaseName)) == 0)
				{
					pub::Player::ForceLand(res.value().clientId, baseinfo->baseId);
					if (res.value().iSystem != baseinfo->systemId)
					{
						Server.BaseEnter(baseinfo->baseId, res.value().clientId);
						Server.BaseExit(baseinfo->baseId, res.value().clientId);
						auto charFileName = Hk::Client::GetCharFileName(res.value().character);
						if (charFileName.has_error()) 
						{
							return;
						}

						const auto fileName = charFileName.value() + L".fl";
						CHARACTER_ID cID;
						strcpy(cID.szCharFilename, wstos(fileName.substr(0, 14)).c_str());
						Server.CharacterSelect(cID, res.value().clientId);
					}
					return;
				}
				baseinfo = Universe::GetNextBase();
			}

			// Exact match failed, try a for an partial match
			baseinfo = Universe::GetFirstBase();
			while (baseinfo)
			{
				std::wstring basename = Hk::Message::GetWStringFromIDS(baseinfo->baseIdS);
				if (ToLower(basename).find(ToLower(targetBaseName)) != -1)
				{
					pub::Player::ForceLand(res.value().clientId, baseinfo->baseId);
					if (res.value().iSystem != baseinfo->systemId)
					{
						Server.BaseEnter(baseinfo->baseId, res.value().clientId);
						Server.BaseExit(baseinfo->baseId, res.value().clientId);
						auto charFileName = Hk::Client::GetCharFileName(res.value().character);
						if (charFileName.has_error())
						{
							return;
						}

						const auto fileName = charFileName.value() + L".fl";
						CHARACTER_ID cID;
						strcpy(cID.szCharFilename, wstos(fileName.substr(0, 14)).c_str());
						Server.CharacterSelect(cID, res.value().clientId);
					}
					return;
				}
				baseinfo = Universe::GetNextBase();
			}
		}
	}
	catch (...)
	{ // exeption, kick player
		Hk::Player::Kick(player);
		Print(L"ERR exception occured, player kicked");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Pull a player to you. Only works in system as you'd need a client hook to move their system **/
void CCmds::CmdPull(std::wstring adminName, const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK_SUPERADMIN();

	const auto res = Hk::Admin::GetPlayerInfo(adminName, false);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	const auto target = Hk::Admin::GetPlayerInfo(player, false);
	if (target.has_error() || target.value().iShip == 0)
	{
		Print(L"ERR Player not found or not in space");
		return;
	}

	Vector pos;
	Matrix ornt;
	pub::SpaceObj::GetLocation(target.value().iShip, pos, ornt);
	pos.y += 400;

	Print(L"Jump to system=%s x=%0.0f y=%0.0f z=%0.0f", target.value().wscSystem.c_str(), pos.x, pos.y, pos.z);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Move to location */
void CCmds::CmdMove(std::wstring adminName, float x, float y, float z)
{
	RIGHT_CHECK_SUPERADMIN();

	const auto res = Hk::Admin::GetPlayerInfo(adminName, false);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Vector pos;
	Matrix rot;
	pub::SpaceObj::GetLocation(res.value().iShip, pos, rot);
	pos.x = x;
	pos.y = y;
	pos.z = z;
	Print(L"Moving to %0.0f %0.0f %0.0f", pos.x, pos.y, pos.z);
	Hk::Player::RelocateClient(res.value().clientId, pos, rot);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdHelp()
{
	std::wstring wszHelpMsg = std::wstring(L"[version]\n") + VersionInformation +
	    L"\n"
	    L"[commands]\n"
	    L"getcash <charname>\n"
	    L"setcash <charname> <amount>\n"
	    L"addcash <charname> <amount>\n"
	    L"kick <charname> <reason>\n"
	    L"ban <charname>\n"
	    L"unban <charname>\n"
	    L"beam <charname> <basename>\n"
	    L"pull <charname>\n"
	    L"chase <charname>\n"
	    L"move <x> <y> <z>\n"
	    L"kill <charname>\n"
	    L"resetrep <charname>\n"
	    L"setrep <charname> <repgroup> <value>\n"
	    L"getrep <charname> <repgroup>\n"
	    L"readcharfile <charname>\n"
	    L"writecharfile <charname> <data>\n"
	    L"enumcargo <charname>\n"
	    L"addcargo <charname> <good> <count> <mission>\n"
	    L"removecargo <charname> <id> <count>\n"
	    L"rename <oldcharname> <newcharname>\n"
	    L"deletechar <charname>\n"
	    L"msg <charname> <text>\n"
	    L"msgs <systemname> <text>\n"
	    L"msgu <text>\n"
	    L"fmsg <charname> <xmltext>\n"
	    L"fmsgs <systemname> <xmltext>\n"
	    L"fmsgu <xmltext>\n"
	    L"enumcargo <charname>\n"
	    L"addcargo <charname> <good> <count> <mission>\n"
	    L"removecargo <charname> <id> <count>\n"
	    L"getgroupmembers <charname>\n"
	    L"getbasestatus <basename>\n"
	    L"getclientid <charname>\n"
	    L"getplayerinfo <charname>\n"
	    L"getplayers\n"
	    L"xgetplayerinfo <charname>\n"
	    L"xgetplayers\n"
	    L"getplayerids\n"
	    L"help\n"
	    L"getaccountdirname <charname>\n"
	    L"getcharfilename <charname>\n"
	    L"isonserver <charname>\n"
	    L"serverinfo\n"
	    L"moneyfixlist\n"
	    L"savechar <charname>\n"
	    L"setadmin <charname> <rights>\n"
	    L"getadmin <charname>\n"
	    L"deladmin <charname>\n"
	    L"getreservedslot <charname>\n"
	    L"setreservedslot <charname> <value>\n"
	    L"loadplugins\n"
	    L"loadplugin <plugin filename>\n"
	    L"listplugins\n"
	    L"unloadplugin <plugin shortname>\n"
	    L"pauseplugin <plugin shortname>\n"
	    L"unpauseplugin <plugin shortname>\n"
	    L"rehash\n";

	CallPluginsBefore(HookedCall::FLHook__AdminCommand__Help, this);

	Print(L"%s", wszHelpMsg.c_str());

	CallPluginsAfter(HookedCall::FLHook__AdminCommand__Help, this);

	Print(L"OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CCmds::ArgCharname(uint iArg)
{
	std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

	if (iArg == 1)
	{
		if (bID)
			return wscArg.replace((int)0, (int)0, L"id ");
		else if (bShortCut)
			return wscArg.replace((int)0, (int)0, L"sc ");
		else if (bSelf)
			return this->GetAdminName();
		else if (bTarget)
		{
			auto clientId = Hk::Client::GetClientIdFromCharName(this->GetAdminName());
			if (clientId.has_error())
				return L"";
			uint iShip;
			pub::Player::GetShip(clientId.value(), iShip);
			if (!iShip)
				return L"";
			uint iTarget;
			pub::SpaceObj::GetTarget(iShip, iTarget);
			if (!iTarget)
				return L"";
			auto targetId = Hk::Client::GetClientIDByShip(iTarget);
			if (!targetId.has_error())
				return L"";
			return L"id " + std::to_wstring(targetId.value());
		}
	}

	{
		if (wscArg == L">s")
			return this->GetAdminName();
		else if (wscArg.find(L">i") == 0)
			return L"id " + wscArg.substr(2);
		else if (wscArg == L">t")
		{
			auto clientId = Hk::Client::GetClientIdFromCharName(this->GetAdminName());
			if (clientId.has_error())
				return L"";
			uint iShip;
			pub::Player::GetShip(clientId.value(), iShip);
			if (!iShip)
				return L"";
			uint iTarget;
			pub::SpaceObj::GetTarget(iShip, iTarget);
			if (!iTarget)
				return L"";
			auto targetId = Hk::Client::GetClientIDByShip(iTarget);
			if (!targetId.has_error())
				return L"";
			return L"id " + std::to_wstring(targetId.value());
		}
		else
			return wscArg;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCmds::ArgInt(uint iArg)
{
	std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

	return ToInt(wscArg);
}

uint CCmds::ArgUInt(uint iArg)
{
	std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

	return ToUInt(wscArg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float CCmds::ArgFloat(uint iArg)
{
	std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);
	return ToFloat(wscArg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CCmds::ArgStr(uint iArg)
{
	std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

	return wscArg;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CCmds::ArgStrToEnd(uint iArg)
{
	for (uint i = 0, iCurArg = 0; (i < wscCurCmdString.length()); i++)
	{
		if (wscCurCmdString[i] == ' ')
		{
			iCurArg++;

			if (iCurArg == iArg)
				return wscCurCmdString.substr(i + 1);

			while (((i + 1) < wscCurCmdString.length()) && (wscCurCmdString[i + 1] == ' '))
				i++; // skip "whitechar"
		}
	}

	return L"";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::ExecuteCommandString(const std::wstring& wscCmdStr)
{
	// check if command was sent by a socket connection
	bool bSocket = false;
	bool bLocalSocket = false;
	std::wstring wscAdminName = GetAdminName();

	if (wscAdminName.find(L"Socket connection") == 0)
	{
		bSocket = true;
		if (wscAdminName.find(L"127.0.0.1") != std::wstring::npos)
			bLocalSocket = true;
	}

	try
	{
		if (bSocket)
			AddLog(LogType::SocketCmds, LogLevel::Info, L"%s: %s", wscAdminName.c_str(), wscCmdStr.c_str());

		AddLog(LogType::AdminCmds, LogLevel::Info, L"%s: %s", wscAdminName.c_str(), wscCmdStr.c_str());

		bID = false;
		bShortCut = false;
		bSelf = false;
		bTarget = false;
		wscCurCmdString = wscCmdStr;

		std::wstring wscCmd = ToLower(GetParam(wscCmdStr, ' ', 0));
		if (wscCmd.length() == 0)
		{
			Print(L"ERR unknown command");
			return;
		}

		size_t wscCmd_pos = wscCmdStr.find(wscCmd);

		if (wscCmd[wscCmd.length() - 1] == '$')
		{
			bID = true;
			wscCmd.erase(wscCmd.length() - 1, 1);
		}
		else if (wscCmd[wscCmd.length() - 1] == '&')
		{
			bShortCut = true;
			wscCmd.erase(wscCmd.length() - 1, 1);
		}
		else if (wscCmd[wscCmd.length() - 1] == '!')
		{
			bSelf = true;
			wscCurCmdString.insert(wscCmd_pos + wscCmd.length() - 1, L" ");
			wscCmd.erase(wscCmd.length() - 1, 1);
		}
		else if (wscCmd[wscCmd.length() - 1] == '?')
		{
			bTarget = true;
			wscCurCmdString.insert(wscCmd_pos + wscCmd.length() - 1, L" ");
			wscCmd.erase(wscCmd.length() - 1, 1);
		}

		if (const bool plugins = CallPluginsBefore(HookedCall::FLHook__AdminCommand__Process, this, wscCmd); !plugins)
		{
			if (wscCmd == L"getcash")
			{
				CmdGetCash(ArgCharname(1));
			}
			else if (wscCmd == L"setcash")
			{
				CmdSetCash(ArgCharname(1), ArgInt(2));
			}
			else if (wscCmd == L"addcash")
			{
				CmdAddCash(ArgCharname(1), ArgInt(2));
			}
			else if (wscCmd == L"kick")
			{
				CmdKick(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (wscCmd == L"ban")
			{
				CmdBan(ArgCharname(1));
			}
			else if (wscCmd == L"unban")
			{
				CmdUnban(ArgCharname(1));
			}
			else if (wscCmd == L"getclientid")
			{
				CmdGetClientId(ArgCharname(1));
			}
			else if (wscCmd == L"beam")
			{
				CmdBeam(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (wscCmd == L"kill")
			{
				CmdKill(ArgCharname(1));
			}
			else if (wscCmd == L"resetrep")
			{
				CmdResetRep(ArgCharname(1));
			}
			else if (wscCmd == L"setrep")
			{
				CmdSetRep(ArgCharname(1), ArgStr(2), ArgFloat(3));
			}
			else if (wscCmd == L"getrep")
			{
				CmdGetRep(ArgCharname(1), ArgStr(2));
			}
			else if (wscCmd == L"msg")
			{
				CmdMsg(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (wscCmd == L"msgs")
			{
				CmdMsgS(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (wscCmd == L"msgu")
			{
				CmdMsgU(ArgStrToEnd(1));
			}
			else if (wscCmd == L"fmsg")
			{
				CmdFMsg(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (wscCmd == L"fmsgs")
			{
				CmdFMsgS(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (wscCmd == L"fmsgu")
			{
				CmdFMsgU(ArgStrToEnd(1));
			}
			else if (wscCmd == L"enumcargo")
			{
				CmdEnumCargo(ArgCharname(1));
			}
			else if (wscCmd == L"removecargo")
			{
				CmdRemoveCargo(ArgCharname(1), ArgInt(2), ArgInt(3));
			}
			else if (wscCmd == L"addcargo")
			{
				CmdAddCargo(ArgCharname(1), ArgStr(2), ArgInt(3), ArgInt(4));
			}
			else if (wscCmd == L"rename")
			{
				CmdRename(ArgCharname(1), ArgStr(2));
			}
			else if (wscCmd == L"deletechar")
			{
				CmdDeleteChar(ArgCharname(1));
			}
			else if (wscCmd == L"readcharfile")
			{
				CmdReadCharFile(ArgCharname(1));
			}
			else if (wscCmd == L"writecharfile")
			{
				CmdWriteCharFile(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (wscCmd == L"getplayerinfo")
			{
				CmdGetPlayerInfo(ArgCharname(1));
			}
			else if (wscCmd == L"getplayers")
			{
				CmdGetPlayers();
			}
			else if (wscCmd == L"xgetplayerinfo")
			{
				CmdXGetPlayerInfo(ArgCharname(1));
			}
			else if (wscCmd == L"xgetplayers")
			{
				CmdXGetPlayers();
			}
			else if (wscCmd == L"getplayerids")
			{
				CmdGetPlayerIDs();
			}
			else if (wscCmd == L"getaccountdirname")
			{
				CmdGetAccountDirName(ArgCharname(1));
			}
			else if (wscCmd == L"getcharfilename")
			{
				CmdGetCharFileName(ArgCharname(1));
			}
			else if (wscCmd == L"savechar")
			{
				CmdSaveChar(ArgCharname(1));
			}
			else if (wscCmd == L"isonserver")
			{
				CmdIsOnServer(ArgCharname(1));
			}
			else if (wscCmd == L"moneyfixlist")
			{
				CmdMoneyFixList();
			}
			else if (wscCmd == L"serverinfo")
			{
				CmdServerInfo();
			}
			else if (wscCmd == L"getgroupmembers")
			{
				CmdGetGroupMembers(ArgCharname(1));
			}
			else if (wscCmd == L"getreservedslot")
			{
				CmdGetReservedSlot(ArgCharname(1));
			}
			else if (wscCmd == L"setreservedslot")
			{
				CmdSetReservedSlot(ArgCharname(1), ArgInt(2));
			}
			else if (wscCmd == L"setadmin")
			{
				CmdSetAdmin(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (wscCmd == L"getadmin")
			{
				CmdGetAdmin(ArgCharname(1));
			}
			else if (wscCmd == L"deladmin")
			{
				CmdDelAdmin(ArgCharname(1));
			}
			else if (wscCmd == L"unloadplugin")
			{
				CmdUnloadPlugin(ArgStrToEnd(1));
			}
			else if (wscCmd == L"loadplugins")
			{
				CmdLoadPlugins();
			}
			else if (wscCmd == L"loadplugin")
			{
				CmdLoadPlugin(ArgStrToEnd(1));
			}
			else if (wscCmd == L"listplugins")
			{
				CmdListPlugins();
			}
			else if (wscCmd == L"rehash")
			{
				CmdRehash();
			}
			else if (wscCmd == L"help")
			{
				CmdHelp();
			}
			else if (wscCmd == L"move")
			{
				CmdMove(wscAdminName, ArgFloat(1), ArgFloat(2), ArgFloat(3));
			}
			else if (wscCmd == L"chase")
			{
				CmdChase(wscAdminName, ArgCharname(1));
			}
			else if (wscCmd == L"beam")
			{
				CmdBeam(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (wscCmd == L"pull")
			{
				CmdPull(wscAdminName, ArgCharname(1));
			}
			else
			{
				Print(L"ERR unknown command");
			}
		}
		if (bSocket)
		{
			AddLog(LogType::SocketCmds, LogLevel::Info, L"finished");
		}
		else
		{
			AddLog(LogType::AdminCmds, LogLevel::Info, L"finished");
		}
	}
	catch (...)
	{
		if (bSocket)
			AddLog(LogType::SocketCmds, LogLevel::Info, L"exception");
		AddLog(LogType::AdminCmds, LogLevel::Info, L"exception");
		Print(L"ERR exception occured");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::SetRightsByString(const std::string& scRights)
{
	rights = RIGHT_NOTHING;
	std::string scRightStr = ToLower(scRights);
	if (scRightStr.find("superadmin") != -1)
		rights |= RIGHT_SUPERADMIN;
	if (scRightStr.find("cash") != -1)
		rights |= RIGHT_CASH;
	if (scRightStr.find("kickban") != -1)
		rights |= RIGHT_KICKBAN;
	if (scRightStr.find("beamkill") != -1)
		rights |= RIGHT_BEAMKILL;
	if (scRightStr.find("msg") != -1)
		rights |= RIGHT_MSG;
	if (scRightStr.find("other") != -1)
		rights |= RIGHT_OTHER;
	if (scRightStr.find("cargo") != -1)
		rights |= RIGHT_CARGO;
	if (scRightStr.find("characters") != -1)
		rights |= RIGHT_CHARACTERS;
	if (scRightStr.find("settings") != -1)
		rights |= RIGHT_SETTINGS;
	if (scRightStr.find("reputation") != -1)
		rights |= RIGHT_REPUTATION;
	if (scRightStr.find("plugins") != -1)
		rights |= RIGHT_PLUGINS;
	if (scRightStr.find("eventmode") != -1)
		rights |= RIGHT_EVENTMODE;
	if (scRightStr.find("special1") != -1)
		rights |= RIGHT_SPECIAL1;
	if (scRightStr.find("special2") != -1)
		rights |= RIGHT_SPECIAL2;
	if (scRightStr.find("special3") != -1)
		rights |= RIGHT_SPECIAL3;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::PrintError(Error err)
{
	Print(L"ERR %s", Hk::Err::ErrGetText(err).c_str());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::Print(std::wstring text, ...)
{
	wchar_t wszBuf[1024 * 8] = L"";
	va_list marker;
	va_start(marker, text);

	_vsnwprintf_s(wszBuf, sizeof wszBuf / 2 - 1, text.c_str(), marker);

	DoPrint(wszBuf);
}
