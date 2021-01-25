//
// WARNING: THIS IS AN AUTO-GENERATED FILE, DO NOT EDIT!
//

#include <Hook.h>

bool HkIClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(uint iClientID, XFireWeaponInfo& fwi) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(\n\tuint iClientID = %u\n\tXFireWeaponInfo& fwi = %s\n)",
			iClientID, fwi);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON,
			iClientID, fwi);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_FIREWEAPON(iClientID, fwi);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON,
			iClientID, fwi);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(uint iClientID, XActivateEquip& aq) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(\n\tuint iClientID = %u\n\tXActivateEquip& aq = %s\n)",
			iClientID, aq);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP,
			iClientID, aq);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_ACTIVATEEQUIP(iClientID, aq);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP,
			iClientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(uint iClientID, XActivateCruise& aq) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(\n\tuint iClientID = %u\n\tXActivateCruise& aq = %s\n)",
			iClientID, aq);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE,
			iClientID, aq);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_ACTIVATECRUISE(iClientID, aq);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE,
			iClientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(uint iClientID, XActivateThrusters& aq) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(\n\tuint iClientID = %u\n\tXActivateThrusters& aq = %s\n)",
			iClientID, aq);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS,
			iClientID, aq);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(iClientID, aq);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS,
			iClientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SETTARGET(uint iClientID, XSetTarget& st) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_SETTARGET(\n\tuint iClientID = %u\n\tXSetTarget& st = %s\n)",
			iClientID, st);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SETTARGET,
			iClientID, st);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_SETTARGET(iClientID, st);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SETTARGET,
			iClientID, st);

	return retVal;
}

void HkIClientImpl::unknown_6(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_6(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_6(iClientID, pDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(uint iClientID, XGoTradelane& tl) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(\n\tuint iClientID = %u\n\tXGoTradelane& tl = %s\n)",
			iClientID, tl);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_GOTRADELANE,
			iClientID, tl);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_GOTRADELANE(iClientID, tl);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_GOTRADELANE,
			iClientID, tl);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(uint iClientID, uint iShip, uint iArchTradelane1, uint iArchTradelane2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(\n\tuint iClientID = %u\n\tuint iShip = %u\n\tuint iArchTradelane1 = %u\n\tuint iArchTradelane2 = %u\n)",
			iClientID, iShip, iArchTradelane1, iArchTradelane2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_STOPTRADELANE,
			iClientID, iShip, iArchTradelane1, iArchTradelane2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_STOPTRADELANE(iClientID, iShip, iArchTradelane1, iArchTradelane2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_STOPTRADELANE,
			iClientID, iShip, iArchTradelane1, iArchTradelane2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(uint iClientID, XJettisonCargo& jc) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(\n\tuint iClientID = %u\n\tXJettisonCargo& jc = %s\n)",
			iClientID, jc);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_JETTISONCARGO,
			iClientID, jc);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_JETTISONCARGO(iClientID, jc);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_JETTISONCARGO,
			iClientID, jc);

	return retVal;
}

bool HkIClientImpl::SendPacket(uint iClientID, void* _genArg1) {
	AddDebugLog("HkIClientImpl::SendPacket(\n\tuint iClientID = %u\n\tvoid* _genArg1 = %p\n)",
			iClientID, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE {
		retVal = SendPacket(iClientID, _genArg1);
	} CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Startup(uint _genArg1, uint _genArg2) {
	HkIClientImpl__Startup__Inner(_genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE {
		retVal = Startup(_genArg1, _genArg2);
	} CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::nullsub(uint _genArg1) {
	AddDebugLog("HkIClientImpl::nullsub(\n\tuint _genArg1 = %u\n)",
			_genArg1);

	CALL_CLIENT_PREAMBLE {
		nullsub(_genArg1);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LOGINRESPONSE,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_LOGINRESPONSE(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LOGINRESPONSE,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CHARACTERINFO,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_CHARACTERINFO(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CHARACTERINFO,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CHARSELECTVERIFIED,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_CHARSELECTVERIFIED(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CHARSELECTVERIFIED,
			iClientID, pDunno);

	return retVal;
}

void HkIClientImpl::Shutdown() {
	CALL_CLIENT_PREAMBLE {
		Shutdown();
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::CDPClientProxy__Disconnect(uint iClientID) {
	AddDebugLog("HkIClientImpl::CDPClientProxy__Disconnect(\n\tuint iClientID = %u\n)",
			iClientID);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__CDPClientProxy__Disconnect,
			iClientID);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = CDPClientProxy__Disconnect(iClientID);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__Disconnect,
			iClientID);

	return retVal;
}

uint HkIClientImpl::CDPClientProxy__GetSendQSize(uint iClientID) {
	AddDebugLog("HkIClientImpl::CDPClientProxy__GetSendQSize(\n\tuint iClientID = %u\n)",
			iClientID);

	auto [retVal, skip] = CallPluginsBefore<uint>(HookedCall::IClientImpl__CDPClientProxy__GetSendQSize,
			iClientID);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = CDPClientProxy__GetSendQSize(iClientID);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__GetSendQSize,
			iClientID);

	return retVal;
}

uint HkIClientImpl::CDPClientProxy__GetSendQBytes(uint iClientID) {
	AddDebugLog("HkIClientImpl::CDPClientProxy__GetSendQBytes(\n\tuint iClientID = %u\n)",
			iClientID);

	auto [retVal, skip] = CallPluginsBefore<uint>(HookedCall::IClientImpl__CDPClientProxy__GetSendQBytes,
			iClientID);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = CDPClientProxy__GetSendQBytes(iClientID);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__GetSendQBytes,
			iClientID);

	return retVal;
}

double HkIClientImpl::CDPClientProxy__GetLinkSaturation(uint iClientID) {
	AddDebugLog("HkIClientImpl::CDPClientProxy__GetLinkSaturation(\n\tuint iClientID = %u\n)",
			iClientID);

	auto [retVal, skip] = CallPluginsBefore<double>(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation,
			iClientID);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = CDPClientProxy__GetLinkSaturation(iClientID);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation,
			iClientID);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(uint iClientID, uint iShipArch) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(\n\tuint iClientID = %u\n\tuint iShipArch = %u\n)",
			iClientID, iShipArch);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH,
			iClientID, iShipArch);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SETSHIPARCH(iClientID, iShipArch);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH,
			iClientID, iShipArch);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(uint iClientID, float fStatus) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(\n\tuint iClientID = %u\n\tfloat fStatus = %f\n)",
			iClientID, fStatus);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS,
			iClientID, fStatus);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SETHULLSTATUS(iClientID, fStatus);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS,
			iClientID, fStatus);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SETEQUIPMENT(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT,
			iClientID, pDunno);

	return retVal;
}

