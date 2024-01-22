/**
 * @date August, 2022
 * @author Raikkonen
 * @defgroup AwayFromKeyboard Away from Keyboard
 * @brief
 * The AFK plugin allows you to set yourself as Away from Keyboard.
 * This will notify other players if they try and speak to you, that you are not at your desk.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - afk - Sets your status to Away from Keyboard. Other players will notified if they try to speak to you.
 * - back - Removes the AFK status.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * No configuration file is needed.
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 */

#include "PCH.hpp"

#include "Afk.hpp"

#include "Utils/Detour.hpp"

#include "stduuid/uuid.h"

struct VanillaSaveData final
{
        wchar_t accId[40];
        long x050, x054, x058, x05C;
        uint numberOCharacters;
        CHARACTER_ID charFile;                                    // 0x98 / 152 / this+608
        uint shipHash;                                            // 0x99 / 153 / this+612
        float shipHullStatus;                                     // 0x9A / 154 / this+616
        st6::list<Archetype::CollisionGroup*> collisionGroupList; // 0x9B / 155 / this+620
        st6::list<EquipDesc> equipment;                           // 0x9E / 158 / this+632
        int rank;                                                 // 0xA0 / 160 / this+640
        int unk4;                                                 // 0xA1, / 161, / this + 644
        Costume comCostume;                                       // 0xA3 / 162 / this+652
        uint voiceLen;
        char voice[32];          // I hate it
        Costume baseCostume;     // 0xB9 / 185 / this+740
        int rep;                 // 0xB9 / 198 / this+792
        int money;               // 0xC7 / 199 / this+796
        int unknown5[5];         // 0xC8 / 200 / this+800
        int killCount;           // 0xCD / 205 / this+820
        int missionSuccessCount; // 0xCE / 206 / this+824
        int missionFailureCount; // 0xCF / 207 / this+828
        uint unknown6[4];        // 0xD0 / 208 / this+832
        Vector pos;
        Matrix rot;
        uint unknown62; // 0xD0 / 208 / this+832
        char* preFilledWeaponGroupInfo;
        uint unknown63[4];         // 0xD0 / 208 / this+832
        int* SPNeuralNetLogUnk;    // 0xE5 / 229 / this+916
        int interfaceToggleBitmap; // 0xE7 / 231 / this + 924 int unknown7[2]; // 0xD0 / 208 / this+832
        st6::list<void*> visitEntries;
        uint unknown8[13]; // 0xEC / 236 / this+944
        uint systemId;     // 0xF8 / 248 / this+992
        uint unknown9;     // 0xF9 / 249 / this+996
        uint unknown10;    // 0xFA / 250 / this+1000
        uint baseId;       // 0xFB / 251, / this + 1004
        uint lastBaseId;   // 0xFC / 252 / this+1008
        uint baseId3;      // duplicate of previous 2 if docked
        uint baseRoomId;
        uint padding[30];
};

BinarySearchTree<uint> items;
BinarySearchTree<uint>::Node item;

struct VanillaLoadData
{
        uint padding[128];
        st6::wstring name;        // 512
        st6::wstring description; // 528
        uint descripStrId;        // 532
        uint datetimeHigh;
        uint datetimeLow;
        uint shipHash;                             // 544
        uint money;                                // 548
        uint numOfKills;                           // 552
        uint numOfSuccessMissions;                 // 556
        uint numOfFailedMissions;                  // 560
        float hullStatus;                          // 564
        st6::list<EquipDesc> currentEquipAndCargo; // 572
        st6::list<CollisionGroupDesc> currentCollisionGroups;
        float baseHullStatus;                   // 592
        st6::list<EquipDesc> baseEquipAndCargo; // 600
        st6::list<CollisionGroupDesc> baseCollisionGroups;
        uint currentBase;    // 620
        uint lastDockedBase; // 624
        uint currentRoom;    // 628
        uint system;         // 632
        Vector pos;
        Matrix rot;
        uint startingRing;
        uint rank;                                   // 688
        uint unused1;                                // 692
        st6::vector<uint> someBinaryTreeAboutGroups; // SP only
        Costume commCostume;                             // 712
        uint voiceLen;                               // 764
        char voice[32];                              // 768
        Costume baseCostume;                         // 800
        uint tempCargoIdEnumerator;                  // 852
        st6::string prefilledWeaponGroupIni;         // 860
        st6::list<FmtStr> neuralNetLog;              // 872 - probably wrong, 'log' entries from character file. Also 'mostly' SP only.
        uint interfaceState;                         // 884
        uint unused2;                                // 888
        BinarySearchTree<uint> visitLists;           // actually binary search tree
};

struct MPlayerDataSaveStruct
{
        uint padding0[8];
        bool padding8;                           // 32
        bool canDock;                            // 33
        uint can_dock2;                          // 36
        st6::list<uint> DockExceptions;          // 40
        bool can_tl;                             // 52
        uint padding14;                          // 56
        uint TLExceptionStart;                   // 60
        uint TLExceptionEnd;                     // 64
        uint padding17;                          // 68
        BinarySearchTree<uint> BST_killed_ships; // 76
        BinarySearchTree<uint> BST_rm_completed; // 96
        BinarySearchTree<uint> BST_rm_Aborted;   // 116
        BinarySearchTree<uint> BST_rm_Failed;    // 136
        float totalCashEarned;                   // 152
        float totalTimePlayed;                   // 156
        uint padding40;                          // 160
        uint visitedSystemsStart;                // 164
        uint visitedSystemsEnd;                  // 168
        uint padding43;                          // 172
        uint padding44;                          // 176
        uint visitedBasesStart;                  // 180
        uint visitedBasesEnd;                    // 184
        uint padding47;                          // 188
        uint padding48;                          // 192
        uint visitedHolesStart;                  // 196
        uint visitedHolesEnd;                    // 200
        uint padding51;                          // 204
        uint padding52;                          // 208
        uint padding53;                          // 212
        uint padding54;                          // 216
        uint padding55;                          // 220
        uint padding56;                          // 224
        uint padding57;                          // 228
        uint padding58;                          // 232
        uint padding59;                          // 236
        uint vNPCStart;                          // 240
        uint vNPCEnd;                            // 244
        uint padding62;                          // 248
        uint padding63;                          // 252
        uint rumorStart;                         // 256
        uint rumorEnd;                           // 260
        uint padding66;                          // 264
};

