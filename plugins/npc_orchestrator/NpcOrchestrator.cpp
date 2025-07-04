#include "PCH.hpp"

#include "NpcOrchestrator.hpp"

#include "API/FLHook/ResourceManager.hpp"

namespace Plugins
{

    NpcOrchestratorPlugin::NpcOrchestratorPlugin(const PluginInfo& info) : Plugin(info) {}

    concurrencpp::result<void>NpcOrchestratorPlugin::AdminCmdCreateNpc(const ClientId client)
    {
        auto resourceManager = FLHook::GetResourceManager();
        resourceManager->NewBuilder()
            .WithNpc(L"MSN01a_Liberty_Rogue")
            .WithSystem(client.GetSystemId().Handle())
            .WithPosition(client.GetShip().Handle().GetPositionAndOrientation().Handle().first)
            .Spawn();

        co_return;
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Npc Orchestrator",
	    .shortName = L"npc_orchestrator",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(NpcOrchestratorPlugin);
