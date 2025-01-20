#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::SystemSwitchOutComplete(Id shipId, ClientId client)
{
    auto ship = shipId.AsShip();

    TRACE(L"{0}{1}}", { L"shipId", std::to_wstring(shipId.GetValue()) }, { L"client", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnSystemSwitchOutComplete, client, ship); !skip)
    {
        CallServerPreamble { Server.SystemSwitchOutComplete(shipId.GetValue(), client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSystemSwitchOutCompleteAfter, client, ship);
}
