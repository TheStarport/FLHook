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

            DEBUG("Loaded Resource DLL: {{debug}}", { "dll", StringUtils::stows(dll) });
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
        DEBUG("Unloaded Resource DLL: {{dll}}", { "dll", buffer.substr(buffer.find('\\') + 1) });
        FreeLibrary(dll);
    }
}

std::wstring_view InfocardManager::GetInfoName(const uint ids) const
{
    if (const auto found = infoCardOverride.find(ids); found != infoCardOverride.end())
    {
        return StringUtils::stows(found->second);
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

    InfocardPayload payload;
    if (isName)
    {
        payload.infoNames[ids] = StringUtils::wstos(override);
    }
    else
    {
        payload.infoCards[ids] = StringUtils::wstos(override);
    }

    if (client)
    {
        client.SendFlufPayload(header, payload);
        return;
    }

    if (isName)
    {
        infoNameOverride[ids] = StringUtils::wstos(override);
    }
    else
    {
        infoCardOverride[ids] = StringUtils::wstos(override);
    }

    for (const auto& flufClient : FLHook::Clients())
    {
        if (flufClient.usingFlufClientHook)
        {
            flufClient.id.SendFlufPayload(header, payload);
        }
    }
}

void InfocardManager::OverrideInfocards(const InfocardPayload& payload, ClientId client)
{
    if (client)
    {
        client.SendFlufPayload(header, payload);
        return;
    }

    for (const auto& flufClient : FLHook::Clients())
    {
        if (flufClient.usingFlufClientHook)
        {
            flufClient.id.SendFlufPayload(header, payload);
        }
    }

    for (auto& [ids, override] : payload.infoNames)
    {
        infoNameOverride[ids] = override;
    }

    for (auto& [ids, override] : payload.infoCards)
    {
        infoCardOverride[ids] = override;
    }
}

void InfocardManager::SendAllOverrides(ClientId client) { OverrideInfocards({ infoCardOverride, infoNameOverride }, client); }

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

uint InfocardManager::ReturnPluginInfocardRange()
{
    static uint registeredPluginsCount = 0;
    return UINT_MAX - (USHRT_MAX * ++registeredPluginsCount);
}
