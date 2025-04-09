#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

void InitiateTradeInner(const ClientId client1, const ClientId client2)
{
    if (client1 && client2)
    {
        auto& clients = FLHook::Clients();
        clients[client1].tradePartner = client2;
        clients[client2].tradePartner = client1;
    }
}

void __stdcall IServerImplHook::InitiateTrade(ClientId client1, ClientId client2)
{
    TRACE(L"{0} {1}", { L"client1", std::to_wstring(client1.GetValue()) }, { L"client2", std::to_wstring(client2.GetValue()) });

    const auto skip = CallPlugins(&Plugin::OnInitiateTrade, client1, client2);

    InitiateTradeInner(client1, client2);

    if (!skip)
    {
        CallServerPreamble { Server.InitiateTrade(client1.GetValue(), client2.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnInitiateTradeAfter, client1, client2);
}
void __stdcall IServerImplHook::AcceptTrade(ClientId client, bool unk1)
{
    TRACE(L"{0} {1}", { L"client", std::to_wstring(client.GetValue()) }, { L"unk1", std::to_wstring(unk1) });

    if (const auto skip = CallPlugins(&Plugin::OnAcceptTrade, client, unk1); !skip)
    {
        CallServerPreamble { Server.AcceptTrade(client.GetValue(), unk1); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnAcceptTradeAfter, client, unk1);
}

void __stdcall IServerImplHook::SetTradeMoney(ClientId client, ulong unk1)
{
    TRACE(L"{0} {1}", { L"client", std::to_wstring(client.GetValue()) }, { L"unk1", std::to_wstring(unk1) });

    if (const auto skip = CallPlugins(&Plugin::OnSetTradeMoney, client, unk1); !skip)
    {
        CallServerPreamble { Server.SetTradeMoney(client.GetValue(), unk1); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetTradeMoneyAfter, client, unk1);
}

void __stdcall IServerImplHook::AddTradeEquip(ClientId client, const EquipDesc& ed)
{
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnAddTradeEquip, client, ed); !skip)
    {
        CallServerPreamble { Server.AddTradeEquip(client.GetValue(), ed); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnAddTradeEquipAfter, client, ed);
}

void __stdcall IServerImplHook::DelTradeEquip(ClientId client, const EquipDesc& ed)
{
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) })

    if (const auto skip = CallPlugins(&Plugin::OnRemoveTradeEquip, client, ed); !skip)
    {
        CallServerPreamble { Server.DelTradeEquip(client.GetValue(), ed); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRemoveTradeEquipAfter, client, ed);
}

void __stdcall IServerImplHook::RequestTrade(ClientId client1, ClientId client2)
{
    TRACE(L"{0} {1}", { L"client1", std::to_wstring(client1.GetValue()) }, { L"client2", std::to_wstring(client2.GetValue()) });

    if (const auto skip = CallPlugins(&Plugin::OnRequestTrade, client1, client2); !skip)
    {
        CallServerPreamble { Server.RequestTrade(client1.GetValue(), client2.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestTradeAfter, client1, client2);
}

void __stdcall IServerImplHook::StopTradeRequest(ClientId client)
{
    TRACE(L"{0}", { L"client", std::to_wstring(client.GetValue()) })

    if (const auto skip = CallPlugins(&Plugin::OnStopTradeRequest, client); !skip)
    {
        CallServerPreamble { Server.StopTradeRequest(client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnStopTradeRequestAfter, client);
}

void __stdcall IServerImplHook::TradeResponse(const unsigned char* unk1, int unk2, ClientId client)
{
    TRACE(L"{0} {1} {2}",
          { L"unk1", StringUtils::stows(std::string(reinterpret_cast<const char*>(unk1))) },
          { L"unk2", std::to_wstring(unk2) },
          { L"client", std::to_wstring(client.GetValue()) })

    if (const auto skip = CallPlugins(&Plugin::OnTradeResponse, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.TradeResponse(unk1, unk2, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnTradeResponseAfter, client, unk1, unk2);
}

void IServerImplHook::TerminateTradeInnerAfter(ClientId client, const int accepted)
{
    TryHook
    {
        auto partner = client.GetData().tradePartner;
        if (accepted)
        {
            // save both chars to prevent cheating in case of server crash
            client.SaveChar();
            if (partner)
            {
                partner.SaveChar();
            }
        }

        if (partner)
        {
            partner.GetData().tradePartner = ClientId();
        }

        client.GetData().tradePartner = ClientId();
    }
    CatchHook({})
}

void __stdcall IServerImplHook::TerminateTrade(ClientId client, int accepted)
{
    TRACE(L"{0} {1}", { L"client", std::to_wstring(client.GetValue()) }, { L"accepted", std::to_wstring(accepted) });

    const auto skip = CallPlugins(&Plugin::OnTerminateTrade, client, accepted);

    CheckForDisconnect;

    if (!skip)
    {
        CallServerPreamble { Server.TerminateTrade(client.GetValue(), accepted); }
        CallServerPostamble(true, );
    }
    TerminateTradeInnerAfter(client, accepted);

    CallPlugins(&Plugin::OnTerminateTradeAfter, client, accepted);
}
