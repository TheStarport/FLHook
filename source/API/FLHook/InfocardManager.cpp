#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/InfocardManager.hpp"

InfocardManager::InfocardManager()
{
    HINSTANCE hDll = LoadLibraryExW(L"resources.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE); // typically resources.dll
    if (hDll)
    {
        loadedDlls.push_back(hDll);
    }

    INI_Reader ini;
    if (!ini.open("freelancer.ini", false))
    {
        return;
    }

    if (!ini.find_header("Resources"))
    {
        ini.close();
        return;
    }

    while (ini.read_value())
    {
        if (!ini.is_value("DLL"))
        {
            continue;
        }

        std::string dll = ini.get_value_string();
        hDll = LoadLibraryExA(dll.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
        if (hDll)
        {

            DEBUG(L"Loaded Resource DLL: {0}", { L"dll", StringUtils::stows(dll) });
            loadedDlls.push_back(hDll);
        }
    }

    ini.close();
}

InfocardManager::~InfocardManager()
{
    for (const auto& dll : loadedDlls)
    {
        std::wstring buffer(MAX_PATH, L'\0');
        GetModuleFileName(dll, buffer.data(), buffer.size());
        DEBUG(L"Unloaded Resource DLL: {0}", { L"dll", buffer.substr(buffer.find('\\') + 1) });
        FreeLibrary(dll);
    }
}

std::wstring_view InfocardManager::GetInfoName(const uint ids) const
{
    if (const auto found = infoCardOverride.find(ids); found != infoCardOverride.end())
    {
        return { found->second.data(), found->second.size() };
    }

    wchar_t* destStr;
    const size_t length = LoadStringW(loadedDlls[ids >> 16], ids & 0xFFFF, reinterpret_cast<wchar_t*>(&destStr), 0);

    return { destStr, length };
}

void InfocardManager::OverrideInfocard(const uint ids, const std::wstring& override, bool isName, ClientId client)
{
    if (client && !client.HasFluf())
    {
        return;
    }

    constexpr ushort fluf = 0xF10F;
    constexpr std::array header = { 'i', 'n', 'f', 'o' };

    std::vector<char> buffer;
    buffer.resize(sizeof(ushort) + header.size() + sizeof(bool) * 2 + override.size() * 2);
    char* offset = buffer.data();

    memcpy_s(offset, buffer.size(), &fluf, sizeof(fluf));
    offset += sizeof(fluf);

    memcpy_s(offset, buffer.size(), header.data(), header.size());
    offset += header.size();

    offset[0] = static_cast<char>(isName);
    offset[1] = 0; // Is UTF8 = false

    memcpy_s(offset + 2, buffer.size(), override.data(), override.size() * 2);
    if (client)
    {
        InternalApi::FMsgSendChat(client, buffer.data(), buffer.size());
        return;
    }

    if (isName)
    {
        infoCardOverride[ids] = override;
    }
    else
    {
        infoNameOverride[ids] = override;
    }

    for (const auto& flufClient : FLHook::Clients())
    {
        if (flufClient.usingFlufClientHook)
        {
            InternalApi::FMsgSendChat(client, buffer.data(), buffer.size());
        }
    }
}

void InfocardManager::ClearOverride(const uint ids, const bool all)
{
    if (all)
    {
        infoNameOverride.clear();
        infoCardOverride.clear();
    }
    else
    {
        infoNameOverride.erase(ids);
        infoCardOverride.erase(ids);
    }
}
