#include "Global.hpp"

#define RIGHT_CHECK(a)               \
	if (!(this->rights & a))         \
	{                                \
		Print("ERR No permission"); \
		return;                      \
	}
#define RIGHT_CHECK_SUPERADMIN()             \
	if (!(this->rights == RIGHT_SUPERADMIN)) \
	{                                        \
		Print("ERR No permission");         \
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

	Print(std::format("cash={}\nOK\n", res.value()));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetCash(const std::variant<uint, std::wstring>& player, uint iAmount)
{
	RIGHT_CHECK(RIGHT_CASH);

	const auto res = Hk::Player::GetCash(player);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Hk::Player::AdjustCash(player, iAmount - res.value());
	CmdGetCash(player);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdAddCash(const std::variant<uint, std::wstring>& player, uint amount)
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

	Print("OK");
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

	Print("Player banned");
	CmdKick(player, L"Player banned");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdTempBan(const std::variant<uint, std::wstring>& player, uint duration)
{
	if (!FLHookConfig::i()->general.tempBansEnabled)
	{
		Print("TempBan disabled");
		return;
	}

	RIGHT_CHECK(RIGHT_KICKBAN);

	if (const auto res = Hk::Player::TempBan(player, duration); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print("Player tempbanned");
	CmdKick(player, L"Player tempbanned");
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

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetClientID(const std::wstring& player)
{
	RIGHT_CHECK(RIGHT_OTHER);

	auto client = Hk::Client::GetClientIdFromCharName(player);
	if (client.has_error())
	{
		PrintError(client.error());
		return;
	}

	Print(std::format("clientid={}\nOK\n", client.value()));
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

	Print("OK");
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

	Print("OK");
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

	Print("OK");
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

	Print(std::format("feelings={}", res.value()));
	Print("OK");
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

	Print("OK");
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

	Print("OK");
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

	Print("OK");
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

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdFMsgS(const std::wstring& system, const std::wstring& xml)
{
	RIGHT_CHECK(RIGHT_MSG);

	uint systemId;
	pub::GetSystemID(systemId, wstos(system).c_str());
	if (!systemId)
	{
		Print("Invalid System");
		return;
	}

	if (const auto res = Hk::Message::FMsgS(systemId, xml); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print("OK");
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

	Print("OK");
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

	Print(std::format("remainingholdsize={}", holdSize));
	for (auto& item : cargo.value())
	{
		if (!item.bMounted)
			Print(std::format("id={} archid={} count={} mission={}", item.iId, item.iArchId, item.iCount, item.bMission ? 1 : 0));
	}

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRemoveCargo(const std::variant<uint, std::wstring>& player, ushort id, uint count)
{
	RIGHT_CHECK(RIGHT_CARGO);

	if (const auto res = Hk::Player::RemoveCargo(player, id, count); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print("OK");
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

	Print("OK");
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

	Print("OK");
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

	Print("OK");
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
		Print(wstos(line));
	}

	Print("OK");
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

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::PrintPlayerInfo(PlayerInfo& pi)
{
	RIGHT_CHECK(RIGHT_OTHER);

	Print(wstos(std::format(L"charname={} clientid={} ip={} host={} ping={} base={} system={}", pi.character, pi.client,
	    pi.wscIP, pi.wscHostname, pi.connectionInfo.dwRoundTripLatencyMS, pi.wscBase, pi.wscSystem)));
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

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::XPrintPlayerInfo(const PlayerInfo& pi)
{
	RIGHT_CHECK(RIGHT_OTHER);

	Print(wstos(std::format(L"Name: {}, Id: {}, IP: {}, Host: {}, Ping: {}, Base: {}, System: {}\n", pi.character, pi.client,
	    pi.wscIP, pi.wscHostname, pi.connectionInfo.dwRoundTripLatencyMS, pi.wscBase, pi.wscSystem)));
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

	for (const auto& p : Hk::Admin::GetPlayers())
		XPrintPlayerInfo(p);

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetPlayerIds()
{
	RIGHT_CHECK(RIGHT_OTHER);

	for (auto& p : Hk::Admin::GetPlayers())
	{
		Print(std::format("{} = {} | ", wstos(p.character), p.client));
	}

	Print("OK");
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

	Print(std::format("dirname={}\nOK", wstos(dir)));
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

	Print(std::format("filename={}", wstos(res.value())));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdIsOnServer(const std::wstring& player)
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
		Print("onserver=noOK\n");
	else
		Print("onserver=yesOK\n");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdMoneyFixList()
{
	RIGHT_CHECK(RIGHT_OTHER);

	PlayerData* playerDb = nullptr;
	while ((playerDb = Players.traverse_active(playerDb)))
	{
		ClientId client = playerDb->iOnlineId;

		if (ClientInfo[client].lstMoneyFix.size())
			Print(std::format("id={}", client));
	}

	Print("OK");
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
	int64 iTimeCreation = (((int64)ftCreation.dwHighDateTime) << 32) + ftCreation.dwLowDateTime;
	int64 iTimeNow = (((int64)ftNow.dwHighDateTime) << 32) + ftNow.dwLowDateTime;

	auto iUptime = static_cast<uint>((iTimeNow - iTimeCreation) / 10000000);
	uint iDays = (iUptime / (60 * 60 * 24));
	iUptime %= (60 * 60 * 24);
	uint iHours = (iUptime / (60 * 60));
	iUptime %= (60 * 60);
	uint iMinutes = (iUptime / 60);
	iUptime %= 60;
	uint iSeconds = iUptime;
	std::string time = std::format("{}:{}:{}:{}", iDays, iHours, iMinutes, iSeconds);

	// print
	Print(std::format(
	    "serverload={} npcspawn={} uptime={}\nOK\n", CoreGlobals::c()->serverLoadInMs, CoreGlobals::c()->disableNpcs ? "disabled" : "enabled", time));
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

	Print(std::format("groupsize={}", res.value().size()));
	for (auto& m : res.value())
		Print(std::format("id={} charname={}", m.client, wstos(m.character)));
	Print("OK");
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

	Print("OK");
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

	Print(std::format("reservedslot={}\nOK\n", res.value() ? "yes" : "no"));
	Print("OK");
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

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdSetAdmin(const std::variant<uint, std::wstring>& player, const std::wstring& playerRights)
{
	RIGHT_CHECK_SUPERADMIN();

	if (const auto res = Hk::Admin::SetAdmin(player, playerRights); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print("OK");
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

	Print(std::format("rights={}\nOK\n", wstos(res.value())));
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

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdLoadPlugins()
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	PluginManager::i()->loadAll(false, this);
	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdLoadPlugin(const std::wstring& wscPlugin)
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	PluginManager::i()->load(wscPlugin, this, false);
	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdReloadPlugin(const std::wstring& wscPlugin)
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	const auto unloadedPlugin = PluginManager::i()->unload(wstos(wscPlugin));
	if (unloadedPlugin.has_error())
	{
		PrintError(unloadedPlugin.error());
		return;
	}

	PluginManager::i()->load(unloadedPlugin.value(), this, false);
	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdListPlugins()
{
	RIGHT_CHECK(RIGHT_PLUGINS);

	for (const auto& data : PluginManager::ir())
		Print(std::format("{} ({}) - {}", data->name, data->shortName, !data->paused ? "running" : "paused"));

	Print("OK");
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

	Print("OK");
}

void CCmds::CmdShutdown()
{
	RIGHT_CHECK(RIGHT_SUPERADMIN);

	Print("Shutting down Server");

	// Kick everyone first and force a save
	PlayerData* playerDb = nullptr;
	while ((playerDb = Players.traverse_active(playerDb)))
	{
		Hk::Player::SaveChar(playerDb->iOnlineId);
		Hk::Player::Kick(playerDb->iOnlineId);
	}

	PostMessageA(GetFLServerHwnd(), WM_CLOSE, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Chase a player. Only works in system as you'd need a client hook to do across system */
void CCmds::CmdChase(std::wstring adminName, const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK_SUPERADMIN();

	if (const auto res = Hk::Admin::GetPlayerInfo(adminName, false);
		res.has_error())
	{
		PrintError(res.error());
		return;
	}

	const auto target = Hk::Admin::GetPlayerInfo(player, false);
	if (target.has_error() || target.value().ship == 0)
	{
		Print("ERR Player not found or not in space");
		return;
	}

	Vector pos;
	Matrix ornt;
	pub::SpaceObj::GetLocation(target.value().ship, pos, ornt);
	pos.y += 100;

	Print(std::format("Jump to system={} x={:.0f} y={:.0f} z={:.0f}", wstos(target.value().wscSystem), pos.x, pos.y, pos.z));
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Beam admin to a base. Works across systems but needs improvement of the path
 * selection algorithm */
void CCmds::CmdBeam(const std::variant<uint, std::wstring>& player, const std::wstring& targetBaseName)
{
	RIGHT_CHECK(RIGHT_BEAMKILL);

	try
	{
		if (Trim(targetBaseName).empty())
		{
			PrintError(Error::InvalidBaseName);
			return;
		}

		const auto base = Hk::Solar::GetBaseByWildcard(targetBaseName);
		if (base.has_error())
		{
			PrintError(base.error());
			return;
		}

		const auto res = Hk::Player::Beam(player, base.value()->baseId);
		if (res.has_error())
		{
			PrintError(res.error());
			return;
		}
	}
	catch (...)
	{ // exeption, kick player
		Hk::Player::Kick(player);
		Print("ERR exception occured, player kicked");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Pull a player to you. Only works in system as you'd need a client hook to move their system **/
void CCmds::CmdPull(std::wstring adminName, const std::variant<uint, std::wstring>& player)
{
	RIGHT_CHECK_SUPERADMIN();

	if (const auto res = Hk::Admin::GetPlayerInfo(adminName, false);
		res.has_error())
	{
		PrintError(res.error());
		return;
	}

	const auto target = Hk::Admin::GetPlayerInfo(player, false);
	if (target.has_error() || target.value().ship == 0)
	{
		Print("ERR Player not found or not in space");
		return;
	}

	Vector pos;
	Matrix ornt;
	pub::SpaceObj::GetLocation(target.value().ship, pos, ornt);
	pos.y += 400;

	Print(std::format("Jump to system={} x={:.2f} y={:.2f} z={:.2f}", wstos(target.value().wscSystem), pos.x, pos.y, pos.z));
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
	pub::SpaceObj::GetLocation(res.value().ship, pos, rot);
	pos.x = x;
	pos.y = y;
	pos.z = z;
	Print(std::format("Moving to {:.2f} {:.2f} {:.2f}", pos.x, pos.y, pos.z));
	Hk::Player::RelocateClient(res.value().client, pos, rot);
	return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdHelp()
{
	// TODO: Reimplement
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CCmds::ArgCharname(uint iArg)
{
	std::wstring wscArg = GetParam(wscCurCmdString, ' ', iArg);

	if (iArg == 1)
	{
		if (bId)
			return wscArg.replace(0, 0, L"id ");
		else if (bShortCut)
			return wscArg.replace(0, 0, L"sc ");
		else if (bSelf)
			return this->GetAdminName();
		else if (bTarget)
		{
			auto client = Hk::Client::GetClientIdFromCharName(this->GetAdminName());
			if (client.has_error())
				return L"";
			uint ship;
			pub::Player::GetShip(client.value(), ship);
			if (!ship)
				return L"";
			uint iTarget = Hk::Player::GetTarget(ship).value();
			if (!iTarget)
				return L"";
			auto targetId = Hk::Client::GetClientIdByShip(iTarget);
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
			auto client = Hk::Client::GetClientIdFromCharName(this->GetAdminName());
			if (client.has_error())
				return L"";
			uint ship;
			pub::Player::GetShip(client.value(), ship);
			if (!ship)
				return L"";
			uint iTarget = Hk::Player::GetTarget(ship).value();
			if (!iTarget)
				return L"";
			auto targetId = Hk::Client::GetClientIdByShip(iTarget);
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
	std::wstring wscAdminName = GetAdminName();

	if (wscAdminName.find(L"Socket connection") == 0)
	{
		bSocket = true;
	}

	try
	{
		if (bSocket)
			AddLog(LogType::SocketCmds, LogLevel::Info, std::format("{}: {}", wstos(wscAdminName).c_str(), wstos(wscCmdStr).c_str()));

		AddLog(LogType::AdminCmds, LogLevel::Info, std::format("{}: {}", wstos(wscAdminName).c_str(), wstos(wscCmdStr).c_str()));

		bId = false;
		bShortCut = false;
		bSelf = false;
		bTarget = false;
		wscCurCmdString = wscCmdStr;

		std::wstring wscCmd = ToLower(GetParam(wscCmdStr, ' ', 0));
		if (wscCmd.length() == 0)
		{
			Print("ERR unknown command");
			return;
		}

		size_t wscCmd_pos = wscCmdStr.find(wscCmd);

		if (wscCmd[wscCmd.length() - 1] == '$')
		{
			bId = true;
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
				CmdSetCash(ArgCharname(1), ArgUInt(2));
			}
			else if (wscCmd == L"addcash")
			{
				CmdAddCash(ArgCharname(1), ArgUInt(2));
			}
			else if (wscCmd == L"kick")
			{
				CmdKick(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (wscCmd == L"ban")
			{
				CmdBan(ArgCharname(1));
			}
			else if (wscCmd == L"tempban")
			{
				CmdTempBan(ArgCharname(1), ArgUInt(2));
			}
			else if (wscCmd == L"unban")
			{
				CmdUnban(ArgCharname(1));
			}
			else if (wscCmd == L"getclientid")
			{
				CmdGetClientID(ArgCharname(1));
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
				CmdRemoveCargo(ArgCharname(1), static_cast<ushort>(ArgInt(2)), ArgInt(3));
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
				CmdGetPlayerIds();
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
			else if (wscCmd == L"reloadplugin")
			{
				CmdReloadPlugin(ArgStrToEnd(1));
			}
			else if (wscCmd == L"loadplugin")
			{
				CmdLoadPlugin(ArgStrToEnd(1));
			}
			else if (wscCmd == L"shutdown")
			{
				CmdShutdown();
			}
			else if (wscCmd == L"listplugins")
			{
				CmdListPlugins();
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
				Print("ERR unknown command");
			}
		}
		if (bSocket)
		{
			AddLog(LogType::SocketCmds, LogLevel::Info, "finished");
		}
		else
		{
			AddLog(LogType::AdminCmds, LogLevel::Info, "finished");
		}
	}
	catch (...)
	{
		if (bSocket)
			AddLog(LogType::SocketCmds, LogLevel::Info, "exception");
		AddLog(LogType::AdminCmds, LogLevel::Info, "exception");
		Print("ERR exception occured");
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
	Print(std::format("ERR: {}", wstos(Hk::Err::ErrGetText(err))));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::Print(const std::string& text)
{
	DoPrint(text);
}
