// This is a template with the bare minimum to have a functional plugin.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

namespace Plugins::Pimpship
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	bool IsItemArchIDAvailable(uint iArchID)
	{
		for (auto& item : global->AvailableItems)
		{
			if (item.second.ArchID == iArchID)
				return true;
		}
		return false;
	}

	std::string GetItemDescription(uint iArchID)
	{
		for (auto& item : global->AvailableItems)
		{
			if (item.second.ArchID == iArchID)
				return item.second.Description;
		}
		return "";
	}

	void LoadSettings()
	{
		auto config = Serializer::JsonToObject<Config>();
		global->Dealers.clear();

		// Patch BaseDataList::get_room_data to suppress annoying warnings
		// flserver-errors.log
		unsigned char patch1[] = {0x90, 0x90};
		WriteProcMem((char*)0x62660F2, &patch1, 2);

		int iItemID = 1;

		for (auto& info : config.AvailableItems)
		{

			info.ArchID = CreateID(info.Nickname.c_str());
			if (!info.Description.length())
				info.Description = info.Nickname;

			global->AvailableItems[iItemID] = info;
			iItemID++;
		}

		for (auto& room : config.Dealers)
		{
			uint iLocationID = CreateID(room.c_str());

			if (!BaseDataList_get()->get_room_data(iLocationID))
			{
				if (FLHookConfig::i()->general.debugMode)
				{
					Console::ConWarn(L"NOTICE: Room %s does not exist", stows(room).c_str());
				}
			}
			else
				global->Dealers[iLocationID] = stows(room);
		}

		global->config = std::make_unique<Config>(config);
		
		// Unpatch BaseDataList::get_room_data to suppress annoying warnings
		// flserver-errors.log
		unsigned char unpatch1[] = {0xFF, 0x12};
		WriteProcMem((char*)0x62660F2, &patch1, 2);
	}

	// On entering a room check to see if we're in a valid ship dealer room (or base
	// if a ShipDealer is not defined). If we are then print the intro text
	// otherwise do nothing.
	void LocationEnter(uint& iLocationID, uint& iClientID)
	{
		if (global->Dealers.find(iLocationID) == global->Dealers.end())
		{
			uint iBaseID = 0;
			pub::Player::GetBase(iClientID, iBaseID);
			if (global->Dealers.find(iBaseID) == global->Dealers.end())
			{
				global->Info[iClientID].InPimpDealer = false;
				global->Info[iClientID].CurrentEquipment.clear();
				return;
			}
		}

		if (global->config->IntroMsg1.length())
			PrintUserCmdText(iClientID, L"%s", global->config->IntroMsg1.c_str());

		if (global->config->IntroMsg2.length())
			PrintUserCmdText(iClientID, L"%s", global->config->IntroMsg2.c_str());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// USER COMMANDS
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void UserCmd_PimpShip(uint iClientID, const std::wstring& wscParam)
	{
		uint iLocationID = 0;
		pub::Player::GetLocation(iClientID, iLocationID);
		if (global->Dealers.find(iLocationID) == global->Dealers.end())
		{
			uint iBaseID = 0;
			pub::Player::GetBase(iClientID, iBaseID);
			if (global->Dealers.find(iBaseID) == global->Dealers.end())
			{
				global->Info[iClientID].InPimpDealer = false;
				global->Info[iClientID].CurrentEquipment.clear();
				return;
			}
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
	void UserCmd_ShowSetup(uint iClientID, const std::wstring& wscParam)
	{
		if (!global->Info[iClientID].InPimpDealer)
			return;

		PrintUserCmdText(iClientID, L"Current ship setup: %d", global->Info[iClientID].CurrentEquipment.size());
		for (auto iter = global->Info[iClientID].CurrentEquipment.begin(); iter != global->Info[iClientID].CurrentEquipment.end(); iter++)
		{
			PrintUserCmdText(
			    iClientID, L"|     %.2d | %s : %s", iter->first, iter->second.HardPoint.c_str(), GetItemDescription(iter->second.ArchID).c_str());
		}
		PrintUserCmdText(iClientID, L"OK");
	}

	/// Show the items that may be changed.
	void UserCmd_ShowItems(uint iClientID, const std::wstring& wscParam)
	{
		if (!global->Info[iClientID].InPimpDealer)
			return;

		PrintUserCmdText(iClientID, L"Available items: %d", global->AvailableItems.size());
		for (auto iter = global->AvailableItems.begin(); iter != global->AvailableItems.end(); iter++)
		{
			PrintUserCmdText(iClientID, L"|     %.2d:  %s", iter->first, iter->second.Description.c_str());
		}
		PrintUserCmdText(iClientID, L"OK");
	}

	/// Change the item on the Slot ID to the specified item.
	void UserCmd_ChangeItem(uint iClientID, const std::wstring& wscParam)
	{
		if (!global->Info[iClientID].InPimpDealer)
			return;

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

		global->Info[iClientID].CurrentEquipment[iHardPointID].ArchID = global->AvailableItems[iSelectedItemID].ArchID;
		return UserCmd_ShowSetup(iClientID, wscParam);
	}

	void UserCmd_BuyNow(uint iClientID, const std::wstring& wscParam)
	{
		HK_ERROR err;

		std::wstring wscCharName = (const wchar_t*)Players.GetActiveCharacterName(iClientID);

		// Check the that player is in a ship dealer.
		if (!global->Info[iClientID].InPimpDealer)
			return;

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
	const std::array<USERCMD, 5> UserCmds = {{
	    {L"/pimpship", UserCmd_PimpShip},
	    {L"/showsetup", UserCmd_ShowSetup},
	    {L"/showitems", UserCmd_ShowItems},
	    {L"/setitem", UserCmd_ChangeItem},
	    {L"/buynow", UserCmd_BuyNow},
	}};

	// Hook on /help
	void UserCmd_Help(uint& iClientID, const std::wstring& wscParam)
	{
		PrintUserCmdText(iClientID, L"/afk ");
		PrintUserCmdText(iClientID,
		    L"Sets the player to AFK. If any other player messages "
		    L"directly, they will be told you are afk.");
		PrintUserCmdText(iClientID, L"/back ");
		PrintUserCmdText(iClientID, L"Turns off AFK for a the player.");
	}
} // namespace Plugins::Pimpship

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::Pimpship;

bool ProcessUserCmds(uint& clientId, const std::wstring& param)
{
	return DefaultUserCommandHandling(clientId, param, UserCmds, global->returncode);
}

REFL_AUTO(type(ITEM_INFO), field(Nickname), field(Description))
REFL_AUTO(type(Config), field(AvailableItems), field(Cost), field(Dealers), field(IntroMsg1), field(IntroMsg2))

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Pimpship");
	pi->shortName("pimpship");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &ProcessUserCmds);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Help, &UserCmd_Help);
	pi->emplaceHook(HookedCall::IServerImpl__LocationEnter, &LocationEnter);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
}
