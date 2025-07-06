// Includes
#include "PCH.hpp"

#include "AutobuyPlugin.hpp"

namespace Plugins
{
    AutobuyPlugin::AutobuyPlugin(const PluginInfo& info) : Plugin(info), autobuyInfo() {}

    void AutobuyPlugin::LoadPlayerAutobuy(const ClientId client)
    {
        auto& [updated, ammo, mines, cm, bb, repairs] = autobuyInfo[client.GetValue()];

        const auto view = client.GetData().characterData;
        if (auto autobuyDoc = view->characterDocument.find("autobuy"); autobuyDoc != view->characterDocument.end())
        {
            for (const bsoncxx::document::view subDoc = autobuyDoc->get_document().view(); auto& item : subDoc)
            {
                switch (Hash(item.key().data()))
                {
                    case Hash("repairs"): repairs = item.get_bool(); break;
                    case Hash("bb"): bb = item.get_bool(); break;
                    case Hash("cm"): cm = item.get_bool(); break;
                    case Hash("mines"): mines = item.get_bool(); break;
                    case Hash("ammo"): ammo = item.get_bool(); break;
                    default:;
                }
            }
        }
    }

    void AutobuyPlugin::OnClearClientInfo(const ClientId client) { autobuyInfo[client.GetValue()] = {}; }

    int PlayerGetAmmoCount(const EquipDescList* cargoList, Id itemArchId)
    {
        for (auto& cargo : cargoList->equip)
        {
            if (cargo.archId == itemArchId)
            {
                return cargo.count;
            }
        }
        return 0;
    }

    void HandleRepairs(const ClientId client)
    {
        auto repairCost = static_cast<uint>(client.GetShipArch().Unwrap().GetValue()->hitPoints * (1 - client.GetRelativeHealth().Unwrap()) / 3);

        std::vector<ushort> eqToFix;

        for (const auto& item : client.GetEquipCargo().Unwrap()->equip)
        {
            if (!item.mounted || item.health == 1)
            {
                continue;
            }

            const GoodInfo* info = GoodList_get()->find_by_archetype(item.archId.GetValue());
            if (!info)
            {
                continue;
            }

            repairCost += static_cast<uint>(info->price * (1.0f - item.health) / 3);
            eqToFix.emplace_back(item.id);
        }

        if (const uint playerCash = client.GetCash().Unwrap(); playerCash < repairCost)
        {
            (void)client.Message(L"Insufficient Cash");
            return;
        }

        if (repairCost)
        {
            client.Message(std::format(L"Auto-Buy: Ship repair costed {}$", repairCost));
            (void)client.RemoveCash(repairCost);
        }

        if (!eqToFix.empty())
        {
            auto currEq = eqToFix.begin();
            auto& equip = *client.GetEquipCargo().Unwrap();
            for (auto& item : equip.equip)
            {
                if (item.id == *currEq)
                {
                    item.health = 1.0f;
                    ++currEq;
                }
            }

            // TODO: Uncomment if anticheat gets angry
            // auto& equip = Players[client].equipDescList.equip;
            //
            // if (&equip != &Players[client].shadowEquipDescList.equip)
            //{
            //    Players[client].shadowEquipDescList.equip = equip;
            //}

            st6::vector<EquipDesc> eqVector;
            for (auto& eq : equip.equip)
            {
                if (eq.mounted)
                {
                    eq.health = 1.0f;
                }
                eqVector.push_back(eq);
            }

            FLHook::GetPacketInterface()->Send_FLPACKET_SERVER_SETEQUIPMENT(client.GetValue(), eqVector);
        }

        if (auto& playerCollision = *client.GetCollisionGroups().Unwrap(); !playerCollision.empty())
        {
            for (auto& colGrp : playerCollision)
            {
                colGrp.health = 1.0f;
            }
            FLHook::GetPacketInterface()->Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(client.GetValue(), playerCollision);
        }

        if (client.GetRelativeHealth().Unwrap() < 1.0f)
        {
            (void)client.SetRelativeHealth(1.0f);
            FLHook::GetPacketInterface()->Send_FLPACKET_SERVER_SETHULLSTATUS(client.GetValue(), 1.0f);
        }
    }

