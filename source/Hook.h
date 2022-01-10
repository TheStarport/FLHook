#pragma once

#include <time.h>
#include "global.h"
#include "flcodec.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// defines

#define HKHKSUCCESS(a) ((a) == HKE_OK)
#define HKSUCCESS(a) ((hkLastErr = (a)) == HKE_OK)

#define SRV_ADDR(a) ((char *)hModServer + a)
#define DALIB_ADDR(a) ((char *)hModDaLib + a)
#define FLSERVER_ADDR(a) ((char *)hProcFL + a)
#define CONTENT_ADDR(a) ((char *)hModContent + a)
#define ARG_CLIENTID(a) (std::wstring(L"id ") + std::to_wstring(a))

#define ADDR_UPDATE 0x1BAB4
#define ADDR_STARTUP 0x1BABC
#define ADDR_SHUTDOWN 0x1BAB8
#define ADDR_ANTIDIEMSG 0x39124
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// exception logging

#define EXTENDED_EXCEPTION_LOGGING
#ifdef EXTENDED_EXCEPTION_LOGGING

struct SEHException {
    SEHException(uint code, EXCEPTION_POINTERS *ep)
        : code(code), record(*ep->ExceptionRecord),
          context(*ep->ContextRecord) {}

    SEHException() = default;

    uint code;
    EXCEPTION_RECORD record;
    CONTEXT context;

    static void Translator(uint code, EXCEPTION_POINTERS *ep) {
        throw SEHException(code, ep);
    }
};

EXPORT void WriteMiniDump(SEHException *ex);
EXPORT void AddExceptionInfoLog();
EXPORT void AddExceptionInfoLog(SEHException *ex);
#define TRY_HOOK                                                               \
    try {                                                                      \
        _set_se_translator(SEHException::Translator);
#define CATCH_HOOK(e)                                                          \
    }                                                                          \
    catch (SEHException & ex) {                                                \
        e;                                                                     \
        AddBothLog(                                                            \
            "ERROR: SEH Exception in %s on line %d; minidump may contain "     \
            "more information.",                                               \
            __FUNCTION__, __LINE__);                                           \
        AddExceptionInfoLog(&ex);                                              \
    }                                                                          \
    catch (std::exception & ex) {                                              \
        e;                                                                     \
        AddBothLog("ERROR: STL Exception in %s on line %d: %s.", __FUNCTION__, \
                   __LINE__, ex.what());                                       \
        AddExceptionInfoLog(0);                                                \
    }                                                                          \
    catch (...) {                                                              \
        e;                                                                     \
        AddBothLog("ERROR: Exception in %s on line %d.", __FUNCTION__,         \
                   __LINE__);                                                  \
        AddExceptionInfoLog();                                                 \
    }
#else
#define TRY_HOOK try
#define CATCH_HOOK(e)                                                          \
    catch (...) {                                                              \
        e;                                                                     \
        AddLog("ERROR: Exception in %s", __FUNCTION__);                        \
    }
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

class CCmds;

