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

#include "API/API.hpp"
#include "Autobuy.hpp"
#include "FLHook.hpp"

namespace Plugins
{



    void Autobuy::LoadPlayerAutobuy(ClientId client)
    {

        //TODO: Implement chardata database retrieval for this as IniUtils is being deprecated for 4.1
        AutobuyInfo playerAutobuyInfo{};
        /*
        playerAutobuyInfo.missiles = StringUtils::Cast<bool>(Hk::IniUtils::c()->GetFromPlayerFile(client, L"autobuy.missiles").value());
        playerAutobuyInfo.torps = StringUtils::Cast<bool>(Hk::IniUtils::c()->GetFromPlayerFile(client, L"autobuy.torps").value());
        playerAutobuyInfo.cd = StringUtils::Cast<bool>(Hk::IniUtils::c()->GetFromPlayerFile(client, L"autobuy.cd").value());
        playerAutobuyInfo.cm = StringUtils::Cast<bool>(Hk::IniUtils::c()->GetFromPlayerFile(client, L"autobuy.cm").value());
        playerAutobuyInfo.bb = StringUtils::Cast<bool>(Hk::IniUtils::c()->GetFromPlayerFile(client, L"autobuy.bb").value());
        playerAutobuyInfo.repairs = StringUtils::Cast<bool>(Hk::IniUtils::c()->GetFromPlayerFile(client, L"autobuy.repairs").value());
        autobuyInfo[client] = playerAutobuyInfo;
        */
    }

    void Autobuy::ClearClientInfo(ClientId& client) { autobuyInfo.erase(client); }

    int PlayerGetAmmoCount(const std::list<CargoInfo>& cargoList, uint itemArchId)
    {
        if (const auto foundCargo = std::ranges::find_if(cargoList, [itemArchId](const CargoInfo& cargo) { return cargo.archId == itemArchId; });
            foundCargo != cargoList.end())
        {
            return foundCargo->count;
        }

        return 0;
    }

    void handleRepairs(ClientId& client)
    {
        auto repairCost = static_cast<uint>(Archetype::GetShip(Players[client].shipArchetype)->hitPoints * (1 - Players[client].relativeHealth) / 3);

        std::set<short> eqToFix;

        for (const auto& item : Players[client].equipDescList.equip)
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
            eqToFix.insert(item.id);
        }

        if (const uint playerCash = Hk::Player::GetCash(client).Unwrap(); playerCash < repairCost)
        {
            client.Message(L"Insufficient Cash");
            return;
        }

        if (repairCost)
        {
            client.Message(std::format(L"Auto-Buy: Ship repair costed {}$", repairCost));
            Hk::Player::RemoveCash(client, repairCost);
        }

        if (!eqToFix.empty())
        {
            for (auto& item : Players[client].equipDescList.equip)
            {
                if (eqToFix.contains(item.id))
                {
                    item.health = 1.0f;
                }
            }

            auto& equip = Players[client].equipDescList.equip;

            if (&equip != &Players[client].shadowEquipDescList.equip)
            {
                Players[client].shadowEquipDescList.equip = equip;
            }

            st6::vector<EquipDesc> eqVector;
            for (auto& eq : equip)
            {
                if (eq.mounted)
                {
                    eq.health = 1.0f;
                }
                eqVector.push_back(eq);
            }

            HookClient->Send_FLPACKET_SERVER_SETEQUIPMENT(client, eqVector);
        }

        if (auto& playerCollision = Players[client].collisionGroupDesc; !playerCollision.empty())
        {
            st6::list<XCollision> componentList;
            for (auto& colGrp : playerCollision)
            {
                auto* newColGrp = reinterpret_cast<XCollision*>(colGrp);
                newColGrp->componentHp = 1.0f;
                componentList.push_back(*newColGrp);
            }
            client.Message(std::format(L"Attempting to repair {} components.", playerCollision.size()));
            HookClient->Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(client, componentList);
        }

