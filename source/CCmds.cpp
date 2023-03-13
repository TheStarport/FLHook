#include "Global.hpp"

#define RIGHT_CHECK(a)              \
	if (!(this->rights & a))        \
	{                               \
		Print("ERR No permission"); \
		return;                     \
	}
#define RIGHT_CHECK_SUPERADMIN()             \
	if (!(this->rights == RIGHT_SUPERADMIN)) \
	{                                        \
		Print("ERR No permission");          \
		return;                              \
	}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetCash(const std::variant<uint, std::wstring>& player)
{
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
	const auto res = Hk::Player::GetRep(player, repGroup);
	if (res.has_error())
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
	int holdSize = 0;
	auto cargo = Hk::Player::EnumCargo(player, holdSize);
	if (cargo.has_error())
	{
		PrintError(cargo.error());
	}

	Print(std::format("remainingholdsize={}", holdSize));
	for (auto& item : cargo.value())
	{
		if (!item.mounted)
			Print(std::format("id={} archid={} count={} mission={}", item.id, item.archId, item.count, item.mission ? 1 : 0));
	}

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdRemoveCargo(const std::variant<uint, std::wstring>& player, ushort id, uint count)
{
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
	Print(wstos(std::format(L"charname={} clientid={} ip={} host={} ping={} base={} system={}",
	    pi.character,
	    pi.client,
	    pi.IP,
	    pi.Hostname,
	    pi.connectionInfo.dwRoundTripLatencyMS,
	    pi.Base,
	    pi.System)));
}

void CCmds::CmdGetPlayerInfo(const std::variant<uint, std::wstring>& player)
{
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
	for (auto& p : Hk::Admin::GetPlayers())
		PrintPlayerInfo(p);

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::XPrintPlayerInfo(const PlayerInfo& pi)
{
	Print(wstos(std::format(L"Name: {}, Id: {}, IP: {}, Host: {}, Ping: {}, Base: {}, System: {}\n",
	    pi.character,
	    pi.client,
	    pi.IP,
	    pi.Hostname,
	    pi.connectionInfo.dwRoundTripLatencyMS,
	    pi.Base,
	    pi.System)));
}

void CCmds::CmdXGetPlayerInfo(const std::variant<uint, std::wstring>& player)
{
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
	for (const auto& p : Hk::Admin::GetPlayers())
		XPrintPlayerInfo(p);

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetPlayerIds()
{
	for (auto& p : Hk::Admin::GetPlayers())
	{
		Print(std::format("{} = {} | ", wstos(p.character), p.client));
	}

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetAccountDirName(const std::variant<uint, std::wstring>& player)
{
	const auto acc = Hk::Client::GetAccountByCharName(std::get<std::wstring>(player));
	if (acc.has_error())
	{
		PrintError(acc.error());
		return;
	}

	const auto dir = Hk::Client::GetAccountDirName(acc.value());

	Print(std::format("dirname={}\nOK", wstos(dir)));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdGetCharFileName(const std::variant<uint, std::wstring>& player)
{
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
	const auto res = Hk::Client::GetAccountByCharName(player);
	if (res.has_error())
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
	PlayerData* playerDb = nullptr;
	while ((playerDb = Players.traverse_active(playerDb)))
	{
		ClientId client = playerDb->onlineId;

		if (ClientInfo[client].MoneyFix.size())
			Print(std::format("id={}", client));
	}

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdServerInfo()
{
	// calculate uptime
	FILETIME ftCreation;
	FILETIME ft;
	GetProcessTimes(GetCurrentProcess(), &ftCreation, &ft, &ft, &ft);
	SYSTEMTIME st;
	GetSystemTime(&st);
	FILETIME ftNow;
	SystemTimeToFileTime(&st, &ftNow);
	const int64 iTimeCreation = (static_cast<int64>(ftCreation.dwHighDateTime) << 32) + ftCreation.dwLowDateTime;
	const int64 iTimeNow = (static_cast<int64>(ftNow.dwHighDateTime) << 32) + ftNow.dwLowDateTime;

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
	PluginManager::i()->loadAll(false, this);
	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdLoadPlugin(const std::wstring& Plugin)
{
	PluginManager::i()->load(Plugin, this, false);
	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdReloadPlugin(const std::wstring& Plugin)
{
	const auto unloadedPlugin = PluginManager::i()->unload(wstos(Plugin));
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
	for (const auto& data : PluginManager::ir())
		Print(std::format("{} ({}) - {}", data->name, data->shortName, !data->paused ? "running" : "paused"));

	Print("OK");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdUnloadPlugin(const std::wstring& Plugin)
{
	if (const auto res = PluginManager::i()->unload(wstos(Plugin)); res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Print("OK");
}

void CCmds::CmdShutdown()
{
	Print("Shutting down Server");

	// Kick everyone first and force a save
	PlayerData* playerDb = nullptr;
	while ((playerDb = Players.traverse_active(playerDb)))
	{
		Hk::Player::SaveChar(playerDb->onlineId);
		Hk::Player::Kick(playerDb->onlineId);
	}

	PostMessageA(GetFLServerHwnd(), WM_CLOSE, 0, 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Chase a player. Only works in system as you'd need a client hook to do across system */
void CCmds::CmdChase(std::wstring adminName, const std::variant<uint, std::wstring>& player)
{
	if (const auto res = Hk::Admin::GetPlayerInfo(adminName, false); res.has_error())
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

	Print(std::format("Jump to system={} x={:.0f} y={:.0f} z={:.0f}", wstos(target.value().System), pos.x, pos.y, pos.z));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Beam admin to a base. Works across systems but needs improvement of the path
 * selection algorithm */
void CCmds::CmdBeam(const std::variant<uint, std::wstring>& player, const std::wstring& targetBaseName)
{
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
		}
	}
	catch (...)
	{
		// exeption, kick player
		Hk::Player::Kick(player);
		Print("ERR exception occured, player kicked");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Pull a player to you. Only works in system as you'd need a client hook to move their system **/
void CCmds::CmdPull(std::wstring adminName, const std::variant<uint, std::wstring>& player)
{
	if (const auto res = Hk::Admin::GetPlayerInfo(adminName, false); res.has_error())
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

	Print(std::format("Jump to system={} x={:.2f} y={:.2f} z={:.2f}", wstos(target.value().System), pos.x, pos.y, pos.z));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Move to location */
void CCmds::CmdMove(std::wstring adminName, float x, float y, float z)
{
	const auto res = Hk::Admin::GetPlayerInfo(adminName, false);
	if (res.has_error())
	{
		PrintError(res.error());
		return;
	}

	Vector pos {};
	Matrix rot {};
	pub::SpaceObj::GetLocation(res.value().ship, pos, rot);
	pos.x = x;
	pos.y = y;
	pos.z = z;
	Print(std::format("Moving to {:.2f} {:.2f} {:.2f}", pos.x, pos.y, pos.z));
	Hk::Player::RelocateClient(res.value().client, pos, rot);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::CmdHelp()
{
	// TODO: Reimplement
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CCmds::ArgCharname(uint iArg)
{
	std::wstring Arg = GetParam(CurCmdString, ' ', iArg);

	if (iArg == 1)
	{
		if (bId)
			return Arg.replace(0, 0, L"id ");
		if (bShortCut)
			return Arg.replace(0, 0, L"sc ");
		if (bSelf)
			return this->GetAdminName();
		if (bTarget)
		{
			auto client = Hk::Client::GetClientIdFromCharName(this->GetAdminName());
			if (client.has_error())
				return L"";
			uint ship;
			pub::Player::GetShip(client.value(), ship);
			if (!ship)
				return L"";
			const uint iTarget = Hk::Player::GetTarget(ship).value();
			if (!iTarget)
				return L"";
			auto targetId = Hk::Client::GetClientIdByShip(iTarget);
			if (!targetId.has_error())
				return L"";
			return L"id " + std::to_wstring(targetId.value());
		}
	}

	{
		if (Arg == L">s")
			return this->GetAdminName();
		if (Arg.find(L">i") == 0)
			return L"id " + Arg.substr(2);
		if (Arg == L">t")
		{
			auto client = Hk::Client::GetClientIdFromCharName(this->GetAdminName());
			if (client.has_error())
				return L"";
			uint ship;
			pub::Player::GetShip(client.value(), ship);
			if (!ship)
				return L"";
			const uint iTarget = Hk::Player::GetTarget(ship).value();
			if (!iTarget)
				return L"";
			auto targetId = Hk::Client::GetClientIdByShip(iTarget);
			if (!targetId.has_error())
				return L"";
			return L"id " + std::to_wstring(targetId.value());
		}
		return Arg;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

int CCmds::ArgInt(uint iArg)
{
	const std::wstring Arg = GetParam(CurCmdString, ' ', iArg);

	return ToInt(Arg);
}

uint CCmds::ArgUInt(uint iArg)
{
	const std::wstring Arg = GetParam(CurCmdString, ' ', iArg);

	return ToUInt(Arg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

float CCmds::ArgFloat(uint iArg)
{
	const std::wstring Arg = GetParam(CurCmdString, ' ', iArg);
	return ToFloat(Arg);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CCmds::ArgStr(uint iArg)
{
	std::wstring Arg = GetParam(CurCmdString, ' ', iArg);

	return Arg;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::wstring CCmds::ArgStrToEnd(uint iArg)
{
	for (uint i = 0, iCurArg = 0; (i < CurCmdString.length()); i++)
	{
		if (CurCmdString[i] == ' ')
		{
			iCurArg++;

			if (iCurArg == iArg)
				return CurCmdString.substr(i + 1);

			while (((i + 1) < CurCmdString.length()) && (CurCmdString[i + 1] == ' '))
				i++; // skip "whitechar"
		}
	}

	return L"";
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::ExecuteCommandString(const std::wstring& CmdStr)
{
	// check if command was sent by a socket connection
	const std::wstring AdminName = GetAdminName();

	try
	{
		AddLog(LogType::AdminCmds, LogLevel::Info, std::format("{}: {}", wstos(AdminName).c_str(), wstos(CmdStr).c_str()));

		bId = false;
		bShortCut = false;
		bSelf = false;
		bTarget = false;
		CurCmdString = CmdStr;

		std::wstring Cmd = ToLower(GetParam(CmdStr, ' ', 0));
		if (Cmd.length() == 0)
		{
			Print("ERR unknown command");
			return;
		}

		const size_t Cmd_pos = CmdStr.find(Cmd);

		if (Cmd[Cmd.length() - 1] == '$')
		{
			bId = true;
			Cmd.erase(Cmd.length() - 1, 1);
		}
		else if (Cmd[Cmd.length() - 1] == '&')
		{
			bShortCut = true;
			Cmd.erase(Cmd.length() - 1, 1);
		}
		else if (Cmd[Cmd.length() - 1] == '!')
		{
			bSelf = true;
			CurCmdString.insert(Cmd_pos + Cmd.length() - 1, L" ");
			Cmd.erase(Cmd.length() - 1, 1);
		}
		else if (Cmd[Cmd.length() - 1] == '?')
		{
			bTarget = true;
			CurCmdString.insert(Cmd_pos + Cmd.length() - 1, L" ");
			Cmd.erase(Cmd.length() - 1, 1);
		}

		if (const bool plugins = CallPluginsBefore(HookedCall::FLHook__AdminCommand__Process, this, Cmd); !plugins)
		{
			if (Cmd == L"getcash")
			{
				CmdGetCash(ArgCharname(1));
			}
			else if (Cmd == L"setcash")
			{
				CmdSetCash(ArgCharname(1), ArgUInt(2));
			}
			else if (Cmd == L"addcash")
			{
				CmdAddCash(ArgCharname(1), ArgUInt(2));
			}
			else if (Cmd == L"kick")
			{
				CmdKick(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (Cmd == L"ban")
			{
				CmdBan(ArgCharname(1));
			}
			else if (Cmd == L"tempban")
			{
				CmdTempBan(ArgCharname(1), ArgUInt(2));
			}
			else if (Cmd == L"unban")
			{
				CmdUnban(ArgCharname(1));
			}
			else if (Cmd == L"getclientid")
			{
				CmdGetClientID(ArgCharname(1));
			}
			else if (Cmd == L"beam")
			{
				CmdBeam(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (Cmd == L"kill")
			{
				CmdKill(ArgCharname(1));
			}
			else if (Cmd == L"resetrep")
			{
				CmdResetRep(ArgCharname(1));
			}
			else if (Cmd == L"setrep")
			{
				CmdSetRep(ArgCharname(1), ArgStr(2), ArgFloat(3));
			}
			else if (Cmd == L"getrep")
			{
				CmdGetRep(ArgCharname(1), ArgStr(2));
			}
			else if (Cmd == L"msg")
			{
				CmdMsg(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (Cmd == L"msgs")
			{
				CmdMsgS(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (Cmd == L"msgu")
			{
				CmdMsgU(ArgStrToEnd(1));
			}
			else if (Cmd == L"fmsg")
			{
				CmdFMsg(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (Cmd == L"fmsgs")
			{
				CmdFMsgS(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (Cmd == L"fmsgu")
			{
				CmdFMsgU(ArgStrToEnd(1));
			}
			else if (Cmd == L"enumcargo")
			{
				CmdEnumCargo(ArgCharname(1));
			}
			else if (Cmd == L"removecargo")
			{
				CmdRemoveCargo(ArgCharname(1), static_cast<ushort>(ArgInt(2)), ArgInt(3));
			}
			else if (Cmd == L"addcargo")
			{
				CmdAddCargo(ArgCharname(1), ArgStr(2), ArgInt(3), ArgInt(4));
			}
			else if (Cmd == L"rename")
			{
				CmdRename(ArgCharname(1), ArgStr(2));
			}
			else if (Cmd == L"deletechar")
			{
				CmdDeleteChar(ArgCharname(1));
			}
			else if (Cmd == L"readcharfile")
			{
				CmdReadCharFile(ArgCharname(1));
			}
			else if (Cmd == L"writecharfile")
			{
				CmdWriteCharFile(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (Cmd == L"getplayerinfo")
			{
				CmdGetPlayerInfo(ArgCharname(1));
			}
			else if (Cmd == L"getplayers")
			{
				CmdGetPlayers();
			}
			else if (Cmd == L"xgetplayerinfo")
			{
				CmdXGetPlayerInfo(ArgCharname(1));
			}
			else if (Cmd == L"xgetplayers")
			{
				CmdXGetPlayers();
			}
			else if (Cmd == L"getplayerids")
			{
				CmdGetPlayerIds();
			}
			else if (Cmd == L"getaccountdirname")
			{
				CmdGetAccountDirName(ArgCharname(1));
			}
			else if (Cmd == L"getcharfilename")
			{
				CmdGetCharFileName(ArgCharname(1));
			}
			else if (Cmd == L"savechar")
			{
				CmdSaveChar(ArgCharname(1));
			}
			else if (Cmd == L"isonserver")
			{
				CmdIsOnServer(ArgCharname(1));
			}
			else if (Cmd == L"moneyfixlist")
			{
				CmdMoneyFixList();
			}
			else if (Cmd == L"serverinfo")
			{
				CmdServerInfo();
			}
			else if (Cmd == L"getgroupmembers")
			{
				CmdGetGroupMembers(ArgCharname(1));
			}
			else if (Cmd == L"getreservedslot")
			{
				CmdGetReservedSlot(ArgCharname(1));
			}
			else if (Cmd == L"setreservedslot")
			{
				CmdSetReservedSlot(ArgCharname(1), ArgInt(2));
			}
			else if (Cmd == L"setadmin")
			{
				CmdSetAdmin(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (Cmd == L"getadmin")
			{
				CmdGetAdmin(ArgCharname(1));
			}
			else if (Cmd == L"deladmin")
			{
				CmdDelAdmin(ArgCharname(1));
			}
			else if (Cmd == L"unloadplugin")
			{
				CmdUnloadPlugin(ArgStrToEnd(1));
			}
			else if (Cmd == L"loadplugins")
			{
				CmdLoadPlugins();
			}
			else if (Cmd == L"reloadplugin")
			{
				CmdReloadPlugin(ArgStrToEnd(1));
			}
			else if (Cmd == L"loadplugin")
			{
				CmdLoadPlugin(ArgStrToEnd(1));
			}
			else if (Cmd == L"shutdown")
			{
				CmdShutdown();
			}
			else if (Cmd == L"listplugins")
			{
				CmdListPlugins();
			}
			else if (Cmd == L"help")
			{
				CmdHelp();
			}
			else if (Cmd == L"move")
			{
				CmdMove(AdminName, ArgFloat(1), ArgFloat(2), ArgFloat(3));
			}
			else if (Cmd == L"chase")
			{
				CmdChase(AdminName, ArgCharname(1));
			}
			else if (Cmd == L"beam")
			{
				CmdBeam(ArgCharname(1), ArgStrToEnd(2));
			}
			else if (Cmd == L"pull")
			{
				CmdPull(AdminName, ArgCharname(1));
			}
			else
			{
				Print("ERR unknown command");
			}
		}

		AddLog(LogType::AdminCmds, LogLevel::Info, "finished");
	}
	catch (...)
	{
		AddLog(LogType::AdminCmds, LogLevel::Info, "exception");
		Print("ERR exception occured");
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CCmds::SetRightsByString(const std::string& Rights)
{
	// TODO: Implement admin rights
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
