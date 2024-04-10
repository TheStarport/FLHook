#pragma once

#include "rfl/bson.hpp"

#include <bsoncxx/oid.hpp>

// Cargo is a namespace in Freelancer, appended with FL to avoid name conflicts
struct FLCargo
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
	bool mounted;
};

struct Character
{
        std::optional<bson_oid_t> _id;
        std::string accountId;
        std::string characterName;
        int money = 0;
        int rank = 0;
        int64 affiliation = 0;
        std::optional<std::string> repGroup;
        std::optional<Vector> pos;
        std::optional<Vector> rot;
        std::string voice;
        int interfaceState = 0;
        float hullStatus = 1.f;
        float baseHullStatus = 1.f;
        bool canDock = true;
        bool canTradeLane = true;
        int64 lastDockedBase = 0;
	int64 currentBase = 0;
        int64 currentRoom = 0;
        int killCount = 0;
        int missionFailureCount = 0;
        int missionSuccessCount = 0;
        int64 shipHash = 0;
        int64 system = 0;
        int64 totalTimePlayed = 0;
        Costume baseCostume;
        Costume commCostume;
        std::vector<FLCargo> cargo;
        std::vector<FLCargo> baseCargo;
        std::vector<Equipment> equipment;
        std::vector<Equipment> baseEquipment;
        std::unordered_map<int, float> collisionGroups;
	std::unordered_map<int, float> baseCollisionGroups;
        std::unordered_map<int64, float> reputation;
        std::unordered_map<int64, int> shipTypesKilled;
        std::unordered_map<int64, int> visits;
        std::vector<int64> systemsVisited;
        std::vector<int64> basesVisited;
        std::vector<NpcVisit> npcVisits;
        std::vector<int64> jumpHolesVisited;
        std::unordered_map<int, std::vector<std::string>> weaponGroups;
};
