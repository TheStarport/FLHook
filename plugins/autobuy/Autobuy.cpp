/**
 * @date unknown
 * @author unknown (Ported by Aingar 2023)
 * @defgroup Autobuy Autobuy
 * @brief
 * The "Autobuy" plugin allows players to set up automatic purchases of various munition/consumable type items.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - autobuy info - Lists status of autobuy features for this character.
 * - autobuy <all/munition type> <on/off> - enables or disables autobuy feature for selected munition types on this character.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "nanobot_nickname": "ge_s_repair_01";
 *     "shield_battery_nickname": "ge_s_battery_01";
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * None
 */

// Includes
#include "PCH.hpp"

#include "Autobuy.hpp"

namespace Plugins
{
    Autobuy::Autobuy(const PluginInfo& info) : Plugin(info), autobuyInfo() {}

    void Autobuy::LoadPlayerAutobuy(const ClientId client)
    {
        auto& [updated, missiles, mines, torps, cd, cm, bb, repairs] = autobuyInfo[client.GetValue()];

        const auto view = client.GetData().characterData;
        if (auto autobuyDoc = view.find("autobuy"); autobuyDoc != view.end())
        {
            for (const bsoncxx::document::view subDoc = autobuyDoc->get_document().view(); auto& item : subDoc)
            {
                switch (Hash(item.key().data()))
                {
                    case Hash("repairs"): repairs = item.get_bool(); break;
                    case Hash("bb"): bb = item.get_bool(); break;
                    case Hash("cm"): cm = item.get_bool(); break;
                    case Hash("cd"): cd = item.get_bool(); break;
                    case Hash("torps"): torps = item.get_bool(); break;
                    case Hash("mines"): mines = item.get_bool(); break;
                    case Hash("missiles"): missiles = item.get_bool(); break;
                    default:;
                }
            }
        }
    }

    void Autobuy::OnClearClientInfo(const ClientId client) { autobuyInfo[client.GetValue()] = {}; }

