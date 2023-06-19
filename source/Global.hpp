#pragma once

#include "Features/Logger.hpp"
#include "plugin.h"

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

// TODO: Move to class with ctor and dtor
namespace StartupCache
{
    void Init();
    void Done();
} // namespace StartupCache

namespace Hk
{
    namespace Personalities
    {
        // TODO: Move to DataManager
        void LoadPersonalities();
    } // namespace Personalities

    namespace Client
    {
        uint ExtractClientID(const std::variant<uint, std::wstring_view>& player);
        cpp::result<CAccount*, Error> ExtractAccount(const std::variant<uint, std::wstring_view>& player);
    } // namespace Client
} // namespace Hk

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
void PublishServerStats();
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
extern std::list<RESOLVE_IP> resolveIPs;
extern std::list<RESOLVE_IP> resolveIPsResult;
extern HANDLE hThreadResolver;

// help

void IClientImpl__Startup__Inner(uint dunno, uint dunno2);

inline auto* ToWChar(const ushort* val) { return reinterpret_cast<const wchar_t*>(val); }

inline auto* ToWChar(ushort* val) { return reinterpret_cast<wchar_t*>(val); }

inline auto* ToUShort(const wchar_t* val) { return reinterpret_cast<const ushort*>(val); }

inline auto* ToUShort(wchar_t* val) { return reinterpret_cast<ushort*>(val); }

#define CALL_SERVER_PREAMBLE                                           \
    {                                                                  \
        static PerfTimer timer(StringUtils::stows(__FUNCTION__), 100); \
        timer.Start();                                                 \
        TRY_HOOK                                                       \
        {
#define CALL_SERVER_POSTAMBLE(catchArgs, rval)                                                                             \
    }                                                                                                                      \
    CATCH_HOOK({                                                                                                           \
        Logger::i()->Log(LogLevel::Err, std::format(L"Exception in {} on server call", StringUtils::stows(__FUNCTION__))); \
        bool ret = catchArgs;                                                                                              \
        if (!ret)                                                                                                          \
        {                                                                                                                  \
            timer.Stop();                                                                                                  \
            return rval;                                                                                                   \
        }                                                                                                                  \
    })                                                                                                                     \
    timer.Stop();                                                                                                          \
    }

#define CALL_CLIENT_PREAMBLE      \
    {                             \
        void* vRet;               \
        char* tmp;                \
        memcpy(&tmp, &Client, 4); \
        memcpy(&Client, &OldClient, 4);

#define CALL_CLIENT_POSTAMBLE \
    __asm { mov [vRet], eax}  \
    memcpy(&Client, &tmp, 4); \
    }

