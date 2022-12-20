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
void HookRehashed();
void LoadUserCharSettings(ClientId client);

void ClearClientInfo(ClientId client);
void LoadUserSettings(ClientId client);

bool UserCmd_Process(ClientId client, const std::wstring& wscCmd);

bool AllowPlayerDamage(ClientId client, ClientId clientTarget);

// Logs
template<typename T>
std::wstring ToLogString(const T& val)
{
	// Get type without reference
	typedef std::remove_reference_t<decltype(val)> DeclType;
	if constexpr (std::is_same_v<DeclType, int> || std::is_same_v<DeclType, uint> || std::is_same_v<DeclType, float> || std::is_same_v<DeclType, double>)
	{
		return std::to_wstring(val);
	}

	return L"<undefined>";
}

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
		void CharacterSelect(CHARACTER_ID const charId, ClientId client);
	}

	namespace Personalities
	{
		void LoadPersonalities();
	}

	namespace Client
	{
		uint ExtractClientID(const std::variant<uint, std::wstring>& player);
		cpp::result<CAccount*, Error> ExtractAccount(const std::variant<uint, std::wstring>& player);
	}
}


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
void TimerCheckKick();
void TimerNPCAndF1Check();
void ThreadResolver();
void TimerCheckResolveResults();

void BaseDestroyed(uint objectId, ClientId clientBy);

extern HookEntry IServerImplEntries[73];

// DataBaseMarket
bool LoadBaseMarket();

extern CRITICAL_SECTION csIPResolve;
extern std::list<RESOLVE_IP> g_lstResolveIPs;
extern std::list<RESOLVE_IP> g_lstResolveIPsResult;
extern HANDLE hThreadResolver;

// help

extern std::list<stHelpEntry> lstHelpEntries;
void IClientImpl__Startup__Inner(uint iDunno, uint iDunno2);

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
#define CALL_SERVER_POSTAMBLE(catchArgs, rval)                                                  \
	}                                                                                           \
	CATCH_HOOK({                                                                                \
		AddLog(LogType::Normal, LogLevel::Info, fmt::format("ERROR: Exception in {} on server call", __FUNCTION__)); \
		bool ret = catchArgs;                                                                   \
		if (!ret)                                                                               \
		{                                                                                       \
			timer.stop();                                                                       \
			return rval;                                                                        \
		}                                                                                       \
	})                                                                                          \
	timer.stop();                                                                               \
	}

#define CALL_CLIENT_PREAMBLE      \
	{                             \
		void* vRet;               \
		char* tmp;                \
		memcpy(&tmp, &Client, 4); \
		memcpy(&Client, &OldClient, 4);

#define CALL_CLIENT_POSTAMBLE \
	__asm { mov [vRet], eax}    \
	memcpy(&Client, &tmp, 4); \
	}

#define CHECK_FOR_DISCONNECT                                                    \
	{                                                                           \
		if (ClientInfo[client].bDisconnected)                                 \
		{                                                                       \
			AddLog(LogType::Normal, LogLevel::Info, fmt::format("ERROR: Ignoring disconnected client in {} id={}", __FUNCTION__, client)); \
			return;                                                             \
		};                                                                      \
	}

#define ADDR_UPDATE 0x1BAB4
#define ADDR_STARTUP 0x1BABC
#define ADDR_SHUTDOWN 0x1BAB8
#define ADDR_ANTIdIEMSG 0x39124
#define ADDR_DISCFENCR 0x6E10D
#define ADDR_DISCFENCR2 0x6BFA6
#define ADDR_CRCANTICHEAT 0x6FAF0
#define ADDR_RCSENDCHAT 0x7F30
#define ADDR_CPLIST 0x43D74
#define ADDR_CDPSERVER 0xA284           // 065CA284
#define ADDR_CREATECHAR 0x6B790         // 06D4B790
#define ADDR_FLNEW 0x80012              // 06D60012
#define ADDR_SERVERFLSERVER 0x1BC90     // 0041BC90
#define ADDR_DISABLENPCSPAWNS1 0x5987B  // 06EF987B
#define ADDR_DISABLENPCSPAWNS2 0x59CD3  // 06EF9CD3
#define ADDR_DATAPTR 0x277EC            // 004277EC
#define ADDR_RC_DISCONNECT 0x93E0       // 06B393E0
#define ADDR_DALIB_DISC_SUPPRESS 0x49C6 // 065C49C6
#define ADDR_SRV_GETCOMMODITIES 0x32EC2 // 06D12EC2
#define ADDR_SRV_MAXGROUPSIZE 0x3A068   // 06D1A068
#define ADDR_SRV_MAXGROUPSIZE2 0x3A46E  // 06D1A46E
#define ADDR_SRV_GETINSPECT 0x206C0     // 06D006C0
#define ADDR_SRV_PLAYERDBMAXPLAYERSPATCH 0x64BC3
#define ADDR_SRV_PLAYERDBMAXPLAYERS 0xB0264
#define ADDR_SRV_REPARRAYFREE 0x7F3F0
#define ADDR_COMMON_VFTABLE_POWER 0x1398F4
#define ADDR_COMMON_VFTABLE_SCANNER 0x139920
#define ADDR_COMMON_VFTABLE_LIGHT 0x13994C
#define ADDR_COMMON_VFTABLE_TRACTOR 0x139978
#define ADDR_COMMON_VFTABLE_MINE 0x139C64
#define ADDR_COMMON_VFTABLE_CM 0x139C90
#define ADDR_COMMON_VFTABLE_GUN 0x139C38
#define ADDR_COMMON_VFTABLE_SHIELDGEN 0x139BB4
#define ADDR_COMMON_VFTABLE_THRUSTER 0x139BE0
#define ADDR_COMMON_VFTABLE_SHIELDBAT 0x1399FC
#define ADDR_COMMON_VFTABLE_NANOBOT 0x1399D0
#define ADDR_COMMON_VFTABLE_MUNITION 0x139CE8
#define ADDR_COMMON_VFTABLE_ENGINE 0x139AAC

