using FLHook.SaveConverter;
using FLHook.SaveConverter.Database;
using FLHook.SaveConverter.Processors;
using LibreLancer;
using LibreLancer.Data;
using LibreLancer.Data.IO;
using Microsoft.Extensions.Logging;
using MongoDB.Bson;
using MongoDB.Driver;
using Serilog;
using Serilog.Core;
using YamlDotNet.Serialization;
using YamlDotNet.Serialization.NamingConventions;

const string configLocation = "config.yml";

if (!File.Exists(configLocation))
{
    var serializer = new SerializerBuilder()
        .WithNamingConvention(CamelCaseNamingConvention.Instance)
        .Build();

    File.WriteAllText(configLocation, serializer.Serialize(new Config()));
    Console.WriteLine($"Initial config file created called {configLocation}. Please edit this file as desired and rerun");
    return 0;
}

var configRaw = File.ReadAllText(configLocation);

var deserializer = new DeserializerBuilder()
    .WithNamingConvention(CamelCaseNamingConvention.Instance)
    .Build();

Config = deserializer.Deserialize<Config>(configRaw);

var loggerBuilder = new LoggerConfiguration();
if (Config.Debug)
{
    loggerBuilder.MinimumLevel.Debug();
}
else
{
    loggerBuilder.MinimumLevel.Information();
}

if (Config.LogToFile)
{
    loggerBuilder.WriteTo.File("log-.txt", rollingInterval: RollingInterval.Day);
}

if (Config.LogToConsole)
{
    loggerBuilder.WriteTo.Console();
}

var logger = loggerBuilder.CreateLogger();
Log.Logger = logger;

var flPath = Path.Join(Config.FreelancerPath, "EXE", "freelancer.ini");
if (!File.Exists(flPath) || !Directory.Exists(Path.Join(Config.FreelancerPath, "DATA")))
{
    logger.Error("Freelancer path did not point to a Freelancer install");
    return 1;
}

logger.Information("Loading Freelancer install: {freelancer}", Config.FreelancerPath);

var vfs = FileSystem.FromPath(Config.FreelancerPath);
var flIni = new FreelancerIni(flPath, vfs);
GameData = new FreelancerData(flIni, vfs);
GameData.LoadData();

var exit = SetupDataDefaults();
if (exit is not 0)
{
    return exit;
}

var client = new DatabaseClient(Config);

if (!client.TestConnection())
{
    logger.Error("Failed to connect to mongodb database {database}", Config.Database);
    return 1;
}

if (await client.CollectionExistsAsync(Config.AccountsCollection) || await client.CollectionExistsAsync(Config.CharactersCollection))
{
    if (args.Contains("--replace-db"))
    {
        logger.Information("--replace-db present, purging collections");
        await client.Db.DropCollectionAsync(Config.AccountsCollection);
        await client.Db.DropCollectionAsync(Config.CharactersCollection);
    }
    else
    {
        logger.Error("Collections already exist within the DB. The conversion utility requires an empty database. " +
                     "Run again with '--replace-db' if you would like to purge the database.");
        return 1;
    }
}

logger.Information("Creating collections");
await client.Db.CreateCollectionAsync(Config.AccountsCollection);
await client.Db.CreateCollectionAsync(Config.CharactersCollection);

// TODO: indexes

var accPath = Path.Join(Config.FreelancerSavePath, "Accts", "MultiPlayer");
Dictionary<string, (string, List<string>)> accounts = [];

foreach (var dir in Directory.GetDirectories(accPath))
{
    var dirString = Path.GetFileName(dir)!;

    var accountString = "";
    List<string> accountFiles = [];

    foreach (var file in Directory.GetFiles(dir))
    {
        if (file.EndsWith($"{Path.DirectorySeparatorChar}name"))
        {
            await using var stream = File.OpenRead(file);
            accountString = AccountProcessor.ProcessNameFile(dirString, stream);

            if (accountString is null)
            {
                break;
            }
        }
        else if (file.EndsWith(".fl"))
        {
            accountFiles.Add(file);
        }
    }

    if (accountFiles.Count is 0 || accountString is null)
    {
        logger.Debug("No account files found or account string was malformed. Not processing directory: {dir}", dirString);
        continue;
    }

    accounts[accountString] = (dirString, accountFiles);
}

logger.Information("Finished collecting accounts. Found {count} - Processing", accounts.Count);
var itemsDone = 0;
float percentDone = 0;
foreach (var kvp in accounts)
{
    bool isBanned = File.Exists(Path.Join(kvp.Value.Item1, "banned"));
    
    logger.Information("Processing account directory: {dir}, found {count} characters", kvp.Value.Item1, kvp.Value.Item2.Count);
    await AccountProcessor.ProcessAccount(kvp.Key, kvp.Value.Item2, client, isBanned);

    itemsDone++;
    var percent = (float)itemsDone / accounts.Count * 100f;
    if (Math.Abs(MathF.Ceiling(percent) - percentDone) > float.Epsilon)
    {
        percentDone = MathF.Ceiling(percent);
        logger.Information("Processed {percent:0.##}% of accounts", percentDone);
    }
}

var charTotal = CharactersSkipped + CharactersProcessed;
logger.Information("Finished processing accounts");
logger.Information("Processed {processed}/{total} accounts ({percent}%)", AccountsProcessed, accounts.Count, (float)AccountsProcessed / accounts.Count * 100f);
logger.Information("Processed {processed}/{total} characters ({percent}%)", CharactersProcessed, charTotal, (float)CharactersProcessed / charTotal * 100f);
logger.Information("Skipped {count} accounts ({percent}%)", AccountsSkipped, (float)AccountsSkipped / accounts.Count * 100f);
logger.Information("Skipped {count} characters ({percent}%)", CharactersSkipped, (float)CharactersSkipped / charTotal * 100f);

return 0;

static int SetupDataDefaults()
{
    if (Config is { CleanBadData: false, ValidateCharacters: false })
    {
        return 0;
    }

    var system = GameData.Universe.Systems.FirstOrDefault(x => x.Nickname.Equals(Config.BrokenCharacterSystem, StringComparison.InvariantCultureIgnoreCase));
    if (system is null)
    {
        Log.Warning("System specified in config was not found in the game data. Verify the correct folder and data is set.");
        return 1;
    }
    
    var @base = GameData.Universe.Bases.FirstOrDefault(x => x.Nickname.Equals(Config.BrokenCharacterBase, StringComparison.InvariantCultureIgnoreCase));
    if (@base is null)
    {
        Log.Warning("Base specified in config was not found in the game data. Verify the correct folder and data is set.");
        return 1;
    }

    return 0;
}

public partial class Program
{
    public static FreelancerData GameData = null!;
    public static Config Config = null!;
    public static int AccountsSkipped = 0;
    public static int AccountsProcessed = 0;
    public static int CharactersProcessed = 0;
    public static int CharactersSkipped = 0;

    public static int Hash(string nickname, bool factionHash = false)

    {
        var hash = factionHash ? FLHash.FLFacHash(nickname) : FLHash.CreateID(nickname);
        var intHash = (int)hash;

        if (!BitConverter.IsLittleEndian)
        {
            intHash = BitConverter.ToInt32(BitConverter.GetBytes(intHash).Reverse().ToArray());
        }

        return intHash;
    }
}