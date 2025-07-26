#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::ReqSetCash(int cash, ClientId client)
{

    TRACE("{{cash}} {{clientId}}", { "cash", cash }, { "clientId", client });

    if (const auto skip = CallPlugins(&Plugin::OnRequestSetCash, client, cash); !skip)
    {
        CallServerPreamble { Server.ReqSetCash(cash, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestSetCashAfter, client, cash);
}

void __stdcall IServerImplHook::ReqChangeCash(int cashAdd, ClientId client)
{
    TRACE("{{cashAdd}} {{client}}", { "cashAdd", cashAdd }, { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnRequestChangeCash, client, cashAdd); !skip)
    {
        CallServerPreamble { Server.ReqChangeCash(cashAdd, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestChangeCashAfter, client, cashAdd);
}
