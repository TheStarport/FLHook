#ifndef _GLOBAL_
#define _GLOBAL_
#pragma warning(disable : 4311 4786)

// includes
#include "blowfish.h"
#include <functional>
#include <list>
#include <map>
#include <set>
#include <stdio.h>
#include <string>
#include <windows.h>

typedef void *(*st6_malloc_t)(size_t);
typedef void (*st6_free_t)(void *);

extern EXPORT st6_malloc_t st6_malloc;
extern EXPORT st6_free_t st6_free;
#define ST6_ALLOCATION_DEFINED

#include <FLCoreCommon.h>
#include <FLCoreDALib.h>
#include <FLCoreRemoteClient.h>
#include <FLCoreServer.h>

// defines
#define VERSION L"2.0.0 plugin"

#define TIME_UPDATE 50
#define IMPORT __declspec(dllimport)
#define EXPORT __declspec(dllexport)

// typedefs
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long ulong;
typedef unsigned __int64 mstime;

// structures
struct INISECTIONVALUE {
    std::string scKey;
    std::string scValue;
};

struct MULTIKILLMESSAGE {
    uint iKillsInARow;
    std::wstring wscMessage;
};

// functions
bool FLHookInit();
void FLHookInit_Pre();
void FLHookShutdown();
EXPORT void ProcessEvent(std::wstring wscText, ...);
void LoadSettings();
void ProcessPendingCommands();

// tools
EXPORT std::wstring stows(const std::string &scText);
EXPORT std::string wstos(const std::wstring &wscText);
EXPORT std::string IniGetS(const std::string &scFile, const std::string &scApp,
                           const std::string &scKey,
                           const std::string &scDefault);
EXPORT int IniGetI(const std::string &scFile, const std::string &scApp,
                   const std::string &scKey, int iDefault);
EXPORT bool IniGetB(const std::string &scFile, const std::string &scApp,
                    const std::string &scKey, bool bDefault);
EXPORT void IniWrite(const std::string &scFile, const std::string &scApp,
                     const std::string &scKey, const std::string &scValue);
EXPORT void WriteProcMem(void *pAddress, void *pMem, int iSize);
EXPORT void ReadProcMem(void *pAddress, void *pMem, int iSize);
EXPORT int ToInt(const std::wstring &wscStr);
EXPORT uint ToUInt(const std::wstring &wscStr);
EXPORT void ConPrint(std::wstring wscText, ...);
EXPORT std::wstring XMLText(const std::wstring &wscText);
EXPORT std::wstring GetParam(const std::wstring &wscLine, wchar_t wcSplitChar,
                             uint iPos);
EXPORT std::string GetParam(std::string scLine, char cSplitChar, uint iPos);
EXPORT std::wstring ReplaceStr(const std::wstring &wscSource,
                               const std::wstring &wscSearchFor,
                               const std::wstring &wscReplaceWith);
EXPORT void IniDelSection(const std::string &scFile, const std::string &scApp);
EXPORT void IniDelete(const std::string &scFile, const std::string &scApp,
                      const std::string &scKey);
EXPORT void IniWriteW(const std::string &scFile, const std::string &scApp,
                      const std::string &scKey, const std::wstring &wscValue);
EXPORT std::wstring IniGetWS(const std::string &scFile,
                             const std::string &scApp, const std::string &scKey,
                             const std::wstring &wscDefault);
EXPORT std::wstring ToMoneyStr(int iCash);
EXPORT float IniGetF(const std::string &scFile, const std::string &scApp,
                     const std::string &scKey, float fDefault);
EXPORT void IniGetSection(const std::string &scFile, const std::string &scApp,
                          std::list<INISECTIONVALUE> &lstValues);
EXPORT float ToFloat(const std::wstring &wscStr);
EXPORT mstime timeInMS();
EXPORT void SwapBytes(void *ptr, uint iLen);
EXPORT FARPROC PatchCallAddr(char *hMod, DWORD dwInstallAddress,
                             char *dwHookFunction);
