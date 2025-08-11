using System.Text;
using FLHook.SaveConverter.Database;
using MongoDB.Driver;
using Serilog;

namespace FLHook.SaveConverter.Processors;

public static class AccountProcessor
{
    public static string? ProcessNameFile(string dirString, Stream file)
    {
        // A special number Freelancer uses to 'hash' account strings
        const ushort magicNumber = 0x2E6E;

        if (file.Length != 70)
        {
            Log.Warning("{dirString} has a non-70 length name file, corrupted?", dirString);
            return null;
        }
        
        var accountStr = "";

        for (var i = 0; i < 70; i += 2)
        {
            var buffer = new byte[2];
            file.ReadExactly(buffer, 0, 2);
            var raw = BitConverter.ToUInt16(buffer);
            var xor = raw ^ magicNumber;
            var bytes = BitConverter.GetBytes(xor);
            
            // If bit converter is not little endian, we need to run one last conversion to pull out the right string
            if (!BitConverter.IsLittleEndian)
            {
                bytes = bytes.Reverse().ToArray();
            }
            
            // Trim back down to two bytes and append
            accountStr += Encoding.Default.GetString(bytes.Take(2).ToArray());
        }

        // Remove the null padding
        return accountStr.Replace("\0", "");
    }

    public static async Task ProcessAccount(string accStr, List<string> characterFiles, DatabaseClient client, bool banned)
    {
        var characters = characterFiles.Select(x => CharacterProcessor.ProcessCharacter(x, accStr)).OfType<Character>().ToList();
        if (characters.Count is 0)
        {
            Log.Information("{accStr} has no characters successfully processed, skipping account", accStr);
            Interlocked.Increment(ref Program.AccountsSkipped);
            return;
        }
        
        try
        {
            var accountCollection = client.Db.GetCollection<Account>(Program.Config.AccountsCollection);
            var characterCollection = client.Db.GetCollection<Character>(Program.Config.CharactersCollection);
            
            await characterCollection.InsertManyAsync(characters);

            var ids = characters.Select(x => x.Id).ToList();

            var account = new Account()
            {
                Cash = 0,
                Characters = ids,
                Id = accStr,
            };

            if (banned)
            {
                account.ScheduledUnbanDate = DateTimeOffset.UtcNow.AddYears(100).ToUnixTimeSeconds();
            }

            await accountCollection.InsertOneAsync(account);
            Interlocked.Increment(ref Program.AccountsProcessed);
        }
        catch (Exception ex)
        {
            Log.Error(ex, "Error while processing account");
            Interlocked.Increment(ref Program.AccountsSkipped);
        }
    }
}