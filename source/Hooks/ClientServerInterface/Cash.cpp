#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

void __stdcall IServerImplHook::ReqSetCash(int cash, ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"ReqSetCash(\n\tint cash = {}\n\tClientId client = {}\n)", cash, client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestSetCash, client, cash); !skip)
    {
        CallServerPreamble { Server.ReqSetCash(cash, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestSetCashAfter, client, cash);
}

void __stdcall IServerImplHook::ReqChangeCash(int cashAdd, ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"ReqChangeCash(\n\tint cashAdd = {}\n\tClientId client = {}\n)", cashAdd, client));

    if (const auto skip = CallPlugins(&Plugin::OnRequestChangeCash, client, cashAdd); !skip)
    {
        CallServerPreamble { Server.ReqChangeCash(cashAdd, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestChangeCashAfter, client, cashAdd);
}
