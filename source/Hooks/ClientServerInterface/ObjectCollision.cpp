#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

void __stdcall IServerImplHook::SpObjCollision(const SSPObjCollisionInfo& oci, ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"SPObjCollision(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnSpObjectCollision, client, oci);

    CheckForDisconnect;

    if (!skip)
    {
        CallServerPreamble { Server.SPObjCollision(oci, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpObjectCollisionAfter, client, oci);
}
