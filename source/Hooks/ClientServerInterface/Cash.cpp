#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    void __stdcall ReqSetCash(int cash, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqSetCash(\n\tint cash = {}\n\tClientId client = {}\n)", cash, client));

        if (const auto skip = CallPlugins(&Plugin::OnRequestSetCash, client, cash); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqSetCash(cash, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestSetCashAfter, client, cash);
    }

    void __stdcall ReqChangeCash(int cashAdd, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqChangeCash(\n\tint cashAdd = {}\n\tClientId client = {}\n)", cashAdd, client));

        if (const auto skip = CallPlugins(&Plugin::OnRequestChangeCash, client, cashAdd); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqChangeCash(cashAdd, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnRequestChangeCashAfter, client, cashAdd);
    }

} // namespace IServerImplHook
