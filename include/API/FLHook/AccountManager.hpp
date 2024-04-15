#pragma once

#include "Defs/Database/Account.hpp"
#include "Utils/Detour.hpp"

// TODO: Reputation list and affiliation needs to be figured.
struct Rumor
{
        uint IDS;
        uint unknown; // number of times read?
};

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
        int interactionCount;
        NpcMissionStatus missionStatus = NpcMissionStatus::NotOnAMissionForThisNpc;
};

struct TLException
{
        uint startRing;
        uint nextRing;
};

struct MPlayerDataSaveStruct
{
        uint unkPtr1;                        // 0
        uint clientId1;                      // 4
        uint unkInt1;                        // 8
        uint clientId2;                      // 12
        uint padding4;                       // 16
        uint padding5;                       // 20
        uint unkPtr2;                        // 24
        uint unkPtr3;                        // 28
        bool padding9;                       // 32
        bool canDock;                        // 33
        uint canDock2;                       // 36
        st6::list<uint> dockExceptions;      // 40
        bool canTL;                          // 52
        uint padding51;                      // 56
        st6::list<TLException> tlExceptions; // 60
        FlMap<uint> killedShips;             // 72
        FlMap<uint> rmCompleted;             // 92
        FlMap<uint> rmAborted;               // 112
        FlMap<uint> rmFailed;                // 132
        float totalCashEarned;               // 156
        float totalTimePlayed;               // 160
        st6::vector<uint> visitedSystems;    // 164
        st6::vector<uint> visitedBases;      // 180
        st6::vector<uint> visitedHoles;      // 196
        uint padding52;                      // 208
        uint padding53;                      // 212
        uint padding54;                      // 216
        uint padding55;                      // 220 // recalculates rank when equal 42?
        uint padding56;                      // 224
        uint padding57;                      // 228
        uint padding58;                      // 232
        st6::vector<VNpc> visitedNPCs;       // 236
        st6::vector<Rumor> receivedRumors;   // 252
};

struct VanillaLoadData
{
        uint padding[128];
        st6::wstring name;                                    // 512
        st6::wstring description;                             // 528
        uint descripStrId;                                    // 544
        uint datetimeHigh;                                    // 548
        uint datetimeLow;                                     // 552
        uint shipHash;                                        // 556
        int money;                                            // 560
        int numOfKills;                                       // 564
        int numOfSuccessMissions;                             // 568
        int numOfFailedMissions;                              // 572
        float hullStatus;                                     // 576
        st6::list<EquipDesc> currentEquipAndCargo;            // 580
        st6::list<CollisionGroupDesc> currentCollisionGroups; // 592
        float baseHullStatus;                                 // 604
        st6::list<EquipDesc> baseEquipAndCargo;               // 608
        st6::list<CollisionGroupDesc> baseCollisionGroups;    // 620
        uint currentBase;                                     // 632
        uint lastDockedBase;                                  // 636
        uint currentRoom;                                     // 640
        uint system;                                          // 644
        Vector pos;                                           // 648 - 656
        Matrix rot;                                           // 660 - 692
        uint startingRing;                                    // 696
        int rank;                                             // 700
        st6::vector<Reputation::Relation> repList;            // 704
        uint affiliation;                                     // 720, see Reputation::get_id();
        Costume commCostume;                                  // 724 - 772
        uint voiceLen;                                        // 776
        char voice[32] = "trent_voice";                       // 780
        Costume baseCostume;                                  // 812 - 860
        SubObjectID::EquipIdMaker equipIdEnumerator;          // 864
        st6::string prefilledWeaponGroupIni;                  // 876
        st6::list<uint> logInfo;                              // 888
        int interfaceState = 3;                               // 896
        FlMap<char> visits;                                   // 900
        // uint unused2;                                         // 918
        // FlMap<VisitEntry> visitLists;                         // 922
        //  934

        void SetRelation(Reputation::Relation relation);
};

struct NewPlayerTemplate
{
        std::string initialRep;
        int rank = 0;
        std::optional<int> money = 0;
        bool canDock = true;
        bool canTradeLane = true;
        std::string costume;
        std::string commCostume;
        std::string system;
        std::string base;
        std::unordered_map<std::string, float> reputationOverrides;
        std::unordered_map<uint, uint> visitValues;
        bool hasPackage = false;

        std::vector<EquipDesc> equipment;
        std::vector<EquipDesc> cargo;

        uint ship = 0;
};

struct AccountData
{
        CAccount* internalAccount;
        Account account;
        std::unordered_map<std::string, Character> characters;
        bool loginSuccess;
};

struct PlayerDbLoadUserDataAssembly final : Xbyak::CodeGenerator
{
        PlayerDbLoadUserDataAssembly();

    private:
        static void* GetInternalCall();
};

class AccountManager
{
        friend PlayerDbLoadUserDataAssembly;
        friend FLHook;
        friend IServerImplHook;
        friend Database;
        inline static AccountManager* instance = nullptr;

        inline static NewPlayerTemplate newPlayerTemplate;

        inline static std::array<AccountData, 256> accounts;

        enum class LoginReturnCode
        {
            Banned,
            AlreadyLoggedIn,
            InvalidUsernamePassword,
            Success
        };

        static LoginReturnCode __stdcall AccountLoginInternal(PlayerData* data, uint clientId);
        static void LoadNewPlayerFLInfo();

        // Detours/Assembly
        using DbInitType = void(__fastcall*)(PlayerDB* db, void* edx, uint unk, bool unk2);
        using LoadMDataType = void(__fastcall*)(MPlayerDataSaveStruct* mdata, void* edx, struct INI_Reader* ini);
        using OnPlayerSaveType = bool(__fastcall*)(PlayerData* data);
        using OnCreateNewCharacterType = bool(__fastcall*)(PlayerData* data, void* edx, SCreateCharacterInfo* character);

        inline static std::unique_ptr<FunctionDetour<DbInitType>> dbInitDetour;
        inline static std::unique_ptr<FunctionDetour<LoadMDataType>> loadPlayerMDataDetour;
        inline static std::unique_ptr<FunctionDetour<OnPlayerSaveType>> onPlayerSaveDetour;
        inline static std::unique_ptr<FunctionDetour<OnCreateNewCharacterType>> onCreateNewCharacterDetour;
        bool static __fastcall OnCreateNewCharacter(PlayerData* data, void* edx, SCreateCharacterInfo* characterInfo);
        bool static __fastcall OnPlayerSave(PlayerData* pd);
        void static __fastcall PlayerDbInitDetour(PlayerDB* db, void* edx, uint unk, bool unk2);
        void static __fastcall CreateAccountInitFromFolderBypass(CAccount* account, void* edx, char* dir);
        inline static GetFLNameT getFlName;

        PlayerDbLoadUserDataAssembly loadUserDataAssembly;

        AccountManager();

    public:
        static bool SaveCharacter(Character& newCharacter, bool isNewCharacter);
        static bool DeleteCharacter(ClientId clientId, const std::wstring& characterCode);
        static void Login(const std::wstring& wideAccountId, ClientId client);
        static void ClearClientInfo(ClientId clientId);
        static void __fastcall LoadPlayerMData(MPlayerDataSaveStruct* mdata, void* edx, struct INI_Reader* ini);
        static void InitContentDLLDetours();
};
