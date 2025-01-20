#include "PCH.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void __stdcall IServerImplHook::ReqSetCash(int cash, ClientId client)
{

    TRACE(L"ReqSetCash({0} {1}.)", { L"cash", std::to_wstring(cash) }, { L"clientId", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnRequestSetCash, client, cash); !skip)
    {
        CallServerPreamble { Server.ReqSetCash(cash, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestSetCashAfter, client, cash);
}

void __stdcall IServerImplHook::ReqChangeCash(int cashAdd, ClientId client)
{
    TRACE(L"ReqChangeCash({}{})", { L"cash added", std::to_wstring(cashAdd) }, { L"client ", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnRequestChangeCash, client, cashAdd); !skip)
    {
        CallServerPreamble { Server.ReqChangeCash(cashAdd, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestChangeCashAfter, client, cashAdd);
}
