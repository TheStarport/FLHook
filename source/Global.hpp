#pragma once

#include <FLHook.hpp>

bool FLHookInit();
void FLHookInit_Pre();
void FLHookShutdown();
void LoadSettings();
void ProcessPendingCommands();

void PatchClientImpl();
bool InitHookExports();
void UnloadHookExports();
void LoadUserCharSettings(ClientId client);

void ClearClientInfo(ClientId client);
void LoadUserSettings(ClientId client);

bool UserCmd_Process(ClientId client, const std::wstring& Cmd);

bool AllowPlayerDamage(ClientId client, ClientId clientTarget);

// FuncCache
namespace StartupCache
{
	void Init();
	void Done();
} // namespace StartupCache

namespace Hk
{
	namespace Ini
	{
		// Ini processing functions
		void CharacterInit();
		void CharacterShutdown();
		void CharacterClearClientInfo(ClientId client);
		void CharacterSelect(CHARACTER_ID charId, ClientId client);
	} // namespace Ini

	namespace Personalities
	{
		void LoadPersonalities();
	}

	namespace Client
	{
		uint ExtractClientID(const std::variant<uint, std::wstring>& player);
		cpp::result<CAccount*, Error> ExtractAccount(const std::variant<uint, std::wstring>& player);
	} // namespace Client
}     // namespace Hk

// Death
void Naked__ShipDestroyed();

// Dmg
void Naked__GuidedHit();
void Naked__AddDamageEntry();
void Naked__NonGunWeaponHitsBase();
void Naked__AddDamageEntry();
void Naked__DamageHit();
void Naked__DamageHit2();
void Naked__DisconnectPacketSent();

// Timers
void TimerTempBanCheck();
void TimerCheckKick();
void TimerNPCAndF1Check();
void ThreadResolver();
void TimerCheckResolveResults();

void BaseDestroyed(uint objectId, ClientId clientBy);

extern HookEntry IServerImplEntries[73];

// DataBaseMarket
bool LoadBaseMarket();

extern CRITICAL_SECTION csIPResolve;
extern std::list<RESOLVE_IP> g_ResolveIPs;
extern std::list<RESOLVE_IP> g_ResolveIPsResult;
extern HANDLE hThreadResolver;

// help

void IClientImpl__Startup__Inner(uint dunno, uint dunno2);

inline auto* ToWChar(const ushort* val)
{
	return reinterpret_cast<const wchar_t*>(val);
}

inline auto* ToWChar(ushort* val)
{
	return reinterpret_cast<wchar_t*>(val);
}

inline auto* ToUShort(const wchar_t* val)
{
	return reinterpret_cast<const ushort*>(val);
}

inline auto* ToUShort(wchar_t* val)
{
	return reinterpret_cast<ushort*>(val);
}

#define CALL_SERVER_PREAMBLE                    \
	{                                           \
		static CTimer timer(__FUNCTION__, 100); \
		timer.start();                          \
		TRY_HOOK                                \
		{
#define CALL_SERVER_POSTAMBLE(catchArgs, rval)                                                               \
	}                                                                                                        \
	CATCH_HOOK({                                                                                             \
		Logger::i()->Log(LogLevel::Err, std::format("Exception in {} on server call", __FUNCTION__)); \
		bool ret = catchArgs;                                                                                \
		if (!ret)                                                                                            \
		{                                                                                                    \
			timer.stop();                                                                                    \
			return rval;                                                                                     \
		}                                                                                                    \
	})                                                                                                       \
	timer.stop();                                                                                            \
	}

#define CALL_CLIENT_PREAMBLE      \
	{                             \
		void* vRet;               \
		char* tmp;                \
		memcpy(&tmp, &Client, 4); \
		memcpy(&Client, &OldClient, 4);

#define CALL_CLIENT_POSTAMBLE \
	__asm { mov [vRet], eax}   \
	memcpy(&Client, &tmp, 4); \
	}

#define CHECK_FOR_DISCONNECT                                                                                                       \
	{                                                                                                                              \
		if (ClientInfo[client].disconnected)                                                                                      \
		{                                                                                                                          \
			Logger::i()->Log(LogLevel::Debug, std::format("Ignoring disconnected client in {} id={}", __FUNCTION__, client)); \
			return;                                                                                                                \
		};                                                                                                                         \
	}

