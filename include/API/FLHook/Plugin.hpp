#pragma once

#include "Defs/Structs.hpp"

#include "API/FLHook/BsonHelper.hpp"

struct FlufPayload;
constexpr auto CurrentMajorVersion = PluginMajorVersion::V05;
constexpr auto CurrentMinorVersion = PluginMinorVersion::V00;

const std::wstring VersionInformation = std::to_wstring(static_cast<int>(CurrentMajorVersion)) + L"." + std::to_wstring(static_cast<int>(CurrentMinorVersion));

class IServerImplHook;
class HttpServer;
namespace httplib
{
    class Server;
}
struct DLL Timer
{
        friend IServerImplHook;

        int64 interval;
        int64 lastTime = TimeUtils::UnixTime<std::chrono::milliseconds>();
        std::optional<std::function<void(std::shared_ptr<Timer>)>> callback;

        static std::shared_ptr<Timer> Add(const std::function<void()>& function, uint intervalInMs);
        static std::shared_ptr<Timer> AddOneShot(const std::function<void()>& function, uint intervalInMs,
                                                 const std::optional<std::function<void(std::shared_ptr<Timer>)>>& callback = std::nullopt);

        /**
         *
         * @param function The function you want to invoke every time the specified interval is up
         * @param cronExpression A cron expression. See https://github.com/mariusbancila/croncpp for more information.
         * @code
         * // Example
         * Timer::AddCron(Func, L"0 0 0 * * *"); // Execute the function every day at 12AM
         * @endcode
         * @throws const cron::bad_cronexpr& - In the event of the expression being invalid it will throw.
         */
        static std::shared_ptr<Timer> AddCron(const std::function<void()>& function, std::wstring_view cronExpression);
        static void Remove(const std::shared_ptr<Timer>& timer);

    private:
        std::function<void()> func;

        struct CronTimer
        {
                time_t nextInterval;
                std::string cronExpression;
        };

        std::optional<CronTimer> cron = std::nullopt;

        inline static std::unordered_set<std::shared_ptr<Timer>> timers;
        inline static std::unordered_set<std::shared_ptr<Timer>> cronTimers;
        inline static std::unordered_set<std::shared_ptr<Timer>> oneShotTimers;
};

struct PluginInfo
{
        std::wstring name;
        std::wstring shortName;
        PluginMajorVersion versionMajor = PluginMajorVersion::Undefined;
        PluginMinorVersion versionMinor = PluginMinorVersion::Undefined;
        bool mayUnload = true;
        bool requiresFluf = false;
};

#ifdef FLHOOK
class PluginManager;
class IServerImplHook;
class IEngineHook;
class AdminCommandProcessor;
class AccountManager;
class FLHook;
#endif

class DLL Plugin
{
        static std::optional<std::weak_ptr<Plugin>> GetPluginFromManager(std::wstring_view shortName);
#ifdef FLHOOK
        friend IServerImplHook;
        friend PluginManager;
        friend AdminCommandProcessor;
        friend AccountManager;
        friend IEngineHook;
        friend FLHook;
        friend HttpServer;
#endif

        PluginMajorVersion versionMajor = PluginMajorVersion::Undefined;
        PluginMinorVersion versionMinor = PluginMinorVersion::Undefined;

        std::wstring name;
        std::wstring shortName;
        bool mayUnload;

        HMODULE dll = nullptr;
        std::wstring dllName;
        int callPriority = 0;
        std::unordered_set<std::shared_ptr<Timer>> timers;

    protected:
        ReturnCode returnCode = ReturnCode::Default;

        void SetPriority(const int priority) { callPriority = priority; }

        template <typename T>
        std::optional<std::weak_ptr<T>> GetPlugin(const std::wstring_view shortName)
            requires std::derived_from<T, Plugin>
        {
            const auto plugin = GetPluginFromManager(shortName);
            if (!plugin.has_value())
            {
                return std::nullopt;
            }

            const auto weakBase = plugin.value();
            std::weak_ptr<T> weakPlugin = std::static_pointer_cast<T>(weakBase.lock());
            return weakPlugin;
        }

