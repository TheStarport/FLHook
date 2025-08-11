using MongoDB.Bson;
using MongoDB.Driver;

namespace FLHook.SaveConverter.Database;

public class DatabaseClient
{
    private readonly Config _config;
    private readonly MongoClient _client;
    public readonly IMongoDatabase Db;

    public DatabaseClient(Config config)
    {
        _config = config;
        _client = new MongoClient(_config.MongoConnectionString);
        Db = _client.GetDatabase(config.Database);
    }

    public bool TestConnection() => Db.RunCommandAsync((Command<BsonDocument>)"{ping:1}").Wait(1000);

    public async Task<bool> CollectionExistsAsync(string collectionName)
    {
        var filter = new BsonDocument("name", collectionName);
        var collections = await Db.ListCollectionsAsync(new ListCollectionsOptions { Filter = filter });
        return await collections.AnyAsync();
    }
}