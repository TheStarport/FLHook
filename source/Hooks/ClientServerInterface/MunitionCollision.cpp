#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "API/Utils/Logger.hpp"

void IServerImplHook::SpMunitionCollisionInner(const SSPMunitionCollisionInfo& mci, uint)
{
    TryHook
    {
        if (const auto isClient = ShipId(mci.targetShip).GetPlayer(); isClient.has_value())
        {
            FLHook::instance->damageToClientId = isClient.value().GetValue();
        }
    }
    CatchHook({})
}

void __stdcall IServerImplHook::SpMunitionCollision(const SSPMunitionCollisionInfo& mci, ClientId client)
{
    Logger::Log(LogLevel::Trace, std::format(L"SPMunitionCollision(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnSpMunitionCollision, client, mci);

    CheckForDisconnect;

    SpMunitionCollisionInner(mci, client.GetValue());

    if (!skip)
    {
        CallServerPreamble { Server.SPMunitionCollision(mci, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpMunitionCollisionAfter, client, mci);
}