class CTimer {
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// typedefs
typedef void(__stdcall *_RCSendChatMsg)(uint iId, uint iTo, uint iSize,
                                        void *pRDL);
typedef void(__stdcall *_CRCAntiCheat)();
typedef void(__stdcall *_CreateChar)(const wchar_t *wszName);
typedef int(__cdecl *_GetFLName)(char *szBuf, const wchar_t *wszStr);
typedef bool(__cdecl *_GetShipInspect)(uint &iShip, IObjInspectImpl *&inspect,
                                       uint &iDunno);

EXPORT extern _GetShipInspect GetShipInspect;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// enums

enum HK_ERROR {
    HKE_OK,
    HKE_PLAYER_NOT_LOGGED_IN,
    HKE_CHAR_DOES_NOT_EXIST,
    HKE_INVALID_CLIENT_ID,
    HKE_COULD_NOT_DECODE_CHARFILE,
    HKE_COULD_NOT_ENCODE_CHARFILE,
    HKE_INVALID_BASENAME,
    HKE_INVALID_ID_STRING,
    HKE_INVALID_SYSTEM,
    HKE_PLAYER_NOT_IN_SPACE,
    HKE_PLAYER_NO_ADMIN,
    HKE_WRONG_XML_SYNTAX,
    HKE_INVALID_GOOD,
    HKE_NO_CHAR_SELECTED,
    HKE_CHARNAME_ALREADY_EXISTS,
    HKE_CHARNAME_TOO_LONG,
    HKE_CHARNAME_TOO_SHORT,
    HKE_AMBIGUOUS_SHORTCUT,
    HKE_NO_MATCHING_PLAYER,
    HKE_INVALID_SHORTCUT_STRING,
    HKE_MPNEWCHARACTERFILE_NOT_FOUND_OR_INVALID,
    HKE_INVALID_REP_GROUP,
    HKE_PLUGIN_UNLOADABLE,
    HKE_PLUGIN_UNPAUSABLE,
    HKE_PLUGIN_NOT_FOUND,
    HKE_UNKNOWN_ERROR,
    HKE_INVALID_GROUP_ID,
    HKE_CUSTOM_1,
    HKE_CUSTOM_2,
    HKE_CUSTOM_3,
    HKE_CUSTOM_4,
    HKE_CUSTOM_5,
    HKE_CUSTOM_6,
    HKE_CUSTOM_7,
    HKE_CUSTOM_8,
    HKE_CUSTOM_9,
    HKE_CUSTOM_10,
};

enum DIEMSGTYPE {
    DIEMSG_ALL = 0,
    DIEMSG_SYSTEM = 1,
    DIEMSG_NONE = 2,
    DIEMSG_SELF = 3,
};

enum CHATSIZE {
    CS_DEFAULT = 0,
    CS_SMALL = 1,
    CS_BIG = 2,
};

enum CHATSTYLE {
    CST_DEFAULT = 0,
    CST_BOLD = 1,
    CST_ITALIC = 2,
    CST_UNDERLINE = 3,
};

enum ENGINE_STATE {
    ES_CRUISE,
    ES_THRUSTER,
    ES_ENGINE,
    ES_KILLED,
    ES_TRADELANE
};

enum EQ_TYPE {
    ET_GUN,
    ET_TORPEDO,
    ET_CD,
    ET_MISSILE,
    ET_MINE,
    ET_CM,
    ET_SHIELDGEN,
    ET_THRUSTER,
    ET_SHIELDBAT,
    ET_NANOBOT,
    ET_MUNITION,
    ET_ENGINE,
    ET_OTHER,
    ET_SCANNER,
    ET_TRACTOR,
    ET_LIGHT
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// plugin functionality & hook prototypes
//

#include <plugin.h>

struct PluginHookData;

struct PluginInfo {
    EXPORT void versionMajor(PluginMajorVersion version);
    EXPORT void versionMinor(PluginMinorVersion version);
    EXPORT void name(const char *name);
    EXPORT void shortName(const char *shortName);
    EXPORT void mayPause(bool pause);
    EXPORT void mayUnload(bool unload);
    EXPORT void autoResetCode(bool reset);
    EXPORT void returnCode(ReturnCode *returnCode);
    EXPORT void addHook(const PluginHook &hook);

    template <typename... Args>
    void addHook(Args &&... args) {
        addHook(PluginHook(std::forward<Args>(args)...));
    }

    PluginMajorVersion versionMajor_ = PluginMajorVersion::UNDEFINED;
    PluginMinorVersion versionMinor_ = PluginMinorVersion::UNDEFINED;
    std::string name_, shortName_;
    bool mayPause_ = false, mayUnload_ = false, resetCode_ = true;
    ReturnCode *returnCode_ = nullptr;
    std::list<PluginHook> hooks_;
};

struct PluginData {
    std::string name;
    std::string shortName;
    size_t hash = 0;
    HMODULE dll = nullptr;
    std::wstring dllName;
    bool mayPause = false;
    bool mayUnload = false;
    ReturnCode *returnCode = nullptr;
    bool resetCode = true;
    bool paused = false;
    std::shared_ptr<PluginInfo> pInfo = nullptr;
};

struct PluginHookData {
    HookedCall targetFunction;
    PluginHook::FunctionType *hookFunction;
    HookStep step;
    int priority;
    size_t index;

    [[nodiscard]] const PluginData& plugin() const;
};

inline bool operator<(const PluginHookData &lhs, const PluginHookData &rhs) {
    return lhs.priority > rhs.priority;
}

EXPORT void PluginCommunication(PLUGIN_MESSAGE msgtype, void *msg);

class PluginManager : public Singleton<PluginManager> {
public:
    struct FunctionHookProps {
        bool callBefore = false, callMid = false, callAfter = false;
        
