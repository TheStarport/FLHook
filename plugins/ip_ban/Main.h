#pragma once

#include <FLHook.hpp>
#include <plugin.h>

struct Config : Reflectable
{
	std::string File() override { return "config/ip_ban.json"; }
	std::wstring BanMessage = L"You are banned, please contact an administrator";
};

struct IPBans : Reflectable
{
	std::string File() override
	{
		char path[MAX_PATH];
		GetUserDataPath(path);
		return std::string(path) + "\\IPBans.json";
	}

	std::vector<std::string> Bans;
};

struct LoginIdBans : Reflectable
{
	std::string File() override
	{
		char path[MAX_PATH];
		GetUserDataPath(path);
		return std::string(path) + "\\LoginIdBans.json";
	}

	std::vector<std::string> Bans;
};

struct AuthenticatedAccounts : Reflectable
{
	std::string File() override
	{
		char path[MAX_PATH];
		GetUserDataPath(path);
		return std::string(path) + "\\AuthenticatedAccounts.json";
	}

	std::vector<std::wstring> Accounts;
};

struct Global final
{
	ReturnCode returncode = ReturnCode::Default;
	std::unique_ptr<Config> config = nullptr;

	/// list of bans
	IPBans ipBans;
	LoginIdBans loginIdBans;

	// Authenticated accounts even if they are banned somehow
	AuthenticatedAccounts authenticatedAccounts;

	std::map<uint, bool> IPChecked;
};