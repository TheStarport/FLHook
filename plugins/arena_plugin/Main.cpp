// Arena Plugin by MadHunter
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes

#include "../base_plugin/Main.h"

#include <FLHook.hpp>
#include <plugin.h>

BaseCommunicator* baseCommunicator = nullptr;

#define CLIENT_STATE_NONE     0
#define CLIENT_STATE_TRANSFER 1
#define CLIENT_STATE_RETURN   2

int transferFlags[MaxClientId + 1];

const std::wstring STR_INFO1 = L"Please dock at nearest base";
const std::wstring STR_INFO2 = L"Cargo hold is not empty";

struct Config final : Reflectable
{
	std::string File() override { return "flhook_plugins/arena.json"; }

	// Reflectable fields
	std::string command = "arena";
	std::string targetBase;
	std::string targetSystem;
	std::string restrictedSystem;

	// Non-reflectable fields
	uint targetBaseId;
	uint targetSystemId;
	uint restrictedSystemId;
	std::wstring wscCommand;
};

REFL_AUTO(
    type(Config), field(command), field(targetBase), field(targetSystem), field(restrictedSystem))

/// A return code to indicate to FLHook if we want the hook processing to
/// continue.
ReturnCode returncode = ReturnCode::Default;

/// Clear client info when a client connects.
void ClearClientInfo(uint& iClientID)
{
	transferFlags[iClientID] = CLIENT_STATE_NONE;
}

std::unique_ptr<Config> config = nullptr;

// Client command processing
void UserCmd_Conn(uint iClientID, const std::wstring& wscParam);
void UserCmd_Return(uint iClientID, const std::wstring& wscParam);
std::array<USERCMD, 2> UserCmds = {{
    {L"/arena", UserCmd_Conn},
    {L"/return", UserCmd_Return},
}};

/// Load the configuration
void LoadSettings()
{
	memset(transferFlags, 0, sizeof(int) * (MaxClientId + 1));
	Config conf = Serializer::JsonToObject<Config>();
	conf.wscCommand = L"/" + stows(conf.command);
	conf.restrictedSystemId = CreateID(conf.restrictedSystem.c_str());
	conf.targetBaseId = CreateID(conf.targetBase.c_str());
	conf.targetSystemId = CreateID(conf.targetSystem.c_str());
	config = std::make_unique<Config>(std::move(conf));

	UserCmds[0] = { config->wscCommand.data(), UserCmd_Conn };
}

bool IsDockedClient(unsigned int client)
{
	unsigned int base = 0;
	pub::Player::GetBase(client, base);
	if (base)
		return true;

	return false;
}

bool ValidateCargo(unsigned int client)
{
	std::wstring playerName = (const wchar_t*)Players.GetActiveCharacterName(client);
	std::list<CARGO_INFO> cargo;
	int holdSize = 0;

	HkEnumCargo(playerName, cargo, holdSize);

	for (std::list<CARGO_INFO>::const_iterator it = cargo.begin(); it != cargo.end(); ++it)
	{
		const CARGO_INFO& item = *it;

		bool flag = false;
		pub::IsCommodity(item.iArchID, flag);

		// Some commodity present.
		if (flag)
			return false;
	}

	return true;
}

void StoreReturnPointForClient(unsigned int client)
{
	// It's not docked at a custom base, check for a regular base
	uint base = 0;

	if (baseCommunicator)
		base = baseCommunicator->GetCustomBaseId(client);

	if (!base)
		pub::Player::GetBase(client, base);
	if (!base)
		return;

	HkSetCharacterIni(client, L"conn.retbase", std::to_wstring(base));
}

unsigned int ReadReturnPointForClient(unsigned int client)
{
	return HkGetCharacterIniUint(client, L"conn.retbase");
}

void MoveClient(unsigned int client, unsigned int targetBase)
{
	// Ask that another plugin handle the beam.
	if (baseCommunicator && baseCommunicator->CustomBaseBeam(client, targetBase))
		return;

	// No plugin handled it, do it ourselves.
	unsigned int system;
	pub::Player::GetSystem(client, system);
	Universe::IBase* base = Universe::get_base(targetBase);

	pub::Player::ForceLand(client, targetBase); // beam

	// if not in the same system, emulate F1 charload
	if (base->iSystemID != system)
	{
		Server.BaseEnter(targetBase, client);
		Server.BaseExit(targetBase, client);
		std::wstring wscCharFileName;
		HkGetCharFileName(client, wscCharFileName);
		wscCharFileName += L".fl";
		CHARACTER_ID cID;
		strcpy_s(cID.szCharFilename, wstos(wscCharFileName.substr(0, 14)).c_str());
		Server.CharacterSelect(cID, client);
	}
}

