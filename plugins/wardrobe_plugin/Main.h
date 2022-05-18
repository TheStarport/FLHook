#pragma once

#include <FLHook.h>
#include <plugin.h>

ReturnCode returncode = ReturnCode::Default;

std::map<std::string, std::string> heads;
std::map<std::string, std::string> bodies;

struct RESTART
{
	std::wstring wscCharname;
	std::wstring wscDir;
	std::wstring wscCharfile;
	std::string costume;
	bool head; // 1 Head, 0 Body
};

std::list<RESTART> pendingRestarts;