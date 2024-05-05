#pragma once

class FLHook;
class IServerImplHook
{
        friend FLHook;
        struct SubmitData
        {
                bool inSubmitChat;
                std::wstring characterName;
        };

        inline static SubmitData chatData;
        inline static std::wstring charBefore;

        // The maximum number of players we can support is MaxClientId
        // Add one to the maximum number to allow renames
        inline static int maxPlayers = MaxClientId + 1;

        // Internal class of FLServer containing various bits of metadata and functions
        inline static void* dataPtr;

        // Base.cpp
        static void __stdcall BaseEnter(BaseId baseId, ClientId client);
        static void BaseEnterInnerAfter(BaseId baseId, ClientId client);
        static void BaseExitInner(BaseId baseId, ClientId client);
        static void __stdcall BaseExit(BaseId baseId, ClientId client);
        static void __stdcall BaseInfoRequest(unsigned int unk1, unsigned int unk2, bool unk3);
        static void __stdcall Dock([[maybe_unused]] const uint& unk1, [[maybe_unused]] const uint& unk2);

        // Cargo.cpp
        static void __stdcall SpScanCargo(const uint& unk1, const uint& unk2, uint unk3);
        static void __stdcall ReqAddItem(GoodId goodId, const char* hardpoint, int count, float status, bool mounted, ClientId client);
        static void __stdcall ReqRemoveItem(ushort slotId, int count, ClientId client);
        static void __stdcall ReqModifyItem(ushort slotId, const char* hardpoint, int count, float status, bool mounted, ClientId client);
        static void __stdcall JettisonCargo(ClientId client, const XJettisonCargo& jc);
        static void __stdcall TractorObjects(ClientId client, const XTractorObjects& to);

        // Cash.cpp
        static void __stdcall ReqSetCash(int cash, ClientId client);
        static void __stdcall ReqChangeCash(int cashAdd, ClientId client);

        // Character.cpp
        static void __stdcall CharacterInfoReq(ClientId client, bool unk1);
        static bool CharacterSelectInner(const CHARACTER_ID& cid, ClientId client);
        static void CharacterSelectInnerAfter(const CHARACTER_ID& charId, ClientId client);
        static void __stdcall CharacterSelect(const CHARACTER_ID& cid, ClientId client);
        static void __stdcall CreateNewCharacter(const SCreateCharacterInfo& createCharacterInfo, ClientId client);
        static void __stdcall DestroyCharacter(const CHARACTER_ID& unk1, ClientId client);
        static void DestroyCharacterCallback(ClientId client, CHARACTER_ID cid);
        static void __stdcall RequestRankLevel(ClientId client, uint unk1, int unk2);
        static void __stdcall RequestPlayerStats(ClientId client, uint unk1, int unk2);
        static bool CharacterInfoReqInner(ClientId client, bool);

        // ChatHooks.cpp
        static bool SubmitChatInner(ClientId from, ulong size, const void* rdlReader, ClientId to, int);
        static void __stdcall SubmitChat(CHAT_ID cidFrom, ulong size, const void* rdlReader, CHAT_ID cidTo, int unk1);
        static void __stdcall SendChat(ClientId client, ClientId clientTo, uint size, void* rdl);

        // Disconnect.cpp
        static void __stdcall DisConnect(ClientId client, EFLConnection conn);
        static bool OnConnectInner(ClientId client);

        // Equipment.cpp
        static void ActivateEquipInner(ClientId client, const XActivateEquip& aq);
        static void __stdcall ActivateEquip(ClientId client, const XActivateEquip& aq);
        static void __stdcall ReqEquipment(const EquipDescList& edl, ClientId client);
        static void __stdcall FireWeapon(ClientId client, const XFireWeaponInfo& fwi);
        static void __stdcall SetWeaponGroup(ClientId client, uint unk1, int unk2);
        static void __stdcall SpRequestUseItem(const SSPUseItem& ui, ClientId client);
        static void ActivateThrustersInner(ClientId client, const XActivateThrusters& at);
        static void __stdcall ActivateThrusters(ClientId client, const XActivateThrusters& at);
        static void ActivateCruiseInner(ClientId client, const XActivateCruise& ac);
        static void __stdcall ActivateCruise(ClientId client, const XActivateCruise& ac);

