#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    void __stdcall SPObjCollision(const SSPObjCollisionInfo& oci, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"SPObjCollision(\n\tClientId client = {}\n)", client));

        const auto skip = CallPlugins(&Plugin::OnSpObjectCollision, client, oci);

        CHECK_FOR_DISCONNECT;

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.SPObjCollision(oci, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnSpObjectCollisionAfter, client, oci);
    }

} // namespace IServerImplHook