        bool matches(HookStep s) const {
            switch(s) {
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

    void load(const std::wstring &fileName, CCmds*, bool);
    HK_ERROR pause(size_t hash, bool pause);
    HK_ERROR unload(size_t hash);
    HK_ERROR pause(const std::string& shortName, bool pause) { return this->pause(std::hash<std::string>{}(shortName), pause); }
    HK_ERROR unload(const std::string& shortName) { return this->unload(std::hash<std::string>{}(shortName)); }

    const PluginData& pluginAt(size_t index) const { return plugins_[index]; }
    PluginData& pluginAt(size_t index) { return plugins_[index]; }
    
    auto begin() const { return plugins_.begin(); }
    auto end() const { return plugins_.end(); }

    template<typename ReturnType, typename... Args>
    ReturnType callPlugins(HookedCall target, HookStep step, bool& skipFunctionCall, Args&& ...args) const {
        using PluginCallType = ReturnType(Args...);
        constexpr bool ReturnTypeIsVoid = std::is_same_v<ReturnType, void>;
        using NoVoidReturnType = std::conditional_t<ReturnTypeIsVoid, int, ReturnType>;

        NoVoidReturnType ret {};
        TRY_HOOK {
            for (const auto &hook : pluginHooks_[uint(target) * 2 + uint(step)]) {
                const auto& plugin = hook.plugin();
                if (plugin.paused)
                    continue;

                if(plugin.resetCode)
                    *plugin.returnCode = ReturnCode::Default;

                TRY_HOOK {
                    if constexpr(ReturnTypeIsVoid)
                        reinterpret_cast<PluginCallType*>(hook.hookFunction)(std::forward<Args>(args)...);
                    else
                        ret = reinterpret_cast<PluginCallType*>(hook.hookFunction)(std::forward<Args>(args)...);
                }
                CATCH_HOOK({
                    AddLog("ERROR: Exception in plugin '%s' in %s",
                           plugin.name.c_str(), __FUNCTION__);
                });

                auto code = *plugin.returnCode;

                if ((code & ReturnCode::SkipFunctionCall) != ReturnCode::Default)
                    skipFunctionCall = true;

                if ((code & ReturnCode::SkipPlugins) != ReturnCode::Default)
                    break;
            }
        } CATCH_HOOK({ AddLog("ERROR: Exception %s", __FUNCTION__); });

        if constexpr(!ReturnTypeIsVoid)
            return ret;
    }
};

template<typename ReturnType = void, typename... Args>
auto CallPluginsBefore(HookedCall target, Args&& ...args) {
    bool skip = false;
    if constexpr(std::is_same_v<ReturnType, void>) {
        PluginManager::i()->callPlugins<void>(target, HookStep::Before, skip, std::forward<Args>(args)...);
        return skip;
    } else {
        ReturnType ret = PluginManager::i()->callPlugins<ReturnType>(target, HookStep::Before, skip, std::forward<Args>(args)...);
        return std::make_tuple(ret, skip);
    }
}

template<typename... Args>
void CallPluginsAfter(HookedCall target, Args&& ...args) {
    bool dontCare = false;
    PluginManager::i()->callPlugins<void>(target, HookStep::After, dontCare, std::forward<Args>(args)...);
}

template<typename... Args>
bool CallPluginsOther(HookedCall target, HookStep step, Args&& ...args) {
    bool skip = false;
    PluginManager::i()->callPlugins<void>(target, step, skip, std::forward<Args>(args)...);
    return skip;
}

using ExportPluginInfoT = void(*)(PluginInfo*);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// structs

struct HookEntry {
    FARPROC fpProc;
    long dwRemoteAddress;
    FARPROC fpOldProc;
};

struct CARGO_INFO {
    uint iID;
    int iCount;
    uint iArchID;
    float fStatus;
    bool bMission;
    bool bMounted;
    CacheString hardpoint;
};

// money stuff
struct MONEY_FIX {
    std::wstring wscCharname;
    int iAmount;

    bool operator==(MONEY_FIX mf1) const {
        if (!wscCharname.compare(mf1.wscCharname))
            return true;

        return false;
    };
};

// ignore
struct IGNORE_INFO {
    std::wstring wscCharname;
    std::wstring wscFlags;
};

// resolver
struct RESOLVE_IP {
    uint iClientID;
    uint iConnects;
    std::wstring wscIP;
    std::wstring wscHostname;
};

struct CLIENT_INFO {
    // kill msgs
    uint iShip;
    uint iShipOld;
    mstime tmSpawnTime;

    DamageList dmgLast;

    // money cmd
    std::list<MONEY_FIX> lstMoneyFix;

    // anticheat
    uint iTradePartner;

    // change cruise disruptor behaviour
    bool bCruiseActivated;
    bool bThrusterActivated;
    bool bEngineKilled;
    bool bTradelane;

    // idle kicks
    uint iBaseEnterTime;
    uint iCharMenuEnterTime;

    // msg, wait and kick
    mstime tmKickTime;

    // eventmode
    uint iLastExitedBaseID;
    bool bDisconnected;

    // f1 laming
    bool bCharSelected;
    mstime tmF1Time;
    mstime tmF1TimeDisconnect;

    // ignore usercommand
    std::list<IGNORE_INFO> lstIgnore;

    // user settings
    DIEMSGTYPE dieMsg;
    CHATSIZE dieMsgSize;
    CHATSTYLE dieMsgStyle;
    CHATSIZE chatSize;
    CHATSTYLE chatStyle;

    // autobuy
    bool bAutoBuyMissiles;
    bool bAutoBuyMines;
    bool bAutoBuyTorps;
    bool bAutoBuyCD;
    bool bAutoBuyCM;
    bool bAutoBuyReload;

    // MultiKillMessages
    uint iKillsInARow;

    // bans
    uint iConnects; // incremented when player connects

    // Group
    uint iGroupID;

    // other
    std::wstring wscHostname;

    bool bSpawnProtected;
    bool bUseServersideHitDetection; // used by AC Plugin
    std::map<PluginInfo*, std::array<uchar, 40>> mapPluginData;
};

// taken from directplay
typedef struct _DPN_CONNECTION_INFO {
    DWORD dwSize;
    DWORD dwRoundTripLatencyMS;
    DWORD dwThroughputBPS;
    DWORD dwPeakThroughputBPS;
    DWORD dwBytesSentGuaranteed;
    DWORD dwPacketsSentGuaranteed;
    DWORD dwBytesSentNonGuaranteed;
    DWORD dwPacketsSentNonGuaranteed;
    DWORD dwBytesRetried;
    DWORD dwPacketsRetried;
    DWORD dwBytesDropped;
    DWORD dwPacketsDropped;
    DWORD dwMessagesTransmittedHighPriority;
    DWORD dwMessagesTimedOutHighPriority;
    DWORD dwMessagesTransmittedNormalPriority;
    DWORD dwMessagesTimedOutNormalPriority;
    DWORD dwMessagesTransmittedLowPriority;
    DWORD dwMessagesTimedOutLowPriority;
    DWORD dwBytesReceivedGuaranteed;
    DWORD dwPacketsReceivedGuaranteed;
    DWORD dwBytesReceivedNonGuaranteed;
    DWORD dwPacketsReceivedNonGuaranteed;
    DWORD dwMessagesReceived;
} DPN_CONNECTION_INFO, *PDPN_CONNECTION_INFO;

struct HKPLAYERINFO {
    uint iClientID;
    std::wstring wscCharname;
    std::wstring wscBase;
    std::wstring wscSystem;
    uint iSystem;
    uint iShip;
    DPN_CONNECTION_INFO ci;
    std::wstring wscIP;
    std::wstring wscHostname;
};

// patch stuff
struct PATCH_INFO_ENTRY {
    ulong pAddress;
    void *pNewValue;
    uint iSize;
    void *pOldValue;
    bool bAlloced;
};

struct PATCH_INFO {
    char *szBinName;
    ulong pBaseAddress;

    PATCH_INFO_ENTRY piEntries[128];
};

struct DATA_MARKETITEM {
    uint iArchID;
    float fRep;
};

struct BASE_INFO {
    uint iBaseID;
    std::string scBasename;
    uint iObjectID;
    bool bDestroyed;
    std::list<DATA_MARKETITEM> lstMarketMisc;
};

struct GROUP_MEMBER {
    uint iClientID;
    std::wstring wscCharname;
};

struct SpecialChatIDs {
    enum : uint {
        CONSOLE = 0,

        PLAYER_MIN = 1,
        PLAYER_MAX = 249,

        SPECIAL_BASE = 0x10000,
        UNIVERSE = SPECIAL_BASE | 0,
        SYSTEM = SPECIAL_BASE | 1,
        LOCAL = SPECIAL_BASE | 2,
        GROUP = SPECIAL_BASE | 3,
        GROUP_EVENT = SPECIAL_BASE | 4
    };
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// prototypes
//

// HkInit
void PatchClientImpl();
bool InitHookExports();
void UnloadHookExports();
void HookRehashed();
void LoadUserCharSettings(uint clientID);

// HkFuncTools
EXPORT uint HkGetClientIdFromAccount(CAccount *acc);
EXPORT uint HkGetClientIdFromPD(struct PlayerData *pPD);
EXPORT CAccount *HkGetAccountByCharname(const std::wstring &wscCharname);
EXPORT uint HkGetClientIdFromCharname(const std::wstring &wscCharname);
EXPORT std::wstring HkGetAccountID(CAccount *acc);
EXPORT bool HkIsEncoded(const std::string &scFilename);
EXPORT bool HkIsInCharSelectMenu(const std::wstring &wscCharname);
EXPORT bool HkIsInCharSelectMenu(uint iClientID);
EXPORT bool HkIsValidClientID(uint iClientID);
EXPORT HK_ERROR HkResolveId(const std::wstring &wscCharname, uint &iClientID);
EXPORT HK_ERROR HkResolveShortCut(const std::wstring &wscShortcut,
                                  uint &iClientID);
EXPORT uint HkGetClientIDByShip(uint iShip);
EXPORT HK_ERROR HkGetAccountDirName(CAccount *acc, std::wstring &wscDir);
EXPORT HK_ERROR HkGetAccountDirName(const std::wstring &wscCharname,
                                    std::wstring &wscDir);
EXPORT HK_ERROR HkGetCharFileName(const std::wstring &wscCharname,
                                  std::wstring &wscFilename);
EXPORT std::wstring HkGetBaseNickByID(uint iBaseID);
EXPORT std::wstring HkGetPlayerSystem(uint iClientID);
EXPORT std::wstring HkGetSystemNickByID(uint iSystemID);
EXPORT void HkLockAccountAccess(CAccount *acc, bool bKick);
EXPORT void HkUnlockAccountAccess(CAccount *acc);
EXPORT void HkGetItemsForSale(uint iBaseID, std::list<uint> &lstItems);
EXPORT IObjInspectImpl *HkGetInspect(uint iClientID);
EXPORT ENGINE_STATE HkGetEngineState(uint iClientID);
EXPORT EQ_TYPE HkGetEqType(Archetype::Equipment *eq);
EXPORT float HkDistance3D(Vector v1, Vector v2);
EXPORT float HkDistance3DByShip(uint iShip1, uint iShip2);
EXPORT Quaternion HkMatrixToQuaternion(Matrix m);
template <typename Str> Str VectorToSectorCoord(uint iSystemID, Vector vPos);
template EXPORT std::string VectorToSectorCoord(uint iSystemID, Vector vPos);
template EXPORT std::wstring VectorToSectorCoord(uint iSystemID, Vector vPos);
EXPORT float degrees(float rad);
EXPORT Vector MatrixToEuler(const Matrix &mat);
EXPORT void Rotate180(Matrix &rot);
EXPORT void TranslateY(Vector &pos, Matrix &rot, float y);
EXPORT void TranslateX(Vector &pos, Matrix &rot, float x);
EXPORT void TranslateZ(Vector &pos, Matrix &rot, float z);

// HkFuncMsg
EXPORT HK_ERROR HkMsg(uint iClientID, const std::wstring &wscMessage);
EXPORT HK_ERROR HkMsg(const std::wstring &wscCharname,
                      const std::wstring &wscMessage);
EXPORT HK_ERROR HkMsgS(const std::wstring &wscSystemname,
                       const std::wstring &wscMessage);
EXPORT HK_ERROR HkMsgU(const std::wstring &wscMessage);
EXPORT HK_ERROR HkFMsgEncodeXML(const std::wstring &wscXML, char *szBuf,
                                uint iSize, uint &iRet);
EXPORT HK_ERROR HkFMsgSendChat(uint iClientID, char *szBuf, uint iSize);
EXPORT HK_ERROR HkFMsg(uint iClientID, const std::wstring &wscXML);
EXPORT HK_ERROR HkFMsg(const std::wstring &wscCharname,
                       const std::wstring &wscXML);
EXPORT HK_ERROR HkFMsgS(const std::wstring &wscSystemname,
                        const std::wstring &wscXML);
EXPORT HK_ERROR HkFMsgU(const std::wstring &wscXML);
EXPORT std::wstring HkGetWStringFromIDS(uint iIDS);
EXPORT void HkLoadStringDLLs();
EXPORT void HkUnloadStringDLLs();
EXPORT void FormatSendChat(uint iToClientID, const std::wstring &wscSender,
                           const std::wstring &wscText,
                           const std::wstring &wscTextColor);
EXPORT void SendGroupChat(uint iFromClientID, const std::wstring &wscText);
EXPORT void SendLocalSystemChat(uint iFromClientID,
                                const std::wstring &wscText);
EXPORT void SendPrivateChat(uint iFromClientID, uint iToClientID,
                            const std::wstring &wscText);
EXPORT void SendSystemChat(uint iFromClientID, const std::wstring &wscText);

// HkFuncPlayers
EXPORT HK_ERROR HkAddToGroup(uint iClientID, uint iGroupID);
EXPORT HK_ERROR HkGetGroupID(uint iClientID, uint &iGroupID);
EXPORT HK_ERROR HkGetCash(const std::wstring &wscCharname, int &iCash);
EXPORT HK_ERROR HkAddCash(const std::wstring &wscCharname, int iAmount);
EXPORT HK_ERROR HkKick(CAccount *acc);
EXPORT HK_ERROR HkKick(const std::wstring &wscCharname);
EXPORT HK_ERROR HkKickReason(const std::wstring &wscCharname,
                             const std::wstring &wscReason);
EXPORT HK_ERROR HkBan(const std::wstring &wscCharname, bool bBan);
EXPORT HK_ERROR HkBeam(const std::wstring &wscCharname,
                       const std::wstring &wscBasename);
EXPORT HK_ERROR HkSaveChar(const std::wstring &wscCharname);
EXPORT HK_ERROR HkEnumCargo(const std::wstring &wscCharname,
                            std::list<CARGO_INFO> &lstCargo,
                            int &iRemainingHoldSize);
EXPORT HK_ERROR HkRemoveCargo(const std::wstring &wscCharname, uint iID,
                              int iCount);
EXPORT HK_ERROR HkAddCargo(const std::wstring &wscCharname, uint iGoodID,
                           int iCount, bool bMission);
EXPORT HK_ERROR HkAddCargo(const std::wstring &wscCharname,
                           const std::wstring &wscGood, int iCount,
                           bool bMission);
EXPORT HK_ERROR HkRename(const std::wstring &wscCharname,
                         const std::wstring &wscNewCharname, bool bOnlyDelete);
EXPORT HK_ERROR HkMsgAndKick(uint iClientID, const std::wstring &wscReason,
                             uint iIntervall);
EXPORT HK_ERROR HkKill(const std::wstring &wscCharname);
EXPORT HK_ERROR HkGetReservedSlot(const std::wstring &wscCharname,
                                  bool &bResult);
EXPORT HK_ERROR HkSetReservedSlot(const std::wstring &wscCharname,
                                  bool bReservedSlot);
EXPORT void HkPlayerAutoBuy(uint iClientID, uint iBaseID);
EXPORT HK_ERROR HkResetRep(const std::wstring &wscCharname);
EXPORT HK_ERROR HkGetGroupMembers(const std::wstring &wscCharname,
                                  std::list<GROUP_MEMBER> &lstMembers);
EXPORT HK_ERROR HkSetRep(const std::wstring &wscCharname,
                         const std::wstring &wscRepGroup, float fValue);
EXPORT HK_ERROR HkGetRep(const std::wstring &wscCharname,
                         const std::wstring &wscRepGroup, float &fValue);
EXPORT HK_ERROR HkReadCharFile(const std::wstring &wscCharname,
                               std::list<std::wstring> &lstOutput);
EXPORT HK_ERROR HkWriteCharFile(const std::wstring &wscCharname,
                                std::wstring wscData);
EXPORT HK_ERROR HkPlayerRecalculateCRC(uint iClientID);
EXPORT std::string HkGetPlayerSystemS(uint iClientID);
EXPORT bool IsInRange(uint iClientID, float fDistance);
EXPORT std::wstring GetLocation(unsigned int iClientID);
EXPORT HK_ERROR HkAddEquip(const std::wstring &wscCharname, uint iGoodID,
                           const std::string &scHardpoint);
EXPORT HK_ERROR HkAntiCheat(uint iClientID);
EXPORT void HkDelayedKick(uint iClientID, uint secs);
EXPORT HK_ERROR HkDeleteCharacter(CAccount *acc, std::wstring &wscCharname);
EXPORT HK_ERROR HkNewCharacter(CAccount *acc, std::wstring &wscCharname);
EXPORT std::wstring HkGetAccountIDByClientID(uint iClientID);
EXPORT HK_ERROR HkGetOnlineTime(const std::wstring &wscCharname, int &iSecs);
EXPORT HK_ERROR HkGetRank(const std::wstring &wscCharname, int &iRank);
EXPORT HK_ERROR HKGetShipValue(const std::wstring &wscCharname, float &fValue);
EXPORT void HkRelocateClient(uint iClientID, Vector vDestination,
                             Matrix mOrientation);
EXPORT void HkSaveChar(uint iClientID);

// HkFuncLog
template<typename T>
const char* ToLogString(const T& val) {
    return "<undefined>";
}

EXPORT void AddDebugLog(const char *szString, ...);
EXPORT void AddLog(const char *szString, ...);
template<typename... Args>
void AddBothLog(const char* format, Args&&... args) {
    AddLog(format, std::forward<Args>(args)...);
    AddDebugLog(format, std::forward<Args>(args)...);
}

EXPORT void HkHandleCheater(uint iClientID, bool bBan, std::wstring wscReason,
                            ...);
EXPORT bool HkAddCheaterLog(const std::wstring &wscCharname,
                            const std::wstring &wscReason);
EXPORT bool HkAddCheaterLog(const uint &iClientID,
                            const std::wstring &wscReason);
EXPORT bool HkAddKickLog(uint iClientID, std::wstring wscReason, ...);
EXPORT bool HkAddConnectLog(uint iClientID, std::wstring wscReason, ...);
EXPORT void HkAddAdminCmdLog(const char *szString, ...);
EXPORT void HkAddSocketCmdLog(const char *szString, ...);
EXPORT void HkAddUserCmdLog(const char *szString, ...);
EXPORT void HkAddPerfTimerLog(const char *szString, ...);

// HkFuncOther
EXPORT void HkGetPlayerIP(uint iClientID, std::wstring &wscIP);
EXPORT HK_ERROR HkGetPlayerInfo(const std::wstring &wscCharname,
                                HKPLAYERINFO &pi, bool bAlsoCharmenu);
EXPORT std::list<HKPLAYERINFO> HkGetPlayers();
EXPORT HK_ERROR HkGetConnectionStats(uint iClientID, DPN_CONNECTION_INFO &ci);
EXPORT HK_ERROR HkSetAdmin(const std::wstring &wscCharname,
                           const std::wstring &wscRights);
EXPORT HK_ERROR HkGetAdmin(const std::wstring &wscCharname,
                           std::wstring &wscRights);
EXPORT HK_ERROR HkDelAdmin(const std::wstring &wscCharname);
EXPORT HK_ERROR HkChangeNPCSpawn(bool bDisable);
EXPORT HK_ERROR HkGetBaseStatus(const std::wstring &wscBasename, float &fHealth,
                                float &fMaxHealth);
EXPORT Fuse *HkGetFuseFromID(uint iFuseID);
EXPORT bool HkLightFuse(IObjRW *ship, uint iFuseID, float fDelay,
                        float fLifetime, float fSkip);
EXPORT bool HkUnLightFuse(IObjRW *ship, uint iFuseID);
EXPORT CEqObj *HkGetEqObjFromObjRW(struct IObjRW *objRW);
EXPORT uint HkGetClientIDFromArg(const std::wstring &wscArg);

// HkFLIni
EXPORT HK_ERROR HkFLIniGet(const std::wstring &wscCharname,
                           const std::wstring &wscKey, std::wstring &wscRet);
EXPORT HK_ERROR HkFLIniWrite(const std::wstring &wscCharname,
                             const std::wstring &wscKey, std::wstring wscValue);

EXPORT std::wstring HkErrGetText(HK_ERROR hkErr);
void ClearClientInfo(uint clientID);
void LoadUserSettings(uint iClientID);

// HkCbUserCmd
bool UserCmd_Process(uint iClientID, const std::wstring &wscCmd);
EXPORT void UserCmd_SetDieMsg(uint iClientID, std::wstring &wscParam);
EXPORT void UserCmd_SetChatFont(uint iClientID, std::wstring &wscParam);
EXPORT void PrintUserCmdText(uint iClientID, std::wstring wscText, ...);
EXPORT void PrintLocalUserCmdText(uint iClientID, const std::wstring &wscMsg,
                                  float fDistance);

bool AllowPlayerDamage(uint iClientID, uint iClientIDTarget);
void BaseDestroyed(uint objectID, uint clientIDBy);

EXPORT extern bool g_NonGunHitsBase;
EXPORT extern float g_LastHitPts;

// HkTimers
void HkTimerCheckKick();
void HkTimerNPCAndF1Check();
void HkThreadResolver();
void HkTimerCheckResolveResults();

extern EXPORT std::list<BASE_INFO> lstBases;
extern CRITICAL_SECTION csIPResolve;
extern std::list<RESOLVE_IP> g_lstResolveIPs;
extern std::list<RESOLVE_IP> g_lstResolveIPsResult;
extern HANDLE hThreadResolver;

// namespaces
namespace HkIServerImpl {
EXPORT extern bool g_InSubmitChat;
EXPORT extern uint g_TextLength;
} // namespace HkIServerImpl

extern HookEntry HkIServerImplEntries[85];

// HkDataBaseMarket
bool HkLoadBaseMarket();

// variables

extern EXPORT HkIClientImpl *FakeClient;
extern EXPORT HkIClientImpl *HookClient;
extern EXPORT char *OldClient;

extern EXPORT uint g_DmgTo;
extern EXPORT uint g_DmgToSpaceID;

extern EXPORT bool g_bMsg;
extern EXPORT bool g_bMsgS;
extern EXPORT bool g_bMsgU;

extern EXPORT CDPClientProxy **g_cClientProxyArray;
extern EXPORT void *pClient;

extern EXPORT _RCSendChatMsg RCSendChatMsg;
extern EXPORT _CRCAntiCheat CRCAntiCheat;
extern EXPORT _CreateChar CreateChar;

extern EXPORT std::string scAcctPath;

#define MAX_CLIENT_ID 249
extern EXPORT CLIENT_INFO ClientInfo[MAX_CLIENT_ID + 1];
extern EXPORT CDPServer *cdpSrv;
extern EXPORT uint g_iServerLoad;
extern EXPORT uint g_iPlayerCount;
extern EXPORT bool g_bNPCDisabled;
extern EXPORT char *g_FLServerDataPtr;

extern EXPORT bool g_bPlugin_nofunctioncall;

// help

typedef bool (*_HelpEntryDisplayed)(uint);
struct stHelpEntry {
    std::wstring wszCommand;
    std::wstring wszArguments;
    std::wstring wszShortHelp;
    std::wstring wszLongHelp;
    _HelpEntryDisplayed fnIsDisplayed;
};

extern std::list<stHelpEntry> lstHelpEntries;
extern EXPORT bool get_bTrue(uint iClientID);
extern EXPORT void HkAddHelpEntry(const std::wstring &wscCommand,
                                  const std::wstring &wscArguments,
                                  const std::wstring &wscShortHelp,
                                  const std::wstring &wscLongHelp,
                                  _HelpEntryDisplayed fnIsDisplayed);
extern EXPORT void HkRemoveHelpEntry(const std::wstring &wscCommand,
                                     const std::wstring &wscArguments);

extern EXPORT HK_ERROR HkGetClientID(bool &bIdString, uint &iClientID,
                                     const std::wstring &wscCharname);

#define HK_GET_CLIENTID(a, b)                                                  \
    bool bIdString = false;                                                    \
    uint a = uint(-1);                                                         \
    if (auto err = HkGetClientID(bIdString, a, b); err != HKE_OK)              \
        return err;

#define HK_GET_CLIENTID_OR_LOGGED_OUT(a, b)                                    \
    bool bIdString = false;                                                    \
    uint a = uint(-1);                                                         \
    if (auto err = HkGetClientID(bIdString, a, b);                             \
        err != HKE_OK && err != HKE_PLAYER_NOT_LOGGED_IN)                      \
        return err;
        
void HkIClientImpl__Startup__Inner(uint iDunno, uint iDunno2);

#define CALL_SERVER_PREAMBLE                                                    \
        {                                                                       \
            static CTimer timer(__FUNCTION__, set_iTimerThreshold);             \
            timer.start();                                                      \
            TRY_HOOK {

#define CALL_SERVER_POSTAMBLE(catchArgs, rval)                                  \
            } CATCH_HOOK({                                                      \
                AddLog("ERROR: Exception in " __FUNCTION__ " on server call");  \
                bool ret = catchArgs;                                           \
                if(!ret) {                                                      \
                    timer.stop();                                               \
                    return rval;                                                \
                }                                                               \
            })                                                                  \
            timer.stop();                                                       \
        }

#define CALL_CLIENT_PREAMBLE                                                    \
    {                                                                           \
        void *vRet;                                                             \
        char *tmp;                                                              \
        memcpy(&tmp, &Client, 4);                                               \
        memcpy(&Client, &OldClient, 4);                                         \

#define CALL_CLIENT_POSTAMBLE                                                   \
        __asm { mov [vRet], eax }                                               \
        memcpy(&Client, &tmp, 4);                                               \
    }

#define CHECK_FOR_DISCONNECT                                                    \
    {                                                                           \
        if (ClientInfo[clientID].bDisconnected) {                               \
            AddLog(                                                             \
                "ERROR: Ignoring disconnected client in " __FUNCTION__ " id=%"  \
                                                                       "u",     \
                clientID);                                                      \
            return;                                                             \
        };                                                                      \
    }

inline auto* ToWChar(const ushort* val) { return reinterpret_cast<const wchar_t*>(val); }
inline auto* ToWChar(ushort* val) { return reinterpret_cast<wchar_t*>(val); }

inline auto* ToUShort(const wchar_t* val) { return reinterpret_cast<const ushort*>(val); }
inline auto* ToUShort(wchar_t* val) { return reinterpret_cast<ushort*>(val); }
