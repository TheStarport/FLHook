#pragma once

#include <bsoncxx/oid.hpp>

struct Cargo
{
        int64 id;
        int64 amount;
        float health;
        bool isMissionCargo;
};

struct NpcVisit
{
        int64 id;
        int64 baseId;
        int interactionCount;
        int missionStatus;
};

struct Equipment
{
        int64 id;
        std::string hardPoint;
        float health;
};

struct Character
{
        bsoncxx::oid _id;
        std::string characterName;
        int money;
        int rank;
        std::optional<std::string> repGroup;
        std::optional<std::array<float, 3>> pos;
        std::optional<std::array<float, 3>> rot;
        int interfaceState;
        float hullStatus;
        float baseHullStatus;
        bool canDock;
        bool canTradeLane;
        int64 lastDockedBase;
        int64 currentRoom;
        int killCount;
        int missionFailureCount;
        int missionSuccessCount;
        int64 shipHash;
        int64 system;
        int64 totalTimePlayed;
        std::optional<Costume> baseCostume;
        std::optional<Costume> commCostume;
        std::vector<Cargo> cargo;
        std::vector<Cargo> baseCargo;
        std::vector<Equipment> equipment;
        std::vector<Equipment> baseEquipment;
        std::unordered_map<std::string, float> collisionGroups;
        std::unordered_map<std::string, float> reputation;
        std::unordered_map<int64, int> shipTypesKilled;
        std::unordered_map<int64, int> visits;
        std::vector<int64> systemsVisited;
        std::vector<int64> basesVisited;
        std::vector<NpcVisit> npcVisits;
        std::vector<int64> jumpHolesVisited;
        std::unordered_map<int, std::vector<std::string>> weaponGroups;
};
