#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/ResourceManager.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"

void IServerImplHook::PlayerLaunchInner(const ShipId& shipId, ClientId client)
{
    TryHook
    {
        auto& data = client.GetData();
        data.ship = shipId;
        data.shipId = Id(shipId.GetId().Unwrap());
        data.cruiseActivated = false;
        data.thrusterActivated = false;
        data.engineKilled = false;
        data.inTradelane = false;

        // adjust cash, this is necessary when cash was added while use was in charmenu/had other char selected
        const std::wstring charName = StringUtils::ToLower(client.GetCharacterName().Handle());
        for (const auto& i : data.moneyFix)
        {
            if (i.character == charName)
            {
                (void)client.AddCash(i.amount);
                data.moneyFix.remove(i);
                break;
            }
        }
    }
    CatchHook({})
}

void __stdcall IServerImplHook::PlayerLaunch(Id shipId, ClientId client)
{
    auto ship = shipId.AsShip();

    TRACE(L"{0} {1}}", { L"shipId", std::to_wstring(shipId.GetValue()) }, { L"client", std::to_wstring(client.GetValue()) });

    const auto skip = CallPlugins(&Plugin::OnPlayerLaunch, client, ship);

    CheckForDisconnect;

    PlayerLaunchInner(ship, client);

    ResourceManager::playerShips.insert(shipId.GetValue());
    if (!skip)
    {
        CallServerPreamble { Server.PlayerLaunch(shipId.GetValue(), client.GetValue()); }
        CallServerPostamble(true, );
    }

    CallPlugins(&Plugin::OnPlayerLaunchAfter, client, ship);
}