        // Goods.cpp
        static bool GFGoodSellInner(const SGFGoodSellInfo& gsi, ClientId client);
        static void __stdcall GFGoodSell(const SGFGoodSellInfo& unk1, ClientId client);
        static void __stdcall GFGoodBuy(const SGFGoodBuyInfo& unk1, ClientId client);
        static void __stdcall GFGoodVaporized(const SGFGoodVaporizedInfo& gvi, ClientId client);

        // Hooks.cpp
        static void ServerReady();
        static void UpdateInner();
        static void StartupInner(SStartupInfo& si);
        static void StartupInnerAfter(SStartupInfo& si);
        static int __stdcall Update();
        static void __stdcall Shutdown();
        static bool __stdcall Startup(SStartupInfo& si);

        // JumpInComplete.cpp
        static void __stdcall JumpInComplete(SystemId systemId, ShipId shipId);

        // LaunchComplete.cpp
        static void LaunchCompleteInner(BaseId, ShipId shipId);
        static void __stdcall LaunchComplete(BaseId baseId, ShipId shipId);

        // Location.cpp
        static void __stdcall SetVisitedState(ClientId client, uint objHash, int state);
        static void __stdcall RequestBestPath(ClientId client, RequestBestPathStruct* bestPath, int unused);
        static void __stdcall LocationInfoRequest(unsigned int unk1, unsigned int unk2, bool unk3);
        static void __stdcall LocationExit(uint locationId, ClientId client);
        static void __stdcall LocationEnter(uint locationId, ClientId client);

        // Login.cpp
        static void DelayedLogin(const SLoginInfo& li, ClientId client);
        static void LoginInnerAfter(const SLoginInfo& li, ClientId client);
        static void __stdcall Login(const SLoginInfo& li, ClientId client);

        // Maneuver.cpp
        static void __stdcall SetManeuver(ClientId client, const XSetManeuver& sm);

        // MineAsteroid.cpp
        static void __stdcall MineAsteroid(SystemId systemId, const Vector& pos, ArchId crateId, ArchId lootId, uint count, ClientId client);

        // Mission.cpp
        static void __stdcall AbortMission(ClientId client, uint unk1);
        static void __stdcall MissionResponse(unsigned int unk1, unsigned long unk2, bool unk3, ClientId client);

        // MunitionCollision.cpp
        static void SpMunitionCollisionInner(const SSPMunitionCollisionInfo& mci, uint);
        static void __stdcall SpMunitionCollision(const SSPMunitionCollisionInfo& mci, ClientId client);

        // ObjectCollision.cpp
        static void __stdcall SpObjCollision(const SSPObjCollisionInfo& oci, ClientId client);

        // ObjectSelect.cpp
        static void __stdcall GfObjSelect(unsigned int unk1, unsigned int unk2);

        // OnConnect.cpp
        static void __stdcall OnConnect(ClientId client);

        static void PlayerLaunchInner(ShipId shipId, ClientId client);
        static void PlayerLaunchInnerAfter(ShipId shipId, ClientId client);
        // PlayerLaunch.cpp
        static void __stdcall PlayerLaunch(ShipId shipId, ClientId client);

        // Ship.cpp
        static void __stdcall RequestCreateShip(ClientId client);
        static void __stdcall ReqCollisionGroups(const st6::list<CollisionGroupDesc>& collisionGroups, ClientId client);
        static void __stdcall ReqShipArch(uint archId, ClientId client);
        static void __stdcall ReqHullStatus(float status, ClientId client);
        static void __stdcall SpRequestInvincibility(ShipId shipId, bool enable, InvincibilityReason reason, ClientId client);

