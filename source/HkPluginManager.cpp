#include "Hook.h"
#include "CCmds.h"

const PluginData& PluginHookData::plugin() const {
    return PluginManager::i()->pluginAt(index);
}

// Map of plugins to their relative communicators, if they have any.
static std::map<std::string, PluginCommunicator *> pluginCommunicators;
void PluginCommunicator::ExportPluginCommunicator(PluginCommunicator *communicator) {
    pluginCommunicators[communicator->plugin] = communicator;
}

void PluginCommunicator::Dispatch(int id, void *dataPack) const {
    for (const auto &i : listeners) {
        i.second(id, dataPack);
    }
}

void PluginCommunicator::AddListener(const std::string plugin, EventSubscription event){
    this->listeners[plugin] = event;
}

PluginCommunicator *PluginCommunicator::ImportPluginCommunicator(const std::string plugin, PluginCommunicator::EventSubscription subscription) {
    const auto el = pluginCommunicators.find(plugin);
    if (el == pluginCommunicators.end())
        return nullptr;

    if (subscription != nullptr) {
        el->second->AddListener(plugin, subscription);   
    }
    return el->second;
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
    setupProps();
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
        return adminInterface->Print(L"ERR plugin %s not found",
                                     dllName.c_str());
    fclose(fp);

    PluginData plugin;
    plugin.dllName = dllName;
    plugin.dll = LoadLibraryW(pathToDLL.c_str());

    if (!plugin.dll)
        return adminInterface->Print(L"ERR can't load plugin DLL %s",
                                     dllName.c_str());

    auto getPluginInfo = reinterpret_cast<ExportPluginInfoT>(GetProcAddress(plugin.dll, "ExportPluginInfo"));

    if (!getPluginInfo) {
        adminInterface->Print(L"ERR could not read plugin info (ExportPluginInfo not exported?) for %s", dllName.c_str());
        FreeLibrary(plugin.dll);
        return;
    }

    std::shared_ptr<PluginInfo> pi = std::make_shared<PluginInfo>();
    getPluginInfo(pi.get());

    if (pi->versionMinor_ == PluginMinorVersion::UNDEFINED || pi->versionMajor_ == PluginMajorVersion::UNDEFINED) {
        adminInterface->Print(L"ERR plugin does not have defined API version. Unloading.",
                              dllName.c_str(), CurrentMajorVersion, pi->versionMajor_);
        FreeLibrary(plugin.dll);
        return;
    }

    if (pi->versionMajor_ != CurrentMajorVersion) {
        adminInterface->Print(L"ERR incompatible plugin API (major) version for %s: expected %d, got %d",
                              dllName.c_str(), CurrentMajorVersion, pi->versionMajor_);
        FreeLibrary(plugin.dll);
        return;
    }

    if ((int)pi->versionMinor_ > (int)CurrentMinorVersion) {
        adminInterface->Print(L"ERR incompatible plugin API (minor) version for %s: expected %d or lower, got %d",
                              dllName.c_str(), CurrentMinorVersion, pi->versionMinor_);
        FreeLibrary(plugin.dll);
        return;
    }

    if (int(pi->versionMinor_) != (int)CurrentMinorVersion) {
        adminInterface->Print(L"Warning, incompatible plugin API version for %s: expected %d, got %d",
                              dllName.c_str(), CurrentMinorVersion, pi->versionMinor_);
        adminInterface->Print(L"Processing will continue, but plugin should be considered unstable.");
    }
    
    if (pi->shortName_.empty() || pi->name_.empty()) {
        adminInterface->Print(L"ERR missing name/short name for %s",
                              dllName.c_str());
        FreeLibrary(plugin.dll);
        return;
    }

    if (pi->returnCode_ == nullptr) {
        adminInterface->Print(L"ERR missing return code pointer %s",
                              dllName.c_str());
        FreeLibrary(plugin.dll);
        return;
    }

    plugin.mayPause = pi->mayPause_;
    plugin.mayUnload = pi->mayUnload_;
    plugin.name = pi->name_;
    plugin.shortName = pi->shortName_;
    plugin.hash = std::hash<std::string>{}(plugin.shortName);
    plugin.resetCode = pi->resetCode_;
    plugin.returnCode = pi->returnCode_;

    // plugins that may not unload are interpreted as crucial plugins that can
    // also not be loaded after FLServer startup
    if (!plugin.mayUnload && !startup) {
        adminInterface->Print(L"ERR could not load plugin %s: plugin cannot be unloaded, need server restart to load",
                              plugin.dllName.c_str());
        FreeLibrary(plugin.dll);
        return;
    }

    size_t index = plugins_.size();
    plugins_.push_back(plugin);

    for (const auto &hook : pi->hooks_) {
        if(!hook.hookFunction_) {
            adminInterface->Print(L"ERR could not load function %d.%d of plugin %s",
                                  hook.targetFunction_, hook.step_, plugin.dllName.c_str());
            continue;
        }

        const auto& targetHookProps = hookProps_[hook.targetFunction_];
        if(!targetHookProps.matches(hook.step_)) {
            adminInterface->Print(L"ERR could not bind function %d.%d of plugin %s, step not available",
                                  hook.targetFunction_, hook.step_, plugin.dllName.c_str());
            continue;
        }

        uint hookId = uint(hook.targetFunction_) * uint(HookStep::Count) + uint(hook.step_);
        auto& list = pluginHooks_[hookId];
        list.push_back({ hook.targetFunction_, hook.hookFunction_, hook.step_, hook.priority_, index });
        std::sort(list.begin(), list.end());
    }

    // Allocate some space in our client info block
    for (auto &i : ClientInfo) {
        i.mapPluginData[pi.get()] = std::array<uchar, 40>();
    }

    plugin.pInfo = std::move(pi);

    adminInterface->Print(L"Plugin %s loaded (%s)",
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

void PluginManager::setProps(HookedCall c, bool b, bool m, bool a) {
    hookProps_[c] = { b, m, a };
}


void PluginInfo::versionMajor(PluginMajorVersion version) {
    versionMajor_ = version;
}

void PluginInfo::versionMinor(PluginMinorVersion version) {
    versionMinor_ = version;
}

void PluginInfo::name(const char *name) {
    name_ = name;
}

void PluginInfo::shortName(const char *shortName) {
    shortName_ = shortName;
}

void PluginInfo::mayPause(bool pause) {
    mayPause_ = pause;
}

void PluginInfo::mayUnload(bool unload) {
    mayUnload_ = unload;
}

void PluginInfo::autoResetCode(bool reset) {
    resetCode_ = reset;
}


void PluginInfo::returnCode(ReturnCode *returnCode) {
    returnCode_ = returnCode;
}

void PluginInfo::addHook(const PluginHook &hook) {
    hooks_.push_back(hook);
}