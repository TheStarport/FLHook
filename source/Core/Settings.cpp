#include "PCH.hpp"

#include "Global.hpp"

#include "Defs/FLHookConfig.hpp"

void LoadSettings()
{
    auto config = Serializer::LoadFromJson<FLHookConfig>(L"flhook.json");

    // NoPVP
    config->general.noPVPSystemsHashed.clear();
    for (const auto& system : config->general.noPVPSystems)
    {
        uint systemId;
        pub::GetSystemID(systemId, StringUtils::wstos(system).c_str());
        config->general.noPVPSystemsHashed.emplace_back(systemId);
    }

    // Explicitly replace the config
    FLHookConfig::i(&config);
}
