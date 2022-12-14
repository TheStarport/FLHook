// Warehouse Plugin

#include "Main.h"

namespace Plugins::Warehouse
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadSettings()
	{ 
		global->config = Serializer::JsonToObject<Config>();
		for (const auto& i : global->config.restrictedItems)
		{
			global->config.restrictedItemsHashed.emplace_back(CreateID(i.c_str()));
		}
		for (const auto& i : global->config.restrictedBases)
		{
			global->config.restrictedBasesHashed.emplace_back(CreateID(i.c_str()));
		}

		CreateSqlTables();
	}

	void UserCmdStoreItem(uint clientId, const std::wstring& param, uint base)
	{
		// This is a generated number to allow players to select the item they want to store.
		const uint itemNumber = ToInt(GetParam(param, ' ', 0));

		if (!itemNumber)
		{
			PrintUserCmdText(clientId, L"Error Invalid Item Number");
			return;
		}

		int _;
		const auto cargo = Hk::Player::EnumCargo(clientId, _);
		int i = 0;
		std::vector<CARGO_INFO> filteredCargo;
		for (auto info : cargo.value())
		{
			if (info.bMounted || info.fStatus < 1.f)
				continue;

			filteredCargo.emplace_back(info);
		}

		const int itemCount = ToInt(GetParam(param, ' ', 1));


		if (itemNumber > filteredCargo.size())
		{
			PrintUserCmdText(clientId, L"Error Invalid Item Number");
			return;
		}
		const auto& item = filteredCargo[itemNumber - 1];
		if (itemCount > item.iCount || itemCount <= 0)
		{
			PrintUserCmdText(clientId, L"Error Invalid Item Quantity");
			return;
		}


		if (const int cash = Hk::Player::GetCash(clientId).value(); cash < global->config.costPerStackStore)
		{
			PrintUserCmdText(clientId, L"Not enough credits. The fee for storing items at this station is %u", global->config.costPerStackStore);
			return;
		}

		Hk::Player::AddCash(clientId, -global->config.costPerStackStore);
		Hk::Player::RemoveCargo(clientId, item.iArchId, itemCount);


		const auto playerAccId = Hk::Client::GetAccountByClientID(clientId);
		auto whPlayer = std::find_if(global->allPlayers.players.begin(), global->allPlayers.players.end(), [playerAccId](const WhPlayer& player) 
			{return player.accID == playerAccId;}
		);

		if (whPlayer == global->allPlayers.players.end())
		{
			auto newPlayer = WhPlayer();
			newPlayer.accID = playerAccId;
			
			global->allPlayers.players.emplace_back(std::move(newPlayer));
			whPlayer = global->allPlayers.players.end();
			--whPlayer;
		}

		
		 auto warehouse =
		    std::find_if(whPlayer->warehouses.begin(), whPlayer->warehouses.end(), [base](const Warehouse& wh) { return wh.warehouseHash == base; });

		if (warehouse == whPlayer->warehouses.end())
		{
			auto newWarehouse = Warehouse();
			newWarehouse.warehouseHash = base;

			whPlayer->warehouses.emplace_back(std::move(newWarehouse));
			warehouse = whPlayer->warehouses.end();
			--whPlayer;
		}

		auto itemToBeStored = std::find_if(warehouse->storedItems.begin(), warehouse->storedItems.end(), [item](const WarehouseItem& whItem) 
			{ return whItem.hashID == item.iArchID; });

		if (itemToBeStored == warehouse->storedItems.end())
		{
			auto newItemToBeStored = WarehouseItem();
			newItemToBeStored.hashID = item.iArchID;

			warehouse->storedItems.emplace_back(std::move(newItemToBeStored));
			itemToBeStored = warehouse->storedItems.end();
			--itemToBeStored;
		}
		
		itemToBeStored->quantity += itemCount;

		Serializer::SaveToJson(global->allPlayers);
		HkSaveChar(clientId);
	}

	void UserCmdGetItems(uint clientId, const std::wstring& param, uint base)
	{
		std::list<CARGO_INFO> cargo;
		int _;
		HkEnumCargo(clientId, cargo, _);

		int i = 0;
		for (auto info : cargo)
		{
			if (info.bMounted || info.fStatus < 1.f)
				continue;

			const auto* equip = Archetype::GetEquipment(info.iArchID);
			i++;
			PrintUserCmdText(clientId, std::to_wstring(i) + L".) " + HkGetWStringFromIDS(equip->iIdsName) + L" (" + std::to_wstring(info.iCount) + L")");
		}

	}
	void UserCmdGetWarehouseItems(uint clientId, const std::wstring& param, uint base)
	{
		const auto whPlayerAccID = HkGetAccountIDByClientID(clientId);

		const auto whPlayer = std::find_if(global->allPlayers.players.begin(), global->allPlayers.players.end(),
		    [whPlayerAccID](const WhPlayer& player) { return player.accID == whPlayerAccID; });

		if (whPlayer == global->allPlayers.players.end())
		{
			PrintUserCmdText(clientId, L"You have no items stored at this warehouse.");
			return;
		}

		const auto warehouse = std::find_if(
		    whPlayer->warehouses.begin(), whPlayer->warehouses.end(), [base](const Warehouse& wh) { return wh.warehouseHash == base; });
		

		if (warehouse == whPlayer->warehouses.end())
		{
			PrintUserCmdText(clientId, L"You have no items stored at this warehouse.");
			return;
		}

		int i = 0;
		for (const auto& info : warehouse->storedItems)
		{
	
			const auto* equip = Archetype::GetEquipment(info.hashID);
			i++;
			PrintUserCmdText(clientId, std::to_wstring(i) + L".) " + HkGetWStringFromIDS(equip->iIdsName) + L" (" + std::to_wstring(info.quantity) + L")");
		}

	}

	void UserCmdWithdrawItem(uint clientId, const std::wstring& param, uint base)
	{

		// This is a generated number to allow players to select the item they want to store.
		const uint itemNumber = ToInt(GetParam(param, ' ', 0));

		if (!itemNumber)
		{
			PrintUserCmdText(clientId, L"Error Invalid Item Number");
			return;
		}
		std::list<CARGO_INFO> cargo;
		int remainingCargo;
		int i = 0;
		std::vector<CARGO_INFO> filteredCargo;
		HkEnumCargo(clientId, cargo, remainingCargo);
		for (auto info : cargo)
		{
			if (info.bMounted || info.fStatus < 1.f)
				continue;

			filteredCargo.emplace_back(info);
		}

		int itemCount = ToInt(GetParam(param, ' ', 1));

		if (itemNumber > filteredCargo.size())
		{
			PrintUserCmdText(clientId, L"Error Invalid Item Number");
			return;
		}
		const auto& item = filteredCargo[itemNumber - 1];
		if (itemCount > item.iCount || itemCount <= 0)
		{
			PrintUserCmdText(clientId, L"Error Invalid Item Quantity");
			return;
		}

		int cash;
		HkFunc(HkGetCash, clientId, cash);

		if (cash < global->config.costPerStackWithdraw)
		{
			PrintUserCmdText(clientId, L"Not enough credits. The fee for storing items at this station is %u", global->config.costPerStackWithdraw);
			return;
		}

		const auto whPlayerAccID = HkGetAccountIDByClientID(clientId);

		const auto whPlayer = std::find_if(global->allPlayers.players.begin(), global->allPlayers.players.end(),
		    [whPlayerAccID](const WhPlayer& player) { return player.accID == whPlayerAccID; });

		if (whPlayer == global->allPlayers.players.end())
		{
			PrintUserCmdText(clientId, L"You have no items to withdraw.");
			return;
		}

		 const auto warehouse = std::find_if(whPlayer->warehouses.begin(), whPlayer->warehouses.end(), [base](const Warehouse& wh) { return wh.warehouseHash == base; });

		if (warehouse == whPlayer->warehouses.end())
		{
			PrintUserCmdText(clientId, L"You have no items to withdraw.");
			return;
		}

		const auto itemToWithdraw = warehouse->storedItems[itemNumber - 1];

		const auto* itemToWithdrawPtr = Archetype::GetEquipment(itemToWithdraw.hashID);

		if (!itemToWithdrawPtr)
		{
			PrintUserCmdText(clientId, L"Item is no longer valid.");
			warehouse->storedItems.erase(warehouse->storedItems.begin() + itemNumber - 1);
			return;
		}

		
		if (itemToWithdrawPtr->fVolume * itemToWithdraw.quantity >= std::floor(remainingCargo))
		{
			PrintUserCmdText(clientId, L"Withdraw request denied. Your ship cannot accomodate cargo of this size");
			return;
		}

		if (itemCount > itemToWithdraw.quantity)
		{
			itemCount = std::clamp(itemCount, 0, static_cast<int>(itemToWithdraw.quantity));
		}


		HkFunc(HkAddCargo, clientId, itemToWithdraw.hashID, itemCount, false);
		HkFunc(HkAddCash, clientId, -global->config.costPerStackWithdraw);
		Serializer::SaveToJson(global->allPlayers);
		HkSaveChar(clientId);
	}

	void UserCmdWarehouse(uint clientId, const std::wstring& param)
	{
		const std::wstring cmd = GetParam(param, ' ', 0);
		if (cmd.empty()) 
		{
			PrintUserCmdText(clientId, L"Usage: /warehouse store <itemId> <count>");
			PrintUserCmdText(clientId, L"Usage: /warehouse list");
			PrintUserCmdText(clientId, L"Usage: /warehouse withdraw <itemId> <count>");
			PrintUserCmdText(clientId, L"Usage: /warehouse liststored <itemId> <count>");
			return;
		}

		uint base;
		pub::Player::GetBase(clientId, base);

		if (!base) 
		{
			PrintUserCmdText(clientId, L"You must be docked in order to use this command.");
			return;
		}

		if (cmd == L"store")
		{
			UserCmdStoreItem(clientId, param, base);
		}
		else if (cmd == L"list")
		{
			UserCmdGetItems(clientId, param, base);
		}
		else if (cmd == L"withdraw")
		{
			UserCmdWithdrawItem(clientId, param, base);
		}
		else if (cmd == L"liststored")
		{
			UserCmdGetWarehouseItems(clientId, param, base);
		}
		else 
		{
			PrintUserCmdText(clientId, L"Invalid Command. Refer to /warehouse to see usage.");
		}
	}

	constexpr USERCMD UserCmds[] = {
		{ L"/warehouse", UserCmdWarehouse }
	};

		// Process user input
	bool UserCmd_Process(uint& iClientID, const std::wstring& wscCmd) { DefaultUserCommandHandling(iClientID, wscCmd, UserCmds, global->returnCode); }


} // namespace Plugins::Warehouse

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::Warehouse;

REFL_AUTO(type(Config), field(restrictedBases), field(restrictedItems), field(costPerStackWithdraw), field(costPerStackStore))
REFL_AUTO(type(AllPlayers), field(players))
REFL_AUTO(type(WhPlayer), field(accID), field(warehouses))
REFL_AUTO(type(Warehouse),field(warehouseNickName), field(storedItems))
REFL_AUTO(type(WarehouseItem), field(hashID), field(quantity))

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Warehouse");
	pi->shortName("warehouse");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->emplaceHook(HookedCall::FLHook__UserCommand__Process, &UserCmd_Process);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);

}
