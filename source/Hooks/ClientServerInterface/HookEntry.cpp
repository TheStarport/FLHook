#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    const std::unique_ptr<SubmitData> chatData = std::make_unique<SubmitData>();
}

#define FarProcCast reinterpret_cast<FARPROC>
HookEntry IServerImplEntries[] = {
    {             FarProcCast(IServerImplHook::SubmitChat), -0x008, nullptr},
    {             FarProcCast(IServerImplHook::FireWeapon),  0x000, nullptr},
    {          FarProcCast(IServerImplHook::ActivateEquip),  0x004, nullptr},
    {         FarProcCast(IServerImplHook::ActivateCruise),  0x008, nullptr},
    {      FarProcCast(IServerImplHook::ActivateThrusters),  0x00C, nullptr},
    {              FarProcCast(IServerImplHook::SetTarget),  0x010, nullptr},
    {         FarProcCast(IServerImplHook::TractorObjects),  0x014, nullptr},
    {            FarProcCast(IServerImplHook::GoTradelane),  0x018, nullptr},
    {          FarProcCast(IServerImplHook::StopTradelane),  0x01C, nullptr},
    {          FarProcCast(IServerImplHook::JettisonCargo),  0x020, nullptr},
    {             FarProcCast(IServerImplHook::DisConnect),  0x040, nullptr},
    {              FarProcCast(IServerImplHook::OnConnect),  0x044, nullptr},
    {                  FarProcCast(IServerImplHook::Login),  0x048, nullptr},
    {       FarProcCast(IServerImplHook::CharacterInfoReq),  0x04C, nullptr},
    {        FarProcCast(IServerImplHook::CharacterSelect),  0x050, nullptr},
    {     FarProcCast(IServerImplHook::CreateNewCharacter),  0x058, nullptr},
    {       FarProcCast(IServerImplHook::DestroyCharacter),  0x05C, nullptr},
    {            FarProcCast(IServerImplHook::ReqShipArch),  0x064, nullptr},
    {             FarProcCast(IServerImplHook::ReqHulatus),  0x068, nullptr},
    {     FarProcCast(IServerImplHook::ReqCollisionGroups),  0x06C, nullptr},
    {           FarProcCast(IServerImplHook::ReqEquipment),  0x070, nullptr},
    {             FarProcCast(IServerImplHook::ReqAddItem),  0x078, nullptr},
    {          FarProcCast(IServerImplHook::ReqRemoveItem),  0x07C, nullptr},
    {          FarProcCast(IServerImplHook::ReqModifyItem),  0x080, nullptr},
    {             FarProcCast(IServerImplHook::ReqSetCash),  0x084, nullptr},
    {          FarProcCast(IServerImplHook::ReqChangeCash),  0x088, nullptr},
    {              FarProcCast(IServerImplHook::BaseEnter),  0x08C, nullptr},
    {               FarProcCast(IServerImplHook::BaseExit),  0x090, nullptr},
    {          FarProcCast(IServerImplHook::LocationEnter),  0x094, nullptr},
    {           FarProcCast(IServerImplHook::LocationExit),  0x098, nullptr},
    {        FarProcCast(IServerImplHook::BaseInfoRequest),  0x09C, nullptr},
    {    FarProcCast(IServerImplHook::LocationInfoRequest),  0x0A0, nullptr},
    {            FarProcCast(IServerImplHook::GFObjSelect),  0x0A4, nullptr},
    {        FarProcCast(IServerImplHook::GFGoodVaporized),  0x0A8, nullptr},
    {        FarProcCast(IServerImplHook::MissionResponse),  0x0AC, nullptr},
    {          FarProcCast(IServerImplHook::TradeResponse),  0x0B0, nullptr},
    {              FarProcCast(IServerImplHook::GFGoodBuy),  0x0B4, nullptr},
    {             FarProcCast(IServerImplHook::GFGoodSell),  0x0B8, nullptr},
    {FarProcCast(IServerImplHook::SystemSwitchOutComplete),  0x0BC, nullptr},
    {           FarProcCast(IServerImplHook::PlayerLaunch),  0x0C0, nullptr},
    {         FarProcCast(IServerImplHook::LaunchComplete),  0x0C4, nullptr},
    {         FarProcCast(IServerImplHook::JumpInComplete),  0x0C8, nullptr},
    {                   FarProcCast(IServerImplHook::Hail),  0x0CC, nullptr},
    {            FarProcCast(IServerImplHook::SPObjUpdate),  0x0D0, nullptr},
    {    FarProcCast(IServerImplHook::SPMunitionCollision),  0x0D4, nullptr},
    {         FarProcCast(IServerImplHook::SPObjCollision),  0x0DC, nullptr},
    {       FarProcCast(IServerImplHook::SPRequestUseItem),  0x0E0, nullptr},
    { FarProcCast(IServerImplHook::SPRequestInvincibility),  0x0E4, nullptr},
    {           FarProcCast(IServerImplHook::RequestEvent),  0x0F0, nullptr},
    {          FarProcCast(IServerImplHook::RequestCancel),  0x0F4, nullptr},
    {           FarProcCast(IServerImplHook::MineAsteroid),  0x0F8, nullptr},
    {      FarProcCast(IServerImplHook::RequestCreateShip),  0x100, nullptr},
    {            FarProcCast(IServerImplHook::SPScanCargo),  0x104, nullptr},
    {            FarProcCast(IServerImplHook::SetManeuver),  0x108, nullptr},
    {      FarProcCast(IServerImplHook::InterfaceItemUsed),  0x10C, nullptr},
    {           FarProcCast(IServerImplHook::AbortMission),  0x110, nullptr},
    {         FarProcCast(IServerImplHook::SetWeaponGroup),  0x118, nullptr},
    {        FarProcCast(IServerImplHook::SetVisitedState),  0x11C, nullptr},
    {        FarProcCast(IServerImplHook::RequestBestPath),  0x120, nullptr},
    {     FarProcCast(IServerImplHook::RequestPlayerStats),  0x124, nullptr},
    {            FarProcCast(IServerImplHook::PopupDialog),  0x128, nullptr},
    {  FarProcCast(IServerImplHook::RequestGroupPositions),  0x12C, nullptr},
    {      FarProcCast(IServerImplHook::SetInterfaceState),  0x134, nullptr},
    {       FarProcCast(IServerImplHook::RequestRankLevel),  0x138, nullptr},
    {          FarProcCast(IServerImplHook::InitiateTrade),  0x13C, nullptr},
    {         FarProcCast(IServerImplHook::TerminateTrade),  0x140, nullptr},
    {            FarProcCast(IServerImplHook::AcceptTrade),  0x144, nullptr},
    {          FarProcCast(IServerImplHook::SetTradeMoney),  0x148, nullptr},
    {          FarProcCast(IServerImplHook::AddTradeEquip),  0x14C, nullptr},
    {          FarProcCast(IServerImplHook::DelTradeEquip),  0x150, nullptr},
    {           FarProcCast(IServerImplHook::RequestTrade),  0x154, nullptr},
    {       FarProcCast(IServerImplHook::StopTradeRequest),  0x158, nullptr},
    {                   FarProcCast(IServerImplHook::Dock),  0x16C, nullptr},
};

#undef Farproccast

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
