//
// WARNING: THIS IS AN AUTO-GENERATED FILE, DO NOT EDIT!
//

#include <Hook.h>

bool HkIClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(uint iClientID, XFireWeaponInfo& fwi) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(\n\tuint iClientID = %u\n\tXFireWeaponInfo& fwi = %s\n)",
			iClientID, fwi);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON,
			iClientID, fwi);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_FIREWEAPON(iClientID, fwi));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON,
			iClientID, fwi);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(uint iClientID, XActivateEquip& aq) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(\n\tuint iClientID = %u\n\tXActivateEquip& aq = %s\n)",
			iClientID, aq);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP,
			iClientID, aq);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_ACTIVATEEQUIP(iClientID, aq));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP,
			iClientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(uint iClientID, XActivateCruise& aq) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(\n\tuint iClientID = %u\n\tXActivateCruise& aq = %s\n)",
			iClientID, aq);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE,
			iClientID, aq);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_ACTIVATECRUISE(iClientID, aq));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE,
			iClientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(uint iClientID, XActivateThrusters& aq) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(\n\tuint iClientID = %u\n\tXActivateThrusters& aq = %s\n)",
			iClientID, aq);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS,
			iClientID, aq);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(iClientID, aq));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS,
			iClientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SETTARGET(uint iClientID, XSetTarget& st) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_SETTARGET(\n\tuint iClientID = %u\n\tXSetTarget& st = %s\n)",
			iClientID, st);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SETTARGET,
			iClientID, st);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_SETTARGET(iClientID, st));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SETTARGET,
			iClientID, st);

	return retVal;
}

void HkIClientImpl::unknown_6(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_6(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_METHOD(unknown_6(iClientID, pDunno));
}

bool HkIClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(uint iClientID, XGoTradelane& tl) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(\n\tuint iClientID = %u\n\tXGoTradelane& tl = %s\n)",
			iClientID, tl);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_GOTRADELANE,
			iClientID, tl);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_GOTRADELANE(iClientID, tl));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_GOTRADELANE,
			iClientID, tl);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(uint iClientID, uint iShip, uint iArchTradelane1, uint iArchTradelane2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(\n\tuint iClientID = %u\n\tuint iShip = %u\n\tuint iArchTradelane1 = %u\n\tuint iArchTradelane2 = %u\n)",
			iClientID, iShip, iArchTradelane1, iArchTradelane2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_STOPTRADELANE,
			iClientID, iShip, iArchTradelane1, iArchTradelane2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_STOPTRADELANE(iClientID, iShip, iArchTradelane1, iArchTradelane2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_STOPTRADELANE,
			iClientID, iShip, iArchTradelane1, iArchTradelane2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(uint iClientID, XJettisonCargo& jc) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(\n\tuint iClientID = %u\n\tXJettisonCargo& jc = %s\n)",
			iClientID, jc);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_JETTISONCARGO,
			iClientID, jc);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_JETTISONCARGO(iClientID, jc));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_JETTISONCARGO,
			iClientID, jc);

	return retVal;
}

bool HkIClientImpl::SendPacket(uint iClientID, void* _genArg1) {
	AddDebugLog("HkIClientImpl::SendPacket(\n\tuint iClientID = %u\n\tvoid* _genArg1 = %p\n)",
			iClientID, _genArg1);

	auto retVal = CALL_CLIENT_METHOD(SendPacket(iClientID, _genArg1));
	return retVal;
}

bool HkIClientImpl::Startup(uint _genArg1, uint _genArg2) {
	HkIClientImpl__Startup__Inner(_genArg1, _genArg2);

	auto retVal = CALL_CLIENT_METHOD(Startup(_genArg1, _genArg2));
	return retVal;
}

void HkIClientImpl::nullsub(uint _genArg1) {
	AddDebugLog("HkIClientImpl::nullsub(\n\tuint _genArg1 = %u\n)",
			_genArg1);

	CALL_CLIENT_METHOD(nullsub(_genArg1));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LOGINRESPONSE,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_LOGINRESPONSE(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LOGINRESPONSE,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CHARACTERINFO,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CHARACTERINFO(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CHARACTERINFO,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CHARSELECTVERIFIED,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CHARSELECTVERIFIED(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CHARSELECTVERIFIED,
			iClientID, pDunno);

	return retVal;
}

void HkIClientImpl::Shutdown() {
	CALL_CLIENT_METHOD(Shutdown());
}

bool HkIClientImpl::CDPClientProxy__Disconnect(uint iClientID) {
	AddDebugLog("HkIClientImpl::CDPClientProxy__Disconnect(\n\tuint iClientID = %u\n)",
			iClientID);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__CDPClientProxy__Disconnect,
			iClientID);

	if(!skip) retVal = CALL_CLIENT_METHOD(CDPClientProxy__Disconnect(iClientID));

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__Disconnect,
			iClientID);

	return retVal;
}

uint HkIClientImpl::CDPClientProxy__GetSendQSize(uint iClientID) {
	AddDebugLog("HkIClientImpl::CDPClientProxy__GetSendQSize(\n\tuint iClientID = %u\n)",
			iClientID);

	auto [retVal, skip] = CallPluginsBefore<uint>(HookedCall::IClientImpl__CDPClientProxy__GetSendQSize,
			iClientID);

	if(!skip) retVal = CALL_CLIENT_METHOD(CDPClientProxy__GetSendQSize(iClientID));

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__GetSendQSize,
			iClientID);

	return retVal;
}

uint HkIClientImpl::CDPClientProxy__GetSendQBytes(uint iClientID) {
	AddDebugLog("HkIClientImpl::CDPClientProxy__GetSendQBytes(\n\tuint iClientID = %u\n)",
			iClientID);

	auto [retVal, skip] = CallPluginsBefore<uint>(HookedCall::IClientImpl__CDPClientProxy__GetSendQBytes,
			iClientID);

	if(!skip) retVal = CALL_CLIENT_METHOD(CDPClientProxy__GetSendQBytes(iClientID));

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__GetSendQBytes,
			iClientID);

	return retVal;
}

double HkIClientImpl::CDPClientProxy__GetLinkSaturation(uint iClientID) {
	AddDebugLog("HkIClientImpl::CDPClientProxy__GetLinkSaturation(\n\tuint iClientID = %u\n)",
			iClientID);

	auto [retVal, skip] = CallPluginsBefore<double>(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation,
			iClientID);

	if(!skip) retVal = CALL_CLIENT_METHOD(CDPClientProxy__GetLinkSaturation(iClientID));

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation,
			iClientID);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(uint iClientID, uint iShipArch) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(\n\tuint iClientID = %u\n\tuint iShipArch = %u\n)",
			iClientID, iShipArch);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH,
			iClientID, iShipArch);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETSHIPARCH(iClientID, iShipArch));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH,
			iClientID, iShipArch);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(uint iClientID, float fStatus) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(\n\tuint iClientID = %u\n\tfloat fStatus = %f\n)",
			iClientID, fStatus);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS,
			iClientID, fStatus);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETHULLSTATUS(iClientID, fStatus));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS,
			iClientID, fStatus);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETEQUIPMENT(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT,
			iClientID, pDunno);

	return retVal;
}