namespace Plugins
{
    // writer 6D47120
    // reader 6D47120
    using OnCharacterSave = bool(__fastcall*)(PlayerData* saveData, void* edx, char* file, ushort* timestamp, int timestampStr);
    using OnCharacterLoad = bool(__fastcall*)(void* unk, void* edx, char* filename, VanillaLoadData* data);
    using OnCharacterMPlayerSectionRead = void(__fastcall*)(DWORD* data, void* edx, INI_Reader* reader);
    using OnCharacterMPlayerSectionSave = int*(__fastcall*)(MPlayerDataSaveStruct* mData, void* edx, FILE* stream);

    FunctionDetour saveDetour(OnCharacterSave(DWORD(GetModuleHandleA("server.dll")) + 0x6CCD0));
    FunctionDetour loadDetour(OnCharacterLoad(DWORD(GetModuleHandleA("server.dll")) + 0x67120));
    std::unique_ptr<FunctionDetour<OnCharacterMPlayerSectionRead>> mPlayerReadDetour;
    std::unique_ptr<FunctionDetour<OnCharacterMPlayerSectionSave>> mPlayerSaveDetour;

    // content.dll + 0x12FBBC

    VanillaSaveData tempData;
    VanillaLoadData loadData;
    bool weSavin = false;
    bool __fastcall SaveDetour(PlayerData* saveData, void* edx, char* file, ushort* timestamp, int timestampStr)
    {
        // Node<MPlayerDataSaveStruct*> testtest;
        // BinarySearchTree<MPlayerDataSaveStruct*> test;
        auto bst = (BinarySearchTree<MPlayerDataSaveStruct*>*)(DWORD(GetModuleHandleA("content.dll")) + 0x130BBC);

        // Stuff is in bst->root->prev->value

        auto e = reinterpret_cast<VanillaSaveData*>(saveData);
        if (weSavin)
        {
            weSavin = false;
        }
        saveDetour.UnDetour();
        auto result = saveDetour.GetOriginalFunc()(saveData, edx, file, timestamp, timestampStr);
        saveDetour.Detour(SaveDetour);
        return result;
    }

