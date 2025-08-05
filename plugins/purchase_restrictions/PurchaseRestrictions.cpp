#include "PCH.hpp"

#include "PurchaseRestrictions.hpp"

namespace Plugins
{

    //! Log items of interest so we can see what cargo cheats people are using.
    void PurchaseRestrictionsPlugin::LogItemsOfInterest(const ClientId& client, Id goodId, const std::wstring_view details)
    {
        if (const auto iter = std::ranges::find(config.itemsOfInterest, goodId); iter != config.itemsOfInterest.end())
        {
            const auto charName = client.GetCharacterId().Handle();
            INFO("Item '{{goodId}}' found in cargo of {{characterName}} - {{details}}"),
                JsonLogFormatter({
                    {        "goodId",   goodId },
                    { "characterName", charName },
                    {       "details",  details }
            });
        }
    }

    //! Check that this client is allowed to buy/mount this piece of equipment or ship Return true if the equipment is mounted to allow this good.
    bool PurchaseRestrictionsPlugin::CheckIdEquipRestrictions(ClientId client, Id goodId, const bool isShip) const
    {
        const auto list = isShip ? config.shipItemRestrictions : config.goodItemRestrictions;

        const auto validItem = list.find(goodId);
        if (validItem == list.end())
        {
            return true;
        }

        int remainingHoldSize;
        auto items = client.GetEquipCargo().Handle();
        for (auto& item : items->equip)
        {
            if (item.mounted && std::ranges::find(validItem->second, item.archId) != validItem->second.end())
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

        clientSuppressBuy[client] = false;
        auto& suppress = clientSuppressBuy[client];
        LogItemsOfInterest(client, info.goodId, L"good-buy");

        if (std::ranges::find(config.unbuyableItems, info.goodId) != config.unbuyableItems.end())
        {
            suppress = true;
            client.PlaySound(Id("info_access_denied"));
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
        if (config.goodItemRestrictions.contains(info.goodId))
        {
            if (!CheckIdEquipRestrictions(client, info.goodId, false))
            {
                const auto charName = client.GetCharacterId().Handle();
                INFO("{{characterName}} attempting to buy {{goodId}} without correct Id", { "characterName", charName }, { "goodId", info.goodId });

                if (config.enforceItemRestrictions)
                {
                    client.Message(config.goodPurchaseDenied);
                    client.PlaySound(Id("info_access_denied"));
                    suppress = true;
                    returnCode = ReturnCode::SkipAll;
                }
            }
        }
        else
        {
            // Check Ship
            const GoodInfo* packageInfo = GoodList::find_by_id(info.goodId.GetValue());
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
                config.shipItemRestrictions.contains(shipGood.GetHash().Unwrap()) && !CheckIdEquipRestrictions(client, shipGood.GetHash().Unwrap(), true))
            {
                const auto charName = client.GetCharacterId().Handle();
                INFO(
                    "{{characterName}} attempting to buy {{shipGood}} without correct Id", { "characterName", charName }, { "shipGood", hullInfo->shipGoodId });
                if (config.enforceItemRestrictions)
                {
                    client.Message(config.shipPurchaseDenied);
                    client.PlaySound(Id("info_access_denied"));
                    suppress = true;
                    returnCode = ReturnCode::SkipAll;
                }
            }
        }
    }

    void PurchaseRestrictionsPlugin::OnRequestAddItem(const ClientId client, GoodId& goodId, std::wstring_view hardpoint, int count, float status, bool mounted)
    {
        LogItemsOfInterest(client, goodId.GetHash().Unwrap(), L"add-item");
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

// clang-format off
constexpr auto getPi = []
{
    return PluginInfo{
        .name = L"PurchaseRestrictions",
        .shortName = L"purchase_restrictions",
        .versionMajor = PluginMajorVersion::V05,
        .versionMinor = PluginMinorVersion::V00
    };
};


SetupPlugin(PurchaseRestrictionsPlugin);