void HkIClientImpl::unknown_26(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_26(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_26(iClientID, iDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETADDITEM(uint iClientID, FLPACKET_UNKNOWN& pDunno, FLPACKET_UNKNOWN& pDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETADDITEM(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n\tFLPACKET_UNKNOWN& pDunno2 = %s\n)",
			iClientID, pDunno, pDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM,
			iClientID, pDunno, pDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETADDITEM(iClientID, pDunno, pDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM,
			iClientID, pDunno, pDunno2);

	return retVal;
}

void HkIClientImpl::unknown_28(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3) {
	AddDebugLog("HkIClientImpl::unknown_28(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n\tuint iDunno3 = %u\n)",
			iClientID, iDunno, iDunno2, iDunno3);

	CALL_CLIENT_METHOD(unknown_28(iClientID, iDunno, iDunno2, iDunno3));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETSTARTROOM(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYCHARACTER,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFDESTROYCHARACTER(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYCHARACTER,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATECHAR,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFUPDATECHAR(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATECHAR,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETECHARLIST,
			iClientID, iDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(iClientID, iDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETECHARLIST,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST,
			iClientID, iDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(iClientID, iDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST,
			iClientID, iDunno);

	return retVal;
}

void HkIClientImpl::unknown_36(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_36(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_METHOD(unknown_36(iClientID, iDunno, iDunno2));
}

void HkIClientImpl::unknown_37(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_37(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_METHOD(unknown_37(iClientID, iDunno, iDunno2));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST,
			iClientID, iDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(iClientID, iDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST,
			iClientID, iDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(iClientID, iDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(uint iClientID, uint iReason) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(\n\tuint iClientID = %u\n\tuint iReason = %u\n)",
			iClientID, iReason);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY,
			iClientID, iReason);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(iClientID, iReason));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY,
			iClientID, iReason);

	return retVal;
}

void HkIClientImpl::unknown_44(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_44(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_METHOD(unknown_44(iClientID, iDunno, iDunno2));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST,
			iClientID, iDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(iClientID, iDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(uint iClientID, FLPACKET_CREATESOLAR& pSolar) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(\n\tuint iClientID = %u\n\tFLPACKET_CREATESOLAR& pSolar = %s\n)",
			iClientID, pSolar);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR,
			iClientID, pSolar);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATESOLAR(iClientID, pSolar));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR,
			iClientID, pSolar);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATESHIP(uint iClientID, FLPACKET_CREATESHIP& pShip) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATESHIP(\n\tuint iClientID = %u\n\tFLPACKET_CREATESHIP& pShip = %s\n)",
			iClientID, pShip);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP,
			iClientID, pShip);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATESHIP(iClientID, pShip));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP,
			iClientID, pShip);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATELOOT(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATELOOT(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATELOOT(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATEMINE(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATEMINE(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATEMINE(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATEGUIDED(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATECOUNTER(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER,
			iClientID, pDunno);

	return retVal;
}

void HkIClientImpl::unknown_53(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_53(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_METHOD(unknown_53(iClientID, pDunno));
}

void HkIClientImpl::unknown_54(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3) {
	AddDebugLog("HkIClientImpl::unknown_54(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n\tuint iDunno3 = %u\n)",
			iClientID, iDunno, iDunno2, iDunno3);

	CALL_CLIENT_METHOD(unknown_54(iClientID, iDunno, iDunno2, iDunno3));
}

bool HkIClientImpl::Send_FLPACKET_COMMON_UPDATEOBJECT(uint iClientID, SSPObjUpdateInfo& pUpdate) {
	auto retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_UPDATEOBJECT(iClientID, pUpdate));
	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(uint iClientID, FLPACKET_DESTROYOBJECT& pDestroy) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(\n\tuint iClientID = %u\n\tFLPACKET_DESTROYOBJECT& pDestroy = %s\n)",
			iClientID, pDestroy);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT,
			iClientID, pDestroy);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_DESTROYOBJECT(iClientID, pDestroy));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT,
			iClientID, pDestroy);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(uint iClientID, XActivateEquip& aq) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(\n\tuint iClientID = %u\n\tXActivateEquip& aq = %s\n)",
			iClientID, aq);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT,
			iClientID, aq);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_ACTIVATEOBJECT(iClientID, aq));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT,
			iClientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LAND(uint iClientID, FLPACKET_LAND& pLand) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_LAND(\n\tuint iClientID = %u\n\tFLPACKET_LAND& pLand = %s\n)",
			iClientID, pLand);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAND,
			iClientID, pLand);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_LAND(iClientID, pLand));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAND,
			iClientID, pLand);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LAUNCH(uint iClientID, FLPACKET_LAUNCH& pLaunch) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_LAUNCH(\n\tuint iClientID = %u\n\tFLPACKET_LAUNCH& pLaunch = %s\n)",
			iClientID, pLaunch);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH,
			iClientID, pLaunch);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_LAUNCH(iClientID, pLaunch));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH,
			iClientID, pLaunch);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(uint iClientID, bool bResponse, uint iShipID) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(\n\tuint iClientID = %u\n\tbool bResponse = %d\n\tuint iShipID = %u\n)",
			iClientID, bResponse, iShipID);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP,
			iClientID, bResponse, iShipID);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(iClientID, bResponse, iShipID));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP,
			iClientID, bResponse, iShipID);

	return retVal;
}