constexpr uint ADDR_UPDATE = 0x1BAB4;
constexpr uint ADDR_STARTUP = 0x1BABC;
constexpr uint ADDR_SHUTDOWN = 0x1BAB8;
constexpr uint ADDR_ANTIdIEMSG = 0x39124;
constexpr uint ADDR_DISCFENCR = 0x6E10D;
constexpr uint ADDR_DISCFENCR2 = 0x6BFA6;
constexpr uint ADDR_CRCANTICHEAT = 0x6FAF0;
constexpr uint ADDR_RCSENDCHAT = 0x7F30;
constexpr uint ADDR_CPLIST = 0x43D74;
constexpr uint ADDR_CDPSERVER = 0xA284;          // 065CA284
constexpr uint ADDR_CREATECHAR = 0x6B790;        // 06D4B790
constexpr uint ADDR_FLNEW = 0x80012;             // 06D60012
constexpr uint ADDR_SERVERFLSERVER = 0x1BC90;    // 0041BC90
constexpr uint ADDR_DISABLENPCSPAWNS1 = 0x5987B; // 06EF987B
constexpr uint ADDR_DISABLENPCSPAWNS2 = 0x59CD3; // 06EF9CD3
constexpr uint ADDR_DATAPTR = 0x277EC;           // 004277EC
constexpr uint ADDR_RC_DISCONNECT = 0x93E0;      // 06B393E0
constexpr uint ADDR_DALIB_DISC_SUPPRESS = 0x49C6;// 065C49C6
constexpr uint ADDR_SRV_GETCOMMODITIES = 0x32EC2;// 06D12EC2
constexpr uint ADDR_SRV_MAXGROUPSIZE = 0x3A068;  // 06D1A068
constexpr uint ADDR_SRV_MAXGROUPSIZE2 = 0x3A46E; // 06D1A46E
constexpr uint ADDR_SRV_GETINSPECT = 0x206C0;    // 06D006C0
constexpr uint ADDR_SRV_PLAYERDBMAXPLAYERSPATCH = 0x64BC3;
constexpr uint ADDR_SRV_PLAYERDBMAXPLAYERS = 0xB0264;
constexpr uint ADDR_SRV_REPARRAYFREE = 0x7F3F0;
constexpr uint ADDR_COMMON_VFTABLE_POWER = 0x1398F4;
constexpr uint ADDR_COMMON_VFTABLE_SCANNER = 0x139920;
constexpr uint ADDR_COMMON_VFTABLE_LIGHT = 0x13994C;
constexpr uint ADDR_COMMON_VFTABLE_TRACTOR = 0x139978;
constexpr uint ADDR_COMMON_VFTABLE_MINE = 0x139C64;
constexpr uint ADDR_COMMON_VFTABLE_CM = 0x139C90;
constexpr uint ADDR_COMMON_VFTABLE_GUN = 0x139C38;
constexpr uint ADDR_COMMON_VFTABLE_SHIELDGEN = 0x139BB4;
constexpr uint ADDR_COMMON_VFTABLE_THRUSTER = 0x139BE0;
constexpr uint ADDR_COMMON_VFTABLE_SHIELDBAT = 0x1399FC;
constexpr uint ADDR_COMMON_VFTABLE_NANOBOT = 0x1399D0;
constexpr uint ADDR_COMMON_VFTABLE_MUNITION = 0x139CE8;
constexpr uint ADDR_COMMON_VFTABLE_ENGINE = 0x139AAC;

class CTimer
{
public:
	EXPORT CTimer(const std::string& function, uint warning);
	EXPORT void start();
	EXPORT uint stop();

private:
	mstime tmStart = 0;
	uint max = 0;
	std::string function;
	uint warning;
};

struct PluginHookData
{
	HookedCall targetFunction;
	PluginHook::FunctionType* hookFunction;
	HookStep step;
	int priority;
	std::weak_ptr<Plugin> plugin;
};

inline bool operator<(const PluginHookData& lhs, const PluginHookData& rhs)
{
	return lhs.priority > rhs.priority;
}

class PluginManager : public Singleton<PluginManager>
{
public:
	struct FunctionHookProps
	{
		bool callBefore = false;
		bool callAfter = false;

