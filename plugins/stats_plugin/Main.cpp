// Stats Plugin
// by Raikkonen

// Includes
#include "Main.h"

std::string jsonFileName;
std::map<uint, std::wstring> mapShips;

// Load configuration file
void LoadSettings() {
    returncode = DEFAULT_RETURNCODE;

    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string configFile =
        std::string(szCurDir) + "\\flhook_plugins\\stats.cfg";
    jsonFileName = IniGetS(configFile, "General", "jsonFileName", "");

    HkLoadStringDLLs();

    // Load in shiparch.ini to generate IDs based off the nickname and generate
    // ship names via ids_name
    std::string shiparchfile = "..\\data\\ships\\shiparch.ini";

    INI_Reader ini;
    if (ini.open(shiparchfile.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("Ship")) {
                int idsname = 0;
                while (ini.read_value()) {
                    if (ini.is_value("nickname")) {
                        uint shiphash = CreateID(ini.get_value_string(0));
                        mapShips[shiphash] = HkGetWStringFromIDS(idsname);
                    }
                    if (ini.is_value("ids_name")) {
                        idsname = ini.get_value_int(0);
                    }
                }
            }
        }
        ini.close();
    }
}

std::string encode(std::string data) {
    std::string scEncoded;
    scEncoded.reserve(data.size());
    for (size_t pos = 0; pos != data.size(); ++pos) {
        if (data[pos] == '\"')
            scEncoded.append("&quot;");
        else
            scEncoded.append(1, data[pos]);
    }
    return scEncoded;
}

void exportJSON() {
    std::ofstream out(jsonFileName);

    // Add Server Load object
    out << "{\"serverload\": \"" + std::to_string(g_iServerLoad) + "\"";

    // Begin Player array
    out << ",\"players\": [";

    std::list<HKPLAYERINFO> lstPlayers = HkGetPlayers();
    for (std::list<HKPLAYERINFO>::iterator player = lstPlayers.begin();
         player != lstPlayers.end(); player++) {
        // Add comma is not the first player
        if (player != lstPlayers.begin())
            out << ",";

        // Add name
        out << "{\"name\": \"" + encode(wstos(player->wscCharname)) + "\"";

        // Add rank
        int iRank;
        pub::Player::GetRank(player->iClientID, iRank);
        out << ",\"rank\": \"" + std::to_string(iRank) + "\"";

        // Add group
        int groupID = Players.GetGroupID(player->iClientID);
        out << ",\"group\": \"" + std::to_string(groupID) + "\"";

        // Add ship
        Archetype::Ship *ship =
            Archetype::GetShip(Players[player->iClientID].iShipArchetype);
        (ship) ? out << ",\"ship\": \"" + wstos(mapShips[ship->get_id()]) + "\""
               : out << ",\"ship\": \"Unknown\"";

        // Add system
        uint iSystemID;
        pub::Player::GetSystem(player->iClientID, iSystemID);
        const Universe::ISystem *iSys = Universe::get_system(iSystemID);
        out << ",\"system\": \"" +
                   wstos(HkGetWStringFromIDS(iSys->strid_name)) + "\"}";
    }

    // End Player array and rest of output
    out << "]}";
    out.close();
}

// Hooks for updating stats
void __stdcall DisConnect_AFTER(unsigned int iClientID,
                                enum EFLConnection state) {
    returncode = DEFAULT_RETURNCODE;
    exportJSON();
}

void __stdcall PlayerLaunch_AFTER(unsigned int iShip, unsigned int client) {
    returncode = DEFAULT_RETURNCODE;
    exportJSON();
}

void __stdcall CharacterSelect_AFTER(struct CHARACTER_ID const &charId,
                                     unsigned int iClientID) {
    returncode = DEFAULT_RETURNCODE;
    exportJSON();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// FLHOOK STUFF
///////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Do things when the dll is loaded
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    srand((uint)time(0));

    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH && set_scCfgFile.length() > 0)
        LoadSettings();

    return true;
}

// Functions to hook
EXPORT PLUGIN_INFO *Get_PluginInfo() {
    PLUGIN_INFO *p_PI = new PLUGIN_INFO();
    p_PI->sName = "Stats";
    p_PI->sShortName = "stats";
    p_PI->bMayPause = true;
    p_PI->bMayUnload = true;
    p_PI->ePluginReturnCode = &returncode;
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&LoadSettings, PLUGIN_LoadSettings, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&PlayerLaunch_AFTER,
                        PLUGIN_HkIServerImpl_PlayerLaunch_AFTER, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&DisConnect_AFTER,
                        PLUGIN_HkIServerImpl_DisConnect_AFTER, 0));
    p_PI->lstHooks.push_back(
        PLUGIN_HOOKINFO((FARPROC *)&CharacterSelect_AFTER,
                        PLUGIN_HkIServerImpl_CharacterSelect_AFTER, 0));
    return p_PI;
}
