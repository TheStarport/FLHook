#pragma once

#include <FLHook.hpp>
#include <plugin.h>

namespace Plugins::PurchaseRestrictions
{
	struct Config : Reflectable
	{
		std::string File() override { return "flhook_plugins/purchaseRestrictions.json"; }
		bool checkItemRestrictions = false;
		bool enforceItemRestrictions = false;
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

	struct Global
	{
		Config config;
		ReturnCode returnCode = ReturnCode::Default;
		std::map<ClientId, bool> clientSuppressBuy;

		std::vector<uint> unbuyableItemsHashed;
		std::map<uint, std::string> itemsOfInterestHashed;
		std::map<uint, std::vector<uint>> goodItemRestrictionsHashed;
		std::map<uint, std::vector<uint>> shipItemRestrictionsHashed;
	};
} // namespace Plugins::PurchaseRestrictions