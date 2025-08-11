using LibreLancer.Data.Universe;
using YamlDotNet.Serialization;

namespace FLHook.SaveConverter;

public class Config
{
    public string MongoConnectionString { get; set; } = "mongodb://localhost:27017/";
    public string Database { get; set; } = "FLHook";
    public string AccountsCollection { get; set; } = "accounts";
    public string CharactersCollection { get; set; } = "characters";
    public string FreelancerPath  { get; set; } = Path.Join(Directory.GetDirectoryRoot(Directory.GetCurrentDirectory()), "Games", "Freelancer");
    public string FreelancerSavePath  { get; set; } = Path.Join(Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments), "My Games", "Freelancer");
    public bool Debug { get; set; }  = false;
    public bool LogToFile  { get; set; } = true;
    public bool LogToConsole { get; set; } = true;
    public bool CleanBadData { get; set; } = true;
    public bool ValidateCharacters { get; set; } = true;
    public bool SkipZeroRankCharacters  { get; set; } = true;
    
    // Fixing fields

    public string BrokenCharacterSystem { get; set; } = "li01";
    public string BrokenCharacterBase { get; set; }= "li01_01_base";
}