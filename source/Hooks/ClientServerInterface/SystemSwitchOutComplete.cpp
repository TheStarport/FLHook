#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/Logger.hpp"

void __stdcall IServerImplHook::SystemSwitchOutComplete(ShipId shipId, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"SystemSwitchOutComplete(\n\tuint shipId = {}\n\tClientId client = {}\n)", shipId, client));

    if (const auto skip = CallPlugins(&Plugin::OnSystemSwitchOutComplete, client, shipId); !skip)
    {
        CallServerPreamble { Server.SystemSwitchOutComplete(shipId.GetValue(), client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSystemSwitchOutCompleteAfter, client, shipId);
}
