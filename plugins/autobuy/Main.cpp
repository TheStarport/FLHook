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

	// Put things that are performed on plugin load here!
	void LoadSettings()
	{
		// Load JSON config
		auto config = Serializer::JsonToObject<Config>();
		global->config = std::make_unique<Config>(std::move(config));
	}

	int PlayerAutoBuyGetCount(const std::list<CARGO_INFO>& cargoList, uint itemArchId)
	{
		for (auto const& cargo : cargoList)
		{
			if (cargo.iArchId == itemArchId)
				return cargo.iCount;
		}

		return 0;
	}

	void AddEquipToCart(const std::list<CARGO_INFO>& cargo, std::list<AUTOBUY_CARTITEM>& cart, AUTOBUY_CARTITEM item, const std::wstring desc)
	{
		item.iCount = MAX_PLAYER_AMMO - PlayerAutoBuyGetCount(cargo, item.iArchId);
		item.wscDescription = desc;
		cart.emplace_back(item);
	}

	void PlayerAutoBuy(ClientId client, uint iBaseId)
	{
		// player cargo
		int iRemHoldSize;
		const auto cargo = Hk::Player::EnumCargo(client, iRemHoldSize);
		if (cargo.has_error())
		{
			return;
		}

		// shopping cart
		std::list<AUTOBUY_CARTITEM> lstCart;

		if (global->autobuyInfo[client].autoBuyRepairs)
		{
			// shield bats & nanobots
			Archetype::Ship const* ship = Archetype::GetShip(Players[client].shipArchetype);

			uint iNanobotsId;
			pub::GetGoodID(iNanobotsId, "ge_s_repair_01");
			uint iShieldBatsId;
			pub::GetGoodID(iShieldBatsId, "ge_s_battery_01");
			bool bNanobotsFound = false;
			bool bShieldBattsFound = false;
			for (auto& item : cargo.value())
			{
				AUTOBUY_CARTITEM aci;
				if (item.iArchId == iNanobotsId)
				{
					aci.iArchId = iNanobotsId;
					aci.iCount = ship->iMaxNanobots - item.iCount;
					aci.wscDescription = L"Nanobots";
					lstCart.push_back(aci);
					bNanobotsFound = true;
				}
				else if (item.iArchId == iShieldBatsId)
				{
					aci.iArchId = iShieldBatsId;
					aci.iCount = ship->iMaxShieldBats - item.iCount;
					aci.wscDescription = L"Shield Batteries";
					lstCart.push_back(aci);
					bShieldBattsFound = true;
				}
			}

			if (!bNanobotsFound)
			{ // no nanos found -> add all
				AUTOBUY_CARTITEM aci;
				aci.iArchId = iNanobotsId;
				aci.iCount = ship->iMaxNanobots;
				aci.wscDescription = L"Nanobots";
				lstCart.push_back(aci);
			}

			if (!bShieldBattsFound)
			{ // no batts found -> add all
				AUTOBUY_CARTITEM aci;
				aci.iArchId = iShieldBatsId;
				aci.iCount = ship->iMaxShieldBats;
				aci.wscDescription = L"Shield Batteries";
				lstCart.push_back(aci);
			}
		}

		if (global->autobuyInfo[client].autoBuyCD || global->autobuyInfo[client].autoBuyCM || global->autobuyInfo[client].autoBuyMines ||
		    global->autobuyInfo[client].autoBuyMissiles || global->autobuyInfo[client].autoBuyTorps)
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
				uint i = mounted.iArchId;
				AUTOBUY_CARTITEM aci;
				Archetype::Equipment* eq = Archetype::GetEquipment(mounted.iArchId);
				auto eqType = Hk::Client::GetEqType(eq);

				switch (eqType)
				{
					case ET_MINE: {
						if (global->autobuyInfo[client].autoBuyMines)
							AddEquipToCart(cargo.value(), lstCart, aci, L"Mines");

						break;
					}
					case ET_CM: {
						if (global->autobuyInfo[client].autoBuyCM)
							AddEquipToCart(cargo.value(), lstCart, aci, L"Countermeasures");

						break;
					}
					case ET_TORPEDO: {
						if (global->autobuyInfo[client].autoBuyTorps)
							AddEquipToCart(cargo.value(), lstCart, aci, L"Torpedoes");

						break;
					}
					case ET_CD: {
						if (global->autobuyInfo[client].autoBuyCD)
							AddEquipToCart(cargo.value(), lstCart, aci, L"Cruise Disrupters");

						break;
					}
					case ET_MISSILE: {
						if (global->autobuyInfo[client].autoBuyMissiles)
							AddEquipToCart(cargo.value(), lstCart, aci, L"Missiles");

						break;
					}

					default:
						break;
				}
			}
		}

		// search base in base-info list
		BASE_INFO* bi = nullptr;
		for (auto& base : lstBases)
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
			if (!buy.iCount || !Arch2Good(buy.iArchId))
				continue;

			// check if good is available and if player has the neccessary rep
			bool bGoodAvailable = false;
			for (const auto& available : bi->lstMarketMisc)
			{
				if (available.iArchId == buy.iArchId)
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
			if (pub::Market::GetPrice(iBaseId, buy.iArchId, fPrice) == -1)
				continue; // good not available

			const Archetype::Equipment* eq = Archetype::GetEquipment(buy.iArchId);
			if (iRemHoldSize < (eq->fVolume * buy.iCount))
			{
				uint iNewCount = (uint)(iRemHoldSize / eq->fVolume);
				if (!iNewCount)
				{
					//				PrintUserCmdText(client,
					// L"Auto-Buy(%s): FAILED! Insufficient cargo space",
					// (*it4).wscDescription.c_str());
					continue;
				}
				else
					buy.iCount = iNewCount;
			}

			uint uCost = ((uint)fPrice * buy.iCount);
			if (cash < uCost)
				PrintUserCmdText(client, L"Auto-Buy(%s): FAILED! Insufficient Credits", buy.wscDescription.c_str());
			else
			{
				Hk::Player::RemoveCash(client, uCost);
				iRemHoldSize -= ((int)eq->fVolume * buy.iCount);

				// add the item, dont use addcargo for performance/bug reasons
				// assume we only mount multicount goods (missiles, ammo, bots)
				pub::Player::AddCargo(client, buy.iArchId, buy.iCount, 1, false);

				PrintUserCmdText(client, L"Auto-Buy(%s): Bought %u unit(s), cost: %s$", buy.wscDescription.c_str(), buy.iCount, ToMoneyStr(uCost).c_str());
			}
		}
	}

	// Demo command
	void UserCmdAutobuy(ClientId& client, const std::wstring& param)
	{ 
		if (!global->config->enableAutobuy)
			return;

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
			PrintUserCmdText(client, L"Missiles: %s", global->autobuyInfo[client].autoBuyMissiles ? L"On" : L"Off");
			PrintUserCmdText(client, L"Mine: %s", global->autobuyInfo[client].autoBuyMines ? L"On" : L"Off");
			PrintUserCmdText(client, L"Torpedos: %s", global->autobuyInfo[client].autoBuyTorps ? L"On" : L"Off");
			PrintUserCmdText(client, L"Cruise Disruptors: %s", global->autobuyInfo[client].autoBuyCD ? L"On" : L"Off");
			PrintUserCmdText(client, L"Countermeasures: %s", global->autobuyInfo[client].autoBuyCM ? L"On" : L"Off");
			PrintUserCmdText(client, L"Nanobots/Shield Batteries: %s", global->autobuyInfo[client].autoBuyRepairs ? L"On" : L"Off");
			return;
		}

		if (!autobuyType.length() || !autobuyType.length() || newState.compare(L"on") != 0 && newState.compare(L"off") != 0)
		{
			PrintUserCmdText(client, L"ERR invalid parameters");
			return;
		}

		const auto fileName = Hk::Client::GetCharFileName(client);
		std::string scSection = "autobuy_" + wstos(fileName.value());

		bool enable = !newState.compare(L"on") ? true : false;
		if (!autobuyType.compare(L"all"))
		{
			global->autobuyInfo[client].autoBuyMissiles = enable;
			global->autobuyInfo[client].autoBuyMines = enable;
			global->autobuyInfo[client].autoBuyTorps = enable;
			global->autobuyInfo[client].autoBuyCD = enable;
			global->autobuyInfo[client].autoBuyCM = enable;
			global->autobuyInfo[client].autoBuyRepairs = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.missiles", stows(enable ? "yes" : "no"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.mine", stows(enable ? "yes" : "no"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.torp", stows(enable ? "yes" : "no"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.cd", stows(enable ? "yes" : "no"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.cm", stows(enable ? "yes" : "no"));
			Hk::Ini::SetCharacterIni(client, L"autobuy.repairs", stows(enable ? "yes" : "no"));
		}
		else if (!autobuyType.compare(L"missiles"))
		{
			global->autobuyInfo[client].autoBuyMissiles = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.missiles", stows(enable ? "yes" : "no"));
		}
		else if (!autobuyType.compare(L"mines"))
		{
			global->autobuyInfo[client].autoBuyMines = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.mines", stows(enable ? "yes" : "no"));
		}
		else if (!autobuyType.compare(L"torps"))
		{
			global->autobuyInfo[client].autoBuyTorps = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.torps", stows(enable ? "yes" : "no"));
		}
		else if (!autobuyType.compare(L"cd"))
		{
			global->autobuyInfo[client].autoBuyCD = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.cd", stows(enable ? "yes" : "no"));
		}
		else if (!autobuyType.compare(L"cm"))
		{
			global->autobuyInfo[client].autoBuyCM = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.cm", stows(enable ? "yes" : "no"));
		}
		else if (!autobuyType.compare(L"repairs"))
		{
			global->autobuyInfo[client].autoBuyRepairs = enable;
			Hk::Ini::SetCharacterIni(client, L"autobuy.repairs", stows(enable ? "yes" : "no"));
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

REFL_AUTO(type(Config), field(enableAutobuy));

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
	pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings, HookStep::After);
	pi->emplaceHook(HookedCall::IServerImpl__BaseEnter, &PlayerAutoBuy, HookStep::After);
}