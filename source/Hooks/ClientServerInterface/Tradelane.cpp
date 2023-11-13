#include "PCH.hpp"

#include "API/API.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

void GoTradelaneInner(ClientId client, [[maybe_unused]] const XGoTradelane& gtl)
{
    if (client <= MaxClientId && client > 0)
    {
        ClientInfo::At(client).tradelane = true;
    }
}

bool GoTradelaneCatch(ClientId client, const XGoTradelane& gtl)
{
    uint system;
    pub::Player::GetSystem(client, system);
    FLHook::GetLogger().Log(LogLevel::Trace,
                     std::format(L"Exception in IServerImpl::GoTradelane charname={} sys=0x{:08X} arch=0x{:08X} arch2=0x{:08X}",
                                 client.GetCharacterName().Unwrap(),
                                 system,
                                 gtl.tradelaneSpaceObj1,
                                 gtl.tradelaneSpaceObj2));
    return true;
}

void __stdcall IServerImplHook::GoTradelane(ClientId client, const XGoTradelane& gt)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"GoTradelane(\n\tClientId client = {}\n)", client));

    const auto skip = CallPlugins(&Plugin::OnTradelaneStart, client, gt);

    GoTradelaneInner(client, gt);

    if (!skip)
    {
        CallServerPreamble { Server.GoTradelane(client, gt); }
        CallServerPostamble(GoTradelaneCatch(client, gt), );
    }

    CallPlugins(&Plugin::OnTradelaneStartAfter, client, gt);
}

void StopTradelaneInner(ClientId client, uint, uint, uint)
{
    if (client <= MaxClientId && client > 0)
    {
        ClientInfo::At(client).tradelane = false;
    }
}

void __stdcall IServerImplHook::StopTradelane(ClientId client, uint shipId, uint tradelaneRing1, uint tradelaneRing2)
{
    FLHook::GetLogger().Log(LogLevel::Trace,
                     std::format(L"StopTradelane(\n\tClientId client = {}\n\tuint shipId = {}\n\tuint tradelaneRing1 = {}\n\tuint tradelaneRing2 = {}\n)",
                                 client,
                                 shipId,
                                 tradelaneRing1,
                                 tradelaneRing2));

    const auto skip = CallPlugins(&Plugin::OnTradelaneStop, client, shipId, tradelaneRing1, tradelaneRing2);

    StopTradelaneInner(client, shipId, tradelaneRing1, tradelaneRing2);

    if (!skip)
    {
        CallServerPreamble { Server.StopTradelane(client, shipId, tradelaneRing1, tradelaneRing2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnTradelaneStopAfter, client, shipId, tradelaneRing1, tradelaneRing2);
}