    int PlayerGetAmmoCount(const st6::list<EquipDesc>* cargoList, const uint itemArchId)
    {
        for (auto& cargo : *cargoList)
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
        auto repairCost = static_cast<uint>(client.GetShipArch().Unwrap()->hitPoints * (1 - client.GetRelativeHealth().Unwrap()) / 3);

        std::vector<ushort> eqToFix;

        for (const auto& item : *client.GetEquipCargo().Unwrap())
        {
            if (!item.mounted || item.health == 1)
            {
                continue;
            }

            const GoodInfo* info = GoodList_get()->find_by_archetype(item.archId);
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
            for (auto& item : equip)
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
            for (auto& eq : equip)
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

    void Autobuy::AddEquipToCart(const Archetype::Launcher* launcher, const st6::list<EquipDesc>* cargo, std::map<uint, AutobuyCartItem>& cart,
                                 AutobuyCartItem& item, const std::wstring_view& desc)
    {
        // TODO: Update to per-weapon ammo limits once implemented
        item.archId = launcher->projectileArchId;
        item.count = MAX_PLAYER_AMMO - PlayerGetAmmoCount(cargo, item.archId);
        item.description = desc;
        cart[item.archId] = item;
    }

    void Autobuy::OnCharacterSelectAfter(const ClientId client) { LoadPlayerAutobuy(client); }

    void Autobuy::OnCharacterSave(const ClientId client, std::wstring_view charName, bsoncxx::builder::basic::document& document)
    {
        using bsoncxx::builder::basic::kvp;
        const auto& [updated, missiles, mines, torps, cd, cm, bb, repairs] = autobuyInfo[client.GetValue()];

        if (!updated)
        {
            return;
        }

        document.append(kvp(
            "autobuy",
            bsoncxx::builder::basic::make_document(
                kvp("cd", cd), kvp("cm", cm), kvp("bb", bb), kvp("repairs", repairs), kvp("mines", mines), kvp("torps", torps), kvp("missiles", missiles))));
    }

    void Autobuy::OnBaseEnterAfter(const BaseId baseId, const ClientId client)
    {
        const Archetype::Ship* ship = client.GetShipArch().Unwrap();

        // player cargo
        float remHoldSize = client.GetRemainingCargo().Unwrap();
        auto equipCargo = client.GetEquipCargo().Unwrap();

        // shopping cart
        std::map<uint, AutobuyCartItem> cartMap;

        const auto& [updated, missiles, mines, torps, cd, cm, bb, repairs] = autobuyInfo[client.GetValue()];
        if (bb)
        {
            // shield bats & nanobots

            bool nanobotsFound = false;
            bool shieldBattsFound = false;
            for (auto& item : *equipCargo)
            {
                if (item.archId == config.nanobot.GetValue())
                {
                    AutobuyCartItem aci;
                    aci.archId = config.nanobot.GetValue();
                    aci.count = ship->maxNanobots - item.count;
                    aci.description = L"Nanobots";
                    cartMap[aci.archId] = aci;
                    nanobotsFound = true;
                }
                else if (item.archId == config.shieldBattery.GetValue())
                {
                    AutobuyCartItem aci;
                    aci.archId = config.shieldBattery.GetValue();
                    aci.count = ship->maxShieldBats - item.count;
                    aci.description = L"Shield Batteries";
                    cartMap[aci.archId] = aci;
                    shieldBattsFound = true;
                }
            }

            if (!nanobotsFound)
            { // no nanos found -> add all
                AutobuyCartItem aci;
                aci.archId = config.nanobot.GetValue();
                aci.count = ship->maxNanobots;
                aci.description = L"Nanobots";
                cartMap[aci.archId] = aci;
            }

            if (!shieldBattsFound)
            { // no batts found -> add all
                AutobuyCartItem aci;
                aci.archId = config.shieldBattery.GetValue();
                aci.count = ship->maxShieldBats;
                aci.description = L"Shield Batteries";
                cartMap[aci.archId] = aci;
            }
        }

        if (cd || cm || mines || missiles || torps)
        {
            // check mounted equip
            for (const auto& equip : *equipCargo)
            {
                if(!equip.mounted)
                {
                    continue;
                }
                AutobuyCartItem aci;

                switch (Archetype::Equipment* eq = Archetype::GetEquipment(equip.archId); EquipmentId(eq->archId).GetType().Unwrap())
                {
                    case EquipmentType::Mine:
                        {
                            if (mines)
                            {
                                AddEquipToCart(dynamic_cast<Archetype::Launcher*>(eq), equipCargo, cartMap, aci, L"Mines");
                            }

                            break;
                        }
                    case EquipmentType::Cm:
                        {
                            if (cm)
                            {
                                AddEquipToCart(dynamic_cast<Archetype::Launcher*>(eq), equipCargo, cartMap, aci, L"Countermeasures");
                            }

                            break;
                        }
                    case EquipmentType::Torpedo:
                        {
                            if (torps)
                            {
                                AddEquipToCart(dynamic_cast<Archetype::Launcher*>(eq), equipCargo, cartMap, aci, L"Torpedoes");
                            }

                            break;
                        }
                    case EquipmentType::Cd:
                        {
                            if (cd)
                            {
                                AddEquipToCart(dynamic_cast<Archetype::Launcher*>(eq), equipCargo, cartMap, aci, L"Cruise Disrupters");
                            }

                            break;
                        }
                    case EquipmentType::Missile:
                        {
                            if (missiles)
                            {
                                AddEquipToCart(dynamic_cast<Archetype::Launcher*>(eq), equipCargo, cartMap, aci, L"Missiles");
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
            if (!value.count || !Arch2Good(value.archId))
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

            const Archetype::Equipment* eq = Archetype::GetEquipment(value.archId);
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

    void Autobuy::UserCmdAutobuy(const std::wstring_view autobuyType, const std::wstring_view newState)
    {
        if (autobuyType.empty())
        {
            (void)userCmdClient.Message(L"Error: Invalid parameters");
            (void)userCmdClient.Message(L"Usage: /autobuy <param> [<on/off>]");
            (void)userCmdClient.Message(L"<Param>:");
            (void)userCmdClient.Message(L"|  info - display current autobuy-settings");
            (void)userCmdClient.Message(L"|  missiles - enable/disable autobuy for missiles");
            (void)userCmdClient.Message(L"|  torps - enable/disable autobuy for torpedos");
            (void)userCmdClient.Message(L"|  mines - enable/disable autobuy for mines");
            (void)userCmdClient.Message(L"|  cd - enable/disable autobuy for cruise disruptors");
            (void)userCmdClient.Message(L"|  cm - enable/disable autobuy for countermeasures");
            (void)userCmdClient.Message(L"|  bb - enable/disable autobuy for nanobots/shield batteries");
            (void)userCmdClient.Message(L"|  repairs - enable/disable automatic repair of ship and equipment");
            (void)userCmdClient.Message(L"|  all: enable/disable autobuy for all of the above");
            (void)userCmdClient.Message(L"Examples:");
            (void)userCmdClient.Message(L"|  \"/autobuy missiles on\" enable autobuy for missiles");
            (void)userCmdClient.Message(L"|  \"/autobuy all off\" completely disable autobuy");
            (void)userCmdClient.Message(L"|  \"/autobuy info\" show autobuy info");
        }

        auto& [updated, missiles, mines, torps, cd, cm, bb, repairs] = autobuyInfo[userCmdClient.GetValue()];
        if (autobuyType == L"info")
        {
            (void)userCmdClient.Message(std::format(L"Missiles: {}", missiles ? L"On" : L"Off"));
            (void)userCmdClient.Message(std::format(L"Mines: {}", mines ? L"On" : L"Off"));
            (void)userCmdClient.Message(std::format(L"Torpedos: {}", torps ? L"On" : L"Off"));
            (void)userCmdClient.Message(std::format(L"Cruise Disruptors: {}", cd ? L"On" : L"Off"));
            (void)userCmdClient.Message(std::format(L"Countermeasures: {}", cm ? L"On" : L"Off"));
            (void)userCmdClient.Message(std::format(L"Nanobots/Shield Batteries: {}", bb ? L"On" : L"Off"));
            (void)userCmdClient.Message(std::format(L"Repairs: {}", repairs ? L"On" : L"Off"));
            return;
        }

        if (newState.empty() || (newState != L"on" && newState != L"off"))
        {
            (void)userCmdClient.Message(L"ERR invalid parameters");
            return;
        }

        const bool enable = newState == L"on";
        if (autobuyType == L"all")
        {
            updated = true;
            missiles = enable;
            mines = enable;
            torps = enable;
            cd = enable;
            cm = enable;
            bb = enable;
            repairs = enable;
        }
        else if (autobuyType == L"missiles")
        {
            updated = true;
            missiles = enable;
        }
        else if (autobuyType == L"mines")
        {
            updated = true;
            mines = enable;
        }
        else if (autobuyType == L"torps")
        {
            updated = true;
            torps = enable;
        }
        else if (autobuyType == L"cd")
        {
            updated = true;
            cd = enable;
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
            (void)userCmdClient.Message(L"ERR invalid parameters");
            return;
        }

        (void)userCmdClient.Message(L"OK");
    }

    void Autobuy::OnLoadSettings()
    {
        if (const auto conf = Json::Load<Config>("config/autobuy.json"); !conf.has_value())
        {
            Json::Save(config, "config/autobuy.json");
        }
        else
        {
            config = conf.value();
        }
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Autobuy", L"autobuy", PluginMajorVersion::V05, PluginMinorVersion::V01);
SetupPlugin(Autobuy, Info);