    std::map<uint, uint> visitValues = {
        {      2626, 65},
        {     21689, 65},
        {     28402, 65},
        {     32354, 65},
        {     33711, 65},
        {     36813, 65},
        {     55219, 65},
        {     57826, 65},
        {2148133827, 41},
        {2151832386, 41},
        {2152084234, 41},
        {2155836289, 41},
        {2172997126, 41},
        {2179466764, 41},
        {2197625223, 41},
        {2197794375, 41},
        {2200697741,  1},
        {2208818767,  1},
        {2209179781, 41},
        {2209190021, 41},
        {2211890816,  1},
        {2219914816, 41},
        {2220981261, 41},
        {2220983309, 41},
        {2239420928, 41},
        {2248709509, 41},
        {2248736133, 41},
        {2248809856, 41},
        {2248948109, 41},
        {2249056653, 41},
        {2260496840, 41},
        {2260502984, 41},
        {2264452224, 41},
        {2269688072,  1},
        {2275469577,  1},
        {2275780875,  1},
        {2275787019,  1},
        {2281715074, 41},
        {2281725314, 41},
        {2282248387, 41},
        {2286036802, 41},
        {2286047042, 41},
        {2286083906, 41},
        {2297978306, 41},
        {2298316232, 41},
        {2299967046, 41},
        {2300079567, 41},
        {2324190592, 45},
        {2327605192, 41},
        {2327615432, 41},
        {2331839879, 41},
        {2334597643, 41},
        {2337537028, 41},
        {2349355715, 41},
        {2353147202, 41},
        {2353153346, 41},
        {2353190210, 41},
        {2355730061, 41},
        {2360482831, 41},
        {2361699650, 41},
        {2361705794, 41},
        {2366627970, 41},
        {2368664909, 41},
        {2378867137, 41},
        {2380966852, 41},
        {2398947207, 41},
        {2401552897, 45},
        {2401693707, 41},
        {2401744907, 41},
        {2409040898,  1},
        {2410509957, 41},
        {2416469187, 41},
        {2418556610, 41},
        {2418562754, 41},
        {2418591426, 41},
        {2418642626, 41},
        {2420147266, 41},
        {2422736708, 41},
        {2438039883, 41},
        {2461839304, 41},
        {2480670600,  1},
        {2480736454, 41},
        {2480758982, 41},
        {2483580611, 41},
        {2485109581, 41},
        {2485673154, 41},
        {2487384386, 41},
        {2487413058, 41},
        {2491031877, 41},
        {2493793743,  1},
        {2494352909, 45},
        {2507118978, 41},
        {2532461388,  1},
        {2532545356,  1},
        {2532571980,  1},
        {2535949323, 41},
        {2544311299, 41},
        {2544362499, 41},
        {2547926531, 41},
        {2548014595, 41},
        {2548021256, 41},
        {2550227469, 41},
        {2550689987, 41},
        {2554335431, 41},
        {2554765066, 41},
        {2559692619, 41},
        {2565826310,  1},
        {2574940160, 41},
        {2574942208, 41},
        {2576689219, 41},
        {2576747148, 41},
        {2593162501, 41},
        {2617337869, 41},
        {2619875522, 41},
        {2619876546, 41},
        {2619922626, 41},
        {2619969730, 41},
        {2626798923, 41},
        {2636585985,  1},
        {2642024960, 41},
        {2642076160, 41},
        {2653466061, 41},
        {2660778562, 30},
        {2661034568, 31},
        {2678523907, 41},
        {2678583299, 41},
        {2682121926, 41},
        {2684475917, 41},
        {2684889027, 41},
        {2691274893, 41},
        {2709428815, 41},
        {2709860870, 41},
        {2734515591, 41},
        {2734529927, 41},
        {2737297931, 41},
        {2745633359,  1},
        {2745678339, 41},
        {2746084485, 41},
        {2751576077, 41},
        {2751582221, 41},
        {2755957258,  1},
        {2757820429, 41},
        {2758345357, 41},
        {2758382221, 41},
        {2762391554,  1},
        {2766674573, 41},
        {2776249856, 41},
        {2776278528, 41},
        {2785601925, 41},
        {2785698176, 41},
        {2785815949, 41},
        {2789809928,  1},
        {2797364680, 41},
        {2797379016, 41},
        {2797401544, 41},
        {2806566152,  1},
        {2812663051,  1},
        {2812784643, 41},
        {2818580866, 41},
        {2818591106, 41},
        {2818879880, 41},
        {2822910786, 41},
        {2822949698, 41},
        {2824936973, 41},
        {2825489549, 41},
        {2825512077, 41},
        {2827478279,  1},
        {2835106248, 41},
        {2843392000, 41},
        {2844683911,  1},
        {2853756098, 41},
        {2864479176, 41},
        {2864481224, 41},
        {2864507848, 41},
        {2868736391, 41},
        {2871512587, 41},
        {2878119873, 41},
        {2879482760,  1},
        {2881203077,  1},
        {2884737347,  1},
        {2885796877, 41},
        {2886260419, 41},
        {2890056002, 41},
        {2892043277, 41},
        {2892477320, 41},
        {2898573634, 41},
        {2931585480, 41},
        {2934678413,  1},
        {2935847815, 41},
        {2937294728, 41},
        {2938612747, 41},
        {2938618891, 41},
        {2947390085, 41},
        {2947412613, 41},
        {2953371843, 41},
        {2955422402, 41},
        {2955459266, 41},
        {2955516610, 41},
        {2957021250, 41},
        {2957047874, 41},
        {2959610692, 41},
        {2959716493, 41},
        {2974901579, 41},
        {2977596416, 41},
        {2998682568, 41},
        {3003235657, 41},
        {3003372872, 41},
        {3017612486, 41},
        {3017663686, 41},
        {3022565570, 41},
        {3026827917, 41},
        {3034444867, 41},
        {3035499597, 41},
        {3041756036,  1},
        {3044707840, 41},
        {3044980815, 41},
        {3068043717, 41},
        {3069337420,  1},
        {3069413196,  1},
        {3081177091, 41},
        {3081213955, 41},
        {3081236483, 41},
        {3084860424, 41},
        {3084880387, 41},
        {3084895240, 41},
        {3087132173, 41},
        {3091235906, 41},
        {3091624714, 41},
        {3093931149, 41},
        {3096519499, 41},
        {3097282184, 41},
        {3104636748, 41},
        {3104867910, 41},
        {3112085071, 41},
        {3112095311, 41},
        {3148318211, 41},
        {3154242573, 41},
        {3156750530, 41},
        {3156751554, 41},
        {3156760770, 41},
        {3156761794, 41},
        {3156780226, 41},
        {3156790466, 41},
        {3160117511, 41},
        {3161042573, 41},
        {3178892800, 41},
        {3179166799, 41},
        {3179195471, 41},
        {3197677122,  1},
        {3197728322,  1},
        {3199256134, 31},
        {3204849734, 41},
        {3207084992,  1},
        {3211513671, 41},
        {3215428611, 41},
        {3215451139, 41},
    };

