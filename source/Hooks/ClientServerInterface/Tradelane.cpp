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
    TRACE("{{characterName}} {{system}} {{tradelaneSpaceObj1}} {{tradelaneSpaceObj2}}",
          { "characterName", std::wstring(client.GetCharacterName().Unwrap()) },
          { "system", system },
          { "tradelaneSpaceObj1", gtl.tradelaneSpaceObj1 },
          { "tradelaneSpaceObj2", gtl.tradelaneSpaceObj2 });

    return true;
}

void __stdcall IServerImplHook::GoTradelane(ClientId client, const XGoTradelane& gt)
{
    TRACE("{{client}}", { "client", client });

    const auto skip = CallPlugins(&Plugin::OnTradelaneStart, client, gt);

    GoTradelaneInner(client, gt);

    if (!skip)
    {
        CallServerPreamble { Server.GoTradelane(client.GetValue(), gt); }
        CallServerPostamble(GoTradelaneCatch(client, gt), );
    }

    CallPlugins(&Plugin::OnTradelaneStartAfter, client, gt);
}

void __stdcall IServerImplHook::StopTradelane(ClientId client, Id shipId, Id tradelaneRing1, Id tradelaneRing2)
{
    auto ship = shipId.AsShip();
    auto tradeLane1 = tradelaneRing1.AsObject();
    auto tradeLane2 = tradelaneRing2.AsObject();

    TRACE("{{client}} {{shipId}} {{tradelaneRing1}} {{tradelaneRing2}}",
          { "client", client },
          { "shipId", shipId },
          { "tradelaneRing1", tradeLane1.GetId().Unwrap() },
          { "tradelaneRing2", tradeLane2.GetId().Unwrap() });

    const auto skip = CallPlugins(&Plugin::OnTradelaneStop, client, ship, tradeLane1, tradeLane2);

    if (client)
    {
        client.GetData().inTradelane = false;
    }

    if (!skip)
    {
        CallServerPreamble { Server.StopTradelane(client.GetValue(), shipId.GetValue(), tradelaneRing1.GetValue(), tradelaneRing2.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnTradelaneStopAfter, client, ship, tradeLane1, tradeLane2);
}
