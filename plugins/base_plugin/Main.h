#ifndef __MAIN_H__
#define __MAIN_H__ 1

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
using namespace std;

struct RECIPE {
    RECIPE() : produced_item(0), cooking_rate(0) {}
    uint nickname;
    uint produced_item;
    wstring infotext;
    uint cooking_rate;
    map<uint, uint> consumed_items;
};

struct MARKET_ITEM {
    MARKET_ITEM()
        : quantity(0), price(1.0f), min_stock(100000), max_stock(100000) {}

    // Number of units of commodity stored in this base
    uint quantity;

    // Buy/Sell price for commodity.
    float price;

    // Stop selling if the base holds less than this number of items
    uint min_stock;

    // Stop buying if the base holds more than this number of items
    uint max_stock;
};

struct NEWS_ITEM {
    wstring headline;
    wstring text;
};

class PlayerBase;

class Module {
  public:
    int type;
    static const int TYPE_BUILD = 0;
    static const int TYPE_CORE = 1;
    static const int TYPE_SHIELDGEN = 2;
    static const int TYPE_STORAGE = 3;
    static const int TYPE_DEFENSE_1 = 4;
    static const int TYPE_M_DOCKING = 5;
    static const int TYPE_M_JUMPDRIVES = 6;
    static const int TYPE_M_HYPERSPACE_SCANNER = 7;
    static const int TYPE_M_CLOAK = 8;
    static const int TYPE_DEFENSE_2 = 9;
    static const int TYPE_DEFENSE_3 = 10;
    static const int TYPE_LAST = TYPE_DEFENSE_3;

    Module(uint the_type) : type(the_type) {}
    virtual ~Module() {}
    virtual void Spawn() {}
    virtual wstring GetInfo(bool xml) = 0;
    virtual void LoadState(INI_Reader &ini) = 0;
    virtual void SaveState(FILE *file) = 0;

    virtual bool Timer(uint time) { return false; }

    virtual float SpaceObjDamaged(uint space_obj, uint attacking_space_obj,
                                  float curr_hitpoints, float damage) {
        return 0.0f;
    }
    virtual bool SpaceObjDestroyed(uint space_obj) { return false; }
    virtual void SetReputation(int player_rep, float attitude) {}
};

class CoreModule : public Module {
  public:
    PlayerBase *base;

    // The space ID of this base
    uint space_obj;

    // If true, do not use food and commodities
    bool dont_eat;

    // If true, do not take damage
    bool dont_rust;

    // The list of goods and usage of goods per minute for the autosys effect
    map<uint, uint> mapAutosysGood;

    // The list of goods and usage of goods per minute for the autosys effect
    map<uint, uint> mapHumansysGood;

    CoreModule(PlayerBase *the_base);
    ~CoreModule();
    void Spawn();
    wstring GetInfo(bool xml);

    void LoadState(INI_Reader &ini);
    void SaveState(FILE *file);

    bool Timer(uint time);
    float SpaceObjDamaged(uint space_obj, uint attacking_space_obj,
                          float curr_hitpoints, float damage);
    bool SpaceObjDestroyed(uint space_obj);
    void SetReputation(int player_rep, float attitude);

    void RepairDamage(float max_base_health);
};

class ShieldModule : public Module {
  public:
    PlayerBase *base;

    // If true then a player has entered the system and so we reset the fuse
    // so that they see the shield
    bool reset_needed;

    ShieldModule(PlayerBase *the_base);
    ~ShieldModule();
    wstring GetInfo(bool xml);

    void LoadState(INI_Reader &ini);
    void SaveState(FILE *file);

    bool Timer(uint time);
    void SetReputation(int player_rep, float attitude);

    bool HasShieldPower();
};

class StorageModule : public Module {
  public:
    PlayerBase *base;

    StorageModule(PlayerBase *the_base);
    ~StorageModule();
    wstring GetInfo(bool xml);

    void LoadState(INI_Reader &ini);
    void SaveState(FILE *file);
};

class DefenseModule : public Module {
  public:
    PlayerBase *base;

    // The space object of the platform
    uint space_obj;

    // The position of the platform
    Vector pos;