    std::map<float, std::string> reps = {
        {  0.1,   "li_n_grp"},
        {  0.1, "li_lsf_grp"},
        { 0.65,   "li_p_grp"},
        {    0,   "br_n_grp"},
        { 0.91,   "br_p_grp"},
        {    0,   "ku_n_grp"},
        {    0,   "ku_p_grp"},
        {    0,   "rh_n_grp"},
        {    0,   "rh_p_grp"},
        {    0, "co_alg_grp"},
        {    0,  "co_be_grp"},
        {    0,   "br_m_grp"},
        {    0, "co_nws_grp"},
        {    0, "co_hsp_grp"},
        { 0.65,  "co_ic_grp"},
        { -0.3, "co_khc_grp"},
        { -0.3,  "co_kt_grp"},
        {    0,   "rh_m_grp"},
        { 0.65,  "co_me_grp"},
        { 0.65,  "co_ni_grp"},
        {    0,  "co_os_grp"},
        { -0.3,  "co_rs_grp"},
        { -0.3, "co_shi_grp"},
        { 0.65,  "co_ss_grp"},
        {    0,  "co_ti_grp"},
        { 0.65,  "co_vr_grp"},
        { 0.65,  "fc_bd_grp"},
        { -0.3,   "fc_b_grp"},
        {-0.65,   "fc_c_grp"},
        { -0.3,  "fc_fa_grp"},
        { -0.3,   "fc_g_grp"},
        { -0.3,  "fc_gc_grp"},
        { -0.3,   "fc_h_grp"},
        { -0.3,   "fc_j_grp"},
        {-0.65,  "fc_lh_grp"},
        {-0.65,  "fc_lr_grp"},
        { -0.3, "fc_lwb_grp"},
        {-0.65,   "fc_m_grp"},
        {-0.65,  "fc_ou_grp"},
        {-0.65,  "fc_rh_grp"},
        {    0,  "fc_or_grp"},
        {-0.65,   "fc_u_grp"},
        {-0.65,   "fc_x_grp"},
        {    0,  "gd_gm_grp"},
        {    0,  "fc_uk_grp"},
        {-0.65,   "fc_n_grp"},
        {-0.65,  "fc_ln_grp"},
        {-0.65,  "fc_kn_grp"},
        {-0.65,  "fc_rn_grp"},
        {    0, "fc_ouk_grp"},
        {    0,   "fc_q_grp"},
        {    0,   "fc_f_grp"},
        {    0,  "gd_im_grp"},
        {    0,   "gd_z_grp"},
        { -0.3,  "gd_bh_grp"},
    };

    bool __fastcall LoadDetour(void* unk, void* edx, char* filename, VanillaLoadData* data)
    {
        DWORD server = DWORD(GetModuleHandleA("server.dll"));
        // loadDetour.UnDetour();
        // auto result = loadDetour.GetOriginalFunc()(unk, edx, filename, data);
        // loadDetour.Detour(LoadDetour);
        //  return result;

        data->baseCostume = {
            2223155968, 3144214861, 2479975689, 2264565644, {0, 0, 0, 0, 0, 0, 0, 0},
                0
        };
        data->commCostume = {
            2223155968, 3144214861, 2479975689, 2264565644, {0, 0, 0, 0, 0, 0, 0, 0},
                0
        };
        data->money = 42069;
        data->rank = 69;
        data->baseHullStatus = 0.9f;
        data->hullStatus = 0.8f;
        data->currentBase = 0;
        data->pos = { 30000.0f, 0, -25000.0f };
        data->rot = EulerMatrix({ 0.0f, 0.0f, 0.0f });
        data->datetimeHigh = 0;
        data->datetimeLow = 0;
        data->descripStrId = 0;
        data->name = (unsigned short*)L"FunnyCHaracter";
        data->description = (unsigned short*)L"08/16/23 19:29:11";
        data->interfaceState = 1;
        data->lastDockedBase = CreateID("Li01_01_Base");
        data->currentRoom = 0;
        data->numOfKills = 5;
        data->numOfFailedMissions = 6;
        data->numOfSuccessMissions = 7;
        data->shipHash = CreateID("co_elite2");
        Archetype::GetShip(data->shipHash)->get_undamaged_collision_group_list(data->baseCollisionGroups);
        Archetype::GetShip(data->shipHash)->get_undamaged_collision_group_list(data->currentCollisionGroups);
        data->system = CreateID("li03");
        const char* voice = "trent_voice";
        memcpy(data->voice, voice, strlen(voice) + 1);
        data->voiceLen = 11;
        /*{
            EquipDesc powerplant;
            powerplant.mounted = true;
            powerplant.health = 1;
            powerplant.archId = CreateID("li_elite_power01");
            powerplant.count = 1;
            powerplant.id = 1;
            powerplant.make_internal();

            EquipDesc engine;
            engine.mounted = true;
            engine.health = 1;
            engine.archId = CreateID("ge_le_engine_01");
            engine.count = 1;
            engine.id = 2;
            engine.make_internal();

            EquipDesc scanner;
            scanner.mounted = true;
            scanner.health = 1;
            scanner.archId = CreateID("ge_s_scanner_01");
            scanner.count = 1;
            scanner.id = 34;
            scanner.make_internal();

            EquipDesc tractorBeam;
            tractorBeam.mounted = true;
            tractorBeam.health = 1;
            tractorBeam.archId = CreateID("ge_s_tractor_01");
            tractorBeam.count = 1;
            tractorBeam.id = 35;
            tractorBeam.make_internal();

            EquipDesc shield;
            shield.set_equipped(true);
            const char* shieldHPName = "HpShield01";
            CacheString str((PCHAR)shieldHPName);
            shield.set_hardpoint(str);
            shield.set_status(1.0f);
            shield.set_arch_id(CreateID("shield03_mark04_hf"));

            data->baseEquipAndCargo.push_back(powerplant);
            data->baseEquipAndCargo.push_back(engine);
            data->baseEquipAndCargo.push_back(scanner);
            data->baseEquipAndCargo.push_back(tractorBeam);
            data->baseEquipAndCargo.push_back(shield);

            data->currentEquipAndCargo.push_back(powerplant);
            data->currentEquipAndCargo.push_back(engine);
            data->currentEquipAndCargo.push_back(scanner);
            data->currentEquipAndCargo.push_back(tractorBeam);
            data->currentEquipAndCargo.push_back(shield);
        }*/

        {
            EquipDesc powerplant;

            uint archId = CreateID("li_elite_power01");
            CacheString hardpointName;
            hardpointName.value = (char*)"";
            float health = 1.0f;

            powerplant.set_equipped(true);
            powerplant.set_count(1);
            powerplant.set_arch_id(archId);
            if (strlen(hardpointName.value) == 0)
            {
                powerplant.make_internal();
            }
            else
            {
                powerplant.set_hardpoint(hardpointName);
            }
            if (health != 0.0f)
            {
                powerplant.set_status(health);
            }

            data->baseEquipAndCargo.push_back(powerplant);
            data->currentEquipAndCargo.push_back(powerplant);
        }

        {
            bool isMission = false;
            int count = 5;
            uint archId = CreateID("commodity_gold");
            EquipDesc cargo;
            CacheString hardpointName;
            hardpointName.value = (char*)"";
            float health = 0.5f;

            cargo.set_equipped(false);
            cargo.set_arch_id(archId);
            if (count)
            {
                cargo.set_count(count);
            }
            if (strlen(hardpointName.value) == 0)
            {
                cargo.make_internal();
            }
            else
            {
                cargo.set_hardpoint(hardpointName);
            }
            if (health != 0.0f)
            {
                cargo.set_status(health);
            }
            if (isMission)
            {
                cargo.mission = isMission;
            }

            data->baseEquipAndCargo.push_back(cargo);
            data->currentEquipAndCargo.push_back(cargo);
        }

        {
            float colGrpHitPts = .5f;

            Archetype::Ship* ship = Archetype::GetShip(data->shipHash);

            auto cg = ship->collisionGroup;
            while (cg)
            {
                if (std::string(cg->name.value) == "wing_port_lod1")
                {
                    for (auto& colGrp : data->currentCollisionGroups)
                    {
                        if (colGrp.id == cg->id)
                        {
                            // set health
                            colGrp.health = 0.5f;
                            break;
                        }
                    }
                    break;
                }
                cg = cg->next;
            }
        }

        EquipDesc engine;
        engine.mounted = true;
        engine.health = 1;
        engine.archId = CreateID("ge_le_engine_01");
        engine.count = 1;
        engine.id = 2;
        engine.make_internal();

        EquipDesc scanner;
        scanner.mounted = true;
        scanner.health = 1;
        scanner.archId = CreateID("ge_s_scanner_01");
        scanner.count = 1;
        scanner.id = 34;
        scanner.make_internal();

        EquipDesc tractorBeam;
        tractorBeam.mounted = true;
        tractorBeam.health = 1;
        tractorBeam.archId = CreateID("ge_s_tractor_01");
        tractorBeam.count = 1;
        tractorBeam.id = 35;
        tractorBeam.make_internal();

        data->currentEquipAndCargo.push_back(engine);
        data->currentEquipAndCargo.push_back(tractorBeam);
        data->currentEquipAndCargo.push_back(scanner);
        data->baseEquipAndCargo.push_back(scanner);
        data->baseEquipAndCargo.push_back(tractorBeam);
        data->baseEquipAndCargo.push_back(engine);

        data->tempCargoIdEnumerator = 37;

        for (auto& visit : visitValues)
        {
            using sub_6D5C600Type = int(__fastcall*)(void* ptr, void* null, uint* u, uint* state);
            static auto sub_6D5C600 = sub_6D5C600Type(DWORD(server) + 0x7C600);

            const auto unkThis = (DWORD*)(DWORD(data) + 0x384);
            uint v166[5];
            uint input[2] = { visit.first, visit.second };
            sub_6D5C600(unkThis, edx, v166, input);
            *(v166 + 16) = static_cast<byte>(visit.second);
        }

        for (auto& rep : reps)
        {
            if (rep.second.empty())
            {
                continue;
            }

            struct Input
            {
                    uint hash;
                    float rep;
            };

            Input input = { 0, rep.first };
            TString<16> str;
            str.len = sprintf_s(str.data, "%s", rep.second.c_str());
            input.hash = Reputation::get_id(str);

            using sub_6D58B40Type = int(__fastcall*)(void* ptr, void* null, void* unk1, uint unk2, void* fac);
            static auto sub_6D58B40 = sub_6D58B40Type(DWORD(server) + 0x78B40);
            void* unkThis = PDWORD(DWORD(data) + 704);
            void* unk2 = *(PDWORD*)(DWORD(data) + 712);
            sub_6D58B40(unkThis, edx, unk2, 1, &input);
        }

        memcpy(&loadData, data, sizeof(VanillaLoadData));
        return true;
    }