void HkIClientImpl::unknown_26(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_26(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_26(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETADDITEM(uint iClientID, FLPACKET_UNKNOWN& pDunno, FLPACKET_UNKNOWN& pDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETADDITEM(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n\tFLPACKET_UNKNOWN& pDunno2 = %s\n)",
			iClientID, pDunno, pDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM,
			iClientID, pDunno, pDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SETADDITEM(iClientID, pDunno, pDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM,
			iClientID, pDunno, pDunno2);

	return retVal;
}

void HkIClientImpl::unknown_28(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3) {
	AddDebugLog("HkIClientImpl::unknown_28(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n\tuint iDunno3 = %u\n)",
			iClientID, iDunno, iDunno2, iDunno3);

	CALL_CLIENT_PREAMBLE {
		unknown_28(iClientID, iDunno, iDunno2, iDunno3);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SETSTARTROOM(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYCHARACTER,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFDESTROYCHARACTER(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYCHARACTER,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATECHAR,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFUPDATECHAR(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATECHAR,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETECHARLIST,
			iClientID, iDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(iClientID, iDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETECHARLIST,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST,
			iClientID, iDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(iClientID, iDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST,
			iClientID, iDunno);

	return retVal;
}

void HkIClientImpl::unknown_36(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_36(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_PREAMBLE {
		unknown_36(iClientID, iDunno, iDunno2);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_37(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_37(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_PREAMBLE {
		unknown_37(iClientID, iDunno, iDunno2);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST,
			iClientID, iDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(iClientID, iDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST,
			iClientID, iDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(iClientID, iDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(uint iClientID, uint iReason) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(\n\tuint iClientID = %u\n\tuint iReason = %u\n)",
			iClientID, iReason);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY,
			iClientID, iReason);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(iClientID, iReason);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY,
			iClientID, iReason);

	return retVal;
}

void HkIClientImpl::unknown_44(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_44(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_PREAMBLE {
		unknown_44(iClientID, iDunno, iDunno2);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST,
			iClientID, iDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(iClientID, iDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(uint iClientID, FLPACKET_CREATESOLAR& pSolar) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(\n\tuint iClientID = %u\n\tFLPACKET_CREATESOLAR& pSolar = %s\n)",
			iClientID, pSolar);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR,
			iClientID, pSolar);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_CREATESOLAR(iClientID, pSolar);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR,
			iClientID, pSolar);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATESHIP(uint iClientID, FLPACKET_CREATESHIP& pShip) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATESHIP(\n\tuint iClientID = %u\n\tFLPACKET_CREATESHIP& pShip = %s\n)",
			iClientID, pShip);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP,
			iClientID, pShip);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_CREATESHIP(iClientID, pShip);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP,
			iClientID, pShip);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATELOOT(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATELOOT(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_CREATELOOT(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATEMINE(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATEMINE(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_CREATEMINE(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_CREATEGUIDED(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_CREATECOUNTER(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER,
			iClientID, pDunno);

	return retVal;
}

void HkIClientImpl::unknown_53(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_53(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_53(iClientID, pDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_54(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3) {
	AddDebugLog("HkIClientImpl::unknown_54(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n\tuint iDunno3 = %u\n)",
			iClientID, iDunno, iDunno2, iDunno3);

	CALL_CLIENT_PREAMBLE {
		unknown_54(iClientID, iDunno, iDunno2, iDunno3);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_UPDATEOBJECT(uint iClientID, SSPObjUpdateInfo& pUpdate) {
	bool retVal;
	CALL_CLIENT_PREAMBLE {
		retVal = Send_FLPACKET_COMMON_UPDATEOBJECT(iClientID, pUpdate);
	} CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(uint iClientID, FLPACKET_DESTROYOBJECT& pDestroy) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(\n\tuint iClientID = %u\n\tFLPACKET_DESTROYOBJECT& pDestroy = %s\n)",
			iClientID, pDestroy);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT,
			iClientID, pDestroy);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_DESTROYOBJECT(iClientID, pDestroy);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT,
			iClientID, pDestroy);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(uint iClientID, XActivateEquip& aq) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(\n\tuint iClientID = %u\n\tXActivateEquip& aq = %s\n)",
			iClientID, aq);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT,
			iClientID, aq);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_ACTIVATEOBJECT(iClientID, aq);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT,
			iClientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LAND(uint iClientID, FLPACKET_LAND& pLand) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_LAND(\n\tuint iClientID = %u\n\tFLPACKET_LAND& pLand = %s\n)",
			iClientID, pLand);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAND,
			iClientID, pLand);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_LAND(iClientID, pLand);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAND,
			iClientID, pLand);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LAUNCH(uint iClientID, FLPACKET_LAUNCH& pLaunch) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_LAUNCH(\n\tuint iClientID = %u\n\tFLPACKET_LAUNCH& pLaunch = %s\n)",
			iClientID, pLaunch);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH,
			iClientID, pLaunch);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_LAUNCH(iClientID, pLaunch);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH,
			iClientID, pLaunch);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(uint iClientID, bool bResponse, uint iShipID) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(\n\tuint iClientID = %u\n\tbool bResponse = %d\n\tuint iShipID = %u\n)",
			iClientID, bResponse, iShipID);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP,
			iClientID, bResponse, iShipID);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(iClientID, bResponse, iShipID);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP,
			iClientID, bResponse, iShipID);

	return retVal;
}

void HkIClientImpl::unknown_63(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_63(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_63(iClientID, pDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(uint iClientID, uint iObj, int& dmlist) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(\n\tuint iClientID = %u\n\tuint iObj = %u\n\tint& dmlist = %s\n)",
			iClientID, iObj, dmlist);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DAMAGEOBJECT,
			iClientID, iObj, dmlist);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_DAMAGEOBJECT(iClientID, iObj, dmlist);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DAMAGEOBJECT,
			iClientID, iObj, dmlist);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ITEMTRACTORED,
			iClientID, iDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_ITEMTRACTORED(iClientID, iDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ITEMTRACTORED,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_USE_ITEM(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_USE_ITEM(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM,
			iClientID, iDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_USE_ITEM(iClientID, iDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM,
			iClientID, iDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(uint iClientID, FLPACKET_SETREPUTATION& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(\n\tuint iClientID = %u\n\tFLPACKET_SETREPUTATION& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SETREPUTATION(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION,
			iClientID, pDunno);

	return retVal;
}

void HkIClientImpl::unknown_68(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_68(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_68(iClientID, pDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SENDCOMM(uint iClientID, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6, uint _genArg7, uint _genArg8, uint _genArg9, uint _genArg10, uint _genArg11, uint _genArg12, uint _genArg13, uint _genArg14, uint _genArg15, uint _genArg16, uint _genArg17, uint _genArg18, uint _genArg19, uint _genArg20, uint _genArg21, uint _genArg22) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SENDCOMM(\n\tuint iClientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint _genArg3 = %u\n\tuint _genArg4 = %u\n\tuint _genArg5 = %u\n\tuint _genArg6 = %u\n\tuint _genArg7 = %u\n\tuint _genArg8 = %u\n\tuint _genArg9 = %u\n\tuint _genArg10 = %u\n\tuint _genArg11 = %u\n\tuint _genArg12 = %u\n\tuint _genArg13 = %u\n\tuint _genArg14 = %u\n\tuint _genArg15 = %u\n\tuint _genArg16 = %u\n\tuint _genArg17 = %u\n\tuint _genArg18 = %u\n\tuint _genArg19 = %u\n\tuint _genArg20 = %u\n\tuint _genArg21 = %u\n\tuint _genArg22 = %u\n)",
			iClientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM,
			iClientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SENDCOMM(iClientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM,
			iClientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);

	return retVal;
}

void HkIClientImpl::unknown_70(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_70(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_70(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE,
			iClientID, pDunno);

	return retVal;
}

void HkIClientImpl::unknown_72(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_72(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_72(iClientID, pDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES,
			iClientID, iDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(iClientID, iDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES,
			iClientID, iDunno);

	return retVal;
}

void HkIClientImpl::unknown_74(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_74(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_74(iClientID, pDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_75(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_75(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_75(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MARKOBJ(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MARKOBJ(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MARKOBJ,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_MARKOBJ(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MARKOBJ,
			iClientID, iDunno, iDunno2);

	return retVal;
}

void HkIClientImpl::unknown_77(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_77(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_77(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETCASH(uint iClientID, uint iCash) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SETCASH(\n\tuint iClientID = %u\n\tuint iCash = %u\n)",
			iClientID, iCash);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH,
			iClientID, iCash);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SETCASH(iClientID, iCash);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH,
			iClientID, iCash);

	return retVal;
}

void HkIClientImpl::unknown_79(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_79(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_79(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_80(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_80(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_80(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_81(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_81(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_81(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_82(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_82(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_82(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_83(uint iClientID, char* szDunno) {
	AddDebugLog("HkIClientImpl::unknown_83(\n\tuint iClientID = %u\n\tchar* szDunno = %s\n)",
			iClientID, szDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_83(iClientID, szDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(uint iClientID, uint iShipID, uint iFlag, uint iDunno3, uint iDunno4) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(\n\tuint iClientID = %u\n\tuint iShipID = %u\n\tuint iFlag = %u\n\tuint iDunno3 = %u\n\tuint iDunno4 = %u\n)",
			iClientID, iShipID, iFlag, iDunno3, iDunno4);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUEST_RETURNED,
			iClientID, iShipID, iFlag, iDunno3, iDunno4);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_REQUEST_RETURNED(iClientID, iShipID, iFlag, iDunno3, iDunno4);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUEST_RETURNED,
			iClientID, iShipID, iFlag, iDunno3, iDunno4);

	return retVal;
}

void HkIClientImpl::unknown_85(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_85(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_85(iClientID, pDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_86(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3) {
	AddDebugLog("HkIClientImpl::unknown_86(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n\tuint iDunno3 = %u\n)",
			iClientID, iDunno, iDunno2, iDunno3);

	CALL_CLIENT_PREAMBLE {
		unknown_86(iClientID, iDunno, iDunno2, iDunno3);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_OBJECTCARGOUPDATE,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_OBJECTCARGOUPDATE,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_BURNFUSE(uint iClientID, FLPACKET_BURNFUSE& pBurnfuse) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_BURNFUSE(\n\tuint iClientID = %u\n\tFLPACKET_BURNFUSE& pBurnfuse = %s\n)",
			iClientID, pBurnfuse);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE,
			iClientID, pBurnfuse);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_BURNFUSE(iClientID, pBurnfuse);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE,
			iClientID, pBurnfuse);

	return retVal;
}

void HkIClientImpl::unknown_89(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_89(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_89(iClientID, pDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_90(uint iClientID) {
	AddDebugLog("HkIClientImpl::unknown_90(\n\tuint iClientID = %u\n)",
			iClientID);

	CALL_CLIENT_PREAMBLE {
		unknown_90(iClientID);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_91(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_91(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_91(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_WEAPON_GROUP,
			iClientID, p2, p3);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_SET_WEAPON_GROUP(iClientID, p2, p3);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_WEAPON_GROUP,
			iClientID, p2, p3);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_VISITED_STATE,
			iClientID, p2, p3);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_SET_VISITED_STATE(iClientID, p2, p3);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_VISITED_STATE,
			iClientID, p2, p3);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_BEST_PATH,
			iClientID, p2, p3);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_REQUEST_BEST_PATH(iClientID, p2, p3);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_BEST_PATH,
			iClientID, p2, p3);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS,
			iClientID, p2, p3);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(iClientID, p2, p3);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS,
			iClientID, p2, p3);

	return retVal;
}

void HkIClientImpl::unknown_96(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3) {
	AddDebugLog("HkIClientImpl::unknown_96(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n\tuint iDunno3 = %u\n)",
			iClientID, iDunno, iDunno2, iDunno3);

	CALL_CLIENT_PREAMBLE {
		unknown_96(iClientID, iDunno, iDunno2, iDunno3);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS,
			iClientID, p2, p3);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(iClientID, p2, p3);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS,
			iClientID, p2, p3);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_MISSION_LOG,
			iClientID, p2, p3);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_SET_MISSION_LOG(iClientID, p2, p3);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_MISSION_LOG,
			iClientID, p2, p3);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(uint iClientID, unsigned char* p2, int p3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(\n\tuint iClientID = %u\n\tunsigned char* p2 = %p\n\tint p3 = %d\n)",
			iClientID, p2, p3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_INTERFACE_STATE,
			iClientID, p2, p3);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_SET_INTERFACE_STATE(iClientID, p2, p3);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_SET_INTERFACE_STATE,
			iClientID, p2, p3);

	return retVal;
}

void HkIClientImpl::unknown_100(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_100(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_PREAMBLE {
		unknown_100(iClientID, iDunno, iDunno2);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_101(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::unknown_101(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_101(iClientID, pDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_102(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_102(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_102(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_103(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_103(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_103(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_104(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_104(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_PREAMBLE {
		unknown_104(iClientID, iDunno, iDunno2);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_105(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_105(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_PREAMBLE {
		unknown_105(iClientID, iDunno, iDunno2);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_106(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_106(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_PREAMBLE {
		unknown_106(iClientID, iDunno, iDunno2);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_107(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::unknown_107(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	CALL_CLIENT_PREAMBLE {
		unknown_107(iClientID, iDunno, iDunno2);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_PLAYER_TRADE,
			iClientID, iDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_COMMON_PLAYER_TRADE(iClientID, iDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_PLAYER_TRADE,
			iClientID, iDunno);

	return retVal;
}

void HkIClientImpl::unknown_109(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_109(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_109(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_SCANNOTIFY(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(uint iClientID, wchar_t* _genArg1, uint _genArg2, char _genArg3) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(\n\tuint iClientID = %u\n\twchar_t* _genArg1 = %p\n\tuint _genArg2 = %u\n\tchar _genArg3 = %s\n)",
			iClientID, _genArg1, _genArg2, _genArg3);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST,
			iClientID, _genArg1, _genArg2, _genArg3);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_PLAYERLIST(iClientID, _genArg1, _genArg2, _genArg3);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST,
			iClientID, _genArg1, _genArg2, _genArg3);

	return retVal;
}

void HkIClientImpl::unknown_112(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_112(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_112(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(uint iClientID) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(\n\tuint iClientID = %u\n)",
			iClientID);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2,
			iClientID);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_PLAYERLIST_2(iClientID);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2,
			iClientID);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_6(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_7(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(uint iClientID, FLPACKET_UNKNOWN& pDunno) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(\n\tuint iClientID = %u\n\tFLPACKET_UNKNOWN& pDunno = %s\n)",
			iClientID, pDunno);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE,
			iClientID, pDunno);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE(iClientID, pDunno);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE,
			iClientID, pDunno);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_2(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(uint iClientID, uint iTargetID, uint iRank) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(\n\tuint iClientID = %u\n\tuint iTargetID = %u\n\tuint iRank = %u\n)",
			iClientID, iTargetID, iRank);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3,
			iClientID, iTargetID, iRank);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_3(iClientID, iTargetID, iRank);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3,
			iClientID, iTargetID, iRank);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_4(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4,
			iClientID, iDunno, iDunno2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(uint iClientID, uint iDunno, uint iDunno2) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n)",
			iClientID, iDunno, iDunno2);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5,
			iClientID, iDunno, iDunno2);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_5(iClientID, iDunno, iDunno2);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5,
			iClientID, iDunno, iDunno2);

	return retVal;
}

void HkIClientImpl::unknown_121(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_121(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_121(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(uint iClientID, uint iShipID, Vector& vFormationOffset) {
	AddDebugLog("HkIClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(\n\tuint iClientID = %u\n\tuint iShipID = %u\n\tVector& vFormationOffset = %s\n)",
			iClientID, iShipID, vFormationOffset);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_FORMATION_UPDATE,
			iClientID, iShipID, vFormationOffset);

	if(!skip) {
		CALL_CLIENT_PREAMBLE {
			retVal = Send_FLPACKET_SERVER_FORMATION_UPDATE(iClientID, iShipID, vFormationOffset);
		} CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_FORMATION_UPDATE,
			iClientID, iShipID, vFormationOffset);

	return retVal;
}

void HkIClientImpl::unknown_123(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3, uint iDunno4, uint iDunno5, uint iDunno6) {
	AddDebugLog("HkIClientImpl::unknown_123(\n\tuint iClientID = %u\n\tuint iDunno = %u\n\tuint iDunno2 = %u\n\tuint iDunno3 = %u\n\tuint iDunno4 = %u\n\tuint iDunno5 = %u\n\tuint iDunno6 = %u\n)",
			iClientID, iDunno, iDunno2, iDunno3, iDunno4, iDunno5, iDunno6);

	CALL_CLIENT_PREAMBLE {
		unknown_123(iClientID, iDunno, iDunno2, iDunno3, iDunno4, iDunno5, iDunno6);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_124(uint iClientID) {
	AddDebugLog("HkIClientImpl::unknown_124(\n\tuint iClientID = %u\n)",
			iClientID);

	CALL_CLIENT_PREAMBLE {
		unknown_124(iClientID);
	} CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_125(uint iClientID, uint iDunno) {
	AddDebugLog("HkIClientImpl::unknown_125(\n\tuint iClientID = %u\n\tuint iDunno = %u\n)",
			iClientID, iDunno);

	CALL_CLIENT_PREAMBLE {
		unknown_125(iClientID, iDunno);
	} CALL_CLIENT_POSTAMBLE;
}

int HkIClientImpl::unknown_126(char* szUnknown) {
	AddDebugLog("HkIClientImpl::unknown_126(\n\tchar* szUnknown = %s\n)",
			szUnknown);

	int retVal;
	CALL_CLIENT_PREAMBLE {
		retVal = unknown_126(szUnknown);
	} CALL_CLIENT_POSTAMBLE;

	return retVal;
}

namespace HkIServerImpl {
void __stdcall FireWeapon(uint clientID, XFireWeaponInfo const& fwi) {
	AddDebugLog("FireWeapon(\n\tuint clientID = %u\n\tXFireWeaponInfo const& fwi = %s\n)",
			clientID, fwi);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__FireWeapon,
			clientID, fwi);

	CHECK_FOR_DISCONNECT;

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.FireWeapon(clientID, fwi);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__FireWeapon,
			clientID, fwi);
}
}

namespace HkIServerImpl {
void __stdcall ActivateEquip(uint clientID, XActivateEquip const& aq) {
	AddDebugLog("ActivateEquip(\n\tuint clientID = %u\n\tXActivateEquip const& aq = %s\n)",
			clientID, aq);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateEquip,
			clientID, aq);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ActivateEquip(clientID, aq);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ActivateEquip,
			clientID, aq);
}
}

namespace HkIServerImpl {
void __stdcall ActivateCruise(uint clientID, XActivateCruise const& ac) {
	AddDebugLog("ActivateCruise(\n\tuint clientID = %u\n\tXActivateCruise const& ac = %s\n)",
			clientID, ac);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateCruise,
			clientID, ac);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ActivateCruise(clientID, ac);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ActivateCruise,
			clientID, ac);
}
}

namespace HkIServerImpl {
void __stdcall ActivateThrusters(uint clientID, XActivateThrusters const& at) {
	AddDebugLog("ActivateThrusters(\n\tuint clientID = %u\n\tXActivateThrusters const& at = %s\n)",
			clientID, at);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateThrusters,
			clientID, at);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ActivateThrusters(clientID, at);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ActivateThrusters,
			clientID, at);
}
}

namespace HkIServerImpl {
void __stdcall SetTarget(uint clientID, XSetTarget const& st) {
	AddDebugLog("SetTarget(\n\tuint clientID = %u\n\tXSetTarget const& st = %s\n)",
			clientID, st);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTarget,
			clientID, st);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SetTarget(clientID, st);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SetTarget,
			clientID, st);
}
}

namespace HkIServerImpl {
void __stdcall TractorObjects(uint clientID, XTractorObjects const& to) {
	AddDebugLog("TractorObjects(\n\tuint clientID = %u\n\tXTractorObjects const& to = %s\n)",
			clientID, to);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TractorObjects,
			clientID, to);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.TractorObjects(clientID, to);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__TractorObjects,
			clientID, to);
}
}

namespace HkIServerImpl {
void __stdcall GoTradelane(uint clientID, XGoTradelane const& gt) {
	AddDebugLog("GoTradelane(\n\tuint clientID = %u\n\tXGoTradelane const& gt = %s\n)",
			clientID, gt);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GoTradelane,
			clientID, gt);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.GoTradelane(clientID, gt);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__GoTradelane,
			clientID, gt);
}
}

namespace HkIServerImpl {
void __stdcall StopTradelane(uint clientID, uint shipID, uint tradelaneRing1, uint tradelaneRing2) {
	AddDebugLog("StopTradelane(\n\tuint clientID = %u\n\tuint shipID = %u\n\tuint tradelaneRing1 = %u\n\tuint tradelaneRing2 = %u\n)",
			clientID, shipID, tradelaneRing1, tradelaneRing2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradelane,
			clientID, shipID, tradelaneRing1, tradelaneRing2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.StopTradelane(clientID, shipID, tradelaneRing1, tradelaneRing2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__StopTradelane,
			clientID, shipID, tradelaneRing1, tradelaneRing2);
}
}

namespace HkIServerImpl {
void __stdcall JettisonCargo(uint clientID, XJettisonCargo const& jc) {
	AddDebugLog("JettisonCargo(\n\tuint clientID = %u\n\tXJettisonCargo const& jc = %s\n)",
			clientID, jc);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JettisonCargo,
			clientID, jc);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.JettisonCargo(clientID, jc);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__JettisonCargo,
			clientID, jc);
}
}

namespace HkIServerImpl {
bool __stdcall Startup(SStartupInfo const& si) {
	AddDebugLog("Startup(\n\tSStartupInfo const& si = %s\n)",
			si);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IServerImpl__Startup,
			si);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			retVal = Server.Startup(si);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__Startup,
			si);

	return retVal;
}
}

namespace HkIServerImpl {
void __stdcall DisConnect(uint clientID, EFLConnection conn) {
	AddDebugLog("DisConnect(\n\tuint clientID = %u\n\tEFLConnection conn = %s\n)",
			clientID, conn);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DisConnect,
			clientID, conn);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.DisConnect(clientID, conn);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__DisConnect,
			clientID, conn);
}
}

namespace HkIServerImpl {
void __stdcall OnConnect(uint clientID) {
	AddDebugLog("OnConnect(\n\tuint clientID = %u\n)",
			clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__OnConnect,
			clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.OnConnect(clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__OnConnect,
			clientID);
}
}

namespace HkIServerImpl {
void __stdcall Login(SLoginInfo const& li, uint clientID) {
	AddDebugLog("Login(\n\tSLoginInfo const& li = %s\n\tuint clientID = %u\n)",
			li, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Login,
			li, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.Login(li, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__Login,
			li, clientID);
}
}

namespace HkIServerImpl {
void __stdcall CharacterInfoReq(uint clientID, bool _genArg1) {
	AddDebugLog("CharacterInfoReq(\n\tuint clientID = %u\n\tbool _genArg1 = %d\n)",
			clientID, _genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterInfoReq,
			clientID, _genArg1);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.CharacterInfoReq(clientID, _genArg1);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__CharacterInfoReq,
			clientID, _genArg1);
}
}

namespace HkIServerImpl {
void __stdcall CharacterSelect(CHARACTER_ID const& cid, uint clientID) {
	AddDebugLog("CharacterSelect(\n\tCHARACTER_ID const& cid = %s\n\tuint clientID = %u\n)",
			cid, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterSelect,
			cid, clientID);

	CHECK_FOR_DISCONNECT;

	bool innerCheck = HkIServerImpl__CharacterSelect__Inner(cid, clientID);
	if(!innerCheck) return;
	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.CharacterSelect(cid, clientID);
		} CALL_SERVER_POSTAMBLE;
	}
	HkIServerImpl__CharacterSelect__InnerAfter(cid, clientID);


	CallPluginsAfter(HookedCall::IServerImpl__CharacterSelect,
			cid, clientID);
}
}

namespace HkIServerImpl {
void __stdcall CreateNewCharacter(SCreateCharacterInfo const& _genArg1, uint clientID) {
	AddDebugLog("CreateNewCharacter(\n\tSCreateCharacterInfo const& _genArg1 = %s\n\tuint clientID = %u\n)",
			_genArg1, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CreateNewCharacter,
			_genArg1, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.CreateNewCharacter(_genArg1, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__CreateNewCharacter,
			_genArg1, clientID);
}
}

namespace HkIServerImpl {
void __stdcall DestroyCharacter(CHARACTER_ID const& _genArg1, uint clientID) {
	AddDebugLog("DestroyCharacter(\n\tCHARACTER_ID const& _genArg1 = %s\n\tuint clientID = %u\n)",
			_genArg1, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DestroyCharacter,
			_genArg1, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.DestroyCharacter(_genArg1, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__DestroyCharacter,
			_genArg1, clientID);
}
}

namespace HkIServerImpl {
void __stdcall ReqShipArch(uint archID, uint clientID) {
	AddDebugLog("ReqShipArch(\n\tuint archID = %u\n\tuint clientID = %u\n)",
			archID, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqShipArch,
			archID, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ReqShipArch(archID, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ReqShipArch,
			archID, clientID);
}
}

namespace HkIServerImpl {
void __stdcall ReqHullStatus(float status, uint clientID) {
	AddDebugLog("ReqHullStatus(\n\tfloat status = %f\n\tuint clientID = %u\n)",
			status, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqHullStatus,
			status, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ReqHullStatus(status, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ReqHullStatus,
			status, clientID);
}
}

namespace HkIServerImpl {
void __stdcall ReqCollisionGroups(st6::list<CollisionGroupDesc> const& collisionGroups, uint clientID) {
	AddDebugLog("ReqCollisionGroups(\n\tst6::list<CollisionGroupDesc> const& collisionGroups = %s\n\tuint clientID = %u\n)",
			collisionGroups, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqCollisionGroups,
			collisionGroups, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ReqCollisionGroups(collisionGroups, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ReqCollisionGroups,
			collisionGroups, clientID);
}
}

namespace HkIServerImpl {
void __stdcall ReqEquipment(EquipDescList const& edl, uint clientID) {
	AddDebugLog("ReqEquipment(\n\tEquipDescList const& edl = %s\n\tuint clientID = %u\n)",
			edl, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqEquipment,
			edl, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ReqEquipment(edl, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ReqEquipment,
			edl, clientID);
}
}

namespace HkIServerImpl {
void __stdcall ReqAddItem(uint goodID, char const* hardpoint, int count, float status, bool mounted, uint iClientID) {
	AddDebugLog("ReqAddItem(\n\tuint goodID = %u\n\tchar const* hardpoint = %p\n\tint count = %d\n\tfloat status = %f\n\tbool mounted = %d\n\tuint iClientID = %u\n)",
			goodID, hardpoint, count, status, mounted, iClientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqAddItem,
			goodID, hardpoint, count, status, mounted, iClientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ReqAddItem(goodID, hardpoint, count, status, mounted, iClientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ReqAddItem,
			goodID, hardpoint, count, status, mounted, iClientID);
}
}

namespace HkIServerImpl {
void __stdcall ReqRemoveItem(ushort slotID, int count, uint clientID) {
	AddDebugLog("ReqRemoveItem(\n\tushort slotID = %u\n\tint count = %d\n\tuint clientID = %u\n)",
			slotID, count, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqRemoveItem,
			slotID, count, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ReqRemoveItem(slotID, count, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ReqRemoveItem,
			slotID, count, clientID);
}
}

namespace HkIServerImpl {
void __stdcall ReqModifyItem(ushort slotID, char const* hardpoint, int count, float status, bool mounted, uint iClientID) {
	AddDebugLog("ReqModifyItem(\n\tushort slotID = %u\n\tchar const* hardpoint = %p\n\tint count = %d\n\tfloat status = %f\n\tbool mounted = %d\n\tuint iClientID = %u\n)",
			slotID, hardpoint, count, status, mounted, iClientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqModifyItem,
			slotID, hardpoint, count, status, mounted, iClientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ReqModifyItem(slotID, hardpoint, count, status, mounted, iClientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ReqModifyItem,
			slotID, hardpoint, count, status, mounted, iClientID);
}
}

namespace HkIServerImpl {
void __stdcall ReqSetCash(int cash, uint clientID) {
	AddDebugLog("ReqSetCash(\n\tint cash = %d\n\tuint clientID = %u\n)",
			cash, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqSetCash,
			cash, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ReqSetCash(cash, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ReqSetCash,
			cash, clientID);
}
}

namespace HkIServerImpl {
void __stdcall ReqChangeCash(int cashAdd, uint clientID) {
	AddDebugLog("ReqChangeCash(\n\tint cashAdd = %d\n\tuint clientID = %u\n)",
			cashAdd, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqChangeCash,
			cashAdd, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.ReqChangeCash(cashAdd, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__ReqChangeCash,
			cashAdd, clientID);
}
}

namespace HkIServerImpl {
void __stdcall BaseEnter(uint baseID, uint clientID) {
	AddDebugLog("BaseEnter(\n\tuint baseID = %u\n\tuint clientID = %u\n)",
			baseID, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseEnter,
			baseID, clientID);

	CHECK_FOR_DISCONNECT;

	HkIServerImpl__BaseEnter__Inner(baseID, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.BaseEnter(baseID, clientID);
		} CALL_SERVER_POSTAMBLE;
	}
	HkIServerImpl__BaseEnter__InnerAfter(baseID, clientID);


	CallPluginsAfter(HookedCall::IServerImpl__BaseEnter,
			baseID, clientID);
}
}

namespace HkIServerImpl {
void __stdcall BaseExit(uint baseID, uint clientID) {
	AddDebugLog("BaseExit(\n\tuint baseID = %u\n\tuint clientID = %u\n)",
			baseID, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseExit,
			baseID, clientID);

	CHECK_FOR_DISCONNECT;

	HkIServerImpl__BaseExit__Inner(baseID, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.BaseExit(baseID, clientID);
		} CALL_SERVER_POSTAMBLE;
	}
	HkIServerImpl__BaseExit__InnerAfter(baseID, clientID);


	CallPluginsAfter(HookedCall::IServerImpl__BaseExit,
			baseID, clientID);
}
}

namespace HkIServerImpl {
void __stdcall LocationEnter(uint locationID, uint clientID) {
	AddDebugLog("LocationEnter(\n\tuint locationID = %u\n\tuint clientID = %u\n)",
			locationID, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationEnter,
			locationID, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.LocationEnter(locationID, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__LocationEnter,
			locationID, clientID);
}
}

namespace HkIServerImpl {
void __stdcall LocationExit(uint locationID, uint clientID) {
	AddDebugLog("LocationExit(\n\tuint locationID = %u\n\tuint clientID = %u\n)",
			locationID, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationExit,
			locationID, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.LocationExit(locationID, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__LocationExit,
			locationID, clientID);
}
}

namespace HkIServerImpl {
void __stdcall BaseInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3) {
	AddDebugLog("BaseInfoRequest(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tbool _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseInfoRequest,
			_genArg1, _genArg2, _genArg3);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.BaseInfoRequest(_genArg1, _genArg2, _genArg3);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__BaseInfoRequest,
			_genArg1, _genArg2, _genArg3);
}
}

namespace HkIServerImpl {
void __stdcall LocationInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3) {
	AddDebugLog("LocationInfoRequest(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tbool _genArg3 = %d\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationInfoRequest,
			_genArg1, _genArg2, _genArg3);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.LocationInfoRequest(_genArg1, _genArg2, _genArg3);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__LocationInfoRequest,
			_genArg1, _genArg2, _genArg3);
}
}

namespace HkIServerImpl {
void __stdcall GFObjSelect(unsigned int _genArg1, unsigned int _genArg2) {
	AddDebugLog("GFObjSelect(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFObjSelect,
			_genArg1, _genArg2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.GFObjSelect(_genArg1, _genArg2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__GFObjSelect,
			_genArg1, _genArg2);
}
}

namespace HkIServerImpl {
void __stdcall GFGoodVaporized(SGFGoodVaporizedInfo const& gvi, uint clientID) {
	AddDebugLog("GFGoodVaporized(\n\tSGFGoodVaporizedInfo const& gvi = %s\n\tuint clientID = %u\n)",
			gvi, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodVaporized,
			gvi, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.GFGoodVaporized(gvi, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__GFGoodVaporized,
			gvi, clientID);
}
}

namespace HkIServerImpl {
void __stdcall MissionResponse(unsigned int _genArg1, unsigned long _genArg2, bool _genArg3, uint clientID) {
	AddDebugLog("MissionResponse(\n\tunsigned int _genArg1 = %u\n\tunsigned long _genArg2 = %u\n\tbool _genArg3 = %d\n\tuint clientID = %u\n)",
			_genArg1, _genArg2, _genArg3, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__MissionResponse,
			_genArg1, _genArg2, _genArg3, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.MissionResponse(_genArg1, _genArg2, _genArg3, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__MissionResponse,
			_genArg1, _genArg2, _genArg3, clientID);
}
}

namespace HkIServerImpl {
void __stdcall TradeResponse(unsigned char const* _genArg1, int _genArg2, uint clientID) {
	AddDebugLog("TradeResponse(\n\tunsigned char const* _genArg1 = %p\n\tint _genArg2 = %d\n\tuint clientID = %u\n)",
			_genArg1, _genArg2, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TradeResponse,
			_genArg1, _genArg2, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.TradeResponse(_genArg1, _genArg2, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__TradeResponse,
			_genArg1, _genArg2, clientID);
}
}

namespace HkIServerImpl {
void __stdcall GFGoodBuy(SGFGoodBuyInfo const& _genArg1, uint clientID) {
	AddDebugLog("GFGoodBuy(\n\tSGFGoodBuyInfo const& _genArg1 = %s\n\tuint clientID = %u\n)",
			_genArg1, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodBuy,
			_genArg1, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.GFGoodBuy(_genArg1, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__GFGoodBuy,
			_genArg1, clientID);
}
}

namespace HkIServerImpl {
void __stdcall GFGoodSell(SGFGoodSellInfo const& _genArg1, uint clientID) {
	AddDebugLog("GFGoodSell(\n\tSGFGoodSellInfo const& _genArg1 = %s\n\tuint clientID = %u\n)",
			_genArg1, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodSell,
			_genArg1, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.GFGoodSell(_genArg1, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__GFGoodSell,
			_genArg1, clientID);
}
}

namespace HkIServerImpl {
void __stdcall SystemSwitchOutComplete(uint shipID, uint clientID) {
	AddDebugLog("SystemSwitchOutComplete(\n\tuint shipID = %u\n\tuint clientID = %u\n)",
			shipID, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SystemSwitchOutComplete,
			shipID, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SystemSwitchOutComplete(shipID, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SystemSwitchOutComplete,
			shipID, clientID);
}
}

namespace HkIServerImpl {
void __stdcall PlayerLaunch(uint shipID, uint clientID) {
	AddDebugLog("PlayerLaunch(\n\tuint shipID = %u\n\tuint clientID = %u\n)",
			shipID, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PlayerLaunch,
			shipID, clientID);

	CHECK_FOR_DISCONNECT;

	HkIServerImpl__PlayerLaunch__Inner(shipID, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.PlayerLaunch(shipID, clientID);
		} CALL_SERVER_POSTAMBLE;
	}
	HkIServerImpl__PlayerLaunch__InnerAfter(shipID, clientID);


	CallPluginsAfter(HookedCall::IServerImpl__PlayerLaunch,
			shipID, clientID);
}
}

namespace HkIServerImpl {
void __stdcall LaunchComplete(uint baseID, uint shipID) {
	AddDebugLog("LaunchComplete(\n\tuint baseID = %u\n\tuint shipID = %u\n)",
			baseID, shipID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LaunchComplete,
			baseID, shipID);

	HkIServerImpl__LaunchComplete__Inner(baseID, shipID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.LaunchComplete(baseID, shipID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__LaunchComplete,
			baseID, shipID);
}
}

namespace HkIServerImpl {
void __stdcall JumpInComplete(uint systemID, uint shipID) {
	AddDebugLog("JumpInComplete(\n\tuint systemID = %u\n\tuint shipID = %u\n)",
			systemID, shipID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JumpInComplete,
			systemID, shipID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.JumpInComplete(systemID, shipID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__JumpInComplete,
			systemID, shipID);
}
}

namespace HkIServerImpl {
void __stdcall Hail(unsigned int _genArg1, unsigned int _genArg2, unsigned int _genArg3) {
	AddDebugLog("Hail(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tunsigned int _genArg3 = %u\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Hail,
			_genArg1, _genArg2, _genArg3);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.Hail(_genArg1, _genArg2, _genArg3);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__Hail,
			_genArg1, _genArg2, _genArg3);
}
}

namespace HkIServerImpl {
void __stdcall SPObjUpdate(SSPObjUpdateInfo const& ui, uint clientID) {
	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjUpdate,
			ui, clientID);

	CHECK_FOR_DISCONNECT;

	bool innerCheck = HkIServerImpl__SPObjUpdate__Inner(ui, clientID);
	if(!innerCheck) return;
	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SPObjUpdate(ui, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SPObjUpdate,
			ui, clientID);
}
}

namespace HkIServerImpl {
void __stdcall SPMunitionCollision(SSPMunitionCollisionInfo const& mci, uint clientID) {
	AddDebugLog("SPMunitionCollision(\n\tSSPMunitionCollisionInfo const& mci = %s\n\tuint clientID = %u\n)",
			mci, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPMunitionCollision,
			mci, clientID);

	CHECK_FOR_DISCONNECT;

	HkIServerImpl__SPMunitionCollision__Inner(mci, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SPMunitionCollision(mci, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SPMunitionCollision,
			mci, clientID);
}
}

namespace HkIServerImpl {
void __stdcall SPObjCollision(SSPObjCollisionInfo const& oci, uint clientID) {
	AddDebugLog("SPObjCollision(\n\tSSPObjCollisionInfo const& oci = %s\n\tuint clientID = %u\n)",
			oci, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjCollision,
			oci, clientID);

	CHECK_FOR_DISCONNECT;

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SPObjCollision(oci, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SPObjCollision,
			oci, clientID);
}
}

namespace HkIServerImpl {
void __stdcall SPRequestUseItem(SSPUseItem const& ui, uint clientID) {
	AddDebugLog("SPRequestUseItem(\n\tSSPUseItem const& ui = %s\n\tuint clientID = %u\n)",
			ui, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestUseItem,
			ui, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SPRequestUseItem(ui, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SPRequestUseItem,
			ui, clientID);
}
}

namespace HkIServerImpl {
void __stdcall SPRequestInvincibility(uint shipID, bool enable, InvincibilityReason reason, uint clientID) {
	AddDebugLog("SPRequestInvincibility(\n\tuint shipID = %u\n\tbool enable = %d\n\tInvincibilityReason reason = %s\n\tuint clientID = %u\n)",
			shipID, enable, reason, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestInvincibility,
			shipID, enable, reason, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SPRequestInvincibility(shipID, enable, reason, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SPRequestInvincibility,
			shipID, enable, reason, clientID);
}
}

namespace HkIServerImpl {
void __stdcall RequestEvent(int eventType, uint shipID, uint dockTarget, uint _genArg1, ulong _genArg2, uint clientID) {
	AddDebugLog("RequestEvent(\n\tint eventType = %d\n\tuint shipID = %u\n\tuint dockTarget = %u\n\tuint _genArg1 = %u\n\tulong _genArg2 = %u\n\tuint clientID = %u\n)",
			eventType, shipID, dockTarget, _genArg1, _genArg2, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestEvent,
			eventType, shipID, dockTarget, _genArg1, _genArg2, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.RequestEvent(eventType, shipID, dockTarget, _genArg1, _genArg2, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__RequestEvent,
			eventType, shipID, dockTarget, _genArg1, _genArg2, clientID);
}
}

namespace HkIServerImpl {
void __stdcall RequestCancel(int eventType, uint shipID, uint _genArg1, ulong _genArg2, uint clientID) {
	AddDebugLog("RequestCancel(\n\tint eventType = %d\n\tuint shipID = %u\n\tuint _genArg1 = %u\n\tulong _genArg2 = %u\n\tuint clientID = %u\n)",
			eventType, shipID, _genArg1, _genArg2, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCancel,
			eventType, shipID, _genArg1, _genArg2, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.RequestCancel(eventType, shipID, _genArg1, _genArg2, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__RequestCancel,
			eventType, shipID, _genArg1, _genArg2, clientID);
}
}

namespace HkIServerImpl {
void __stdcall MineAsteroid(uint systemID, Vector const& pos, uint crateID, uint lootID, uint count, uint clientID) {
	AddDebugLog("MineAsteroid(\n\tuint systemID = %u\n\tVector const& pos = %s\n\tuint crateID = %u\n\tuint lootID = %u\n\tuint count = %u\n\tuint clientID = %u\n)",
			systemID, pos, crateID, lootID, count, clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__MineAsteroid,
			systemID, pos, crateID, lootID, count, clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.MineAsteroid(systemID, pos, crateID, lootID, count, clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__MineAsteroid,
			systemID, pos, crateID, lootID, count, clientID);
}
}

namespace HkIServerImpl {
void __stdcall RequestCreateShip(uint clientID) {
	AddDebugLog("RequestCreateShip(\n\tuint clientID = %u\n)",
			clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCreateShip,
			clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.RequestCreateShip(clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__RequestCreateShip,
			clientID);
}
}

namespace HkIServerImpl {
void __stdcall SPScanCargo(uint const& _genArg1, uint const& _genArg2, uint _genArg3) {
	AddDebugLog("SPScanCargo(\n\tuint const& _genArg1 = %s\n\tuint const& _genArg2 = %s\n\tuint _genArg3 = %u\n)",
			_genArg1, _genArg2, _genArg3);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPScanCargo,
			_genArg1, _genArg2, _genArg3);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SPScanCargo(_genArg1, _genArg2, _genArg3);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SPScanCargo,
			_genArg1, _genArg2, _genArg3);
}
}

namespace HkIServerImpl {
void __stdcall SetManeuver(uint clientID, XSetManeuver const& sm) {
	AddDebugLog("SetManeuver(\n\tuint clientID = %u\n\tXSetManeuver const& sm = %s\n)",
			clientID, sm);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetManeuver,
			clientID, sm);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SetManeuver(clientID, sm);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SetManeuver,
			clientID, sm);
}
}

namespace HkIServerImpl {
void __stdcall InterfaceItemUsed(uint _genArg1, uint _genArg2) {
	AddDebugLog("InterfaceItemUsed(\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InterfaceItemUsed,
			_genArg1, _genArg2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.InterfaceItemUsed(_genArg1, _genArg2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__InterfaceItemUsed,
			_genArg1, _genArg2);
}
}

namespace HkIServerImpl {
void __stdcall AbortMission(uint clientID, uint _genArg1) {
	AddDebugLog("AbortMission(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)",
			clientID, _genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AbortMission,
			clientID, _genArg1);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.AbortMission(clientID, _genArg1);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__AbortMission,
			clientID, _genArg1);
}
}

namespace HkIServerImpl {
void __stdcall SetWeaponGroup(uint clientID, uchar* _genArg1, int _genArg2) {
	AddDebugLog("SetWeaponGroup(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
			clientID, _genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetWeaponGroup,
			clientID, _genArg1, _genArg2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SetWeaponGroup(clientID, _genArg1, _genArg2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SetWeaponGroup,
			clientID, _genArg1, _genArg2);
}
}

namespace HkIServerImpl {
void __stdcall SetVisitedState(uint clientID, uchar* _genArg1, int _genArg2) {
	AddDebugLog("SetVisitedState(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
			clientID, _genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetVisitedState,
			clientID, _genArg1, _genArg2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SetVisitedState(clientID, _genArg1, _genArg2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SetVisitedState,
			clientID, _genArg1, _genArg2);
}
}

namespace HkIServerImpl {
void __stdcall RequestBestPath(uint clientID, uchar* _genArg1, int _genArg2) {
	AddDebugLog("RequestBestPath(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
			clientID, _genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestBestPath,
			clientID, _genArg1, _genArg2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.RequestBestPath(clientID, _genArg1, _genArg2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__RequestBestPath,
			clientID, _genArg1, _genArg2);
}
}

namespace HkIServerImpl {
void __stdcall RequestPlayerStats(uint clientID, uchar* _genArg1, int _genArg2) {
	AddDebugLog("RequestPlayerStats(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
			clientID, _genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestPlayerStats,
			clientID, _genArg1, _genArg2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.RequestPlayerStats(clientID, _genArg1, _genArg2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__RequestPlayerStats,
			clientID, _genArg1, _genArg2);
}
}

namespace HkIServerImpl {
void __stdcall RequestGroupPositions(uint clientID, uchar* _genArg1, int _genArg2) {
	AddDebugLog("RequestGroupPositions(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
			clientID, _genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestGroupPositions,
			clientID, _genArg1, _genArg2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.RequestGroupPositions(clientID, _genArg1, _genArg2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__RequestGroupPositions,
			clientID, _genArg1, _genArg2);
}
}

namespace HkIServerImpl {
void __stdcall SetInterfaceState(uint clientID, uchar* _genArg1, int _genArg2) {
	AddDebugLog("SetInterfaceState(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
			clientID, _genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetInterfaceState,
			clientID, _genArg1, _genArg2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SetInterfaceState(clientID, _genArg1, _genArg2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SetInterfaceState,
			clientID, _genArg1, _genArg2);
}
}

namespace HkIServerImpl {
void __stdcall RequestRankLevel(uint clientID, uchar* _genArg1, int _genArg2) {
	AddDebugLog("RequestRankLevel(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
			clientID, _genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestRankLevel,
			clientID, _genArg1, _genArg2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.RequestRankLevel(clientID, _genArg1, _genArg2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__RequestRankLevel,
			clientID, _genArg1, _genArg2);
}
}

namespace HkIServerImpl {
void __stdcall InitiateTrade(uint clientID1, uint clientID2) {
	AddDebugLog("InitiateTrade(\n\tuint clientID1 = %u\n\tuint clientID2 = %u\n)",
			clientID1, clientID2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InitiateTrade,
			clientID1, clientID2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.InitiateTrade(clientID1, clientID2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__InitiateTrade,
			clientID1, clientID2);
}
}

namespace HkIServerImpl {
void __stdcall TerminateTrade(uint clientID, int accepted) {
	AddDebugLog("TerminateTrade(\n\tuint clientID = %u\n\tint accepted = %d\n)",
			clientID, accepted);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TerminateTrade,
			clientID, accepted);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.TerminateTrade(clientID, accepted);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__TerminateTrade,
			clientID, accepted);
}
}

namespace HkIServerImpl {
void __stdcall AcceptTrade(uint clientID, bool _genArg1) {
	AddDebugLog("AcceptTrade(\n\tuint clientID = %u\n\tbool _genArg1 = %d\n)",
			clientID, _genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AcceptTrade,
			clientID, _genArg1);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.AcceptTrade(clientID, _genArg1);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__AcceptTrade,
			clientID, _genArg1);
}
}

namespace HkIServerImpl {
void __stdcall SetTradeMoney(uint clientID, ulong _genArg1) {
	AddDebugLog("SetTradeMoney(\n\tuint clientID = %u\n\tulong _genArg1 = %u\n)",
			clientID, _genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTradeMoney,
			clientID, _genArg1);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SetTradeMoney(clientID, _genArg1);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__SetTradeMoney,
			clientID, _genArg1);
}
}

namespace HkIServerImpl {
void __stdcall AddTradeEquip(uint clientID, EquipDesc const& ed) {
	AddDebugLog("AddTradeEquip(\n\tuint clientID = %u\n\tEquipDesc const& ed = %s\n)",
			clientID, ed);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AddTradeEquip,
			clientID, ed);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.AddTradeEquip(clientID, ed);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__AddTradeEquip,
			clientID, ed);
}
}

namespace HkIServerImpl {
void __stdcall DelTradeEquip(uint clientID, EquipDesc const& ed) {
	AddDebugLog("DelTradeEquip(\n\tuint clientID = %u\n\tEquipDesc const& ed = %s\n)",
			clientID, ed);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DelTradeEquip,
			clientID, ed);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.DelTradeEquip(clientID, ed);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__DelTradeEquip,
			clientID, ed);
}
}

namespace HkIServerImpl {
void __stdcall RequestTrade(uint _genArg1, uint _genArg2) {
	AddDebugLog("RequestTrade(\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
			_genArg1, _genArg2);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestTrade,
			_genArg1, _genArg2);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.RequestTrade(_genArg1, _genArg2);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__RequestTrade,
			_genArg1, _genArg2);
}
}

namespace HkIServerImpl {
void __stdcall StopTradeRequest(uint clientID) {
	AddDebugLog("StopTradeRequest(\n\tuint clientID = %u\n)",
			clientID);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradeRequest,
			clientID);

	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.StopTradeRequest(clientID);
		} CALL_SERVER_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IServerImpl__StopTradeRequest,
			clientID);
}
}

namespace HkIServerImpl {
void __stdcall Dock(uint const& _genArg1, uint const& _genArg2) {

}
}

namespace HkIServerImpl {
void __stdcall SubmitChat(CHAT_ID cidFrom, ulong size, void const* rdlReader, CHAT_ID cidTo, int _genArg1) {
	AddDebugLog("SubmitChat(\n\tCHAT_ID cidFrom = %s\n\tulong size = %u\n\tvoid const* rdlReader = %p\n\tCHAT_ID cidTo = %s\n\tint _genArg1 = %d\n)",
			cidFrom, size, rdlReader, cidTo, _genArg1);

	auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SubmitChat,
			cidFrom, size, rdlReader, cidTo, _genArg1);

	HkIServerImpl__SubmitChat__Inner(cidFrom, size, rdlReader, cidTo, _genArg1);

	g_bInSubmitChat = true;
	if(!skip) {
		CALL_SERVER_PREAMBLE {
			Server.SubmitChat(cidFrom, size, rdlReader, cidTo, _genArg1);
		} CALL_SERVER_POSTAMBLE;
	}
	g_bInSubmitChat = false;

	CallPluginsAfter(HookedCall::IServerImpl__SubmitChat,
			cidFrom, size, rdlReader, cidTo, _genArg1);
}
}

HookEntry HkIServerImplEntries[] = {
	{ FARPROC(HkIServerImpl::FireWeapon), 0x004, nullptr },
	{ FARPROC(HkIServerImpl::ActivateEquip), 0x008, nullptr },
	{ FARPROC(HkIServerImpl::ActivateCruise), 0x00C, nullptr },
	{ FARPROC(HkIServerImpl::ActivateThrusters), 0x010, nullptr },
	{ FARPROC(HkIServerImpl::SetTarget), 0x014, nullptr },
	{ FARPROC(HkIServerImpl::TractorObjects), 0x018, nullptr },
	{ FARPROC(HkIServerImpl::GoTradelane), 0x01C, nullptr },
	{ FARPROC(HkIServerImpl::StopTradelane), 0x020, nullptr },
	{ FARPROC(HkIServerImpl::JettisonCargo), 0x024, nullptr },
	{ FARPROC(HkIServerImpl::Startup), 0x028, nullptr },
	{ FARPROC(HkIServerImpl::DisConnect), 0x044, nullptr },
	{ FARPROC(HkIServerImpl::OnConnect), 0x048, nullptr },
	{ FARPROC(HkIServerImpl::Login), 0x04C, nullptr },
	{ FARPROC(HkIServerImpl::CharacterInfoReq), 0x050, nullptr },
	{ FARPROC(HkIServerImpl::CharacterSelect), 0x054, nullptr },
	{ FARPROC(HkIServerImpl::CreateNewCharacter), 0x05C, nullptr },
	{ FARPROC(HkIServerImpl::DestroyCharacter), 0x060, nullptr },
	{ FARPROC(HkIServerImpl::ReqShipArch), 0x068, nullptr },
	{ FARPROC(HkIServerImpl::ReqHullStatus), 0x06C, nullptr },
	{ FARPROC(HkIServerImpl::ReqCollisionGroups), 0x070, nullptr },
	{ FARPROC(HkIServerImpl::ReqEquipment), 0x074, nullptr },
	{ FARPROC(HkIServerImpl::ReqAddItem), 0x07C, nullptr },
	{ FARPROC(HkIServerImpl::ReqRemoveItem), 0x080, nullptr },
	{ FARPROC(HkIServerImpl::ReqModifyItem), 0x084, nullptr },
	{ FARPROC(HkIServerImpl::ReqSetCash), 0x088, nullptr },
	{ FARPROC(HkIServerImpl::ReqChangeCash), 0x08C, nullptr },
	{ FARPROC(HkIServerImpl::BaseEnter), 0x090, nullptr },
	{ FARPROC(HkIServerImpl::BaseExit), 0x094, nullptr },
	{ FARPROC(HkIServerImpl::LocationEnter), 0x098, nullptr },
	{ FARPROC(HkIServerImpl::LocationExit), 0x09C, nullptr },
	{ FARPROC(HkIServerImpl::BaseInfoRequest), 0x0A0, nullptr },
	{ FARPROC(HkIServerImpl::LocationInfoRequest), 0x0A4, nullptr },
	{ FARPROC(HkIServerImpl::GFObjSelect), 0x0A8, nullptr },
	{ FARPROC(HkIServerImpl::GFGoodVaporized), 0x0AC, nullptr },
	{ FARPROC(HkIServerImpl::MissionResponse), 0x0B0, nullptr },
	{ FARPROC(HkIServerImpl::TradeResponse), 0x0B4, nullptr },
	{ FARPROC(HkIServerImpl::GFGoodBuy), 0x0B8, nullptr },
	{ FARPROC(HkIServerImpl::GFGoodSell), 0x0BC, nullptr },
	{ FARPROC(HkIServerImpl::SystemSwitchOutComplete), 0x0C0, nullptr },
	{ FARPROC(HkIServerImpl::PlayerLaunch), 0x0C4, nullptr },
	{ FARPROC(HkIServerImpl::LaunchComplete), 0x0C8, nullptr },
	{ FARPROC(HkIServerImpl::JumpInComplete), 0x0CC, nullptr },
	{ FARPROC(HkIServerImpl::Hail), 0x0D0, nullptr },
	{ FARPROC(HkIServerImpl::SPObjUpdate), 0x0D4, nullptr },
	{ FARPROC(HkIServerImpl::SPMunitionCollision), 0x0D8, nullptr },
	{ FARPROC(HkIServerImpl::SPObjCollision), 0x0E0, nullptr },
	{ FARPROC(HkIServerImpl::SPRequestUseItem), 0x0E4, nullptr },
	{ FARPROC(HkIServerImpl::SPRequestInvincibility), 0x0E8, nullptr },
	{ FARPROC(HkIServerImpl::RequestEvent), 0x0F4, nullptr },
	{ FARPROC(HkIServerImpl::RequestCancel), 0x0F8, nullptr },
	{ FARPROC(HkIServerImpl::MineAsteroid), 0x0FC, nullptr },
	{ FARPROC(HkIServerImpl::RequestCreateShip), 0x104, nullptr },
	{ FARPROC(HkIServerImpl::SPScanCargo), 0x108, nullptr },
	{ FARPROC(HkIServerImpl::SetManeuver), 0x10C, nullptr },
	{ FARPROC(HkIServerImpl::InterfaceItemUsed), 0x110, nullptr },
	{ FARPROC(HkIServerImpl::AbortMission), 0x114, nullptr },
	{ FARPROC(HkIServerImpl::SetWeaponGroup), 0x11C, nullptr },
	{ FARPROC(HkIServerImpl::SetVisitedState), 0x120, nullptr },
	{ FARPROC(HkIServerImpl::RequestBestPath), 0x124, nullptr },
	{ FARPROC(HkIServerImpl::RequestPlayerStats), 0x128, nullptr },
	{ FARPROC(HkIServerImpl::RequestGroupPositions), 0x130, nullptr },
	{ FARPROC(HkIServerImpl::SetInterfaceState), 0x138, nullptr },
	{ FARPROC(HkIServerImpl::RequestRankLevel), 0x13C, nullptr },
	{ FARPROC(HkIServerImpl::InitiateTrade), 0x140, nullptr },
	{ FARPROC(HkIServerImpl::TerminateTrade), 0x144, nullptr },
	{ FARPROC(HkIServerImpl::AcceptTrade), 0x148, nullptr },
	{ FARPROC(HkIServerImpl::SetTradeMoney), 0x14C, nullptr },
	{ FARPROC(HkIServerImpl::AddTradeEquip), 0x150, nullptr },
	{ FARPROC(HkIServerImpl::DelTradeEquip), 0x154, nullptr },
	{ FARPROC(HkIServerImpl::RequestTrade), 0x158, nullptr },
	{ FARPROC(HkIServerImpl::StopTradeRequest), 0x15C, nullptr },
	{ FARPROC(HkIServerImpl::Dock), 0x170, nullptr },
	{ FARPROC(HkIServerImpl::SubmitChat), 0xFFFFFFF8, nullptr },
};

