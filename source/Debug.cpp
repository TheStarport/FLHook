#include "Global.hpp"

#define SPDLOG_USE_STD_FORMAT
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

PBYTE DebugTools::createIdMemory;
DebugTools::CreateIDType DebugTools::originalCreateId;
std::map<std::string, uint> DebugTools::hashMap;

std::shared_ptr<spdlog::logger> hashList = nullptr;

uint DebugTools::CreateIdDetour(const char* str)
{
	if (!str)
		return 0;

	UnDetour(PBYTE(originalCreateId), createIdMemory);
	uint hash = CreateID(str);
	Detour(PBYTE(originalCreateId), CreateIdDetour, createIdMemory);

	std::string fullStr = str;
	if (hashMap.contains(fullStr))
	{
		return hash;
	}

	hashMap[fullStr] = hash;
	hashList->log(spdlog::level::debug, std::format("{}  {:#X}  {}", hash, hash, fullStr));

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

	createIdMemory = allocator.allocate(5);
	originalCreateId = CreateID;
	Detour(PBYTE(originalCreateId), CreateIdDetour, createIdMemory);
};