    void AutobuyPlugin::AddEquipToCart(const Archetype::Launcher* launcher, const EquipDescList* cargo, std::map<Id, AutobuyCartItem>& cart,
                                       AutobuyCartItem& item, const std::wstring_view& desc)
    {
        // TODO: Update to per-weapon ammo limits once implemented
        item.archId = launcher->projectileArchId;
        Id itemId = Id(Arch2Good(item.archId.GetValue()));
        if (ammoLimits.contains(itemId))
        {
            item.count = ammoLimits[itemId] - PlayerGetAmmoCount(cargo, item.archId);
        }
        else
        {
            item.count = MAX_PLAYER_AMMO - PlayerGetAmmoCount(cargo, item.archId);
        }
        cart[item.archId] = item;
    }

    void AutobuyPlugin::OnCharacterSelectAfter(const ClientId client) { LoadPlayerAutobuy(client); }

    void AutobuyPlugin::OnCharacterSave(const ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document)
    {
        using bsoncxx::builder::basic::kvp;
        const auto& [updated, ammo, mines, cm, bb, repairs] = autobuyInfo[client.GetValue()];

        if (!updated)
        {
            return;
        }

        document.append(kvp(
            "autobuy",
            bsoncxx::builder::basic::make_document(
                kvp("cm", cm), kvp("bb", bb), kvp("repairs", repairs), kvp("mines", mines), kvp("ammo", ammo))));
    }

