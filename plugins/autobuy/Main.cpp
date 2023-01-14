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
		global->autobuyInfo[client].autoBuyMissiles = Hk::Ini::GetCharacterIniBool(client, L"autobuy.missiles");
		global->autobuyInfo[client].autoBuyMines = Hk::Ini::GetCharacterIniBool(client, L"autobuy.mine");
		global->autobuyInfo[client].autoBuyTorps = Hk::Ini::GetCharacterIniBool(client, L"autobuy.torp");
		global->autobuyInfo[client].autoBuyCD = Hk::Ini::GetCharacterIniBool(client, L"autobuy.cd");
		global->autobuyInfo[client].autoBuyCM = Hk::Ini::GetCharacterIniBool(client, L"autobuy.cm");
		global->autobuyInfo[client].autoBuyRepairs = Hk::Ini::GetCharacterIniBool(client, L"autobuy.repairs");
	}

	void ClearClientInfo(ClientId client) { global->autobuyInfo.erase(client); }

	// Put things that are performed on plugin load here!
	void LoadSettings()
	{
		const std::list<PLAYERINFO> players = Hk::Admin::GetPlayers();
		for (auto& p : players)
			LoadPlayerAutobuy(p.client);

	}

	int PlayerGetAmmoCount(const std::list<CARGO_INFO>& cargoList, uint itemArchId)
	{
		for (auto const& cargo : cargoList)
		{
			if (cargo.iArchId == itemArchId)
				return cargo.iCount;
		}

		return 0;
	}

	void AddEquipToCart(const std::list<CARGO_INFO>& cargo, std::list<AUTOBUY_CARTITEM>& cart, AUTOBUY_CARTITEM item, const std::wstring_view desc)
	{
		item.count = MAX_PLAYER_AMMO - PlayerGetAmmoCount(cargo, item.archId);
		item.description = desc;
		cart.emplace_back(item);
	}

	ClientInfo& LoadAutobuyInfo(ClientId client) {
		if (global->autobuyInfo.find(client) != global->autobuyInfo.end())
		{
			LoadPlayerAutobuy(client);
		}

		return global->autobuyInfo[client];
	}

	void PlayerAutoBuy(ClientId client, uint iBaseId)
	{

		const ClientInfo& clientInfo = LoadAutobuyInfo(client);

		// player cargo
		int remHoldSize;
		const auto cargo = Hk::Player::EnumCargo(client, remHoldSize);
		if (cargo.has_error())
		{
			return;
		}

		// shopping cart
		std::list<AUTOBUY_CARTITEM> lstCart;

		if (clientInfo.autoBuyRepairs)
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
				AUTOBUY_CARTITEM aci;
				if (item.iArchId == nanobotsId)
				{
					aci.archId = nanobotsId;
					aci.count = ship->iMaxNanobots - item.iCount;
					aci.description = L"Nanobots";
					lstCart.push_back(aci);
					nanobotsFound = true;
				}
				else if (item.iArchId == shieldBatsId)
				{
					aci.archId = shieldBatsId;
					aci.count = ship->iMaxShieldBats - item.iCount;
					aci.description = L"Shield Batteries";
					lstCart.push_back(aci);
					shieldBattsFound = true;
				}
			}

			if (!nanobotsFound)
			{ // no nanos found -> add all
				AUTOBUY_CARTITEM aci;
				aci.archId = nanobotsId;
				aci.count = ship->iMaxNanobots;
				aci.description = L"Nanobots";
				lstCart.push_back(aci);
			}

			if (!shieldBattsFound)
			{ // no batts found -> add all
				AUTOBUY_CARTITEM aci;
				aci.archId = shieldBatsId;
				aci.count = ship->iMaxShieldBats;
				aci.description = L"Shield Batteries";
				lstCart.push_back(aci);
			}
		}

		if (clientInfo.autoBuyCD || clientInfo.autoBuyCM || clientInfo.autoBuyMines ||
		    clientInfo.autoBuyMissiles || clientInfo.autoBuyTorps)
		{
			// add mounted equip to a new list and eliminate double equipment(such
			// as 2x lancer etc)
			std::list<CARGO_INFO> lstMounted;
			for (auto& item : cargo.value())
			{
				if (!item.bMounted)
					continue;

				bool bFound = false;
				for (const auto& mounted : lstMounted)
				{
					if (mounted.iArchId == item.iArchId)
					{
						bFound = true;
						break;
					}
				}

				if (!bFound)
					lstMounted.push_back(item);
			}

			// check mounted equip
			for (const auto& mounted : lstMounted)
			{
				AUTOBUY_CARTITEM aci;
				Archetype::Equipment* eq = Archetype::GetEquipment(mounted.iArchId);
				auto eqType = Hk::Client::GetEqType(eq);

				switch (eqType)
				{
					case ET_MINE: {
						if (clientInfo.autoBuyMines)
							AddEquipToCart(cargo.value(), lstCart, aci, L"Mines");

						break;
					}
					case ET_CM: {
						if (clientInfo.autoBuyCM)
							AddEquipToCart(cargo.value(), lstCart, aci, L"Countermeasures");

						break;
					}
					case ET_TORPEDO: {
						if (clientInfo.autoBuyTorps)
							AddEquipToCart(cargo.value(), lstCart, aci, L"Torpedoes");

						break;
					}
					case ET_CD: {
						if (clientInfo.autoBuyCD)
							AddEquipToCart(cargo.value(), lstCart, aci, L"Cruise Disrupters");

						break;
					}
					case ET_MISSILE: {
						if (clientInfo.autoBuyMissiles)
							AddEquipToCart(cargo.value(), lstCart, aci, L"Missiles");

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
			if (base.baseId == iBaseId)
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

		for (auto& buy : lstCart)
		{
			if (!buy.count || !Arch2Good(buy.archId))
				continue;

			// check if good is available and if player has the neccessary rep
			bool bGoodAvailable = false;
			for (const auto& available : bi->lstMarketMisc)
			{
				if (available.iArchId == buy.archId)
				{
					// get base rep
					int iSolarRep;
					pub::SpaceObj::GetSolarRep(bi->iObjectId, iSolarRep);
					uint iBaseRep;
					pub::Reputation::GetAffiliation(iSolarRep, iBaseRep);
					if (iBaseRep == -1)
						continue; // rep can't be determined yet(space object not
						          // created yet?)

					// get player rep
					int iRepId;
					pub::Player::GetRep(client, iRepId);

					// check if rep is sufficient
					float fPlayerRep;
					pub::Reputation::GetGroupFeelingsTowards(iRepId, iBaseRep, fPlayerRep);
					// good rep, allowedto buy
					if (fPlayerRep >= available.fRep)
						bGoodAvailable = true;
					break;
				}
			}

			if (!bGoodAvailable)
				continue; // base does not sell this item or bad rep

			float fPrice;
			if (pub::Market::GetPrice(iBaseId, buy.archId, fPrice) == -1)
				continue; // good not available

			const Archetype::Equipment* eq = Archetype::GetEquipment(buy.archId);
			if (eq->fVolume == 0.0f && static_cast < float > (remHoldSize) < std::ceil(eq->fVolume * static_cast<float>(buy.count)))
			{
				// round to the nearest possible
				auto newCount = static_cast<uint>(static_cast<float>(remHoldSize) / eq->fVolume);
				if (!newCount)
					continue;
				else
					buy.count = newCount;
			}

			uint uCost = ((uint)fPrice * buy.count);
			if (cash < uCost)
				PrintUserCmdText(client, L"Auto-Buy(%s): FAILED! Insufficient Credits", buy.description.c_str());
			else
			{
				Hk::Player::RemoveCash(client, uCost);
				remHoldSize -= ((int)eq->fVolume * buy.count);

				// add the item, dont use addcargo for performance/bug reasons
				// assume we only mount multicount goods (missiles, ammo, bots)
				pub::Player::AddCargo(client, buy.archId, buy.count, 1, false);

				PrintUserCmdText(client, L"Auto-Buy(%s): Bought %u unit(s), cost: %s$", buy.description.c_str(), buy.count, ToMoneyStr(uCost).c_str());
			}
		}
	}

	void UserCmdAutobuy(ClientId& client, const std::wstring& param)
	{

		ClientInfo& clientInfo = LoadAutobuyInfo(client);

		const std::wstring autobuyType = GetParam(param, ' ', 0);
		const std::wstring newState = GetParam(param, ' ', 1);

		std::vector<std::wstring> errorMessages = {
		L"Error: Invalid parameters",
		L"Usage: /autobuy <param> [<on/off>]",
		L"<Param>:",
		L"   info - display current autobuy-settings",
		L"   missiles - enable/disable autobuy for missiles",
		L"   torps - enable/disable autobuy for torpedos",
		L"   mines - enable/disable autobuy for mines",
		L"   cd - enable/disable autobuy for cruise disruptors",
		L"   cm - enable/disable autobuy for countermeasures",
		L"   reload - enable/disable autobuy for nanobots/shield batteries",
		L"   all: enable/disable autobuy for all of the above",
		L"Examples:",
		L"\"/autobuy missiles on\" enable autobuy for missiles",
		L"\"/autobuy all off\" completely disable autobuy",
		L"\"/autobuy info\" show autobuy info",
		};

		if (autobuyType.empty())
		{
			for (std::wstring messageText : errorMessages)
			{
				PrintUserCmdText(client, messageText.c_str());
			}
			return;
		}

		
	if (!autobuyType.compare(L"info"))
		{
			PrintUserCmdText(client, L"Missiles: %s", clientInfo.autoBuyMissiles ? L"On" : L"Off");
			PrintUserCmdText(client, L"Mine: %s", clientInfo.autoBuyMines ? L"On" : L"Off");
			PrintUserCmdText(client, L"Torpedos: %s", clientInfo.autoBuyTorps ? L"On" : L"Off");
			PrintUserCmdText(client, L"Cruise Disruptors: %s", clientInfo.autoBuyCD ? L"On" : L"Off");
			PrintUserCmdText(client, L"Countermeasures: %s", clientInfo.autoBuyCM ? L"On" : L"Off");
			PrintUserCmdText(client, L"Nanobots/Shield Batteries: %s", clientInfo.autoBuyRepairs ? L"On" : L"Off");
			return;
		}

		if (!autobuyType.length() || !newState.length() || newState.compare(L"on") != 0 && newState.compare(L"off") != 0)
		{
			PrintUserCmdText(client, L"ERR invalid parameters");
			return;
		}

		const auto fileName = Hk::Client::GetCharFileName(client);
		std::string scSection = "autobuy_" + wstos(fileName.value());

		bool enable = !newState.compare(L"on") ? true : false;
		if (!autobuyType.compare(L"all"))
		{
			clientInfo.autoBuyMissiles = enable;
			clientInfo.autoBuyMines = enable;
			clientInfo.autoBuyTorps = enable;
			clientInfo.autoBuyCD = enable;
			clientInfo.autoBuyCM = enable;
			clientInfo.autoBuyRepairs = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.missiles", stows(enable ? "true" : "false"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.mine", stows(enable ? "true" : "false"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.torp", stows(enable ? "true" : "false"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.cd", stows(enable ? "true" : "false"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.cm", stows(enable ? "true" : "false"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.repairs", stows(enable ? "true" : "false"));
		}
		else if (!autobuyType.compare(L"missiles"))
		{
			clientInfo.autoBuyMissiles = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.missiles", stows(enable ? "true" : "false"));
		}
		else if (!autobuyType.compare(L"mines"))
		{
			clientInfo.autoBuyMines = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.mines", stows(enable ? "true" : "false"));
		}
		else if (!autobuyType.compare(L"torps"))
		{
			clientInfo.autoBuyTorps = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.torps", stows(enable ? "true" : "false"));
		}
		else if (!autobuyType.compare(L"cd"))
		{
			clientInfo.autoBuyCD = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.cd", stows(enable ? "true" : "false"));
		}
		else if (!autobuyType.compare(L"cm"))
		{
			clientInfo.autoBuyCM = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.cm", stows(enable ? "true" : "false"));
		}
		else if (!autobuyType.compare(L"repairs"))
		{
			clientInfo.autoBuyRepairs = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.repairs", stows(enable ? "true" : "false"));
		}
		else
		{
			PrintUserCmdText(client, L"ERR invalid parameters");
			return;
		}

		PrintUserCmdText(client, L"OK");

	}

	// Define usable chat commands here
	const std::vector<UserCommand> commands = {{
	    {L"/autobuy", L"", UserCmdAutobuy, L"Sets up automatic purchases for consumables."},
	}};

}

using namespace Plugins::Autobuy;

DefaultDllMainSettings(LoadSettings)

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name("Autobuy");
	pi->shortName("autobuy");
	pi->mayUnload(true);
	pi->commands(commands);
	pi->returnCode(&global->returnCode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &PlayerAutoBuy, HookStep::After);
}