#pragma once

#include "rfl/bson.hpp"

#include <bsoncxx/oid.hpp>

// Cargo is a namespace in Freelancer, appended with FL to avoid name conflicts
struct FLCargo
{
	uint archId;
	ushort amount;
	float health;
	bool isMissionCargo;
};

struct NpcVisit
{
	uint id;
	uint baseId;
	int interactionCount;
	int missionStatus;
};

struct Equipment
{
	uint archId;
	std::string hardPoint;
	float health;
	bool mounted;
};

struct TradeLaneException
{
	uint startRingId;
	uint nextRingId;
};

struct RumorData
{
	uint rumorIds;
	uint unk;
};

struct Character
{
	std::optional<bson_oid_t> _id;
	std::string accountId;
	std::string characterName;
	int money = 0;
	int rank = 0;
	uint affiliation = 0;
	std::optional<std::string> repGroup;
	std::optional<Vector> pos;
	std::optional<Vector> rot;
	std::string voice;
	int interfaceState = 0;
	float hullStatus = 1.f;
	float baseHullStatus = 1.f;
	bool canDock = true;
	bool canTradeLane = true;
	std::optional<std::vector<TradeLaneException>> tlExceptions;
	std::optional<std::vector<uint>> dockExceptions;
	uint lastDockedBase = 0;
	uint currentBase = 0;
	uint currentRoom = 0;
	int killCount = 0;
	int missionFailureCount = 0;
	int missionSuccessCount = 0;
	uint shipHash = 0;
	uint system = 0;
	float totalTimePlayed = 0.f;
	float totalCashEarned = 0.f;
	Costume baseCostume;
	Costume commCostume;
	std::vector<FLCargo> cargo;
	std::vector<FLCargo> baseCargo;
	std::vector<Equipment> equipment;
	std::vector<Equipment> baseEquipment;
	std::unordered_map<ushort, float> collisionGroups;
	std::unordered_map<ushort, float> baseCollisionGroups;
	//keys are actually unsigned ints, but this works anyway
	std::unordered_map<uint, float> reputation;
	std::unordered_map<uint, int> shipTypesKilled;
	std::unordered_map<uint, int> randomMissionsCompleted;
	std::unordered_map<uint, int> randomMissionsAborted;
	std::unordered_map<uint, int> randomMissionsFailed;
	std::unordered_map<uint, char> visits;
	std::vector<uint> systemsVisited;
	std::vector<uint> basesVisited;
	std::vector<NpcVisit> npcVisits;
	std::vector<uint> jumpHolesVisited;
	std::vector<RumorData> rumorsReceived;
	std::unordered_map<int, std::vector<std::string>> weaponGroups;
};
