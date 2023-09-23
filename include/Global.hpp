#pragma once

#include "API/FlHook/Plugin.hpp"
#include "Core/Logger.hpp"

bool FLHookInit();
void FLHookInit_Pre();
void FLHookShutdown();
void LoadSettings();
void ProcessPendingCommands();

void PatchClientImpl();
bool InitHookExports();
void UnloadHookExports();

void ClearClientInfo(ClientId client);
void LoadUserSettings(ClientId client);

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

// DataBaseMarket
bool LoadBaseMarket();

extern CRITICAL_SECTION csIPResolve;
extern std::list<RESOLVE_IP> resolveIPs;
extern std::list<RESOLVE_IP> resolveIPsResult;
extern HANDLE hThreadResolver;

// help

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

// TODO: Move this to its own CPP file and use the Detour class
class DebugTools : public Singleton<DebugTools>
{
        static std::map<std::string, uint> hashMap;

        std::allocator<BYTE> allocator;

        static uint CreateIdDetour(const char* str);

    public:
        DebugTools() = default;
        void Init() const;
};

void DetourSendComm();
void UnDetourSendComm();