/**
 Base Plugin for FLHook-Plugin
 by Cannon.

0.1:
 Initial release
*/

// includes

#include "Main.h"
#include <FLHook.h>
#include <algorithm>
#include <list>
#include <map>
#include <math.h>
#include <plugin.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

#include <plugin_comms.h>

// Clients
map<uint, CLIENT_DATA> clients;

// Bases
map<uint, PlayerBase *> player_bases;

/// The debug mode
int set_plugin_debug = 0;

/// The ship used to construct and upgrade bases
uint set_construction_shiparch = 0;

/// Map of good to quantity for items required by construction ship
map<uint, uint> construction_items;

/// list of items and quantity used to repair 10000 units of damage
list<REPAIR_ITEM> set_base_repair_items;

/// list of items used by human crew
map<uint, uint> set_base_crew_consumption_items;
map<uint, uint> set_base_crew_food_items;

/// The commodity used as crew for the base
uint set_base_crew_type;

/// A return code to indicate to FLHook if we want the hook processing to
/// continue.
PLUGIN_RETURNCODE returncode;

/// Map of item nickname hash to recipes to construct item.
map<uint, RECIPE> recipes;

/// Map of item nickname hash to recipes to operate shield.
map<uint, uint> shield_power_items;

/// Map of space obj IDs to base modules to speed up damage algorithms.
map<uint, Module *> spaceobj_modules;

/// Path to shield status html page
std::string set_status_path_html;

/// Damage to the base every tick
uint set_damage_per_tick = 600;

// The seconds per tick
uint set_tick_time = 16;

/// If the shield is up then damage to the base is changed by this multiplier.
float set_shield_damage_multiplier = 0.01f;

/// If true, use the new solar based defense platform spawn
bool set_new_spawn = false;

/// True if the settings should be reloaded
bool load_settings_required = true;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

