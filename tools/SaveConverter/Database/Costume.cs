using MongoDB.Bson.Serialization.Attributes;

namespace FLHook.SaveConverter.Database;

[Serializable]
[BsonNoId]
public class Costume
{
    [BsonElement("body")]
    public int Body { get; set; }

    [BsonElement("head")]
    public int Head { get; set; }

    [BsonElement("leftHand")]
    public int LeftHand { get; set; }

    [BsonElement("rightHand")]
    public int RightHand { get; set; }

    [BsonElement("accessories")]
    public List<int> Accessories { get; set; } = [];
}