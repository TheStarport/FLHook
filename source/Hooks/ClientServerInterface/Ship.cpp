#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/ExceptionHandler.hpp"
#include "Core/PluginManager.hpp"
#include "Exceptions/StopProcessingException.hpp"

void __stdcall IServerImplHook::RequestCreateShip(ClientId client)
{
    TRACE("IServerImplHook::RequestCreateShip client={{client}}", { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnRequestCreateShip, client); !skip)
    {
        CallServerPreamble { Server.RequestCreateShip(client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestCreateShipAfter, client);
}

void __stdcall IServerImplHook::ReqCollisionGroups(const st6::list<CollisionGroupDesc>& collisionGroups, ClientId client)
{
    TRACE("IServerImplHook::ReqCollisionGroups client={{client}}", { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnRequestCollisionGroups, client, collisionGroups); !skip)
    {
        CallServerPreamble { Server.ReqCollisionGroups(collisionGroups, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestCollisionGroupsAfter, client, collisionGroups);
}

void __stdcall IServerImplHook::ReqShipArch(ArchId archId, ClientId client)
{
    TRACE("IServerImplHook::ReqShipArch archId={{archId}} client={{client}}", { "archId", archId }, { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnRequestShipArch, client, archId); !skip)
    {
        CallServerPreamble { Server.ReqShipArch(archId, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestShipArchAfter, client, archId);
}

void __stdcall IServerImplHook::ReqHullStatus(float status, ClientId client)
{
    TRACE("IServerImplHook::ReqHullStatus status={{status}} client={{client}}", { "status", status }, { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnRequestHullStatus, client, status); !skip)
    {
        CallServerPreamble { Server.ReqHullStatus(status, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestHullStatusAfter, client, status);
}

void __stdcall IServerImplHook::SpRequestInvincibility(Id shipId, bool enable, InvincibilityReason reason, ClientId client)
{
    auto ship = shipId.AsShip();

    TRACE("IServerImplHook::SpRequestInvincibility shipId={{shipId}} enable={{enable}} reason={{reason}} client={{client}}",
          { "shipId", shipId },
          { "enable", enable },
          { "reason", static_cast<int>(reason) },
          { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnSpRequestInvincibility, client, ship, enable, reason); !skip)
    {
        CallServerPreamble { Server.SPRequestInvincibility(shipId.GetValue(), enable, reason, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpRequestInvincibilityAfter, client, ship, enable, reason);
}
