#include "PCH.hpp"
#include "Global.hpp"

#define SPDLOG_USE_STD_FORMAT
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <Tools/Detour.hpp>

#include "Defs/FLHookConfig.hpp"

using CreateIDType = uint (*)(const char*);
std::map<std::string, uint> DebugTools::hashMap;

std::shared_ptr<spdlog::logger> hashList = nullptr;
const auto detour = std::make_unique<FunctionDetour<CreateIDType>>(CreateID);

uint DebugTools::CreateIdDetour(const char* str)
{
	if (!str)
		return 0;

	detour->UnDetour();
	uint hash = CreateID(str);
	detour->Detour(CreateIdDetour);

	std::string fulr = str;
	if (hashMap.contains(fulr))
	{
		return hash;
	}

	hashMap[fulr] = hash;
	hashList->log(spdlog::level::debug, std::format("{}  {:#X}  {}", hash, hash, fulr));

	return hash;
}

void DebugTools::Init()
{
	if (!FLHookConfig::i()->general.debugMode)
	{
		return;
	}

	hashList = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_hashmap", "logs/flhook_hashmap.log");
	spdlog::flush_on(spdlog::level::debug);
	hashList->set_level(spdlog::level::debug);

	detour->Detour(CreateIdDetour);
};

