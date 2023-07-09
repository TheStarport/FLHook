#pragma once
#include "Global.hpp"

// Doesn't call a Client method, so we need a custom hook
namespace IServerImplHook
{
    extern const std::unique_ptr<SubmitData> chatData;
    extern std::wstring g_CharBefore;

    // The maximum number of players we can support is MaxClientId
    // Add one to the maximum number to allow renames
    const int g_MaxPlayers = MaxClientId + 1;

    void __stdcall ReqSetCash(int cash, ClientId client);
    void __stdcall ReqChangeCash(int cashAdd, ClientId client);

    void __stdcall ActivateEquip(ClientId client, const XActivateEquip& aq);
    void __stdcall ReqEquipment(const EquipDescList& edl, ClientId client);
    void __stdcall FireWeapon(ClientId client, const XFireWeaponInfo& fwi);
    void __stdcall SetWeaponGroup(ClientId client, uint _genArg1, int _genArg2);
    void __stdcall SPRequestUseItem(const SSPUseItem& ui, ClientId client);
    void __stdcall ActivateThrusters(ClientId client, const XActivateThrusters& at);
    void __stdcall ActivateCruise(ClientId client, const XActivateCruise& ac);

    void __stdcall BaseEnter(uint baseId, ClientId client);
    void __stdcall BaseExit(uint baseId, ClientId client);
    void __stdcall BaseInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3);
    void __stdcall Dock([[maybe_unused]] const uint& genArg1, [[maybe_unused]] const uint& genArg2);

    void __stdcall SPScanCargo(const uint& _genArg1, const uint& _genArg2, uint _genArg3);
    void __stdcall ReqAddItem(uint goodId, const char* hardpoint, int count, float status, bool mounted, ClientId client);
    void __stdcall ReqRemoveItem(ushort slotId, int count, ClientId client);
    void __stdcall ReqModifyItem(ushort slotId, const char* hardpoint, int count, float status, bool mounted, ClientId client);
    void __stdcall JettisonCargo(ClientId client, const XJettisonCargo& jc);
    void __stdcall TractorObjects(ClientId client, const XTractorObjects& to);

    void __stdcall CharacterInfoReq(ClientId client, bool _genArg1);
    void __stdcall CharacterSelect(const CHARACTER_ID& cid, ClientId client);
    void __stdcall CreateNewCharacter(const SCreateCharacterInfo& _genArg1, ClientId client);
    void __stdcall DestroyCharacter(const CHARACTER_ID& _genArg1, ClientId client);
    void __stdcall RequestRankLevel(ClientId client, uint _genArg1, int _genArg2);
    void __stdcall RequestPlayerStats(ClientId client, uint _genArg1, int _genArg2);

    void __stdcall DisConnect(ClientId client, EFLConnection conn);
    void __stdcall OnConnect(ClientId client);

    void __stdcall GFGoodSell(const SGFGoodSellInfo& _genArg1, ClientId client);
    void __stdcall GFGoodBuy(const SGFGoodBuyInfo& _genArg1, ClientId client);
    void __stdcall GFGoodVaporized(const SGFGoodVaporizedInfo& gvi, ClientId client);

    void __stdcall GoTradelane(ClientId client, const XGoTradelane& gt);
    void __stdcall StopTradelane(ClientId client, uint shipId, uint tradelaneRing1, uint tradelaneRing2);

    void __stdcall JumpInComplete(uint systemId, uint shipId);
    void __stdcall LaunchComplete(uint baseId, uint shipId);

    void __stdcall SetVisitedState(ClientId client, uint objHash, int state);
    void __stdcall RequestBestPath(ClientId client, uint _genArg1, int _genArg2);
    void __stdcall LocationInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3);
    void __stdcall LocationExit(uint locationId, ClientId client);
    void __stdcall LocationEnter(uint locationId, ClientId client);

    void __stdcall Login(const SLoginInfo& li, ClientId client);
    void __stdcall SetManeuver(ClientId client, const XSetManeuver& sm);
    void __stdcall MineAsteroid(uint systemId, const Vector& pos, uint crateId, uint lootId, uint count, ClientId client);

    void __stdcall AbortMission(ClientId client, uint _genArg1);
    void __stdcall MissionResponse(unsigned int _genArg1, unsigned long _genArg2, bool _genArg3, ClientId client);

    void SPMunitionCollision__Inner(const SSPMunitionCollisionInfo& mci, uint);
    void __stdcall SPMunitionCollision(const SSPMunitionCollisionInfo& mci, ClientId client);
    void __stdcall SPObjCollision(const SSPObjCollisionInfo& oci, ClientId client);
    void __stdcall GFObjSelect(unsigned int _genArg1, unsigned int _genArg2);
    void __stdcall PlayerLaunch(uint shipId, ClientId client);

    void __stdcall RequestCreateShip(ClientId client);
    void __stdcall ReqCollisionGroups(const st6::list<CollisionGroupDesc>& collisionGroups, ClientId client);
    void __stdcall ReqShipArch(uint archId, ClientId client);
    void __stdcall ReqHulatus(float status, ClientId client);
    void __stdcall SPRequestInvincibility(uint shipId, bool enable, InvincibilityReason reason, ClientId client);

    void __stdcall SPObjUpdate(const SSPObjUpdateInfo& ui, ClientId client);
    void __stdcall SubmitChat(CHAT_ID cidFrom, ulong size, const void* rdlReader, CHAT_ID cidTo, int genArg1);
    void __stdcall SystemSwitchOutComplete(uint shipId, ClientId client);

    void __stdcall InitiateTrade(ClientId client1, ClientId client2);
    void __stdcall AcceptTrade(ClientId client, bool _genArg1);
    void __stdcall SetTradeMoney(ClientId client, ulong _genArg1);
    void __stdcall AddTradeEquip(ClientId client, const EquipDesc& ed);
    void __stdcall DelTradeEquip(ClientId client, const EquipDesc& ed);
    void __stdcall RequestTrade(uint _genArg1, uint _genArg2);
    void __stdcall StopTradeRequest(ClientId client);
    void __stdcall TradeResponse(const unsigned char* _genArg1, int _genArg2, ClientId client);
    void __stdcall TerminateTrade(ClientId client, int accepted);

    int __stdcall Update();
    void __stdcall Shutdown();
    bool __stdcall Startup(const SStartupInfo& si);

    void __stdcall Hail(unsigned int _genArg1, unsigned int _genArg2, unsigned int _genArg3);
    void __stdcall RequestEvent(int eventType, uint shipId, uint dockTarget, uint _genArg1, ulong _genArg2, ClientId client);
    void __stdcall RequestCancel(int eventType, uint shipId, uint _genArg1, ulong _genArg2, ClientId client);
    void __stdcall InterfaceItemUsed(uint _genArg1, uint _genArg2);
    void __stdcall PopupDialog(ClientId client, uint buttonClicked);
    void __stdcall SetInterfaceState(ClientId client, uint _genArg1, int _genArg2);
    void __stdcall RequestGroupPositions(ClientId client, uint _genArg1, int _genArg2);
    void __stdcall SetTarget(ClientId client, const XSetTarget& st);

} // namespace IServerImplHook

extern HookEntry IServerImplEntries[73];