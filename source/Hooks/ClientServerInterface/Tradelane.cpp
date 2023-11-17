#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void GoTradelaneInner(ClientId client, [[maybe_unused]] const XGoTradelane& gtl)
{
    if (client)
    {
        client.GetData().inTradelane = true;
    }
}

bool GoTradelaneCatch(ClientId client, const XGoTradelane& gtl)
{
    uint system;
    pub::Player::GetSystem(client.GetValue(), system);
    Logger::Log(LogLevel::Trace,
                            std::format(L"Exception in IServerImpl::GoTradelane charname={} sys=0x{:08X} arch=0x{:08X} arch2=0x{:08X}",
                                        client.GetCharacterName().Unwrap(),
                                        system,
                                        gtl.tradelaneSpaceObj1,
                                        gtl.tradelaneSpaceObj2));
    return true;
}

void __stdcall IServerImplHook::GoTradelane(ClientId client, const XGoTradelane& gt)
{
    Logger::Log(LogLevel::Trace, std::format(L"GoTradelane(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnTradelaneStart, client, gt);

    GoTradelaneInner(client, gt);

    if (!skip)
    {
        CallServerPreamble { Server.GoTradelane(client.GetValue(), gt); }
        CallServerPostamble(GoTradelaneCatch(client, gt), );
    }

    CallPlugins(&Plugin::OnTradelaneStartAfter, client, gt);
}

void __stdcall IServerImplHook::StopTradelane(ClientId client, ShipId shipId, ObjectId tradelaneRing1, ObjectId tradelaneRing2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"StopTradelane(\n\tClientId client = {}\n\tuint shipId = {}\n\tuint tradelaneRing1 = {}\n\tuint tradelaneRing2 = {}\n)",
                    client,
                    shipId,
                    tradelaneRing1,
                    tradelaneRing2));

    const auto skip = CallPlugins(&Plugin::OnTradelaneStop, client, shipId, tradelaneRing1, tradelaneRing2);

    if (client)
    {
        client.GetData().inTradelane = false;
    }

    if (!skip)
    {
        CallServerPreamble { Server.StopTradelane(client.GetValue(), shipId.GetValue(), tradelaneRing1.GetValue(), tradelaneRing2.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnTradelaneStopAfter, client, shipId, tradelaneRing1, tradelaneRing2);
}
