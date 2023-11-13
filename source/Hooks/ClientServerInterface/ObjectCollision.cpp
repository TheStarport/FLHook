#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::SpObjCollision(const SSPObjCollisionInfo& oci, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"SPObjCollision(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnSpObjectCollision, client, oci);

    CheckForDisconnect;

    if (!skip)
    {
        CallServerPreamble { Server.SPObjCollision(oci, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpObjectCollisionAfter, client, oci);
}
