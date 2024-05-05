#include "PCH.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "API/Utils/Logger.hpp"

void __stdcall IServerImplHook::RequestCreateShip(ClientId client)
{
    Logger::Trace(std::format(L"RequestCreateShip(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestCreateShip, client); !skip)
    {
        CallServerPreamble { Server.RequestCreateShip(client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestCreateShipAfter, client);
}

void __stdcall IServerImplHook::ReqCollisionGroups(const st6::list<CollisionGroupDesc>& collisionGroups, ClientId client)
{
    Logger::Trace(std::format(L"ReqCollisionGroups(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestCollisionGroups, client, collisionGroups); !skip)
    {
        CallServerPreamble { Server.ReqCollisionGroups(collisionGroups, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestCollisionGroupsAfter, client, collisionGroups);
}

void __stdcall IServerImplHook::ReqShipArch(ArchId archId, ClientId client)
{
    Logger::Trace(std::format(L"ReqShipArch(\n\tuint archId = {}\n\tClientId client = {}\n)", archId, client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestShipArch, client, archId); !skip)
    {
        CallServerPreamble { Server.ReqShipArch(archId, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestShipArchAfter, client, archId);
}

void __stdcall IServerImplHook::ReqHullStatus(float status, ClientId client)
{
    Logger::Trace(std::format(L"ReqHulatus(\n\tfloat status = {}\n\tClientId client = {}\n)", status, client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestHullStatus, client, status); !skip)
    {
        CallServerPreamble { Server.ReqHullStatus(status, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestHullStatusAfter, client, status);
}

void __stdcall IServerImplHook::SpRequestInvincibility(ShipId shipId, bool enable, InvincibilityReason reason, ClientId client)
{
    Logger::Trace(std::format(L"SPRequestInvincibility(\n\tuint shipId = {}\n\tbool enable = {}\n\tInvincibilityReason reason = {}\n\tClientId client = {}\n)",
                    shipId,
                    enable,
                    (int)reason,
                    client));

    if (const auto skip = CallPlugins(&Plugin::OnSpRequestInvincibility, client, shipId, enable, reason); !skip)
    {
        CallServerPreamble { Server.SPRequestInvincibility(shipId.GetValue(), enable, reason, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSpRequestInvincibilityAfter, client, shipId, enable, reason);
}