    // The orientation of the platform
    Vector rot;

    DefenseModule(PlayerBase *the_base);
    DefenseModule(PlayerBase *the_base, uint the_type);
    ~DefenseModule();
    wstring GetInfo(bool xml);

    void LoadState(INI_Reader &ini);
    void SaveState(FILE *file);

    bool Timer(uint time);
    float SpaceObjDamaged(uint space_obj, uint attacking_space_obj,
                          float curr_hitpoints, float damage);
    bool SpaceObjDestroyed(uint space_obj);
    void SetReputation(int player_rep, float attitude);
    void Reset();
};

class BuildModule : public Module {
  public:
    PlayerBase *base;

    int build_type;

    RECIPE active_recipe;

    BuildModule(PlayerBase *the_base);
    BuildModule(PlayerBase *the_base, uint the_building_type);

    wstring GetInfo(bool xml);

    void LoadState(INI_Reader &ini);
    void SaveState(FILE *file);

    bool Timer(uint time);
};

class FactoryModule : public Module {
  public:
    PlayerBase *base;

    // The currently active recipe
    RECIPE active_recipe;

    // List of queued recipes;
    list<uint> build_queue;

    FactoryModule(PlayerBase *the_base);
    FactoryModule(PlayerBase *the_base, uint type);
    wstring GetInfo(bool xml);
    void LoadState(INI_Reader &ini);
    void SaveState(FILE *file);
    bool Timer(uint time);

    bool AddToQueue(uint the_equipment_type);
    bool ClearQueue();
};

class PlayerBase {
  public:
    PlayerBase(uint client, const wstring &password, const wstring &basename);
    PlayerBase(const string &path);
    ~PlayerBase();

    void Spawn();

    bool Timer(uint curr_time);

    void SetupDefaults();
    void Load();
    void Save();

    bool AddMarketGood(uint good, uint quantity);
    void RemoveMarketGood(uint good, uint quantity);
    void ChangeMoney(INT64 quantity);
    uint GetRemainingCargoSpace();
    uint GetMaxCargoSpace();
    uint HasMarketItem(uint good);

    static string CreateBaseNickname(const string &basename);

    float GetAttitudeTowardsClient(uint client);
    void SyncReputationForBase();
    void SyncReputationForBaseObject(uint space_obj);

    float SpaceObjDamaged(uint space_obj, uint attacking_space_obj,
                          float curr_hitpoints, float damage);

    // The base nickname
    string nickname;

    // The base affiliation
    uint affiliation;

    // The name of the base shown to other players
    wstring basename;

    // The infocard for the base
    wstring infocard;

    // The infocard paragraphs for the base
#define MAX_CHARACTERS 500
#define MAX_PARAGRAPHS 5
    wstring infocard_para[MAX_PARAGRAPHS + 1];

    // The system the base is in
    uint system;

    // The position of the base
    Vector position;

    // The rotation of the base
    Matrix rotation;

    // The basic armour and commodity storage available on this base->
    uint base_level;

    // The commodities carried by this base->
    map<uint, MARKET_ITEM> market_items;

    // The money this base has
    INT64 money;

    // The current hit points of the base
    float base_health;

    // The maximum hit points of the base
    float max_base_health;

    // When the base is spawned, this is the IDS of the base name
    uint solar_ids;

    // The ingame hash of the nickname
    uint base;

    // The list of administration passwords
    list<wstring> passwords;

    // If 0 then base is neutral to all ships. Only ships on the ally tag list
    // may dock. If 1 then base is hostile to all ships unless they are on the
    // ally tag list. If 2 then base is neutral to all ships and any ship may
    // dock.
    int defense_mode;

    // List of allied ship tags.
    list<wstring> ally_tags;

    // List of ships that are hostile to this base
    map<wstring, wstring> hostile_tags;

    // Modules for base
    vector<Module *> modules;

    // Path to base ini file.
    string path;

    // The proxy base associated with the system this base is in.
    uint proxy_base;

    // If true, the base should not take any damage
    bool invulnerable;

    // if true, the base was repaired or is able to be repaired
    bool repairing;

