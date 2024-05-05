#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"

bool IServerImplHook::GFGoodSellInner(const SGFGoodSellInfo& gsi, ClientId client)
{
    TryHook
    {
        // anti-cheat check

        int _;
        const auto cargoList = client.EnumCargo(_).Raw();
        bool legalSell = false;
        for (const auto& cargo : cargoList.value())
        {
            if (cargo.archId == gsi.archId)
            {
                legalSell = true;
                if (abs(gsi.count) > cargo.count)
                {
                    auto charName = client.GetCharacterName().Handle();
                    // AddCheaterLog(charName, std::format(L"Sold more good than possible item={} count={}", gsi.archId, gsi.count));

                    FLHook::MessageUniverse(std::format(L"Possible cheating detected ({})", charName));
                    client.Kick();
                    client.GetAccount().Unwrap().Ban();
                    return false;
                }
                break;
            }
        }
        if (!legalSell)
        {
            // AddCheaterLog(charName, std::format(L"Sold good player does not have (buggy test), item={}", gsi.archId));

            return false;
        }
    }
    CatchHook({
        Logger::Trace(std::format(L"Exception in {} (client={} ({}))", FUNCTION_W, client, client.GetCharacterName().Unwrap()));
    })

        return true;
}

void __stdcall IServerImplHook::GFGoodSell(const SGFGoodSellInfo& unk1, ClientId client)
{
    Logger::Trace(std::format(L"GFGoodSell(\n\tClientId client = {}\n)", client));

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

void __stdcall IServerImplHook::GFGoodBuy(const SGFGoodBuyInfo& unk1, ClientId client)
{
    Logger::Trace(std::format(L"GFGoodBuy(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnGfGoodBuy, client, unk1); !skip)
    {
        CallServerPreamble { Server.GFGoodBuy(unk1, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnGfGoodBuyAfter, client, unk1);
}

void __stdcall IServerImplHook::GFGoodVaporized(const SGFGoodVaporizedInfo& gvi, ClientId client)
{
    Logger::Trace(std::format(L"GFGoodVaporized(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnGfGoodVaporized, client, gvi); !skip)
    {
        CallServerPreamble { Server.GFGoodVaporized(gvi, client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnGfGoodVaporizedAfter, client, gvi);
}
