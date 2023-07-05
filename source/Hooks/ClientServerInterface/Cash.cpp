#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    void __stdcall ReqSetCash(int cash, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqSetCash(\n\tint cash = {}\n\tClientId client = {}\n)", cash, client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqSetCash, cash, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqSetCash(cash, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__ReqSetCash, cash, client);
    }

    void __stdcall ReqChangeCash(int cashAdd, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"ReqChangeCash(\n\tint cashAdd = {}\n\tClientId client = {}\n)", cashAdd, client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqChangeCash, cashAdd, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.ReqChangeCash(cashAdd, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__ReqChangeCash, cashAdd, client);
    }

} // namespace IServerImplHook