		bool matches(HookStep s) const
		{
			switch (s)
			{
				case HookStep::Before:
					return callBefore;
				case HookStep::After:
					return callAfter;
				default:
					return false;
			}
		}
	};

private:
	std::array<std::vector<PluginHookData>, static_cast<uint>(HookedCall::Count) * magic_enum::enum_count<HookStep>()> pluginHooks_;
	std::vector<std::shared_ptr<Plugin>> plugins;
	std::unordered_map<HookedCall, FunctionHookProps> hookProps_;

	void ClearData(bool free);
	void setupProps();
	void SetProps(HookedCall c, bool before, bool after);

public:
	PluginManager();
	~PluginManager();

	void LoadAll(bool, CCmds*);
	void UnloadAll();

	void Load(const std::string& fileName, CCmds*, bool);
	cpp::result<std::string, Error> Unload(const std::string& shortName);

	auto begin() { return plugins.begin(); }
	auto end() { return plugins.end(); }
	auto begin() const { return plugins.begin(); }
	auto end() const { return plugins.end(); }

	template<typename ReturnType, typename... Args>
	ReturnType CallPlugins(HookedCall target, HookStep step, bool& skipFunctionCall, Args&&... args) const
	{
		using PluginCallType = ReturnType(Args...);
		constexpr bool ReturnTypeIsVoid = std::is_same_v<ReturnType, void>;
		using NoVoidReturnType = std::conditional_t<ReturnTypeIsVoid, int, ReturnType>;

		NoVoidReturnType ret{};
		TRY_HOOK
			{
				for (const auto& hook : pluginHooks_[static_cast<uint>(target) * magic_enum::enum_count<HookStep>() + static_cast<uint>(step)])
				{
					const auto& plugin = hook.plugin;
					if (plugin->paused)
						continue;

					if (plugin->resetCode)
						*plugin->returnCode = ReturnCode::Default;

					TRY_HOOK
						{
							if constexpr (ReturnTypeIsVoid)
								reinterpret_cast<PluginCallType*>(hook.hookFunction)(std::forward<Args>(args)...);
							else
								ret = reinterpret_cast<PluginCallType*>(hook.hookFunction)(std::forward<Args>(args)...);
						}
					CATCH_HOOK({ Logger::i()->Log(LogLevel::Err, std::format("Exception in plugin '{}' in {}", plugin->name, __FUNCTION__)); });

					auto code = *plugin->returnCode;

					if ((code & ReturnCode::SkipFunctionCall) != ReturnCode::Default)
						skipFunctionCall = true;

					if ((code & ReturnCode::SkipPlugins) != ReturnCode::Default)
						break;
				}
			}
		CATCH_HOOK({ Logger::i()->Log(LogLevel::Err, std::format("Exception {}", __FUNCTION__)); });

		if constexpr (!ReturnTypeIsVoid)
			return ret;
	}
};

template<typename ReturnType = void, typename... Args>
auto CallPluginsBefore(HookedCall target, Args&&... args)
{
	bool skip = false;
	if constexpr (std::is_same_v<ReturnType, void>)
	{
		PluginManager::i()->CallPlugins<void>(target, HookStep::Before, skip, std::forward<Args>(args)...);
		return skip;
	}
	else
	{
		ReturnType ret = PluginManager::i()->CallPlugins<ReturnType>(target, HookStep::Before, skip, std::forward<Args>(args)...);
		return std::make_tuple(ret, skip);
	}
}

template<typename... Args>
void CallPluginsAfter(HookedCall target, Args&&... args)
{
	bool _ = false;
	PluginManager::i()->CallPlugins<void>(target, HookStep::After, _, std::forward<Args>(args)...);
}

template<typename... Args>
bool CallPluginsOther(HookedCall target, HookStep step, Args&&... args)
{
	bool skip = false;
	PluginManager::i()->CallPlugins<void>(target, step, skip, std::forward<Args>(args)...);
	return skip;
}

using PluginFactoryT = std::shared_ptr<Plugin>(*)();

// TODO: Move this to its own CPP file and use the Detour class
class DebugTools : public Singleton<DebugTools>
{
	static std::map<std::string, uint> hashMap;

	std::allocator<BYTE> allocator;

	static uint CreateIdDetour(const char* str);

public:
	DebugTools() = default;
	void Init();
};

void DetourSendComm();
void UnDetourSendComm();