    std::map<uint, uint> rumours = {
        {393649, 1},
        {393441, 1},
        {131316, 1},
        {393390, 1},
        {393573, 1},
        {131719, 1},
        {393584, 1},
        {131974, 1},
        {393606, 1},
        {393597, 1},
        {131977, 1},
        {131980, 1},
        {393607, 1},
        {393608, 1},
        {131975, 1},
        {393600, 2},
        {393601, 1},
        {393598, 1},
        {393613, 1},
        {393614, 1},
        {393610, 1},
        {393622, 1},
        {132458, 1},
        {393616, 1},
        {132457, 1},
        {393627, 1}
    };

    std::vector<uint> lockedGates = { 2293643521, 2298368769 };

    std::map<uint, uint> tlrExceptions = {
        {2215217857, 2752093889},
        {2752093889, 2215211713},
    };

    std::vector<uint> dockExceptions = { 2312418241 };

    struct VNpc
    {
            enum class NpcMissionStatus
            {
                NotOnAMissionForThisNpc,
                OnAMissionForThisNpc,
                CompletedMissionForThisNpc
            };

            uint baseHash;
            uint npcHash;
            uint interactionCount;
            NpcMissionStatus missionStatus = NpcMissionStatus::NotOnAMissionForThisNpc;
    };

