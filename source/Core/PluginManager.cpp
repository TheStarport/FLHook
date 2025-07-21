#include "PCH.hpp"

#include "Core/PluginManager.hpp"

void PluginManager::ClearData(const bool free)
{
    std::set<HMODULE> dlls;
    if (free)
    {
        for (const auto& p : plugins)
        {
            if (p->mayUnload)
            {
                dlls.insert(p->dll);
            }
        }
    }

    for (auto plugin = plugins.begin(); plugin != plugins.end();)
    {
        if (auto dll = plugin->get()->dll; dlls.contains(dll))
        {
            plugin = plugins.erase(plugin);
            FreeLibrary(dll);
        }
        else
        {
            ++plugin;
        }
    }
}

PluginManager::PluginManager() { ClearData(false); }

PluginManager::~PluginManager() { ClearData(false); }

cpp::result<std::wstring, Error> PluginManager::Unload(std::wstring_view name)
{
    const auto pluginIterator = std::ranges::find_if(plugins, [&name](const std::shared_ptr<Plugin>& data) { return name == data->shortName; });

    if (pluginIterator == end())
    {
        return { cpp::fail(Error::PluginNotFound) };
    }

    const auto plugin = pluginIterator->get();

    if (!plugin->mayUnload)
    {
        WARN("Plugin may not be unloaded.");
        return {};
    }

    HMODULE dllAddr = plugin->dll;

    std::wstring unloadedPluginDll = plugin->dllName;
    INFO("Unloading {{plugin}} ({{dllName}})", { "plugin", plugin->name }, { "dllName", plugin->dllName });

    plugins.erase(pluginIterator);

    FreeLibrary(dllAddr);
    return unloadedPluginDll;
}

std::weak_ptr<Plugin> PluginManager::GetPlugin(std::wstring_view shortName)
{
    for (const auto& plugin : plugins)
    {
        if (plugin->shortName == shortName)
        {
            std::weak_ptr weak = plugin;
            return weak;
        }
    }

    return {};
}

void PluginManager::UnloadAll() { ClearData(true); }

bool PluginManager::Load(std::wstring_view fileName, bool startup)
{
    auto dllName = std::wstring(fileName);
    if (dllName.find(L".dll") == std::wstring::npos)
    {
        dllName.append(L".dll");
    }

    for (const auto& plugin : plugins)
    {
        if (plugin->dllName == dllName)
        {
            INFO("{{plugin}} already loaded, skipping", { "plugin", plugin->dllName });

            return false;
        }
    }

    const std::wstring pathToDll = L"./plugins/" + dllName;

    if (!std::filesystem::exists(pathToDll))
    {
        ERROR("ERR plugin {{dllName}} not found", { "dllName", dllName });
        return false;
    }

    const HMODULE dll = LoadLibraryW(pathToDll.c_str());

    if (!dll)
    {
        ERROR("ERR can't load plugin DLL {{dllName}}", { "dllName", dllName });
        return false;
    }

    const auto pluginFactory = reinterpret_cast<PluginFactoryT>(GetProcAddress(dll, "PluginFactory"));

    if (!pluginFactory)
    {
        ERROR("ERR could not create plugin instance for {{dllName}}", { "dllName", dllName });

        FreeLibrary(dll);
        return false;
    }

    auto plugin = pluginFactory();
    plugin->dll = dll;
    plugin->dllName = dllName;

    if (plugin->versionMinor == PluginMinorVersion::Undefined || plugin->versionMajor == PluginMajorVersion::Undefined)
    {
        ERROR("ERR plugin {{dllName}} does not have defined API version. Unloading.", { "dllName", dllName });
        FreeLibrary(dll);
        return false;
    }

    if (plugin->versionMajor != CurrentMajorVersion)
    {
        ERROR("ERR incompatible plugin API (major) version for {{dllName}}: expected {{currentMajor}}, got {{major}}",
              { "dllName", dllName },
              { "currentMajor", static_cast<int>(CurrentMajorVersion) },
              { "major", static_cast<int>(plugin->versionMajor) });

        plugin = nullptr;
        FreeLibrary(dll);
        return false;
    }

    if (static_cast<int>(plugin->versionMinor) > static_cast<int>(CurrentMinorVersion))
    {
        ERROR("ERR incompatible plugin API (minor) version for {{dllName}}: expected {{currentMinor}}, got {{minor}}",
              { "dllName", dllName },
              { "currentMinor", static_cast<int>(CurrentMinorVersion) },
              { "minor", static_cast<int>(plugin->versionMinor) });

        plugin = nullptr;
        FreeLibrary(dll);
        return false;
    }

    if (static_cast<int>(plugin->versionMinor) != static_cast<int>(CurrentMinorVersion))
    {

        WARN("incompatible plugin API (minor) version for {{dllName}}: expected {{currentMinor}}, got {{minor}}",
             { "dllName", dllName },
             { "currentMinor", static_cast<int>(CurrentMinorVersion) },
             { "minor", static_cast<int>(plugin->versionMinor) });

        INFO("Processing will continue, but plugin should be considered unstable.");
    }

    if (plugin->shortName.empty() || plugin->name.empty())
    {
        ERROR("ERR missing name/short name for {{dllName}}", { "dllName", dllName });
        plugin = nullptr;
        FreeLibrary(dll);
        return false;
    }

    // plugins that may not unload are interpreted as crucial plugins that can
    // also not be loaded after FLServer startup
    if (!plugin->mayUnload && !startup)
    {
        ERROR("ERR could not load plugin {{dllName}}: plugin cannot be unloaded, need server restart to load", { "dllName", dllName });
        plugin = nullptr;
        FreeLibrary(dll);
        return false;
    }

    plugins.emplace_back(plugin);

    const std::weak_ptr weakRef = plugins.back();
    if (const auto userCmdPtr = std::dynamic_pointer_cast<AbstractUserCommandProcessor>(weakRef.lock()); userCmdPtr)
    {
        userCommands.emplace_back(std::weak_ptr(userCmdPtr));
    }

    if (const auto userCmdPtr = std::dynamic_pointer_cast<AbstractAdminCommandProcessor>(weakRef.lock()); userCmdPtr)
    {
        adminCommands.emplace_back(std::weak_ptr(userCmdPtr));
    }

    std::ranges::sort(plugins, [](const std::shared_ptr<Plugin>& a, const std::shared_ptr<Plugin>& b) { return a->callPriority > b->callPriority; });

    INFO("Plugin Loaded {{dll}}", { "dll", plugin->dllName });
    return true;
}

void PluginManager::LoadAll(bool startup)
{
    WIN32_FIND_DATAW findData;
    const HANDLE findPluginsHandle = FindFirstFileW(L"./plugins/*.dll", &findData);

    const static std::unordered_set<std::wstring> blackList = { L"bson-1.0.dll", L"bsoncxx.dll" };

    do
    {
        if (findPluginsHandle == INVALID_HANDLE_VALUE)
        {
            break;
        }

        if (findData.cFileName && !blackList.contains(findData.cFileName))
        {
            Load(findData.cFileName, startup);
        }
    }
    while (FindNextFileW(findPluginsHandle, &findData));
}
