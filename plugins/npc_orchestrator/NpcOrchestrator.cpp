#include "PCH.hpp"

#include "NpcOrchestrator.hpp"

#include "API/FLHook/ResourceManager.hpp"

namespace Plugins
{

    NpcOrchestratorPlugin::NpcOrchestratorPlugin(const PluginInfo& info) : Plugin(info) {}

    Task NpcOrchestratorPlugin::AdminCmdCreateNpc(const ClientId client)
    {
        auto resourceManager = FLHook::GetResourceManager();
        resourceManager->NewBuilder()
            .WithNpc(L"MSN01a_Liberty_Rogue")
            .WithSystem(client.GetSystemId().Handle())
            .WithPosition(client.GetShip().Handle().GetPositionAndOrientation().Handle().first)
            .Spawn();

        co_return TaskStatus::Finished;
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Npc Orchestrator", L"npc_orchestrator", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(NpcOrchestratorPlugin, Info);
