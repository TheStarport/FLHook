/**
 * @date Feb, 2010
 * @author Cannon. Ported by Raikkonen, Nen and Laz
 * @defgroup PurchaseRestrictions Purchase Restrictions
 * @brief
 * The Purchase Restrictions plugin restricts the purchase of equipment, goods and ships unless the player holds a certain item.
 *
 * @paragraph cmds Player Commands
 * None
 *
 * @paragraph adminCmds Admin Commands
 * None
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *    "checkItemRestrictions": false,
 *    "enforceItemRestrictions": false,
 *    "goodItemRestrictions": {
 *        "li_gun01_mark02": [
 *            "li_gun01_mark03"
 *        ]
 *    },
 *    "goodPurchaseDenied": "You are not authorized to buy this item.",
 *    "itemsOfInterest": [
 *        "li_gun01_mark01"
 *    ],
 *    "shipItemRestrictions": {
 *        "li_fighter": [
 *            "li_gun01_mark03"
 *        ]
 *    },
 *    "shipPurchaseDenied": "You are not authorized to buy this ship.",
 *    "unbuyableItems": [
 *        "li_gun01_mark01"
 *    ]
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

#include "Main.h"

namespace Plugins::PurchaseRestrictions
{
	const auto global = std::make_unique<Global>();

	//! Log items of interest so we can see what cargo cheats people are using.
	static void LogItemsOfInterest(ClientId& client, uint iGoodId, const std::string& details)
	{
		const auto iter = global->itemsOfInterestHashed.find(iGoodId);
		if (iter != global->itemsOfInterestHashed.end())
		{
			const auto charName = Hk::Client::GetCharacterNameByID(client);
			AddLog(LogType::Normal,
			    LogLevel::Info,
			    std::format("Item '{}' found in cargo of {} - {}", iter->second.c_str(), wstos(charName.value()), details.c_str()));
		}
	}

	//! Load settings from json file
	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();

		for (const auto& str : config.itemsOfInterest)
		{
			global->itemsOfInterestHashed[CreateID(str.c_str())] = str;
		}

		for (const auto& str : config.unbuyableItems)
		{
			global->unbuyableItemsHashed.emplace_back(CreateID(str.c_str()));
		}

		for (const auto& [key, value] : config.goodItemRestrictions)
		{
			auto& map = global->goodItemRestrictionsHashed[CreateID(key.c_str())] = {};
			for (const auto& i : value)
				map.emplace_back(CreateID(i.c_str()));
		}

		for (const auto& [key, value] : config.shipItemRestrictions)
		{
			auto& map = global->shipItemRestrictionsHashed[CreateID(key.c_str())] = {};
			for (const auto& i : value)
				map.emplace_back(CreateID(i.c_str()));
		}
		global->config = std::make_unique<Config>(config);
	}

 
	//! Check that this client is allowed to buy/mount this piece of equipment or ship Return true if the equipment is mounted to allow this good.
	bool CheckIdEquipRestrictions(ClientId client, uint iGoodId, bool isShip)
	{
		const auto list = isShip ? global->shipItemRestrictionsHashed : global->goodItemRestrictionsHashed;

		const auto validItem = list.find(iGoodId);
		if (validItem == list.end())
			return true;

		int remainingHoldSize;
		const auto cargoList = Hk::Player::EnumCargo(client, remainingHoldSize);
		return std::ranges::any_of(cargoList.value(), [validItem](const CARGO_INFO& cargo) {
			return cargo.bMounted && std::ranges::find(validItem->second, cargo.iArchId) != validItem->second.end();
		});
	}

	//! Clear Client Info hook
	void ClearClientInfo(ClientId& client)
	{
		global->clientSuppressBuy[client] = false;
	}

	//! PlayerLaunch hook
	void PlayerLaunch([[maybe_unused]] const uint& ship, ClientId& client)
	{
		global->clientSuppressBuy[client] = false;
	}

	//! Base Enter hook
	void BaseEnter([[maybe_unused]] const uint& baseId, ClientId& client)
	{
		global->clientSuppressBuy[client] = false;
	}

	//! Suppress the buying of goods.
	void GFGoodBuy(struct SGFGoodBuyInfo const& gbi, ClientId& client)
	{
		global->clientSuppressBuy[client] = false;
		auto& suppress = global->clientSuppressBuy[client];
		LogItemsOfInterest(client, gbi.iGoodId, "good-buy");

		if (std::ranges::find(global->unbuyableItemsHashed, gbi.iGoodId) != global->unbuyableItemsHashed.end())
		{
			suppress = true;
			pub::Player::SendNNMessage(client, pub::GetNicknameId("info_access_denied"));
			PrintUserCmdText(client, global->config->goodPurchaseDenied);
			global->returnCode = ReturnCode::SkipAll;
			return;
		}

		/// Check restrictions for the Id that a player has.
		if (global->config->checkItemRestrictions)
		{
			// Check Item
			if (global->goodItemRestrictionsHashed.contains(gbi.iGoodId))
			{
				if (!CheckIdEquipRestrictions(client, gbi.iGoodId, false))
				{
					const auto charName = Hk::Client::GetCharacterNameByID(client);
					AddLog(LogType::Normal, LogLevel::Info, std::format("{} attempting to buy {} without correct Id", wstos(charName.value()), gbi.iGoodId));
					if (global->config->enforceItemRestrictions)
					{
						PrintUserCmdText(client, global->config->goodPurchaseDenied);
						pub::Player::SendNNMessage(client, pub::GetNicknameId("info_access_denied"));
						suppress = true;
						global->returnCode = ReturnCode::SkipAll;
					}
				}
			}
			else
			{
				// Check Ship
				const GoodInfo* packageInfo = GoodList::find_by_id(gbi.iGoodId);
				if (packageInfo->iType != 3)
				{
					return;
				}

				const GoodInfo* hullInfo = GoodList::find_by_id(packageInfo->iHullGoodId);
				if (hullInfo->iType != 2)
				{
					return;
				}

				if (global->shipItemRestrictionsHashed.contains(hullInfo->shipGoodId) &&
				    !CheckIdEquipRestrictions(client, hullInfo->shipGoodId, true))
				{
					const auto charName = Hk::Client::GetCharacterNameByID(client);
					AddLog(LogType::Normal,
					    LogLevel::Info,
					    std::format("{} attempting to buy {} without correct Id", wstos(charName.value()), hullInfo->shipGoodId));
					if (global->config->enforceItemRestrictions)
					{
						PrintUserCmdText(client, global->config->shipPurchaseDenied);
						pub::Player::SendNNMessage(client, pub::GetNicknameId("info_access_denied"));
						suppress = true;
						global->returnCode = ReturnCode::SkipAll;
					}
				}
			}
		}
	}

	//!  Suppress the buying of goods.
	void ReqAddItem(const uint& goodId, [[maybe_unused]] char const* hardpoint, [[maybe_unused]] const int& count, [[maybe_unused]] const float& status,
	    [[maybe_unused]] const bool& mounted, ClientId& client)
	{
		LogItemsOfInterest(client, goodId, "add-item");
		if (global->clientSuppressBuy[client])
		{
			global->returnCode = ReturnCode::SkipAll;
		}
	}

	//!  Suppress the buying of goods.
	void ReqChangeCash([[maybe_unused]]const int& moneyDiff, ClientId& client)
	{
		if (global->clientSuppressBuy[client])
		{
			global->clientSuppressBuy[client] = false;
			global->returnCode = ReturnCode::SkipAll;
		}
	}

	//!  Suppress ship purchases
	void ReqSetCash([[maybe_unused]]const int& money, ClientId& client)
	{
		if (global->clientSuppressBuy[client])
		{
			global->returnCode = ReturnCode::SkipAll;
		}
	}

	//!  Suppress ship purchases
	void ReqEquipment([[maybe_unused]] class EquipDescList const& eqDesc, ClientId& client)
	{
		if (global->clientSuppressBuy[client])
		{
			global->returnCode = ReturnCode::SkipAll;
		}
	}

	//!  Suppress ship purchases
	void ReqShipArch([[maybe_unused]]const uint& archId, ClientId& client)
	{
		if (global->clientSuppressBuy[client])
		{
			global->returnCode = ReturnCode::SkipAll;
		}
	}

	//!  Suppress ship purchases
	void ReqHullStatus([[maybe_unused]]const float& status, ClientId& client)
	{
		if (global->clientSuppressBuy[client])
		{
			global->clientSuppressBuy[client] = false;
			global->returnCode = ReturnCode::SkipAll;
		}
	}
} // namespace Plugins::PurchaseRestrictions

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::PurchaseRestrictions;
REFL_AUTO(type(Config), field(checkItemRestrictions), field(enforceItemRestrictions), field(shipPurchaseDenied), field(goodPurchaseDenied),
    field(itemsOfInterest), field(unbuyableItems), field(goodItemRestrictions), field(shipItemRestrictions));

DefaultDllMainSettings(LoadSettings);

    // Functions to hook
    extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Purchase Restrictions");
	pi->shortName("PurchaseRestrictions");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__GFGoodBuy, &GFGoodBuy);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__PlayerLaunch, &PlayerLaunch);
	pi->emplaceHook(HookedCall::IServerImpl__ReqAddItem, &ReqAddItem);
	pi->emplaceHook(HookedCall::IServerImpl__ReqChangeCash, &ReqChangeCash);
	pi->emplaceHook(HookedCall::IServerImpl__ReqEquipment, &ReqEquipment);
	pi->emplaceHook(HookedCall::IServerImpl__ReqHullStatus, &ReqHullStatus);
	pi->emplaceHook(HookedCall::IServerImpl__ReqSetCash, &ReqSetCash);
	pi->emplaceHook(HookedCall::IServerImpl__ReqShipArch, &ReqShipArch);
}
