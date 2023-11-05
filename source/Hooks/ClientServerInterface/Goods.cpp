#include "PCH.hpp"

#include "API/API.hpp"
#include "Global.hpp"

namespace IServerImplHook
{
    bool GFGoodSell__Inner(const SGFGoodSellInfo& gsi, ClientId client)
    {
        TRY_HOOK
        {
            // anti-cheat check

            int _;
            const auto cargoList = Hk::Player::EnumCargo(client, _).Raw();
            bool legalSell = false;
            for (const auto& cargo : cargoList.value())
            {
                if (cargo.archId == gsi.archId)
                {
                    legalSell = true;
                    if (abs(gsi.count) > cargo.count)
                    {
                        const std::wstring charName = Hk::Client::GetCharacterNameByID(client).Handle();
                        // AddCheaterLog(charName, std::format(L"Sold more good than possible item={} count={}", gsi.archId, gsi.count));

                        Hk::Chat::MsgU(std::format(L"Possible cheating detected ({})", charName));
                        Hk::Player::Ban(client, true);
                        Hk::Player::Kick(client);
                        return false;
                    }
                    break;
                }
            }
            if (!legalSell)
            {
                const std::wstring charName = Hk::Client::GetCharacterNameByID(client).Handle();
                // AddCheaterLog(charName, std::format(L"Sold good player does not have (buggy test), item={}", gsi.archId));

                return false;
            }
        }
        CATCH_HOOK({
            Logger::i()->Log(
                LogLevel::Trace,
                std::format(L"Exception in {} (client={} ({}))", StringUtils::stows(__FUNCTION__), client, Hk::Client::GetCharacterNameByID(client).Unwrap()));
        })

        return true;
    }
    void __stdcall GFGoodSell(const SGFGoodSellInfo& _genArg1, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"GFGoodSell(\n\tClientId client = {}\n)", client));

        const auto skip = CallPlugins(&Plugin::OnGfGoodSell, client, _genArg1);

        CHECK_FOR_DISCONNECT;

        if (const bool innerCheck = GFGoodSell__Inner(_genArg1, client); !innerCheck)
        {
            return;
        }
        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.GFGoodSell(_genArg1, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnGfGoodSellAfter, client, _genArg1);
    }

    void __stdcall GFGoodBuy(const SGFGoodBuyInfo& _genArg1, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"GFGoodBuy(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPlugins(&Plugin::OnGfGoodBuy, client, _genArg1); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.GFGoodBuy(_genArg1, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnGfGoodBuyAfter, client, _genArg1);
    }

    void __stdcall GFGoodVaporized(const SGFGoodVaporizedInfo& gvi, ClientId client)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"GFGoodVaporized(\n\tClientId client = {}\n)", client));

        if (const auto skip = CallPlugins(&Plugin::OnGfGoodVaporized, client, gvi); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.GFGoodVaporized(gvi, client); }
            CALL_SERVER_POSTAMBLE(true, );
        }

        CallPlugins(&Plugin::OnGfGoodVaporizedAfter, client, gvi);
    }

} // namespace IServerImplHook
