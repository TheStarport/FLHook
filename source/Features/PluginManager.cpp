#include "PCH.hpp"
#include "Global.hpp"

// Map of plugins to their relative communicators, if they have any.
static std::map<std::wstring, PluginCommunicator*> pluginCommunicators;

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

void PluginCommunicator::AddListener(const std::wstring plugin, EventSubscription event)
{
	this->listeners[plugin] = event;
}

PluginCommunicator* PluginCommunicator::ImportPluginCommunicator(const std::wstring plugin, EventSubscription subscription)
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

void PluginManager::ClearData(bool free)
{
	if (free)
	{
		for (const auto& p : plugins)
			if (p->mayUnload)
				FreeLibrary(p->dll);
	}
	plugins.clear();

	for (auto& p : pluginHooks_)
		p.clear();
}

PluginManager::PluginManager()
{
	ClearData(false);
	setupProps();
}

PluginManager::~PluginManager()
{
	ClearData(false);
}

cpp::result<std::wstring, Error> PluginManager::Unload(std::wstring_view name)
{
	const auto pluginIterator = std::ranges::find_if(plugins, [&name](const std::shared_ptr<Plugin>& data) { return name == data->shortName; });

	if (pluginIterator == end())
		return cpp::fail(Error::PluginNotFound);

	const auto plugin = pluginIterator->get();

	if (!plugin->mayUnload)
	{
		Logger::i()->Log(LogLevel::Warn, L"Plugin may not be unloaded.");
		return {};
	}

	HMODULE dllAddr = plugin->dll;

	std::wstring unloadedPluginDll = plugin->dllName;
	Logger::i()->Log(LogLevel::Info, std::format(L"Unloading {} ({})", plugin->name, plugin->dllName));

	for (const auto& hook : plugin->hooks)
	{
		const uint hookId = static_cast<uint>(hook.targetFunction) * static_cast<uint>(magic_enum::enum_count<HookStep>()) + static_cast<uint>(hook.step);
		auto& list = pluginHooks_[hookId];

		std::erase_if(list, [dllAddr](const PluginHookData& x) { return x.plugin.lock()->dll == dllAddr; });
	}

	plugins.erase(pluginIterator);

	FreeLibrary(dllAddr);
	return unloadedPluginDll;
}

void PluginManager::UnloadAll()
{
	ClearData(true);
}

