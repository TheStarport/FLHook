using MongoDB.Bson.Serialization.Attributes;

namespace FLHook.SaveConverter.Database;

[Serializable]
[BsonNoId]
public class Cargo
{
    [BsonElement("cargoId")]
    public int CargoId { get; set; }

    [BsonElement("amount")] 
    public int Amount { get; set; }

    [BsonElement("health")] 
    public float Health { get; set; }

    [BsonElement("isMissionCargo")] 
    public bool IsMissionCargo { get; set; }
}