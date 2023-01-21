#include "Global.hpp"

const PluginData& PluginHookData::plugin() const
{
	return PluginManager::i()->pluginAt(index);
}

// Map of plugins to their relative communicators, if they have any.
static std::map<std::string, PluginCommunicator*> pluginCommunicators;
void PluginCommunicator::ExportPluginCommunicator(PluginCommunicator* communicator)
{
	pluginCommunicators[communicator->plugin] = communicator;
}

void PluginCommunicator::Dispatch(int id, void* dataPack) const
{
	for (const auto& i : listeners)
	{
		i.second(id, dataPack);
	}
}

void PluginCommunicator::AddListener(const std::string plugin, EventSubscription event)
{
	this->listeners[plugin] = event;
}

PluginCommunicator* PluginCommunicator::ImportPluginCommunicator(const std::string plugin, PluginCommunicator::EventSubscription subscription)
{
	const auto el = pluginCommunicators.find(plugin);
	if (el == pluginCommunicators.end())
		return nullptr;

	if (subscription != nullptr)
	{
		el->second->AddListener(plugin, subscription);
	}
	return el->second;
}

void PluginManager::clearData(bool free)
{
	if (free)
	{
		for (auto& p : plugins_)
			if (p.mayUnload)
				FreeLibrary(p.dll);
	}
	plugins_.clear();

	for (auto& p : pluginHooks_)
		p.clear();
}

PluginManager::PluginManager()
{
	clearData(false);
	setupProps();
}

PluginManager::~PluginManager()
{
	clearData(false);
}

cpp::result<void, Error> PluginManager::unload(const std::string& name)
{
	const auto plugin = std::find_if(begin(), end(), [name](const PluginData& data) {
		return name == data.shortName;
	});

	if (plugin == end())
		return cpp::fail(Error::PluginNotFound);

	if (!plugin->mayUnload)
	{
		Console::ConWarn("Plugin may not be unloaded.");
		return {};
	}

	HMODULE dllAddr = plugin->dll;

	Console::ConPrint(std::format("Unloading {} ({})", plugin->name, wstos(plugin->dllName)));
	plugins_.erase(plugin);
	FreeLibrary(dllAddr);
	return {};
}

void PluginManager::unloadAll()
{
	clearData(true);
}

