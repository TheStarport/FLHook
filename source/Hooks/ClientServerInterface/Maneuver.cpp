#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::SetManeuver(ClientId client, const XSetManeuver& sm)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"SetManeuver(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnSetManeuver, client, sm); !skip)
    {
        CallServerPreamble { Server.SetManeuver(client, sm); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetManeuverAfter, client, sm);
}
