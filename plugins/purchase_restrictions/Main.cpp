// Purchase Restrictions plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// Ported by Raikkonen 2022
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#include "Main.h"

namespace Plugins::PurchaseRestrictions
{
	const auto global = std::make_unique<Global>();

	//! Log items of interest so we can see what cargo cheats people are using.
	static void LogItemsOfInterest(uint iClientID, uint iGoodID, const std::string& details)
	{
		const auto iter = global->itemsOfInterestHashed.find(iGoodID);
		if (iter != global->itemsOfInterestHashed.end())
		{
			const std::wstring charName = GetCharacterNameById(iClientID);
			AddLog(LogType::Normal, LogLevel::Info, L"Item '%s' found in cargo of %s (%s) %s", iter->second.c_str(), charName.c_str(), GetAccountID(GetAccountByCharname(charName)).c_str(), 
				details.c_str());
		}
	}

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
			auto map = global->goodItemRestrictionsHashed[CreateID(key.c_str())] = {};
			for (const auto& i : value)
				map.emplace_back(CreateID(i.c_str()));
		}

		for (const auto& [key, value] : config.shipItemRestrictions)
		{
			auto map = global->shipItemRestrictionsHashed[CreateID(key.c_str())] = {};
			for (const auto& i : value)
				map.emplace_back(CreateID(i.c_str()));
		}
		global->config = std::make_unique<Config>(config);
	}

	/// Check that this client is allowed to buy/mount this piece of equipment or
	/// ship Return true if the equipment is mounted to allow this good.
	bool CheckIDEquipRestrictions(uint iClientID, uint iGoodID)
	{
		const auto validItem = global->goodItemRestrictionsHashed.find(iGoodID);
		if (validItem == global->goodItemRestrictionsHashed.end())
			return true;

		std::list<CARGO_INFO> lstCargo;
		int iRemainingHoldSize;
		EnumCargo(iClientID, lstCargo, iRemainingHoldSize);
		return std::any_of(lstCargo.begin(), lstCargo.end(), [validItem](const CARGO_INFO& cargo) {
			return cargo.bMounted && std::find(validItem->second.begin(), validItem->second.end(), cargo.iArchID) == validItem->second.end();
		});
	}

	void ClearClientInfo(uint& iClientID) { global->clientSuppressBuy[iClientID] = false; }

	void PlayerLaunch(uint& iShip, uint& iClientID) { global->clientSuppressBuy[iClientID] = false; }

	void BaseEnter(uint& iBaseID, uint& iClientID) { global->clientSuppressBuy[iClientID] = false; }

	/// Suppress the buying of goods.
	bool GFGoodBuy(struct SGFGoodBuyInfo const& gbi, uint& iClientID)
	{
		auto& suppress = global->clientSuppressBuy[iClientID] = false;
		LogItemsOfInterest(iClientID, gbi.iGoodID, "good-buy");

		if (std::find(global->unbuyableItemsHashed.begin(), global->unbuyableItemsHashed.end(), gbi.iGoodID) == global->unbuyableItemsHashed.end())
		{
			suppress = true;
			pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("info_access_denied"));
			PrintUserCmdText(iClientID, L"ERR Temporarily out of stock");
			return true;
		}

		/// Check restrictions for the ID that a player has.
		if (global->config->checkItemRestrictions)
		{
			// Check Item
			if (global->goodItemRestrictionsHashed.find(gbi.iGoodID) != global->goodItemRestrictionsHashed.end())
			{
				if (!CheckIDEquipRestrictions(iClientID, gbi.iGoodID))
				{
					const std::wstring charName = GetCharacterNameById(iClientID);
					AddLog(LogType::Normal, LogLevel::Info, L"%s attempting to buy %u without correct ID", charName.c_str(), gbi.iGoodID);
					if (global->config->enforceItemRestrictions)
					{
						PrintUserCmdText(iClientID, global->config->goodPurchaseDenied);
						pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("info_access_denied"));
						suppress = true;
						return true;
					}
				}
			}
			else
			{
				// Check Ship
				const GoodInfo* packageInfo = GoodList::find_by_id(gbi.iGoodID);
				if (packageInfo->iType != 3)
				{
					return false;
				}

				const GoodInfo* hullInfo = GoodList::find_by_id(packageInfo->iHullGoodID);
				if (hullInfo->iType != 2)
				{
					return false;
				}

				if (global->shipItemRestrictionsHashed.find(gbi.iGoodID) != global->shipItemRestrictionsHashed.end() && !CheckIDEquipRestrictions(iClientID, hullInfo->iShipGoodID))
				{
					const std::wstring charName = GetCharacterNameById(iClientID);
					AddLog(LogType::Normal, LogLevel::Info, L"%s attempting to buy %u without correct ID", charName.c_str(), hullInfo->iShipGoodID);
					if (global->config->enforceItemRestrictions)
					{
						PrintUserCmdText(iClientID, global->config->shipPurchaseDenied);
						pub::Player::SendNNMessage(iClientID, pub::GetNicknameId("info_access_denied"));
						suppress = true;
						return true;
					}
				}
			}
		}
		return false;
	}

	/// Suppress the buying of goods.
	bool ReqAddItem(uint& goodID, char const* hardpoint, int& count, float& status, bool& mounted, uint& iClientID)
	{
		LogItemsOfInterest(iClientID, goodID, "add-item");
		if (global->clientSuppressBuy[iClientID])
		{
			return true;
		}
		return false;
	}

	/// Suppress the buying of goods.
	bool ReqChangeCash(int& iMoneyDiff, uint& iClientID)
	{
		if (global->clientSuppressBuy[iClientID])
		{
			global->clientSuppressBuy[iClientID] = false;
			return true;
		}
		return false;
	}

	/// Suppress ship purchases
	bool ReqSetCash(int& iMoney, uint& iClientID)
	{
		if (global->clientSuppressBuy[iClientID])
		{
			return true;
		}
		return false;
	}

	/// Suppress ship purchases
	bool ReqEquipment(class EquipDescList const& eqDesc, uint& iClientID)
	{
		if (global->clientSuppressBuy[iClientID])
		{
			return true;
		}
		return false;
	}

	/// Suppress ship purchases
	bool ReqShipArch(uint& iArchID, uint& iClientID)
	{
		if (global->clientSuppressBuy[iClientID])
		{
			return true;
		}
		return false;
	}

	/// Suppress ship purchases
	bool ReqHullStatus(float& fStatus, uint& iClientID)
	{
		if (global->clientSuppressBuy[iClientID])
		{
			global->clientSuppressBuy[iClientID] = false;
			return true;
		}
		return false;
	}
} // namespace Plugins::PurchaseRestrictions

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::PurchaseRestrictions;
REFL_AUTO(type(Config), field(checkItemRestrictions), field(enforceItemRestrictions), field(shipPurchaseDenied), field(goodPurchaseDenied),
    field(itemsOfInterest), field(unbuyableItems), field(goodItemRestrictions), field(shipItemRestrictions));

DefaultDllMainSettings(LoadSettings)

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
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
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