void HkIClientImpl::unknown_63(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_63(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_METHOD(unknown_63(iClientID, pDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(uint iClientID, uint iObj, int& dmlist) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(\n\tuint iClientID = %u\n\tuint iObj = %u\n\tint& dmlist = %s\n)",
			iClientID, iObj, dmlist);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DAMAGEOBJECT,
			iClientID, iObj, dmlist);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_DAMAGEOBJECT(iClientID, iObj, dmlist));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DAMAGEOBJECT,
			iClientID, iObj, dmlist);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ITEMTRACTORED,
			iClientID, iDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_ITEMTRACTORED(iClientID, iDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ITEMTRACTORED,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_USE_ITEM(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_USE_ITEM(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM,
			iClientID, iDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_USE_ITEM(iClientID, iDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(uint iClientID, FLPACKET_SETREPUTATION& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(\n\tuint iClientID = %u\n\tFLPACKET_SETREPUTATION& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETREPUTATION(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION,
			iClientID, pDunno);

	return retVal;
}

void HkIClientImpl::unknown_68(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_68(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_METHOD(unknown_68(iClientID, pDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SENDCOMM(uint iClientID, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6, uint _genArg7, uint _genArg8, uint _genArg9, uint _genArg10, uint _genArg11, uint _genArg12, uint _genArg13, uint _genArg14, uint _genArg15, uint _genArg16, uint _genArg17, uint _genArg18, uint _genArg19, uint _genArg20, uint _genArg21, uint _genArg22) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SENDCOMM(\n\tuint iClientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint _genArg3 = %u\n\tuint _genArg4 = %u\n\tuint _genArg5 = %u\n\tuint _genArg6 = %u\n\tuint _genArg7 = %u\n\tuint _genArg8 = %u\n\tuint _genArg9 = %u\n\tuint _genArg10 = %u\n\tuint _genArg11 = %u\n\tuint _genArg12 = %u\n\tuint _genArg13 = %u\n\tuint _genArg14 = %u\n\tuint _genArg15 = %u\n\tuint _genArg16 = %u\n\tuint _genArg17 = %u\n\tuint _genArg18 = %u\n\tuint _genArg19 = %u\n\tuint _genArg20 = %u\n\tuint _genArg21 = %u\n\tuint _genArg22 = %u\n)",
			iClientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM,
			iClientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SENDCOMM(iClientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM,
			iClientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);

	return retVal;
}

void HkIClientImpl::unknown_70(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_70(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_70(iClientID, iDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE,
			iClientID, pDunno);

	return retVal;
}

void HkIClientImpl::unknown_72(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_72(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_METHOD(unknown_72(iClientID, pDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES,
			iClientID, iDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(iClientID, iDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES,
			iClientID, iDunno);

	return retVal;
}

void HkIClientImpl::unknown_74(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_74(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_METHOD(unknown_74(iClientID, pDunno));
}

void HkIClientImpl::unknown_75(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_75(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_75(iClientID, iDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MARKOBJ(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MARKOBJ(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MARKOBJ,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MARKOBJ(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MARKOBJ,
			iClientID, iDunno, iDunno2);

	return retVal;
}

void HkIClientImpl::unknown_77(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_77(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_77(iClientID, iDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETCASH(uint iClientID, uint iCash) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETCASH(\n\tuint iClientID = %u\n\tuint iCash = %u\n)",
			iClientID, iCash);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH,
			iClientID, iCash);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETCASH(iClientID, iCash));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH,
			iClientID, iCash);

	return retVal;
}

void HkIClientImpl::unknown_79(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_79(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_79(iClientID, iDunno));
}

void HkIClientImpl::unknown_80(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_80(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_80(iClientID, iDunno));
}

void HkIClientImpl::unknown_81(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_81(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_81(iClientID, iDunno));
}

void HkIClientImpl::unknown_82(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_82(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_82(iClientID, iDunno));
}

void HkIClientImpl::unknown_83(uint iClientID, char* szDunno) {
	AddDebugLog("HkIClientImpl::unknown_83(\n\tuint iClientID = %u\n\tchar* szDunno = %s\n)",
			iClientID, szDunno);

	CALL_CLIENT_METHOD(unknown_83(iClientID, szDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(uint iClientID, uint iShipID, uint iFlag, uint iDunno3, uint iDunno4) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(\n\tuint iClientID = %u\n\tuint iShipID = %u\n\tuint iFlag = %u\n\tuint iDunno3 = %u\n\tuint iDunno4 = %u\n)",
			iClientID, iShipID, iFlag, iDunno3, iDunno4);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUEST_RETURNED,
			iClientID, iShipID, iFlag, iDunno3, iDunno4);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_REQUEST_RETURNED(iClientID, iShipID, iFlag, iDunno3, iDunno4));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUEST_RETURNED,
			iClientID, iShipID, iFlag, iDunno3, iDunno4);

	return retVal;
}

void HkIClientImpl::unknown_85(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_85(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_METHOD(unknown_85(iClientID, pDunno));
}

void HkIClientImpl::unknown_86(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3) {
	AddDebugLog("HkIClientImpl::unknown_86(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n\tuint iDunno3 = %u\n)",
			iClientID, iDunno, iDunno2, iDunno3);

	CALL_CLIENT_METHOD(unknown_86(iClientID, iDunno, iDunno2, iDunno3));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_OBJECTCARGOUPDATE,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_OBJECTCARGOUPDATE,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_BURNFUSE(uint iClientID, FLPACKET_BURNFUSE& pBurnfuse) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_BURNFUSE(\n\tuint iClientID = %u\n\tFLPACKET_BURNFUSE& pBurnfuse = %s\n)",
			iClientID, pBurnfuse);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE,
			iClientID, pBurnfuse);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_BURNFUSE(iClientID, pBurnfuse));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE,
			iClientID, pBurnfuse);

	return retVal;
}

void HkIClientImpl::unknown_89(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_89(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_METHOD(unknown_89(iClientID, pDunno));
}

void HkIClientImpl::unknown_90(uint iClientID) {
	AddDebugLog("HkIClientImpl::unknown_90(\n\tuint iClientID = %u\n)",
			iClientID);

	CALL_CLIENT_METHOD(unknown_90(iClientID));
}

void HkIClientImpl::unknown_91(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_91(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_91(iClientID, iDunno));
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_WEAPON_GROUP,
			iClientID, p2, p3);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_SET_WEAPON_GROUP(iClientID, p2, p3));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_WEAPON_GROUP,
			iClientID, p2, p3);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_VISITED_STATE,
			iClientID, p2, p3);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_SET_VISITED_STATE(iClientID, p2, p3));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_VISITED_STATE,
			iClientID, p2, p3);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_BEST_PATH,
			iClientID, p2, p3);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_REQUEST_BEST_PATH(iClientID, p2, p3));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_BEST_PATH,
			iClientID, p2, p3);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS,
			iClientID, p2, p3);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(iClientID, p2, p3));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS,
			iClientID, p2, p3);

	return retVal;
}

void HkIClientImpl::unknown_96(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3) {
	AddDebugLog("HkIClientImpl::unknown_96(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n\tuint iDunno3 = %u\n)",
			iClientID, iDunno, iDunno2, iDunno3);

	CALL_CLIENT_METHOD(unknown_96(iClientID, iDunno, iDunno2, iDunno3));
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS,
			iClientID, p2, p3);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(iClientID, p2, p3));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS,
			iClientID, p2, p3);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_MISSION_LOG,
			iClientID, p2, p3);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_SET_MISSION_LOG(iClientID, p2, p3));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_MISSION_LOG,
			iClientID, p2, p3);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_INTERFACE_STATE,
			iClientID, p2, p3);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_SET_INTERFACE_STATE(iClientID, p2, p3));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_INTERFACE_STATE,
			iClientID, p2, p3);

	return retVal;
}