#define CHECK_FOR_DISCONNECT                                                                                                                       \
    {                                                                                                                                              \
        if (ClientInfo[client].disconnected)                                                                                                       \
        {                                                                                                                                          \
            Logger::i()->Log(LogLevel::Debug, std::format(L"Ignoring disconnected client in {} id={}", StringUtils::stows(__FUNCTION__), client)); \
            return;                                                                                                                                \
        };                                                                                                                                         \
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
constexpr uint ADDR_CDPSERVER = 0xA284;           // 065CA284
constexpr uint ADDR_CREATECHAR = 0x6B790;         // 06D4B790
constexpr uint ADDR_FLNEW = 0x80012;              // 06D60012
constexpr uint ADDR_SERVERFLSERVER = 0x1BC90;     // 0041BC90
constexpr uint ADDR_DISABLENPCSPAWNS1 = 0x5987B;  // 06EF987B
constexpr uint ADDR_DISABLENPCSPAWNS2 = 0x59CD3;  // 06EF9CD3
constexpr uint ADDR_DATAPTR = 0x277EC;            // 004277EC
constexpr uint ADDR_RC_DISCONNECT = 0x93E0;       // 06B393E0
constexpr uint ADDR_DALIB_DISC_SUPPRESS = 0x49C6; // 065C49C6
constexpr uint ADDR_SRV_GETCOMMODITIES = 0x32EC2; // 06D12EC2
constexpr uint ADDR_SRV_MAXGROUPSIZE = 0x3A068;   // 06D1A068
constexpr uint ADDR_SRV_MAXGROUPSIZE2 = 0x3A46E;  // 06D1A46E
constexpr uint ADDR_SRV_GETINSPECT = 0x206C0;     // 06D006C0
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

class PerfTimer
{
    public:
        EXPORT PerfTimer(const std::wstring& function, uint warning);
        EXPORT void Start();
        EXPORT uint Stop();

    private:
        mstime tmStart = 0;
        uint max = 0;
        std::wstring function;
        uint warning;
};

struct PluginHookData
{
        HookedCall targetFunction;
        PluginHook::FunctionType hookFunction;
        HookStep step;
        int priority;
        std::weak_ptr<Plugin> plugin;
};

inline bool operator<(const PluginHookData& lhs, const PluginHookData& rhs) { return lhs.priority > rhs.priority; }

// Forward declarations for friend defs

class AdminCommandProcessor;
class UserCommandProcessor;

class PluginManager : public Singleton<PluginManager>
{
        friend AdminCommandProcessor;
        friend UserCommandProcessor;

    public:
        struct FunctionHookProps
        {
                bool callBefore = false;
                bool callAfter = false;

                bool matches(HookStep s) const
                {
                    switch (s)
                    {
                        case HookStep::Before: return callBefore;
                        case HookStep::After: return callAfter;
                        default: return false;
                    }
                }
        };

    private:
        std::array<std::vector<PluginHookData>, static_cast<uint>(HookedCall::Count) * magic_enum::enum_count<HookStep>()> pluginHooks;
        // TODO: Add a getter function of a const ref so other classes can look at thi list of plugins.
        std::vector<std::shared_ptr<Plugin>> plugins;
        std::vector<std::weak_ptr<AbstractUserCommandProcessor>> userCommands;
        std::vector<std::weak_ptr<AbstractAdminCommandProcessor>> adminCommands;
        std::unordered_map<HookedCall, FunctionHookProps> hookProps;

        void ClearData(bool free);
        void setupProps();
        void SetProps(HookedCall c, bool before, bool after);

    public:
        PluginManager();
        ~PluginManager();

        void LoadAll(bool);
        void UnloadAll();

        bool Load(std::wstring_view fileName, bool);
        cpp::result<std::wstring, Error> Unload(std::wstring_view shortName);

        auto begin() { return plugins.begin(); }
        auto end() { return plugins.end(); }

        [[nodiscard]]
        auto begin() const
        {
            return plugins.begin();
        }

        [[nodiscard]]
        auto end() const
        {
            return plugins.end();
        }

        template <typename ReturnType, typename... Args>
        ReturnType CallPlugins(HookedCall target, HookStep step, bool& skipFunctionCall, Args&&... args) const
        {
            using PluginCallType = ReturnType(__thiscall*)(void*, Args...);
            constexpr bool returnTypeIsVoid = std::is_same_v<ReturnType, void>;
            using NoVoidReturnType = std::conditional_t<returnTypeIsVoid, int, ReturnType>;

            NoVoidReturnType ret{};
            TRY_HOOK
            {
                for (const PluginHookData& hook : pluginHooks[static_cast<uint>(target) * magic_enum::enum_count<HookStep>() + static_cast<uint>(step)])
                {
                    if (hook.plugin.expired())
                    {
                        continue;
                    }

                    const auto& plugin = hook.plugin.lock();

                    plugin->returnCode = ReturnCode::Default;

                    TRY_HOOK
                    {
                        void* pluginRaw = plugin.get();
                        if constexpr (returnTypeIsVoid)
                        {
                            reinterpret_cast<PluginCallType>(hook.hookFunction)(pluginRaw, std::forward<Args>(args)...);
                        }
                        else
                        {
                            ret = reinterpret_cast<PluginCallType>(hook.hookFunction)(pluginRaw, std::forward<Args>(args)...);
                        }
                    }
                    CATCH_HOOK({
                        Logger::i()->Log(
                            LogLevel::Err,
                            std::format(L"Exception in plugin '{}' in {}-{}", plugin->name, magic_enum::enum_name(target), magic_enum::enum_name(step)));
                    })

                    const auto code = plugin->returnCode;

                    if ((code & ReturnCode::SkipFunctionCall) != ReturnCode::Default)
                    {
                        skipFunctionCall = true;
                    }

                    if ((code & ReturnCode::SkipPlugins) != ReturnCode::Default)
                    {
                        break;
                    }
                }
            }
            CATCH_HOOK({ Logger::i()->Log(LogLevel::Err, std::format(L"Exception {}", StringUtils::stows(__FUNCTION__))); });

            if constexpr (!returnTypeIsVoid)
            {
                return ret;
            }
        }
};

template <typename ReturnType = void, typename... Args>
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
        auto ret = PluginManager::i()->CallPlugins<ReturnType>(target, HookStep::Before, skip, std::forward<Args>(args)...);
        return std::make_tuple(ret, skip);
    }
}

template <typename... Args>
void CallPluginsAfter(HookedCall target, Args&&... args)
{
    bool _ = false;
    PluginManager::i()->CallPlugins<void>(target, HookStep::After, _, std::forward<Args>(args)...);
}

template <typename... Args>
bool CallPluginsOther(HookedCall target, HookStep step, Args&&... args)
{
    bool skip = false;
    PluginManager::i()->CallPlugins<void>(target, step, skip, std::forward<Args>(args)...);
    return skip;
}

using PluginFactoryT = std::shared_ptr<Plugin> (*)();

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