        void AddTimer(const std::function<void()>& function, const uint intervalInMs) { timers.emplace(Timer::Add(function, intervalInMs)); }

        void AddOneShotTimer(const std::function<void()>& function, const uint intervalInMs)
        {
            timers.emplace(Timer::AddOneShot(function, intervalInMs, [this](const std::shared_ptr<Timer>& timer) { timers.erase(timer); }));
        }

        /**
         *
         * @param function The function you want to invoke every time the specified interval is up
         * @param cronExpression A cron expression. See https://github.com/mariusbancila/croncpp for more information.
         * @code
         * // Example
         * this->AddCronTimer(Func, L"0 0 0 * * *"); // Execute the function every day at 12AM
         * @endcode
         * @throws const cron::bad_cronexpr& - In the event of the expression being invalid it will throw.
         */
        void AddCronTimer(const std::function<void()>& function, std::wstring_view cronExpression) { timers.emplace(Timer::AddCron(function, cronExpression)); }

    public:
        explicit Plugin(const PluginInfo& info)
        {
            if (info.name.empty() || info.shortName.empty() || info.versionMajor == PluginMajorVersion::Undefined ||
                info.versionMinor == PluginMinorVersion::Undefined)
            {
                throw GameException(L"Plugin was created without valid data!");
            }

            name = info.name;
            shortName = info.shortName;
            mayUnload = info.mayUnload;
            versionMajor = info.versionMajor;
            versionMinor = info.versionMinor;
        }

        virtual ~Plugin()
        {
            auto timer = timers.begin();
            while (timer != timers.end())
            {
                Timer::Remove(*timer);
                timer = timers.erase(timer);
            }
        }

        Plugin& operator=(const Plugin&&) = delete;
        Plugin& operator=(const Plugin&) = delete;

        [[nodiscard]]
        std::wstring_view GetName() const
        {
            return name;
        }

        [[nodiscard]]
        std::wstring_view GetShortName() const
        {
            return shortName;
        }

    protected:
        // Define a macro that specifies a hook has an 'after' event
#define Aft(type, name, params)                 \
    virtual type name params { return type(); } \
    virtual void name##After params {}

        // Hooks
        virtual void OnCShipInitAfter(CShip* ship) {}
        virtual void OnCLootInitAfter(CLoot* loot) {}
        virtual void OnCSolarInitAfter(CSolar* solar) {}
        virtual void OnCGuidedInitAfter(CGuided* guided) {}

        virtual void OnShipDestroy(Ship* ship, DamageList* dmgList, ShipId killerId) {}
        virtual void OnShipDespawn(Ship* ship) {}
        virtual void OnShipEquipmentDestroy(Ship*, CEquip* eq, DamageEntry::SubObjFate fate, DamageList* dmg) {}
        virtual void OnLootDestroy(Loot* loot, DestroyType& destroyType, ShipId killerId) {}
        virtual void OnSolarDestroy(Solar* solar, DestroyType& destroyType, ShipId killerId) {}
        virtual void OnMineDestroy(Mine* mine, DestroyType& destroyType, ShipId killerId) {}
        virtual void OnGuidedDestroy(Guided* guided, DestroyType& destroyType, ShipId killerId) {}

        virtual void OnShipDropAllCargo(Ship* ship, const char* hardPoint, DamageList* dmgList) {}

        Aft(void, OnShipMunitionHit, (Ship* ship, MunitionImpactData* munition, DamageList* dmgList));

        virtual void OnShipHullDmg(Ship* ship, float& damage, DamageList* dmgList) {}
        virtual void OnSolarHullDmg(Solar* solar, float& damage, DamageList* dmgList) {}