void HkIClientImpl::unknown_100(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_100(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_METHOD(unknown_100(iClientID, iDunno, iDunno2));
}

void HkIClientImpl::unknown_101(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_101(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_METHOD(unknown_101(iClientID, pDunno));
}

void HkIClientImpl::unknown_102(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_102(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_102(iClientID, iDunno));
}

void HkIClientImpl::unknown_103(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_103(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_103(iClientID, iDunno));
}

void HkIClientImpl::unknown_104(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_104(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_METHOD(unknown_104(iClientID, iDunno, iDunno2));
}

void HkIClientImpl::unknown_105(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_105(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_METHOD(unknown_105(iClientID, iDunno, iDunno2));
}

void HkIClientImpl::unknown_106(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_106(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_METHOD(unknown_106(iClientID, iDunno, iDunno2));
}

void HkIClientImpl::unknown_107(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_107(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_METHOD(unknown_107(iClientID, iDunno, iDunno2));
}

bool HkIClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_PLAYER_TRADE,
			iClientID, iDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_PLAYER_TRADE(iClientID, iDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_PLAYER_TRADE,
			iClientID, iDunno);

	return retVal;
}

void HkIClientImpl::unknown_109(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_109(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_109(iClientID, iDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SCANNOTIFY(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(uint iClientID, wchar_t* _genArg1, uint _genArg2, char _genArg3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(\n\tuint iClientID = %u\n\twchar_t* _genArg1 = %p\n\tuint _genArg2 = %u\n\tchar _genArg3 = %s\n)",
			iClientID, _genArg1, _genArg2, _genArg3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST,
			iClientID, _genArg1, _genArg2, _genArg3);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_PLAYERLIST(iClientID, _genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST,
			iClientID, _genArg1, _genArg2, _genArg3);

	return retVal;
}

void HkIClientImpl::unknown_112(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_112(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_112(iClientID, iDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(uint iClientID) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(\n\tuint iClientID = %u\n)",
			iClientID);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2,
			iClientID);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_PLAYERLIST_2(iClientID));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2,
			iClientID);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_6(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_7(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE,
			iClientID, pDunno);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE(iClientID, pDunno));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_2(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(uint iClientID, uint iTargetID, uint iRank) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(\n\tuint iClientID = %u\n\tuint iTargetID = %u\n\tuint iRank = %u\n)",
			iClientID, iTargetID, iRank);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3,
			iClientID, iTargetID, iRank);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_3(iClientID, iTargetID, iRank));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3,
			iClientID, iTargetID, iRank);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_4(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5,
			iClientID, iDunno, iDunno2);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_5(iClientID, iDunno, iDunno2));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5,
			iClientID, iDunno, iDunno2);

	return retVal;
}

void HkIClientImpl::unknown_121(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_121(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_121(iClientID, iDunno));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(uint iClientID, uint iShipID, Vector& vFormationOffset) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(\n\tuint iClientID = %u\n\tuint iShipID = %u\n\tVector& vFormationOffset = %s\n)",
			iClientID, iShipID, vFormationOffset);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_FORMATION_UPDATE,
			iClientID, iShipID, vFormationOffset);

	if(!skip) retVal = CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_FORMATION_UPDATE(iClientID, iShipID, vFormationOffset));

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_FORMATION_UPDATE,
			iClientID, iShipID, vFormationOffset);

	return retVal;
}

void HkIClientImpl::unknown_123(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3, uint iDunno4, uint iDunno5, uint iDunno6) {
	AddDebugLog("HkIClientImpl::unknown_123(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n\tuint iDunno3 = %u\n\tuint iDunno4 = %u\n\tuint iDunno5 = %u\n\tuint iDunno6 = %u\n)",
			iClientID, iDunno, iDunno2, iDunno3, iDunno4, iDunno5, iDunno6);

	CALL_CLIENT_METHOD(unknown_123(iClientID, iDunno, iDunno2, iDunno3, iDunno4, iDunno5, iDunno6));
}

void HkIClientImpl::unknown_124(uint iClientID) {
	AddDebugLog("HkIClientImpl::unknown_124(\n\tuint iClientID = %u\n)",
			iClientID);

	CALL_CLIENT_METHOD(unknown_124(iClientID));
}

