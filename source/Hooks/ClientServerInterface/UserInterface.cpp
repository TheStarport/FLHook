#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    void __stdcall Hail(unsigned int _genArg1, unsigned int _genArg2, unsigned int _genArg3)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"Hail(\n\tunsigned int _genArg1 = {}\n\tunsigned int _genArg2 = {}\n\tunsigned int _genArg3 = {}\n)", _genArg1, _genArg2, _genArg3));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Hail, _genArg1, _genArg2, _genArg3); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.Hail(_genArg1, _genArg2, _genArg3); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__Hail, _genArg1, _genArg2, _genArg3);
    }

    void __stdcall RequestEvent(int eventType, uint shipId, uint dockTarget, uint _genArg1, ulong _genArg2, ClientId client)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"RequestEvent(\n\tint eventType = {}\n\tuint shipId = {}\n\tuint dockTarget = {}\n\tuint _genArg1 = {}\n\tulong _genArg2 = "
                        "{}\n\tClientId client = {}\n)",
                        eventType,
                        shipId,
                        dockTarget,
                        _genArg1,
                        _genArg2,
                        client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestEvent, eventType, shipId, dockTarget, _genArg1, _genArg2, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.RequestEvent(eventType, shipId, dockTarget, _genArg1, _genArg2, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__RequestEvent, eventType, shipId, dockTarget, _genArg1, _genArg2, client);
    }

    void __stdcall RequestCancel(int eventType, uint shipId, uint _genArg1, ulong _genArg2, ClientId client)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"RequestCancel(\n\tint eventType = {}\n\tuint shipId = {}\n\tuint _genArg1 = {}\n\tulong _genArg2 = {}\n\tClientId client = {}\n)",
                        eventType,
                        shipId,
                        _genArg1,
                        _genArg2,
                        client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCancel, eventType, shipId, _genArg1, _genArg2, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.RequestCancel(eventType, shipId, _genArg1, _genArg2, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__RequestCancel, eventType, shipId, _genArg1, _genArg2, client);
    }

    void __stdcall InterfaceItemUsed(uint _genArg1, uint _genArg2)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"InterfaceItemUsed(\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", _genArg1, _genArg2));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InterfaceItemUsed, _genArg1, _genArg2); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.InterfaceItemUsed(_genArg1, _genArg2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__InterfaceItemUsed, _genArg1, _genArg2);
    }

    void __stdcall PopupDialog(ClientId client, uint buttonClicked)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"PopupDialog(\n\tClientId client = {}\n\tuint buttonClicked = {}\n)", client, buttonClicked));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PopupDialog, client, buttonClicked); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.PopUpDialog(client, buttonClicked); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__PopupDialog, client, buttonClicked);
    }

    void __stdcall SetInterfaceState(ClientId client, uint _genArg1, int _genArg2)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"SetInterfaceState(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetInterfaceState, client, _genArg1, _genArg2); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SetInterfaceState(client, (uchar*)_genArg1, _genArg2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__SetInterfaceState, client, _genArg1, _genArg2);
    }

    void __stdcall RequestGroupPositions(ClientId client, uint _genArg1, int _genArg2)
    {
        Logger::i()->Log(
            LogLevel::Trace,
            std::format(L"RequestGroupPositions(\n\tClientId client = {}\n\tuint _genArg1 = 0x{:08X}\n\tint _genArg2 = {}\n)", client, _genArg1, _genArg2));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestGroupPositions, client, _genArg1, _genArg2); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.RequestGroupPositions(client, (uchar*)_genArg1, _genArg2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__RequestGroupPositions, client, _genArg1, _genArg2);
    }

    void __stdcall SetTarget(ClientId client, const XSetTarget& st)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"SetTarget(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTarget, client, st); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SetTarget(client, st); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__SetTarget, client, st);
    }

} // namespace IServerImplHook