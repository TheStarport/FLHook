#include "PCH.hpp"

#include "PurchaseRestrictions.hpp"

namespace Plugins
{

    //! Log items of interest so we can see what cargo cheats people are using.
    void PurchaseRestrictionsPlugin::LogItemsOfInterest(const ClientId& client, GoodId goodId, const std::wstring_view details)
    {
        if (const auto iter = std::ranges::find(config.itemsOfInterest, goodId); iter != config.itemsOfInterest.end())
        {
            const auto charName = client.GetCharacterName().Handle();
            INFO(L"Item '{0}' found in cargo of {1} - {2}", {L"GoodID", std::to_wstring(goodId.GetHash().Unwrap())}, {L"CharName", charName}, {L"Detail", details});
        }
    }

    //! Check that this client is allowed to buy/mount this piece of equipment or ship Return true if the equipment is mounted to allow this good.
    bool PurchaseRestrictionsPlugin::CheckIdEquipRestrictions(ClientId client, GoodId goodId, const bool isShip) const
    {
        const auto list = isShip ? config.shipItemRestrictions : config.goodItemRestrictions;

        const auto validItem = list.find(goodId);
        if (validItem == list.end())
        {
            return true;
        }

        int remainingHoldSize;
        auto items = client.GetEquipCargo().Handle();
        for (auto& item : *items)
        {
            if (item.mounted && std::ranges::find(validItem->second, GoodId(item.archId)) != validItem->second.end())
            {
                return true;
            }
        }

        return false;
    }

    bool PurchaseRestrictionsPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/purchase_restrictions.json");

        return true;
    }

    void PurchaseRestrictionsPlugin::OnClearClientInfo(const ClientId client) { clientSuppressBuy[client] = false; }

    void PurchaseRestrictionsPlugin::OnPlayerLaunch(const ClientId client, const ShipId& ship) { clientSuppressBuy[client] = false; }

    void PurchaseRestrictionsPlugin::OnBaseEnter(BaseId base, const ClientId client) { clientSuppressBuy[client] = false; }

    void PurchaseRestrictionsPlugin::OnGfGoodBuy(ClientId client, const SGFGoodBuyInfo& info)
    {
        auto good = GoodId(info.goodId);

        clientSuppressBuy[client] = false;
        auto& suppress = clientSuppressBuy[client];
        LogItemsOfInterest(client, good, L"good-buy");

        if (std::ranges::find(config.unbuyableItems, good) != config.unbuyableItems.end())
        {
            suppress = true;
            client.PlaySound(CreateID("info_access_denied"));
            client.Message(config.goodPurchaseDenied);
            returnCode = ReturnCode::SkipAll;
            return;
        }

        /// Check restrictions for a piece of equipment that a player has.
        if (!config.checkItemRestrictions)
        {
            return;
        }

        // Check Item
        if (config.goodItemRestrictions.contains(good))
        {
            if (!CheckIdEquipRestrictions(client, good, false))
            {
                const auto charName = client.GetCharacterName().Handle();
                Logger::Info(std::format(L"{} attempting to buy {} without correct Id", charName, info.goodId));
                if (config.enforceItemRestrictions)
                {
                    client.Message(config.goodPurchaseDenied);
                    client.PlaySound(CreateID("info_access_denied"));
                    suppress = true;
                    returnCode = ReturnCode::SkipAll;
                }
            }
        }
        else
        {
            // Check Ship
            const GoodInfo* packageInfo = GoodList::find_by_id(info.goodId);
            if (packageInfo->type != GoodType::Ship)
            {
                return;
            }

            const GoodInfo* hullInfo = GoodList::find_by_id(packageInfo->hullGoodId);
            if (hullInfo->type != GoodType::Hull)
            {
                return;
            }

            if (const auto shipGood = GoodId(hullInfo->shipGoodId);
                config.shipItemRestrictions.contains(shipGood) && !CheckIdEquipRestrictions(client, shipGood, true))
            {
                const auto charName = client.GetCharacterName().Handle();
                Logger::Info(std::format(L"{} attempting to buy {} without correct Id", charName, hullInfo->shipGoodId));
                if (config.enforceItemRestrictions)
                {
                    client.Message(config.shipPurchaseDenied);
                    client.PlaySound(CreateID("info_access_denied"));
                    suppress = true;
                    returnCode = ReturnCode::SkipAll;
                }
            }
        }
    }

    void PurchaseRestrictionsPlugin::OnRequestAddItem(const ClientId client, const GoodId goodId, std::wstring_view hardpoint, int count, float status,
                                                      bool mounted)
    {
        LogItemsOfInterest(client, goodId, L"add-item");
        if (clientSuppressBuy[client])
        {
            returnCode = ReturnCode::SkipAll;
        }
    }

    void PurchaseRestrictionsPlugin::OnRequestChangeCash(const ClientId client, int cash)
    {
        if (clientSuppressBuy[client])
        {
            returnCode = ReturnCode::SkipAll;
        }
    }

    void PurchaseRestrictionsPlugin::OnRequestSetCash(const ClientId client, int cash)
    {
        if (clientSuppressBuy[client])
        {
            clientSuppressBuy[client] = false;
            returnCode = ReturnCode::SkipAll;
        }
    }

    void PurchaseRestrictionsPlugin::OnRequestEquipment(const ClientId client, const EquipDescList& edl)
    {
        if (clientSuppressBuy[client])
        {
            returnCode = ReturnCode::SkipAll;
        }
    }

    void PurchaseRestrictionsPlugin::OnRequestShipArch(const ClientId client, ArchId arch)
    {
        if (clientSuppressBuy[client])
        {
            returnCode = ReturnCode::SkipAll;
        }
    }

    void PurchaseRestrictionsPlugin::OnRequestHullStatus(const ClientId client, float status)
    {
        if (clientSuppressBuy[client])
        {
            clientSuppressBuy[client] = false;
            returnCode = ReturnCode::SkipAll;
        }
    }
    PurchaseRestrictionsPlugin::PurchaseRestrictionsPlugin(const PluginInfo& info) : Plugin(info) {}
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"PurchaseRestrictions", L"purchase_restrictions", PluginMajorVersion::V05, PluginMinorVersion::V00);

SetupPlugin(PurchaseRestrictionsPlugin, Info);
