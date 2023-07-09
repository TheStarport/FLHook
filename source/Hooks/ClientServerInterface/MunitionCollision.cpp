#include "PCH.hpp"

#include "Global.hpp"
#include "API/FLServer/Client.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    void SPMunitionCollision__Inner(const SSPMunitionCollisionInfo& mci, uint)
    {
        TRY_HOOK
        {
            if (const auto isClient = Hk::Client::GetClientIdByShip(mci.targetShip).Raw(); isClient.has_value())
            {
                CoreGlobals::i()->damageToClientId = isClient.value();
            }
        }
        CATCH_HOOK({})
    }

    void __stdcall SPMunitionCollision(const SSPMunitionCollisionInfo& mci, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"SPMunitionCollision(\n\tClientId client = {}\n)", client));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPMunitionCollision, mci, client);

        CHECK_FOR_DISCONNECT;

        SPMunitionCollision__Inner(mci, client);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.SPMunitionCollision(mci, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__SPMunitionCollision, mci, client);
    }

} // namespace IServerImplHook