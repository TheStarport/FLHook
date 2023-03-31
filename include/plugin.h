#pragma once

using UserCmdProc = std::function<void(ClientId& client)>;
using UserCmdProcWithParam = std::function<void(ClientId& client, const std::wstring& param)>;

struct UserCommand
{
	std::variant<std::wstring, std::vector<std::wstring>> command;
	std::wstring usage;
	std::variant<UserCmdProc, UserCmdProcWithParam> proc;
	std::wstring description;
};

struct Timer
{
	std::function<void()> func;
	mstime intervalInSeconds;
	mstime lastTime = 0;
};

constexpr PluginMajorVersion CurrentMajorVersion = PluginMajorVersion::VERSION_04;
constexpr PluginMinorVersion CurrentMinorVersion = PluginMinorVersion::VERSION_01;

const std::wstring VersionInformation = std::to_wstring(static_cast<int>(CurrentMajorVersion)) + L"." + std::to_wstring(static_cast<int>(CurrentMinorVersion));

struct PluginHook
{
	using FunctionType = void();

	HookedCall targetFunction;
	FunctionType* hookFunction;
	HookStep step;
	int priority;

	template<typename Func>
	PluginHook(const HookedCall targetFunction, Func* hookFunction, const HookStep step = HookStep::Before, const int priority = 0)
	    : targetFunction(targetFunction), hookFunction(reinterpret_cast<FunctionType*>(hookFunction)), step(step), priority(priority)
	{
		switch (step)
		{
			case HookStep::Before:
				if (targetFunction == HookedCall::FLHook__LoadSettings)
					throw std::invalid_argument("Load settings can only be called HookStep::After.");
				break;
			case HookStep::After:
				break;
			default:;
		}
	}

	friend class PluginManager;
};

#define PluginCall(name, ...) (*(name))(__VA_ARGS__)

// Inherit from this to define a IPC (Inter-Plugin Communication) class.
class DLL PluginCommunicator
{
  public:
	using EventSubscription = void (*)(int id, void* dataPack);
	void Dispatch(int id, void* dataPack) const;
	void AddListener(std::string plugin, EventSubscription event);

	std::string plugin;

	explicit PluginCommunicator(std::string plugin) : plugin(std::move(plugin)) {}

	static void ExportPluginCommunicator(PluginCommunicator* communicator);
	static PluginCommunicator* ImportPluginCommunicator(std::string plugin, EventSubscription subscription = nullptr);

  private:
	std::map<std::string, EventSubscription> listeners;
};

struct PluginInfo
{
	std::string name;
	std::string shortName;
	PluginMajorVersion versionMajor = PluginMajorVersion::UNDEFINED;
	PluginMinorVersion versionMinor = PluginMinorVersion::UNDEFINED;
	bool mayUnload;

	PluginInfo() = delete;
	PluginInfo(
	    const std::string& name, const std::string& shortName, const PluginMajorVersion major, const PluginMinorVersion minor, const bool mayUnload = true)
	    : name(name), shortName(shortName), versionMajor(major), versionMinor(minor), mayUnload(mayUnload)
	{
	}
};

class DLL Plugin
{
	friend PluginManager;

	PluginMajorVersion versionMajor = PluginMajorVersion::UNDEFINED;
	PluginMinorVersion versionMinor = PluginMinorVersion::UNDEFINED;

	std::string name;
	std::string shortName;
	bool mayUnload;

	std::vector<PluginHook> hooks;
	std::vector<UserCommand> commands;
	HMODULE dll = nullptr;
	std::string dllName;

  protected:
	ReturnCode returnCode = ReturnCode::Default;

	std::vector<Timer> timers;

  public:
	explicit Plugin(const PluginInfo& info)
	{
		name = info.name;
		shortName = info.shortName;
		mayUnload = info.mayUnload;
		versionMajor = info.versionMajor;
		versionMinor = info.versionMinor;
	}

	virtual ~Plugin() = default;
	Plugin& operator=(const Plugin&&) = delete;
	Plugin& operator=(const Plugin&) = delete;

	template<typename... Args>
	void EmplaceHook(Args&&... args)
	{
		PluginHook ph(std::forward<Args>(args)...);
		if (std::ranges::find_if(hooks, [ph](const PluginHook& hook) { return hook.targetFunction == ph.targetFunction && ph.step == hook.step; }) !=
		    hooks.end())
		{
			return;
		}

		hooks.emplace_back(ph);
	}

	void RemoveHook(HookedCall target, HookStep step)
	{
		std::erase_if(hooks, [target, step](const PluginHook& hook) { return hook.targetFunction == target && step == hook.step; });
	}

	void AddCommand(
	    const std::variant<std::wstring, std::vector<std::wstring>>& command, const std::wstring& usage, UserCmdProc proc, const std::wstring& description)
	{
		commands.emplace_back(UserCommand(command, usage, proc, description));
	}

	void RemoveCommand(const std::wstring& cmd)
	{
		std::erase_if(
		    commands, [cmd](const UserCommand& userCommand) { return !userCommand.command.index() && std::get<std::wstring>(userCommand.command) == cmd; });
	}

	[[nodiscard]] std::string_view GetName() const { return name; }
	[[nodiscard]] std::string_view GetShortName() const { return shortName; }
	[[nodiscard]] auto& GetCommands() const { return commands; }
	[[nodiscard]] auto& GetTimers() { return timers; }
};

#define SetupPlugin(type, info)                             \
	extern "C" EXPORT std::shared_ptr<type> PluginFactory() \
	{                                                       \
		return std::move(std::make_shared<type>(info));      \
	}

#define Cmd(func)                      \
	[this](const ClientId& clientId) { \
		func(clientId);                \
	}
#define CmdP(func)                                                \
	[this](const ClientId& clientId, const std::wstring& param) { \
		func(clientId, param);                                    \
	}