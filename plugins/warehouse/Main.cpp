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

	void UserCmdStoreItem(uint client, const std::wstring& param, uint base)
	{
		// This is a generated number to allow players to select the item they want to store.
		const uint databaseItemId = ToInt(GetParam(param, ' ', 1));

		if (!databaseItemId)
		{
			PrintUserCmdText(client, L"Error Invalid Item Number");
			return;
		}

		int _;
		const auto cargo = Hk::Player::EnumCargo(client, _);
		int i = 0;
		std::vector<CARGO_INFO> filteredCargo;
		for (auto info : cargo.value())
		{
			if (info.bMounted || info.fStatus < 1.f)
				continue;

			filteredCargo.emplace_back(info);
		}

		const int itemCount = ToInt(GetParam(param, ' ', 2));

		if (databaseItemId > filteredCargo.size())
		{
			PrintUserCmdText(client, L"Error Invalid Item Number");
			return;
		}
		const auto& item = filteredCargo[databaseItemId - 1];
		if (itemCount > item.iCount || itemCount <= 0)
		{
			PrintUserCmdText(client, L"Error Invalid Item Quantity");
			return;
		}

		if (const uint cash = Hk::Player::GetCash(client).value(); cash < global->config.costPerStackStore)
		{
			PrintUserCmdText(client, L"Not enough credits. The fee for storing items at this station is %u", global->config.costPerStackStore);
			return;
		}

		Hk::Player::RemoveCash(client, global->config.costPerStackStore);
		Hk::Player::RemoveCargo(client, item.iId, itemCount);

		const auto account = Hk::Client::GetAccountByClientID(client);
		const auto sqlBaseId = GetOrAddBase(base);
		const auto sqlPlayerId = GetOrAddPlayer(sqlBaseId, account);
		const auto wareHouseItem = GetOrAddItem(item.iArchId, sqlPlayerId, itemCount);

		PrintUserCmdText(client, fmt::format(L"Successfully stored {} item(s) for a total of {} (ID: {})"), itemCount, wareHouseItem.quantity, wareHouseItem.id);

		Hk::Player::SaveChar(client);
	}

	void UserCmdGetItems(uint client, const std::wstring& param, uint base)
	{
		int _;
		const auto cargo = Hk::Player::EnumCargo(client, _);

		int i = 0;
		for (auto info : cargo.value())
		{
			if (info.bMounted || info.fStatus < 1.f)
				continue;

			const auto* equip = Archetype::GetEquipment(info.iArchId);
			i++;
			PrintUserCmdText(
			    client, std::to_wstring(i) + L".) " + Hk::Message::GetWStringFromIdS(equip->iIdsName) + L" (" + std::to_wstring(info.iCount) + L")");
		}
	}
	void UserCmdGetWarehouseItems(uint client, const std::wstring& param, uint base)
	{
		const auto account = Hk::Client::GetAccountByClientID(client);
		const auto sqlBaseId = GetOrAddBase(base);
		const auto sqlPlayerId = GetOrAddPlayer(sqlBaseId, account);
		const auto itemList = GetAllItemsOnBase(sqlPlayerId);

		if (itemList.empty())
		{
			PrintUserCmdText(client, L"You have no items stored at this warehouse.");
			return;
		}

		for (const auto& info : itemList)
		{
			const auto* equip = Archetype::GetEquipment(info.equipArchId);
			if (!equip)
			{
				Console::ConWarn("Item archetype %u no loner exists", info.equipArchId);
				continue;
			}

			PrintUserCmdText(
			    client, std::to_wstring(info.id) + L".) " + Hk::Message::GetWStringFromIdS(equip->iIdsName) + L" (" + std::to_wstring(info.quantity) + L")");
		}
	}

	void UserCmdWithdrawItem(uint client, const std::wstring& param, uint base)
	{
		// This is a generated number to allow players to select the item they want to store.
		const uint databaseItemId = ToInt(GetParam(param, ' ', 1));

		if (!databaseItemId)
		{
			PrintUserCmdText(client, L"Error Invalid Item Number");
			return;
		}

		int remainingCargo;
		const auto cargo = Hk::Player::EnumCargo(client, remainingCargo);

		int itemCount = ToInt(GetParam(param, ' ', 2));

		if (itemCount <= 0)
		{
			PrintUserCmdText(client, L"Error Invalid Item Quantity");
			return;
		}

		const uint cash = Hk::Player::GetCash(client).value();

		if (cash < global->config.costPerStackWithdraw)
		{
			PrintUserCmdText(client, L"Not enough credits. The fee for storing items at this station is %u", global->config.costPerStackWithdraw);
			return;
		}

		const auto account = Hk::Client::GetAccountByClientID(client);
		const auto sqlBaseId = GetOrAddBase(base);
		const auto sqlPlayerId = GetOrAddPlayer(sqlBaseId, account);
		const auto warehouseItem = GetItemById(databaseItemId, sqlPlayerId);

		if (!warehouseItem.has_value())
		{
			PrintUserCmdText(client, L"Invalid Item Id");
			return;
		}

		const auto itemArch = Archetype::GetEquipment(warehouseItem->equipArchId);
		if (!itemArch)
		{
			Console::ConWarn(L"User tried to withdraw an item that no longer exists");
			PrintUserCmdText(client, L"Internal server error. Item does not exist.");
			return;
		}

		if (itemArch->fVolume * itemCount >= std::floor(remainingCargo))
		{
			PrintUserCmdText(client, L"Withdraw request denied. Your ship cannot accomodate cargo of this size");
			return;
		}

		const auto withdrawnQuantity = RemoveItem(databaseItemId, sqlPlayerId, itemCount);

		if (withdrawnQuantity == 0)
		{
			PrintUserCmdText(client, L"Invalid item Id");
			return;
		}

		Hk::Player::AddCargo(client, warehouseItem->equipArchId, static_cast<int>(withdrawnQuantity), false);
		Hk::Player::RemoveCash(client, global->config.costPerStackWithdraw);

		Hk::Player::SaveChar(client);

		PrintUserCmdText(
		    client, L"Successfully withdrawn Item: " + std::to_wstring(withdrawnQuantity) + L" " + Hk::Message::GetWStringFromIdS(itemArch->iIdsName));
	}

	void UserCmdWarehouse(ClientId& client, const std::wstring& param)
	{
		const std::wstring cmd = GetParam(param, ' ', 0);
		if (cmd.empty())
		{
			PrintUserCmdText(client, L"Usage: /warehouse store <itemId> <count>");
			PrintUserCmdText(client, L"Usage: /warehouse list");
			PrintUserCmdText(client, L"Usage: /warehouse withdraw <itemId> <count>");
			PrintUserCmdText(client, L"Usage: /warehouse liststored");
			return;
		}

		auto base = Hk::Player::GetCurrentBase(client);

		if (base.has_error())
		{
			PrintUserCmdText(client, L"You must be docked in order to use this command.");
			return;
		}

		if (cmd == L"store")
		{
			UserCmdStoreItem(client, param, base.value());
		}
		else if (cmd == L"list")
		{
			UserCmdGetItems(client, param, base.value());
		}
		else if (cmd == L"withdraw")
		{
			UserCmdWithdrawItem(client, param, base.value());
		}
		else if (cmd == L"liststored")
		{
			UserCmdGetWarehouseItems(client, param, base.value());
		}
		else
		{
			PrintUserCmdText(client, L"Invalid Command. Refer to /warehouse to see usage.");
		}
	}

	const std::vector commands = {{
	    CreateUserCommand(L"/warehouse", L"",UserCmdWarehouse, L""),
	}};

} // namespace Plugins::Warehouse

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace Plugins::Warehouse;

REFL_AUTO(type(Config), field(restrictedBases), field(restrictedItems), field(costPerStackWithdraw), field(costPerStackStore))

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	return true;
}

// Functions to hook
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("warehouse");
	pi->shortName("warehouse");
	pi->mayUnload(true);
	pi->returnCode(&global->returnCode);
	pi->commands(&commands);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
}
