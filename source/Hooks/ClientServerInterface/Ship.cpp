#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"


namespace IServerImplHook
{
    void __stdcall RequestCreateShip(ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"RequestCreateShip(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCreateShip, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.RequestCreateShip(client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__RequestCreateShip, client);
    }

    void __stdcall ReqCollisionGroups(const st6::list<CollisionGroupDesc>& collisionGroups, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqCollisionGroups(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqCollisionGroups, collisionGroups, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqCollisionGroups(collisionGroups, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__ReqCollisionGroups, collisionGroups, client);
    }

    void __stdcall ReqShipArch(uint archId, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqShipArch(\n\tuint archId = {}\n\tClientId client = {}\n)", archId, client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqShipArch, archId, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqShipArch(archId, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__ReqShipArch, archId, client);
    }

    void __stdcall ReqHulatus(float status, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqHulatus(\n\tfloat status = {}\n\tClientId client = {}\n)", status, client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqHulatus, status, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqHullStatus(status, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__ReqHulatus, status, client);
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

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestInvincibility, shipId, enable, reason, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SPRequestInvincibility(shipId, enable, reason, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__SPRequestInvincibility, shipId, enable, reason, client);
    }

} // namespace IServerImplHook
