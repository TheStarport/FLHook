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

        std::string dll = ini.get_value_string();
        hDll = LoadLibraryExA(dll.c_str(), nullptr, LOAD_LIBRARY_AS_DATAFILE);
        if (hDll)
        {
            Logger::Debug(std::format(L"Loaded Resource DLL: {}", StringUtils::stows(dll)));
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

        Logger::Debug(std::format(L"Unloaded Resource DLL: {}", buffer.substr(buffer.find('\\') + 1)));
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
