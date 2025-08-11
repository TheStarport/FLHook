using MongoDB.Bson.Serialization.Attributes;

namespace FLHook.SaveConverter.Database;

[Serializable]
[BsonNoId]
public class Equipment
{
    [BsonElement("archId")] 
    public int ArchId { get; set; }

    [BsonElement("hp")] 
    public required string HardPoint { get; set; }

    [BsonElement("health")] 
    public float Health { get; set; }
}