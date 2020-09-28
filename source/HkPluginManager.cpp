#include "Hook.h"
#include "CCmds.h"

const PluginData& PluginHookData::plugin() const {
    return PluginManager::i()->pluginAt(index);
}

void PluginCommunication(PLUGIN_MESSAGE msg, void *data) {
    CallPluginsBefore(HookedCall::PluginCommunication, msg, data);
    CallPluginsAfter(HookedCall::PluginCommunication, msg, data);
}

void PluginManager::clearData(bool free) {
    if(free) {
        for(auto& p : plugins_)
            if(p.mayUnload)
                FreeLibrary(p.dll);
    }
    plugins_.clear();

    for(auto& p : pluginHooks_)
        p.clear();
}

PluginManager::PluginManager() {
    clearData(false);
}

PluginManager::~PluginManager() {
    clearData(false);
}

HK_ERROR PluginManager::pause(size_t hash, bool pause) {
    for (auto &plugin : plugins_) {
        if (plugin.hash == hash) {
            if (!plugin.mayPause)
                return HKE_PLUGIN_UNPAUSABLE;
            plugin.paused = pause;
            return HKE_OK;
        }
    }

    return HKE_PLUGIN_NOT_FOUND;
}

HK_ERROR PluginManager::unload(size_t hash) {
    for (auto plugin = plugins_.begin(); plugin != plugins_.end(); ++plugin) {
        if (plugin->hash == hash) {
            if (!plugin->mayUnload)
                return HKE_PLUGIN_UNLOADABLE;

            FreeLibrary(plugin->dll);

            for (auto& i : pluginHooks_) {
                for (auto hook = i.begin(); hook != i.end(); ++hook) {
                    if (hook->plugin().hash == plugin->hash) {
                        i.erase(hook);
                        break;
                    }
                }
            }

            plugins_.erase(plugin);
            return HKE_OK;
        }
    }

    return HKE_PLUGIN_NOT_FOUND;
}

void PluginManager::unloadAll() {
    clearData(true);
}

void PluginManager::load(const std::wstring &fileName, CCmds *adminInterface, bool startup) {

    std::wstring dllName = fileName;
    if (dllName.find(L".dll") == std::string::npos)
        dllName.append(L".dll");

    for (auto &plugin : plugins_) {
        if (plugin.dllName == dllName)
            return adminInterface->Print(
                L"Plugin %s already loaded, skipping\n",
                plugin.dllName.c_str());
    }

    std::wstring pathToDLL = L"./flhook_plugins/" + dllName;

    FILE *fp;
    _wfopen_s(&fp, pathToDLL.c_str(), L"r");
    if (!fp)
        return adminInterface->Print(L"Error, plugin %s not found\n",
                                     dllName.c_str());
    fclose(fp);

    PluginData plugin;
    plugin.dllName = dllName;
    plugin.dll = LoadLibraryW(pathToDLL.c_str());

    if (!plugin.dll)
        return adminInterface->Print(L"Error, can't load plugin DLL %s\n",
                                     dllName.c_str());

    auto getPluginInfo = reinterpret_cast<ExportPluginInfoT>(GetProcAddress(plugin.dll, "ExportPluginInfo"));

    if (!getPluginInfo) {
        adminInterface->Print(
            L"Error, could not read plugin info (ExportPluginInfo "
            L"not exported?) for %s\n",
            dllName.c_str());
        FreeLibrary(plugin.dll);
        return;
    }

    PluginInfo pi;
    getPluginInfo(&pi);
    
    if (pi.shortName_.empty() || pi.name_.empty()) {
        adminInterface->Print(L"Error, missing name/short name for %s\n",
                              dllName.c_str());
        FreeLibrary(plugin.dll);
        return;
    }

    if (pi.version_ != PLUGIN_API_VERSION) {
        adminInterface->Print(L"Error, incompatible plugin API version for %s: expected %d, got %d\n",
                              dllName.c_str(), PLUGIN_API_VERSION, pi.version_);
        FreeLibrary(plugin.dll);
        return;
    }

    if (pi.returnCode_ == nullptr) {
        adminInterface->Print(L"Error, missing return code pointer %s\n",
                              dllName.c_str());
        FreeLibrary(plugin.dll);
        return;
    }

    plugin.mayPause = pi.mayPause_;
    plugin.mayUnload = pi.mayUnload_;
    plugin.name = pi.name_;
    plugin.shortName = pi.shortName_;
    plugin.hash = std::hash<std::string>{}(plugin.shortName);

    // plugins that may not unload are interpreted as crucial plugins that can
    // also not be loaded after FLServer startup
    if (!plugin.mayUnload && !startup) {
        adminInterface->Print(L"Error, could not load plugin %s: plugin is not unloadable, need "
                              L"server restart to load\n",
                              plugin.dllName.c_str());
        FreeLibrary(plugin.dll);
        return;
    }

    plugins_.push_back(plugin);
    size_t index = plugins_.size();

    for (const auto &hook : pi.hooks_) {
        if(!hook.hookFunction_) {
            adminInterface->Print(L"Error, could not load function %d.%d of plugin %s\n",
                                  hook.targetFunction_, hook.step_, plugin.dllName.c_str());
            continue;
        }
        uint hookId = uint(hook.targetFunction_) * 2 + uint(hook.step_);
        auto& list = pluginHooks_[hookId];
        list.emplace_back(hook.targetFunction_, hook.hookFunction_, hook.step_, hook.priority_, index);
        std::sort(list.begin(), list.end());
    }

    adminInterface->Print(L"Plugin %s loaded (%s)\n",
                          stows(plugin.shortName).c_str(),
                          plugin.dllName.c_str());

}

void PluginManager::loadAll(bool startup, CCmds *adminInterface) {
    WIN32_FIND_DATAW findData;
    HANDLE findPluginsHandle = FindFirstFileW(L"./flhook_plugins/*.dll", &findData);

    do {
        if (findPluginsHandle == INVALID_HANDLE_VALUE)
            break;

        load(findData.cFileName, adminInterface, startup);

    } while (FindNextFileW(findPluginsHandle, &findData));
}