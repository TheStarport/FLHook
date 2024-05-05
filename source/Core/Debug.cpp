#include "PCH.hpp"

#include "Core/DebugTools.hpp"
#include <Utils/Detour.hpp>

using CreateIDType = uint (*)(const char*);

//std::shared_ptr<spdlog::logger> hashList = nullptr;
const auto detour = std::make_unique<FunctionDetour<CreateIDType>>(CreateID);

uint DebugTools::CreateIdDetour(const char* str)
{
    if (!str)
    {
        return 0;
    }

    detour->UnDetour();
    const uint hash = CreateID(str);
    detour->Detour(CreateIdDetour);

    const std::string fullStr = str;
    if (hashMap.contains(fullStr))
    {
        return hash;
    }

    hashMap[fullStr] = hash;
    //TODO: remove spdlog
    //hashList->log(spdlog::level::debug, std::format("{}  {:#X}  {}", hash, hash, fullStr));

    return hash;
}

void DebugTools::Init()
{
    if (FLHook::GetConfig().logging.minLogLevel <= 1)
    {
        return;
    }

    //TODO: remove spdlog
    //hashList = spdlog::basic_logger_mt<spdlog::async_factory>("flhook_hashmap", "logs/flhook_hashmap.log");
    //spdlog::flush_on(spdlog::level::debug);
    //hashList->set_level(spdlog::level::debug);

    detour->Detour(CreateIdDetour);
};
