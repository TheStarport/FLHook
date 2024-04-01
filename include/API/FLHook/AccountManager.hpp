#pragma once

#include "Defs/Database/Account.hpp"
#include "Utils/Detour.hpp"

// TODO: Reputation list and affiliation needs to be figured.
struct VanillaLoadData
{
        uint padding[128];
        st6::wstring name;                                    // 512
        st6::wstring description;                             // 528
        uint descripStrId;                                    // 544
        uint datetimeHigh;                                    // 548
        uint datetimeLow;                                     // 552
        uint shipHash;                                        // 556
        int money;                                           // 560
        int numOfKills;                                      // 564
        int numOfSuccessMissions;                            // 568
        int numOfFailedMissions;                             // 572
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
        int rank;                                            // 700
        st6::vector<Reputation::Relation> repList;            // 704
        uint reputationId;                                    // 720, see Reputation::get_id();
        Costume commCostume;                                  // 724 - 776
        uint voiceLen;                                        // 780
        char voice[32] = "trent_voice";                       // 812
        Costume baseCostume;                                  // 816 - 868
        uint equipIdEnumerator;                           // 872
        st6::string prefilledWeaponGroupIni;                  // 886
        st6::list<FmtStr> neuralNetLog;                       // 902 - probably wrong, 'log' entries from character file. Also 'mostly' SP only.
        int interfaceState;                                  // 914
        uint unused2;                                         // 918
        BinarySearchTree<VisitEntry> visitLists;              // 922
                                                              // 934

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
        std::vector<Character> characters;
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
        using OnPlayerSaveType = bool(__fastcall*)(PlayerData* data);
        using OnCreateNewCharacterType = bool(__fastcall*)(PlayerData* data, void* edx, SCreateCharacterInfo* character);

        inline static std::unique_ptr<FunctionDetour<DbInitType>> dbInitDetour;
        inline static std::unique_ptr<FunctionDetour<OnPlayerSaveType>> onPlayerSaveDetour;
        inline static std::unique_ptr<FunctionDetour<OnCreateNewCharacterType>> onCreateNewCharacterDetour;
        bool static __fastcall OnCreateNewCharacter(PlayerData* data, void* edx, SCreateCharacterInfo* characterInfo);
        bool static __fastcall OnPlayerSave(PlayerData* pd);
        void static __fastcall PlayerDbInitDetour(PlayerDB* db, void* edx, uint unk, bool unk2);
        void static __fastcall CreateAccountInitFromFolderBypass(CAccount* account, void* edx, char* dir);

        PlayerDbLoadUserDataAssembly loadUserDataAssembly;

        AccountManager();

    public:
        static bool SaveCharacter(const Character& newCharacter, bool isNewCharacter);
        static void DeleteCharacter(const std::wstring& characterName);
        static void Login(const std::wstring& info, const ClientId& client);
};