    // The state of the shield
    static const int SHIELD_STATE_OFFLINE = 0;
    static const int SHIELD_STATE_ONLINE = 1;
    static const int SHIELD_STATE_ACTIVE = 2;
    int shield_state;

    // The number of seconds that shield will be active
    int shield_active_time;

    // When this timer drops to less than 0 the base is saved
    int save_timer;
};

PlayerBase *GetPlayerBase(uint base);
PlayerBase *GetPlayerBaseForClient(uint client);

void SaveBases();
void DeleteBase(PlayerBase *base);
void LoadDockState(uint client);
void SaveDockState(uint client);
void DeleteDockState(uint client);

/// Send a command to the client at destination ID 0x9999
void SendCommand(uint client, const wstring &message);
void SendSetBaseInfoText(uint client, const wstring &message);
void SendSetBaseInfoText2(uint client, const wstring &message);
void SendResetMarketOverride(uint client);
void SendMarketGoodUpdated(PlayerBase *base, uint good, MARKET_ITEM &item);
void SendMarketGoodSync(PlayerBase *base, uint client);
void SendBaseStatus(uint client, PlayerBase *base);
void SendBaseStatus(PlayerBase *base);
void ForceLaunch(uint client);

struct CLIENT_DATA {
    CLIENT_DATA()
        : reverse_sell(false), stop_buy(false), admin(false), player_base(0),
          last_player_base(0) {}

    // If true reverse the last sell by readding the item.
    bool reverse_sell;

    // The cargo list used by the reverse sell.
    list<CARGO_INFO> cargo;

    // If true block the current buy and associated reqitemadd function.
    bool stop_buy;

    // True if this player is the base administrator.
    bool admin;

    // Set to player base hash if ship is in base-> 0 if not.
    uint player_base;

    // Set to player base hash if ship is in base or was last in a player base->
    // 0 after docking at any non player base->
    uint last_player_base;
};

namespace PlayerCommands {
void BaseHelp(uint client, const wstring &args);

void BaseLogin(uint client, const wstring &args);
void BaseAddPwd(uint client, const wstring &args);
void BaseRmPwd(uint client, const wstring &args);
void BaseLstPwd(uint client, const wstring &args);
void BaseSetMasterPwd(uint client, const wstring &args);

void BaseAddAllyTag(uint client, const wstring &args);
void BaseRmAllyTag(uint client, const wstring &args);
void BaseLstAllyTag(uint client, const wstring &args);
void BaseRep(uint client, const wstring &args);

void BaseInfo(uint client, const wstring &args);
void BaseDefenseMode(uint client, const wstring &args);
void BaseDefMod(uint client, const wstring &args);
void BaseBuildMod(uint client, const wstring &args);
void BaseFacMod(uint client, const wstring &args);
void BaseShieldMod(uint client, const wstring &args);
void Bank(uint client, const wstring &args);
void Shop(uint client, const wstring &args);

void BaseDeploy(uint client, const wstring &args);
} // namespace PlayerCommands

extern map<uint, CLIENT_DATA> clients;

extern map<uint, Module *> spaceobj_modules;

// Map of ingame hash to info
extern map<uint, class PlayerBase *> player_bases;

extern int set_plugin_debug;

extern map<uint, RECIPE> recipes;

struct REPAIR_ITEM {
    uint good;
    uint quantity;
};
extern list<REPAIR_ITEM> set_base_repair_items;

extern uint set_base_crew_type;

extern map<uint, uint> set_base_crew_consumption_items;

extern map<uint, uint> set_base_crew_food_items;

/// The ship used to construct and upgrade bases
extern uint set_construction_shiparch;

/// Map of good to quantity for items required by construction ship
extern map<uint, uint> construction_items;

/// Map of item nickname hash to recipes to operate shield.
extern map<uint, uint> shield_power_items;

/// Damage to the base every 10 seconds
extern uint set_damage_per_10sec;

/// Damage to the base every tick
extern uint set_damage_per_tick;

/// The seconds per tick
extern uint set_tick_time;

/// If the shield is up then damage to the base is changed by this multiplier.
extern float set_shield_damage_multiplier;

wstring HtmlEncode(wstring text);

#endif
