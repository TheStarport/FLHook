#include "Global.hpp"
#include "HkIClientImpl.inl"
#include "HkIServerImpl.inl"

bool HkIClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(uint clientID, XFireWeaponInfo& fwi)
{
	AddLog(
		LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(\n\tuint clientID = %u\n\tXFireWeaponInfo& fwi = %s\n)",
	    clientID, ToLogString(fwi));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, clientID, fwi);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_FIREWEAPON(clientID, fwi);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, clientID, fwi);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(uint clientID, XActivateEquip& aq)
{
	AddLog(LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(\n\tuint clientID = %u\n\tXActivateEquip& aq = %s\n)",
	    clientID, ToLogString(aq));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, clientID, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_ACTIVATEEQUIP(clientID, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, clientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(uint clientID, XActivateCruise& aq)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(\n\tuint clientID = %u\n\tXActivateCruise& aq = %s\n)",
	    clientID, ToLogString(aq));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, clientID, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_ACTIVATECRUISE(clientID, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, clientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(uint clientID, XActivateThrusters& aq)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(\n\tuint clientID = %u\n\tXActivateThrusters& aq = "
	    L"%s\n)",
	    clientID, ToLogString(aq));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, clientID, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(clientID, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, clientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SETTARGET(uint clientID, XSetTarget& st)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_COMMON_SETTARGET(\n\tuint clientID = %u\n\tXSetTarget& st = %s\n)",
	    clientID, ToLogString(st));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SETTARGET(clientID, st);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_6(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_6(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientID,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_6(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(uint clientID, XGoTradelane& tl)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(\n\tuint clientID = %u\n\tXGoTradelane& tl = %s\n)",
	    clientID, ToLogString(tl));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_GOTRADELANE(clientID, tl);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(
    uint clientID, uint shipID, uint archTradelane1, uint archTradelane2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(\n\tuint clientID = %u\n\tuint shipID = %u\n\tuint "
	    L"archTradelane1 = %u\n\tuint archTradelane2 = %u\n)",
	    clientID, shipID, archTradelane1, archTradelane2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_STOPTRADELANE(clientID, shipID, archTradelane1, archTradelane2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(uint clientID, XJettisonCargo& jc)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(\n\tuint clientID = %u\n\tXJettisonCargo& jc = %s\n)",
	    clientID, ToLogString(jc));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_JETTISONCARGO(clientID, jc);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::SendPacket(uint clientID, void* _genArg1)
{
	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = SendPacket(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Startup(uint _genArg1, uint _genArg2)
{
	HkIClientImpl__Startup__Inner(_genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Startup(_genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::nullsub(uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::nullsub(\n\tuint _genArg1 = %u\n)", _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		nullsub(_genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientID, ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_LOGINRESPONSE(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientID, ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_CHARACTERINFO(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 "
	    L"= %s\n)",
	    clientID, ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_CHARSELECTVERIFIED(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::Shutdown()
{
	CALL_CLIENT_PREAMBLE
	{
		Shutdown();
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::CDPClientProxy__Disconnect(uint clientID)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::CDPClientProxy__Disconnect(\n\tuint clientID = %u\n)", clientID);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = CDPClientProxy__Disconnect(clientID);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

uint HkIClientImpl::CDPClientProxy__GetSendQSize(uint clientID)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::CDPClientProxy__GetSendQSize(\n\tuint clientID = %u\n)", clientID);

	uint retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = CDPClientProxy__GetSendQSize(clientID);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

uint HkIClientImpl::CDPClientProxy__GetSendQBytes(uint clientID)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::CDPClientProxy__GetSendQBytes(\n\tuint clientID = %u\n)", clientID);

	uint retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = CDPClientProxy__GetSendQBytes(clientID);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

double HkIClientImpl::CDPClientProxy__GetLinkSaturation(uint clientID)
{
	auto [retVal, skip] =
	    CallPluginsBefore<double>(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, clientID);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = CDPClientProxy__GetLinkSaturation(clientID);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, clientID);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(uint clientID, uint shipArch)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(\n\tuint clientID = %u\n\tuint shipArch = %u\n)",
	    clientID, shipArch);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, clientID, shipArch);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETSHIPARCH(clientID, shipArch);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, clientID, shipArch);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(uint clientID, float status)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(\n\tuint clientID = %u\n\tfloat status = %f\n)",
	    clientID, status);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, clientID, status);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETHULLSTATUS(clientID, status);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, clientID, status);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 "
	    L"= %s\n)",
	    clientID, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, clientID, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(clientID, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, clientID, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientID, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, clientID, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETEQUIPMENT(clientID, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, clientID, _genArg1);

	return retVal;
}

void HkIClientImpl::unknown_26(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_26(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_26(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETADDITEM(
    uint clientID, FLPACKET_UNKNOWN& _genArg1, FLPACKET_UNKNOWN& _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETADDITEM(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n\tFLPACKET_UNKNOWN& _genArg2 = %s\n)",
	    clientID, ToLogString(_genArg1), ToLogString(_genArg2));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, clientID, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETADDITEM(clientID, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, clientID, _genArg1, _genArg2);

	return retVal;
}

void HkIClientImpl::unknown_28(uint clientID, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::unknown_28(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientID, _genArg1, _genArg2, _genArg3);

	CALL_CLIENT_PREAMBLE
	{
		unknown_28(clientID, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, clientID, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETSTARTROOM(clientID, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, clientID, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFDESTROYCHARACTER(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFUPDATECHAR(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(uint clientID, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)",
	    clientID, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(\n\tuint clientID = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(uint clientID, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(\n\tuint clientID = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientID, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_36(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_36(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_36(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_37(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_37(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_37(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(uint clientID, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(\n\tuint clientID = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientID, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(\n\tuint clientID = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(\n\tuint clientID = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(uint clientID, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(\n\tuint clientID = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientID, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(\n\tuint clientID = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(uint clientID, uint reason)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(\n\tuint clientID = %u\n\tuint reason = %u\n)",
	    clientID, reason);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(clientID, reason);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_44(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_44(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_44(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(\n\tuint clientID = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(uint clientID, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(\n\tuint clientID = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientID, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(uint clientID, FLPACKET_CREATESOLAR& solar)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(\n\tuint clientID = %u\n\tFLPACKET_CREATESOLAR& solar = "
	    L"%s\n)",
	    clientID, ToLogString(solar));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, clientID, solar);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATESOLAR(clientID, solar);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, clientID, solar);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATESHIP(uint clientID, FLPACKET_CREATESHIP& ship)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATESHIP(\n\tuint clientID = %u\n\tFLPACKET_CREATESHIP& ship = %s\n)",
	    clientID, ToLogString(ship));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, clientID, ship);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATESHIP(clientID, ship);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, clientID, ship);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATELOOT(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATELOOT(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientID, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, clientID, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATELOOT(clientID, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, clientID, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATEMINE(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATEMINE(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientID, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, clientID, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATEMINE(clientID, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, clientID, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientID, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, clientID, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATEGUIDED(clientID, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, clientID, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientID, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, clientID, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATECOUNTER(clientID, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, clientID, _genArg1);

	return retVal;
}

void HkIClientImpl::unknown_53(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_53(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientID,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_53(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_54(uint clientID, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::unknown_54(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientID, _genArg1, _genArg2, _genArg3);

	CALL_CLIENT_PREAMBLE
	{
		unknown_54(clientID, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_UPDATEOBJECT(uint clientID, SSPObjUpdateInfo& update)
{
	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, clientID, update);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_UPDATEOBJECT(clientID, update);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, clientID, update);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(uint clientID, FLPACKET_DESTROYOBJECT& destroy)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(\n\tuint clientID = %u\n\tFLPACKET_DESTROYOBJECT& destroy "
	    L"= %s\n)",
	    clientID, ToLogString(destroy));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, clientID, destroy);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_DESTROYOBJECT(clientID, destroy);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, clientID, destroy);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(uint clientID, XActivateEquip& aq)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(\n\tuint clientID = %u\n\tXActivateEquip& aq = %s\n)",
	    clientID, ToLogString(aq));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, clientID, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_ACTIVATEOBJECT(clientID, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, clientID, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientID, ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientID, ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LAND(uint clientID, FLPACKET_LAND& land)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_LAND(\n\tuint clientID = %u\n\tFLPACKET_LAND& land = %s\n)",
	    clientID, ToLogString(land));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_LAND(clientID, land);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LAUNCH(uint clientID, FLPACKET_LAUNCH& launch)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_LAUNCH(\n\tuint clientID = %u\n\tFLPACKET_LAUNCH& launch = %s\n)",
	    clientID, ToLogString(launch));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, clientID, launch);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_LAUNCH(clientID, launch);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, clientID, launch);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(uint clientID, bool response, uint shipID)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(\n\tuint clientID = %u\n\tbool response = "
	    L"%d\n\tuint shipID = %u\n)",
	    clientID, response, shipID);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, clientID, response, shipID);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(clientID, response, shipID);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, clientID, response, shipID);

	return retVal;
}

void HkIClientImpl::unknown_63(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_63(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientID,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_63(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(uint clientID, uint objID, DamageList& dmgList)
{
	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_DAMAGEOBJECT(clientID, objID, dmgList);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(uint clientID, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)",
	    clientID, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_ITEMTRACTORED(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_USE_ITEM(uint clientID, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_USE_ITEM(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)",
	    clientID, _genArg1);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, clientID, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_USE_ITEM(clientID, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, clientID, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(uint clientID, FLPACKET_SETREPUTATION& rep)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(\n\tuint clientID = %u\n\tFLPACKET_SETREPUTATION& rep = "
	    L"%s\n)",
	    clientID, ToLogString(rep));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, clientID, rep);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETREPUTATION(clientID, rep);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, clientID, rep);

	return retVal;
}

void HkIClientImpl::unknown_68(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_68(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientID,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_68(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SENDCOMM(
    uint clientID, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6,
    uint _genArg7, uint _genArg8, uint _genArg9, uint _genArg10, uint _genArg11, uint _genArg12, uint _genArg13,
    uint _genArg14, uint _genArg15, uint _genArg16, uint _genArg17, uint _genArg18, uint _genArg19, uint _genArg20,
    uint _genArg21, uint _genArg22)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SENDCOMM(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = "
	    L"%u\n\tuint _genArg3 = %u\n\tuint _genArg4 = %u\n\tuint _genArg5 = %u\n\tuint _genArg6 = %u\n\tuint _genArg7 "
	    L"= %u\n\tuint _genArg8 = %u\n\tuint _genArg9 = %u\n\tuint _genArg10 = %u\n\tuint _genArg11 = %u\n\tuint "
	    L"_genArg12 = %u\n\tuint _genArg13 = %u\n\tuint _genArg14 = %u\n\tuint _genArg15 = %u\n\tuint _genArg16 = "
	    L"%u\n\tuint _genArg17 = %u\n\tuint _genArg18 = %u\n\tuint _genArg19 = %u\n\tuint _genArg20 = %u\n\tuint "
	    L"_genArg21 = %u\n\tuint _genArg22 = %u\n)",
	    clientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10,
	    _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20,
	    _genArg21, _genArg22);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM, clientID, _genArg1, _genArg2, _genArg3, _genArg4,
	    _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14,
	    _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SENDCOMM(
			    clientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9,
			    _genArg10, _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18,
			    _genArg19, _genArg20, _genArg21, _genArg22);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM, clientID, _genArg1, _genArg2, _genArg3, _genArg4,
	    _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14,
	    _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);

	return retVal;
}

void HkIClientImpl::unknown_70(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_70(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_70(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 "
	    L"= %s\n)",
	    clientID, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, clientID, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(clientID, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, clientID, _genArg1);

	return retVal;
}

void HkIClientImpl::unknown_72(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_72(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientID,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_72(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(uint clientID, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)",
	    clientID, _genArg1);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, clientID, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(clientID, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, clientID, _genArg1);

	return retVal;
}

void HkIClientImpl::unknown_74(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_74(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientID,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_74(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_75(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_75(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_75(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MARKOBJ(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MARKOBJ(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = "
	    L"%u\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_MARKOBJ(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_77(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_77(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_77(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETCASH(uint clientID, uint cash)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_SETCASH(\n\tuint clientID = %u\n\tuint cash = %u\n)", clientID,
	    cash);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, clientID, cash);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETCASH(clientID, cash);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, clientID, cash);

	return retVal;
}

void HkIClientImpl::unknown_79(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_79(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_79(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_80(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_80(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_80(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_81(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_81(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_81(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_82(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_82(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_82(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_83(uint clientID, char* _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_83(\n\tuint clientID = %u\n\tchar* _genArg1 = %s\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_83(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(
    uint clientID, uint shipID, uint flag, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(\n\tuint clientID = %u\n\tuint shipID = %u\n\tuint flag "
	    L"= %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientID, shipID, flag, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_REQUEST_RETURNED(clientID, shipID, flag, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_85(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_85(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientID,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_85(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_86(uint clientID, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::unknown_86(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientID, _genArg1, _genArg2, _genArg3);

	CALL_CLIENT_PREAMBLE
	{
		unknown_86(clientID, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_BURNFUSE(uint clientID, FLPACKET_BURNFUSE& burnFuse)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_BURNFUSE(\n\tuint clientID = %u\n\tFLPACKET_BURNFUSE& burnFuse = %s\n)",
	    clientID, ToLogString(burnFuse));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, clientID, burnFuse);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_BURNFUSE(clientID, burnFuse);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, clientID, burnFuse);

	return retVal;
}

void HkIClientImpl::unknown_89(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_89(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientID,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_89(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_90(uint clientID)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_90(\n\tuint clientID = %u\n)", clientID);

	CALL_CLIENT_PREAMBLE
	{
		unknown_90(clientID);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_91(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_91(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_91(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(uint clientID, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_WEAPON_GROUP(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(uint clientID, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_VISITED_STATE(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(uint clientID, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_REQUEST_BEST_PATH(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(uint clientID, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(\n\tuint clientID = %u\n\tuchar* _genArg1 = "
	    L"%p\n\tint _genArg2 = %d\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_96(uint clientID, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::unknown_96(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientID, _genArg1, _genArg2, _genArg3);

	CALL_CLIENT_PREAMBLE
	{
		unknown_96(clientID, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(uint clientID, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(\n\tuint clientID = %u\n\tuchar* _genArg1 = "
	    L"%p\n\tint _genArg2 = %d\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(uint clientID, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_MISSION_LOG(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(uint clientID, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(\n\tuint clientID = %u\n\tuchar* _genArg1 = "
	    L"%p\n\tint _genArg2 = %d\n)",
	    clientID, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_INTERFACE_STATE(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_100(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_100(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_100(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_101(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_101(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientID,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_101(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_102(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_102(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_102(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_103(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_103(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_103(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_104(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_104(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_104(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_105(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_105(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_105(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_106(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_106(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_106(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_107(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_107(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_107(clientID, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(uint clientID, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)",
	    clientID, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_PLAYER_TRADE(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_109(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_109(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_109(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 "
	    L"= %u\n)",
	    clientID, _genArg1, _genArg2);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, clientID, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SCANNOTIFY(clientID, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, clientID, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(uint clientID, wchar_t* _genArg1, uint _genArg2, char _genArg3)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(\n\tuint clientID = %u\n\twchar_t* _genArg1 = %p\n\tuint "
	    L"_genArg2 = %u\n\tchar _genArg3 = %s\n)",
	    clientID, _genArg1, _genArg2, ToLogString(_genArg3));

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, clientID, _genArg1, _genArg2, _genArg3);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_PLAYERLIST(clientID, _genArg1, _genArg2, _genArg3);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, clientID, _genArg1, _genArg2, _genArg3);

	return retVal;
}

void HkIClientImpl::unknown_112(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_112(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_112(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(uint clientID)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(\n\tuint clientID = %u\n)", clientID);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, clientID);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_PLAYERLIST_2(clientID);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, clientID);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, clientID, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_6(clientID, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, clientID, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, clientID, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_7(clientID, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, clientID, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(uint clientID, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(\n\tuint clientID = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientID, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, clientID, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE(clientID, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, clientID, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, clientID, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_2(clientID, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, clientID, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(uint clientID, uint targetID, uint rank)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(\n\tuint clientID = %u\n\tuint targetID = %u\n\tuint "
	    L"rank = %u\n)",
	    clientID, targetID, rank);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, clientID, targetID, rank);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_3(clientID, targetID, rank);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, clientID, targetID, rank);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, clientID, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_4(clientID, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, clientID, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(uint clientID, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientID, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, clientID, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_5(clientID, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, clientID, _genArg1, _genArg2);

	return retVal;
}

void HkIClientImpl::unknown_121(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_121(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_121(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(uint clientID, uint shipID, Vector& formationOffset)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(\n\tuint clientID = %u\n\tuint shipID = %u\n\tVector& "
	    L"formationOffset = %s\n)",
	    clientID, shipID, ToLogString(formationOffset));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_FORMATION_UPDATE(clientID, shipID, formationOffset);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_123(
    uint clientID, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::unknown_123(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n\tuint _genArg4 = %u\n\tuint _genArg5 = %u\n\tuint _genArg6 = %u\n)",
	    clientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

	CALL_CLIENT_PREAMBLE
	{
		unknown_123(clientID, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_124(uint clientID)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_124(\n\tuint clientID = %u\n)", clientID);

	CALL_CLIENT_PREAMBLE
	{
		unknown_124(clientID);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_125(uint clientID, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_125(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_125(clientID, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

int HkIClientImpl::unknown_126(char* _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_126(\n\tchar* _genArg1 = %s\n)", _genArg1);

	int retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = unknown_126(_genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

namespace HkIServerImpl
{
	void __stdcall FireWeapon(uint clientID, XFireWeaponInfo const& fwi)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"FireWeapon(\n\tuint clientID = %u\n\tXFireWeaponInfo const& fwi = %s\n)", clientID,
		    ToLogString(fwi));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__FireWeapon, clientID, fwi);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.FireWeapon(clientID, fwi);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__FireWeapon, clientID, fwi);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ActivateEquip(uint clientID, XActivateEquip const& aq)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ActivateEquip(\n\tuint clientID = %u\n\tXActivateEquip const& aq = %s\n)", clientID,
		    ToLogString(aq));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateEquip, clientID, aq);

		CHECK_FOR_DISCONNECT;

		ActivateEquip__Inner(clientID, aq);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ActivateEquip(clientID, aq);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateEquip, clientID, aq);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ActivateCruise(uint clientID, XActivateCruise const& ac)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ActivateCruise(\n\tuint clientID = %u\n\tXActivateCruise const& ac = %s\n)", clientID,
		    ToLogString(ac));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateCruise, clientID, ac);

		CHECK_FOR_DISCONNECT;

		ActivateCruise__Inner(clientID, ac);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ActivateCruise(clientID, ac);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateCruise, clientID, ac);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ActivateThrusters(uint clientID, XActivateThrusters const& at)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ActivateThrusters(\n\tuint clientID = %u\n\tXActivateThrusters const& at = %s\n)", clientID,
		    ToLogString(at));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateThrusters, clientID, at);

		CHECK_FOR_DISCONNECT;

		ActivateThrusters__Inner(clientID, at);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ActivateThrusters(clientID, at);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateThrusters, clientID, at);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetTarget(uint clientID, XSetTarget const& st)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SetTarget(\n\tuint clientID = %u\n\tXSetTarget const& st = %s\n)", clientID, ToLogString(st));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTarget, clientID, st);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetTarget(clientID, st);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetTarget, clientID, st);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall TractorObjects(uint clientID, XTractorObjects const& to)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"TractorObjects(\n\tuint clientID = %u\n\tXTractorObjects const& to = %s\n)", clientID,
		    ToLogString(to));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TractorObjects, clientID, to);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.TractorObjects(clientID, to);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__TractorObjects, clientID, to);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall GoTradelane(uint clientID, XGoTradelane const& gt)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GoTradelane(\n\tuint clientID = %u\n\tXGoTradelane const& gt = %s\n)", clientID, ToLogString(gt));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GoTradelane, clientID, gt);

		GoTradelane__Inner(clientID, gt);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GoTradelane(clientID, gt);
			}
			CALL_SERVER_POSTAMBLE(GoTradelane__Catch(clientID, gt), );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GoTradelane, clientID, gt);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall StopTradelane(uint clientID, uint shipID, uint tradelaneRing1, uint tradelaneRing2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"StopTradelane(\n\tuint clientID = %u\n\tuint shipID = %u\n\tuint tradelaneRing1 = %u\n\tuint "
		    L"tradelaneRing2 = %u\n)",
		    clientID, shipID, tradelaneRing1, tradelaneRing2);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__StopTradelane, clientID, shipID, tradelaneRing1, tradelaneRing2);

		StopTradelane__Inner(clientID, shipID, tradelaneRing1, tradelaneRing2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.StopTradelane(clientID, shipID, tradelaneRing1, tradelaneRing2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__StopTradelane, clientID, shipID, tradelaneRing1, tradelaneRing2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall JettisonCargo(uint clientID, XJettisonCargo const& jc)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"JettisonCargo(\n\tuint clientID = %u\n\tXJettisonCargo const& jc = %s\n)", clientID,
		    ToLogString(jc));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JettisonCargo, clientID, jc);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.JettisonCargo(clientID, jc);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__JettisonCargo, clientID, jc);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	bool __stdcall Startup(SStartupInfo const& si)
	{
		Startup__Inner(si);

		auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IServerImpl__Startup, si);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				retVal = Server.Startup(si);
			}
			CALL_SERVER_POSTAMBLE(true, bool());
		}
		Startup__InnerAfter(si);

		CallPluginsAfter(HookedCall::IServerImpl__Startup, si);

		return retVal;
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall Shutdown()
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"Shutdown()");

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Shutdown);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.Shutdown();
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		Shutdown__InnerAfter();
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	int __stdcall Update()
	{
		auto [retVal, skip] = CallPluginsBefore<int>(HookedCall::IServerImpl__Update);

		Update__Inner();

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				retVal = Server.Update();
			}
			CALL_SERVER_POSTAMBLE(true, int());
		}

		CallPluginsAfter(HookedCall::IServerImpl__Update);

		return retVal;
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall DisConnect(uint clientID, EFLConnection conn)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"DisConnect(\n\tuint clientID = %u\n\tEFLConnection conn = %s\n)", clientID, ToLogString(conn));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DisConnect, clientID, conn);

		DisConnect__Inner(clientID, conn);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.DisConnect(clientID, conn);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DisConnect, clientID, conn);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall OnConnect(uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"OnConnect(\n\tuint clientID = %u\n)", clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__OnConnect, clientID);

		bool innerCheck = OnConnect__Inner(clientID);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.OnConnect(clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		OnConnect__InnerAfter(clientID);

		CallPluginsAfter(HookedCall::IServerImpl__OnConnect, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall Login(SLoginInfo const& li, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"Login(\n\tSLoginInfo const& li = %s\n\tuint clientID = %u\n)", ToLogString(li), clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Login, li, clientID);

		if (!skip && Login__InnerBefore(li, clientID))
		{
			CALL_SERVER_PREAMBLE
			{
				Server.Login(li, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		Login__InnerAfter(li, clientID);

		CallPluginsAfter(HookedCall::IServerImpl__Login, li, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall CharacterInfoReq(uint clientID, bool _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"CharacterInfoReq(\n\tuint clientID = %u\n\tbool _genArg1 = %d\n)", clientID, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterInfoReq, clientID, _genArg1);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = CharacterInfoReq__Inner(clientID, _genArg1);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.CharacterInfoReq(clientID, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(CharacterInfoReq__Catch(clientID, _genArg1), );
		}

		CallPluginsAfter(HookedCall::IServerImpl__CharacterInfoReq, clientID, _genArg1);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall CharacterSelect(CHARACTER_ID const& cid, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"CharacterSelect(\n\tCHARACTER_ID const& cid = %s\n\tuint clientID = %u\n)", ToLogString(cid),
		    clientID);

		std::string charName = cid.szCharFilename;
		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__CharacterSelect, charName, clientID);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = CharacterSelect__Inner(cid, clientID);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.CharacterSelect(cid, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		CharacterSelect__InnerAfter(cid, clientID);

		CallPluginsAfter(HookedCall::IServerImpl__CharacterSelect, charName, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall CreateNewCharacter(SCreateCharacterInfo const& _genArg1, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"CreateNewCharacter(\n\tSCreateCharacterInfo const& _genArg1 = %s\n\tuint clientID = %u\n)",
		    ToLogString(_genArg1), clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CreateNewCharacter, _genArg1, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.CreateNewCharacter(_genArg1, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__CreateNewCharacter, _genArg1, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall DestroyCharacter(CHARACTER_ID const& _genArg1, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"DestroyCharacter(\n\tCHARACTER_ID const& _genArg1 = %s\n\tuint clientID = %u\n)",
		    ToLogString(_genArg1), clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DestroyCharacter, _genArg1, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.DestroyCharacter(_genArg1, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DestroyCharacter, _genArg1, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqShipArch(uint archID, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqShipArch(\n\tuint archID = %u\n\tuint clientID = %u\n)", archID, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqShipArch, archID, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqShipArch(archID, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqShipArch, archID, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqHullStatus(float status, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqHullStatus(\n\tfloat status = %f\n\tuint clientID = %u\n)", status, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqHullStatus, status, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqHullStatus(status, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqHullStatus, status, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqCollisionGroups(st6::list<CollisionGroupDesc> const& collisionGroups, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"ReqCollisionGroups(\n\tst6::list<CollisionGroupDesc> const& collisionGroups = %s\n\tuint clientID = "
		    L"%u\n)",
		    ToLogString(collisionGroups), clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqCollisionGroups, collisionGroups, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqCollisionGroups(collisionGroups, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqCollisionGroups, collisionGroups, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqEquipment(EquipDescList const& edl, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ReqEquipment(\n\tEquipDescList const& edl = %s\n\tuint clientID = %u\n)", ToLogString(edl),
		    clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqEquipment, edl, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqEquipment(edl, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqEquipment, edl, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqAddItem(uint goodID, char const* hardpoint, int count, float status, bool mounted, uint iClientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"ReqAddItem(\n\tuint goodID = %u\n\tchar const* hardpoint = %p\n\tint count = %d\n\tfloat status = "
		    L"%f\n\tbool mounted = %d\n\tuint iClientID = %u\n)",
		    goodID, hardpoint, count, status, mounted, iClientID);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__ReqAddItem, goodID, hardpoint, count, status, mounted, iClientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqAddItem(goodID, hardpoint, count, status, mounted, iClientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqAddItem, goodID, hardpoint, count, status, mounted, iClientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqRemoveItem(ushort slotID, int count, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ReqRemoveItem(\n\tushort slotID = %u\n\tint count = %d\n\tuint clientID = %u\n)", slotID, count,
		    clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqRemoveItem, slotID, count, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqRemoveItem(slotID, count, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqRemoveItem, slotID, count, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqModifyItem(
	    ushort slotID, char const* hardpoint, int count, float status, bool mounted, uint iClientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"ReqModifyItem(\n\tushort slotID = %u\n\tchar const* hardpoint = %p\n\tint count = %d\n\tfloat status = "
		    L"%f\n\tbool mounted = %d\n\tuint iClientID = %u\n)",
		    slotID, hardpoint, count, status, mounted, iClientID);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__ReqModifyItem, slotID, hardpoint, count, status, mounted, iClientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqModifyItem(slotID, hardpoint, count, status, mounted, iClientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqModifyItem, slotID, hardpoint, count, status, mounted, iClientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqSetCash(int cash, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqSetCash(\n\tint cash = %d\n\tuint clientID = %u\n)", cash, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqSetCash, cash, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqSetCash(cash, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqSetCash, cash, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqChangeCash(int cashAdd, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqChangeCash(\n\tint cashAdd = %d\n\tuint clientID = %u\n)", cashAdd, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqChangeCash, cashAdd, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqChangeCash(cashAdd, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqChangeCash, cashAdd, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall BaseEnter(uint baseID, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"BaseEnter(\n\tuint baseID = %u\n\tuint clientID = %u\n)", baseID, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseEnter, baseID, clientID);

		CHECK_FOR_DISCONNECT;

		BaseEnter__Inner(baseID, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.BaseEnter(baseID, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		BaseEnter__InnerAfter(baseID, clientID);

		CallPluginsAfter(HookedCall::IServerImpl__BaseEnter, baseID, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall BaseExit(uint baseID, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"BaseExit(\n\tuint baseID = %u\n\tuint clientID = %u\n)", baseID, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseExit, baseID, clientID);

		CHECK_FOR_DISCONNECT;

		BaseExit__Inner(baseID, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.BaseExit(baseID, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		BaseExit__InnerAfter(baseID, clientID);

		CallPluginsAfter(HookedCall::IServerImpl__BaseExit, baseID, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall LocationEnter(uint locationID, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"LocationEnter(\n\tuint locationID = %u\n\tuint clientID = %u\n)", locationID, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationEnter, locationID, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.LocationEnter(locationID, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationEnter, locationID, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall LocationExit(uint locationID, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"LocationExit(\n\tuint locationID = %u\n\tuint clientID = %u\n)", locationID, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationExit, locationID, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.LocationExit(locationID, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationExit, locationID, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall BaseInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"BaseInfoRequest(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tbool _genArg3 = %d\n)",
		    _genArg1, _genArg2, _genArg3);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseInfoRequest, _genArg1, _genArg2, _genArg3);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.BaseInfoRequest(_genArg1, _genArg2, _genArg3);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__BaseInfoRequest, _genArg1, _genArg2, _genArg3);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall LocationInfoRequest(unsigned int _genArg1, unsigned int _genArg2, bool _genArg3)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"LocationInfoRequest(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tbool _genArg3 = "
		    L"%d\n)",
		    _genArg1, _genArg2, _genArg3);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationInfoRequest, _genArg1, _genArg2, _genArg3);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.LocationInfoRequest(_genArg1, _genArg2, _genArg3);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationInfoRequest, _genArg1, _genArg2, _genArg3);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall GFObjSelect(unsigned int _genArg1, unsigned int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GFObjSelect(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n)", _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFObjSelect, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFObjSelect(_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFObjSelect, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall GFGoodVaporized(SGFGoodVaporizedInfo const& gvi, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GFGoodVaporized(\n\tSGFGoodVaporizedInfo const& gvi = %s\n\tuint clientID = %u\n)",
		    ToLogString(gvi), clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodVaporized, gvi, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFGoodVaporized(gvi, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodVaporized, gvi, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall MissionResponse(unsigned int _genArg1, unsigned long _genArg2, bool _genArg3, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"MissionResponse(\n\tunsigned int _genArg1 = %u\n\tunsigned long _genArg2 = %u\n\tbool _genArg3 = "
		    L"%d\n\tuint clientID = %u\n)",
		    _genArg1, _genArg2, _genArg3, clientID);

		auto skip =
		    CallPluginsBefore<void>(HookedCall::IServerImpl__MissionResponse, _genArg1, _genArg2, _genArg3, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.MissionResponse(_genArg1, _genArg2, _genArg3, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__MissionResponse, _genArg1, _genArg2, _genArg3, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall TradeResponse(unsigned char const* _genArg1, int _genArg2, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"TradeResponse(\n\tunsigned char const* _genArg1 = %p\n\tint _genArg2 = %d\n\tuint clientID = %u\n)",
		    _genArg1, _genArg2, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TradeResponse, _genArg1, _genArg2, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.TradeResponse(_genArg1, _genArg2, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__TradeResponse, _genArg1, _genArg2, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall GFGoodBuy(SGFGoodBuyInfo const& _genArg1, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GFGoodBuy(\n\tSGFGoodBuyInfo const& _genArg1 = %s\n\tuint clientID = %u\n)", ToLogString(_genArg1),
		    clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodBuy, _genArg1, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFGoodBuy(_genArg1, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodBuy, _genArg1, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall GFGoodSell(SGFGoodSellInfo const& _genArg1, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GFGoodSell(\n\tSGFGoodSellInfo const& _genArg1 = %s\n\tuint clientID = %u\n)",
		    ToLogString(_genArg1), clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodSell, _genArg1, clientID);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = GFGoodSell__Inner(_genArg1, clientID);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFGoodSell(_genArg1, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodSell, _genArg1, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SystemSwitchOutComplete(uint shipID, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SystemSwitchOutComplete(\n\tuint shipID = %u\n\tuint clientID = %u\n)", shipID, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SystemSwitchOutComplete, shipID, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SystemSwitchOutComplete(shipID, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		SystemSwitchOutComplete__InnerAfter(shipID, clientID);

		CallPluginsAfter(HookedCall::IServerImpl__SystemSwitchOutComplete, shipID, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall PlayerLaunch(uint shipID, uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"PlayerLaunch(\n\tuint shipID = %u\n\tuint clientID = %u\n)", shipID, clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PlayerLaunch, shipID, clientID);

		CHECK_FOR_DISCONNECT;

		PlayerLaunch__Inner(shipID, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.PlayerLaunch(shipID, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		PlayerLaunch__InnerAfter(shipID, clientID);

		CallPluginsAfter(HookedCall::IServerImpl__PlayerLaunch, shipID, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall LaunchComplete(uint baseID, uint shipID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"LaunchComplete(\n\tuint baseID = %u\n\tuint shipID = %u\n)", baseID, shipID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LaunchComplete, baseID, shipID);

		LaunchComplete__Inner(baseID, shipID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.LaunchComplete(baseID, shipID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LaunchComplete, baseID, shipID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall JumpInComplete(uint systemID, uint shipID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"JumpInComplete(\n\tuint systemID = %u\n\tuint shipID = %u\n)", systemID, shipID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JumpInComplete, systemID, shipID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.JumpInComplete(systemID, shipID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		JumpInComplete__InnerAfter(systemID, shipID);

		CallPluginsAfter(HookedCall::IServerImpl__JumpInComplete, systemID, shipID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall Hail(unsigned int _genArg1, unsigned int _genArg2, unsigned int _genArg3)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"Hail(\n\tunsigned int _genArg1 = %u\n\tunsigned int _genArg2 = %u\n\tunsigned int _genArg3 = %u\n)",
		    _genArg1, _genArg2, _genArg3);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Hail, _genArg1, _genArg2, _genArg3);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.Hail(_genArg1, _genArg2, _genArg3);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__Hail, _genArg1, _genArg2, _genArg3);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SPObjUpdate(SSPObjUpdateInfo const& ui, uint clientID)
	{
		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjUpdate, ui, clientID);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = SPObjUpdate__Inner(ui, clientID);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPObjUpdate(ui, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPObjUpdate, ui, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SPMunitionCollision(SSPMunitionCollisionInfo const& mci, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SPMunitionCollision(\n\tSSPMunitionCollisionInfo const& mci = %s\n\tuint clientID = %u\n)",
		    ToLogString(mci), clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPMunitionCollision, mci, clientID);

		CHECK_FOR_DISCONNECT;

		SPMunitionCollision__Inner(mci, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPMunitionCollision(mci, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPMunitionCollision, mci, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SPObjCollision(SSPObjCollisionInfo const& oci, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SPObjCollision(\n\tSSPObjCollisionInfo const& oci = %s\n\tuint clientID = %u\n)", ToLogString(oci),
		    clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjCollision, oci, clientID);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPObjCollision(oci, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPObjCollision, oci, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SPRequestUseItem(SSPUseItem const& ui, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SPRequestUseItem(\n\tSSPUseItem const& ui = %s\n\tuint clientID = %u\n)", ToLogString(ui),
		    clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestUseItem, ui, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPRequestUseItem(ui, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPRequestUseItem, ui, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SPRequestInvincibility(uint shipID, bool enable, InvincibilityReason reason, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"SPRequestInvincibility(\n\tuint shipID = %u\n\tbool enable = %d\n\tInvincibilityReason reason = "
		    L"%s\n\tuint clientID = %u\n)",
		    shipID, enable, ToLogString(reason), clientID);

		auto skip =
		    CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestInvincibility, shipID, enable, reason, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPRequestInvincibility(shipID, enable, reason, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPRequestInvincibility, shipID, enable, reason, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestEvent(
	    int eventType, uint shipID, uint dockTarget, uint _genArg1, ulong _genArg2, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"RequestEvent(\n\tint eventType = %d\n\tuint shipID = %u\n\tuint dockTarget = %u\n\tuint _genArg1 = "
		    L"%u\n\tulong _genArg2 = %u\n\tuint clientID = %u\n)",
		    eventType, shipID, dockTarget, _genArg1, _genArg2, clientID);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__RequestEvent, eventType, shipID, dockTarget, _genArg1, _genArg2, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestEvent(eventType, shipID, dockTarget, _genArg1, _genArg2, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(
		    HookedCall::IServerImpl__RequestEvent, eventType, shipID, dockTarget, _genArg1, _genArg2, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestCancel(int eventType, uint shipID, uint _genArg1, ulong _genArg2, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"RequestCancel(\n\tint eventType = %d\n\tuint shipID = %u\n\tuint _genArg1 = %u\n\tulong _genArg2 = "
		    L"%u\n\tuint clientID = %u\n)",
		    eventType, shipID, _genArg1, _genArg2, clientID);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__RequestCancel, eventType, shipID, _genArg1, _genArg2, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestCancel(eventType, shipID, _genArg1, _genArg2, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestCancel, eventType, shipID, _genArg1, _genArg2, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall MineAsteroid(uint systemID, Vector const& pos, uint crateID, uint lootID, uint count, uint clientID)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"MineAsteroid(\n\tuint systemID = %u\n\tVector const& pos = %s\n\tuint crateID = %u\n\tuint lootID = "
		    L"%u\n\tuint count = %u\n\tuint clientID = %u\n)",
		    systemID, ToLogString(pos), crateID, lootID, count, clientID);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__MineAsteroid, systemID, pos, crateID, lootID, count, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.MineAsteroid(systemID, pos, crateID, lootID, count, clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__MineAsteroid, systemID, pos, crateID, lootID, count, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestCreateShip(uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"RequestCreateShip(\n\tuint clientID = %u\n)", clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCreateShip, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestCreateShip(clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestCreateShip, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SPScanCargo(uint const& _genArg1, uint const& _genArg2, uint _genArg3)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SPScanCargo(\n\tuint const& _genArg1 = %s\n\tuint const& _genArg2 = %s\n\tuint _genArg3 = %u\n)",
		    ToLogString(_genArg1), ToLogString(_genArg2), _genArg3);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPScanCargo, _genArg1, _genArg2, _genArg3);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPScanCargo(_genArg1, _genArg2, _genArg3);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPScanCargo, _genArg1, _genArg2, _genArg3);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetManeuver(uint clientID, XSetManeuver const& sm)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SetManeuver(\n\tuint clientID = %u\n\tXSetManeuver const& sm = %s\n)", clientID, ToLogString(sm));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetManeuver, clientID, sm);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetManeuver(clientID, sm);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetManeuver, clientID, sm);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall InterfaceItemUsed(uint _genArg1, uint _genArg2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"InterfaceItemUsed(\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)", _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InterfaceItemUsed, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.InterfaceItemUsed(_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__InterfaceItemUsed, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall AbortMission(uint clientID, uint _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"AbortMission(\n\tuint clientID = %u\n\tuint _genArg1 = %u\n)", clientID, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AbortMission, clientID, _genArg1);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.AbortMission(clientID, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AbortMission, clientID, _genArg1);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetWeaponGroup(uint clientID, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SetWeaponGroup(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)", clientID,
		    _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetWeaponGroup, clientID, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetWeaponGroup(clientID, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetWeaponGroup, clientID, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetVisitedState(uint clientID, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SetVisitedState(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)", clientID,
		    _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetVisitedState, clientID, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetVisitedState(clientID, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetVisitedState, clientID, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestBestPath(uint clientID, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"RequestBestPath(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)", clientID,
		    _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestBestPath, clientID, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestBestPath(clientID, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestBestPath, clientID, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestPlayerStats(uint clientID, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"RequestPlayerStats(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientID, _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestPlayerStats, clientID, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestPlayerStats(clientID, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestPlayerStats, clientID, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall PopupDialog(uint clientID, uint buttonClicked)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"PopupDialog(\n\tuint clientID = %u\n\tuint buttonClicked = %u\n)", clientID, buttonClicked);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PopupDialog, clientID, buttonClicked);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.PopUpDialog(clientID, buttonClicked);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__PopupDialog, clientID, buttonClicked);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestGroupPositions(uint clientID, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"RequestGroupPositions(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientID, _genArg1, _genArg2);

		auto skip =
		    CallPluginsBefore<void>(HookedCall::IServerImpl__RequestGroupPositions, clientID, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestGroupPositions(clientID, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestGroupPositions, clientID, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetInterfaceState(uint clientID, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SetInterfaceState(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientID, _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetInterfaceState, clientID, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetInterfaceState(clientID, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetInterfaceState, clientID, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestRankLevel(uint clientID, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"RequestRankLevel(\n\tuint clientID = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)", clientID,
		    _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestRankLevel, clientID, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestRankLevel(clientID, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestRankLevel, clientID, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall InitiateTrade(uint clientID1, uint clientID2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"InitiateTrade(\n\tuint clientID1 = %u\n\tuint clientID2 = %u\n)", clientID1, clientID2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InitiateTrade, clientID1, clientID2);

		InitiateTrade__Inner(clientID1, clientID2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.InitiateTrade(clientID1, clientID2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__InitiateTrade, clientID1, clientID2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall TerminateTrade(uint clientID, int accepted)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"TerminateTrade(\n\tuint clientID = %u\n\tint accepted = %d\n)", clientID, accepted);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TerminateTrade, clientID, accepted);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.TerminateTrade(clientID, accepted);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		TerminateTrade__InnerAfter(clientID, accepted);

		CallPluginsAfter(HookedCall::IServerImpl__TerminateTrade, clientID, accepted);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall AcceptTrade(uint clientID, bool _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"AcceptTrade(\n\tuint clientID = %u\n\tbool _genArg1 = %d\n)", clientID, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AcceptTrade, clientID, _genArg1);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.AcceptTrade(clientID, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AcceptTrade, clientID, _genArg1);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetTradeMoney(uint clientID, ulong _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SetTradeMoney(\n\tuint clientID = %u\n\tulong _genArg1 = %u\n)", clientID, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTradeMoney, clientID, _genArg1);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetTradeMoney(clientID, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetTradeMoney, clientID, _genArg1);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall AddTradeEquip(uint clientID, EquipDesc const& ed)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"AddTradeEquip(\n\tuint clientID = %u\n\tEquipDesc const& ed = %s\n)", clientID, ToLogString(ed));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AddTradeEquip, clientID, ed);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.AddTradeEquip(clientID, ed);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AddTradeEquip, clientID, ed);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall DelTradeEquip(uint clientID, EquipDesc const& ed)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"DelTradeEquip(\n\tuint clientID = %u\n\tEquipDesc const& ed = %s\n)", clientID, ToLogString(ed));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DelTradeEquip, clientID, ed);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.DelTradeEquip(clientID, ed);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DelTradeEquip, clientID, ed);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestTrade(uint _genArg1, uint _genArg2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"RequestTrade(\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)", _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestTrade, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestTrade(_genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestTrade, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall StopTradeRequest(uint clientID)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"StopTradeRequest(\n\tuint clientID = %u\n)", clientID);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradeRequest, clientID);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.StopTradeRequest(clientID);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__StopTradeRequest, clientID);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall Dock(uint const& _genArg1, uint const& _genArg2)
	{
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SubmitChat(CHAT_ID cidFrom, ulong size, void const* rdlReader, CHAT_ID cidTo, int _genArg1)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"SubmitChat(\n\tCHAT_ID cidFrom = %s\n\tulong size = %u\n\tvoid const* rdlReader = %p\n\tCHAT_ID cidTo = "
		    L"%s\n\tint _genArg1 = %d\n)",
		    ToLogString(cidFrom), size, rdlReader, ToLogString(cidTo), _genArg1);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__SubmitChat, cidFrom.iID, size, rdlReader, cidTo.iID, _genArg1);

		bool innerCheck = SubmitChat__Inner(cidFrom, size, rdlReader, cidTo, _genArg1);
		if (!innerCheck)
			return;
		g_InSubmitChat = true;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SubmitChat(cidFrom, size, rdlReader, cidTo, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		g_InSubmitChat = false;

		CallPluginsAfter(HookedCall::IServerImpl__SubmitChat, cidFrom.iID, size, rdlReader, cidTo.iID, _genArg1);
	}
} // namespace HkIServerImpl

HookEntry HkIServerImplEntries[] = {
	{ FARPROC(HkIServerImpl::SubmitChat), -0x008, nullptr },
	{ FARPROC(HkIServerImpl::FireWeapon), 0x000, nullptr },
	{ FARPROC(HkIServerImpl::ActivateEquip), 0x004, nullptr },
	{ FARPROC(HkIServerImpl::ActivateCruise), 0x008, nullptr },
	{ FARPROC(HkIServerImpl::ActivateThrusters), 0x00C, nullptr },
	{ FARPROC(HkIServerImpl::SetTarget), 0x010, nullptr },
	{ FARPROC(HkIServerImpl::TractorObjects), 0x014, nullptr },
	{ FARPROC(HkIServerImpl::GoTradelane), 0x018, nullptr },
	{ FARPROC(HkIServerImpl::StopTradelane), 0x01C, nullptr },
	{ FARPROC(HkIServerImpl::JettisonCargo), 0x020, nullptr },
	{ FARPROC(HkIServerImpl::DisConnect), 0x040, nullptr },
	{ FARPROC(HkIServerImpl::OnConnect), 0x044, nullptr },
	{ FARPROC(HkIServerImpl::Login), 0x048, nullptr },
	{ FARPROC(HkIServerImpl::CharacterInfoReq), 0x04C, nullptr },
	{ FARPROC(HkIServerImpl::CharacterSelect), 0x050, nullptr },
	{ FARPROC(HkIServerImpl::CreateNewCharacter), 0x058, nullptr },
	{ FARPROC(HkIServerImpl::DestroyCharacter), 0x05C, nullptr },
	{ FARPROC(HkIServerImpl::ReqShipArch), 0x064, nullptr },
	{ FARPROC(HkIServerImpl::ReqHullStatus), 0x068, nullptr },
	{ FARPROC(HkIServerImpl::ReqCollisionGroups), 0x06C, nullptr },
	{ FARPROC(HkIServerImpl::ReqEquipment), 0x070, nullptr },
	{ FARPROC(HkIServerImpl::ReqAddItem), 0x078, nullptr },
	{ FARPROC(HkIServerImpl::ReqRemoveItem), 0x07C, nullptr },
	{ FARPROC(HkIServerImpl::ReqModifyItem), 0x080, nullptr },
	{ FARPROC(HkIServerImpl::ReqSetCash), 0x084, nullptr },
	{ FARPROC(HkIServerImpl::ReqChangeCash), 0x088, nullptr },
	{ FARPROC(HkIServerImpl::BaseEnter), 0x08C, nullptr },
	{ FARPROC(HkIServerImpl::BaseExit), 0x090, nullptr },
	{ FARPROC(HkIServerImpl::LocationEnter), 0x094, nullptr },
	{ FARPROC(HkIServerImpl::LocationExit), 0x098, nullptr },
	{ FARPROC(HkIServerImpl::BaseInfoRequest), 0x09C, nullptr },
	{ FARPROC(HkIServerImpl::LocationInfoRequest), 0x0A0, nullptr },
	{ FARPROC(HkIServerImpl::GFObjSelect), 0x0A4, nullptr },
	{ FARPROC(HkIServerImpl::GFGoodVaporized), 0x0A8, nullptr },
	{ FARPROC(HkIServerImpl::MissionResponse), 0x0AC, nullptr },
	{ FARPROC(HkIServerImpl::TradeResponse), 0x0B0, nullptr },
	{ FARPROC(HkIServerImpl::GFGoodBuy), 0x0B4, nullptr },
	{ FARPROC(HkIServerImpl::GFGoodSell), 0x0B8, nullptr },
	{ FARPROC(HkIServerImpl::SystemSwitchOutComplete), 0x0BC, nullptr },
	{ FARPROC(HkIServerImpl::PlayerLaunch), 0x0C0, nullptr },
	{ FARPROC(HkIServerImpl::LaunchComplete), 0x0C4, nullptr },
	{ FARPROC(HkIServerImpl::JumpInComplete), 0x0C8, nullptr },
	{ FARPROC(HkIServerImpl::Hail), 0x0CC, nullptr },
	{ FARPROC(HkIServerImpl::SPObjUpdate), 0x0D0, nullptr },
	{ FARPROC(HkIServerImpl::SPMunitionCollision), 0x0D4, nullptr },
	{ FARPROC(HkIServerImpl::SPObjCollision), 0x0DC, nullptr },
	{ FARPROC(HkIServerImpl::SPRequestUseItem), 0x0E0, nullptr },
	{ FARPROC(HkIServerImpl::SPRequestInvincibility), 0x0E4, nullptr },
	{ FARPROC(HkIServerImpl::RequestEvent), 0x0F0, nullptr },
	{ FARPROC(HkIServerImpl::RequestCancel), 0x0F4, nullptr },
	{ FARPROC(HkIServerImpl::MineAsteroid), 0x0F8, nullptr },
	{ FARPROC(HkIServerImpl::RequestCreateShip), 0x100, nullptr },
	{ FARPROC(HkIServerImpl::SPScanCargo), 0x104, nullptr },
	{ FARPROC(HkIServerImpl::SetManeuver), 0x108, nullptr },
	{ FARPROC(HkIServerImpl::InterfaceItemUsed), 0x10C, nullptr },
	{ FARPROC(HkIServerImpl::AbortMission), 0x110, nullptr },
	{ FARPROC(HkIServerImpl::SetWeaponGroup), 0x118, nullptr },
	{ FARPROC(HkIServerImpl::SetVisitedState), 0x11C, nullptr },
	{ FARPROC(HkIServerImpl::RequestBestPath), 0x120, nullptr },
	{ FARPROC(HkIServerImpl::RequestPlayerStats), 0x124, nullptr },
	{ FARPROC(HkIServerImpl::PopupDialog), 0x128, nullptr },
	{ FARPROC(HkIServerImpl::RequestGroupPositions), 0x12C, nullptr },
	{ FARPROC(HkIServerImpl::SetInterfaceState), 0x134, nullptr },
	{ FARPROC(HkIServerImpl::RequestRankLevel), 0x138, nullptr },
	{ FARPROC(HkIServerImpl::InitiateTrade), 0x13C, nullptr },
	{ FARPROC(HkIServerImpl::TerminateTrade), 0x140, nullptr },
	{ FARPROC(HkIServerImpl::AcceptTrade), 0x144, nullptr },
	{ FARPROC(HkIServerImpl::SetTradeMoney), 0x148, nullptr },
	{ FARPROC(HkIServerImpl::AddTradeEquip), 0x14C, nullptr },
	{ FARPROC(HkIServerImpl::DelTradeEquip), 0x150, nullptr },
	{ FARPROC(HkIServerImpl::RequestTrade), 0x154, nullptr },
	{ FARPROC(HkIServerImpl::StopTradeRequest), 0x158, nullptr },
	{ FARPROC(HkIServerImpl::Dock), 0x16C, nullptr },
};

void PluginManager::setupProps()
{
	setProps(HookedCall::IEngine__CShip__Init, true, false, false);
	setProps(HookedCall::IEngine__CShip__Destroy, true, false, false);
	setProps(HookedCall::IEngine__UpdateTime, true, false, true);
	setProps(HookedCall::IEngine__ElapseTime, true, false, true);
	setProps(HookedCall::IEngine__DockCall, true, false, false);
	setProps(HookedCall::IEngine__LaunchPosition, true, false, false);
	setProps(HookedCall::IEngine__ShipDestroyed, true, false, false);
	setProps(HookedCall::IEngine__BaseDestroyed, true, false, false);
	setProps(HookedCall::IEngine__GuidedHit, true, false, false);
	setProps(HookedCall::IEngine__AddDamageEntry, true, false, false);
	setProps(HookedCall::IEngine__DamageHit, true, false, false);
	setProps(HookedCall::IEngine__AllowPlayerDamage, true, false, false);
	setProps(HookedCall::IEngine__SendDeathMessage, true, false, false);
	setProps(HookedCall::FLHook__TimerCheckKick, true, false, false);
	setProps(HookedCall::FLHook__TimerNPCAndF1Check, true, false, false);
	setProps(HookedCall::FLHook__UserCommand__Process, true, false, false);
	setProps(HookedCall::FLHook__AdminCommand__Help, true, false, true);
	setProps(HookedCall::FLHook__AdminCommand__Process, true, false, false);
	setProps(HookedCall::FLHook__LoadSettings, true, false, true);
	setProps(HookedCall::FLHook__LoadCharacterSettings, true, false, true);
	setProps(HookedCall::FLHook__ClearClientInfo, true, false, true);
	setProps(HookedCall::FLHook__ProcessEvent, true, false, false);
	setProps(HookedCall::IChat__SendChat, true, false, false);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, true, false, true);
	setProps(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, true, false, true);
	setProps(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, true, false, true);
	setProps(HookedCall::IServerImpl__FireWeapon, true, false, true);
	setProps(HookedCall::IServerImpl__ActivateEquip, true, false, true);
	setProps(HookedCall::IServerImpl__ActivateCruise, true, false, true);
	setProps(HookedCall::IServerImpl__ActivateThrusters, true, false, true);
	setProps(HookedCall::IServerImpl__SetTarget, true, false, true);
	setProps(HookedCall::IServerImpl__TractorObjects, true, false, true);
	setProps(HookedCall::IServerImpl__GoTradelane, true, false, true);
	setProps(HookedCall::IServerImpl__StopTradelane, true, false, true);
	setProps(HookedCall::IServerImpl__JettisonCargo, true, false, true);
	setProps(HookedCall::IServerImpl__Startup, true, false, true);
	setProps(HookedCall::IServerImpl__Shutdown, true, false, false);
	setProps(HookedCall::IServerImpl__Update, true, false, true);
	setProps(HookedCall::IServerImpl__DisConnect, true, false, true);
	setProps(HookedCall::IServerImpl__OnConnect, true, false, true);
	setProps(HookedCall::IServerImpl__Login, true, false, true);
	setProps(HookedCall::IServerImpl__CharacterInfoReq, true, false, true);
	setProps(HookedCall::IServerImpl__CharacterSelect, true, false, true);
	setProps(HookedCall::IServerImpl__CreateNewCharacter, true, false, true);
	setProps(HookedCall::IServerImpl__DestroyCharacter, true, false, true);
	setProps(HookedCall::IServerImpl__ReqShipArch, true, false, true);
	setProps(HookedCall::IServerImpl__ReqHullStatus, true, false, true);
	setProps(HookedCall::IServerImpl__ReqCollisionGroups, true, false, true);
	setProps(HookedCall::IServerImpl__ReqEquipment, true, false, true);
	setProps(HookedCall::IServerImpl__ReqAddItem, true, false, true);
	setProps(HookedCall::IServerImpl__ReqRemoveItem, true, false, true);
	setProps(HookedCall::IServerImpl__ReqModifyItem, true, false, true);
	setProps(HookedCall::IServerImpl__ReqSetCash, true, false, true);
	setProps(HookedCall::IServerImpl__ReqChangeCash, true, false, true);
	setProps(HookedCall::IServerImpl__BaseEnter, true, false, true);
	setProps(HookedCall::IServerImpl__BaseExit, true, false, true);
	setProps(HookedCall::IServerImpl__LocationEnter, true, false, true);
	setProps(HookedCall::IServerImpl__LocationExit, true, false, true);
	setProps(HookedCall::IServerImpl__BaseInfoRequest, true, false, true);
	setProps(HookedCall::IServerImpl__LocationInfoRequest, true, false, true);
	setProps(HookedCall::IServerImpl__GFObjSelect, true, false, true);
	setProps(HookedCall::IServerImpl__GFGoodVaporized, true, false, true);
	setProps(HookedCall::IServerImpl__MissionResponse, true, false, true);
	setProps(HookedCall::IServerImpl__TradeResponse, true, false, true);
	setProps(HookedCall::IServerImpl__GFGoodBuy, true, false, true);
	setProps(HookedCall::IServerImpl__GFGoodSell, true, false, true);
	setProps(HookedCall::IServerImpl__SystemSwitchOutComplete, true, false, true);
	setProps(HookedCall::IServerImpl__PlayerLaunch, true, false, true);
	setProps(HookedCall::IServerImpl__LaunchComplete, true, false, true);
	setProps(HookedCall::IServerImpl__JumpInComplete, true, false, true);
	setProps(HookedCall::IServerImpl__Hail, true, false, true);
	setProps(HookedCall::IServerImpl__SPObjUpdate, true, false, true);
	setProps(HookedCall::IServerImpl__SPMunitionCollision, true, false, true);
	setProps(HookedCall::IServerImpl__SPObjCollision, true, false, true);
	setProps(HookedCall::IServerImpl__SPRequestUseItem, true, false, true);
	setProps(HookedCall::IServerImpl__SPRequestInvincibility, true, false, true);
	setProps(HookedCall::IServerImpl__RequestEvent, true, false, true);
	setProps(HookedCall::IServerImpl__RequestCancel, true, false, true);
	setProps(HookedCall::IServerImpl__MineAsteroid, true, false, true);
	setProps(HookedCall::IServerImpl__RequestCreateShip, true, false, true);
	setProps(HookedCall::IServerImpl__SPScanCargo, true, false, true);
	setProps(HookedCall::IServerImpl__SetManeuver, true, false, true);
	setProps(HookedCall::IServerImpl__InterfaceItemUsed, true, false, true);
	setProps(HookedCall::IServerImpl__AbortMission, true, false, true);
	setProps(HookedCall::IServerImpl__SetWeaponGroup, true, false, true);
	setProps(HookedCall::IServerImpl__SetVisitedState, true, false, true);
	setProps(HookedCall::IServerImpl__RequestBestPath, true, false, true);
	setProps(HookedCall::IServerImpl__RequestPlayerStats, true, false, true);
	setProps(HookedCall::IServerImpl__PopupDialog, true, false, true);
	setProps(HookedCall::IServerImpl__RequestGroupPositions, true, false, true);
	setProps(HookedCall::IServerImpl__SetInterfaceState, true, false, true);
	setProps(HookedCall::IServerImpl__RequestRankLevel, true, false, true);
	setProps(HookedCall::IServerImpl__InitiateTrade, true, false, true);
	setProps(HookedCall::IServerImpl__TerminateTrade, true, false, true);
	setProps(HookedCall::IServerImpl__AcceptTrade, true, false, true);
	setProps(HookedCall::IServerImpl__SetTradeMoney, true, false, true);
	setProps(HookedCall::IServerImpl__AddTradeEquip, true, false, true);
	setProps(HookedCall::IServerImpl__DelTradeEquip, true, false, true);
	setProps(HookedCall::IServerImpl__RequestTrade, true, false, true);
	setProps(HookedCall::IServerImpl__StopTradeRequest, true, false, true);
	setProps(HookedCall::IServerImpl__SubmitChat, true, false, true);
}