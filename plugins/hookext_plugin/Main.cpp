/**
HookExtension Plugin by Cannon
*/

// includes

#include <FLHook.h>
#include <algorithm>
#include <list>
#include <map>
#include <math.h>
#include <plugin.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <windows.h>

struct FLHOOK_PLAYER_DATA {
    std::string charfilename;
    std::map<std::string, std::string> lines;
};

std::map<uint, FLHOOK_PLAYER_DATA> clients;

/// A return code to indicate to FLHook if we want the hook processing to
/// continue.
ReturnCode returncode;

std::string GetAccountDir(uint client) {
    static _GetFLName GetFLName = (_GetFLName)((char *)hModServer + 0x66370);
    char dirname[1024];
    GetFLName(dirname, Players[client].Account->wszAccID);
    return dirname;
}

std::string GetCharfilename(const std::wstring &charname) {
    static _GetFLName GetFLName = (_GetFLName)((char *)hModServer + 0x66370);
    char filename[1024];
    GetFLName(filename, charname.c_str());
    return filename;
}

static PlayerData *CurrPlayer;
int __stdcall HkCb_UpdateFile(char *filename, wchar_t *savetime, int b) {
    // Call the original save charfile function
    int retv;
    __asm {
        pushad
        mov ecx, [CurrPlayer]
        push b
        push savetime
        push filename
        mov eax, 0x6d4ccd0
        call eax
        mov retv, eax
        popad
    }

    // Readd the flhook section.
    if (retv) {
        uint client = CurrPlayer->iOnlineID;

        std::string path = scAcctPath + GetAccountDir(client) + "\\" + filename;
        FILE *file;
        fopen_s(&file, path.c_str(), "a");
        if (file) {
            fprintf(file, "[flhook]\n");
            for (auto &i : clients[client].lines)
                fprintf(file, "%s = %s\n", i.first.c_str(), i.second.c_str());
            fclose(file);
        }
    }

    return retv;
}

__declspec(naked) void HkCb_UpdateFileNaked() {
    __asm {
        mov CurrPlayer, ecx
        jmp HkCb_UpdateFile
    }
}

/// Clear client info when a client connects.
void ClearClientInfo(uint client) {
    clients.erase(client);
}

/// Load the flhook section from the character file
void __stdcall CharacterSelect(struct CHARACTER_ID const &charid,
                               unsigned int client) {
    std::string path =
        scAcctPath + GetAccountDir(client) + "\\" + charid.szCharFilename;

    // Console::ConPrint(L"CharacterSelect=%s", stows(path).c_str());

    clients[client].charfilename = charid.szCharFilename;
    clients[client].lines.clear();

    // Read the flhook section so that we can rewrite after the save so that it
    // isn't lost
    INI_Reader ini;
    if (ini.open(path.c_str(), false)) {
        while (ini.read_header()) {
            if (ini.is_header("flhook")) {
                std::wstring tag;
                while (ini.read_value()) {
                    clients[client].lines[ini.get_name_ptr()] =
                        ini.get_value_string();
                }
            }
        }
        ini.close();
    }
}

void __stdcall DisConnect(unsigned int client, enum EFLConnection p2) {
    clients.erase(client);
}

/// Load the configuration
void LoadSettings() {
    // The path to the configuration file.
    char szCurDir[MAX_PATH];
    GetCurrentDirectory(sizeof(szCurDir), szCurDir);
    std::string scPluginCfgFile =
        std::string(szCurDir) + "\\flhook_plugins\\hookext.cfg";

    clients.clear();
    struct PlayerData *pPD = 0;
    while (pPD = Players.traverse_active(pPD)) {
        uint client = HkGetClientIdFromPD(pPD);
        std::wstring charname =
            (const wchar_t *)Players.GetActiveCharacterName(client);
        std::string filename = GetCharfilename(charname) + ".fl";
        CHARACTER_ID charid;
        strcpy_s(charid.szCharFilename, filename.c_str());
        CharacterSelect(charid, client);
    }
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    static bool patched = false;
    srand((uint)time(0));

    // If we're being loaded from the command line while FLHook is running then
    // set_scCfgFile will not be empty so load the settings as FLHook only
    // calls load settings on FLHook startup and .rehash.
    if (fdwReason == DLL_PROCESS_ATTACH) {
        if (set_scCfgFile.length() > 0)
            LoadSettings();

        if (!patched) {
            patched = true;
            PatchCallAddr((char *)hModServer, 0x6c547,
                          (char *)HkCb_UpdateFileNaked);
            PatchCallAddr((char *)hModServer, 0x6c9cd,
                          (char *)HkCb_UpdateFileNaked);
        }
    } else if (fdwReason == DLL_PROCESS_DETACH) {
        if (patched) {
            patched = false;
            {
                BYTE patch[] = {0xE8, 0x84, 0x07, 0x00, 0x00};
                WriteProcMem((char *)hModServer + 0x6c547, patch, 5);
            }

            {
                BYTE patch[] = {0xE8, 0xFE, 0x2, 0x00, 0x00};
                WriteProcMem((char *)hModServer + 0x6c9cd, patch, 5);
            }
        }
    }
    return true;
}

