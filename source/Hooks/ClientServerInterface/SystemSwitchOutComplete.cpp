#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::SystemSwitchOutComplete(Id shipId, ClientId client)
{
    auto ship = shipId.AsShip();

    TRACE("{{shipId}} {{client}}", { "shipId", shipId }, { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnSystemSwitchOutComplete, client, ship); !skip)
    {
        CallServerPreamble { Server.SystemSwitchOutComplete(shipId.GetValue(), client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSystemSwitchOutCompleteAfter, client, ship);
}