PlayerBase *GetPlayerBase(uint base) {
    map<uint, PlayerBase *>::iterator i = player_bases.find(base);
    if (i != player_bases.end())
        return i->second;
    return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

PlayerBase *GetPlayerBaseForClient(uint client) {
    map<uint, CLIENT_DATA>::iterator j = clients.find(client);
    if (j == clients.end())
        return 0;

    map<uint, PlayerBase *>::iterator i =
        player_bases.find(j->second.player_base);
    if (i == player_bases.end())
        return 0;

    return i->second;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// For the specified client setup the reputation to any bases in the
// client's system.
void SyncReputationForClientShip(uint ship, uint client) {
    int player_rep;
    pub::SpaceObj::GetRep(ship, player_rep);

    uint system;
    pub::SpaceObj::GetSystem(ship, system);

    map<uint, PlayerBase *>::iterator base = player_bases.begin();
    for (; base != player_bases.end(); base++) {
        if (base->second->system == system) {
            float attitude = base->second->GetAttitudeTowardsClient(client);
            if (set_plugin_debug > 1)
                ConPrint(L"SyncReputationForClientShip:: ship=%u attitude=%f "
                         L"base=%08x\n",
                         ship, attitude, base->first);
            for (vector<Module *>::iterator module =
                     base->second->modules.begin();
                 module != base->second->modules.end(); ++module) {
                if (*module) {
                    (*module)->SetReputation(player_rep, attitude);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// HTML-encodes a std::string and returns the encoded std::string.
std::wstring HtmlEncode(std::wstring text) {
    std::wstring sb;
    int len = text.size();
    for (int i = 0; i < len; i++) {
        switch (text[i]) {
        case L'<':
            sb.append(L"&lt;");
            break;
        case L'>':
            sb.append(L"&gt;");
            break;
        case L'"':
            sb.append(L"&quot;");
            break;
        case L'&':
            sb.append(L"&amp;");
            break;
        default:
            if (text[i] > 159) {
                sb.append(L"&#");
                sb.append(std::to_wstring(text[i]));
                sb.append(L";");
            } else {
                sb.append(1, text[i]);
            }
            break;
        }
    }
    return sb;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Clear client info when a client connects.
void ClearClientInfo(uint client) { clients.erase(client); }

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void LoadSettings() {
    returncode = DEFAULT_RETURNCODE;
    load_settings_required = true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/// Load the configuration
void LoadSettingsActual() {
    returncode = DEFAULT_RETURNCODE;

    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string cfg_file = std::string(szCurDir) + "\\flhook_plugins\\base.cfg";

    map<uint, PlayerBase *>::iterator base = player_bases.begin();
    for (; base != player_bases.end(); base++) {
        delete base->second;
    }

    recipes.clear();
    construction_items.clear();
    set_base_repair_items.clear();
    set_base_crew_consumption_items.clear();
    set_base_crew_food_items.clear();
    shield_power_items.clear();

    INI_Reader ini;
    if (ini.open(cfg_file.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("recipe")) {
                RECIPE recipe;
                while (ini.read_value()) {
                    if (ini.is_value("nickname")) {
                        recipe.nickname = CreateID(ini.get_value_string(0));
                    } else if (ini.is_value("produced_item")) {
                        recipe.produced_item =
                            CreateID(ini.get_value_string(0));
                    } else if (ini.is_value("infotext")) {
                        recipe.infotext = stows(ini.get_value_string());
                    } else if (ini.is_value("cooking_rate")) {
                        recipe.cooking_rate = ini.get_value_int(0);
                    } else if (ini.is_value("consumed")) {
                        recipe
                            .consumed_items[CreateID(ini.get_value_string(0))] =
                            ini.get_value_int(1);
                    }
                }
                recipes[recipe.nickname] = recipe;
            } else if (ini.is_header("general")) {
                while (ini.read_value()) {
                    if (ini.is_value("debug")) {
                        set_plugin_debug = ini.get_value_int(0);
                    }
                    if (ini.is_value("status_path_html")) {
                        set_status_path_html = ini.get_value_string();
                    } else if (ini.is_value("damage_per_tick")) {
                        set_damage_per_tick = ini.get_value_int(0);
                    } else if (ini.is_value("tick_time")) {
                        set_tick_time = ini.get_value_int(0);
                    } else if (ini.is_value("shield_damage_multiplier")) {
                        set_shield_damage_multiplier = ini.get_value_float(0);
                    } else if (ini.is_value("construction_shiparch")) {
                        set_construction_shiparch =
                            CreateID(ini.get_value_string(0));
                    } else if (ini.is_value("construction_item")) {
                        uint good = CreateID(ini.get_value_string(0));
                        uint quantity = ini.get_value_int(1);
                        construction_items[good] = quantity;
                    } else if (ini.is_value("base_crew_item")) {
                        set_base_crew_type = CreateID(ini.get_value_string(0));
                    } else if (ini.is_value("base_repair_item")) {
                        REPAIR_ITEM item;
                        item.good = CreateID(ini.get_value_string(0));
                        item.quantity = ini.get_value_int(1);
                        set_base_repair_items.push_back(item);
                    } else if (ini.is_value("base_crew_consumption_item")) {
                        uint good = CreateID(ini.get_value_string(0));
                        uint quantity = ini.get_value_int(1);
                        set_base_crew_consumption_items[good] = quantity;
                    } else if (ini.is_value("base_crew_food_item")) {
                        uint good = CreateID(ini.get_value_string(0));
                        uint quantity = ini.get_value_int(1);
                        set_base_crew_food_items[good] = quantity;
                    } else if (ini.is_value("shield_power_item")) {
                        uint good = CreateID(ini.get_value_string(0));
                        uint quantity = ini.get_value_int(1);
                        shield_power_items[good] = quantity;
                    } else if (ini.is_value("set_new_spawn")) {
                        set_new_spawn = true;
                    }
                }
            }
        }
        ini.close();
    }

    char datapath[MAX_PATH];
    GetUserDataPath(datapath);

    // Create base account dir if it doesn't exist
    std::string basedir =
        std::string(datapath) + "\\Accts\\MultiPlayer\\player_bases\\";
    CreateDirectoryA(basedir.c_str(), 0);

    // Load and spawn all bases
    std::string path = std::string(datapath) +
                       "\\Accts\\MultiPlayer\\player_bases\\base_*.ini";

    WIN32_FIND_DATA findfile;
    HANDLE h = FindFirstFile(path.c_str(), &findfile);
    if (h != INVALID_HANDLE_VALUE) {
        do {
            std::string filepath = std::string(datapath) +
                                   "\\Accts\\MultiPlayer\\player_bases\\" +
                                   findfile.cFileName;
            PlayerBase *base = new PlayerBase(filepath);
            player_bases[base->base] = base;
            base->Spawn();
        } while (FindNextFile(h, &findfile));
        FindClose(h);
    }

    // Load and sync player state
    struct PlayerData *pd = 0;
    while (pd = Players.traverse_active(pd)) {
        uint client = pd->iOnlineID;
        if (HkIsInCharSelectMenu(client))
            continue;

        // If this player is in space, set the reputations.
        if (pd->iShipID)
            SyncReputationForClientShip(pd->iShipID, client);

        // Get state if player is in player base and  reset the commodity list
        // and send a dummy entry if there are no commodities in the market
        LoadDockState(client);
        if (clients[client].player_base) {
            PlayerBase *base = GetPlayerBaseForClient(client);
            if (base) {
                // Reset the commodity list	and send a dummy entry if there are
                // no commodities in the market
                SaveDockState(client);
                SendMarketGoodSync(base, client);
                SendBaseStatus(client, base);
            } else {
                // Force the ship to launch to space as the base has been
                // destroyed
                DeleteDockState(client);
                SendResetMarketOverride(client);
                ForceLaunch(client);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

void HkTimerCheckKick() {
    returncode = DEFAULT_RETURNCODE;

    if (load_settings_required) {
        load_settings_required = false;
        LoadSettingsActual();
    }

    uint curr_time = (uint)time(0);
    map<uint, PlayerBase *>::iterator iter = player_bases.begin();
    while (iter != player_bases.end()) {
        PlayerBase *base = iter->second;
        // Advance to next base in case base is deleted in timer dispatcher
        ++iter;
        // Dispatch timer but we can safely ignore the return
        base->Timer(curr_time);
    }

    // Write status to a json formatted page every 13 seconds
    if ((curr_time % 13) == 0 && set_status_path_html.size() > 0) {
        FILE *file;
        fopen_s(&file, set_status_path_html.c_str(), "w");
        if (file) {
            fprintf(file,
                    "<html>\n<head><title>Player Base Status</title><style "
                    "type=text/css>\n");
            fprintf(
                file,
                ".ColumnH {FONT-FAMILY: Tahoma; FONT-SIZE: 10pt;  TEXT-ALIGN: "
                "left; COLOR: #000000; BACKGROUND: #ECE9D8;}\n");
            fprintf(
                file,
                ".Column0 {FONT-FAMILY: Tahoma; FONT-SIZE: 10pt;  TEXT-ALIGN: "
                "left; COLOR: #000000; BACKGROUND: #FFFFFF;}\n");
            fprintf(file, "</style></head><body>\n\n");

            fprintf(file,
                    "<table width=\"90%%\" border=\"1\" cellspacing=\"0\" "
                    "cellpadding=\"2\">\n");

            fprintf(file, "<tr>");
            fprintf(file, "<th class=\"ColumnH\">Base Name</th>");
            fprintf(file, "<th class=\"ColumnH\">Health (%%)</th>");
            fprintf(file, "<th class=\"ColumnH\">Shield Status</th>");
            fprintf(file, "<th class=\"ColumnH\">Money</th>");
            fprintf(file, "<th class=\"ColumnH\">Description</th>");
            fprintf(file, "</tr>\n\n");

            map<uint, PlayerBase *>::iterator iter = player_bases.begin();
            while (iter != player_bases.end()) {
                PlayerBase *base = iter->second;
                fprintf(file, "<tr>");
                fprintf(file, "<td class=\"column0\">%s</td>",
                        wstos(HtmlEncode(base->basename)).c_str());
                fprintf(file, "<td class=\"column0\">%0.0f</td>",
                        100 * (base->base_health / base->max_base_health));
                fprintf(file, "<td class=\"column0\">%s</td>",
                        base->shield_state == PlayerBase::SHIELD_STATE_ACTIVE
                            ? "On"
                            : "Off");
                fprintf(file, "<td class=\"column0\">%I64d</td>", base->money);

                std::string desc;
                for (int i = 1; i <= MAX_PARAGRAPHS; i++) {
                    desc += "<p>";
                    desc += wstos(HtmlEncode(base->infocard_para[i]));
                    desc += "</p>";
                }
                fprintf(file, "<td class=\"column0\">%s</td>", desc.c_str());
                fprintf(file, "</tr>\n");
                ++iter;
            }

            fprintf(file, "</table>\n\n</body><html>\n");
            fclose(file);
        }
    }
}

bool __stdcall HkCb_IsDockableError(uint dock_with, uint base) {
    if (GetPlayerBase(base))
        return false;
    ConPrint(L"ERROR: Base not found dock_with=%08x base=%08x\n", base, base);
    return true;
}

__declspec(naked) void HkCb_IsDockableErrorNaked() {
    __asm {
        test [esi+0x1b4], eax
        jnz no_error
        push [edi+0xB8]
        push [esi+0x1b4]
        call HkCb_IsDockableError
        test al, al
        jz no_error
        push 0x62b76d3
        ret
no_error:
        push 0x62b76fc
        ret
    }
}

bool __stdcall HkCb_Land(IObjInspectImpl *obj, uint base_dock_id, uint base) {
    if (obj) {
        uint client = HkGetClientIDByShip(obj->get_id());
        if (client) {
            if (set_plugin_debug > 1)
                ConPrint(L"Land client=%u base_dock_id=%u base=%u\n", client,
                         base_dock_id, base);

            // If we're docking at a player base, do nothing.
            if (clients[client].player_base)
                return true;

            // If we're not docking at a player base then clear
            // the last base flag
            clients[client].last_player_base = 0;
            clients[client].player_base = 0;
            if (base == 0) {
                char szSystem[1024];
                pub::GetSystemNickname(szSystem, sizeof(szSystem),
                                       Players[client].iSystemID);

                char szProxyBase[1024];
                sprintf_s(szProxyBase, "%s_proxy_base", szSystem);

                uint iProxyBaseID = CreateID(szProxyBase);

                clients[client].player_base = base_dock_id;
                clients[client].last_player_base = base_dock_id;
                if (set_plugin_debug > 1)
                    ConPrint(L"Land[2] client=%u baseDockID=%u base=%u "
                             L"player_base=%u\n",
                             client, base_dock_id, base,
                             clients[client].player_base);
                pub::Player::ForceLand(client, iProxyBaseID);
                return false;
            }
        }
    }
    return true;
}

__declspec(naked) void HkCb_LandNaked() {
    __asm {
        mov al, [ebx+0x1c]
        test al, al
        jz not_in_dock

        mov eax, [ebx+0x18] // base id
        push eax
        mov eax, [esp+0x14] // dock target
        push eax
        push edi    // objinspect
        call HkCb_Land
        test al, al
        jz done

not_in_dock:
                             // Copied from moor.dll to support mooring.
        mov	al, [ebx+0x1c]
        test	al, al
        jnz	done
            // It's false, so a safe bet that it's a moor.  Is it the player?
        mov	eax, [edi]
        mov	ecx, edi
        call	[eax+0xbc] // is_player
        test	al, al
        jnz done
done:
        push 0x6D0C251
        ret
    }
}

static bool patched = false;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    srand((uint)time(0));
    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (!patched) {
            patched = true;

            hModServer = GetModuleHandleA("server.dll");
            {

                // Call our function on landing
                uchar patch[] = {0xe9}; // jmpr
                WriteProcMem((char *)hModServer + 0x2c24c, patch,
                             sizeof(patch));
                PatchCallAddr((char *)hModServer, 0x2c24c,
                              (char *)HkCb_LandNaked);
            }

            hModCommon = GetModuleHandleA("common.dll");
            {
                // Suppress "is dockable " error message
                uchar patch[] = {0xe9}; // jmpr
                WriteProcMem((char *)hModCommon + 0x576cb, patch,
                             sizeof(patch));
                PatchCallAddr((char *)hModCommon, 0x576cb,
                              (char *)HkCb_IsDockableErrorNaked);
            }

            {
                // Suppress GetArch() error on max hit points call
                uchar patch[] = {0x90, 0x90}; // nop nop
                WriteProcMem((char *)hModCommon + 0x995b6, patch,
                             sizeof(patch));
                WriteProcMem((char *)hModCommon + 0x995fc, patch,
                             sizeof(patch));
            }
        }

        HkLoadStringDLLs();
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        if (patched) {
            {
                // Unpatch the landing hook
                uchar patch[] = {0x8A, 0x43, 0x1C, 0x84, 0xC0};
                WriteProcMem((char *)hModServer + 0x2c24c, patch,
                             sizeof(patch));
            }

            {
                // Unpatch the Suppress "is dockable " error message
                uchar patch[] = {0x85, 0x86, 0xb4, 0x01, 0x00};
                WriteProcMem((char *)hModCommon + 0x576cb, patch,
                             sizeof(patch));
            }
        }

        map<uint, PlayerBase *>::iterator base = player_bases.begin();
        for (; base != player_bases.end(); base++) {
            delete base->second;
        }

        HkUnloadStringDLLs();
    }
    return true;
}

bool UserCmd_Process(uint client, const std::wstring &args) {
    returncode = DEFAULT_RETURNCODE;
    if (args.find(L"/base login") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseLogin(client, args);
        return true;
    } else if (args.find(L"/base addpwd") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseAddPwd(client, args);
        return true;
    } else if (args.find(L"/base rmpwd") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseRmPwd(client, args);
        return true;
    } else if (args.find(L"/base lstpwd") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseLstPwd(client, args);
        return true;
    } else if (args.find(L"/base setmasterpwd") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseSetMasterPwd(client, args);
        return true;
    } else if (args.find(L"/base addtag") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseAddAllyTag(client, args);
        return true;
    } else if (args.find(L"/base rmtag") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseRmAllyTag(client, args);
        return true;
    } else if (args.find(L"/base lsttag") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseLstAllyTag(client, args);
        return true;
    } else if (args.find(L"/base rep") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseRep(client, args);
        return true;
    } else if (args.find(L"/base defensemode") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseDefenseMode(client, args);
        return true;
    } else if (args.find(L"/base deploy") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseDeploy(client, args);
        return true;
    } else if (args.find(L"/shop") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::Shop(client, args);
        return true;
    } else if (args.find(L"/bank") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::Bank(client, args);
        return true;
    } else if (args.find(L"/base info") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseInfo(client, args);
        return true;
    } else if (args.find(L"/base facmod") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseFacMod(client, args);
        return true;
    } else if (args.find(L"/base defmod") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseDefMod(client, args);
        return true;
    } else if (args.find(L"/base shieldmod") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseShieldMod(client, args);
        return true;
    } else if (args.find(L"/base buildmod") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseBuildMod(client, args);
        return true;
    } else if (args.find(L"/base") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        PlayerCommands::BaseHelp(client, args);
        return true;
    }
    return false;
}

static bool IsDockingAllowed(PlayerBase *base, uint client) {
    // Allies can always dock.
    std::wstring charname =
        (const wchar_t *)Players.GetActiveCharacterName(client);
    for (list<std::wstring>::iterator i = base->ally_tags.begin();
         i != base->ally_tags.end(); ++i) {
        if (charname.find(*i) == 0) {
            return true;
        }
    }

    // Base allows neutral ships to dock
    if (base->defense_mode == 2)
        return true;

    return false;
}

// If this is a docking request at a player controlled based then send
// an update to set the base arrival text, base economy and change the
// infocards.
int __cdecl Dock_Call(unsigned int const &iShip, unsigned int const &base,
                      int iCancel, enum DOCK_HOST_RESPONSE response) {
    returncode = DEFAULT_RETURNCODE;

    uint client = HkGetClientIDByShip(iShip);
    if (client && response == PROCEED_DOCK) {
        PlayerBase *pbase = GetPlayerBase(base);
        if (pbase) {
            // Shield is up, docking is not possible.
            if (pbase->shield_active_time) {
                PrintUserCmdText(
                    client, L"Docking failed because base shield is active");
                pub::Player::SendNNMessage(
                    client, pub::GetNicknameId("info_access_denied"));
                returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                return 0;
            }

            // Docking prohibited
            if (!IsDockingAllowed(pbase, client)) {
                PrintUserCmdText(client, L"Docking at this base is restricted");
                pub::Player::SendNNMessage(
                    client, pub::GetNicknameId("info_access_denied"));
                returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                return 0;
            }

            SendBaseStatus(client, pbase);
        }
    }
    return 0;
}

void __stdcall CharacterSelect(struct CHARACTER_ID const &cId,
                               unsigned int client) {
    returncode = DEFAULT_RETURNCODE;

    // Sync base names for the
    map<uint, PlayerBase *>::iterator base = player_bases.begin();
    for (; base != player_bases.end(); base++) {
        HkChangeIDSString(client, base->second->solar_ids,
                          base->second->basename);
    }
}

void __stdcall CharacterSelect_AFTER(struct CHARACTER_ID const &cId,
                                     unsigned int client) {
    returncode = DEFAULT_RETURNCODE;

    if (set_plugin_debug > 1)
        ConPrint(L"CharacterSelect_AFTER client=%u player_base=%u\n", client,
                 clients[client].player_base);

    // If this ship is in a player base is then set then docking ID to emulate
    // a landing.
    LoadDockState(client);
    if (clients[client].player_base) {
        if (set_plugin_debug > 1)
            ConPrint(L"CharacterSelect_AFTER[2] client=%u player_base=%u\n",
                     client, clients[client].player_base);

        // If this base does not exist, dump the ship into space
        PlayerBase *base = GetPlayerBase(clients[client].player_base);
        if (!base) {
            DeleteDockState(client);
            SendResetMarketOverride(client);
            ForceLaunch(client);
        }
        // If the player file indicates that the ship is in a base but this
        // isn't this base then dump the ship into space.
        else if (Players[client].iBaseID != base->proxy_base) {
            DeleteDockState(client);
            SendResetMarketOverride(client);
            ForceLaunch(client);
        }
    }
}

void __stdcall BaseEnter(uint base, uint client) {
    if (set_plugin_debug > 1)
        ConPrint(
            L"BaseEnter base=%u client=%u player_base=%u last_player_base=%u\n",
            base, client, clients[client].player_base,
            clients[client].last_player_base);

    returncode = DEFAULT_RETURNCODE;

    clients[client].admin = false;

    // If the last player base is set then we have not docked at a non player
    // base yet.
    if (clients[client].last_player_base) {
        clients[client].player_base = clients[client].last_player_base;
    }

    // If the player is registered as being in a player controlled base then
    // send the economy update, player system update and save a file to indicate
    // that we're in the base->
    if (clients[client].player_base) {
        PlayerBase *base = GetPlayerBaseForClient(client);
        if (base) {
            // Reset the commodity list	and send a dummy entry if there are no
            // commodities in the market
            SaveDockState(client);
            SendMarketGoodSync(base, client);
            SendBaseStatus(client, base);
            return;
        } else {
            // Force the ship to launch to space as the base has been destroyed
            DeleteDockState(client);
            SendResetMarketOverride(client);
            ForceLaunch(client);
            return;
        }
    }

    DeleteDockState(client);
    SendResetMarketOverride(client);
}

void __stdcall BaseExit(uint base, uint client) {
    returncode = DEFAULT_RETURNCODE;

    if (set_plugin_debug > 1)
        ConPrint(L"BaseExit base=%u client=%u player_base=%u\n", base, client,
                 clients[client].player_base);

    // Reset client state and save it retaining the last player base ID to deal
    // with respawn.
    clients[client].admin = false;
    if (clients[client].player_base) {
        if (set_plugin_debug)
            ConPrint(L"BaseExit base=%u client=%u player_base=%u\n", base,
                     client, clients[client].player_base);

        clients[client].last_player_base = clients[client].player_base;
        clients[client].player_base = 0;
        SaveDockState(client);
    } else {
        DeleteDockState(client);
    }

    // Clear the base market and text
    SendResetMarketOverride(client);
    SendSetBaseInfoText2(client, L"");

    // std::wstring base_status = L"<RDL><PUSH/>";
    // base_status += L"<TEXT>" + XMLText(base->name) + L", " +
    // HkGetWStringFromIDS(sys->strid_name) +  L"</TEXT><PARA/><PARA/>";
}

void __stdcall RequestEvent(int iIsFormationRequest, unsigned int iShip,
                            unsigned int iDockTarget, unsigned int p4,
                            unsigned long p5, unsigned int client) {
    returncode = DEFAULT_RETURNCODE;
    if (client) {
        if (!iIsFormationRequest) {
            PlayerBase *base = GetPlayerBase(iDockTarget);
            if (base) {
                // Shield is up, docking is not possible.
                if (base->shield_active_time) {
                    PrintUserCmdText(
                        client,
                        L"Docking failed because base shield is active");
                    pub::Player::SendNNMessage(
                        client, pub::GetNicknameId("info_access_denied"));
                    returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                    return;
                }

                if (!IsDockingAllowed(base, client)) {
                    PrintUserCmdText(client,
                                     L"Docking at this base is restricted");
                    pub::Player::SendNNMessage(
                        client, pub::GetNicknameId("info_access_denied"));
                    returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                    return;
                }
            }
        }
    }
}

/// The base the player is launching from.
PlayerBase *player_launch_base = 0;

/// If the ship is launching from a player base record this so that
/// override the launch location.
bool __stdcall LaunchPosHook(uint space_obj, struct CEqObj &p1, Vector &pos,
                             Matrix &rot, int dock_mode) {
    returncode = DEFAULT_RETURNCODE;
    if (player_launch_base) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        pos = player_launch_base->position;
        rot = player_launch_base->rotation;
        TranslateX(pos, rot, -750);
        if (set_plugin_debug)
            ConPrint(L"LaunchPosHook[1] space_obj=%u pos=%0.0f %0.0f %0.0f "
                     L"dock_mode=%u\n",
                     space_obj, pos.x, pos.y, pos.z, dock_mode);
        player_launch_base = 0;
    }
    return true;
}

/// If the ship is launching from a player base record this so that
/// we will override the launch location.
void __stdcall PlayerLaunch(unsigned int ship, unsigned int client) {
    returncode = DEFAULT_RETURNCODE;
    if (set_plugin_debug > 1)
        ConPrint(L"PlayerLaunch ship=%u client=%u\n", ship, client);
    player_launch_base = GetPlayerBase(clients[client].last_player_base);
}

void __stdcall PlayerLaunch_AFTER(unsigned int ship, unsigned int client) {
    returncode = DEFAULT_RETURNCODE;
    SyncReputationForClientShip(ship, client);
}

void __stdcall JumpInComplete(unsigned int system, unsigned int ship) {
    returncode = DEFAULT_RETURNCODE;

    if (set_plugin_debug > 1)
        ConPrint(L"JumpInComplete system=%u ship=%u\n");

    uint client = HkGetClientIDByShip(ship);
    if (client) {
        SyncReputationForClientShip(ship, client);
    }
}

void __stdcall GFGoodSell(struct SGFGoodSellInfo const &gsi,
                          unsigned int client) {
    returncode = DEFAULT_RETURNCODE;

    // If the client is in a player controlled base
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (base) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;

        if (base->market_items.find(gsi.iArchID) == base->market_items.end() &&
            !clients[client].admin) {
            PrintUserCmdText(client, L"ERR: Base will not accept goods");
            clients[client].reverse_sell = true;
            return;
        }

        MARKET_ITEM &item = base->market_items[gsi.iArchID];

        uint count = gsi.iCount;
        int price = (int)item.price * count;

        // If the base doesn't have sufficient cash to support this purchase
        // reduce the amount purchased and shift the cargo back to the ship.
        if (base->money < price) {
            PrintUserCmdText(
                client, L"ERR: Base cannot accept goods, insufficient cash");
            clients[client].reverse_sell = true;
            return;
        }

        if ((item.quantity + count) > item.max_stock) {
            PrintUserCmdText(
                client, L"ERR: Base cannot accept goods, stock limit reached");
            clients[client].reverse_sell = true;
            return;
        }

        // Prevent player from getting invalid net worth.
        float fValue;
        pub::Player::GetAssetValue(client, fValue);

        int iCurrMoney;
        pub::Player::InspectCash(client, iCurrMoney);
        if (fValue + price > 2100000000 || iCurrMoney + price > 2100000000) {
            PrintUserCmdText(client, L"ERR: Base will not accept goods");
            clients[client].reverse_sell = true;
            return;
        }

        if (!base->AddMarketGood(gsi.iArchID, gsi.iCount)) {
            PrintUserCmdText(client, L"ERR: Base will not accept goods");
            clients[client].reverse_sell = true;
            return;
        }

        pub::Player::AdjustCash(client, price);
        base->ChangeMoney(0 - price);
        base->Save();
    }
}

void __stdcall ReqRemoveItem(unsigned short slot, int count,
                             unsigned int client) {
    returncode = DEFAULT_RETURNCODE;

    if (clients[client].player_base) {
        returncode = SKIPPLUGINS;
        if (clients[client].reverse_sell) {
            int hold_size;
            HkEnumCargo((const wchar_t *)Players.GetActiveCharacterName(client),
                        clients[client].cargo, hold_size);
        }
    }
}

void __stdcall ReqRemoveItem_AFTER(unsigned short iID, int count,
                                   unsigned int client) {
    returncode = DEFAULT_RETURNCODE;

    uint player_base = clients[client].player_base;
    if (player_base) {
        returncode = SKIPPLUGINS;
        if (clients[client].reverse_sell) {
            clients[client].reverse_sell = false;

            for (auto &ci : clients[client].cargo) {
                if (ci.iID == iID) {
                    Server.ReqAddItem(ci.iArchID, ci.hardpoint.value, count,
                                      ci.fStatus, ci.bMounted, client);
                    return;
                }
            }
        } else {
            // Update the player CRC so that the player is not kicked for 'ship
            // related' kick
            HkPlayerRecalculateCRC(client);
        }
    }
}

void __stdcall GFGoodBuy(struct SGFGoodBuyInfo const &gbi,
                         unsigned int client) {
    returncode = DEFAULT_RETURNCODE;

    // If the client is in a player controlled base
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (base) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;

        uint count = gbi.iCount;
        if (count > base->market_items[gbi.iGoodID].quantity)
            count = base->market_items[gbi.iGoodID].quantity;

        int price = (int)base->market_items[gbi.iGoodID].price * count;
        int curr_money;
        pub::Player::InspectCash(client, curr_money);

        // In theory, these should never be called.
        if (count == 0 || base->market_items[gbi.iGoodID].min_stock >=
                              base->market_items[gbi.iGoodID].quantity) {
            PrintUserCmdText(client, L"ERR Base will not sell goods");
            returncode = SKIPPLUGINS_NOFUNCTIONCALL;
            clients[client].stop_buy = true;
            return;
        } else if (curr_money < price) {
            PrintUserCmdText(client, L"ERR Not enough credits");
            returncode = SKIPPLUGINS_NOFUNCTIONCALL;
            clients[client].stop_buy = true;
            return;
        }

        clients[client].stop_buy = false;
        base->RemoveMarketGood(gbi.iGoodID, count);
        pub::Player::AdjustCash(client, 0 - price);
        base->ChangeMoney(price);
        base->Save();
    }
}

void __stdcall ReqAddItem(unsigned int good, char const *hardpoint, int count,
                          float fStatus, bool bMounted, unsigned int client) {
    returncode = DEFAULT_RETURNCODE;
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (base) {
        returncode = SKIPPLUGINS;
        if (clients[client].stop_buy) {
            if (clients[client].stop_buy)
                clients[client].stop_buy = false;
            returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        }
    }
}

void __stdcall ReqAddItem_AFTER(unsigned int good, char const *hardpoint,
                                int count, float fStatus, bool bMounted,
                                unsigned int client) {
    returncode = DEFAULT_RETURNCODE;

    // If the client is in a player controlled base
    PlayerBase *base = GetPlayerBaseForClient(client);
    if (base) {
        returncode = SKIPPLUGINS;
        PlayerData *pd = &Players[client];

        // Update the player CRC so that the player is not kicked for 'ship
        // related' kick
        HkPlayerRecalculateCRC(client);

        // Add to check-list which is being compared to the users equip-list
        // when saving char to fix "Ship or Equipment not sold on base" kick
        EquipDesc ed;
        ed.sID = pd->sLastEquipID;
        ed.iCount = 1;
        ed.iArchID = good;
        pd->lShadowEquipDescList.add_equipment_item(ed, false);
    }
}

/// Ignore cash commands from the client when we're in a player base.
void __stdcall ReqChangeCash(int cash, unsigned int client) {
    returncode = DEFAULT_RETURNCODE;
    if (clients[client].player_base)
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
}

/// Ignore cash commands from the client when we're in a player base.
void __stdcall ReqSetCash(int cash, unsigned int client) {
    returncode = DEFAULT_RETURNCODE;
    if (clients[client].player_base)
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
}

void __stdcall ReqEquipment(class EquipDescList const &edl,
                            unsigned int client) {
    returncode = DEFAULT_RETURNCODE;
    if (clients[client].player_base)
        returncode = SKIPPLUGINS;
}

void __stdcall CShip_destroy(CShip *ship) {
    returncode = DEFAULT_RETURNCODE;

    // Dispatch the destroy event to the appropriate module.
    uint space_obj = ship->get_id();
    map<uint, Module *>::iterator i = spaceobj_modules.find(space_obj);
    if (i != spaceobj_modules.end()) {
        returncode = SKIPPLUGINS;
        i->second->SpaceObjDestroyed(space_obj);
    }
}

void BaseDestroyed(uint space_obj, uint client) {
    returncode = DEFAULT_RETURNCODE;
    map<uint, Module *>::iterator i = spaceobj_modules.find(space_obj);
    if (i != spaceobj_modules.end()) {
        returncode = SKIPPLUGINS;
        i->second->SpaceObjDestroyed(space_obj);
    }
}

void __stdcall HkCb_AddDmgEntry(DamageList *dmg, unsigned short p1,
                                float damage,
                                enum DamageEntry::SubObjFate fate) {
    returncode = DEFAULT_RETURNCODE;
    if (iDmgToSpaceID && dmg->get_inflictor_id()) {
        float curr, max;
        pub::SpaceObj::GetHealth(iDmgToSpaceID, curr, max);

        map<uint, Module *>::iterator i = spaceobj_modules.find(iDmgToSpaceID);
        if (i != spaceobj_modules.end()) {
            if (set_plugin_debug)
                ConPrint(
                    L"HkCb_AddDmgEntry iDmgToSpaceID=%u get_inflictor_id=%u "
                    L"curr=%0.2f max=%0.0f damage=%0.2f cause=%u is_player=%u "
                    L"player_id=%u fate=%u\n",
                    iDmgToSpaceID, dmg->get_inflictor_id(), curr, max, damage,
                    dmg->get_cause(), dmg->is_inflictor_a_player(),
                    dmg->get_inflictor_owner_player(), fate);

            // A work around for an apparent bug where mines/missiles at the
            // base causes the base damage to jump down to 0 even if the base is
            // otherwise healthy.
            if (damage == 0.0f /*&& dmg->get_cause()==7*/ && curr > 200000) {
                returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                if (set_plugin_debug)
                    ConPrint(L"HkCb_AddDmgEntry[1] - invalid damage?\n");
                return;
            }

            // If this is an NPC hit then suppress the call completely
            if (!dmg->is_inflictor_a_player()) {
                if (set_plugin_debug)
                    ConPrint(L"HkCb_AddDmgEntry[2] suppressed - npc\n");
                returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                iDmgToSpaceID = 0;
                return;
            }

            // This call is for us, skip all plugins.
            returncode = SKIPPLUGINS;
            float new_damage = i->second->SpaceObjDamaged(
                iDmgToSpaceID, dmg->get_inflictor_id(), curr, damage);
            if (new_damage != 0.0f) {
                returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                if (set_plugin_debug)
                    ConPrint(L"HkCb_AddDmgEntry[3] suppressed - shield up - "
                             L"new_damage=%0.0f\n",
                             new_damage);
                dmg->add_damage_entry(p1, new_damage, fate);
                iDmgToSpaceID = 0;
                return;
            }
        }
    }
}

static void ForcePlayerBaseDock(uint client, PlayerBase *base) {
    char system_nick[1024];
    pub::GetSystemNickname(system_nick, sizeof(system_nick), base->system);

    char proxy_base_nick[1024];
    sprintf_s(proxy_base_nick, "%s_proxy_base", system_nick);

    uint proxy_base_id = CreateID(proxy_base_nick);

    clients[client].player_base = base->base;
    clients[client].last_player_base = base->base;

    if (set_plugin_debug > 1)
        ConPrint(L"ForcePlayerBaseDock client=%u player_base=%u\n", client,
                 clients[client].player_base);

    uint system;
    pub::Player::GetSystem(client, system);

    pub::Player::ForceLand(client, proxy_base_id);
    if (system != base->system) {
        Server.BaseEnter(proxy_base_id, client);
        Server.BaseExit(proxy_base_id, client);

        std::wstring charname =
            (const wchar_t *)Players.GetActiveCharacterName(client);
        std::wstring charfilename;
        HkGetCharFileName(charname, charfilename);
        charfilename += L".fl";
        CHARACTER_ID charid;
        strcpy_s(charid.szCharFilename, wstos(charname.substr(0, 14)).c_str());

        Server.CharacterSelect(charid, client);
    }
}

#define IS_CMD(a) !args.compare(L##a)

bool ExecuteCommandString_Callback(CCmds *cmd, const std::wstring &args) {
    returncode = DEFAULT_RETURNCODE;
    /*if (args.find(L"dumpbases")==0)
    {
            Universe::ISystem *sys = Universe::GetFirstSystem();
            FILE* f = fopen("bases.txt", "w");
            while (sys)
            {
                    fprintf(f, "[Base]\n");
                    fprintf(f, "nickname = %s_proxy_base\n", sys->nickname);
                    fprintf(f, "system = %s\n", sys->nickname);
                    fprintf(f, "strid_name = 0\n");
                    fprintf(f, "file=Universe\\Systems\\proxy_base->ini\n");
                    fprintf(f, "BGCS_base_run_by=W02bF35\n\n");

                    sys = Universe::GetNextSystem();
            }
            fclose(f);
    }
    if (args.find(L"makebases")==0)
    {
            struct Universe::ISystem *sys = Universe::GetFirstSystem();
            while (sys)
            {
                    std::string path =
    std::string("..\\DATA\\UNIVERSE\\SYSTEMS\\") + std::string(sys->nickname) +
    "\\" + std::string(sys->nickname) + ".ini"; FILE *file = fopen(path.c_str(),
    "a+"); if (file)
                    {
                            ConPrint(L"doing path %s\n", stows(path).c_str());
                            fprintf(file, "\n\n[Object]\n");
                            fprintf(file, "nickname = %s_proxy_base\n",
    sys->nickname); fprintf(file, "dock_with = %s_proxy_base\n", sys->nickname);
                            fprintf(file, "base = %s_proxy_base\n",
    sys->nickname); fprintf(file, "pos = 0, -100000, 0\n"); fprintf(file,
    "archetype = invisible_base\n"); fprintf(file, "behavior = NOTHING\n");
                            fprintf(file, "visit = 128\n");
                            fclose(file);
                    }
                    sys = Universe::GetNextSystem();
            }
            return true;
    }*/
    if (args.find(L"testrecipe") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;

        uint client = HkGetClientIdFromCharname(cmd->GetAdminName());
        PlayerBase *base = GetPlayerBaseForClient(client);
        if (!base) {
            cmd->Print(L"ERR Not in player base");
            return true;
        }

        uint recipe_name = CreateID(wstos(cmd->ArgStr(1)).c_str());

        RECIPE recipe = recipes[recipe_name];
        for (map<uint, uint>::iterator i = recipe.consumed_items.begin();
             i != recipe.consumed_items.end(); ++i) {
            base->market_items[i->first].quantity += i->second;
            SendMarketGoodUpdated(base, i->first, base->market_items[i->first]);
            cmd->Print(L"Added %ux %08x", i->second, i->first);
        }
        base->Save();
        cmd->Print(L"OK");
        return true;
    } else if (args.find(L"testdeploy") == 0) {
        returncode = SKIPPLUGINS_NOFUNCTIONCALL;
        uint client = HkGetClientIdFromCharname(cmd->GetAdminName());
        if (!client) {
            cmd->Print(L"ERR Not in game");
            return true;
        }

        for (map<uint, uint>::iterator i = construction_items.begin();
             i != construction_items.end(); ++i) {
            uint good = i->first;
            uint quantity = i->second;
            pub::Player::AddCargo(client, good, quantity, 1.0, false);
        }

        cmd->Print(L"OK");
        return true;
    } else if (args.compare(L"beam") == 0) {
        returncode = DEFAULT_RETURNCODE;
        std::wstring charname = cmd->ArgCharname(1);
        std::wstring basename = cmd->ArgStrToEnd(2);

        // Fall back to default behaviour.
        if (!(cmd->rights & RIGHT_SUPERADMIN)) {
            return false;
        }

        HKPLAYERINFO info;
        if (HkGetPlayerInfo(charname, info, false) != HKE_OK) {
            return false;
        }

        if (info.iShip == 0) {
            return false;
        }

        // Search for an match at the start of the name
        for (map<uint, PlayerBase *>::iterator i = player_bases.begin();
             i != player_bases.end(); ++i) {
            if (ToLower(i->second->basename).find(ToLower(basename)) == 0) {
                returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                ForcePlayerBaseDock(info.iClientID, i->second);
                cmd->Print(L"OK");
                return true;
            }
        }

        // Exact match failed, try a for an partial match
        for (map<uint, PlayerBase *>::iterator i = player_bases.begin();
             i != player_bases.end(); ++i) {
            if (ToLower(i->second->basename).find(ToLower(basename)) != -1) {
                returncode = SKIPPLUGINS_NOFUNCTIONCALL;
                ForcePlayerBaseDock(info.iClientID, i->second);
                cmd->Print(L"OK");
                return true;
            }
        }

        // Fall back to default flhook .beam command
        return false;
    }

    return false;
}

void Plugin_Communication_CallBack(PLUGIN_MESSAGE msg, void *data) {
    returncode = DEFAULT_RETURNCODE;

    if (msg == CUSTOM_BASE_BEAM) {
        auto *info = static_cast<CUSTOM_BASE_BEAM_STRUCT *>(data);
        PlayerBase *base = GetPlayerBase(info->iTargetBaseID);
        if (base) {
            returncode = SKIPPLUGINS;
            ForcePlayerBaseDock(info->iClientID, base);
            info->bBeamed = true;
        }
    } else if (msg == CUSTOM_BASE_IS_DOCKED) {
        auto *info = static_cast<CUSTOM_BASE_IS_DOCKED_STRUCT *>(data);
        PlayerBase *base = GetPlayerBaseForClient(info->iClientID);
        if (base) {
            returncode = SKIPPLUGINS;
            info->iDockedBaseID = base->base;
        }
    }
    return;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** Functions to hook */
EXPORT PLUGIN_INFO *Get_PluginInfo() {
    PLUGIN_INFO *p_PI = new PLUGIN_INFO();
    p_PI->sName = "Base Plugin by cannon";
    p_PI->sShortName = "base";
    p_PI->bMayPause = true;
    p_PI->bMayUnload = true;
    p_PI->ePluginReturnCode = &returncode;
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&LoadSettings, PLUGIN_LoadSettings, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&ClearClientInfo,
                                             PLUGIN_ClearClientInfo, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&CharacterSelect, PLUGIN_HkIServerImpl_CharacterSelect, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&RequestEvent, PLUGIN_HkIServerImpl_RequestEvent, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&LaunchPosHook, PLUGIN_LaunchPosHook, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&PlayerLaunch, PLUGIN_HkIServerImpl_PlayerLaunch, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&PlayerLaunch_AFTER,
                        PLUGIN_HkIServerImpl_PlayerLaunch_AFTER, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&CharacterSelect_AFTER,
                        PLUGIN_HkIServerImpl_CharacterSelect_AFTER, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&JumpInComplete, PLUGIN_HkIServerImpl_JumpInComplete, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&BaseEnter, PLUGIN_HkIServerImpl_BaseEnter, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&BaseExit,
                                             PLUGIN_HkIServerImpl_BaseExit, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&Dock_Call, PLUGIN_HkCb_Dock_Call, 0));

    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&GFGoodSell, PLUGIN_HkIServerImpl_GFGoodSell, 15));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&ReqRemoveItem, PLUGIN_HkIServerImpl_ReqRemoveItem, 15));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&ReqRemoveItem_AFTER,
                        PLUGIN_HkIServerImpl_ReqRemoveItem_AFTER, 15));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&GFGoodBuy, PLUGIN_HkIServerImpl_GFGoodBuy, 15));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&ReqAddItem, PLUGIN_HkIServerImpl_ReqAddItem, 15));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&ReqAddItem_AFTER,
                        PLUGIN_HkIServerImpl_ReqAddItem_AFTER, 15));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&ReqChangeCash, PLUGIN_HkIServerImpl_ReqChangeCash, 15));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&ReqSetCash, PLUGIN_HkIServerImpl_ReqSetCash, 15));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&ReqEquipment, PLUGIN_HkIServerImpl_ReqEquipment, 11));

    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&HkTimerCheckKick,
                                             PLUGIN_HkTimerCheckKick, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&UserCmd_Process,
                                             PLUGIN_UserCmd_Process, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&ExecuteCommandString_Callback,
                        PLUGIN_ExecuteCommandString_Callback, 0));

    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO(
        (FARPROC *)&CShip_destroy, PLUGIN_HkIEngine_CShip_destroy, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&BaseDestroyed, PLUGIN_BaseDestroyed, 0));
    p_PI->lstHooks.push_back(PLUGIN_HOOKINFO((FARPROC *)&HkCb_AddDmgEntry,
                                             PLUGIN_HkCb_AddDmgEntry, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&Plugin_Communication_CallBack,
                        PLUGIN_Plugin_Communication, 11));
    return p_PI;
}