void HkIClientImpl::unknown_125(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_125(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_METHOD(unknown_125(iClientID, iDunno));
}

int HkIClientImpl::unknown_126(char* szUnknown) {
	AddDebugLog("HkIClientImpl::unknown_126(\n\tchar* szUnknown = %s\n)",
			szUnknown);

	auto retVal = CALL_CLIENT_METHOD(unknown_126(szUnknown));
	return retVal;
}

void __stdcall HkIServerImpl::AbortMission(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::AbortMission(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AbortMission,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.AbortMission(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__AbortMission,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::AcceptTrade(unsigned int _genArg1, bool _genArg2) {
	AddDebugLog("HkIServerImpl::AcceptTrade(\n\tunsigned int _genArg1 = %u\n\tbool _genArg2 = %d\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AcceptTrade,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.AcceptTrade(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__AcceptTrade,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ActivateCruise(unsigned int _genArg1, XActivateCruise const& _genArg2) {
	AddDebugLog("HkIServerImpl::ActivateCruise(\n\tunsigned int _genArg1 = %u\n\tXActivateCruise const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateCruise,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ActivateCruise(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ActivateCruise,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ActivateEquip(unsigned int _genArg1, XActivateEquip const& _genArg2) {
	AddDebugLog("HkIServerImpl::ActivateEquip(\n\tunsigned int _genArg1 = %u\n\tXActivateEquip const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateEquip,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ActivateEquip(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ActivateEquip,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ActivateThrusters(unsigned int _genArg1, XActivateThrusters const& _genArg2) {
	AddDebugLog("HkIServerImpl::ActivateThrusters(\n\tunsigned int _genArg1 = %u\n\tXActivateThrusters const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateThrusters,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ActivateThrusters(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ActivateThrusters,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::AddTradeEquip(unsigned int _genArg1, EquipDesc const& _genArg2) {
	AddDebugLog("HkIServerImpl::AddTradeEquip(\n\tunsigned int _genArg1 = %u\n\tEquipDesc const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AddTradeEquip,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.AddTradeEquip(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__AddTradeEquip,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::BaseEnter(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::BaseEnter(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseEnter,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.BaseEnter(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__BaseEnter,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::BaseExit(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::BaseExit(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseExit,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.BaseExit(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__BaseExit,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::BaseInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3) {
	AddDebugLog("HkIServerImpl::BaseInfoRequest(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tbool _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseInfoRequest,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.BaseInfoRequest(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__BaseInfoRequest,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::CharacterInfoReq(unsigned int _genArg1, bool _genArg2) {
	AddDebugLog("HkIServerImpl::CharacterInfoReq(\n\tunsigned int _genArg1 = %u\n\tbool _genArg2 = %d\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterInfoReq,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.CharacterInfoReq(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__CharacterInfoReq,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::CharacterSelect(CHARACTER_ID const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::CharacterSelect(\n\tCHARACTER_ID const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterSelect,
			_genArg1, _genArg2);

	CHECK_FOR_DISCONNECT;

	bool innerCheck = HkIServerImpl__CharacterSelect__Inner(_genArg1, _genArg2);
	if(!innerCheck) return;
	if(!skip) EXECUTE_SERVER_CALL(Server.CharacterSelect(_genArg1, _genArg2));
	HkIServerImpl__CharacterSelect__InnerAfter(_genArg1, _genArg2);


	CallPluginsAfter(HookedCall::IServerImpl__CharacterSelect,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::CharacterSkipAutosave(unsigned int _genArg1) {
	AddDebugLog("HkIServerImpl::CharacterSkipAutosave(\n\tunsigned int _genArg1 = %u\n)",
			_genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterSkipAutosave,
			_genArg1);

	if(!skip) EXECUTE_SERVER_CALL(Server.CharacterSkipAutosave(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__CharacterSkipAutosave,
			_genArg1);

}

void __stdcall HkIServerImpl::CommComplete(unsigned int _genArg1, unsigned int _genArg2, unsigned int _genArg3, enum CommResult _genArg4) {
	AddDebugLog("HkIServerImpl::CommComplete(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tunsigned int _genArg3 = %u\n\tenum CommResult _genArg4 = %s\n)",
			_genArg1, _genArg2, _genArg3, _genArg4);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CommComplete,
			_genArg1, _genArg2, _genArg3, _genArg4);

	if(!skip) EXECUTE_SERVER_CALL(Server.CommComplete(_genArg1, _genArg2, _genArg3, _genArg4));

	CallPluginsAfter(HookedCall::IServerImpl__CommComplete,
			_genArg1, _genArg2, _genArg3, _genArg4);

}

void __stdcall HkIServerImpl::Connect(char const* _genArg1, unsigned short* _genArg2) {
	AddDebugLog("HkIServerImpl::Connect(\n\tchar const* _genArg1 = %p\n\tunsigned short* _genArg2 = %p\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Connect,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.Connect(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__Connect,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::CreateNewCharacter(SCreateCharacterInfo const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::CreateNewCharacter(\n\tSCreateCharacterInfo const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CreateNewCharacter,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.CreateNewCharacter(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__CreateNewCharacter,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::DelTradeEquip(unsigned int _genArg1, EquipDesc const& _genArg2) {
	AddDebugLog("HkIServerImpl::DelTradeEquip(\n\tunsigned int _genArg1 = %u\n\tEquipDesc const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DelTradeEquip,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.DelTradeEquip(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__DelTradeEquip,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::DestroyCharacter(CHARACTER_ID const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::DestroyCharacter(\n\tCHARACTER_ID const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DestroyCharacter,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.DestroyCharacter(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__DestroyCharacter,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::DisConnect(unsigned int _genArg1, enum EFLConnection _genArg2) {
	AddDebugLog("HkIServerImpl::DisConnect(\n\tunsigned int _genArg1 = %u\n\tenum EFLConnection _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DisConnect,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.DisConnect(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__DisConnect,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::Dock(unsigned int const& _genArg1, unsigned int const& _genArg2) {
	AddDebugLog("HkIServerImpl::Dock(\n\tunsigned int const& _genArg1 = %s\n\tunsigned int const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Dock,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.Dock(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__Dock,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::DumpPacketStats(char const* _genArg1) {
	AddDebugLog("HkIServerImpl::DumpPacketStats(\n\tchar const* _genArg1 = %p\n)",
			_genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DumpPacketStats,
			_genArg1);

	if(!skip) EXECUTE_SERVER_CALL(Server.DumpPacketStats(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__DumpPacketStats,
			_genArg1);

}

void __stdcall HkIServerImpl::ElapseTime(float _genArg1) {
	AddDebugLog("HkIServerImpl::ElapseTime(\n\tfloat _genArg1 = %f\n)",
			_genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ElapseTime,
			_genArg1);

	if(!skip) EXECUTE_SERVER_CALL(Server.ElapseTime(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__ElapseTime,
			_genArg1);

}

void __stdcall HkIServerImpl::FireWeapon(unsigned int _genArg1, XFireWeaponInfo const& _genArg2) {
	AddDebugLog("HkIServerImpl::FireWeapon(\n\tunsigned int _genArg1 = %u\n\tXFireWeaponInfo const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__FireWeapon,
			_genArg1, _genArg2);

	CHECK_FOR_DISCONNECT;

	if(!skip) EXECUTE_SERVER_CALL(Server.FireWeapon(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__FireWeapon,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::GFGoodBuy(SGFGoodBuyInfo const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::GFGoodBuy(\n\tSGFGoodBuyInfo const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodBuy,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.GFGoodBuy(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__GFGoodBuy,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::GFGoodSell(SGFGoodSellInfo const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::GFGoodSell(\n\tSGFGoodSellInfo const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodSell,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.GFGoodSell(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__GFGoodSell,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::GFGoodVaporized(SGFGoodVaporizedInfo const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::GFGoodVaporized(\n\tSGFGoodVaporizedInfo const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodVaporized,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.GFGoodVaporized(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__GFGoodVaporized,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::GFObjSelect(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::GFObjSelect(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFObjSelect,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.GFObjSelect(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__GFObjSelect,
			_genArg1, _genArg2);

}

unsigned int __stdcall HkIServerImpl::GetServerID() {
	AddDebugLog("HkIServerImpl::GetServerID()");

	auto [retVal, skip] = CallPluginsBefore<unsigned int>(HookedCall::IServerImpl__GetServerID);

	if(!skip) retVal = EXECUTE_SERVER_CALL(Server.GetServerID());

	CallPluginsAfter(HookedCall::IServerImpl__GetServerID);

	return retVal;
}

char const* __stdcall HkIServerImpl::GetServerSig() {
	AddDebugLog("HkIServerImpl::GetServerSig()");

	auto [retVal, skip] = CallPluginsBefore<char const*>(HookedCall::IServerImpl__GetServerSig);

	if(!skip) retVal = EXECUTE_SERVER_CALL(Server.GetServerSig());

	CallPluginsAfter(HookedCall::IServerImpl__GetServerSig);

	return retVal;
}

void __stdcall HkIServerImpl::GetServerStats(ServerStats& _genArg1) {
	AddDebugLog("HkIServerImpl::GetServerStats(\n\tServerStats& _genArg1 = %s\n)",
			_genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GetServerStats,
			_genArg1);

	if(!skip) EXECUTE_SERVER_CALL(Server.GetServerStats(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__GetServerStats,
			_genArg1);

}

void __stdcall HkIServerImpl::GoTradelane(unsigned int _genArg1, XGoTradelane const& _genArg2) {
	AddDebugLog("HkIServerImpl::GoTradelane(\n\tunsigned int _genArg1 = %u\n\tXGoTradelane const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GoTradelane,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.GoTradelane(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__GoTradelane,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::Hail(unsigned int _genArg1, unsigned int _genArg2, unsigned int _genArg3) {
	AddDebugLog("HkIServerImpl::Hail(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tunsigned int _genArg3 = %u\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Hail,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.Hail(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__Hail,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::InitiateTrade(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::InitiateTrade(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InitiateTrade,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.InitiateTrade(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__InitiateTrade,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::InterfaceItemUsed(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::InterfaceItemUsed(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InterfaceItemUsed,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.InterfaceItemUsed(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__InterfaceItemUsed,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::JettisonCargo(unsigned int _genArg1, XJettisonCargo const& _genArg2) {
	AddDebugLog("HkIServerImpl::JettisonCargo(\n\tunsigned int _genArg1 = %u\n\tXJettisonCargo const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JettisonCargo,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.JettisonCargo(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__JettisonCargo,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::JumpInComplete(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::JumpInComplete(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JumpInComplete,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.JumpInComplete(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__JumpInComplete,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::LaunchComplete(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::LaunchComplete(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LaunchComplete,
			_genArg1, _genArg2);

	HkIServerImpl__LaunchComplete__Inner(_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.LaunchComplete(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__LaunchComplete,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::LocationEnter(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::LocationEnter(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationEnter,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.LocationEnter(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__LocationEnter,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::LocationExit(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::LocationExit(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationExit,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.LocationExit(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__LocationExit,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::LocationInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3) {
	AddDebugLog("HkIServerImpl::LocationInfoRequest(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tbool _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationInfoRequest,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.LocationInfoRequest(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__LocationInfoRequest,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::Login(SLoginInfo const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::Login(\n\tSLoginInfo const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Login,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.Login(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__Login,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::MineAsteroid(unsigned int _genArg1, Vector const& _genArg2, unsigned int _genArg3, unsigned int _genArg4, unsigned int _genArg5, unsigned int _genArg6) {
	AddDebugLog("HkIServerImpl::MineAsteroid(\n\tunsigned int _genArg1 = %u\n\tVector const& _genArg2 = %s\n\tunsigned int _genArg3 = %u\n\tunsigned int _genArg4 = %u\n\tunsigned int _genArg5 = %u\n\tunsigned int _genArg6 = %u\n)",
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__MineAsteroid,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

	if(!skip) EXECUTE_SERVER_CALL(Server.MineAsteroid(_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6));

	CallPluginsAfter(HookedCall::IServerImpl__MineAsteroid,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

}

void __stdcall HkIServerImpl::MissionResponse(unsigned int _genArg1, unsigned long _genArg2, bool _genArg3, unsigned int _genArg4) {
	AddDebugLog("HkIServerImpl::MissionResponse(\n\tunsigned int _genArg1 = %u\n\tunsigned long _genArg2 = %u\n\tbool _genArg3 = %d\n\tunsigned int _genArg4 = %u\n)",
			_genArg1, _genArg2, _genArg3, _genArg4);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__MissionResponse,
			_genArg1, _genArg2, _genArg3, _genArg4);

	if(!skip) EXECUTE_SERVER_CALL(Server.MissionResponse(_genArg1, _genArg2, _genArg3, _genArg4));

	CallPluginsAfter(HookedCall::IServerImpl__MissionResponse,
			_genArg1, _genArg2, _genArg3, _genArg4);

}

void __stdcall HkIServerImpl::MissionSaveB(unsigned int _genArg1, unsigned long _genArg2) {
	AddDebugLog("HkIServerImpl::MissionSaveB(\n\tunsigned int _genArg1 = %u\n\tunsigned long _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__MissionSaveB,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.MissionSaveB(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__MissionSaveB,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::NewCharacterInfoReq(unsigned int _genArg1) {
	AddDebugLog("HkIServerImpl::NewCharacterInfoReq(\n\tunsigned int _genArg1 = %u\n)",
			_genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__NewCharacterInfoReq,
			_genArg1);

	if(!skip) EXECUTE_SERVER_CALL(Server.NewCharacterInfoReq(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__NewCharacterInfoReq,
			_genArg1);

}

void __stdcall HkIServerImpl::OnConnect(unsigned int _genArg1) {
	AddDebugLog("HkIServerImpl::OnConnect(\n\tunsigned int _genArg1 = %u\n)",
			_genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__OnConnect,
			_genArg1);

	if(!skip) EXECUTE_SERVER_CALL(Server.OnConnect(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__OnConnect,
			_genArg1);

}

void __stdcall HkIServerImpl::PlayerLaunch(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::PlayerLaunch(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PlayerLaunch,
			_genArg1, _genArg2);

	CHECK_FOR_DISCONNECT;

	HkIServerImpl__PlayerLaunch__Inner(_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.PlayerLaunch(_genArg1, _genArg2));
	HkIServerImpl__PlayerLaunch__InnerAfter(_genArg1, _genArg2);


	CallPluginsAfter(HookedCall::IServerImpl__PlayerLaunch,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::PopUpDialog(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::PopUpDialog(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PopUpDialog,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.PopUpDialog(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__PopUpDialog,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::PushToServer(CDAPacket* _genArg1) {
	AddDebugLog("HkIServerImpl::PushToServer(\n\tCDAPacket* _genArg1 = %p\n)",
			_genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PushToServer,
			_genArg1);

	if(!skip) EXECUTE_SERVER_CALL(Server.PushToServer(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__PushToServer,
			_genArg1);

}

void __stdcall HkIServerImpl::RTCDone(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::RTCDone(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RTCDone,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.RTCDone(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__RTCDone,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ReqAddItem(unsigned int _genArg1, char const* _genArg2, int _genArg3, float _genArg4, bool _genArg5, unsigned int _genArg6) {
	AddDebugLog("HkIServerImpl::ReqAddItem(\n\tunsigned int _genArg1 = %u\n\tchar const* _genArg2 = %p\n\tint _genArg3 = %d\n\tfloat _genArg4 = %f\n\tbool _genArg5 = %d\n\tunsigned int _genArg6 = %u\n)",
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqAddItem,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqAddItem(_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6));

	CallPluginsAfter(HookedCall::IServerImpl__ReqAddItem,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

}

void __stdcall HkIServerImpl::ReqCargo(EquipDescList const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::ReqCargo(\n\tEquipDescList const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqCargo,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqCargo(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ReqCargo,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ReqChangeCash(int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::ReqChangeCash(\n\tint _genArg1 = %d\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqChangeCash,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqChangeCash(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ReqChangeCash,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ReqCollisionGroups(st6::list<struct CollisionGroupDesc, class st6::allocator<struct CollisionGroupDesc>> const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::ReqCollisionGroups(\n\tst6::list<struct CollisionGroupDesc, class st6::allocator<struct CollisionGroupDesc>> const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqCollisionGroups,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqCollisionGroups(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ReqCollisionGroups,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ReqDifficultyScale(float _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::ReqDifficultyScale(\n\tfloat _genArg1 = %f\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqDifficultyScale,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqDifficultyScale(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ReqDifficultyScale,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ReqEquipment(EquipDescList const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::ReqEquipment(\n\tEquipDescList const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqEquipment,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqEquipment(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ReqEquipment,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ReqHullStatus(float _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::ReqHullStatus(\n\tfloat _genArg1 = %f\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqHullStatus,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqHullStatus(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ReqHullStatus,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ReqModifyItem(unsigned short _genArg1, char const* _genArg2, int _genArg3, float _genArg4, bool _genArg5, unsigned int _genArg6) {
	AddDebugLog("HkIServerImpl::ReqModifyItem(\n\tunsigned short _genArg1 = %u\n\tchar const* _genArg2 = %p\n\tint _genArg3 = %d\n\tfloat _genArg4 = %f\n\tbool _genArg5 = %d\n\tunsigned int _genArg6 = %u\n)",
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqModifyItem,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqModifyItem(_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6));

	CallPluginsAfter(HookedCall::IServerImpl__ReqModifyItem,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

}

void __stdcall HkIServerImpl::ReqRemoveItem(unsigned short _genArg1, int _genArg2, unsigned int _genArg3) {
	AddDebugLog("HkIServerImpl::ReqRemoveItem(\n\tunsigned short _genArg1 = %u\n\tint _genArg2 = %d\n\tunsigned int _genArg3 = %u\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqRemoveItem,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqRemoveItem(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__ReqRemoveItem,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::ReqSetCash(int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::ReqSetCash(\n\tint _genArg1 = %d\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqSetCash,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqSetCash(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ReqSetCash,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::ReqShipArch(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::ReqShipArch(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqShipArch,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.ReqShipArch(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__ReqShipArch,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::RequestBestPath(unsigned int _genArg1, unsigned char* _genArg2, int _genArg3) {
	AddDebugLog("HkIServerImpl::RequestBestPath(\n\tunsigned int _genArg1 = %u\n\tunsigned char* _genArg2 = %p\n\tint _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestBestPath,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.RequestBestPath(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__RequestBestPath,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::RequestCancel(int _genArg1, unsigned int _genArg2, unsigned int _genArg3, unsigned long _genArg4, unsigned int _genArg5) {
	AddDebugLog("HkIServerImpl::RequestCancel(\n\tint _genArg1 = %d\n\tunsigned int _genArg2 = %u\n\tunsigned int _genArg3 = %u\n\tunsigned long _genArg4 = %u\n\tunsigned int _genArg5 = %u\n)",
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCancel,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5);

	if(!skip) EXECUTE_SERVER_CALL(Server.RequestCancel(_genArg1, _genArg2, _genArg3, _genArg4, _genArg5));

	CallPluginsAfter(HookedCall::IServerImpl__RequestCancel,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5);

}

void __stdcall HkIServerImpl::RequestCreateShip(unsigned int _genArg1) {
	AddDebugLog("HkIServerImpl::RequestCreateShip(\n\tunsigned int _genArg1 = %u\n)",
			_genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCreateShip,
			_genArg1);

	if(!skip) EXECUTE_SERVER_CALL(Server.RequestCreateShip(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__RequestCreateShip,
			_genArg1);

}

void __stdcall HkIServerImpl::RequestEvent(int _genArg1, unsigned int _genArg2, unsigned int _genArg3, unsigned int _genArg4, unsigned long _genArg5, unsigned int _genArg6) {
	AddDebugLog("HkIServerImpl::RequestEvent(\n\tint _genArg1 = %d\n\tunsigned int _genArg2 = %u\n\tunsigned int _genArg3 = %u\n\tunsigned int _genArg4 = %u\n\tunsigned long _genArg5 = %u\n\tunsigned int _genArg6 = %u\n)",
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestEvent,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

	if(!skip) EXECUTE_SERVER_CALL(Server.RequestEvent(_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6));

	CallPluginsAfter(HookedCall::IServerImpl__RequestEvent,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

}

void __stdcall HkIServerImpl::RequestGroupPositions(unsigned int _genArg1, unsigned char* _genArg2, int _genArg3) {
	AddDebugLog("HkIServerImpl::RequestGroupPositions(\n\tunsigned int _genArg1 = %u\n\tunsigned char* _genArg2 = %p\n\tint _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestGroupPositions,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.RequestGroupPositions(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__RequestGroupPositions,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::RequestPlayerStats(unsigned int _genArg1, unsigned char* _genArg2, int _genArg3) {
	AddDebugLog("HkIServerImpl::RequestPlayerStats(\n\tunsigned int _genArg1 = %u\n\tunsigned char* _genArg2 = %p\n\tint _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestPlayerStats,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.RequestPlayerStats(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__RequestPlayerStats,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::RequestRankLevel(unsigned int _genArg1, unsigned char* _genArg2, int _genArg3) {
	AddDebugLog("HkIServerImpl::RequestRankLevel(\n\tunsigned int _genArg1 = %u\n\tunsigned char* _genArg2 = %p\n\tint _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestRankLevel,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.RequestRankLevel(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__RequestRankLevel,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::RequestTrade(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::RequestTrade(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestTrade,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.RequestTrade(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__RequestTrade,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::SPBadLandsObjCollision(SSPBadLandsObjCollisionInfo const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::SPBadLandsObjCollision(\n\tSSPBadLandsObjCollisionInfo const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPBadLandsObjCollision,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.SPBadLandsObjCollision(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__SPBadLandsObjCollision,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::SPMunitionCollision(SSPMunitionCollisionInfo const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::SPMunitionCollision(\n\tSSPMunitionCollisionInfo const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPMunitionCollision,
			_genArg1, _genArg2);

	CHECK_FOR_DISCONNECT;

	HkIServerImpl__SPMunitionCollision__Inner(_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.SPMunitionCollision(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__SPMunitionCollision,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::SPObjCollision(SSPObjCollisionInfo const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::SPObjCollision(\n\tSSPObjCollisionInfo const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjCollision,
			_genArg1, _genArg2);

	CHECK_FOR_DISCONNECT;

	if(!skip) EXECUTE_SERVER_CALL(Server.SPObjCollision(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__SPObjCollision,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::SPObjUpdate(SSPObjUpdateInfo const& _genArg1, unsigned int _genArg2) {
	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjUpdate,
			_genArg1, _genArg2);

	CHECK_FOR_DISCONNECT;

	bool innerCheck = HkIServerImpl__SPObjUpdate__Inner(_genArg1, _genArg2);
	if(!innerCheck) return;
	if(!skip) EXECUTE_SERVER_CALL(Server.SPObjUpdate(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__SPObjUpdate,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::SPRequestInvincibility(unsigned int _genArg1, bool _genArg2, enum InvincibilityReason _genArg3, unsigned int _genArg4) {
	AddDebugLog("HkIServerImpl::SPRequestInvincibility(\n\tunsigned int _genArg1 = %u\n\tbool _genArg2 = %d\n\tenum InvincibilityReason _genArg3 = %s\n\tunsigned int _genArg4 = %u\n)",
			_genArg1, _genArg2, _genArg3, _genArg4);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestInvincibility,
			_genArg1, _genArg2, _genArg3, _genArg4);

	if(!skip) EXECUTE_SERVER_CALL(Server.SPRequestInvincibility(_genArg1, _genArg2, _genArg3, _genArg4));

	CallPluginsAfter(HookedCall::IServerImpl__SPRequestInvincibility,
			_genArg1, _genArg2, _genArg3, _genArg4);

}

void __stdcall HkIServerImpl::SPRequestUseItem(SSPUseItem const& _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::SPRequestUseItem(\n\tSSPUseItem const& _genArg1 = %s\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestUseItem,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.SPRequestUseItem(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__SPRequestUseItem,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::SPScanCargo(unsigned int const& _genArg1, unsigned int const& _genArg2, unsigned int _genArg3) {
	AddDebugLog("HkIServerImpl::SPScanCargo(\n\tunsigned int const& _genArg1 = %s\n\tunsigned int const& _genArg2 = %s\n\tunsigned int _genArg3 = %u\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPScanCargo,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.SPScanCargo(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__SPScanCargo,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::SaveGame(CHARACTER_ID const& _genArg1, unsigned short const* _genArg2, unsigned int _genArg3) {
	AddDebugLog("HkIServerImpl::SaveGame(\n\tCHARACTER_ID const& _genArg1 = %s\n\tunsigned short const* _genArg2 = %p\n\tunsigned int _genArg3 = %u\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SaveGame,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.SaveGame(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__SaveGame,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::SetActiveConnection(enum EFLConnection _genArg1) {
	AddDebugLog("HkIServerImpl::SetActiveConnection(\n\tenum EFLConnection _genArg1 = %s\n)",
			_genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetActiveConnection,
			_genArg1);

	if(!skip) EXECUTE_SERVER_CALL(Server.SetActiveConnection(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__SetActiveConnection,
			_genArg1);

}

void __stdcall HkIServerImpl::SetInterfaceState(unsigned int _genArg1, unsigned char* _genArg2, int _genArg3) {
	AddDebugLog("HkIServerImpl::SetInterfaceState(\n\tunsigned int _genArg1 = %u\n\tunsigned char* _genArg2 = %p\n\tint _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetInterfaceState,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.SetInterfaceState(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__SetInterfaceState,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::SetManeuver(unsigned int _genArg1, XSetManeuver const& _genArg2) {
	AddDebugLog("HkIServerImpl::SetManeuver(\n\tunsigned int _genArg1 = %u\n\tXSetManeuver const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetManeuver,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.SetManeuver(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__SetManeuver,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::SetMissionLog(unsigned int _genArg1, unsigned char* _genArg2, int _genArg3) {
	AddDebugLog("HkIServerImpl::SetMissionLog(\n\tunsigned int _genArg1 = %u\n\tunsigned char* _genArg2 = %p\n\tint _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetMissionLog,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.SetMissionLog(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__SetMissionLog,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::SetTarget(unsigned int _genArg1, XSetTarget const& _genArg2) {
	AddDebugLog("HkIServerImpl::SetTarget(\n\tunsigned int _genArg1 = %u\n\tXSetTarget const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTarget,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.SetTarget(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__SetTarget,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::SetTradeMoney(unsigned int _genArg1, unsigned long _genArg2) {
	AddDebugLog("HkIServerImpl::SetTradeMoney(\n\tunsigned int _genArg1 = %u\n\tunsigned long _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTradeMoney,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.SetTradeMoney(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__SetTradeMoney,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::SetVisitedState(unsigned int _genArg1, unsigned char* _genArg2, int _genArg3) {
	AddDebugLog("HkIServerImpl::SetVisitedState(\n\tunsigned int _genArg1 = %u\n\tunsigned char* _genArg2 = %p\n\tint _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetVisitedState,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.SetVisitedState(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__SetVisitedState,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::SetWeaponGroup(unsigned int _genArg1, unsigned char* _genArg2, int _genArg3) {
	AddDebugLog("HkIServerImpl::SetWeaponGroup(\n\tunsigned int _genArg1 = %u\n\tunsigned char* _genArg2 = %p\n\tint _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetWeaponGroup,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.SetWeaponGroup(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__SetWeaponGroup,
			_genArg1, _genArg2, _genArg3);

}

void __stdcall HkIServerImpl::Shutdown() {
	AddDebugLog("HkIServerImpl::Shutdown()");

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Shutdown);

	if(!skip) EXECUTE_SERVER_CALL(Server.Shutdown());

	CallPluginsAfter(HookedCall::IServerImpl__Shutdown);

}

bool __stdcall HkIServerImpl::Startup(SStartupInfo const& _genArg1) {
	AddDebugLog("HkIServerImpl::Startup(\n\tSStartupInfo const& _genArg1 = %s\n)",
			_genArg1);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IServerImpl__Startup,
			_genArg1);

	if(!skip) retVal = EXECUTE_SERVER_CALL(Server.Startup(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__Startup,
			_genArg1);

	return retVal;
}

void __stdcall HkIServerImpl::StopTradeRequest(unsigned int _genArg1) {
	AddDebugLog("HkIServerImpl::StopTradeRequest(\n\tunsigned int _genArg1 = %u\n)",
			_genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradeRequest,
			_genArg1);

	if(!skip) EXECUTE_SERVER_CALL(Server.StopTradeRequest(_genArg1));

	CallPluginsAfter(HookedCall::IServerImpl__StopTradeRequest,
			_genArg1);

}

void __stdcall HkIServerImpl::StopTradelane(unsigned int _genArg1, unsigned int _genArg2, unsigned int _genArg3, unsigned int _genArg4) {
	AddDebugLog("HkIServerImpl::StopTradelane(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tunsigned int _genArg3 = %u\n\tunsigned int _genArg4 = %u\n)",
			_genArg1, _genArg2, _genArg3, _genArg4);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradelane,
			_genArg1, _genArg2, _genArg3, _genArg4);

	if(!skip) EXECUTE_SERVER_CALL(Server.StopTradelane(_genArg1, _genArg2, _genArg3, _genArg4));

	CallPluginsAfter(HookedCall::IServerImpl__StopTradelane,
			_genArg1, _genArg2, _genArg3, _genArg4);

}

void __stdcall HkIServerImpl::SubmitChat(CHAT_ID _genArg1, unsigned long _genArg2, void const* _genArg3, CHAT_ID _genArg4, int _genArg5) {
	AddDebugLog("HkIServerImpl::SubmitChat(\n\tCHAT_ID _genArg1 = %s\n\tunsigned long _genArg2 = %u\n\tvoid const* _genArg3 = %p\n\tCHAT_ID _genArg4 = %s\n\tint _genArg5 = %d\n)",
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SubmitChat,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5);

	HkIServerImpl__SubmitChat__Inner(_genArg1, _genArg2, _genArg3, _genArg4, _genArg5);

	g_bInSubmitChat = true;
	if(!skip) EXECUTE_SERVER_CALL(Server.SubmitChat(_genArg1, _genArg2, _genArg3, _genArg4, _genArg5));
	g_bInSubmitChat = false;

	CallPluginsAfter(HookedCall::IServerImpl__SubmitChat,
			_genArg1, _genArg2, _genArg3, _genArg4, _genArg5);

}

bool __stdcall HkIServerImpl::SwapConnections(enum EFLConnection _genArg1, enum EFLConnection _genArg2) {
	AddDebugLog("HkIServerImpl::SwapConnections(\n\tenum EFLConnection _genArg1 = %s\n\tenum EFLConnection _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IServerImpl__SwapConnections,
			_genArg1, _genArg2);

	if(!skip) retVal = EXECUTE_SERVER_CALL(Server.SwapConnections(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__SwapConnections,
			_genArg1, _genArg2);

	return retVal;
}

void __stdcall HkIServerImpl::SystemSwitchOutComplete(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("HkIServerImpl::SystemSwitchOutComplete(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SystemSwitchOutComplete,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.SystemSwitchOutComplete(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__SystemSwitchOutComplete,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::TerminateTrade(unsigned int _genArg1, int _genArg2) {
	AddDebugLog("HkIServerImpl::TerminateTrade(\n\tunsigned int _genArg1 = %u\n\tint _genArg2 = %d\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TerminateTrade,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.TerminateTrade(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__TerminateTrade,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::TractorObjects(unsigned int _genArg1, XTractorObjects const& _genArg2) {
	AddDebugLog("HkIServerImpl::TractorObjects(\n\tunsigned int _genArg1 = %u\n\tXTractorObjects const& _genArg2 = %s\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TractorObjects,
			_genArg1, _genArg2);

	if(!skip) EXECUTE_SERVER_CALL(Server.TractorObjects(_genArg1, _genArg2));

	CallPluginsAfter(HookedCall::IServerImpl__TractorObjects,
			_genArg1, _genArg2);

}

void __stdcall HkIServerImpl::TradeResponse(unsigned char const* _genArg1, int _genArg2, unsigned int _genArg3) {
	AddDebugLog("HkIServerImpl::TradeResponse(\n\tunsigned char const* _genArg1 = %p\n\tint _genArg2 = %d\n\tunsigned int _genArg3 = %u\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TradeResponse,
			_genArg1, _genArg2, _genArg3);

	if(!skip) EXECUTE_SERVER_CALL(Server.TradeResponse(_genArg1, _genArg2, _genArg3));

	CallPluginsAfter(HookedCall::IServerImpl__TradeResponse,
			_genArg1, _genArg2, _genArg3);

}

int __stdcall HkIServerImpl::Update() {
	HkIServerImpl__Update__Inner();

	auto retVal = EXECUTE_SERVER_CALL(Server.Update());
	return retVal;
}