        virtual void OnShipEquipDmg(Ship* ship, CAttachedEquip* eq, float& incDmg, DamageList* dmgList) {}
        virtual void OnShipEquipDestroy(Ship* ship, CEquip* eq, DamageEntry::SubObjFate fate, DamageList* dmgList) {}

        virtual void OnShipColGrpDmg(Ship* ship, CArchGroup* colGrp, float& incDmg, DamageList* dmg) {}
        virtual void OnShipColGrpDestroy(Ship* ship, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmg) {}

        virtual void OnSolarColGrpDestroy(Solar*, CArchGroup* colGrp, DamageEntry::SubObjFate fate, DamageList* dmg, bool killParented) {}

        virtual void OnShipShieldDmg(Ship* ship, CEShield* shield, float& damage, DamageList* dmgList) {}
        virtual void OnShipEnergyDmg(Ship* ship, float& damage, DamageList* dmgList) {}

        virtual void OnShipExplosionHit(Ship* ship, ExplosionDamageEvent* explosion, DamageList* dmgList) {}
        virtual void OnGuidedExplosionHit(Guided* guided, ExplosionDamageEvent* explosion, DamageList* dmgList){}
        virtual void OnSolarExplosionHit(Solar* solar, ExplosionDamageEvent* explosion, DamageList* dmgList){}

        virtual void OnShipFuse(Ship*, uint fuseCause, uint& fuseId, ushort sId, float radius, float fuseLifetime){}

        virtual void OnCELauncherFireAfter(CELauncher* launcher, const Vector& pos, FireResult) {}
        virtual int OnGetAmmoCapacity(CShip* ship, Id ammoArch) { return 0; }
        virtual float OnGetCargoRemaining(CShip* ship) { return 0.0f; }
        virtual int OnGetSpaceForCargoType(CShip* ship, Archetype::Equipment* cargo) { return 0; }

        virtual TractorFailureCode OnTractorVerifyTarget(CETractor* tractor, TractorFailureCode originalReturnValue) { return TractorFailureCode::Success; }

