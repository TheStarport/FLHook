#include "PCH.hpp"

#include "Global.hpp"
#include "Core/ClientServerInterface.hpp"

#include "API/API.hpp"

namespace IServerImplHook
{
    void InitiateTrade__Inner(ClientId client1, ClientId client2)
    {
        if (client1 <= MaxClientId && client2 <= MaxClientId)
        {
            ClientInfo[client1].tradePartner = client2;
            ClientInfo[client2].tradePartner = client1;
        }
    }

    void __stdcall InitiateTrade(ClientId client1, ClientId client2)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"InitiateTrade(\n\tClientId client1 = {}\n\tClientId client2 = {}\n)", client1, client2));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InitiateTrade, client1, client2);

        InitiateTrade__Inner(client1, client2);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.InitiateTrade(client1, client2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__InitiateTrade, client1, client2);
    }
    void __stdcall AcceptTrade(ClientId client, bool _genArg1)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"AcceptTrade(\n\tClientId client = {}\n\tbool _genArg1 = {}\n)", client, _genArg1));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AcceptTrade, client, _genArg1); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.AcceptTrade(client, _genArg1); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__AcceptTrade, client, _genArg1);
    }

    void __stdcall SetTradeMoney(ClientId client, ulong _genArg1)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"SetTradeMoney(\n\tClientId client = {}\n\tulong _genArg1 = {}\n)", client, _genArg1));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTradeMoney, client, _genArg1); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.SetTradeMoney(client, _genArg1); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__SetTradeMoney, client, _genArg1);
    }

    void __stdcall AddTradeEquip(ClientId client, const EquipDesc& ed)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"AddTradeEquip(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AddTradeEquip, client, ed); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.AddTradeEquip(client, ed); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__AddTradeEquip, client, ed);
    }

    void __stdcall DelTradeEquip(ClientId client, const EquipDesc& ed)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"DelTradeEquip(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DelTradeEquip, client, ed); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.DelTradeEquip(client, ed); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__DelTradeEquip, client, ed);
    }

    void __stdcall RequestTrade(uint _genArg1, uint _genArg2)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"RequestTrade(\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", _genArg1, _genArg2));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestTrade, _genArg1, _genArg2); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.RequestTrade(_genArg1, _genArg2); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__RequestTrade, _genArg1, _genArg2);
    }

    void __stdcall StopTradeRequest(ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"StopTradeRequest(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradeRequest, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.StopTradeRequest(client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__StopTradeRequest, client);
    }

    void __stdcall TradeResponse(const unsigned char* _genArg1, int _genArg2, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace,
                         std::format(L"TradeResponse(\n\tunsigned char const* _genArg1 = {}\n\tint _genArg2 = {}\n\tClientId client = {}\n)",
                                     StringUtils::stows(std::string(reinterpret_cast<const char*>(_genArg1))),
                                     _genArg2,
                                     client));

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TradeResponse, _genArg1, _genArg2, client); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.TradeResponse(_genArg1, _genArg2, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPluginsAfter(HookedCall::IServerImpl__TradeResponse, _genArg1, _genArg2, client);
    }
    void TerminateTrade__InnerAfter(ClientId client, int accepted)
    {
        TRY_HOOK
        {
            if (accepted)
            {
                // save both chars to prevent cheating in case of server crash
                Hk::Player::SaveChar(client);
                if (ClientInfo[client].tradePartner)
                {
                    Hk::Player::SaveChar(ClientInfo[client].tradePartner);
                }
            }

            if (ClientInfo[client].tradePartner)
            {
                ClientInfo[ClientInfo[client].tradePartner].tradePartner = 0;
            }
            ClientInfo[client].tradePartner = 0;
        }
        CATCH_HOOK({})
    }
    void __stdcall TerminateTrade(ClientId client, int accepted)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"TerminateTrade(\n\tClientId client = {}\n\tint accepted = {}\n)", client, accepted));

        const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TerminateTrade, client, accepted);

        CHECK_FOR_DISCONNECT;

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.TerminateTrade(client, accepted); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        TerminateTrade__InnerAfter(client, accepted);

        CallPluginsAfter(HookedCall::IServerImpl__TerminateTrade, client, accepted);
    }

} // namespace IServerImplHook