template <typename Str> Str Trim(const Str &scIn);
template EXPORT std::string Trim(const std::string &scIn);
template EXPORT std::wstring Trim(const std::wstring &scIn);
EXPORT BOOL FileExists(LPCTSTR szPath);
EXPORT std::wstring ToLower(std::wstring wscStr);
EXPORT std::string ToLower(std::string wscStr);
EXPORT std::wstring GetParamToEnd(const std::wstring &wscLine,
                                  wchar_t wcSplitChar, uint iPos);
EXPORT void ini_write_wstring(FILE *file, const std::string &parmname,
                              const std::wstring &in);
EXPORT void ini_get_wstring(INI_Reader &ini, std::wstring &wscValue);
EXPORT std::wstring GetTimeString(bool bLocalTime);
EXPORT std::string GetUserFilePath(const std::wstring &wscCharname,
                                   const std::string &scExtension);
EXPORT mstime GetTimeInMS();

// variables
extern EXPORT HANDLE hProcFL;
extern EXPORT HMODULE hModServer;
extern EXPORT HMODULE hModCommon;
extern EXPORT HMODULE hModRemoteClient;
extern EXPORT HMODULE hModDPNet;
extern EXPORT HMODULE hModDaLib;
extern EXPORT HMODULE hModContent;
extern EXPORT FILE *fLog;
extern EXPORT FILE *fLogDebug;
extern EXPORT FARPROC fpOldUpdate;
extern EXPORT std::string sDebugLog;

// setting variables
extern EXPORT bool set_bLoadedSettings;
extern EXPORT std::string set_scCfgFile;
extern EXPORT uint set_iAntiDockKill;
extern EXPORT std::set<uint> set_setNoPVPSystems;
extern EXPORT std::set<std::wstring> set_setChatSuppress;
extern EXPORT bool set_bSocketActivated;
extern EXPORT bool set_bDebug;
extern EXPORT bool set_bLogConnects;
extern EXPORT std::wstring set_wscDeathMsgStyle;
extern EXPORT std::wstring set_wscDeathMsgStyleSys;
extern EXPORT bool set_bDieMsg;
extern EXPORT bool set_bDisableCharfileEncryption;
extern EXPORT uint set_iAntiBaseIdle;
extern EXPORT uint set_iAntiCharMenuIdle;
extern EXPORT bool set_bChangeCruiseDisruptorBehaviour;
extern EXPORT bool set_bUserCmdSetDieMsg;
extern EXPORT bool set_bUserCmdSetChatFont;
extern EXPORT bool set_bUserCmdIgnore;
extern EXPORT bool set_bUserCmdHelp;
extern EXPORT bool set_bDefaultLocalChat;
extern EXPORT uint set_iDisableNPCSpawns;
extern EXPORT int set_iPort;
extern EXPORT int set_iWPort;
extern EXPORT int set_iEPort;
extern EXPORT int set_iEWPort;
extern EXPORT BLOWFISH_CTX *set_BF_CTX;
extern EXPORT std::wstring set_wscKickMsg;
extern EXPORT std::wstring set_wscUserCmdStyle;
extern EXPORT std::wstring set_wscAdminCmdStyle;
extern EXPORT uint set_iKickMsgPeriod;
extern EXPORT std::wstring set_wscDeathMsgTextPlayerKill;
extern EXPORT std::wstring set_wscDeathMsgTextSelfKill;
extern EXPORT std::wstring set_wscDeathMsgTextNPC;
extern EXPORT std::wstring set_wscDeathMsgTextSuicide;
extern EXPORT uint set_iAntiF1;
extern EXPORT std::wstring set_wscDeathMsgTextAdminKill;
extern EXPORT uint set_iUserCmdMaxIgnoreList;
extern EXPORT uint set_iReservedSlots;
extern EXPORT uint set_iDisconnectDelay;
extern EXPORT bool set_bAutoBuy;
extern EXPORT float set_fTorpMissileBaseDamageMultiplier;
extern EXPORT bool set_bPersistGroup;
extern EXPORT bool set_MKM_bActivated;
extern EXPORT std::wstring set_MKM_wscStyle;
extern EXPORT std::list<MULTIKILLMESSAGE> set_MKM_lstMessages;
extern EXPORT bool set_bUserCmdSetDieMsgSize;
extern EXPORT uint set_iMaxGroupSize;
extern EXPORT std::set<std::wstring> set_setBans;
extern EXPORT bool set_bBanAccountOnMatch;
extern EXPORT uint set_iTimerThreshold;
extern EXPORT uint set_iTimerDebugThreshold;
extern EXPORT uint set_iDebugMaxSize;
extern EXPORT bool set_bLogAdminCmds;
extern EXPORT bool set_bLogSocketCmds;
extern EXPORT bool set_bLogLocalSocketCmds;
extern EXPORT bool set_bLogUserCmds;
extern EXPORT bool set_bPerfTimer;