class CTimer
{
  public:
	EXPORT CTimer(std::string sFunction, uint iWarning);
	EXPORT void start();
	EXPORT uint stop();

  private:
	mstime tmStart;
	uint iMax;
	std::string sFunction;
	uint iWarning;
};

struct PluginData
{
	std::string name;
	std::string shortName;
	HMODULE dll = nullptr;
	std::wstring dllName;
	bool mayUnload = false;
	ReturnCode* returnCode = nullptr;
	bool resetCode = true;
	bool paused = false;
	std::vector<UserCommand> commands;
	std::vector<Timer> timers;
	std::shared_ptr<PluginInfo> pInfo = nullptr;
};

struct PluginHookData
{
	HookedCall targetFunction;
	PluginHook::FunctionType* hookFunction;
	HookStep step;
	int priority;
	size_t index;

	[[nodiscard]] const PluginData& plugin() const;
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
		bool callBefore = false, callMid = false, callAfter = false;

		bool matches(HookStep s) const
		{
			switch (s)
			{
				case HookStep::Before:
					return callBefore;
				case HookStep::After:
					return callAfter;
				case HookStep::Mid:
					return callMid;
				default:
					return false;
			}
		}
	};

  private:
	std::array<std::vector<PluginHookData>, size_t(HookedCall::Count) * size_t(HookStep::Count)> pluginHooks_;
	std::vector<PluginData> plugins_;
	std::unordered_map<HookedCall, FunctionHookProps> hookProps_;

	void clearData(bool free);
	void setupProps();
	void setProps(HookedCall c, bool b, bool m, bool a);

  public:
	PluginManager();
	~PluginManager();

	void loadAll(bool, CCmds*);
	void unloadAll();

	void load(const std::wstring& fileName, CCmds*, bool);
	cpp::result<void, Error> unload(const std::string& shortName);

	const PluginData& pluginAt(size_t index) const { return plugins_[index]; }
	PluginData& pluginAt(size_t index) { return plugins_[index]; }

	auto begin() { return plugins_.begin(); }
	auto end() { return plugins_.end(); }
	auto begin() const { return plugins_.begin(); }
	auto end() const { return plugins_.end(); }

	template<typename ReturnType, typename... Args>
	ReturnType callPlugins(HookedCall target, HookStep step, bool& skipFunctionCall, Args&&... args) const
	{
		using PluginCallType = ReturnType(Args...);
		constexpr bool ReturnTypeIsVoid = std::is_same_v<ReturnType, void>;
		using NoVoidReturnType = std::conditional_t<ReturnTypeIsVoid, int, ReturnType>;

		NoVoidReturnType ret {};
		TRY_HOOK
		{
			for (const auto& hook : pluginHooks_[uint(target) * uint(HookStep::Count) + uint(step)])
			{
				const auto& plugin = hook.plugin();
				if (plugin.paused)
					continue;

				if (plugin.resetCode)
					*plugin.returnCode = ReturnCode::Default;

				TRY_HOOK
				{
					if constexpr (ReturnTypeIsVoid)
						reinterpret_cast<PluginCallType*>(hook.hookFunction)(std::forward<Args>(args)...);
					else
						ret = reinterpret_cast<PluginCallType*>(hook.hookFunction)(std::forward<Args>(args)...);
				}
				CATCH_HOOK({
					AddLog(LogType::Normal, LogLevel::Err, fmt::format("ERROR: Exception in plugin '{}' in {}", plugin.name, __FUNCTION__));
				});

				auto code = *plugin.returnCode;

				if ((code & ReturnCode::SkipFunctionCall) != ReturnCode::Default)
					skipFunctionCall = true;

				if ((code & ReturnCode::SkipPlugins) != ReturnCode::Default)
					break;
			}
		}
		CATCH_HOOK({ AddLog(LogType::Normal, LogLevel::Err, fmt::format("ERROR: Exception {}", __FUNCTION__)); });

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
		PluginManager::i()->callPlugins<void>(target, HookStep::Before, skip, std::forward<Args>(args)...);
		return skip;
	}
	else
	{
		ReturnType ret =
		    PluginManager::i()->callPlugins<ReturnType>(target, HookStep::Before, skip, std::forward<Args>(args)...);
		return std::make_tuple(ret, skip);
	}
}

template<typename... Args>
void CallPluginsAfter(HookedCall target, Args&&... args)
{
	bool dontCare = false;
	PluginManager::i()->callPlugins<void>(target, HookStep::After, dontCare, std::forward<Args>(args)...);
}

template<typename... Args>
bool CallPluginsOther(HookedCall target, HookStep step, Args&&... args)
{
	bool skip = false;
	PluginManager::i()->callPlugins<void>(target, step, skip, std::forward<Args>(args)...);
	return skip;
}

using ExportPluginInfoT = void (*)(PluginInfo*);