    std::vector<VNpc> visitedNpcs = {
        {2809674765, 2873717645, 2, VNpc::NpcMissionStatus::CompletedMissionForThisNpc},
        {2809674765, 2635402251, 1,       VNpc::NpcMissionStatus::OnAMissionForThisNpc},
        {2809674765, 2554918656, 1,    VNpc::NpcMissionStatus::NotOnAMissionForThisNpc},
    };

    std::map<uint, uint> shipTypesKilled = {
        {2248965056,  5},
        {2806013121, 99},
        {2917601664,  3}
    };

    std::map<uint, uint> missionsFailed = {
        {0, 1},
        {2, 1},
        {3, 1},
        {4, 1},
        {5, 1},
        {7, 1},
    };

    std::map<uint, uint> missionsCompleted = {
        {0, 1},
        {2, 1},
        {3, 1},
        {4, 1},
        {7, 1},
    };

    std::map<uint, uint> missionsAborted = {
        {0, 1},
        {2, 1},
        {3, 1},
        {4, 1},
    };

    std::vector<uint> systemsVisited = { 2208796239, 2208818767, 2745633359, 2409040898 };
    std::vector<uint> basesVisited = { 2804956685, 2725952648, 2812574479 };
    std::vector<uint> holesVisited = { 2879482760, 2200697741, 2881203077, 2934678413 };

    void __fastcall MDataLoadDetour(DWORD* data, void* edx, INI_Reader* reader)
    {
        // mPlayerReadDetour->UnDetour();
        // mPlayerReadDetour->GetOriginalFunc()(data, edx, reader);
        // mPlayerReadDetour->Detour(MDataDetour);

        *PDWORD(DWORD(data) + 33) = true; // CAN DOCK
        *PDWORD(DWORD(data) + 52) = true; // CAN TL

        struct Container
        {
                uint id;
                uint state;
        };

        using sub_6F96770Type = PDWORD(__fastcall*)(DWORD * data, void* null, void* unk, uint unk2, void*);
        static sub_6F96770Type sub_6F96770 = sub_6F96770Type(DWORD(GetModuleHandleA("content.dll")) + 0xF6770);

        for (auto& rumour : rumours)
        {
            DWORD* v12 = *(DWORD**)(DWORD(data) + 260);
            DWORD* _this = (DWORD*)(DWORD(data) + 252);
            Container c = { rumour.first, rumour.second };
            sub_6F96770(_this, edx, v12, 1, &c);
        }

        for (auto gate : lockedGates)
        {
            using sub_6F6D400Type = PDWORD (*)(DWORD* a1, uint* hash);
            static sub_6F6D400Type sub_6F6D400 = sub_6F6D400Type(DWORD(GetModuleHandleA("content.dll")) + 0xCD400);
            DWORD unk = *(DWORD*)(DWORD(data) + 12);
            sub_6F6D400(&unk, &gate);
        }

        for (auto ring : tlrExceptions)
        {
            DWORD* _this = (DWORD*)(DWORD(data) + 56);
            DWORD* unk = *(DWORD**)(DWORD(_this) + 8);

            Container c = { ring.first, ring.second };
            sub_6F96770(_this, edx, unk, 1, &c);
        }

        for (auto dockException : dockExceptions)
        {
            DWORD* _this = (DWORD*)(DWORD(data) + 36);
            DWORD* unk = *(DWORD**)(DWORD(_this) + 8);

            static sub_6F96770Type sub_6F937C0 = sub_6F96770Type(DWORD(GetModuleHandleA("content.dll")) + 0xF6770);
            sub_6F937C0(_this, edx, unk, 1, &dockException);
        }

        for (auto& npc : visitedNpcs)
        {
            auto* _this = (DWORD*)(DWORD(data) + 236);
            DWORD* unk = *(DWORD**)(DWORD(_this) + 8);
            static sub_6F96770Type sub_6F69D50 = sub_6F96770Type(DWORD(GetModuleHandleA("content.dll")) + 0xC9D50);
            sub_6F69D50(_this, edx, unk, 1, &npc);
        }

        auto* _this = (DWORD*)(DWORD(data) + 72);
        for (auto [fst, snd] : shipTypesKilled)
        {
            using sub_6F003E0Type = PDWORD(__fastcall*)(void* unk, void* null, uint* returnData, uint* hashAndCount);
            static sub_6F003E0Type sub_6F003E0 = sub_6F003E0Type(DWORD(GetModuleHandleA("content.dll")) + 0x603E0);

            uint returnData[2]{ 1, 0x1 };
            uint hashAndCount[2] = { fst, snd };
            sub_6F003E0(_this, edx, returnData, hashAndCount);
        }

        using sub_6F4A140Type = PDWORD(__fastcall*)(void* unk, void* null, uint* returnData, void* pair);
        static sub_6F4A140Type sub_6F4A140 = sub_6F4A140Type(DWORD(GetModuleHandleA("content.dll")) + 0xAA140);

        for (auto& complete : missionsCompleted)
        {
            auto* _otherThis = (DWORD*)(DWORD(_this) + 0x14);
            uint ret[2];

            sub_6F4A140(_otherThis, edx, ret, &complete);
        }

        for (auto& aborted : missionsAborted)
        {
            auto* _otherThis = (DWORD*)(DWORD(_this) + 0x28);
            uint ret[2];
            sub_6F4A140(_otherThis, edx, ret, &aborted);
        }

        for (auto& failed : missionsFailed)
        {
            auto* _otherThis = (DWORD*)(DWORD(_this) + 0x3C);
            uint ret[2];

            sub_6F4A140(_otherThis, edx, ret, &failed);
        }

        *(PFLOAT)(DWORD(_this) + 80) = 66666.f;       // Total cashed earned (as a float for some reason)
        *(PFLOAT)(DWORD(_this) + 84) = 30.0f * 60.0f; // Total time played in seconds

        for (auto& hole : holesVisited)
        {
            using sub_6F95E00Type = PDWORD(__fastcall*)(void* unk, void* null, uint* returnData, void* pair);
            static sub_6F95E00Type sub_6F95E00 = sub_6F95E00Type(DWORD(GetModuleHandleA("content.dll")) + 0xF5E00);

            auto* _otherThis = (DWORD*)(DWORD(_this) + 0x78);
            uint ret[2];

            sub_6F95E00(_otherThis, edx, ret, &hole);
        }

        using sub_6EB5770Type = PDWORD(__fastcall*)(void* unk, void* null, PDWORD unk2, uint* hash);
        static sub_6EB5770Type sub_6EB5770 = sub_6EB5770Type(DWORD(GetModuleHandleA("content.dll")) + 0x15770);

        using sub_6EB5F30Type = PDWORD(__fastcall*)(void* unk, void* null, uint* hash);
        static sub_6EB5F30Type sub_6EB5F30 = sub_6EB5F30Type(DWORD(GetModuleHandleA("content.dll")) + 0x15F30);

        using sub_6FA6E80Type = PDWORD(__fastcall*)(void* unk, void* null, void* unk2, byte* hash);
        static sub_6FA6E80Type sub_6FA6E80 = sub_6FA6E80Type(DWORD(GetModuleHandleA("content.dll")) + 0x106E80);

        for (auto& base : basesVisited)
        {
            auto* _otherThis = (DWORD*)(DWORD(_this) + 0x68);

            PDWORD ret = sub_6EB5F30(_otherThis, edx, &base);
            auto unk = *(PDWORD*)(DWORD(_this) + 112);
            if (ret == unk)
            {
                sub_6EB5770(_otherThis, edx, unk, &base);
                sub_6EB5F30(_otherThis, edx, &base);
            }
        }

        for (auto& system : systemsVisited)
        {
            auto* _otherThis = (DWORD*)(DWORD(_this) + 88);

            uint input[2] = { system, 0 };
            auto v11 = *(DWORD**)(DWORD(_this) + 92);
            auto v12 = *(DWORD**)(DWORD(_this) + 96);

            if (v11 == v12)
            {
                v11 = v12;
            }
            else
            {
                while (*v11 != system)
                {
                    if (++v11 == v12)
                    {
                        v11 = v12;
                        break;
                    }
                }
            }

            auto v16 = v11;
            if (v11 == v12)
            {
                sub_6EB5770(_otherThis, edx, v12, input);
                sub_6EB5F30(_otherThis, edx, input);
            }
            else
            {
                byte null = 0;
                sub_6FA6E80(_otherThis, edx, &v16, &null);
            }
        }
    }