        if (Players[client].relativeHealth < 1.0f)
        {
            Players[client].relativeHealth = 1.0f;
            HookClient->Send_FLPACKET_SERVER_SETHULLSTATUS(client, 1.0f);
        }
    }

    void Autobuy::AddEquipToCart(const Archetype::Launcher* launcher, const std::list<CargoInfo>& cargo, std::list<AutobuyCartItem>& cart, AutobuyCartItem& item,
                        const std::wstring_view& desc)
    {
        // TODO: Update to per-weapon ammo limits once implemented
        item.archId = launcher->projectileArchId;
        item.count = MAX_PLAYER_AMMO - PlayerGetAmmoCount(cargo, item.archId);
        item.description = desc;
        cart.emplace_back(item);
    }

    AutobuyInfo& Autobuy::LoadAutobuyInfo(ClientId& client)
    {
        if (!autobuyInfo.contains(client))
        {
            LoadPlayerAutobuy(client);
        }

        return autobuyInfo[client];
    }

    void Autobuy::OnBaseEnter(BaseId& baseId, ClientId& client)
    {
        const AutobuyInfo& clientInfo = LoadAutobuyInfo(client);

        const Archetype::Ship* ship = Archetype::GetShip(Players[client].shipArchetype);

        // player cargo
        int remHoldSize;
        const auto cargo = Hk::Player::EnumCargo(client, remHoldSize).Handle();

        // shopping cart
        std::list<AutobuyCartItem> cartList;

        if (clientInfo.bb)
        {
            // shield bats & nanobots

            uint nanobotsId;
            pub::GetGoodID(nanobotsId, config->nanobotNickname.c_str());
            uint shieldBatsId;
            pub::GetGoodID(shieldBatsId, config->shieldBatteryNickname.c_str());
            bool nanobotsFound = false;
            bool shieldBattsFound = false;
            for (auto& item : cargo)
            {
                AutobuyCartItem aci;
                if (item.archId == nanobotsId)
                {
                    aci.archId = nanobotsId;
                    aci.count = ship->maxNanobots - item.count;
                    aci.description = L"Nanobots";
                    cartList.push_back(aci);
                    nanobotsFound = true;
                }
                else if (item.archId == shieldBatsId)
                {
                    aci.archId = shieldBatsId;
                    aci.count = ship->maxShieldBats - item.count;
                    aci.description = L"Shield Batteries";
                    cartList.push_back(aci);
                    shieldBattsFound = true;
                }
            }

            if (!nanobotsFound)
            { // no nanos found -> add all
                AutobuyCartItem aci;
                aci.archId = nanobotsId;
                aci.count = ship->maxNanobots;
                aci.description = L"Nanobots";
                cartList.push_back(aci);
            }

            if (!shieldBattsFound)
            { // no batts found -> add all
                AutobuyCartItem aci;
                aci.archId = shieldBatsId;
                aci.count = ship->maxShieldBats;
                aci.description = L"Shield Batteries";
                cartList.push_back(aci);
            }
        }

        if (clientInfo.cd || clientInfo.cm || clientInfo.mines || clientInfo.missiles || clientInfo.torps)
        {
            // add mounted equip to a new list and eliminate double equipment(such
            // as 2x lancer etc)
            std::list<CargoInfo> mountedList;
            for (auto& item : cargo)
            {
                if (!item.mounted)
                {
                    continue;
                }

                bool found = false;
                for (const auto& mounted : mountedList)
                {
                    if (mounted.archId == item.archId)
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    mountedList.push_back(item);
                }
            }

            // check mounted equip
            for (const auto& mounted : mountedList)
            {
                AutobuyCartItem aci;
                Archetype::Equipment* eq = Archetype::GetEquipment(mounted.archId);
                auto eqType = Hk::Client::GetEqType(eq);

                switch (eqType)
                {
                    case ET_MINE:
                        {
                            if (clientInfo.mines)
                            {
                                AddEquipToCart(static_cast<Archetype::Launcher*>(eq), cargo, cartList, aci, L"Mines");
                            }

                            break;
                        }
                    case ET_CM:
                        {
                            if (clientInfo.cm)
                            {
                                AddEquipToCart(static_cast<Archetype::Launcher*>(eq), cargo, cartList, aci, L"Countermeasures");
                            }

                            break;
                        }
                    case ET_TORPEDO:
                        {
                            if (clientInfo.torps)
                            {
                                AddEquipToCart(static_cast<Archetype::Launcher*>(eq), cargo, cartList, aci, L"Torpedoes");
                            }

                            break;
                        }
                    case ET_CD:
                        {
                            if (clientInfo.cd)
                            {
                                AddEquipToCart(static_cast<Archetype::Launcher*>(eq), cargo, cartList, aci, L"Cruise Disrupters");
                            }

                            break;
                        }
                    case ET_MISSILE:
                        {
                            if (clientInfo.missiles)
                            {
                                AddEquipToCart(static_cast<Archetype::Launcher*>(eq), cargo, cartList, aci, L"Missiles");
                            }

                            break;
                        }

                    default: break;
                }
            }
        }

        if (clientInfo.repairs)
        {
            handleRepairs(client);
        }

        // search base in base-info list
        const BaseInfo* bi = nullptr;

        if (auto foundBase = std::ranges::find_if(CoreGlobals::c()->allBases, [baseId](const BaseInfo& base) { return base.baseId == baseId; });
            foundBase != CoreGlobals::c()->allBases.end())
        {
            bi = std::to_address(foundBase);
        }

        if (!bi)
        {
            return; // base not found
        }

        const auto cash= Hk::Player::GetCash(client).Handle();

        for (auto& buy : cartList)
        {
            if (!buy.count || !Arch2Good(buy.archId))
            {
                continue;
            }

            // check if good is available and if player has the neccessary rep
            bool goodAvailable = false;
            for (const auto& available : bi->MarketMisc)
            {
                if (available.archId == buy.archId)
                {
                    auto baseRep = Hk::Solar::GetAffiliation(bi->objectId).Handle();
                    const auto playerRep = Hk::Player::GetRep(client, baseRep).Handle();

                    // good rep, allowed to buy
                    if (playerRep >= available.rep)
                    {
                        goodAvailable = true;
                    }
                    break;
                }
            }

            if (!goodAvailable)
            {
                continue; // base does not sell this item or bad rep
            }
            auto goodPrice = Hk::Solar::GetCommodityPrice(baseId, buy.archId).Raw();
            if (goodPrice.has_error())
            {
                continue; // good not available
            }

            const Archetype::Equipment* eq = Archetype::GetEquipment(buy.archId);
            // will always fail for volume == 0, no need to worry about potential div by 0
            if (static_cast<float>(remHoldSize) < std::ceil(eq->volume * static_cast<float>(buy.count)))
            {
                // round to the nearest possible
                auto newCount = static_cast<uint>(static_cast<float>(remHoldSize) / eq->volume);
                if (!newCount)
                {
                    client.Message(std::format(L"Auto-Buy({}): FAILED! Insufficient Cargo Space", buy.description));
                    continue;
                }
                else
                {
                    buy.count = newCount;
                }
            }

            if (uint uCost = (static_cast<uint>(goodPrice.value()) * buy.count); cash < uCost)
            {
                client.Message(std::format(L"Auto-Buy({}): FAILED! Insufficient Credits", buy.description));
            }
            else
            {
                Hk::Player::RemoveCash(client, uCost);
                remHoldSize -= ((int)eq->volume * buy.count);

                // add the item, dont use addcargo for performance/bug reasons
                // assume we only mount multicount goods (missiles, ammo, bots
                Hk::Player::AddCargo(client, buy.archId, buy.count, false);

                client.Message(std::format(L"Auto-Buy({}): Bought {} unit(s), cost: {}$", buy.description, buy.count, StringUtils::ToMoneyStr(uCost)));
            }
        }
        Hk::Player::SaveChar(client);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // USER COMMANDS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////



    void Autobuy::UserCmdAutobuy(std::wstring_view autobuyType, std::wstring_view newState)
    {
        AutobuyInfo& autobuyInfo = LoadAutobuyInfo(client);
        if (autobuyType.empty())
        {
            client.Message(L"Error: Invalid parameters");
            client.Message(L"Usage: /autobuy <param> [<on/off>]");
            client.Message(L"<Param>:");
            client.Message(L"|  info - display current autobuy-settings");
            client.Message(L"|  missiles - enable/disable autobuy for missiles");
            client.Message(L"|  torps - enable/disable autobuy for torpedos");
            client.Message(L"|  mines - enable/disable autobuy for mines");
            client.Message(L"|  cd - enable/disable autobuy for cruise disruptors");
            client.Message(L"|  cm - enable/disable autobuy for countermeasures");
            client.Message(L"|  bb - enable/disable autobuy for nanobots/shield batteries");
            client.Message(L"|  repairs - enable/disable automatic repair of ship and equipment");
            client.Message(L"|  all: enable/disable autobuy for all of the above");
            client.Message(L"Examples:");
            client.Message(L"|  \"/autobuy missiles on\" enable autobuy for missiles");
            client.Message(L"|  \"/autobuy all off\" completely disable autobuy");
            client.Message(L"|  \"/autobuy info\" show autobuy info");
        }

        if (autobuyType == L"info")
        {
            client.Message(std::format(L"Missiles: {}", autobuyInfo.missiles ? L"On" : L"Off"));
            client.Message(std::format(L"Mines: {}", autobuyInfo.mines ? L"On" : L"Off"));
            client.Message(std::format(L"Torpedos: {}", autobuyInfo.torps ? L"On" : L"Off"));
            client.Message(std::format(L"Cruise Disruptors: {}", autobuyInfo.cd ? L"On" : L"Off"));
            client.Message(std::format(L"Countermeasures: {}", autobuyInfo.cm ? L"On" : L"Off"));
            client.Message(std::format(L"Nanobots/Shield Batteries: {}", autobuyInfo.bb ? L"On" : L"Off"));
            client.Message(std::format(L"Repairs: {}", autobuyInfo.repairs ? L"On" : L"Off"));
            return;
        }

        if (newState.empty() || (newState != L"on" && newState != L"off"))
        {
            client.Message(L"ERR invalid parameters");
            return;
        }

        const auto fileName = Hk::Client::GetCharFileName(client).Handle();
        std::string Section = "autobuy_" + StringUtils::wstos(fileName);

        const bool enable = newState == L"on";
        if (autobuyType == L"all")
        {
            autobuyInfo.missiles = enable;
            autobuyInfo.mines = enable;
            autobuyInfo.torps = enable;
            autobuyInfo.cd = enable;
            autobuyInfo.cm = enable;
            autobuyInfo.bb = enable;
            autobuyInfo.repairs = enable;
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.missiles", StringUtils::stows(enable ? "true" : "false"));
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.mines", StringUtils::stows(enable ? "true" : "false"));
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.torps", StringUtils::stows(enable ? "true" : "false"));
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.cd", StringUtils::stows(enable ? "true" : "false"));
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.cm", StringUtils::stows(enable ? "true" : "false"));
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.bb", StringUtils::stows(enable ? "true" : "false"));
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.repairs", StringUtils::stows(enable ? "true" : "false"));
        }
        else if (autobuyType == L"missiles")
        {
            autobuyInfo.missiles = enable;
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.missiles", StringUtils::stows(enable ? "true" : "false"));
        }
        else if (autobuyType == L"mines")
        {
            autobuyInfo.mines = enable;
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.mines", StringUtils::stows(enable ? "true" : "false"));
        }
        else if (autobuyType == L"torps")
        {
            autobuyInfo.torps = enable;
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.torps", StringUtils::stows(enable ? "true" : "false"));
        }
        else if (autobuyType == L"cd")
        {
            autobuyInfo.cd = enable;
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.cd", StringUtils::stows(enable ? "true" : "false"));
        }
        else if (autobuyType == L"cm")
        {
            autobuyInfo.cm = enable;
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.cm", StringUtils::stows(enable ? "true" : "false"));
        }
        else if (autobuyType == L"bb")
        {
            autobuyInfo.bb = enable;
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.bb", StringUtils::stows(enable ? "true" : "false"));
        }
        else if (autobuyType == L"repairs")
        {
            autobuyInfo.repairs = enable;
            Hk::IniUtils::c()->SetCharacterIni(client, L"autobuy.repairs", StringUtils::stows(enable ? "true" : "false"));
        }
        else
        {
            client.Message(L"ERR invalid parameters");
            return;
        }

        Hk::Player::SaveChar(client);
        client.Message(L"OK");
    }


    using namespace Plugins;

    DefaultDllMain();

    const PluginInfo Info(L"Autobuy", L"autobuy", PluginMajorVersion::V04, PluginMinorVersion::V01);
    void Autobuy::LoadSettings() { config = Serializer::LoadFromJson<Config>(L"config/autobuy.json"); }
    Autobuy::Autobuy(const PluginInfo info) : Plugin(Info)
    {
        EmplaceHook(HookedCall::FLHook__LoadSettings, &Autobuy::LoadSettings, HookStep::After);
        EmplaceHook(HookedCall::FLHook__ClearClientInfo, &Autobuy::ClearClientInfo, HookStep::After);
        EmplaceHook(HookedCall::IServerImpl__BaseEnter, &Autobuy::OnBaseEnter, HookStep::After);
    }
    SetupPlugin(Autobuy, Info);

} // namespace Plugins
