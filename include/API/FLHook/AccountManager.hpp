#pragma once

#include "Utils/Detour.hpp"

class AccountManager
{
        friend FLHook;
        friend IServerImplHook;
        inline static AccountManager* instance = nullptr;

        inline static struct NewPlayerTemplate
        {
                std::string initialRep;
                int rank = 0;
                int money = 0;
                std::string costume;
                std::string commCostume;
                std::string system;
                std::string base;
                std::map<std::string, float> reputationOverrides;
                std::map<uint, uint> visitValues;
                bool hasPackage = false;
        } newPlayerTemplate;

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
                Costume costume;                             // 712
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

        struct AccountData
        {
                CAccount* account;
                std::map<std::string, VanillaLoadData> characters;
        };

        inline static std::array<AccountData, 256> accounts;

        enum class LoginReturnCode
        {
            Banned,
            AlreadyLoggedIn,
            InvalidUsernamePassword,
            Success
        };

        // The current account string we are loading
        std::wstring currentAccountString;
        static void LoadCharacter(VanillaLoadData* data, std::wstring_view characterName);
        LoginReturnCode AccountLoginInternal(PlayerData* data, uint clientId);
        static void LoadNewPlayerFLInfo();

        // Detours/Assembly
        using DbInitType = void(__fastcall*)(PlayerDB* db, void* edx, uint unk, bool unk2);
        using OnPlayerSaveType = bool(__fastcall*)(PlayerData* data);
        using OnCreateNewCharacterType = bool(__fastcall*)(PlayerData* data, void* edx, SCreateCharacterInfo* character);

        inline static std::unique_ptr<FunctionDetour<DbInitType>> dbInitDetour;
        inline static std::unique_ptr<FunctionDetour<OnPlayerSaveType>> onPlayerSaveDetour;
        inline static std::unique_ptr<FunctionDetour<OnCreateNewCharacterType>> onCreateNewCharacterDetour;
        bool static __fastcall OnCreateNewCharacter(PlayerData* data, void* edx, SCreateCharacterInfo* character);
        bool static __fastcall OnPlayerSave(PlayerData* data);
        void static __fastcall PlayerDbInitDetour(PlayerDB* db, void* edx, uint unk, bool unk2);
        void static PlayerDbLoadUserDataNaked();

        AccountManager();

    public:
        void DeleteCharacter(const std::wstring& characterName);
        void OnLogin(const ClientId& client);
};