    int* __fastcall MDataSaveDetour(MPlayerDataSaveStruct* mData, void* edx, FILE* stream)
    {
        auto a = sizeof(st6::vector<uint>);
        auto aa = sizeof(st6::list<uint>);
        auto aaa = sizeof(st6::map<uint, uint>);
        auto start = *PDWORD(DWORD(mData) + 240);
        auto end = *PDWORD(DWORD(mData) + 244);

        std::vector<VNpc> vnpcData;
        while (start != end)
        {
            vnpcData.push_back(*reinterpret_cast<VNpc*>(start));
            start += sizeof(VNpc);
        }

        struct Rumor
        {
                uint IDS;
                uint unknown; // number of times read?
        };
        start = *PDWORD(DWORD(mData) + 256);
        end = *PDWORD(DWORD(mData) + 260);

        std::vector<Rumor> rumorData;
        while (start != end)
        {
            rumorData.push_back(*reinterpret_cast<Rumor*>(start));
            start += sizeof(Rumor);
        }

        bool can_dock = *PDWORD(DWORD(mData) + 33);

        start = *PDWORD(DWORD(mData) + 40);
        end = *PDWORD(DWORD(mData) + 44);

        std::vector<uint> dockExceptions;
        while (start != end)
        {
            dockExceptions.push_back(*reinterpret_cast<uint*>(start));
            start += sizeof(uint);
        }

        bool can_tl = *PDWORD(DWORD(mData) + 52);

        struct TradeLaneException
        {
                uint entryLaneId;
                uint nextLaneId;
        };

        start = *PDWORD(DWORD(mData) + 60);
        end = *PDWORD(DWORD(mData) + 64);

        std::vector<TradeLaneException> tlExceptions;
        while (start != end)
        {
            tlExceptions.push_back(*reinterpret_cast<TradeLaneException*>(start));
            start += sizeof(TradeLaneException);
        }

        auto* extendedMData = (DWORD*)(DWORD(mData) + 0x48);

        auto iterNode = mData->BST_killed_ships.begin();
        std::vector<std::pair<uint, uint>> killedShips;

        /*for (BinarySearchTree<uint>::Iterator node : mData->BST_killed_ships)
        {
            killedShips.emplace_back(*node.key(), *node.value());
        }*/

        /*BinarySearchTree<uint>* rmCompleted = reinterpret_cast<BinarySearchTree<uint>*>(DWORD(extendedMData) + 24);
        iterNode = rmCompleted->root->prev;
        std::vector<std::pair<uint, uint>> completedMissions;
        while (iterNode != rmCompleted->root)
        {
            completedMissions.emplace_back(std::make_pair(iterNode->key, iterNode->value));
            iterNode = iterNode->Traverse();
        }

        BinarySearchTree<uint>* rmAborted = reinterpret_cast<BinarySearchTree<uint>*>(DWORD(extendedMData) + 44);
        iterNode = rmAborted->root->prev;
        std::vector<std::pair<uint, uint>> abortedMissions;
        while (iterNode != rmAborted->root)
        {
            abortedMissions.emplace_back(std::make_pair(iterNode->key, iterNode->value));
            iterNode = iterNode->Traverse();
        }

        BinarySearchTree<uint>* rmFailed = reinterpret_cast<BinarySearchTree<uint>*>(DWORD(extendedMData) + 64);
        iterNode = rmFailed->->prev;
        std::vector<std::pair<uint, uint>> failedMissions;
        while (iterNode != rmFailed->root)
        {
            failedMissions.emplace_back(std::make_pair(iterNode->key, iterNode->value));
            iterNode = iterNode->Traverse();
        }*/

        float totalCashEarned = *PFLOAT(DWORD(extendedMData) + 80);
        float totalTimePlayed = *PFLOAT(DWORD(extendedMData) + 84);

        start = *PDWORD(DWORD(extendedMData) + 92);
        end = *PDWORD(DWORD(extendedMData) + 96);

        std::vector<uint> visitedSystems;
        while (start != end)
        {
            visitedSystems.push_back(*reinterpret_cast<uint*>(start));
            start += sizeof(uint);
        }

        start = *PDWORD(DWORD(extendedMData) + 108);
        end = *PDWORD(DWORD(extendedMData) + 112);

        std::vector<uint> visitedBases;
        while (start != end)
        {
            visitedBases.push_back(*reinterpret_cast<uint*>(start));
            start += sizeof(uint);
        }

        start = *PDWORD(DWORD(extendedMData) + 124);
        end = *PDWORD(DWORD(extendedMData) + 128);

        std::vector<uint> visitedHoles;
        while (start != end)
        {
            visitedHoles.push_back(*reinterpret_cast<uint*>(start));
            start += sizeof(uint);
        }

        mPlayerSaveDetour->UnDetour();
        auto result = mPlayerSaveDetour->GetOriginalFunc()(mData, edx, stream);
        mPlayerSaveDetour->Detour(MDataSaveDetour);
        return result;
    }

