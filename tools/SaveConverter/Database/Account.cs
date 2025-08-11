using MongoDB.Bson;
using MongoDB.Bson.Serialization.Attributes;

namespace FLHook.SaveConverter.Database;

[Serializable]
public class Account
{
    [BsonId]
    public required string Id { get; set; }

    [BsonElement("characters")]
    public List<ObjectId> Characters { get; set; } = [];
    
    [BsonElement("scheduledUnbanDate")]
    public long? ScheduledUnbanDate { get; set; } = null;
    
    [BsonElement("cash")]
    public long Cash = 0;
}