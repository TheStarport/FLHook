#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"
#include "Global.hpp"

#include "API/API.hpp"

void InitiateTradeInner(ClientId client1, ClientId client2)
{
    if (client1 <= MaxClientId && client2 <= MaxClientId)
    {
        ClientInfo[client1].tradePartner = client2;
        ClientInfo[client2].tradePartner = client1;
    }
}

void __stdcall IServerImplHook::InitiateTrade(ClientId client1, ClientId client2)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"InitiateTrade(\n\tClientId client1 = {}\n\tClientId client2 = {}\n)", client1, client2));

    const auto skip = CallPlugins(&Plugin::OnInitiateTrade, client1, client2);

    InitiateTradeInner(client1, client2);

    if (!skip)
    {
        CallServerPreamble { Server.InitiateTrade(client1, client2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnInitiateTradeAfter, client1, client2);
}
void __stdcall IServerImplHook::AcceptTrade(ClientId client, bool unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"AcceptTrade(\n\tClientId client = {}\n\tbool unk1 = {}\n)", client, unk1));

    if (const auto skip = CallPlugins(&Plugin::OnAcceptTrade, client, unk1); !skip)
    {
        CallServerPreamble { Server.AcceptTrade(client, unk1); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnAcceptTradeAfter, client, unk1);
}

void __stdcall IServerImplHook::SetTradeMoney(ClientId client, ulong unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"SetTradeMoney(\n\tClientId client = {}\n\tulong unk1 = {}\n)", client, unk1));

    if (const auto skip = CallPlugins(&Plugin::OnSetTradeMoney, client, unk1); !skip)
    {
        CallServerPreamble { Server.SetTradeMoney(client, unk1); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnSetTradeMoneyAfter, client, unk1);
}

void __stdcall IServerImplHook::AddTradeEquip(ClientId client, const EquipDesc& ed)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"AddTradeEquip(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnAddTradeEquip, client, ed); !skip)
    {
        CallServerPreamble { Server.AddTradeEquip(client, ed); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnAddTradeEquipAfter, client, ed);
}

void __stdcall IServerImplHook::DelTradeEquip(ClientId client, const EquipDesc& ed)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"DelTradeEquip(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnRemoveTradeEquip, client, ed); !skip)
    {
        CallServerPreamble { Server.DelTradeEquip(client, ed); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRemoveTradeEquipAfter, client, ed);
}

void __stdcall IServerImplHook::RequestTrade(uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"RequestTrade(\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", unk1, unk2));

    if (const auto skip = CallPlugins(&Plugin::OnRequestTrade, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.RequestTrade(unk1, unk2); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnRequestTradeAfter, unk1, unk2);
}

void __stdcall IServerImplHook::StopTradeRequest(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"StopTradeRequest(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnStopTradeRequest, client); !skip)
    {
        CallServerPreamble { Server.StopTradeRequest(client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnStopTradeRequestAfter, client);
}

void __stdcall IServerImplHook::TradeResponse(const unsigned char* unk1, int unk2, ClientId client)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"TradeResponse(\n\tunsigned char const* unk1 = {}\n\tint unk2 = {}\n\tClientId client = {}\n)",
                                 StringUtils::stows(std::string(reinterpret_cast<const char*>(unk1))),
                                 unk2,
                                 client));

    if (const auto skip = CallPlugins(&Plugin::OnTradeResponse, client, unk1, unk2); !skip)
    {
        CallServerPreamble { Server.TradeResponse(unk1, unk2, client); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnTradeResponseAfter, client, unk1, unk2);
}

void TerminateTradeInnerAfter(ClientId client, int accepted)
{
    TryHook
    {
        if (accepted)
        {
            // save both chars to prevent cheating in case of server crash
            Hk::Player::SaveChar(client);
            if (ClientInfo::At(client).tradePartner)
            {
                Hk::Player::SaveChar(ClientInfo::At(client).tradePartner);
            }
        }

        if (ClientInfo::At(client).tradePartner)
        {
            ClientInfo[ClientInfo::At(client).tradePartner].tradePartner = 0;
        }
        ClientInfo::At(client).tradePartner = 0;
    }
    CatchHook({})
}

void __stdcall IServerImplHook::TerminateTrade(ClientId client, int accepted)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"TerminateTrade(\n\tClientId client = {}\n\tint accepted = {}\n)", client, accepted));

    const auto skip = CallPlugins(&Plugin::OnTerminateTrade, client, accepted);

    CheckForDisconnect;

    if (!skip)
    {
        CallServerPreamble { Server.TerminateTrade(client, accepted); }
        CallServerPostamble(true, );
    }
    TerminateTradeInnerAfter(client, accepted);

    CallPlugins(&Plugin::OnTerminateTradeAfter, client, accepted);
}
