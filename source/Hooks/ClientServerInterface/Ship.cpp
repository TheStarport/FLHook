#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::RequestCreateShip(ClientId client)
{
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnRequestCreateShip, client); !skip)
    {
        CallServerPreamble { Server.RequestCreateShip(client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestCreateShipAfter, client);
}

void __stdcall IServerImplHook::ReqCollisionGroups(const st6::list<CollisionGroupDesc>& collisionGroups, ClientId client)
{
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnRequestCollisionGroups, client, collisionGroups); !skip)
    {
        CallServerPreamble { Server.ReqCollisionGroups(collisionGroups, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestCollisionGroupsAfter, client, collisionGroups);
}

void __stdcall IServerImplHook::ReqShipArch(ArchId archId, ClientId client)
{
    TRACE(L"{0} {1}}", { L"archId", std::to_wstring(archId) }, { L"client", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnRequestShipArch, client, archId); !skip)
    {
        CallServerPreamble { Server.ReqShipArch(archId, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestShipArchAfter, client, archId);
}

void __stdcall IServerImplHook::ReqHullStatus(float status, ClientId client)
{
    TRACE(L"{0} {1}}", { L"status", std::to_wstring(status) }, { L"client", std::to_wstring(client.GetValue()) });

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

    TRACE(L"{0} {1} {2} {3}",
          { L"shipId", std::to_wstring(shipId.GetValue()) },
          { L"enable", std::to_wstring(enable) },
          { L"reason", std::to_wstring(static_cast<int>(reason)) },
          { L"client", std::to_wstring(client.GetValue()) })

    if (const auto skip = CallPlugins(&Plugin::OnSpRequestInvincibility, client, ship, enable, reason); !skip)
    {
        CallServerPreamble { Server.SPRequestInvincibility(shipId.GetValue(), enable, reason, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpRequestInvincibilityAfter, client, ship, enable, reason);
}
