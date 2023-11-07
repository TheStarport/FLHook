#include "PCH.hpp"

#include "API/FLServer/Client.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

void SPMunitionCollisionInner(const SSPMunitionCollisionInfo& mci, uint)
{
    TryHook
    {
        if (const auto isClient = Hk::Client::GetClientIdByShip(mci.targetShip).Raw(); isClient.has_value())
        {
            CoreGlobals::i()->damageToClientId = isClient.value();
        }
    }
    CatchHook({})
}

void __stdcall IServerImplHook::SpMunitionCollision(const SSPMunitionCollisionInfo& mci, ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"SPMunitionCollision(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnSpMunitionCollision, client, mci);

    CheckForDisconnect;

    SPMunitionCollisionInner(mci, client);

    if (!skip)
    {
        CallServerPreamble { Server.SPMunitionCollision(mci, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpMunitionCollisionAfter, client, mci);
}
