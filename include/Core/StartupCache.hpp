#pragma once

class StartupCache
{
        // The number of characters loaded.
        inline static int charsLoaded = 0;

        // The original function read charname function
        using ReadCharacterName = int(__stdcall*)(char* filename, st6::wstring* str);
        inline static ReadCharacterName readCharName;

        // map of acc_char_path to char name
        inline static std::map<std::wstring, std::wstring> cache;

        std::wstring baseAcctPath;

        // length of the user data path + accts\multiplayer to remove so that
        // we can search only for the acc_char_path
        inline static size_t accPathPrefixLength = 0;

        // A fast alternative to the built in read character name function in server.dll
        static int __stdcall ReadCharacterNameHook(char* fileNameRaw, st6::wstring* str);

        void LoadCache() const;
        void SaveCache();

    public:
        StartupCache();
        void Done();
};
