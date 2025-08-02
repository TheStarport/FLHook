#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "API/Utils/Logger.hpp"

void __stdcall IServerImplHook::SetManeuver(ClientId client, const XSetManeuver& sm)
{
    TRACE("IServerImplHook::SetManeuver client={{client}}", { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnSetManeuver, client, sm); !skip)
    {
        CallServerPreamble { Server.SetManeuver(client.GetValue(), sm); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetManeuverAfter, client, sm);
}