        /**
         * @brief Hook call for when a player attempts to dock with a station.
         * @param ShipId Ship id of the ship attempting to dock.
         * @param ObjectId the object Id of the station that the ship is attempting to dock with.
         * @param dockPortIndex the docking port number for the station.
         * @param response the dock behavior the game will execute without being overriden. eg DOCK_HOST_RESPONSE::ProceedDock here means the game intends for
         * the ship to proceed to the docking setup.
         * @returns nullopt will not override the base game behavior, sending a DOCK_HOST_RESPONSE will override the base game behavior.
         */
        Aft(std::optional<DOCK_HOST_RESPONSE>, OnDockCall, (const ShipId& shipId, const ObjectId& spaceId, int dockPortIndex, DOCK_HOST_RESPONSE response));
        virtual std::optional<LaunchData> OnLaunchPosition(const ObjectId& spaceId, const LaunchData& data) { return std::nullopt; }
        Aft(bool, OnGuidedHit, (const ShipId& inflictorShip, ClientId hitClient, const ObjectId& hitObject, DamageList* dmg));
        Aft(void, OnAddDamageEntry, (DamageList * dmg, ushort subObjId, float newHitPts, DamageEntry::SubObjFate fate));
        virtual void OnDamageHit(ClientId hitClient, const ObjectId& spaceId) {}
        Aft(bool, OnAllowPlayerDamage, (ClientId client, ClientId target));
        Aft(void, OnSendDeathMessage, (ClientId & killer, ClientId victim, SystemId system, std::wstring_view msg));
        virtual bool OnLoadSettings() { return true; }
        virtual void OnClearClientInfo(ClientId client) {}
        virtual void OnSendChat(ClientId client, ClientId targetClient, const uint size, void* rdl) {}
        Aft(void, OnFireWeapon, (ClientId client, const XFireWeaponInfo& info));
        Aft(void, OnActivateEquip, (ClientId client, const XActivateEquip& activate));
        Aft(void, OnActivateCruise, (ClientId client, const XActivateCruise& activate));
        Aft(void, OnActivateThrusters, (ClientId client, const XActivateThrusters& activate));
        Aft(void, OnSetTarget, (ClientId client, const XSetTarget& target));
        Aft(void, OnTradelaneStart, (ClientId client, const XGoTradelane& tradelane));
        Aft(void, OnTradelaneStop, (ClientId client, const ShipId& ship, const ObjectId& tradelaneRing1, const ObjectId& tradelaneRing2));
        Aft(bool, OnServerStartup, ());
        virtual void OnServerShutdown() {}
        Aft(int, OnServerUpdate, ());
        Aft(void, OnLogin, (ClientId client, const SLoginInfo& li));
        Aft(void, OnConnect, (ClientId client));
        Aft(void, OnDisconnect, (ClientId client, EFLConnection connection));
        Aft(void, OnCharacterInfoRequest, (ClientId client, bool unk1));
        Aft(void, OnCharacterSelect, (ClientId client));
        Aft(void, OnCharacterCreation, (ClientId client, const SCreateCharacterInfo& info));
        Aft(void, OnCharacterDelete, (ClientId client, std::wstring_view charName));
        virtual void OnCharacterSave(ClientId client, std::wstring_view charName, B_DOC& document) {};
        Aft(void, OnRequestShipArch, (ClientId client, ArchId arch));
        Aft(void, OnRequestHullStatus, (ClientId client, float status));
        Aft(void, OnRequestCollisionGroups, (ClientId client, const st6::list<CollisionGroupDesc>& groups));
        Aft(void, OnRequestEquipment, (ClientId client, const EquipDescList& edl));
        Aft(void, OnRequestModifyItem, (ClientId client, ushort slotId, std::wstring_view hardpoint, int count, float status, bool mounted));
        Aft(void, OnRequestAddItem, (ClientId client, GoodId& goodId, std::wstring_view hardpoint, int count, float status, bool mounted));
        Aft(void, OnRequestRemoveItem, (ClientId client, ushort slotId, int count));
        Aft(void, OnCargoJettison, (ClientId client, const XJettisonCargo& cargo));
        Aft(void, OnTractorObjects, (ClientId client, const XTractorObjects& tractor));
        Aft(void, OnRequestSetCash, (ClientId client, int cash));
        Aft(void, OnRequestChangeCash, (ClientId client, int cash));
        Aft(void, OnBaseEnter, (BaseId base, ClientId client));
        Aft(void, OnBaseExit, (BaseId base, ClientId client));
        Aft(void, OnLocationEnter, (ClientId client, LocationId location));
        Aft(void, OnLocationExit, (ClientId client, LocationId location));
        Aft(void, OnRequestBaseInfo, (uint unk1, uint unk2, uint unk3));
        Aft(void, OnRequestLocationInfo, (uint unk1, uint unk2, uint unk3));
        Aft(void, OnGfObjectSelect, (uint unk1, uint unk2));
        Aft(void, OnGfGoodBuy, (ClientId client, const SGFGoodBuyInfo& info));
        Aft(void, OnGfGoodSell, (ClientId client, const SGFGoodSellInfo& info));
        Aft(void, OnGfGoodVaporized, (ClientId client, const SGFGoodVaporizedInfo& info));
        Aft(void, OnMissionResponse, (ClientId client, uint unk1, ulong unk2, bool unk3));
        Aft(void, OnSystemSwitchOutComplete, (ClientId client, const ShipId& ship));
        Aft(void, OnPlayerLaunch, (ClientId client, const ShipId& ship));
        Aft(void, OnLaunchComplete, (BaseId baseId, const ShipId& ship));
        Aft(void, OnJumpInComplete, (SystemId system, const ShipId& ship));
        Aft(void, OnHail, (uint unk1, uint unk2, uint unk3));
        Aft(void, OnSpObjectUpdate, (ClientId client, const SSPObjUpdateInfo& info));
        Aft(void, OnSpMunitionCollision, (ClientId client, const SSPMunitionCollisionInfo& info));
        Aft(void, OnSpObjectCollision, (ClientId client, const SSPObjCollisionInfo& info));
        Aft(void, OnSpScanCargo, (uint unk1, uint unk2, uint unk3));
        Aft(void, OnSpRequestUseItem, (ClientId client, const SSPUseItem& item));
        Aft(void, OnSpRequestInvincibility, (ClientId client, const ShipId& shipId, bool enable, InvincibilityReason reason));
        Aft(void, OnRequestEvent, (ClientId client, EventRequestType eventType, const ShipId& ship, const ObjectId& dockTarget, uint unk1, uint unk2));
        Aft(void, OnRequestCancel, (ClientId client, EventRequestType eventType, const ShipId& ship, const ObjectId& dockTarget, uint unk1));
        Aft(void, OnMineAsteroid, (ClientId client, SystemId system, const Vector& pos, Id crateId, Id lootId, uint count));
        Aft(void, OnRequestCreateShip, (ClientId client));
        Aft(void, OnSetManeuver, (ClientId client, const XSetManeuver& maneuver));
        Aft(void, OnInterfaceItemUsed, (uint unk1, uint unk2));
        Aft(void, OnAbortMission, (ClientId client, uint unk1));
        Aft(void, OnSetWeaponGroup, (ClientId client, uint unk1, int unk2));
        Aft(void, OnSetVisitedState, (ClientId client, uint objectHash, int state));
        Aft(void, OnRequestBestPath, (ClientId client, RequestBestPathStruct* bestPath, int unused));
        Aft(void, OnRequestPlayerStats, (ClientId id, uint unk1, int unk2));
        Aft(void, OnPopupDialogueConfirm, (ClientId client, PopupDialog buttonClicked));
        Aft(void, OnRequestGroupPositions, (ClientId client, uint unk1, int unk2));
        Aft(void, OnSetInterfaceState, (ClientId client, uint unk1, int unk2));
        Aft(void, OnRequestRankLevel, (ClientId client, uint unk1, int unk2));
        Aft(void, OnTradeResponse, (ClientId client, const unsigned char* unk1, int unk2));
        Aft(void, OnInitiateTrade, (ClientId client1, ClientId client2));
        Aft(void, OnTerminateTrade, (ClientId client, int accepted));
        Aft(void, OnAcceptTrade, (ClientId client, bool newTradeAcceptState));
        Aft(void, OnSetTradeMoney, (ClientId client, ulong cash));
        Aft(void, OnAddTradeEquip, (ClientId client, const EquipDesc& equip));
        Aft(void, OnRemoveTradeEquip, (ClientId client, const EquipDesc& equip));
        Aft(void, OnStopTradeRequest, (ClientId client));
        Aft(void, OnRequestTrade, (ClientId client1, ClientId client2));
        Aft(void, OnSubmitChat, (ClientId from, ulong size, const void* rdlReader, ClientId to, int genArg1));
        virtual void OnHttpServerRegister(std::shared_ptr<httplib::Server> httpServer) {}
        virtual void OnPayloadReceived(ClientId client, const FlufPayload&) {};
};

