#pragma once

#include "rfl/bson.hpp"

#include <bsoncxx/oid.hpp>
#include <bsoncxx/builder/basic/document.hpp>
#include <API/FLHook/BsonHelper.hpp>

// Cargo is a namespace in Freelancer, appended with FL to avoid name conflicts
DLL struct FLCargo
{
        FLCargo() = default;
        FLCargo(int i, ushort u, float f, bool b)
        {
            archId = i;
            amount = u;
            health = f;
            isMissionCargo = b;
        };
        FLCargo(const EquipDesc& desc)
        {
            archId = desc.archId.GetValue();
            health = desc.health;
            isMissionCargo = desc.mission;
            amount = desc.count;
        }

        B_VAL ToBson()
        {
            return B_MDOC(B_KVP("archId", archId), B_KVP("amount", amount), B_KVP("health", health), B_KVP("isMissionCargo", isMissionCargo));
        }

        int archId;
        ushort amount;
        float health;
        bool isMissionCargo;
};

struct NpcVisit
{
        int id;
        int baseId;
        int interactionCount;
        int missionStatus;
};

struct Equipment
{
        int archId;
        std::string hardPoint;
        float health;
        short amount;
        bool mounted;
};

struct TradeLaneException
{
        int startRingId;
        int nextRingId;
};

struct RumorData
{
        int rumorIds;
        int priority;
};

struct Character
{
        std::optional<bsoncxx::oid> _id;
        std::string accountId;
        std::string characterName;
        std::wstring wideCharacterName;
        int money = 0;
        int rank = 0;
        int affiliation = 0;
        std::optional<std::string> repGroup;
        Vector pos;
        Vector rot;
        std::string voice;
        int interfaceState = 0;
        float hullStatus = 1.f;
        float baseHullStatus = 1.f;
        bool canDock = true;
        bool canTradeLane = true;
        std::optional<std::vector<TradeLaneException>> tlExceptions;
        std::optional<std::vector<int>> dockExceptions;
        int lastDockedBase = 0;
        int currentBase = 0;
        int currentRoom = 0;
        int killCount = 0;
        int missionFailureCount = 0;
        int missionSuccessCount = 0;
        int shipHash = 0;
        int system = 0;
        float totalTimePlayed = 0.f;
        float totalCashEarned = 0.f;
        Costume baseCostume;
        Costume commCostume;
        std::vector<FLCargo> cargo;
        std::vector<FLCargo> baseCargo;
        std::vector<Equipment> equipment;
        std::vector<Equipment> baseEquipment;
        std::unordered_map<std::string, float> collisionGroups;
        std::unordered_map<std::string, float> baseCollisionGroups;
        // keys are actually unsigned ints, but this works anyway
        std::unordered_map<std::string, float> reputation;
        std::unordered_map<std::string, int> shipTypesKilled;
        std::unordered_map<std::string, int> randomMissionsCompleted;
        std::unordered_map<std::string, int> randomMissionsAborted;
        std::unordered_map<std::string, int> randomMissionsFailed;
        std::vector<std::array<int, 2>> visits;
        std::vector<int> systemsVisited;
        std::vector<int> basesVisited;
        std::vector<NpcVisit> npcVisits;
        std::vector<int> jumpHolesVisited;
        std::vector<RumorData> rumorsReceived;
        std::unordered_map<std::string, std::vector<std::string>> weaponGroups;

        std::optional<int64> lastRenameTimestamp;
        std::optional<std::string> characterTransferCode;
        std::array<std::string, 10> presetMsgs;

        B_VAL characterDocument = B_MDOC();

        void ToBson(B_DOC& document);
        void HandleDeadCharacter();
        explicit Character(B_VIEW view);
        Character() = default;
};
