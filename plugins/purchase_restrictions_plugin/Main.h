#pragma once

#include <FLHook.h>
#include <plugin.h>

ReturnCode returncode = ReturnCode::Default;

/// If true then an ID is used to check for valid IDs
static bool set_bCheckIDRestrictions = false;

/// If true then an ID is used to check for valid IDs
static bool set_bEnforceIDRestrictions = false;

/// Message if ship purchase denied
static std::wstring set_wscShipPurchaseDenied = L"";

/// Message if good purchase denied
static std::wstring set_wscGoodPurchaseDenied = L"";

/// list of items that we log transfers for
static std::map<uint, std::string> set_mapItemsOfInterest;

/** A map of baseid to goods that are not allowed to be bought */
static std::multimap<uint, uint> set_mapNoBuy;
typedef std::multimap<uint, uint, std::less<uint>>::value_type mapnobuy_map_pair_t;
typedef std::multimap<uint, uint, std::less<uint>>::iterator mapnobuy_map_iter_t;

/** A map of goods that are not allowed to be bought without the correct item.
 */
static std::multimap<uint, uint> set_mapGoodItemRestrictions;
typedef std::multimap<uint, uint, std::less<uint>>::value_type mapGoodItemRestrictions_map_pair_t;
typedef std::multimap<uint, uint, std::less<uint>>::iterator mapGoodItemRestrictions_map_iter_t;

/** A map of ship that are not allowed to be bought without the correct item. */
static std::multimap<uint, uint> set_mapShipItemRestrictions;
typedef std::multimap<uint, uint, std::less<uint>>::value_type mapShipItemRestrictions_map_pair_t;
typedef std::multimap<uint, uint, std::less<uint>>::iterator mapShipItemRestrictions_map_iter_t;

// List of connected clients.
struct INFO
{
	INFO() : bSuppressBuy(false) {}

	bool bSuppressBuy;
};
static std::map<uint, INFO> mapInfo;