    void AutobuyPlugin::OnBaseEnterAfter(const BaseId baseId, const ClientId client)
    {
        const Archetype::Ship* ship = client.GetShipArch().Unwrap().Cast<Archetype::Ship>().Handle();

        // player cargo
        float remHoldSize = client.GetRemainingCargo().Unwrap();
        auto equipCargo = client.GetEquipCargo().Unwrap();

        // shopping cart
        std::map<Id, AutobuyCartItem> cartMap;

        const auto& [updated, ammo, mines, cm, bb, repairs] = autobuyInfo[client.GetValue()];
        if (bb)
        {
            // shield bats & nanobots

            bool nanobotsFound = false;
            bool shieldBattsFound = false;
            for (auto& item : equipCargo->equip)
            {
                if (item.archId == config.nanobot.GetId())
                {
                    AutobuyCartItem aci;
                    aci.archId = config.nanobot.GetId();
                    aci.count = ship->maxNanobots - item.count;
                    aci.description = L"Nanobots";
                    cartMap[aci.archId] = aci;
                    nanobotsFound = true;
                }
                else if (item.archId == config.shieldBattery.GetId())
                {
                    AutobuyCartItem aci;
                    aci.archId = config.shieldBattery.GetId();
                    aci.count = ship->maxShieldBats - item.count;
                    aci.description = L"Shield Batteries";
                    cartMap[aci.archId] = aci;
                    shieldBattsFound = true;
                }
            }

            if (!nanobotsFound)
            { // no nanos found -> add all
                AutobuyCartItem aci;
                aci.archId = config.nanobot.GetId();
                aci.count = ship->maxNanobots;
                aci.description = L"Nanobots";
                cartMap[aci.archId] = aci;
            }

            if (!shieldBattsFound)
            { // no batts found -> add all
                AutobuyCartItem aci;
                aci.archId = config.shieldBattery.GetId();
                aci.count = ship->maxShieldBats;
                aci.description = L"Shield Batteries";
                cartMap[aci.archId] = aci;
            }
        }

        if (ammo || cm || mines)
        {
            // check mounted equip
            for (const auto& equip : equipCargo->equip)
            {
                if (!equip.mounted)
                {
                    continue;
                }
                AutobuyCartItem aci;

                switch (Archetype::Equipment* eq = Archetype::GetEquipment(equip.archId.GetValue()); EquipmentId(eq->archId).GetType().Unwrap())
                {
                    case EquipmentType::MineDropper:
                        {
                            if (mines)
                            {
                                AddEquipToCart(dynamic_cast<Archetype::Launcher*>(eq), equipCargo, cartMap, aci, L"Mines");
                            }

                            break;
                        }
                    case EquipmentType::CmDropper:
                        {
                            if (cm)
                            {
                                AddEquipToCart(dynamic_cast<Archetype::Launcher*>(eq), equipCargo, cartMap, aci, L"Countermeasures");
                            }

                            break;
                        }
                    case EquipmentType::Gun:
                    case EquipmentType::TorpedoLauncher:
                    case EquipmentType::CdLauncher:
                    case EquipmentType::MissileLauncher:
                        {
                            if (ammo)
                            {
                                AddEquipToCart(dynamic_cast<Archetype::Launcher*>(eq), equipCargo, cartMap, aci, L"Ammo");
                            }

                            break;
                        }

                    default: break;
                }
            }
        }

        if (repairs)
        {
            HandleRepairs(client);
        }

        BaseData* bi = BaseDataList_get()->get_base_data(baseId.GetValue());

        if (!bi)
        {
            return; // base not found
        }

        const auto cash = client.GetCash().Unwrap();

        RepId clientRep = client.GetReputation().Handle();
        RepId baseRep = baseId.GetAffiliation().Handle();
        float baseStanding = clientRep.GetAttitudeTowardsRepId(baseRep).Handle();

        for (auto& [key, value] : cartMap)
        {
            if (!value.count || !Arch2Good(value.archId.GetValue()))
            {
                continue;
            }

            const auto itemIter = bi->marketMap.find(value.archId);
            if (itemIter == bi->marketMap.end())
            {
                continue;
            }

            MarketGoodInfo& marketInfo = itemIter->second;

            // check if good is available and if player has the neccessary rep
            if (marketInfo.rep > baseStanding)
            {
                continue;
            }

            const Archetype::Equipment* eq = Archetype::GetEquipment(value.archId.GetValue());
            uint amountToBuy = value.count;
            // will always fail for volume == 0, no need to worry about potential div by 0
            if (remHoldSize < eq->volume * static_cast<float>(value.count))
            {
                // round to the nearest possible
                if (const auto newCount = static_cast<uint>(remHoldSize / eq->volume); !newCount)
                {
                    client.Message(std::format(L"Auto-Buy({}): FAILED! Insufficient Cargo Space", value.description));
                    continue;
                }
                else
                {
                    amountToBuy = newCount;
                }
            }

            if (uint uCost = static_cast<uint>(marketInfo.price) * amountToBuy; cash < uCost)
            {
                client.Message(std::format(L"Auto-Buy({}): FAILED! Insufficient Credits", value.description));
            }
            else
            {
                (void)client.RemoveCash(uCost);
                remHoldSize -= eq->volume * static_cast<float>(amountToBuy);

                // add the item, dont use addcargo for performance/bug reasons
                // assume we only mount multicount goods (missiles, ammo, bots
                (void)client.AddCargo(value.archId, amountToBuy, false);

                client.Message(std::format(L"Auto-Buy({}): Bought {} unit(s), cost: {}$", value.description, amountToBuy, StringUtils::ToMoneyStr(uCost)));
            }
        }
        (void)client.SaveChar();
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // USER COMMANDS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    concurrencpp::result<void>AutobuyPlugin::UserCmdAutobuy(const ClientId client, const std::wstring_view autobuyType, const std::wstring_view newState)
    {
        if (autobuyType.empty())
        {
            (void)client.Message(L"Error: Invalid parameters");
            (void)client.Message(L"Usage: /autobuy <param> [<on/off>]");
            (void)client.Message(L"<Param>:");
            (void)client.Message(L"|  info - display current autobuy-settings");
            (void)client.Message(L"|  ammo - enable/disable autobuy for ammo");
            (void)client.Message(L"|  mines - enable/disable autobuy for mines");
            (void)client.Message(L"|  cm - enable/disable autobuy for countermeasures");
            (void)client.Message(L"|  bb - enable/disable autobuy for nanobots/shield batteries");
            (void)client.Message(L"|  repairs - enable/disable automatic repair of ship and equipment");
            (void)client.Message(L"|  all: enable/disable autobuy for all of the above");
            (void)client.Message(L"Examples:");
            (void)client.Message(L"|  \"/autobuy ammo on\" enable autobuy of ammunition");
            (void)client.Message(L"|  \"/autobuy all off\" completely disable autobuy");
            (void)client.Message(L"|  \"/autobuy info\" show autobuy info");
        }

        auto& [updated, ammo, mines, cm, bb, repairs] = autobuyInfo[client.GetValue()];
        if (autobuyType == L"info")
        {
            (void)client.Message(std::format(L"Ammo: {}", ammo ? L"On" : L"Off"));
            (void)client.Message(std::format(L"Mines: {}", mines ? L"On" : L"Off"));
            (void)client.Message(std::format(L"Countermeasures: {}", cm ? L"On" : L"Off"));
            (void)client.Message(std::format(L"Nanobots/Shield Batteries: {}", bb ? L"On" : L"Off"));
            (void)client.Message(std::format(L"Repairs: {}", repairs ? L"On" : L"Off"));
            co_return;
        }

        if (newState.empty() || (newState != L"on" && newState != L"off"))
        {
            (void)client.Message(L"ERR invalid parameters");
            co_return;
            ;
        }

        const bool enable = newState == L"on";
        if (autobuyType == L"all")
        {
            updated = true;
            ammo = enable;
            mines = enable;
            cm = enable;
            bb = enable;
            repairs = enable;
        }
        else if (autobuyType == L"ammo")
        {
            updated = true;
            ammo = enable;
        }
        else if (autobuyType == L"mines")
        {
            updated = true;
            mines = enable;
        }
        else if (autobuyType == L"cm")
        {
            updated = true;
            cm = enable;
        }
        else if (autobuyType == L"bb")
        {
            updated = true;
            bb = enable;
        }
        else if (autobuyType == L"repairs")
        {
            updated = true;
            repairs = enable;
        }
        else
        {
            (void)client.Message(L"ERR invalid parameters");
            co_return;
            ;
        }

        (void)client.Message(L"OK");

        co_return;
    }

    bool AutobuyPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/autobuy.json");

        INI_Reader ini;
        std::vector<std::string> iniPaths;
        if (!ini.open("freelancer.ini", false) || !ini.find_header("Data"))
        {
            ERROR(L"Unable to read freelancer.ini's Data header");
            return false;
        }

        while (ini.read_value())
        {
            if (!ini.is_value("equipment"))
            {
                continue;
            }
            iniPaths.push_back(std::string("../DATA/") + ini.get_value_string());
        }

        ini.close();

        // Get ammo limit
        for (const auto& iniPath : iniPaths)
        {
            if (!ini.open(iniPath.c_str(), false))
            {
                ERROR(L"Was unable to read ammo limits from the {0}", { L"file", StringUtils::stows(iniPath) });
                return false;
            }

            while (ini.read_header())
            {
                if (ini.is_header("Munition"))
                {
                    uint itemname = 0;
                    int itemlimit = 0;
                    bool valid = false;

                    while (ini.read_value())
                    {
                        if (ini.is_value("nickname"))
                        {
                            itemname = CreateID(ini.get_value_string(0));
                        }
                        else if (ini.is_value("ammo_limit"))
                        {
                            valid = true;
                            itemlimit = ini.get_value_int(0);
                        }
                    }

                    if (valid)
                    {
                        ammoLimits[Id(itemname)] = itemlimit;
                    }
                }
            }
        }

        return true;
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Autobuy",
	    .shortName = L"autobuy",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(AutobuyPlugin);
