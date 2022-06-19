#pragma once

#include <Wildcard.hpp>

#include <FLHook.h>
#include <plugin.h>

struct IPBans : Reflectable
{
	std::string File() override
	{
		char path[MAX_PATH];
		GetUserDataPath(path);
		return std::string(path) + "\\IPBans.json";
	}

	std::list<std::string> Bans;
};

struct LoginIDBans : Reflectable
{
	std::string File() override
	{
		char path[MAX_PATH];
		GetUserDataPath(path);
		return std::string(path) + "\\LoginIDBans.json";
	}

	std::list<std::string> Bans;
};

struct AuthenticatedAccounts : Reflectable
{
	std::string File() override
	{
		char path[MAX_PATH];
		GetUserDataPath(path);
		return std::string(path) + "\\AuthenticatedAccounts.json";
	}

	std::list<std::wstring> Accounts;
};

struct Global final
{
	ReturnCode returncode = ReturnCode::Default;

	/// list of bans
	IPBans ipBans;
	LoginIDBans loginIDBans;

	// Authenticated accounts even if they are banned somehow
	AuthenticatedAccounts authenticatedAccounts;
	
	std::map<uint, bool> IPChecked;
};