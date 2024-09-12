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

    const std::string fullStr = str;
    if (const auto hash = hashMap.find(fullStr); hash != hashMap.end())
    {
        return hash->second;
    }

    detour->UnDetour();
    const uint hash = CreateID(str);
    detour->Detour(CreateIdDetour);

    std::ofstream file;
    file.open("logs/hashmap.csv", std::ios::app);
    file << fullStr << "," << hash << ",0x" << std::hex << hash << "\n";
    file.close();

    hashMap[fullStr] = hash;
    return hash;
}

void DebugTools::Init()
{
    if (FLHook::GetConfig()->logging.minLogLevel > 2)
    {
        return;
    }

    if (constexpr std::string_view hashMap = "logs/hashmap.csv"; std::filesystem::exists(hashMap))
    {
        std::filesystem::remove(hashMap);
    }

    detour->Detour(CreateIdDetour);
};
