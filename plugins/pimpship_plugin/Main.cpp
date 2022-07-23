// Pimpship plugin, adds functionality to changing the lights on player ships.
//
// Created by Canon
//
// Ported to 4.0 by Nen
//
#include "Main.h"

namespace Plugins::Pimpship
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	bool IsItemArchIDAvailable(uint iArchID)
	{
		for (auto& item : global->AvailableItems)
		{
			if (item.second.archId == iArchID)
				return true;
		}
		return false;
	}

	std::string GetItemDescription(uint iArchID)
	{
		for (auto& item : global->AvailableItems)
		{
			if (item.second.archId == iArchID)
				return item.second.description;
		}
		return "";
	}

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(config);

		int iItemID = 1;

		for (auto& info : config.AvailableItems)
		{
			info.archId = CreateID(info.nickname.c_str());
			if (!info.description.length())
				info.description = info.nickname;

			global->AvailableItems[iItemID] = info;
			iItemID++;
		}

		for (auto& base : config.bases)
		{
			uint baseIdHash = CreateID(base.c_str());
			global->config->baseIdHashes.push_back(baseIdHash);
		}
	}

	void BaseEnter(const uint& baseId, const uint& clientId)
	{
		if (!global->config->notifyAvailabilityOnEnter)
		{
			return;
		}

		if (std::find(global->config->baseIdHashes.begin(), global->config->baseIdHashes.end(), baseId) == global->config->baseIdHashes.end())
		{
			return;
		}

		for (auto& baseIdIter : global->config->baseIdHashes)
		{
			if (global->config->IntroMsg1.length())
				PrintUserCmdText(clientId, L"%s", global->config->IntroMsg1.c_str());

			if (global->config->IntroMsg2.length())
				PrintUserCmdText(clientId, L"%s", global->config->IntroMsg2.c_str());
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// USER COMMANDS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UserCmd_PimpShip(const uint& iClientID, const std::wstring_view& wscParam)
	{
		uint baseId;
		HkFunc(HkGetCurrentBase, iClientID, baseId);
		if (std::find(global->config->baseIdHashes.begin(), global->config->baseIdHashes.end(), baseId) == global->config->baseIdHashes.end())
		{
			global->Info[iClientID].InPimpDealer = false;
			global->Info[iClientID].CurrentEquipment.clear();
			PrintUserCmdText(iClientID, L"This station does not have pimpship available");
			return;
		}

		global->Info[iClientID].CurrentEquipment.clear();
		global->Info[iClientID].InPimpDealer = true;

		PrintUserCmdText(iClientID, L"Available ship pimping commands:");

		PrintUserCmdText(iClientID, L"/showsetup");
		PrintUserCmdText(iClientID, L"|     Display current ship setup.");

		PrintUserCmdText(iClientID, L"/showitems");
		PrintUserCmdText(iClientID, L"|     Display items that may be added to your ship.");

		PrintUserCmdText(iClientID, L"/setitem <hardpoint id> <new item id>");
		PrintUserCmdText(iClientID, L"|     Change the item at <hp id> to <item id>.");
		PrintUserCmdText(iClientID, L"|     <hi id>s are shown by typing /show setup.");
		PrintUserCmdText(iClientID, L"|     <item id>s are shown by typing /show items.");

		PrintUserCmdText(iClientID, L"/buynow");
		PrintUserCmdText(iClientID, L"|     Confirms the changes.");
		PrintUserCmdText(iClientID, L"This facility costs " + ToMoneyStr(global->config->Cost) + L" credits to use.");

		std::wstring wscCharName = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		// Build the equipment list.
		int iSlotID = 1;

		st6::list<EquipDesc>& eqLst = Players[iClientID].equipDescList.equip;
		for (auto eq = eqLst.begin(); eq != eqLst.end(); eq++)
		{
			if (IsItemArchIDAvailable(eq->iArchID))
			{
				global->Info[iClientID].CurrentEquipment[iSlotID].ID = eq->sID;
				global->Info[iClientID].CurrentEquipment[iSlotID].ArchID = eq->iArchID;
				global->Info[iClientID].CurrentEquipment[iSlotID].OriginalArchID = eq->iArchID;
				global->Info[iClientID].CurrentEquipment[iSlotID].HardPoint = stows(eq->szHardPoint.value);
				iSlotID++;
			}
		}
	}

	/// Show the setup of the player's ship.
	void UserCmd_ShowSetup(const uint& iClientID, const std::wstring_view& wscParam)
	{
		uint baseId;
		HkFunc(HkGetCurrentBase, iClientID, baseId);
		if (std::find(global->config->baseIdHashes.begin(), global->config->baseIdHashes.end(), baseId) == global->config->baseIdHashes.end())
		{
			return;
		}

		PrintUserCmdText(iClientID, L"Current ship setup: %d", global->Info[iClientID].CurrentEquipment.size());
		for (auto iter = global->Info[iClientID].CurrentEquipment.begin(); iter != global->Info[iClientID].CurrentEquipment.end(); iter++)
		{
			PrintUserCmdText(iClientID, L"|     %.2d | %s : %s", iter->first, iter->second.HardPoint.c_str(), GetItemDescription(iter->second.ArchID).c_str());
		}
		PrintUserCmdText(iClientID, L"OK");
	}

	/// Show the items that may be changed.
	void UserCmd_ShowItems(const uint& iClientID, const std::wstring_view& wscParam)
	{
		uint baseId;
		HkFunc(HkGetCurrentBase, iClientID, baseId);
		if (auto iterator = std::find(global->config->baseIdHashes.begin(), global->config->baseIdHashes.end(), baseId) == global->config->baseIdHashes.end())
		{
			return;
		}

		PrintUserCmdText(iClientID, L"Available items: %d", global->AvailableItems.size());
		for (auto iter = global->AvailableItems.begin(); iter != global->AvailableItems.end(); iter++)
		{
			PrintUserCmdText(iClientID, L"|     %.2d:  %s", iter->first, iter->second.description.c_str());
		}
		PrintUserCmdText(iClientID, L"OK");
	}

	/// Change the item on the Slot ID to the specified item.
	void UserCmd_ChangeItem(const uint& iClientID, const std::wstring_view& wscParam)
	{
		uint baseId;
		HkFunc(HkGetCurrentBase, iClientID, baseId);
		if (std::find(global->config->baseIdHashes.begin(), global->config->baseIdHashes.end(), baseId) == global->config->baseIdHashes.end())
		{
			return;
		}

		int iHardPointID = ToInt(GetParam(wscParam, ' ', 0));
		int iSelectedItemID = ToInt(GetParam(wscParam, ' ', 1));

		if (global->Info[iClientID].CurrentEquipment.find(iHardPointID) == global->Info[iClientID].CurrentEquipment.end())
		{
			PrintUserCmdText(iClientID, L"ERR Invalid hard point ID");
			return;
		}

		if (global->AvailableItems.find(iSelectedItemID) == global->AvailableItems.end())
		{
			PrintUserCmdText(iClientID, L"ERR Invalid item ID");
			return;
		}

		global->Info[iClientID].CurrentEquipment[iHardPointID].ArchID = global->AvailableItems[iSelectedItemID].archId;
		return UserCmd_ShowSetup(iClientID, wscParam);
	}

	void UserCmd_BuyNow(const uint& iClientID, const std::wstring_view& wscParam)
	{
		HK_ERROR err;

		std::wstring wscCharName = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		// Check the that player is in a ship dealer.
		uint baseId;
		HkFunc(HkGetCurrentBase, iClientID, baseId);
		if (std::find(global->config->baseIdHashes.begin(), global->config->baseIdHashes.end(), baseId) == global->config->baseIdHashes.end())
		{
			return;
		}

		// Charge for the equipment pimp.
		if (global->config->Cost)
		{
			int iCash = 0;
			if ((err = HkGetCash(wscCharName, iCash)) != HKE_OK)
			{
				PrintUserCmdText(iClientID, L"ERR %s", HkErrGetText(err).c_str());
				return;
			}
			if (iCash < 0 && iCash < global->config->Cost)
			{
				PrintUserCmdText(iClientID, L"ERR Insufficient credits");
				return;
			}
			HkAddCash(wscCharName, 0 - global->config->Cost);
		}

		// Remove all lights.
		for (auto i = global->Info[iClientID].CurrentEquipment.begin(); i != global->Info[iClientID].CurrentEquipment.end(); ++i)
		{
			pub::Player::RemoveCargo(iClientID, i->second.ID, 1);
		}

		// Re-add all lights so that the order is kept the same
		for (auto i = global->Info[iClientID].CurrentEquipment.begin(); i != global->Info[iClientID].CurrentEquipment.end(); ++i)
		{
			HkAddEquip(wscCharName, i->second.ArchID, wstos(i->second.HardPoint));
		}

		PrintUserCmdText(iClientID, L"OK Ship pimp complete. Please wait 10 seconds and reconnect.");
		HkDelayedKick(iClientID, 5);
	}

	// Client command processing
	const std::vector commands = {{
	    CreateUserCommand(L"/pimpship", L"", UserCmd_PimpShip, L""),
	    CreateUserCommand(L"/showsetup", L"", UserCmd_ShowSetup, L""),
	    CreateUserCommand(L"/showitems", L"", UserCmd_ShowItems, L""),
	    CreateUserCommand(L"/setitem", L"", UserCmd_ChangeItem, L""),
	    CreateUserCommand(L"/buynow", L"", UserCmd_BuyNow, L""),
	}};
} // namespace Plugins::Pimpship

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::Pimpship;

REFL_AUTO(type(ItemInfo), field(nickname), field(description))
REFL_AUTO(type(Config), field(AvailableItems), field(Cost), field(bases), field(IntroMsg1), field(IntroMsg2), field(notifyAvailabilityOnEnter))
DefaultDllMainSettings(LoadSettings)

    // Functions to hook
    extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Pimpship");
	pi->shortName("pimpship");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &BaseEnter);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}