void PluginManager::load(const std::wstring& fileName, CCmds* adminInterface, bool startup)
{
	std::wstring dllName = fileName;
	if (dllName.find(L".dll") == std::string::npos)
		dllName.append(L".dll");

	for (auto& plugin : plugins_)
	{
		if (plugin.dllName == dllName)
			return adminInterface->Print(std::format("Plugin {} already loaded, skipping\n", wstos(plugin.dllName)));
	}

	std::wstring pathToDLL = L"./plugins/" + dllName;

	FILE* fp;
	_wfopen_s(&fp, pathToDLL.c_str(), L"r");
	if (!fp)
		return adminInterface->Print(std::format("ERR plugin {} not found", wstos(dllName)));
	fclose(fp);

	PluginData plugin;
	plugin.dllName = dllName;
	plugin.dll = LoadLibraryW(pathToDLL.c_str());

	if (!plugin.dll)
		return adminInterface->Print(std::format("ERR can't load plugin DLL {}", wstos(plugin.dllName)));

	auto getPluginInfo = reinterpret_cast<ExportPluginInfoT>(GetProcAddress(plugin.dll, "ExportPluginInfo"));

	if (!getPluginInfo)
	{
		adminInterface->Print(std::format("ERR could not read plugin info (ExportPluginInfo not exported?) for {}", wstos(plugin.dllName)));
		FreeLibrary(plugin.dll);
		return;
	}

	std::shared_ptr<PluginInfo> pi = std::make_shared<PluginInfo>();
	getPluginInfo(pi.get());

	if (pi->versionMinor_ == PluginMinorVersion::UNDEFINED || pi->versionMajor_ == PluginMajorVersion::UNDEFINED)
	{
		adminInterface->Print(std::format("ERR plugin {} does not have defined API version. Unloading.", wstos(plugin.dllName)));
		FreeLibrary(plugin.dll);
		return;
	}

	if (pi->versionMajor_ != CurrentMajorVersion)
	{
		adminInterface->Print(std::format(
		    "ERR incompatible plugin API (major) version for {}: expected {}, got {}", wstos(plugin.dllName), (int)CurrentMajorVersion, (int)pi->versionMajor_));
		FreeLibrary(plugin.dll);
		return;
	}

	if ((int)pi->versionMinor_ > (int)CurrentMinorVersion)
	{
		adminInterface->Print(std::format(
		    "ERR incompatible plugin API (minor) version for {}: expected {} or lower, got {}", wstos(plugin.dllName), (int)CurrentMinorVersion, (int)pi->versionMinor_));
		FreeLibrary(plugin.dll);
		return;
	}

	if (int(pi->versionMinor_) != (int)CurrentMinorVersion)
	{
		adminInterface->Print(
		    std::format("Warning, incompatible plugin API version for {}: expected {}, got {}", wstos(plugin.dllName), (int)CurrentMinorVersion, (int)pi->versionMinor_));
		adminInterface->Print("Processing will continue, but plugin should be considered unstable.");
	}

	if (pi->shortName_.empty() || pi->name_.empty())
	{
		adminInterface->Print(std::format("ERR missing name/short name for {}", wstos(plugin.dllName)));
		FreeLibrary(plugin.dll);
		return;
	}

	if (pi->returnCode_ == nullptr)
	{
		adminInterface->Print(std::format("ERR missing return code pointer {}", wstos(plugin.dllName)));
		FreeLibrary(plugin.dll);
		return;
	}

	plugin.mayUnload = pi->mayUnload_;
	plugin.name = pi->name_;
	plugin.shortName = pi->shortName_;
	plugin.resetCode = pi->resetCode_;
	plugin.returnCode = pi->returnCode_;

	// plugins that may not unload are interpreted as crucial plugins that can
	// also not be loaded after FLServer startup
	if (!plugin.mayUnload && !startup)
	{
		adminInterface->Print(std::format("ERR could not load plugin {}: plugin cannot be unloaded, need server restart to load", wstos(plugin.dllName)));
		FreeLibrary(plugin.dll);
		return;
	}

	plugin.timers = pi->timers_;
	plugin.commands = pi->commands_;

	size_t index = plugins_.size();
	plugins_.push_back(plugin);

	for (const auto& hook : pi->hooks_)
	{
		if (!hook.hookFunction_)
		{
			adminInterface->Print(std::format("ERR could not load function. has step {} of plugin {}", magic_enum::enum_name(hook.step_), wstos(plugin.dllName)));
			continue;
		}

		if (const auto& targetHookProps = hookProps_[hook.targetFunction_]; !targetHookProps.matches(hook.step_))
		{
			adminInterface->Print(
			    std::format("ERR could not bind function. plugin: {}, step not available", wstos(plugin.dllName)));
			continue;
		}

		uint hookId = uint(hook.targetFunction_) * uint(HookStep::Count) + uint(hook.step_);
		auto& list = pluginHooks_[hookId];
		list.push_back({hook.targetFunction_, hook.hookFunction_, hook.step_, hook.priority_, index});
		std::sort(list.begin(), list.end());
	}

	// Allocate some space in our client info block
	for (auto& i : ClientInfo)
	{
		i.mapPluginData[pi.get()] = std::array<uchar, 40>();
	}

	plugin.pInfo = std::move(pi);

	adminInterface->Print(std::format("Plugin {} loaded ({})", plugin.shortName, wstos(plugin.dllName)));
}

void PluginManager::loadAll(bool startup, CCmds* adminInterface)
{
	WIN32_FIND_DATAW findData;
	HANDLE findPluginsHandle = FindFirstFileW(L"./plugins/*.dll", &findData);

	do
	{
		if (findPluginsHandle == INVALID_HANDLE_VALUE)
			break;

		load(findData.cFileName, adminInterface, startup);

	} while (FindNextFileW(findPluginsHandle, &findData));
}

void PluginManager::setProps(HookedCall c, bool b, bool m, bool a)
{
	hookProps_[c] = {b, m, a};
}

void PluginInfo::versionMajor(PluginMajorVersion version)
{
	versionMajor_ = version;
}

void PluginInfo::versionMinor(PluginMinorVersion version)
{
	versionMinor_ = version;
}

void PluginInfo::name(const char* name)
{
	name_ = name;
}

void PluginInfo::shortName(const char* shortName)
{
	shortName_ = shortName;
}

void PluginInfo::mayUnload(bool unload)
{
	mayUnload_ = unload;
}

void PluginInfo::autoResetCode(bool reset)
{
	resetCode_ = reset;
}

void PluginInfo::returnCode(ReturnCode* returnCode)
{
	returnCode_ = returnCode;
}

void PluginInfo::addHook(const PluginHook& hook)
{
	hooks_.push_back(hook);
}

void PluginInfo::commands(const std::vector<UserCommand>* cmds)
{
	commands_ = const_cast<std::vector<UserCommand>*>(cmds);

}

void PluginInfo::timers(const std::vector<Timer>* timers)
{
	timers_ = const_cast<std::vector<Timer>*>(timers);
}
