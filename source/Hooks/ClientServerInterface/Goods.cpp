#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/ExceptionHandler.hpp"
#include "Core/PluginManager.hpp"
#include "Exceptions/StopProcessingException.hpp"

bool IServerImplHook::GFGoodSellInner(const SGFGoodSellInfo& gsi, ClientId client)
{
    TryHook
    {
        // anti-cheat check

        const auto cargoList = client.GetEquipCargo().Raw();
        bool legalSell = false;
        for (const auto& cargo : cargoList.value()->equip)
        {
            if (cargo.archId != gsi.archId)
            {
                continue;
            }
            legalSell = true;
            if (abs(gsi.count) > cargo.count)
            {
                auto charName = client.GetCharacterId().Handle();
                // AddCheaterLog(charName, std::format(L"Sold more good than possible item={} count={}", gsi.archId, gsi.count));

                FLHook::MessageUniverse(std::format(L"Possible cheating detected ({})", charName));
                (void)client.Kick();
                (void)client.GetAccount().Unwrap().Ban();
                return false;
            }
            break;
        }

        if (!legalSell)
        {
            // AddCheaterLog(charName, std::format(L"Sold good player does not have (buggy test), item={}", gsi.archId));

            return false;
        }

        GoodId good = GoodId(gsi.archId.GetValue());
        if (good.GetType().Unwrap() != GoodType::Commodity && good.GetType().Unwrap() != GoodType::Equipment)
        {
            return true;
        }

        BaseId base = client.GetCurrentBase().Unwrap();
        auto iter = clientState[client].clientSales.find(good.GetHash().Unwrap().GetValue());
        if (iter == clientState[client].clientSales.end())
        {
            clientState[client].clientSales[good.GetHash().Unwrap().GetValue()] = gsi.count;
        }
        else
        {
            iter->second += gsi.count;
        }
    }

    CatchHook({ TRACE("Exception {{client}} {{characterName}}", { "client", client }, { "characterName", client.GetCharacterId().Unwrap() }); });

    return true;
}

void __stdcall IServerImplHook::GFGoodSell(const SGFGoodSellInfo& unk1, ClientId client)
{
    TRACE("IServerImplHook::GFGoodSell client={{client}}", { "client", client });

    const auto skip = CallPlugins(&Plugin::OnGfGoodSell, client, unk1);

    CheckForDisconnect;

    if (const bool innerCheck = GFGoodSellInner(unk1, client); !innerCheck)
    {
        return;
    }
    if (!skip)
    {
        CallServerPreamble { Server.GFGoodSell(unk1, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnGfGoodSellAfter, client, unk1);
}
bool IServerImplHook::CheckBuybackCache(ClientId client, const SGFGoodBuyInfo& gsi)
{
    auto clientStateIter = clientState.find(client);
    if (clientStateIter != clientState.end())
    {
        auto itemIter = clientStateIter->second.clientSales.find(gsi.goodId.GetValue());
        if (itemIter != clientStateIter->second.clientSales.end())
        {
            if (itemIter->second > gsi.count)
            {
                itemIter->second -= gsi.count;
                return true;
            }
        }
    }

    return false;
}

bool IServerImplHook::GFGoodBuyInner(const SGFGoodBuyInfo& gsi, ClientId client)
{

    auto baseData = BaseDataList_get()->get_base_data(gsi.baseId.GetValue());
    if (!baseData)
    {
        return CheckBuybackCache(client, gsi);
    }
    auto iter = baseData->marketMap.find(Id(gsi.goodId));
    if (iter == baseData->marketMap.end())
    {
        return CheckBuybackCache(client, gsi);
    }
    if (!iter->second.stock)
    {
        return CheckBuybackCache(client, gsi);
    }

    return true;
}

void __stdcall IServerImplHook::GFGoodBuy(const SGFGoodBuyInfo& unk1, ClientId client)
{
    TRACE("IServerImplHook::GFGoodBuy client={{client}}", { "client", client });

    if (const bool innerCheck = GFGoodBuyInner(unk1, client); !innerCheck)
    {
        WARN("{{characterName}} has attempted an impossible purchase", { "characterName", client.GetCharacterId().Unwrap() });
        client.Kick();
        return;
    }

    if (const auto skip = CallPlugins(&Plugin::OnGfGoodBuy, client, unk1); !skip)
    {
        CallServerPreamble { Server.GFGoodBuy(unk1, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnGfGoodBuyAfter, client, unk1);
}

void __stdcall IServerImplHook::GFGoodVaporized(const SGFGoodVaporizedInfo& gvi, ClientId client)
{
    TRACE("IServerImplHook::GFGoodVaporized client={{client}}", { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnGfGoodVaporized, client, gvi); !skip)
    {
        CallServerPreamble { Server.GFGoodVaporized(gvi, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnGfGoodVaporizedAfter, client, gvi);
}
