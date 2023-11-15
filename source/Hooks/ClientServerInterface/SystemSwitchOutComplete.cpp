#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/Logger.hpp"

void SystemSwitchOutCompleteInnerAfter(uint, ClientId client)
{
    TryHook
    {
        // TODO: Implement event for switch out
    }
    CatchHook({})
}

void __stdcall IServerImplHook::SystemSwitchOutComplete(uint shipId, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"SystemSwitchOutComplete(\n\tuint shipId = {}\n\tClientId client = {}\n)", shipId, client));

    if (const auto skip = CallPlugins(&Plugin::OnSystemSwitchOutComplete, client, shipId); !skip)
    {
        CallServerPreamble { Server.SystemSwitchOutComplete(shipId, client.GetValue()); }
        CallServerPostamble(true, );
    }
    SystemSwitchOutCompleteInnerAfter(shipId, client);

    CallPlugins(&Plugin::OnSystemSwitchOutCompleteAfter, client, shipId);
}