    std::array<CAccount*, 256> accounts;

    AfkPlugin::AfkPlugin(const PluginInfo& info) : Plugin(info) {}

    void AfkPlugin::OnServerStartupAfter(const SStartupInfo& info)
    {
        /*mPlayerReadDetour =
            std::make_unique<FunctionDetour<OnCharacterMPlayerSectionRead>>(OnCharacterMPlayerSectionRead(DWORD(GetModuleHandleA("content.dll")) + 0xA83D0));
        mPlayerSaveDetour =
            std::make_unique<FunctionDetour<OnCharacterMPlayerSectionSave>>(OnCharacterMPlayerSectionSave(DWORD(GetModuleHandleA("content.dll")) + 0xA81F0));

        saveDetour.Detour(SaveDetour);
        loadDetour.Detour(LoadDetour);
        mPlayerReadDetour->Detour(MDataLoadDetour);
        mPlayerSaveDetour->Detour(MDataSaveDetour);*/
    }
    /** @ingroup AwayFromKeyboard
     * @brief This command is called when a player types /afk. It prints a message in red text to nearby players saying they are afk. It will also let anyone
     * who messages them know too.
     */
    void AfkPlugin::UserCmdAfk()
    {
        awayClients.emplace_back(userCmdClient);
        const auto playerName = userCmdClient.GetCharacterName().Handle();
        const auto message = std::format(L"{} is now away from keyboard.", playerName);

        const auto system = userCmdClient.GetSystemId().Handle();
        system.Message(message, MessageColor::Red, MessageFormat::Normal);

        userCmdClient.Message(L"Use the /back command to stop sending automatic replies to PMs.");
    }

    /** @ingroup AwayFromKeyboard
     * @brief This command is called when a player types /back. It removes the afk status and welcomes the player back.
     * who messages them know too.
     */
    void AfkPlugin::UserCmdBack()
    {
        if (const auto it = awayClients.begin(); std::find(it, awayClients.end(), userCmdClient) != awayClients.end())
        {
            const auto system = userCmdClient.GetSystemId().Handle();

            awayClients.erase(it);

            auto playerName = userCmdClient.GetCharacterName().Handle();

            system.Message(std::format(L"{} has returned.", playerName), MessageColor::Red);
        }
    }

    // Clean up when a client disconnects

    // Hook on chat being sent (This gets called twice with the client and to swapped
    void AfkPlugin::OnSendChat(ClientId client, ClientId targetClient, [[maybe_unused]] const uint size, [[maybe_unused]] void* rdl)
    {
        if (std::ranges::find(awayClients, targetClient) != awayClients.end())
        {
            client.Message(L"This user is away from keyboard.");
        }
    }

    // Hooks on chat being submitted
    void AfkPlugin::OnSubmitChat(ClientId triggeringClient, [[maybe_unused]] const unsigned long lP1, [[maybe_unused]] const void* rdlReader,
                                 [[maybe_unused]] ClientId to, [[maybe_unused]] const int dunno)
    {
        if (const auto it = awayClients.begin(); triggeringClient && std::find(it, awayClients.end(), triggeringClient) != awayClients.end())
        {
            userCmdClient = triggeringClient;
            UserCmdBack();
        }
    }

    void AfkPlugin::OnClearClientInfo(ClientId client)
    {
        auto [first, last] = std::ranges::remove(awayClients, client);
        awayClients.erase(first, last);
    }

    // Client command processing
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"AFK", L"afk", PluginMajorVersion::V04, PluginMinorVersion::V01);
SetupPlugin(AfkPlugin, Info);
