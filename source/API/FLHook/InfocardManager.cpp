#include "PCH.hpp"

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

        // TODO: Log loaded dll file
        hDll = LoadLibraryExA(ini.get_value_string(0), nullptr, LOAD_LIBRARY_AS_DATAFILE);
        if (hDll)
        {
            loadedDlls.push_back(hDll);
        }
    }

    ini.close();
}

InfocardManager::~InfocardManager()
{
    for (const auto& dll : loadedDlls)
    {
        // TODO: Log unloaded dll file
        FreeLibrary(dll);
    }
}

std::wstring_view InfocardManager::GetInfocard(const uint ids) const
{
    if (const auto found = infocardOverride.find(ids); found != infocardOverride.end())
    {
        return { found->second.data(), found->second.size() };
    }

    wchar_t* destStr;
    const size_t length = LoadStringW(loadedDlls[ids >> 16], ids & 0xFFFF, reinterpret_cast<wchar_t*>(&destStr), 0);

    return { destStr, length };
}

void InfocardManager::OverrideInfocard(const uint ids, const std::wstring& override, uint client)
{
    infocardOverride[ids] = override;
    // TODO: Send update to client
}
