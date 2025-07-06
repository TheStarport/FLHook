#include "PCH.hpp"

#include "MarketController.hpp"

namespace Plugins
{
    void MarketControllerPlugin::ClearClientInfoCL(ClientId client) { mapBuySuppression[client.GetValue()] = false; }

    bool MarketControllerPlugin::LoadSettingsCL()
    {
        LoadJsonWithValidation(ConfigCL, configCl, "config/commodity_limits.json");
        return true;
    }

    bool MarketControllerPlugin::GFGoodBuyCL(SGFGoodBuyInfo const& gbi, ClientId client)
    {
        if (client.GetEquipCargo().Unwrap()->equip.size() >= 127)
        {
            client.MessageErr(L"ERR Too many individual items in hold, aborting purchase to prevent character corruption");
            mapBuySuppression[client.GetValue()] = true;
            return false;
        }
        // Check if this a purchase this plugin must handle
        auto commodityRestriction = configCl.mapCommodityRestrictions.find(gbi.goodId);
        if (commodityRestriction == configCl.mapCommodityRestrictions.end())
        {
            return true;
        }
        // Check to ensure this ship has been undocked at least once and the character has an hookext ID value stored
        // TODO: ID System
        EquipmentId pID;
        if (pID)
        {
            bool valid = false;
            // Check the ID to begin with, it's the most likely type of restriction
            if (commodityRestriction->second.allowedIds.contains(pID))
            {
                // Allow the purchase
                valid = true;
            }
            else
            {
                // If the ID doesn't match, check for the tag
                std::wstring_view charName = client.GetCharacterName().Unwrap();
                for (auto& tag : commodityRestriction->second.tagRestrictions)
                {
                    if (charName.find(tag) == 0)
                    {
                        valid = true;
                        break;
                    }
                }
            }

            // If none of the conditions have been met, deny the purchase
            if (!valid)
            {
                // deny the purchase
                client.MessageErr(L"Sorry, you do not have permission to buy this item.");
                mapBuySuppression[client.GetValue()] = true;
                return false;
            }
        }
        return true;
    }

    bool MarketControllerPlugin::ReqAddItemCL(ClientId client)
    {
        if (mapBuySuppression[client.GetValue()])
        {
            return false;
        }
        return true;
    }

    bool MarketControllerPlugin::ReqChangeCashCL(ClientId client)
    {
        if (mapBuySuppression[client.GetValue()])
        {
            mapBuySuppression[client.GetValue()] = false;
            return false;
        }
        return true;
    }
} // namespace CommodityLimit
