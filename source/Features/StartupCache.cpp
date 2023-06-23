#include "Global.hpp"
#include "PCH.hpp"

// TODO: convert to a class (singleton)
namespace StartupCache
{
    // The number of characters loaded.
    static int charsLoaded = 0;

    // The original function read charname function
    using _ReadCharacterName = int(__stdcall*)(char* filename, st6::wstring* str);
    _ReadCharacterName ReadCharName;

    // map of acc_char_path to char name
    static std::map<std::wstring, std::wstring> cache;

    static std::wstring baseAcctPath;

    // length of the user data path + accts\multiplayer to remove so that
    // we can search only for the acc_char_path
    static int accPathPrefixLength = 0;

    // A fast alternative to the built in read character name function in server.dll
    static int __stdcall ReadCharacterName(char* fileNameRaw, st6::wstring* str)
    {
        std::wstring fileName = StringUtils::stows(fileNameRaw);
        Logger::i()->Log(LogLevel::Debug, std::format(L"Read character - {}", fileName));

        // If this account/charfile can be found in the character return then name immediately.
        const std::wstring accPath(&fileName.c_str()[accPathPrefixLength]);
        if (const auto i = cache.find(accPath); i != cache.end())
        {
            *str = st6::wstring((ushort*)i->second.c_str());
            return 1;
        }

        // Otherwise use the original FL function to load the char name
        // and cache the result and report that this is an uncached file
        ReadCharName(fileNameRaw, str);
        cache[accPath] = std::wstring((wchar_t*)str->c_str());
        return 1;
    }

    struct NameInfo
    {
            wchar_t acc_path[27]; // accdir(11)/charfile(11).fl + terminator
            wchar_t name[25];     // max name is 24 chars + terminator
    };

    static void LoadCache()
    {
        // Open the name cache file and load it into memory.
        std::wstring path = baseAcctPath + L"namecache.bin";

        // TODO: Remove capi file access
        Logger::i()->Log(LogLevel::Info, L"Loading character name cache");
        FILE* file;
        _wfopen_s(&file, path.c_str(), L"rb");
        if (file)
        {
            NameInfo ni{};
            while (fread(&ni, sizeof(NameInfo), 1, file))
            {
                std::wstring acc_path(ni.acc_path);
                const std::wstring name(ni.name);
                cache[acc_path] = name;
            }
            fclose(file);
        }
        Logger::i()->Log(LogLevel::Info, std::format(L"Loaded {} names", cache.size()));
    }

    static void SaveCache()
    {
        // Save the name cache file
        std::wstring path = baseAcctPath + L"namecache.bin";

        FILE* file;
        _wfopen_s(&file, path.c_str(), L"wb");
        if (file)
        {
            Logger::i()->Log(LogLevel::Info, L"Saving character name cache");
            for (const auto& [charPath, charName] : cache)
            {
                NameInfo ni{};
                memset(&ni, 0, sizeof ni);
                wcsncpy_s(ni.acc_path, 27, charPath.c_str(), charPath.size());
                wcsncpy_s(ni.name, 25, charName.c_str(), charName.size());
                if (!fwrite(&ni, sizeof(NameInfo), 1, file))
                {
                    Logger::i()->Log(LogLevel::Err, L"Saving character name cache failed");
                    break;
                }
            }
            fclose(file);
            Logger::i()->Log(LogLevel::Info, std::format(L"Saved {} names", cache.size()));
        }

        cache.clear();
    }

    // Call from Startup
    void Init()
    {
        // Disable the admin and banned file checks.
        {
            const BYTE patch[] = { 0x5f, 0x5e, 0x5d, 0x5b, 0x81, 0xC4, 0x08, 0x11, 0x00, 0x00, 0xC2, 0x04, 0x00 }; // pop regs, restore esp, ret 4
            MemUtils::WriteProcMem((char*)server + 0x76b3e, patch, 13);
        }

        // Hook the read character name and replace it with the caching version
        MemUtils::PatchCallAddr((char*)server, 0x717be, (char*)ReadCharacterName);

        // Keep a reference to the old read character name function.
        ReadCharName = reinterpret_cast<_ReadCharacterName>(reinterpret_cast<char*>(server) + 0x72fe0);

        // Calculate our base path
        char DataPath[MAX_PATH];
        GetUserDataPath(DataPath);
        baseAcctPath = StringUtils::stows(DataPath) + L"\\Accts\\MultiPlayer\\";
        accPathPrefixLength = baseAcctPath.length();

        // Load the cache
        LoadCache();
    }

    // Call from Startup_AFTER
    void Done()
    {
        SaveCache();

        // Restore admin and banned file checks
        {
            const BYTE patch[] = { 0x8b, 0x35, 0xc0, 0x4b, 0xd6, 0x06, 0x6a, 0x00, 0x68, 0xB0, 0xB8, 0xD6, 0x06 };
            MemUtils::WriteProcMem((char*)server + 0x76b3e, patch, 13);
        }

        // Unhook the read character name function.
        {
            const BYTE patch[] = { 0xe8, 0x1d, 0x18, 0x00, 0x00 };
            MemUtils::WriteProcMem((char*)server + 0x717be, patch, 5);
        }
    }
} // namespace StartupCache
