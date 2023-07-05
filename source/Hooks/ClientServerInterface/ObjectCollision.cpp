#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    void __stdcall SPObjCollision(const SSPObjCollisionInfo& oci, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"SPObjCollision(\n\tClientId client = {}\n)", client));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjCollision, oci, client);

        CHECK_FOR_DISCONNECT;

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.SPObjCollision(oci, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__SPObjCollision, oci, client);
    }

} // namespace IServerImplHook