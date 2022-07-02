#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::PurchaseRestrictions
{
	//! Config data for this plugin
	struct Config : Reflectable
	{
		std::string File() override { return "flhook_plugins/purchaseRestrictions.json"; }

		//! Check whether a player is trying to buy items without the correct id
		bool checkItemRestrictions = false;

		//! Block them for buying said item without the correct id
		bool enforceItemRestrictions = false;

		//! Messages when they are blocked from buying something
		std::wstring shipPurchaseDenied = L"You are not authorized to buy this ship.";
		std::wstring goodPurchaseDenied = L"You are not authorized to buy this item.";

		//! Items that we log transfers for
		std::vector<std::string> itemsOfInterest;

		//! Items that cannot be bought at all.
		std::vector<std::string> unbuyableItems;

		//! Items that can only be bought with a certain item equipped (item, [ equippedItemsThatAllowPurchase ])
		std::map<std::string, std::vector<std::string>> goodItemRestrictions;

		//! Ships that can only be bought with a certain item equipped (ship, [ equippedItemsThatAllowPurchase ])
		std::map<std::string, std::vector<std::string>> shipItemRestrictions;
	};

	//! Global data for this plugin
	struct Global
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returnCode = ReturnCode::Default;
		std::map<ClientId, bool> clientSuppressBuy;

		std::vector<uint> unbuyableItemsHashed;
		std::map<uint, std::string> itemsOfInterestHashed;
		std::map<uint, std::vector<uint>> goodItemRestrictionsHashed;
		std::map<uint, std::vector<uint>> shipItemRestrictionsHashed;
	};
} // namespace Plugins::PurchaseRestrictions