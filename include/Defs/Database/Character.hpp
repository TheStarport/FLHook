#pragma once

#include <bsoncxx/oid.hpp>
#include "rfl/bson.hpp"

//Cargo is a namespace in Freelancer, appended with FL to avoid name conflicts
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
        bsoncxx::oid _id;
        std::string characterName;
        int money;
        int rank;
		int64 reputationId;
        std::optional<std::string> repGroup;
        std::optional<Vector> pos;
        std::optional<Vector> rot;
        int interfaceState;
        float hullStatus;
        float baseHullStatus;
        bool canDock;
        bool canTradeLane;
        int64 lastDockedBase;
		int64 currentBase;
        int64 currentRoom;
        int killCount;
        int missionFailureCount;
        int missionSuccessCount;
        int64 shipHash;
        int64 system;
        int64 totalTimePlayed;
        std::optional<Costume> baseCostume;
        std::optional<Costume> commCostume;
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
