// This is a template with the bare minimum to have a functional plugin.
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

// Includes
#include "Main.h"

namespace Plugins::Autobuy
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	void LoadPlayerAutobuy(ClientId client)
	{
		AutobuyInfo playerAutobuyInfo{};
		playerAutobuyInfo.missiles = Hk::Ini::GetCharacterIniBool(client, L"autobuy.missiles");
		playerAutobuyInfo.mines = Hk::Ini::GetCharacterIniBool(client, L"autobuy.mines");
		playerAutobuyInfo.torps = Hk::Ini::GetCharacterIniBool(client, L"autobuy.torps");
		playerAutobuyInfo.cd = Hk::Ini::GetCharacterIniBool(client, L"autobuy.cd");
		playerAutobuyInfo.cm = Hk::Ini::GetCharacterIniBool(client, L"autobuy.cm");
		playerAutobuyInfo.repairs = Hk::Ini::GetCharacterIniBool(client, L"autobuy.repairs");
		global->autobuyInfo[client] = playerAutobuyInfo;
	}

	void ClearClientInfo(ClientId& client) { global->autobuyInfo.erase(client); }

	int PlayerGetAmmoCount(const std::list<CARGO_INFO>& cargoList, uint itemArchId)
	{
		for (auto const& cargo : cargoList)
		{
			if (cargo.iArchId == itemArchId)
				return cargo.iCount;
		}

		return 0;
	}

	void AddEquipToCart(const Archetype::Launcher* launcher, const std::list<CARGO_INFO>& cargo, std::list<AutobuyCartItem>& cart, AutobuyCartItem item, const std::wstring_view desc)
	{
		// TODO: Update to per-weapon ammo limits once implemented
		item.archId = launcher->iProjectileArchId;
		item.count = MAX_PLAYER_AMMO - PlayerGetAmmoCount(cargo, item.archId);
		item.description = desc;
		cart.emplace_back(item);
	}

	AutobuyInfo& LoadAutobuyInfo(ClientId& client) 
	{
		if (!global->autobuyInfo.contains(client))
		{
			LoadPlayerAutobuy(client);
		}

		return global->autobuyInfo[client];
	}

	void OnBaseEnter(BaseId& baseId, ClientId& client)
	{
		const AutobuyInfo& clientInfo = LoadAutobuyInfo(client);

		// player cargo
		int remHoldSize;
		const auto cargo = Hk::Player::EnumCargo(client, remHoldSize);
		if (cargo.has_error())
		{
			return;
		}

		// shopping cart
		std::list<AutobuyCartItem> cartList;

		if (clientInfo.repairs)
		{
			// shield bats & nanobots
			Archetype::Ship const* ship = Archetype::GetShip(Players[client].shipArchetype);

			uint nanobotsId;
			pub::GetGoodID(nanobotsId, "ge_s_repair_01");
			uint shieldBatsId;
			pub::GetGoodID(shieldBatsId, "ge_s_battery_01");
			bool nanobotsFound = false;
			bool shieldBattsFound = false;
			for (auto& item : cargo.value())
			{
				AutobuyCartItem aci;
				if (item.iArchId == nanobotsId)
				{
					aci.archId = nanobotsId;
					aci.count = ship->iMaxNanobots - item.iCount;
					aci.description = L"Nanobots";
					cartList.push_back(aci);
					nanobotsFound = true;
				}
				else if (item.iArchId == shieldBatsId)
				{
					aci.archId = shieldBatsId;
					aci.count = ship->iMaxShieldBats - item.iCount;
					aci.description = L"Shield Batteries";
					cartList.push_back(aci);
					shieldBattsFound = true;
				}
			}

			if (!nanobotsFound)
			{ // no nanos found -> add all
				AutobuyCartItem aci;
				aci.archId = nanobotsId;
				aci.count = ship->iMaxNanobots;
				aci.description = L"Nanobots";
				cartList.push_back(aci);
			}

			if (!shieldBattsFound)
			{ // no batts found -> add all
				AutobuyCartItem aci;
				aci.archId = shieldBatsId;
				aci.count = ship->iMaxShieldBats;
				aci.description = L"Shield Batteries";
				cartList.push_back(aci);
			}
		}

		if (clientInfo.cd || clientInfo.cm || clientInfo.mines ||
		    clientInfo.missiles || clientInfo.torps)
		{
			// add mounted equip to a new list and eliminate double equipment(such
			// as 2x lancer etc)
			std::list<CARGO_INFO> mountedList;
			for (auto& item : cargo.value())
			{
				if (!item.bMounted)
					continue;

				bool found = false;
				for (const auto& mounted : mountedList)
				{
					if (mounted.iArchId == item.iArchId)
					{
						found = true;
						break;
					}
				}

				if (!found)
					mountedList.push_back(item);
			}

			// check mounted equip
			for (const auto& mounted : mountedList)
			{
				AutobuyCartItem aci;
				Archetype::Equipment* eq = Archetype::GetEquipment(mounted.iArchId);
				auto eqType = Hk::Client::GetEqType(eq);

				switch (eqType)
				{
					case ET_MINE: {
						if (clientInfo.mines)
							AddEquipToCart(static_cast<Archetype::Launcher*>(eq), cargo.value(), cartList, aci, L"Mines");

						break;
					}
					case ET_CM: {
						if (clientInfo.cm)
							AddEquipToCart(static_cast<Archetype::Launcher*>(eq), cargo.value(), cartList, aci, L"Countermeasures");

						break;
					}
					case ET_TORPEDO: {
						if (clientInfo.torps)
							AddEquipToCart(static_cast<Archetype::Launcher*>(eq), cargo.value(), cartList, aci, L"Torpedoes");

						break;
					}
					case ET_CD: {
						if (clientInfo.cd)
							AddEquipToCart(static_cast<Archetype::Launcher*>(eq), cargo.value(), cartList, aci, L"Cruise Disrupters");

						break;
					}
					case ET_MISSILE: {
						if (clientInfo.missiles)
							AddEquipToCart(static_cast<Archetype::Launcher*>(eq), cargo.value(), cartList, aci, L"Missiles");

						break;
					}

					default:
						break;
				}
			}
		}

		// search base in base-info list
		BASE_INFO const* bi = nullptr;
		for (auto const& base : lstBases)
		{
			if (base.baseId == baseId)
			{
				bi = &base;
				break;
			}
		}

		if (!bi)
			return; // base not found

		const auto cashErr = Hk::Player::GetCash(client);
		if (cashErr.has_error())
		{
			return;
		}

		auto cash = cashErr.value();

		for (auto& buy : cartList)
		{
			if (!buy.count || !Arch2Good(buy.archId))
				continue;

			// check if good is available and if player has the neccessary rep
			bool goodAvailable = false;
			for (const auto& available : bi->lstMarketMisc)
			{
				if (available.iArchId == buy.archId)
				{
					auto baseRep = Hk::Solar::GetAffiliation(bi->iObjectId);
					if (baseRep.has_error())
						PrintUserCmdText(client, Hk::Err::ErrGetText(baseRep.error()));

					const auto playerRep = Hk::Player::GetRep(client, baseRep.value());
					if (playerRep.has_error())
						PrintUserCmdText(client, Hk::Err::ErrGetText(playerRep.error()));
					
					// good rep, allowed to buy
					if (playerRep.value() >= available.fRep)
						goodAvailable = true;
					break;
				}
			}

			if (!goodAvailable)
				continue; // base does not sell this item or bad rep
			auto goodPrice = Hk::Solar::GetCommodityPrice(baseId, buy.archId);
			if (goodPrice.has_error())
				continue; // good not available

			const Archetype::Equipment* eq = Archetype::GetEquipment(buy.archId);
			if (eq->fVolume == 0.0f && static_cast < float > (remHoldSize) < std::ceil(eq->fVolume * static_cast<float>(buy.count)))
			{
				// round to the nearest possible
				auto newCount = static_cast<uint>(static_cast<float>(remHoldSize) / eq->fVolume);
				if (!newCount)
				{
					PrintUserCmdText(client, L"Auto-Buy(%s): FAILED! Insufficient Cargo Space", buy.description.c_str());
					continue;
				}
				else
					buy.count = newCount;
			}

			if (uint uCost = (static_cast<uint>(goodPrice.value()) * buy.count); cash < uCost)
				PrintUserCmdText(client, L"Auto-Buy(%s): FAILED! Insufficient Credits", buy.description.c_str());
			else
			{
				Hk::Player::RemoveCash(client, uCost);
				remHoldSize -= ((int)eq->fVolume * buy.count);

				// add the item, dont use addcargo for performance/bug reasons
				// assume we only mount multicount goods (missiles, ammo, bots
				Hk::Player::AddCargo(client, buy.archId, buy.count, false);

				PrintUserCmdText(client, L"Auto-Buy(%s): Bought %u unit(s), cost: %s$", buy.description.c_str(), buy.count, ToMoneyStr(uCost).c_str());
			}

		}
		Hk::Player::SaveChar(client);
	}

	void UserCmdAutobuy(ClientId& client, const std::wstring& param)
	{

		AutobuyInfo& autobuyInfo = LoadAutobuyInfo(client);

		const std::wstring autobuyType = GetParam(param, ' ', 0);
		const std::wstring newState = GetParam(param, ' ', 1);

		if (autobuyType.empty())
		{
			PrintUserCmdText(client, L"Error: Invalid parameters");
			PrintUserCmdText(client, L"Usage: /autobuy <param> [<on/off>]");
			PrintUserCmdText(client, L"<Param>:");
			PrintUserCmdText(client, L"|  info - display current autobuy-settings");
			PrintUserCmdText(client, L"|  missiles - enable/disable autobuy for missiles");
			PrintUserCmdText(client, L"|  torps - enable/disable autobuy for torpedos");
			PrintUserCmdText(client, L"|  mines - enable/disable autobuy for mines");
			PrintUserCmdText(client, L"|  cd - enable/disable autobuy for cruise disruptors");
			PrintUserCmdText(client, L"|  cm - enable/disable autobuy for countermeasures");
			PrintUserCmdText(client, L"|  repairs - enable/disable autobuy for nanobots/shield batteries");
			PrintUserCmdText(client, L"|  all: enable/disable autobuy for all of the above");
			PrintUserCmdText(client, L"Examples:");
			PrintUserCmdText(client, L"|  \"/autobuy missiles on\" enable autobuy for missiles");
			PrintUserCmdText(client, L"|  \"/autobuy all off\" completely disable autobuy");
			PrintUserCmdText(client, L"|  \"/autobuy info\" show autobuy info");
		}

		
	if (!autobuyType.compare(L"info"))
		{
			PrintUserCmdText(client, L"Missiles: %s", autobuyInfo.missiles ? L"On" : L"Off");
			PrintUserCmdText(client, L"Mines: %s", autobuyInfo.mines ? L"On" : L"Off");
			PrintUserCmdText(client, L"Torpedos: %s", autobuyInfo.torps ? L"On" : L"Off");
			PrintUserCmdText(client, L"Cruise Disruptors: %s", autobuyInfo.cd ? L"On" : L"Off");
			PrintUserCmdText(client, L"Countermeasures: %s", autobuyInfo.cm ? L"On" : L"Off");
			PrintUserCmdText(client, L"Nanobots/Shield Batteries: %s", autobuyInfo.repairs ? L"On" : L"Off");
			return;
		}

		if (!autobuyType.length() || !newState.length() || (newState != L"on" && newState != L"off"))
		{
			PrintUserCmdText(client, L"ERR invalid parameters");
			return;
		}

		const auto fileName = Hk::Client::GetCharFileName(client);
		std::string scSection = "autobuy_" + wstos(fileName.value());

		bool enable = newState == L"on";
		if (autobuyType == L"all")
		{
			autobuyInfo.missiles = enable;
			autobuyInfo.mines = enable;
			autobuyInfo.torps = enable;
			autobuyInfo.cd = enable;
			autobuyInfo.cm = enable;
			autobuyInfo.repairs = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.missiles", stows(enable ? "true" : "false"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.mines", stows(enable ? "true" : "false"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.torps", stows(enable ? "true" : "false"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.cd", stows(enable ? "true" : "false"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.cm", stows(enable ? "true" : "false"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.repairs", stows(enable ? "true" : "false"));
		}
		else if (autobuyType == L"missiles")
		{
			autobuyInfo.missiles = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.missiles", stows(enable ? "true" : "false"));
		}
		else if (autobuyType == L"mines")
		{
			autobuyInfo.mines = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.mines", stows(enable ? "true" : "false"));
		}
		else if (autobuyType == L"torps")
		{
			autobuyInfo.torps = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.torps", stows(enable ? "true" : "false"));
		}
		else if (autobuyType == L"cd")
		{
			autobuyInfo.cd = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.cd", stows(enable ? "true" : "false"));
		}
		else if (autobuyType == L"cm")
		{
			autobuyInfo.cm = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.cm", stows(enable ? "true" : "false"));
		}
		else if (autobuyType == L"repairs")
		{
			autobuyInfo.repairs = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.repairs", stows(enable ? "true" : "false"));
		}
		else
		{
			PrintUserCmdText(client, L"ERR invalid parameters");
			return;
		}

		Hk::Player::SaveChar(client);
		PrintUserCmdText(client, L"OK");

	}

	// Define usable chat commands here
	const std::vector commands = {{
	    CreateUserCommand(L"/autobuy", L"<consumable type> <on/off>", UserCmdAutobuy, L"Sets up automatic purchases for consumables."),
	}};

}

using namespace Plugins::Autobuy;

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Autobuy");
	pi->shortName("autobuy");
	pi->mayUnload(true);
	pi->commands(&commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &OnBaseEnter, HookStep::After);
}