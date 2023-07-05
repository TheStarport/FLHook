#pragma once
#include "Global.hpp"

// Doesn't call a Client method, so we need a custom hook
namespace IServerImplHook
{
    const std::unique_ptr<SubmitData> chatData = std::make_unique<SubmitData>();
    std::wstring g_CharBefore;
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


void PluginManager::setupProps()
{
    SetProps(HookedCall::IEngine__CShip__Init, true, false);
    SetProps(HookedCall::IEngine__CShip__Destroy, true, false);
    SetProps(HookedCall::IEngine__UpdateTime, true, true);
    SetProps(HookedCall::IEngine__ElapseTime, true, true);
    SetProps(HookedCall::IEngine__DockCall, true, false);
    SetProps(HookedCall::IEngine__LaunchPosition, true, false);
    SetProps(HookedCall::IEngine__ShipDestroyed, true, false);
    SetProps(HookedCall::IEngine__BaseDestroyed, true, false);
    SetProps(HookedCall::IEngine__GuidedHit, true, false);
    SetProps(HookedCall::IEngine__AddDamageEntry, true, true);
    SetProps(HookedCall::IEngine__DamageHit, true, false);
    SetProps(HookedCall::IEngine__AllowPlayerDamage, true, false);
    SetProps(HookedCall::IEngine__SendDeathMessage, true, false);
    SetProps(HookedCall::FLHook__TimerCheckKick, true, false);
    SetProps(HookedCall::FLHook__TimerNPCAndF1Check, true, false);
    SetProps(HookedCall::FLHook__UserCommand__Process, true, false);
    SetProps(HookedCall::FLHook__AdminCommand__Help, true, true);
    SetProps(HookedCall::FLHook__AdminCommand__Process, true, false);
    SetProps(HookedCall::FLHook__LoadSettings, true, true);
    SetProps(HookedCall::FLHook__LoadCharacterSettings, true, true);
    SetProps(HookedCall::FLHook__ClearClientInfo, true, true);
    SetProps(HookedCall::FLHook__ProcessEvent, true, false);
    SetProps(HookedCall::IChat__SendChat, true, false);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, true, true);
    SetProps(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULATUS, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, true, true);
    SetProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, true, true);
    SetProps(HookedCall::IServerImpl__FireWeapon, true, true);
    SetProps(HookedCall::IServerImpl__ActivateEquip, true, true);
    SetProps(HookedCall::IServerImpl__ActivateCruise, true, true);
    SetProps(HookedCall::IServerImpl__ActivateThrusters, true, true);
    SetProps(HookedCall::IServerImpl__SetTarget, true, true);
    SetProps(HookedCall::IServerImpl__TractorObjects, true, true);
    SetProps(HookedCall::IServerImpl__GoTradelane, true, true);
    SetProps(HookedCall::IServerImpl__StopTradelane, true, true);
    SetProps(HookedCall::IServerImpl__JettisonCargo, true, true);
    SetProps(HookedCall::IServerImpl__Startup, true, true);
    SetProps(HookedCall::IServerImpl__Shutdown, true, false);
    SetProps(HookedCall::IServerImpl__Update, true, true);
    SetProps(HookedCall::IServerImpl__DisConnect, true, true);
    SetProps(HookedCall::IServerImpl__OnConnect, true, true);
    SetProps(HookedCall::IServerImpl__Login, true, true);
    SetProps(HookedCall::IServerImpl__CharacterInfoReq, true, true);
    SetProps(HookedCall::IServerImpl__CharacterSelect, true, true);
    SetProps(HookedCall::IServerImpl__CreateNewCharacter, true, true);
    SetProps(HookedCall::IServerImpl__DestroyCharacter, true, true);
    SetProps(HookedCall::IServerImpl__ReqShipArch, true, true);
    SetProps(HookedCall::IServerImpl__ReqHulatus, true, true);
    SetProps(HookedCall::IServerImpl__ReqCollisionGroups, true, true);
    SetProps(HookedCall::IServerImpl__ReqEquipment, true, true);
    SetProps(HookedCall::IServerImpl__ReqAddItem, true, true);
    SetProps(HookedCall::IServerImpl__ReqRemoveItem, true, true);
    SetProps(HookedCall::IServerImpl__ReqModifyItem, true, true);
    SetProps(HookedCall::IServerImpl__ReqSetCash, true, true);
    SetProps(HookedCall::IServerImpl__ReqChangeCash, true, true);
    SetProps(HookedCall::IServerImpl__BaseEnter, true, true);
    SetProps(HookedCall::IServerImpl__BaseExit, true, true);
    SetProps(HookedCall::IServerImpl__LocationEnter, true, true);
    SetProps(HookedCall::IServerImpl__LocationExit, true, true);
    SetProps(HookedCall::IServerImpl__BaseInfoRequest, true, true);
    SetProps(HookedCall::IServerImpl__LocationInfoRequest, true, true);
    SetProps(HookedCall::IServerImpl__GFObjSelect, true, true);
    SetProps(HookedCall::IServerImpl__GFGoodVaporized, true, true);
    SetProps(HookedCall::IServerImpl__MissionResponse, true, true);
    SetProps(HookedCall::IServerImpl__TradeResponse, true, true);
    SetProps(HookedCall::IServerImpl__GFGoodBuy, true, true);
    SetProps(HookedCall::IServerImpl__GFGoodSell, true, true);
    SetProps(HookedCall::IServerImpl__SystemSwitchOutComplete, true, true);
    SetProps(HookedCall::IServerImpl__PlayerLaunch, true, true);
    SetProps(HookedCall::IServerImpl__LaunchComplete, true, true);
    SetProps(HookedCall::IServerImpl__JumpInComplete, true, true);
    SetProps(HookedCall::IServerImpl__Hail, true, true);
    SetProps(HookedCall::IServerImpl__SPObjUpdate, true, true);
    SetProps(HookedCall::IServerImpl__SPMunitionCollision, true, true);
    SetProps(HookedCall::IServerImpl__SPObjCollision, true, true);
    SetProps(HookedCall::IServerImpl__SPRequestUseItem, true, true);
    SetProps(HookedCall::IServerImpl__SPRequestInvincibility, true, true);
    SetProps(HookedCall::IServerImpl__RequestEvent, true, true);
    SetProps(HookedCall::IServerImpl__RequestCancel, true, true);
    SetProps(HookedCall::IServerImpl__MineAsteroid, true, true);
    SetProps(HookedCall::IServerImpl__RequestCreateShip, true, true);
    SetProps(HookedCall::IServerImpl__SPScanCargo, true, true);
    SetProps(HookedCall::IServerImpl__SetManeuver, true, true);
    SetProps(HookedCall::IServerImpl__InterfaceItemUsed, true, true);
    SetProps(HookedCall::IServerImpl__AbortMission, true, true);
    SetProps(HookedCall::IServerImpl__SetWeaponGroup, true, true);
    SetProps(HookedCall::IServerImpl__SetVisitedState, true, true);
    SetProps(HookedCall::IServerImpl__RequestBestPath, true, true);
    SetProps(HookedCall::IServerImpl__RequestPlayerStats, true, true);
    SetProps(HookedCall::IServerImpl__PopupDialog, true, true);
    SetProps(HookedCall::IServerImpl__RequestGroupPositions, true, true);
    SetProps(HookedCall::IServerImpl__SetInterfaceState, true, true);
    SetProps(HookedCall::IServerImpl__RequestRankLevel, true, true);
    SetProps(HookedCall::IServerImpl__InitiateTrade, true, true);
    SetProps(HookedCall::IServerImpl__TerminateTrade, true, true);
    SetProps(HookedCall::IServerImpl__AcceptTrade, true, true);
    SetProps(HookedCall::IServerImpl__SetTradeMoney, true, true);
    SetProps(HookedCall::IServerImpl__AddTradeEquip, true, true);
    SetProps(HookedCall::IServerImpl__DelTradeEquip, true, true);
    SetProps(HookedCall::IServerImpl__RequestTrade, true, true);
    SetProps(HookedCall::IServerImpl__StopTradeRequest, true, true);
    SetProps(HookedCall::IServerImpl__SubmitChat, true, true);
}