//TODO: Let this return a bool or error type.
void PluginManager::Load(std::wstring_view fileName, bool startup)
{
	std::wstring dllName = std::wstring(fileName);
	if (dllName.find(L".dll") == std::wstring::npos)
		dllName.append(L".dll");

	for (const auto& plugin : plugins)
	{
		if (plugin->dllName == dllName)
			return Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Info, std::format(L"Plugin {} already loaded, skipping\n", plugin->dllName));
	}

	const std::wstring pathToDLL = L"./plugins/" + dllName;

	if (!std::filesystem::exists(pathToDLL))
	{
		Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Err, std::format(L"ERR plugin {} not found", dllName));
		return;
	}

	HMODULE dll = LoadLibraryW(pathToDLL.c_str());

	if (!dll)
	{
		Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Err, std::format(L"ERR can't load plugin DLL {}", dllName));
	}

	const auto pluginFactory = reinterpret_cast<PluginFactoryT>(GetProcAddress(dll, "PluginFactory"));

	if (!pluginFactory)
	{
		Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Err, std::format(L"ERR could not create plugin instance for {}", dllName));
		FreeLibrary(dll);
		return;
	}

	const auto plugin = pluginFactory();
	plugin->dll = dll;
	plugin->dllName = dllName;

	if (plugin->versionMinor == PluginMinorVersion::UNDEFINED || plugin->versionMajor == PluginMajorVersion::UNDEFINED)
	{
		Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Err, std::format(L"ERR plugin {} does not have defined API version. Unloading.", dllName));
		FreeLibrary(dll);
		return;
	}

	if (plugin->versionMajor != CurrentMajorVersion)
	{
		Logger::i()->Log(LogFile::ConsoleOnly,
		    LogLevel::Err,
		    std::format(L"ERR incompatible plugin API (major) version for {}: expected {}, got {}",
		        dllName,
		        static_cast<int>(CurrentMajorVersion),
		        static_cast<int>(plugin->versionMajor)));
		FreeLibrary(dll);
		return;
	}

	if (static_cast<int>(plugin->versionMinor) > static_cast<int>(CurrentMinorVersion))
	{
		Logger::i()->Log(LogFile::ConsoleOnly,
		    LogLevel::Err,
		    std::format(L"ERR incompatible plugin API (minor) version for {}: expected {} or lower, got {}",
		        dllName,
		        static_cast<int>(CurrentMinorVersion),
		        static_cast<int>(plugin->versionMinor)));
		FreeLibrary(dll);
		return;
	}

	if (static_cast<int>(plugin->versionMinor) != static_cast<int>(CurrentMinorVersion))
	{
		Logger::i()->Log(LogFile::ConsoleOnly,
		    LogLevel::Warn,
		    std::format(L"Warning, incompatible plugin API version for {}: expected {}, got {}",
		        dllName,
		        static_cast<int>(CurrentMinorVersion),
		        static_cast<int>(plugin->versionMinor)));
		Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Info, L"Processing will continue, but plugin should be considered unstable.");
	}

	if (plugin->shortName.empty() || plugin->name.empty())
	{
		Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Err, std::format(L"ERR missing name/short name for {}", dllName));
		FreeLibrary(dll);
		return;
	}

	// plugins that may not unload are interpreted as crucial plugins that can
	// also not be loaded after FLServer startup
	if (!plugin->mayUnload && !startup)
	{
		Logger::i()->Log(
		    LogFile::ConsoleOnly, LogLevel::Err, std::format(L"ERR could not load plugin {}: plugin cannot be unloaded, need server restart to load", dllName));
		FreeLibrary(dll);
		return;
	}

	for (const auto& hook : plugin->hooks)
	{
		if (!hook.hookFunction)
		{
			Logger::i()->Log(LogFile::ConsoleOnly,
			    LogLevel::Err,
			    std::format(L"ERR could not load function. has step {} of plugin {}", magic_enum::enum_name(hook.step), dllName));
			continue;
		}

		if (const auto& targetHookProps = hookProps_[hook.targetFunction]; !targetHookProps.matches(hook.step))
		{
			Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Warn, std::format(L"ERR could not bind function. plugin: {}, step not available", dllName));
			continue;
		}

		uint hookId = static_cast<uint>(hook.targetFunction) * magic_enum::enum_count<HookStep>() + static_cast<uint>(hook.step);
		std::vector<PluginHookData>& list = pluginHooks_[hookId];
		PluginHookData data = { hook.targetFunction, hook.hookFunction, hook.step, hook.priority, plugin };
		list.emplace_back(std::move(data));
	}

	plugins.emplace_back(plugin);

	std::ranges::sort(plugins, [](const std::shared_ptr<Plugin>& a, std::shared_ptr<Plugin>& b) { return a->name < b->name; });

	Logger::i()->Log(LogFile::ConsoleOnly, LogLevel::Info, std::format(L"Plugin {} loaded ({})", plugin->shortName, plugin->dllName));
}

void PluginManager::LoadAll(bool startup)
{
	WIN32_FIND_DATAW findData;
	const HANDLE findPluginsHandle = FindFirstFileW(L"./plugins/*.dll", &findData);

	do
	{
		if (findPluginsHandle == INVALID_HANDLE_VALUE)
			break;

		Load(findData.cFileName, startup);
	} while (FindNextFileW(findPluginsHandle, &findData));
}

void PluginManager::SetProps(HookedCall c, bool before, bool after)
{
	hookProps_[c] = {before, after};
}
