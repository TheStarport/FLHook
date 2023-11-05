#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    void __stdcall RequestCreateShip(ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"RequestCreateShip(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPlugins(&Plugin::OnRequestCreateShip, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.RequestCreateShip(client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestCreateShipAfter, client);
    }

    void __stdcall ReqCollisionGroups(const st6::list<CollisionGroupDesc>& collisionGroups, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqCollisionGroups(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPlugins(&Plugin::OnRequestCollisionGroups, client, collisionGroups); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqCollisionGroups(collisionGroups, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestCollisionGroupsAfter, client, collisionGroups);
    }

    void __stdcall ReqShipArch(ArchId archId, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqShipArch(\n\tuint archId = {}\n\tClientId client = {}\n)", archId, client));

        if (const auto skip = CallPlugins(&Plugin::OnRequestShipArch, client, archId); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqShipArch(archId, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestShipArchAfter, client, archId);
    }

    void __stdcall ReqHulatus(float status, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqHulatus(\n\tfloat status = {}\n\tClientId client = {}\n)", status, client));

        if (const auto skip = CallPlugins(&Plugin::OnRequestHullStatus, client, status); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqHullStatus(status, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestHullStatusAfter, client, status);
    }

    void __stdcall SPRequestInvincibility(uint shipId, bool enable, InvincibilityReason reason, ClientId client)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"SPRequestInvincibility(\n\tuint shipId = {}\n\tbool enable = {}\n\tInvincibilityReason reason = {}\n\tClientId client = {}\n)",
                        shipId,
                        enable,
                        (int)reason,
                        client));

        if (const auto skip = CallPlugins(&Plugin::OnSpRequestInvincibility, client, shipId, enable, reason); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SPRequestInvincibility(shipId, enable, reason, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnSpRequestInvincibilityAfter, client, shipId, enable, reason);
    }

} // namespace IServerImplHook
