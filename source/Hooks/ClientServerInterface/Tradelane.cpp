#include "PCH.hpp"

#include "Global.hpp"
#include "API/API.hpp"

namespace IServerImplHook
{
    void GoTradelane__Inner(ClientId client, [[maybe_unused]] const XGoTradelane& gtl)
    {
        if (client <= MaxClientId && client > 0)
        {
            ClientInfo[client].tradelane = true;
        }
    }

    bool GoTradelane__Catch(ClientId client, const XGoTradelane& gtl)
    {
        uint system;
        pub::Player::GetSystem(client, system);
        Logger::i()->Log(LogLevel::Trace,
                         std::format(L"Exception in IServerImpl::GoTradelane charname={} sys=0x{:08X} arch=0x{:08X} arch2=0x{:08X}",
                                     Hk::Client::GetCharacterNameByID(client).Unwrap(),
                                     system,
                                     gtl.tradelaneSpaceObj1,
                                     gtl.tradelaneSpaceObj2));
        return true;
    }

    void __stdcall GoTradelane(ClientId client, const XGoTradelane& gt)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"GoTradelane(\n\tClientId client = {}\n)", client));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GoTradelane, client, gt);

        GoTradelane__Inner(client, gt);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.GoTradelane(client, gt); }
            CALL_SERVER_POSTAMBLE(GoTradelane__Catch(client, gt), );
        }

        CallPluginsAfter(HookedCall::IServerImpl__GoTradelane, client, gt);
    }

    void StopTradelane__Inner(ClientId client, uint, uint, uint)
    {
        if (client <= MaxClientId && client > 0)
        {
            ClientInfo[client].tradelane = false;
        }
    }

    void __stdcall StopTradelane(ClientId client, uint shipId, uint tradelaneRing1, uint tradelaneRing2)
    {
        Logger::i()->Log(LogLevel::Trace,
                         std::format(L"StopTradelane(\n\tClientId client = {}\n\tuint shipId = {}\n\tuint tradelaneRing1 = {}\n\tuint tradelaneRing2 = {}\n)",
                                     client,
                                     shipId,
                                     tradelaneRing1,
                                     tradelaneRing2));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradelane, client, shipId, tradelaneRing1, tradelaneRing2);

        StopTradelane__Inner(client, shipId, tradelaneRing1, tradelaneRing2);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.StopTradelane(client, shipId, tradelaneRing1, tradelaneRing2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__StopTradelane, client, shipId, tradelaneRing1, tradelaneRing2);
    }
} // namespace IServerImplHook