namespace HookExt {
EXPORT std::string IniGetS(uint client, const std::string &name) {
    if (clients.find(client) == clients.end())
        return "";

    if (!clients[client].charfilename.length())
        return "";

    if (clients[client].lines.find(name) == clients[client].lines.end())
        return "";

    return clients[client].lines[name];
}

EXPORT std::wstring IniGetWS(uint client, const std::string &name) {
    std::string svalue = HookExt::IniGetS(client, name);

    std::wstring value;
    long lHiByte;
    long lLoByte;
    while (sscanf_s(svalue.c_str(), "%02X%02X", &lHiByte, &lLoByte) == 2) {
        svalue = svalue.substr(4);
        wchar_t wChar = (wchar_t)((lHiByte << 8) | lLoByte);
        value.append(1, wChar);
    }
    return value;
}

EXPORT uint IniGetI(uint client, const std::string &name) {
    std::string svalue = HookExt::IniGetS(client, name);
    return strtoul(svalue.c_str(), 0, 10);
}

EXPORT bool IniGetB(uint client, const std::string &name) {
    std::string svalue = HookExt::IniGetS(client, name);
    if (svalue == "yes")
        return true;
    return false;
}

EXPORT float IniGetF(uint client, const std::string &name) {
    std::string svalue = HookExt::IniGetS(client, name);
    return (float)atof(svalue.c_str());
}

EXPORT void IniSetS(uint client, const std::string &name,
                    const std::string &value) {
    clients[client].lines[name] = value;
}

EXPORT void IniSetWS(uint client, const std::string &name,
                     const std::wstring &value) {
    std::string svalue = "";
    for (uint i = 0; (i < value.length()); i++) {
        char cHiByte = value[i] >> 8;
        char cLoByte = value[i] & 0xFF;
        char szBuf[8];
        sprintf_s(szBuf, "%02X%02X", ((uint)cHiByte) & 0xFF,
                  ((uint)cLoByte) & 0xFF);
        svalue += szBuf;
    }

    HookExt::IniSetS(client, name, svalue);
}

EXPORT void IniSetI(uint client, const std::string &name, uint value) {
    char svalue[100];
    sprintf_s(svalue, "%u", value);
    HookExt::IniSetS(client, name, svalue);
}

EXPORT void IniSetB(uint client, const std::string &name, bool value) {
    std::string svalue = value ? "yes" : "no";
    HookExt::IniSetS(client, name, svalue);
}

EXPORT void IniSetF(uint client, const std::string &name, float value) {
    char svalue[100];
    sprintf_s(svalue, "%0.02f", value);
    HookExt::IniSetS(client, name, svalue);
}

EXPORT void IniSetS(const std::wstring &charname, const std::string &name,
                    const std::string &value) {
    // If the player is online then update the in memory cache.
    std::string charfilename = GetCharfilename(charname) + ".fl";
    for (auto &i : clients) {
        if (i.second.charfilename == charfilename) {
            HookExt::IniSetS(i.first, name, value);
            return;
        }
    }

    // Otherwise write directly to the character file if it exists.
    CAccount *acc = HkGetAccountByCharname(charname);
    if (acc) {
        std::string charpath =
            scAcctPath + GetCharfilename(acc->wszAccID) + "\\" + charfilename;
        WritePrivateProfileString("flhook", name.c_str(), value.c_str(),
                                  charpath.c_str());
    }
}

EXPORT void IniSetWS(const std::wstring &charname, const std::string &name,
                     const std::wstring &value) {
    std::string svalue = "";
    for (uint i = 0; (i < value.length()); i++) {
        char cHiByte = value[i] >> 8;
        char cLoByte = value[i] & 0xFF;
        char szBuf[8];
        sprintf_s(szBuf, "%02X%02X", ((uint)cHiByte) & 0xFF,
                  ((uint)cLoByte) & 0xFF);
        svalue += szBuf;
    }

    HookExt::IniSetS(charname, name, svalue);
}

EXPORT void IniSetI(const std::wstring &charname, const std::string &name,
                    uint value) {
    char svalue[100];
    sprintf_s(svalue, "%u", value);
    HookExt::IniSetS(charname, name, svalue);
}

EXPORT void IniSetB(const std::wstring &charname, const std::string &name,
                    bool value) {
    std::string svalue = value ? "yes" : "no";
    HookExt::IniSetS(charname, name, svalue);
}

EXPORT void IniSetF(const std::wstring &charname, const std::string &name,
                    float value) {
    char svalue[100];
    sprintf_s(svalue, "%0.02f", value);
    HookExt::IniSetS(charname, name, svalue);
}
} // namespace HookExt

/** Functions to hook */
extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi) {
    pi->name("HookExt Plugin by Cannon");
    pi->shortName("hookext");
    pi->mayPause(true);
    pi->mayUnload(true);
    pi->returnCode(&returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
    pi->emplaceHook(HookedCall::FLHook__LoadSettings, &LoadSettings);
    pi->emplaceHook(HookedCall::FLHook__ClearClientInfo, &ClearClientInfo);
    pi->emplaceHook(HookedCall::IServerImpl__CharacterSelect, &CharacterSelect);
    // pi->emplaceHook(PLUGIN_HOOKINFO((FARPROC*)&DisConnect,
    // PLUGIN_HkIServerImpl_DisConnect,0));
}