        // SpObjectUpdate.cpp
        static void __stdcall SpObjUpdate(const SSPObjUpdateInfo& ui, ClientId client);

        // SystemSwitchOutComplete.cpp
        static void __stdcall SystemSwitchOutComplete(ShipId shipId, ClientId client);

        // Trade.cpp
        static void __stdcall InitiateTrade(ClientId client1, ClientId client2);
        static void __stdcall AcceptTrade(ClientId client, bool unk1);
        static void __stdcall SetTradeMoney(ClientId client, ulong unk1);
        static void __stdcall AddTradeEquip(ClientId client, const EquipDesc& ed);
        static void __stdcall DelTradeEquip(ClientId client, const EquipDesc& ed);
        static void __stdcall RequestTrade(ClientId client1, ClientId client2);
        static void __stdcall StopTradeRequest(ClientId client);
        static void __stdcall TradeResponse(const unsigned char* unk1, int unk2, ClientId client);
        static void TerminateTradeInnerAfter(ClientId client, int accepted);
        static void __stdcall TerminateTrade(ClientId client, int accepted);

        // Tradelane.cpp
        static void __stdcall GoTradelane(ClientId client, const XGoTradelane& gt);
        static void __stdcall StopTradelane(ClientId client, ShipId shipId, ObjectId tradelaneRing1, ObjectId tradelaneRing2);

        // UserInterface.cpp
        static void __stdcall Hail(unsigned int unk1, unsigned int unk2, unsigned int unk3);
        static void __stdcall RequestEvent(int eventType, ShipId shipId, ObjectId dockTarget, uint unk1, ulong unk2, ClientId client);
        static void __stdcall RequestCancel(int eventType, ShipId shipId, ObjectId dockTarget, ulong unk2, ClientId client);
        static void __stdcall InterfaceItemUsed(uint unk1, uint unk2);
        static void __stdcall PopupDialog(ClientId client, PopupDialog buttonClicked);
        static void __stdcall SetInterfaceState(ClientId client, uint unk1, int unk2);
        static void __stdcall RequestGroupPositions(ClientId client, uint unk1, int unk2);
        static void __stdcall SetTarget(ClientId client, const XSetTarget& st);

        struct HookEntry
        {
                FARPROC proc;
                long remoteAddress;
                FARPROC oldProc;
        };

        static std::array<HookEntry, 73> entries;

    public:
        IServerImplHook() = delete;
};

#define CallServerPreamble                       \
    {                                            \
        static PerfTimer timer(FUNCTION_W, 100); \
        timer.Start();                           \
        TryHook                                  \
        {
#define CallServerPostamble(catchArgs, rval)                                                    \
    }                                                                                           \
    CatchHook({                                                                                 \
        Logger::Err(std::format(L"Exception in {} on server call", FUNCTION_W)); \
        bool ret = catchArgs;                                                                   \
        if (!ret)                                                                               \
        {                                                                                       \
            timer.Stop();                                                                       \
            return rval;                                                                        \
        }                                                                                       \
    }) timer.Stop();                                                                            \
    }

#define CallClientPreamble        \
    {                             \
        void* vRet;               \
        char* tmp;                \
        memcpy(&tmp, &Client, 4); \
        memcpy(&Client, &FLHook::oldClientImpl, 4);

#define CallClientPostamble   \
    __asm { mov [vRet], eax}  \
    memcpy(&Client, &tmp, 4); \
    }

#define CheckForDisconnect                                                                                              \
    {                                                                                                                   \
        if (client.GetData().disconnected)                                                                              \
        {                                                                                                               \
            Logger::Debug(std::format(L"Ignoring disconnected client in {} id={}", FUNCTION_W, client)); \
            return;                                                                                                     \
        };                                                                                                              \
    }