bool CheckReturnDock(unsigned int client, unsigned int target)
{
	unsigned int base = 0;
	pub::Player::GetBase(client, base);

	if (base == target)
		return true;

	return false;
}

void __stdcall CharacterSelect(std::string& szCharFilename, uint& client)
{
	transferFlags[client] = CLIENT_STATE_NONE;
}

void __stdcall PlayerLaunch_AFTER(uint& ship, uint& client)
{
	if (transferFlags[client] == CLIENT_STATE_TRANSFER)
	{
		if (!ValidateCargo(client))
		{
			PrintUserCmdText(client, STR_INFO2);
			return;
		}

		transferFlags[client] = CLIENT_STATE_NONE;
		MoveClient(client, config->targetBaseId);
		return;
	}

	if (transferFlags[client] == CLIENT_STATE_RETURN)
	{
		if (!ValidateCargo(client))
		{
			PrintUserCmdText(client, STR_INFO2);
			return;
		}

		transferFlags[client] = CLIENT_STATE_NONE;
		unsigned int returnPoint = ReadReturnPointForClient(client);

		if (!returnPoint)
			return;

		MoveClient(client, returnPoint);
		HkSetCharacterIni(client, L"conn.retbase", L"0");
		return;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// USER COMMANDS
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void UserCmd_Conn(uint iClientID, const std::wstring& wscParam)
{
	// Prohibit jump if in a restricted system or in the target system
	uint system = 0;
	pub::Player::GetSystem(iClientID, system);
	if (system == config->restrictedSystemId || system == config->targetSystemId ||
	    (baseCommunicator && baseCommunicator->GetCustomBaseId(iClientID)))
	{
		PrintUserCmdText(iClientID, L"ERR Cannot use command in this system or base");
		return;
	}

	if (!IsDockedClient(iClientID))
	{
		PrintUserCmdText(iClientID, STR_INFO1);
		return;
	}

	if (!ValidateCargo(iClientID))
	{
		PrintUserCmdText(iClientID, STR_INFO2);
		return;
	}

	StoreReturnPointForClient(iClientID);
	PrintUserCmdText(iClientID, L"Redirecting undock to Arena.");
	transferFlags[iClientID] = CLIENT_STATE_TRANSFER;
}

void UserCmd_Return(uint iClientID, const std::wstring& wscParam)
{
	if (!ReadReturnPointForClient(iClientID))
	{
		PrintUserCmdText(iClientID, L"No return possible");
		return;
	}

	if (!IsDockedClient(iClientID))
	{
		PrintUserCmdText(iClientID, STR_INFO1);
		return;
	}

	if (!CheckReturnDock(iClientID, config->targetBaseId))
	{
		PrintUserCmdText(iClientID, L"Not in correct base");
		return;
	}

	if (!ValidateCargo(iClientID))
	{
		PrintUserCmdText(iClientID, STR_INFO2);
		return;
	}

	PrintUserCmdText(iClientID, L"Redirecting undock to previous base");
	transferFlags[iClientID] = CLIENT_STATE_RETURN;
}

// Hook on /help
void UserCmd_Help(uint& iClientID, const std::wstring& wscParam)
{
	PrintUserCmdText(iClientID, config->wscCommand);
	PrintUserCmdText(iClientID, L"Beams you to the Arena system.");
	PrintUserCmdText(iClientID, L"/return ");
	PrintUserCmdText(iClientID, L"Returns you to the previous base.");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

DefaultDllMainSettings(LoadSettings)

bool ProcessUserCmds(uint& clientId, const std::wstring& param)
{
	return DefaultUserCommandHandling(clientId, param, UserCmds, returncode);
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Arena");
	pi->shortName("arena");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &ProcessUserCmds);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch_AFTER, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);

	// We import the definitions for TempBan Communicator so we can talk to it
	baseCommunicator = static_cast<BaseCommunicator*>(PluginCommunicator::ImportPluginCommunicator(BaseCommunicator::pluginName));
}