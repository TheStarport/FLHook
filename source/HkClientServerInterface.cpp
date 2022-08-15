#include "Global.hpp"
#include "HkIClientImpl.inl"
#include "HkIServerImpl.inl"

bool HkIClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(uint clientId, XFireWeaponInfo& fwi)
{
	AddLog(
		LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(\n\tuint clientId = %u\n\tXFireWeaponInfo& fwi = %s\n)",
	    clientId, ToLogString(fwi));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, clientId, fwi);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_FIREWEAPON(clientId, fwi);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_FIREWEAPON, clientId, fwi);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(uint clientId, XActivateEquip& aq)
{
	AddLog(LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(\n\tuint clientId = %u\n\tXActivateEquip& aq = %s\n)",
	    clientId, ToLogString(aq));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, clientId, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_ACTIVATEEQUIP(clientId, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATEEQUIP, clientId, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(uint clientId, XActivateCruise& aq)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(\n\tuint clientId = %u\n\tXActivateCruise& aq = %s\n)",
	    clientId, ToLogString(aq));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, clientId, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_ACTIVATECRUISE(clientId, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATECRUISE, clientId, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(uint clientId, XActivateThrusters& aq)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(\n\tuint clientId = %u\n\tXActivateThrusters& aq = "
	    L"%s\n)",
	    clientId, ToLogString(aq));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, clientId, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(clientId, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_ACTIVATETHRUSTERS, clientId, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SETTARGET(uint clientId, XSetTarget& st)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_COMMON_SETTARGET(\n\tuint clientId = %u\n\tXSetTarget& st = %s\n)",
	    clientId, ToLogString(st));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SETTARGET(clientId, st);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_6(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_6(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_6(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(uint clientId, XGoTradelane& tl)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(\n\tuint clientId = %u\n\tXGoTradelane& tl = %s\n)",
	    clientId, ToLogString(tl));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_GOTRADELANE(clientId, tl);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(
    uint clientId, uint shipID, uint archTradelane1, uint archTradelane2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(\n\tuint clientId = %u\n\tuint shipID = %u\n\tuint "
	    L"archTradelane1 = %u\n\tuint archTradelane2 = %u\n)",
	    clientId, shipID, archTradelane1, archTradelane2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_STOPTRADELANE(clientId, shipID, archTradelane1, archTradelane2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(uint clientId, XJettisonCargo& jc)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(\n\tuint clientId = %u\n\tXJettisonCargo& jc = %s\n)",
	    clientId, ToLogString(jc));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_JETTISONCARGO(clientId, jc);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::SendPacket(uint clientId, void* _genArg1)
{
	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = SendPacket(clientId, _genArg1);
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

bool HkIClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId, ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_LOGINRESPONSE(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId, ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_CHARACTERINFO(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 "
	    L"= %s\n)",
	    clientId, ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_CHARSELECTVERIFIED(clientId, _genArg1);
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

bool HkIClientImpl::CDPClientProxy__Disconnect(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::CDPClientProxy__Disconnect(\n\tuint clientId = %u\n)", clientId);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = CDPClientProxy__Disconnect(clientId);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

uint HkIClientImpl::CDPClientProxy__GetSendQSize(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::CDPClientProxy__GetSendQSize(\n\tuint clientId = %u\n)", clientId);

	uint retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = CDPClientProxy__GetSendQSize(clientId);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

uint HkIClientImpl::CDPClientProxy__GetSendQBytes(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::CDPClientProxy__GetSendQBytes(\n\tuint clientId = %u\n)", clientId);

	uint retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = CDPClientProxy__GetSendQBytes(clientId);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

double HkIClientImpl::CDPClientProxy__GetLinkSaturation(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::CDPClientProxy__GetLinkSaturation(\n\tuint clientId = %u\n)", clientId);

	auto [retVal, skip] =
	    CallPluginsBefore<double>(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, clientId);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = CDPClientProxy__GetLinkSaturation(clientId);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__CDPClientProxy__GetLinkSaturation, clientId);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(uint clientId, uint shipArch)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(\n\tuint clientId = %u\n\tuint shipArch = %u\n)",
	    clientId, shipArch);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, clientId, shipArch);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETSHIPARCH(clientId, shipArch);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSHIPARCH, clientId, shipArch);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(uint clientId, float status)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(\n\tuint clientId = %u\n\tfloat status = %f\n)",
	    clientId, status);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, clientId, status);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETHULLSTATUS(clientId, status);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETHULLSTATUS, clientId, status);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 "
	    L"= %s\n)",
	    clientId, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(clientId, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCOLLISIONGROUPS, clientId, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETEQUIPMENT(clientId, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETEQUIPMENT, clientId, _genArg1);

	return retVal;
}

void HkIClientImpl::unknown_26(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_26(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_26(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETADDITEM(
    uint clientId, FLPACKET_UNKNOWN& _genArg1, FLPACKET_UNKNOWN& _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETADDITEM(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n\tFLPACKET_UNKNOWN& _genArg2 = %s\n)",
	    clientId, ToLogString(_genArg1), ToLogString(_genArg2));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETADDITEM(clientId, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETADDITEM, clientId, _genArg1, _genArg2);

	return retVal;
}

void HkIClientImpl::unknown_28(uint clientId, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::unknown_28(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientId, _genArg1, _genArg2, _genArg3);

	CALL_CLIENT_PREAMBLE
	{
		unknown_28(clientId, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETSTARTROOM(clientId, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETSTARTROOM, clientId, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFDESTROYCHARACTER(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFUPDATECHAR(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(uint clientId, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)",
	    clientId, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(uint clientId, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientId, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_36(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_36(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_36(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_37(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_37(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_37(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(uint clientId, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientId, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(uint clientId, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientId, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(uint clientId, uint reason)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(\n\tuint clientId = %u\n\tuint reason = %u\n)",
	    clientId, reason);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(clientId, reason);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_44(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_44(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_44(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(uint clientId, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(\n\tuint clientId = %u\n\tuint _genArg1 = "
	    L"%u\n)",
	    clientId, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(uint clientId, FLPACKET_CREATESOLAR& solar)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(\n\tuint clientId = %u\n\tFLPACKET_CREATESOLAR& solar = "
	    L"%s\n)",
	    clientId, ToLogString(solar));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, clientId, solar);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATESOLAR(clientId, solar);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESOLAR, clientId, solar);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATESHIP(uint clientId, FLPACKET_CREATESHIP& ship)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATESHIP(\n\tuint clientId = %u\n\tFLPACKET_CREATESHIP& ship = %s\n)",
	    clientId, ToLogString(ship));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, clientId, ship);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATESHIP(clientId, ship);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATESHIP, clientId, ship);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATELOOT(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATELOOT(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATELOOT(clientId, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATELOOT, clientId, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATEMINE(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATEMINE(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)",
	    clientId, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATEMINE(clientId, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEMINE, clientId, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATEGUIDED(clientId, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATEGUIDED, clientId, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_CREATECOUNTER(clientId, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_CREATECOUNTER, clientId, _genArg1);

	return retVal;
}

void HkIClientImpl::unknown_53(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_53(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_53(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_54(uint clientId, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::unknown_54(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientId, _genArg1, _genArg2, _genArg3);

	CALL_CLIENT_PREAMBLE
	{
		unknown_54(clientId, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_UPDATEOBJECT(uint clientId, SSPObjUpdateInfo& update)
{
	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, clientId, update);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_COMMON_UPDATEOBJECT(clientId, update);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_COMMON_UPDATEOBJECT, clientId, update);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(uint clientId, FLPACKET_DESTROYOBJECT& destroy)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(\n\tuint clientId = %u\n\tFLPACKET_DESTROYOBJECT& destroy "
	    L"= %s\n)",
	    clientId, ToLogString(destroy));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, clientId, destroy);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_DESTROYOBJECT(clientId, destroy);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_DESTROYOBJECT, clientId, destroy);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(uint clientId, XActivateEquip& aq)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(\n\tuint clientId = %u\n\tXActivateEquip& aq = %s\n)",
	    clientId, ToLogString(aq));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, clientId, aq);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_ACTIVATEOBJECT(clientId, aq);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_ACTIVATEOBJECT, clientId, aq);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId, ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId, ToLogString(_genArg1));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LAND(uint clientId, FLPACKET_LAND& land)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_LAND(\n\tuint clientId = %u\n\tFLPACKET_LAND& land = %s\n)",
	    clientId, ToLogString(land));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_LAND(clientId, land);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_LAUNCH(uint clientId, FLPACKET_LAUNCH& launch)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_LAUNCH(\n\tuint clientId = %u\n\tFLPACKET_LAUNCH& launch = %s\n)",
	    clientId, ToLogString(launch));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, clientId, launch);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_LAUNCH(clientId, launch);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_LAUNCH, clientId, launch);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(uint clientId, bool response, uint shipID)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(\n\tuint clientId = %u\n\tbool response = "
	    L"%d\n\tuint shipID = %u\n)",
	    clientId, response, shipID);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, clientId, response, shipID);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(clientId, response, shipID);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP, clientId, response, shipID);

	return retVal;
}

void HkIClientImpl::unknown_63(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_63(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_63(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(uint clientId, uint objID, DamageList& dmgList)
{
	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_DAMAGEOBJECT(clientId, objID, dmgList);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(uint clientId, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)",
	    clientId, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_ITEMTRACTORED(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_USE_ITEM(uint clientId, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_USE_ITEM(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)",
	    clientId, _genArg1);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_USE_ITEM(clientId, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_USE_ITEM, clientId, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(uint clientId, FLPACKET_SETREPUTATION& rep)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(\n\tuint clientId = %u\n\tFLPACKET_SETREPUTATION& rep = "
	    L"%s\n)",
	    clientId, ToLogString(rep));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, clientId, rep);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETREPUTATION(clientId, rep);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETREPUTATION, clientId, rep);

	return retVal;
}

void HkIClientImpl::unknown_68(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_68(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_68(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SENDCOMM(
    uint clientId, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6,
    uint _genArg7, uint _genArg8, uint _genArg9, uint _genArg10, uint _genArg11, uint _genArg12, uint _genArg13,
    uint _genArg14, uint _genArg15, uint _genArg16, uint _genArg17, uint _genArg18, uint _genArg19, uint _genArg20,
    uint _genArg21, uint _genArg22)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SENDCOMM(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = "
	    L"%u\n\tuint _genArg3 = %u\n\tuint _genArg4 = %u\n\tuint _genArg5 = %u\n\tuint _genArg6 = %u\n\tuint _genArg7 "
	    L"= %u\n\tuint _genArg8 = %u\n\tuint _genArg9 = %u\n\tuint _genArg10 = %u\n\tuint _genArg11 = %u\n\tuint "
	    L"_genArg12 = %u\n\tuint _genArg13 = %u\n\tuint _genArg14 = %u\n\tuint _genArg15 = %u\n\tuint _genArg16 = "
	    L"%u\n\tuint _genArg17 = %u\n\tuint _genArg18 = %u\n\tuint _genArg19 = %u\n\tuint _genArg20 = %u\n\tuint "
	    L"_genArg21 = %u\n\tuint _genArg22 = %u\n)",
	    clientId, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10,
	    _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20,
	    _genArg21, _genArg22);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM, clientId, _genArg1, _genArg2, _genArg3, _genArg4,
	    _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14,
	    _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SENDCOMM(
			    clientId, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6, _genArg7, _genArg8, _genArg9,
			    _genArg10, _genArg11, _genArg12, _genArg13, _genArg14, _genArg15, _genArg16, _genArg17, _genArg18,
			    _genArg19, _genArg20, _genArg21, _genArg22);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_SENDCOMM, clientId, _genArg1, _genArg2, _genArg3, _genArg4,
	    _genArg5, _genArg6, _genArg7, _genArg8, _genArg9, _genArg10, _genArg11, _genArg12, _genArg13, _genArg14,
	    _genArg15, _genArg16, _genArg17, _genArg18, _genArg19, _genArg20, _genArg21, _genArg22);

	return retVal;
}

void HkIClientImpl::unknown_70(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_70(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_70(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 "
	    L"= %s\n)",
	    clientId, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(clientId, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SET_MISSION_MESSAGE, clientId, _genArg1);

	return retVal;
}

void HkIClientImpl::unknown_72(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_72(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_72(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(uint clientId, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)",
	    clientId, _genArg1);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(clientId, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES, clientId, _genArg1);

	return retVal;
}

void HkIClientImpl::unknown_74(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_74(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_74(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_75(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_75(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_75(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MARKOBJ(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MARKOBJ(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = "
	    L"%u\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_MARKOBJ(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_77(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_77(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_77(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SETCASH(uint clientId, uint cash)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_SETCASH(\n\tuint clientId = %u\n\tuint cash = %u\n)", clientId,
	    cash);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, clientId, cash);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SETCASH(clientId, cash);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SETCASH, clientId, cash);

	return retVal;
}

void HkIClientImpl::unknown_79(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_79(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_79(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_80(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_80(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_80(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_81(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_81(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_81(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_82(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_82(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_82(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_83(uint clientId, char* _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_83(\n\tuint clientId = %u\n\tchar* _genArg1 = %s\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_83(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(
    uint clientId, uint shipID, uint flag, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(\n\tuint clientId = %u\n\tuint shipID = %u\n\tuint flag "
	    L"= %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId, shipID, flag, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_REQUEST_RETURNED(clientId, shipID, flag, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_85(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_85(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_85(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_86(uint clientId, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::unknown_86(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientId, _genArg1, _genArg2, _genArg3);

	CALL_CLIENT_PREAMBLE
	{
		unknown_86(clientId, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_BURNFUSE(uint clientId, FLPACKET_BURNFUSE& burnFuse)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_BURNFUSE(\n\tuint clientId = %u\n\tFLPACKET_BURNFUSE& burnFuse = %s\n)",
	    clientId, ToLogString(burnFuse));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, clientId, burnFuse);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_BURNFUSE(clientId, burnFuse);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_BURNFUSE, clientId, burnFuse);

	return retVal;
}

void HkIClientImpl::unknown_89(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_89(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_89(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_90(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_90(\n\tuint clientId = %u\n)", clientId);

	CALL_CLIENT_PREAMBLE
	{
		unknown_90(clientId);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_91(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_91(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_91(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_WEAPON_GROUP(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_VISITED_STATE(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_REQUEST_BEST_PATH(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(\n\tuint clientId = %u\n\tuchar* _genArg1 = "
	    L"%p\n\tint _genArg2 = %d\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_96(uint clientId, uint _genArg1, uint _genArg2, uint _genArg3)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::unknown_96(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n)",
	    clientId, _genArg1, _genArg2, _genArg3);

	CALL_CLIENT_PREAMBLE
	{
		unknown_96(clientId, _genArg1, _genArg2, _genArg3);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(\n\tuint clientId = %u\n\tuchar* _genArg1 = "
	    L"%p\n\tint _genArg2 = %d\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint "
	    L"_genArg2 = %d\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_MISSION_LOG(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(uint clientId, uchar* _genArg1, int _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(\n\tuint clientId = %u\n\tuchar* _genArg1 = "
	    L"%p\n\tint _genArg2 = %d\n)",
	    clientId, _genArg1, _genArg2);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_SET_INTERFACE_STATE(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_100(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_100(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_100(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_101(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_101(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = %s\n)", clientId,
	    ToLogString(_genArg1));

	CALL_CLIENT_PREAMBLE
	{
		unknown_101(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_102(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_102(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_102(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_103(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_103(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_103(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_104(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_104(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_104(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_105(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_105(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_105(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_106(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_106(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_106(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_107(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_107(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	CALL_CLIENT_PREAMBLE
	{
		unknown_107(clientId, _genArg1, _genArg2);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(uint clientId, uint _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)",
	    clientId, _genArg1);

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_COMMON_PLAYER_TRADE(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_109(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_109(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_109(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 "
	    L"= %u\n)",
	    clientId, _genArg1, _genArg2);

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_SCANNOTIFY(clientId, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_SCANNOTIFY, clientId, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(uint clientId, wchar_t* _genArg1, uint _genArg2, char _genArg3)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(\n\tuint clientId = %u\n\twchar_t* _genArg1 = %p\n\tuint "
	    L"_genArg2 = %u\n\tchar _genArg3 = %s\n)",
	    clientId, _genArg1, _genArg2, ToLogString(_genArg3));

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, clientId, _genArg1, _genArg2, _genArg3);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_PLAYERLIST(clientId, _genArg1, _genArg2, _genArg3);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST, clientId, _genArg1, _genArg2, _genArg3);

	return retVal;
}

void HkIClientImpl::unknown_112(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_112(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_112(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(\n\tuint clientId = %u\n)", clientId);

	auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, clientId);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_PLAYERLIST_2(clientId);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_PLAYERLIST_2, clientId);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_6(clientId, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_6, clientId, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_7(clientId, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_7, clientId, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(uint clientId, FLPACKET_UNKNOWN& _genArg1)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(\n\tuint clientId = %u\n\tFLPACKET_UNKNOWN& _genArg1 = "
	    L"%s\n)",
	    clientId, ToLogString(_genArg1));

	auto [retVal, skip] =
	    CallPluginsBefore<bool>(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, clientId, _genArg1);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE(clientId, _genArg1);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE, clientId, _genArg1);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_2(clientId, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_2, clientId, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(uint clientId, uint targetID, uint rank)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(\n\tuint clientId = %u\n\tuint targetID = %u\n\tuint "
	    L"rank = %u\n)",
	    clientId, targetID, rank);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, clientId, targetID, rank);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_3(clientId, targetID, rank);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_3, clientId, targetID, rank);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_4(clientId, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_4, clientId, _genArg1, _genArg2);

	return retVal;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(uint clientId, uint _genArg1, uint _genArg2)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint "
	    L"_genArg2 = %u\n)",
	    clientId, _genArg1, _genArg2);

	auto [retVal, skip] = CallPluginsBefore<bool>(
	    HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, clientId, _genArg1, _genArg2);

	if (!skip)
	{
		CALL_CLIENT_PREAMBLE
		{
			retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_5(clientId, _genArg1, _genArg2);
		}
		CALL_CLIENT_POSTAMBLE;
	}

	CallPluginsAfter(HookedCall::IClientImpl__Send_FLPACKET_SERVER_MISCOBJUPDATE_5, clientId, _genArg1, _genArg2);

	return retVal;
}

void HkIClientImpl::unknown_121(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_121(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_121(clientId, _genArg1);
	}
	CALL_CLIENT_POSTAMBLE;
}

bool HkIClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(uint clientId, uint shipID, Vector& formationOffset)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(\n\tuint clientId = %u\n\tuint shipID = %u\n\tVector& "
	    L"formationOffset = %s\n)",
	    clientId, shipID, ToLogString(formationOffset));

	bool retVal;
	CALL_CLIENT_PREAMBLE
	{
		retVal = Send_FLPACKET_SERVER_FORMATION_UPDATE(clientId, shipID, formationOffset);
	}
	CALL_CLIENT_POSTAMBLE;

	return retVal;
}

void HkIClientImpl::unknown_123(
    uint clientId, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6)
{
	AddLog(
	    LogType::Normal, LogLevel::Debug,
	    L"HkIClientImpl::unknown_123(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n\tuint _genArg2 = %u\n\tuint "
	    L"_genArg3 = %u\n\tuint _genArg4 = %u\n\tuint _genArg5 = %u\n\tuint _genArg6 = %u\n)",
	    clientId, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);

	CALL_CLIENT_PREAMBLE
	{
		unknown_123(clientId, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_124(uint clientId)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_124(\n\tuint clientId = %u\n)", clientId);

	CALL_CLIENT_PREAMBLE
	{
		unknown_124(clientId);
	}
	CALL_CLIENT_POSTAMBLE;
}

void HkIClientImpl::unknown_125(uint clientId, uint _genArg1)
{
	AddLog(LogType::Normal, LogLevel::Debug, L"HkIClientImpl::unknown_125(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

	CALL_CLIENT_PREAMBLE
	{
		unknown_125(clientId, _genArg1);
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
	void __stdcall FireWeapon(uint clientId, XFireWeaponInfo const& fwi)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"FireWeapon(\n\tuint clientId = %u\n\tXFireWeaponInfo const& fwi = %s\n)", clientId,
		    ToLogString(fwi));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__FireWeapon, clientId, fwi);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.FireWeapon(clientId, fwi);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__FireWeapon, clientId, fwi);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ActivateEquip(uint clientId, XActivateEquip const& aq)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ActivateEquip(\n\tuint clientId = %u\n\tXActivateEquip const& aq = %s\n)", clientId,
		    ToLogString(aq));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateEquip, clientId, aq);

		CHECK_FOR_DISCONNECT;

		ActivateEquip__Inner(clientId, aq);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ActivateEquip(clientId, aq);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateEquip, clientId, aq);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ActivateCruise(uint clientId, XActivateCruise const& ac)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ActivateCruise(\n\tuint clientId = %u\n\tXActivateCruise const& ac = %s\n)", clientId,
		    ToLogString(ac));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateCruise, clientId, ac);

		CHECK_FOR_DISCONNECT;

		ActivateCruise__Inner(clientId, ac);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ActivateCruise(clientId, ac);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateCruise, clientId, ac);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ActivateThrusters(uint clientId, XActivateThrusters const& at)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ActivateThrusters(\n\tuint clientId = %u\n\tXActivateThrusters const& at = %s\n)", clientId,
		    ToLogString(at));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ActivateThrusters, clientId, at);

		CHECK_FOR_DISCONNECT;

		ActivateThrusters__Inner(clientId, at);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ActivateThrusters(clientId, at);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ActivateThrusters, clientId, at);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetTarget(uint clientId, XSetTarget const& st)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SetTarget(\n\tuint clientId = %u\n\tXSetTarget const& st = %s\n)", clientId, ToLogString(st));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTarget, clientId, st);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetTarget(clientId, st);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetTarget, clientId, st);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall TractorObjects(uint clientId, XTractorObjects const& to)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"TractorObjects(\n\tuint clientId = %u\n\tXTractorObjects const& to = %s\n)", clientId,
		    ToLogString(to));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TractorObjects, clientId, to);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.TractorObjects(clientId, to);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__TractorObjects, clientId, to);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall GoTradelane(uint clientId, XGoTradelane const& gt)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GoTradelane(\n\tuint clientId = %u\n\tXGoTradelane const& gt = %s\n)", clientId, ToLogString(gt));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GoTradelane, clientId, gt);

		GoTradelane__Inner(clientId, gt);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GoTradelane(clientId, gt);
			}
			CALL_SERVER_POSTAMBLE(GoTradelane__Catch(clientId, gt), );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GoTradelane, clientId, gt);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall StopTradelane(uint clientId, uint shipID, uint tradelaneRing1, uint tradelaneRing2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"StopTradelane(\n\tuint clientId = %u\n\tuint shipID = %u\n\tuint tradelaneRing1 = %u\n\tuint "
		    L"tradelaneRing2 = %u\n)",
		    clientId, shipID, tradelaneRing1, tradelaneRing2);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__StopTradelane, clientId, shipID, tradelaneRing1, tradelaneRing2);

		StopTradelane__Inner(clientId, shipID, tradelaneRing1, tradelaneRing2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.StopTradelane(clientId, shipID, tradelaneRing1, tradelaneRing2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__StopTradelane, clientId, shipID, tradelaneRing1, tradelaneRing2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall JettisonCargo(uint clientId, XJettisonCargo const& jc)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"JettisonCargo(\n\tuint clientId = %u\n\tXJettisonCargo const& jc = %s\n)", clientId,
		    ToLogString(jc));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__JettisonCargo, clientId, jc);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.JettisonCargo(clientId, jc);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__JettisonCargo, clientId, jc);
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
	void __stdcall DisConnect(uint clientId, EFLConnection conn)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"DisConnect(\n\tuint clientId = %u\n\tEFLConnection conn = %s\n)", clientId, ToLogString(conn));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DisConnect, clientId, conn);

		DisConnect__Inner(clientId, conn);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.DisConnect(clientId, conn);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DisConnect, clientId, conn);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall OnConnect(uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"OnConnect(\n\tuint clientId = %u\n)", clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__OnConnect, clientId);

		bool innerCheck = OnConnect__Inner(clientId);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.OnConnect(clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		OnConnect__InnerAfter(clientId);

		CallPluginsAfter(HookedCall::IServerImpl__OnConnect, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall Login(SLoginInfo const& li, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"Login(\n\tSLoginInfo const& li = %s\n\tuint clientId = %u\n)", ToLogString(li), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Login, li, clientId);

		if (!skip && Login__InnerBefore(li, clientId))
		{
			CALL_SERVER_PREAMBLE
			{
				Server.Login(li, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		Login__InnerAfter(li, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__Login, li, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall CharacterInfoReq(uint clientId, bool _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"CharacterInfoReq(\n\tuint clientId = %u\n\tbool _genArg1 = %d\n)", clientId, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CharacterInfoReq, clientId, _genArg1);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = CharacterInfoReq__Inner(clientId, _genArg1);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.CharacterInfoReq(clientId, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(CharacterInfoReq__Catch(clientId, _genArg1), );
		}

		CallPluginsAfter(HookedCall::IServerImpl__CharacterInfoReq, clientId, _genArg1);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall CharacterSelect(CHARACTER_ID const& cid, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"CharacterSelect(\n\tCHARACTER_ID const& cid = %s\n\tuint clientId = %u\n)", ToLogString(cid),
		    clientId);

		std::string charName = cid.szCharFilename;
		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__CharacterSelect, charName, clientId);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = CharacterSelect__Inner(cid, clientId);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.CharacterSelect(cid, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		CharacterSelect__InnerAfter(cid, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__CharacterSelect, charName, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall CreateNewCharacter(SCreateCharacterInfo const& _genArg1, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"CreateNewCharacter(\n\tSCreateCharacterInfo const& _genArg1 = %s\n\tuint clientId = %u\n)",
		    ToLogString(_genArg1), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__CreateNewCharacter, _genArg1, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.CreateNewCharacter(_genArg1, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__CreateNewCharacter, _genArg1, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall DestroyCharacter(CHARACTER_ID const& _genArg1, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"DestroyCharacter(\n\tCHARACTER_ID const& _genArg1 = %s\n\tuint clientId = %u\n)",
		    ToLogString(_genArg1), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DestroyCharacter, _genArg1, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.DestroyCharacter(_genArg1, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DestroyCharacter, _genArg1, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqShipArch(uint archID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqShipArch(\n\tuint archID = %u\n\tuint clientId = %u\n)", archID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqShipArch, archID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqShipArch(archID, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqShipArch, archID, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqHullStatus(float status, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqHullStatus(\n\tfloat status = %f\n\tuint clientId = %u\n)", status, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqHullStatus, status, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqHullStatus(status, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqHullStatus, status, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqCollisionGroups(st6::list<CollisionGroupDesc> const& collisionGroups, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"ReqCollisionGroups(\n\tst6::list<CollisionGroupDesc> const& collisionGroups = %s\n\tuint clientId = "
		    L"%u\n)",
		    ToLogString(collisionGroups), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqCollisionGroups, collisionGroups, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqCollisionGroups(collisionGroups, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqCollisionGroups, collisionGroups, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqEquipment(EquipDescList const& edl, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ReqEquipment(\n\tEquipDescList const& edl = %s\n\tuint clientId = %u\n)", ToLogString(edl),
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqEquipment, edl, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqEquipment(edl, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqEquipment, edl, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqAddItem(uint goodID, char const* hardpoint, int count, float status, bool mounted, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"ReqAddItem(\n\tuint goodID = %u\n\tchar const* hardpoint = %p\n\tint count = %d\n\tfloat status = "
		    L"%f\n\tbool mounted = %d\n\tuint clientId = %u\n)",
		    goodID, hardpoint, count, status, mounted, clientId);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__ReqAddItem, goodID, hardpoint, count, status, mounted, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqAddItem(goodID, hardpoint, count, status, mounted, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqAddItem, goodID, hardpoint, count, status, mounted, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqRemoveItem(ushort slotID, int count, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"ReqRemoveItem(\n\tushort slotID = %u\n\tint count = %d\n\tuint clientId = %u\n)", slotID, count,
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqRemoveItem, slotID, count, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqRemoveItem(slotID, count, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqRemoveItem, slotID, count, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqModifyItem(
	    ushort slotID, char const* hardpoint, int count, float status, bool mounted, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"ReqModifyItem(\n\tushort slotID = %u\n\tchar const* hardpoint = %p\n\tint count = %d\n\tfloat status = "
		    L"%f\n\tbool mounted = %d\n\tuint clientId = %u\n)",
		    slotID, hardpoint, count, status, mounted, clientId);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__ReqModifyItem, slotID, hardpoint, count, status, mounted, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqModifyItem(slotID, hardpoint, count, status, mounted, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqModifyItem, slotID, hardpoint, count, status, mounted, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqSetCash(int cash, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqSetCash(\n\tint cash = %d\n\tuint clientId = %u\n)", cash, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqSetCash, cash, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqSetCash(cash, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqSetCash, cash, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall ReqChangeCash(int cashAdd, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"ReqChangeCash(\n\tint cashAdd = %d\n\tuint clientId = %u\n)", cashAdd, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__ReqChangeCash, cashAdd, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.ReqChangeCash(cashAdd, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__ReqChangeCash, cashAdd, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall BaseEnter(uint baseID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"BaseEnter(\n\tuint baseID = %u\n\tuint clientId = %u\n)", baseID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseEnter, baseID, clientId);

		CHECK_FOR_DISCONNECT;

		BaseEnter__Inner(baseID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.BaseEnter(baseID, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		BaseEnter__InnerAfter(baseID, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__BaseEnter, baseID, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall BaseExit(uint baseID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"BaseExit(\n\tuint baseID = %u\n\tuint clientId = %u\n)", baseID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__BaseExit, baseID, clientId);

		CHECK_FOR_DISCONNECT;

		BaseExit__Inner(baseID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.BaseExit(baseID, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		BaseExit__InnerAfter(baseID, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__BaseExit, baseID, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall LocationEnter(uint locationID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"LocationEnter(\n\tuint locationID = %u\n\tuint clientId = %u\n)", locationID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationEnter, locationID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.LocationEnter(locationID, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationEnter, locationID, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall LocationExit(uint locationID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"LocationExit(\n\tuint locationID = %u\n\tuint clientId = %u\n)", locationID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__LocationExit, locationID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.LocationExit(locationID, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__LocationExit, locationID, clientId);
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
	void __stdcall GFGoodVaporized(SGFGoodVaporizedInfo const& gvi, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GFGoodVaporized(\n\tSGFGoodVaporizedInfo const& gvi = %s\n\tuint clientId = %u\n)",
		    ToLogString(gvi), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodVaporized, gvi, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFGoodVaporized(gvi, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodVaporized, gvi, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall MissionResponse(unsigned int _genArg1, unsigned long _genArg2, bool _genArg3, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"MissionResponse(\n\tunsigned int _genArg1 = %u\n\tunsigned long _genArg2 = %u\n\tbool _genArg3 = "
		    L"%d\n\tuint clientId = %u\n)",
		    _genArg1, _genArg2, _genArg3, clientId);

		auto skip =
		    CallPluginsBefore<void>(HookedCall::IServerImpl__MissionResponse, _genArg1, _genArg2, _genArg3, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.MissionResponse(_genArg1, _genArg2, _genArg3, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__MissionResponse, _genArg1, _genArg2, _genArg3, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall TradeResponse(unsigned char const* _genArg1, int _genArg2, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"TradeResponse(\n\tunsigned char const* _genArg1 = %p\n\tint _genArg2 = %d\n\tuint clientId = %u\n)",
		    _genArg1, _genArg2, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TradeResponse, _genArg1, _genArg2, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.TradeResponse(_genArg1, _genArg2, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__TradeResponse, _genArg1, _genArg2, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall GFGoodBuy(SGFGoodBuyInfo const& _genArg1, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GFGoodBuy(\n\tSGFGoodBuyInfo const& _genArg1 = %s\n\tuint clientId = %u\n)", ToLogString(_genArg1),
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodBuy, _genArg1, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFGoodBuy(_genArg1, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodBuy, _genArg1, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall GFGoodSell(SGFGoodSellInfo const& _genArg1, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"GFGoodSell(\n\tSGFGoodSellInfo const& _genArg1 = %s\n\tuint clientId = %u\n)",
		    ToLogString(_genArg1), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__GFGoodSell, _genArg1, clientId);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = GFGoodSell__Inner(_genArg1, clientId);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.GFGoodSell(_genArg1, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__GFGoodSell, _genArg1, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SystemSwitchOutComplete(uint shipID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SystemSwitchOutComplete(\n\tuint shipID = %u\n\tuint clientId = %u\n)", shipID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SystemSwitchOutComplete, shipID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SystemSwitchOutComplete(shipID, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		SystemSwitchOutComplete__InnerAfter(shipID, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__SystemSwitchOutComplete, shipID, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall PlayerLaunch(uint shipID, uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"PlayerLaunch(\n\tuint shipID = %u\n\tuint clientId = %u\n)", shipID, clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PlayerLaunch, shipID, clientId);

		CHECK_FOR_DISCONNECT;

		PlayerLaunch__Inner(shipID, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.PlayerLaunch(shipID, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		PlayerLaunch__InnerAfter(shipID, clientId);

		CallPluginsAfter(HookedCall::IServerImpl__PlayerLaunch, shipID, clientId);
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
	void __stdcall SPObjUpdate(SSPObjUpdateInfo const& ui, uint clientId)
	{
		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjUpdate, ui, clientId);

		CHECK_FOR_DISCONNECT;

		bool innerCheck = SPObjUpdate__Inner(ui, clientId);
		if (!innerCheck)
			return;
		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPObjUpdate(ui, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPObjUpdate, ui, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SPMunitionCollision(SSPMunitionCollisionInfo const& mci, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SPMunitionCollision(\n\tSSPMunitionCollisionInfo const& mci = %s\n\tuint clientId = %u\n)",
		    ToLogString(mci), clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPMunitionCollision, mci, clientId);

		CHECK_FOR_DISCONNECT;

		SPMunitionCollision__Inner(mci, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPMunitionCollision(mci, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPMunitionCollision, mci, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SPObjCollision(SSPObjCollisionInfo const& oci, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SPObjCollision(\n\tSSPObjCollisionInfo const& oci = %s\n\tuint clientId = %u\n)", ToLogString(oci),
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPObjCollision, oci, clientId);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPObjCollision(oci, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPObjCollision, oci, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SPRequestUseItem(SSPUseItem const& ui, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SPRequestUseItem(\n\tSSPUseItem const& ui = %s\n\tuint clientId = %u\n)", ToLogString(ui),
		    clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestUseItem, ui, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPRequestUseItem(ui, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPRequestUseItem, ui, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SPRequestInvincibility(uint shipID, bool enable, InvincibilityReason reason, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"SPRequestInvincibility(\n\tuint shipID = %u\n\tbool enable = %d\n\tInvincibilityReason reason = "
		    L"%s\n\tuint clientId = %u\n)",
		    shipID, enable, ToLogString(reason), clientId);

		auto skip =
		    CallPluginsBefore<void>(HookedCall::IServerImpl__SPRequestInvincibility, shipID, enable, reason, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SPRequestInvincibility(shipID, enable, reason, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SPRequestInvincibility, shipID, enable, reason, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestEvent(
	    int eventType, uint shipID, uint dockTarget, uint _genArg1, ulong _genArg2, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"RequestEvent(\n\tint eventType = %d\n\tuint shipID = %u\n\tuint dockTarget = %u\n\tuint _genArg1 = "
		    L"%u\n\tulong _genArg2 = %u\n\tuint clientId = %u\n)",
		    eventType, shipID, dockTarget, _genArg1, _genArg2, clientId);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__RequestEvent, eventType, shipID, dockTarget, _genArg1, _genArg2, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestEvent(eventType, shipID, dockTarget, _genArg1, _genArg2, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(
		    HookedCall::IServerImpl__RequestEvent, eventType, shipID, dockTarget, _genArg1, _genArg2, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestCancel(int eventType, uint shipID, uint _genArg1, ulong _genArg2, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"RequestCancel(\n\tint eventType = %d\n\tuint shipID = %u\n\tuint _genArg1 = %u\n\tulong _genArg2 = "
		    L"%u\n\tuint clientId = %u\n)",
		    eventType, shipID, _genArg1, _genArg2, clientId);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__RequestCancel, eventType, shipID, _genArg1, _genArg2, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestCancel(eventType, shipID, _genArg1, _genArg2, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestCancel, eventType, shipID, _genArg1, _genArg2, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall MineAsteroid(uint systemID, Vector const& pos, uint crateID, uint lootID, uint count, uint clientId)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug,
		    L"MineAsteroid(\n\tuint systemID = %u\n\tVector const& pos = %s\n\tuint crateID = %u\n\tuint lootID = "
		    L"%u\n\tuint count = %u\n\tuint clientId = %u\n)",
		    systemID, ToLogString(pos), crateID, lootID, count, clientId);

		auto skip = CallPluginsBefore<void>(
		    HookedCall::IServerImpl__MineAsteroid, systemID, pos, crateID, lootID, count, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.MineAsteroid(systemID, pos, crateID, lootID, count, clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__MineAsteroid, systemID, pos, crateID, lootID, count, clientId);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestCreateShip(uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"RequestCreateShip(\n\tuint clientId = %u\n)", clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestCreateShip, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestCreateShip(clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestCreateShip, clientId);
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
	void __stdcall SetManeuver(uint clientId, XSetManeuver const& sm)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SetManeuver(\n\tuint clientId = %u\n\tXSetManeuver const& sm = %s\n)", clientId, ToLogString(sm));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetManeuver, clientId, sm);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetManeuver(clientId, sm);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetManeuver, clientId, sm);
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
	void __stdcall AbortMission(uint clientId, uint _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"AbortMission(\n\tuint clientId = %u\n\tuint _genArg1 = %u\n)", clientId, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AbortMission, clientId, _genArg1);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.AbortMission(clientId, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AbortMission, clientId, _genArg1);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetWeaponGroup(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SetWeaponGroup(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)", clientId,
		    _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetWeaponGroup, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetWeaponGroup(clientId, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetWeaponGroup, clientId, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetVisitedState(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SetVisitedState(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)", clientId,
		    _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetVisitedState, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetVisitedState(clientId, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetVisitedState, clientId, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestBestPath(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"RequestBestPath(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)", clientId,
		    _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestBestPath, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestBestPath(clientId, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestBestPath, clientId, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestPlayerStats(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"RequestPlayerStats(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientId, _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestPlayerStats, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestPlayerStats(clientId, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestPlayerStats, clientId, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall PopupDialog(uint clientId, uint buttonClicked)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"PopupDialog(\n\tuint clientId = %u\n\tuint buttonClicked = %u\n)", clientId, buttonClicked);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__PopupDialog, clientId, buttonClicked);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.PopUpDialog(clientId, buttonClicked);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__PopupDialog, clientId, buttonClicked);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestGroupPositions(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"RequestGroupPositions(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientId, _genArg1, _genArg2);

		auto skip =
		    CallPluginsBefore<void>(HookedCall::IServerImpl__RequestGroupPositions, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestGroupPositions(clientId, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestGroupPositions, clientId, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetInterfaceState(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"SetInterfaceState(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)",
		    clientId, _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetInterfaceState, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetInterfaceState(clientId, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetInterfaceState, clientId, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall RequestRankLevel(uint clientId, uchar* _genArg1, int _genArg2)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"RequestRankLevel(\n\tuint clientId = %u\n\tuchar* _genArg1 = %p\n\tint _genArg2 = %d\n)", clientId,
		    _genArg1, _genArg2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__RequestRankLevel, clientId, _genArg1, _genArg2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.RequestRankLevel(clientId, _genArg1, _genArg2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__RequestRankLevel, clientId, _genArg1, _genArg2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall InitiateTrade(uint clientId1, uint clientId2)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"InitiateTrade(\n\tuint clientId1 = %u\n\tuint clientId2 = %u\n)", clientId1, clientId2);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__InitiateTrade, clientId1, clientId2);

		InitiateTrade__Inner(clientId1, clientId2);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.InitiateTrade(clientId1, clientId2);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__InitiateTrade, clientId1, clientId2);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall TerminateTrade(uint clientId, int accepted)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"TerminateTrade(\n\tuint clientId = %u\n\tint accepted = %d\n)", clientId, accepted);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__TerminateTrade, clientId, accepted);

		CHECK_FOR_DISCONNECT;

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.TerminateTrade(clientId, accepted);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}
		TerminateTrade__InnerAfter(clientId, accepted);

		CallPluginsAfter(HookedCall::IServerImpl__TerminateTrade, clientId, accepted);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall AcceptTrade(uint clientId, bool _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"AcceptTrade(\n\tuint clientId = %u\n\tbool _genArg1 = %d\n)", clientId, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AcceptTrade, clientId, _genArg1);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.AcceptTrade(clientId, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AcceptTrade, clientId, _genArg1);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall SetTradeMoney(uint clientId, ulong _genArg1)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"SetTradeMoney(\n\tuint clientId = %u\n\tulong _genArg1 = %u\n)", clientId, _genArg1);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SetTradeMoney, clientId, _genArg1);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.SetTradeMoney(clientId, _genArg1);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__SetTradeMoney, clientId, _genArg1);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall AddTradeEquip(uint clientId, EquipDesc const& ed)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"AddTradeEquip(\n\tuint clientId = %u\n\tEquipDesc const& ed = %s\n)", clientId, ToLogString(ed));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__AddTradeEquip, clientId, ed);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.AddTradeEquip(clientId, ed);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__AddTradeEquip, clientId, ed);
	}
} // namespace HkIServerImpl

namespace HkIServerImpl
{
	void __stdcall DelTradeEquip(uint clientId, EquipDesc const& ed)
	{
		AddLog(
		    LogType::Normal, LogLevel::Debug, L"DelTradeEquip(\n\tuint clientId = %u\n\tEquipDesc const& ed = %s\n)", clientId, ToLogString(ed));

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__DelTradeEquip, clientId, ed);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.DelTradeEquip(clientId, ed);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__DelTradeEquip, clientId, ed);
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
	void __stdcall StopTradeRequest(uint clientId)
	{
		AddLog(LogType::Normal, LogLevel::Debug, L"StopTradeRequest(\n\tuint clientId = %u\n)", clientId);

		auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__StopTradeRequest, clientId);

		if (!skip)
		{
			CALL_SERVER_PREAMBLE
			{
				Server.StopTradeRequest(clientId);
			}
			CALL_SERVER_POSTAMBLE(true, );
		}

		CallPluginsAfter(HookedCall::IServerImpl__StopTradeRequest, clientId);
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