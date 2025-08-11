using System.Numerics;
using LibreLancer.Data.Characters;
using MongoDB.Bson;
using MongoDB.Bson.Serialization.Attributes;

namespace FLHook.SaveConverter.Database;

[Serializable]
public class Character
{
    [BsonId]
    public ObjectId Id { get; set; }

    [BsonElement("accountId")]
    public required string AccountId { get; set; }

    [BsonElement("characterName")]
    public required string CharacterName { get; set; }

    [BsonElement("money")]
    public int Money { get; set; }

    [BsonElement("rank")]
    public int Rank { get; set; }

    [BsonElement("repGroup")]
    public string? RepGroup { get; set; }

    [BsonElement("pos")]
    public float[]? Pos { get; set; }

    [BsonElement("rot")]
    public float[]? Rot { get; set; }

    [BsonElement("interfaceState")]
    public int InterfaceState { get; set; }

    [BsonElement("hullStatus")]
    public float HullStatus { get; set; } = 1f;

    [BsonElement("baseHullStatus")]
    public float BaseHullStatus { get; set; } = 1f;

    [BsonElement("canDock")]
    public bool CanDock { get; set; }

    [BsonElement("canTradeLane")]
    public bool CanTradeLane { get; set; }

    [BsonElement("lastDockedBase")]
    public int LastDockedBase { get; set; }

    [BsonElement("currentBase")]
    public int CurrentBase { get; set; }

    [BsonElement("currentRoom")]
    public int CurrentRoom { get; set; }

    [BsonElement("killCount")]
    public int KillCount { get; set; }

    [BsonElement("missionFailureCount")]
    public int MissionFailureCount { get; set; }

    [BsonElement("missionSuccessCount")]
    public int MissionSuccessCount { get; set; }

    [BsonElement("voice")]
    public string Voice { get; set; } = "trent_voice";

    [BsonElement("shipHash")]
    public int ShipHash { get; set; }

    [BsonElement("system")]
    public int System { get; set; }

    [BsonElement("totalTimePlayed")]
    public float TotalTimePlayed { get; set; }

    [BsonElement("baseCostume")]
    public Costume? BaseCostume { get; set; }

    [BsonElement("commCostume")]
    public Costume? CommCostume { get; set; }

    [BsonElement("reputation")]
    public Dictionary<string, float> Reputation { get; set; } = new();

    [BsonElement("equipment")]
    public List<Equipment> Equipment { get; set; } = [];

    [BsonElement("baseEquipment")]
    public List<Equipment> BaseEquipment { get; set; } = [];

    [BsonElement("cargo")]
    public List<Cargo> Cargo { get; set; } = [];

    [BsonElement("baseCargo")]
    public List<Cargo> BaseCargo { get; set; } = [];

    [BsonElement("collisionGroups")]
    public Dictionary<string, float> CollisionGroups { get; set; } = new();

    [BsonElement("visits")]
    public List<List<int>> Visits { get; set; } = [];

    [BsonElement("shipTypesKilled")]
    public Dictionary<string, int> ShipTypesKilled { get; set; } = new();

    [BsonElement("systemsVisited")]
    public List<int> SystemsVisited { get; set; } = [];

    [BsonElement("basesVisited")]
    public List<int> BasesVisited { get; set; } = [];

    [BsonElement("jumpHolesVisited")]
    public List<int> JumpHolesVisited { get; set; } = [];

    [BsonElement("npcVisits")]
    public List<NpcVisit> NpcVisits { get; set; } = [];

    [BsonElement("weaponGroups")]
    public Dictionary<string, List<string>> WeaponGroups { get; set; } = new();

    [BsonElement("randomMissionsFailed")]
    public Dictionary<string, int> RandomMissionsFailed { get; set; } = [];
    
    [BsonElement("randomMissionsCompleted")]
    public Dictionary<string, int> RandomMissionsCompleted  { get; set; } = [];
    
    [BsonElement("randomMissionsAborted")]
    public Dictionary<string, int> RandomMissionsAborted  { get; set; } = [];
}