class DLL PacketInterface
{
    public:
        virtual ~PacketInterface() = default;
        Aft(bool, OnFireWeaponPacket, (ClientId client, XFireWeaponInfo& info));
        Aft(bool, OnActivateEquipPacket, (ClientId client, XActivateEquip& activate));
        Aft(bool, OnActivateCruisePacket, (ClientId client, XActivateCruise& activate));
        Aft(bool, OnActivateThrusterPacket, (ClientId client, XActivateThrusters& activate));
        Aft(bool, OnSetShipArchPacket, (ClientId client, ArchId arch));
        Aft(bool, OnSetHullStatusPacket, (ClientId client, float status));
        Aft(bool, OnSetCollisionGroupsPacket, (ClientId client, st6::list<CollisionGroupDesc>& collisionGroupList));
        Aft(bool, OnSetEquipmentPacket, (ClientId client, st6::vector<EquipDesc>& equipmentVector));
        Aft(bool, OnSetAddItemPacket, (ClientId client, FLPACKET_UNKNOWN* unk1, FLPACKET_UNKNOWN* unk2));
        Aft(bool, OnSetStartRoomPacket, (ClientId client, uint unk1, uint unk2));
        Aft(bool, OnCreateSolarPacket, (ClientId client, FLPACKET_CREATESOLAR& solar));
        Aft(bool, OnCreateLootPacket, (ClientId client, FLPACKET_UNKNOWN* unk1));
        Aft(bool, OnCreateShipPacket, (ClientId client, FLPACKET_CREATESHIP& ship));
        Aft(bool, OnCreateMinePacket, (ClientId client, FLPACKET_UNKNOWN* unk1));
        Aft(bool, OnCreateGuidedPacket, (ClientId client, FLPACKET_CREATEGUIDED& guided));
        Aft(bool, OnCreateCounterPacket, (ClientId client, FLPACKET_UNKNOWN* unk1));
        Aft(bool, OnUpdateObjectPacket, (ClientId client, SSPObjUpdateInfo& update));
        Aft(bool, OnDestroyObjectPacket, (ClientId client, FLPACKET_DESTROYOBJECT& destroy));
        Aft(bool, OnActivateObjectPacket, (ClientId client, XActivateEquip& aq));
        Aft(bool, OnLaunchPacket, (ClientId client, FLPACKET_LAUNCH& launch));
        Aft(bool, OnRequestCreateShipResponsePacket, (ClientId client, bool response, ShipId shipId));
        Aft(bool, OnUseItemPacket, (ClientId client, uint unk1));
        Aft(bool, OnSetReputationPacket, (ClientId client, FLPACKET_SETREPUTATION& rep));
        Aft(bool, OnSetMissionMessagePacket, (ClientId client, FLPACKET_UNKNOWN* unk1));
        Aft(bool, OnSetMissionObjectivesPacket, (ClientId client, uint unk1));
        Aft(bool, OnSetCashPacket, (ClientId client, uint cash));
        Aft(bool, OnBurnFusePacket, (ClientId client, FLPACKET_BURNFUSE& burnFuse));
        Aft(bool, OnScanNotifyPacket, (ClientId client, uint unk1, uint unk2));
        Aft(bool, OnPlayerListPacket, (ClientId client, std::wstring_view characterName, uint unk2, char unk3));
        Aft(bool, OnPlayerList2Packet, (ClientId client));
        Aft(bool, OnMiscObjectUpdatePacket, (ClientId client, FLPACKET_UNKNOWN* unk1));
        Aft(bool, OnMiscObjectUpdate2Packet, (ClientId client, uint unk1, uint unk2));
        Aft(bool, OnMiscObjectUpdate3Packet, (ClientId client, uint targetId, uint rank));
        Aft(bool, OnMiscObjectUpdate4Packet, (ClientId client, uint unk1, uint unk2));
        Aft(bool, OnMiscObjectUpdate5Packet, (ClientId client, uint unk1, uint unk2));
        Aft(bool, OnMiscObjectUpdate6Packet, (ClientId client, uint unk1, uint unk2));
        Aft(bool, OnMiscObjectUpdate7Packet, (ClientId client, uint unk1, uint unk2));
        Aft(bool, OnSystemSwitchOutPacket, (ClientId client, FLPACKET_SYSTEM_SWITCH_OUT& packet));
};

#undef Aft

#define SetupPlugin(type)                                                     \
    EXPORT std::shared_ptr<type> PluginFactory()                              \
    {                                                                         \
        auto pi = getPi();                                                    \
        __pragma(comment(linker, "/EXPORT:" __FUNCTION__ "=" __FUNCDNAME__)); \
        return std::move(std::make_shared<type>(pi));                         \
    }