struct SYSTEMINFO {
    /** The system nickname */
    std::string sysNick;

    /** The system id */
    uint systemId;

    /** The system scale */
    float scale;
};

struct TransformMatrix {
    float d[4][4];
};

struct ZONE {
    /** The system nickname */
    std::string sysNick;

    /** The zone nickname */
    std::string zoneNick;

    /** The id of the system for this zone */
    uint systemId;

    /** The zone transformation matrix */
    TransformMatrix transform;

    /** The zone ellipsoid size */
    Vector size;

    /** The zone position */
    Vector pos;

    /** The damage this zone causes per second */
    int damage;

    /** Is this an encounter zone */
    bool encounter;
};

class JUMPPOINT {
  public:
    /** The system nickname */
    std::string sysNick;

    /** The jump point nickname */
    std::string jumpNick;

    /** The jump point destination system nickname */
    std::string jumpDestSysNick;

    /** The id of the system for this jump point. */
    uint System;

    /** The id of the jump point. */
    uint jumpID;

    /** The jump point destination system id */
    uint jumpDestSysID;
};

struct LOOTABLE_ZONE {
    /** The zone nickname */
    std::string zoneNick;

    /** The id of the system for this lootable zone */
    uint systemID;

    /** The nickname and arch id of the loot dropped by the asteroids */
    std::string lootNick;
    uint iLootID;

    /** The arch id of the crate the loot is dropped in */
    uint iCrateID;

    /** The minimum number of loot items to drop */
    uint iMinLoot;

    /** The maximum number of loot items to drop */
    uint iMaxLoot;

    /** The drop difficultly */
    uint iLootDifficulty;

    /** The lootable zone ellipsoid size */
    Vector size;

    /** The lootable zone position */
    Vector pos;
};
typedef std::multimap<uint, LOOTABLE_ZONE, std::less<>> zone_map_t;

/** A map of system id to system info */
extern EXPORT std::map<uint, SYSTEMINFO> mapSystems;

/** A map of system id to zones */
extern EXPORT std::multimap<uint, ZONE> zones;

/** A map of system id to jumppoint info */
extern EXPORT std::multimap<uint, JUMPPOINT> jumpPoints;

namespace ZoneUtilities {
EXPORT void ReadUniverse(zone_map_t *set_mmapZones);
EXPORT void ReadLootableZone(zone_map_t &set_mmapZones,
                             const std::string &systemNick,
                             const std::string &defaultZoneNick,
                             const std::string &file);
EXPORT void ReadSystemLootableZones(zone_map_t &set_mmapZones,
                                    const std::string &systemNick,
                                    const std::string &file);
EXPORT void ReadSystemZones(zone_map_t &set_mmapZones,
                            const std::string &systemNick,
                            const std::string &file);
EXPORT bool InZone(uint systemID, const Vector &pos, ZONE &rlz);
EXPORT bool InDeathZone(uint systemID, const Vector &pos, ZONE &rlz);
EXPORT SYSTEMINFO *GetSystemInfo(uint systemID);
EXPORT void PrintZones();
} // namespace ZoneUtilities

#endif
