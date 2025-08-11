using MongoDB.Bson.Serialization.Attributes;

namespace FLHook.SaveConverter.Database;

[Serializable]
[BsonNoId]
public class NpcVisit
{
    [BsonElement("npcId")] 
    public int NpcId { get; set; }

    [BsonElement("baseId")] 
    public int BaseId { get; set; }

    [BsonElement("interactionCount")] 
    public int InteractionCount { get; set; }

    [BsonElement("missionStatus")] 
    public int MissionStatus { get; set; }
}