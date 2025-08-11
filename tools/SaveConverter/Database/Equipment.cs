using MongoDB.Bson.Serialization.Attributes;

namespace FLHook.SaveConverter.Database;

[Serializable]
[BsonNoId]
public class Equipment
{
    [BsonElement("equipId")] 
    public int EquipmentId { get; set; }

    [BsonElement("hardPoint")] 
    public required string HardPoint { get; set; }

    [BsonElement("health")] 
    public float Health { get; set; }
}