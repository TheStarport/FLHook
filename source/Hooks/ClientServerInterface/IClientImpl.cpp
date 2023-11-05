#include "PCH.hpp"

#include "Global.hpp"

bool IClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(ClientId client, XFireWeaponInfo& fwi)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnFireWeaponPacket, client, fwi);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_FIREWEAPON(client, fwi); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnFireWeaponPacketAfter, client, fwi);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(ClientId client, XActivateEquip& aq)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnActivateEquipPacket, client, aq);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_ACTIVATEEQUIP(client, aq); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnActivateEquipPacketAfter, client, aq);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(ClientId client, XActivateCruise& aq)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnActivateCruisePacket, client, aq);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_ACTIVATECRUISE(client, aq); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnActivateCruisePacketAfter, client, aq);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(ClientId client, XActivateThrusters& aq)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnActivateThrusterPacket, client, aq);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(client, aq); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnActivateThrusterPacketAfter, client, aq);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SETTARGET(ClientId client, XSetTarget& st)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_SETTARGET(\n\tClientId client = {}\n)", client));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_SETTARGET(client, st); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::unknown_6(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_6(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_6(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(ClientId client, XGoTradelane& tl)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(\n\tClientId client = {}\n)", client));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_GOTRADELANE(client, tl); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(ClientId client, uint shipId, uint archTradelane1, uint archTradelane2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(\n\tClientId client = {}\n\tuint shipId = {}\n\tuint archTradelane1 = {}\n\tuint "
                    L"archTradelane2 = {}\n)",
                    client,
                    shipId,
                    archTradelane1,
                    archTradelane2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_STOPTRADELANE(client, shipId, archTradelane1, archTradelane2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(ClientId client, XJettisonCargo& jc)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(\n\tClientId client = {}\n)", client));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_JETTISONCARGO(client, jc); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::SendPacket(ClientId client, void* _genArg1)
{
    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = SendPacket(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Startup(uint _genArg1, uint _genArg2)
{
    // TODO: Rewrite this base loading code
    CoreGlobals::i()->allBases.clear();
    Universe::IBase* base = Universe::GetFirstBase();
    while (base)
    {
        BaseInfo bi;
        bi.destroyed = false;
        bi.objectId = base->spaceObjId;
        const char* name = "";
        __asm {
            pushad
            mov ecx, [base]
            mov eax, [base]
            mov eax, [eax]
            call [eax+4]
            mov [name], eax
            popad
        }

        bi.baseName = StringUtils::stows(name);
        bi.baseId = CreateID(name);
        CoreGlobals::i()->allBases.push_back(bi);
        pub::System::LoadSystem(base->systemId);

        base = Universe::GetNextBase();
    }

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Startup(_genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::nullsub(uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::nullsub(\n\tuint _genArg1 = {}\n)", _genArg1));

    CALL_CLIENT_PREAMBLE { nullsub(_genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(\n\tClientId client = {}\n)", client));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_LOGINRESPONSE(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(\n\tClientId client = {}\n)", client));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CHARACTERINFO(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(\n\tClientId client = {}\n)", client));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CHARSELECTVERIFIED(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::Shutdown()
{
    CALL_CLIENT_PREAMBLE { Shutdown(); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::CDPClientProxy__Disconnect(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::CDPClientProxy__Disconnect(\n\tClientId client = {}\n)", client));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = CDPClientProxy__Disconnect(client); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

uint IClientImpl::CDPClientProxy__GetSendQSize(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::CDPClientProxy__GetSendQSize(\n\tClientId client = {}\n)", client));

    uint retVal;
    CALL_CLIENT_PREAMBLE { retVal = CDPClientProxy__GetSendQSize(client); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

uint IClientImpl::CDPClientProxy__GetSendQBytes(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::CDPClientProxy__GetSendQBytes(\n\tClientId client = {}\n)", client));

    uint retVal;
    CALL_CLIENT_PREAMBLE { retVal = CDPClientProxy__GetSendQBytes(client); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

double IClientImpl::CDPClientProxy__GetLinkSaturation(ClientId client)
{
    double retVal = 0.0;
    CALL_CLIENT_PREAMBLE { retVal = CDPClientProxy__GetLinkSaturation(client); }
    CALL_CLIENT_POSTAMBLE;
    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(ClientId client, uint shipArch)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(\n\tClientId client = {}\n\tuint shipArch = {}\n)", client, shipArch));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetShipArchPacket, client, shipArch);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETSHIPARCH(client, shipArch); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnSetShipArchPacketAfter, client, shipArch);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(ClientId client, float status)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETHULATUS(\n\tClientId client = {}\n\tfloat status = {}\n)", client, status));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetHullStatusPacket, client, status);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETHULLSTATUS(client, status); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnSetHullStatusPacketAfter, client, status);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(ClientId client, st6::list<XCollision>& collisionGroupList)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetCollisionGroupsPacket, client, collisionGroupList);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(client, collisionGroupList); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnSetCollisionGroupsPacketAfter, client, collisionGroupList);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(ClientId client, st6::vector<EquipDesc>& equipmentVector)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetEquipmentPacket, client, equipmentVector);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETEQUIPMENT(client, equipmentVector); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnSetEquipmentPacketAfter, client, equipmentVector);

    return retVal;
}

void IClientImpl::unknown_26(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_26(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_26(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETADDITEM(ClientId client, FLPACKET_UNKNOWN& _genArg1, FLPACKET_UNKNOWN& _genArg2)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETADDITEM(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetAddItemPacket, client, _genArg1, _genArg2);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETADDITEM(client, _genArg1, _genArg2); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnSetAddItemPacketAfter, client, _genArg1, _genArg2);

    return retVal;
}

void IClientImpl::unknown_28(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_28(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n\tuint _genArg3 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2,
                                 _genArg3));

    CALL_CLIENT_PREAMBLE { unknown_28(client, _genArg1, _genArg2, _genArg3); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetStartRoomPacket, client, _genArg1, _genArg2);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETSTARTROOM(client, _genArg1, _genArg2); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnSetStartRoomPacketAfter, client, _genArg1, _genArg2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
                    client,
                    _genArg1,
                    _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFDESTROYCHARACTER(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFUPDATECHAR(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(\n\tClientId client = {}\n\tuint _genArg1 = "
                                 L"{}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(ClientId client, uint _genArg1)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::unknown_36(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_36(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2));

    CALL_CLIENT_PREAMBLE { unknown_36(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_37(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_37(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2));

    CALL_CLIENT_PREAMBLE { unknown_37(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(ClientId client, uint _genArg1)
{
    Logger::i()->Log(
        LogLevel::Trace,

        std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(\n\tClientId client = {}\n\tuint _genArg1 = "
                                 L"{}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
                    client,
                    _genArg1,
                    _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(\n\tClientId client = {}\n\tuint _genArg1 = "
                                 L"{}\n)",
                                 client,
                                 _genArg1));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(\n\tClientId client = {}\n\tuint _genArg1 = "
                                 L"{}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(ClientId client, uint reason)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(\n\tClientId client = {}\n\tuint reason = {}\n)", client, reason));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(client, reason); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::unknown_44(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_44(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2));

    CALL_CLIENT_PREAMBLE { unknown_44(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(\n\tClientId client = {}\n\tuint _genArg1 = "
                                 L"{}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(\n\tClientId client = {}\n\tuint _genArg1 = "
                                 L"{}\n)",
                                 client,
                                 _genArg1));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(ClientId client, FLPACKET_CREATESOLAR& solar)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateSolarPacket, client, solar);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATESOLAR(client, solar); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnCreateSolarPacketAfter, client, solar);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATESHIP(ClientId client, FLPACKET_CREATESHIP& ship)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATESHIP(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateShipPacket, client, ship);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATESHIP(client, ship); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnCreateShipPacketAfter, client, ship);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATELOOT(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATELOOT(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateLootPacket, client, _genArg1);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATELOOT(client, _genArg1); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnCreateLootPacketAfter, client, _genArg1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATEMINE(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATEMINE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateMinePacket, client, _genArg1);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATEMINE(client, _genArg1); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnCreateMinePacketAfter, client, _genArg1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(uint& client, FLPACKET_CREATEGUIDED& guided)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateGuidedPacket, client, guided);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATEGUIDED(client, guided); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnCreateGuidedPacketAfter, client, guided);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateCounterPacket, client, _genArg1);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_CREATECOUNTER(client, _genArg1); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnCreateCounterPacketAfter, client, _genArg1);

    return retVal;
}

void IClientImpl::unknown_53(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_53(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_53(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_54(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_54(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n\tuint _genArg3 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2,
                                 _genArg3));

    CALL_CLIENT_PREAMBLE { unknown_54(client, _genArg1, _genArg2, _genArg3); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_UPDATEOBJECT(ClientId client, SSPObjUpdateInfo& update)
{
    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnUpdateObjectPacket, client, update);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_UPDATEOBJECT(client, update); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnUpdateObjectPacketAfter, client, update);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(ClientId client, FLPACKET_DESTROYOBJECT& destroy)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnDestroyObjectPacket, client, destroy);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_DESTROYOBJECT(client, destroy); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnDestroyObjectPacketAfter, client, destroy);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(ClientId client, XActivateEquip& aq)
{
    Logger::i()->Log(LogLevel::Trace,

                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnActivateObjectPacket, client, aq);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_ACTIVATEOBJECT(client, aq); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnActivateObjectPacketAfter, client, aq);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(ClientId client, FLPACKET_SYSTEM_SWITCH_OUT& systemSwitchOut)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(\n\tClientId client = {}\n)", client));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(client, systemSwitchOut); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(ClientId client, FLPACKET_SYSTEM_SWITCH_IN& systemSwitchIn)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(\n\tClientId client = {}\n)", client));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(client, systemSwitchIn); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_LAND(ClientId client, FLPACKET_LAND& land)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_LAND(\n\tClientId client = {}\n)", client));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_LAND(client, land); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_LAUNCH(ClientId client, FLPACKET_LAUNCH& launch)
{
    Logger::i()->Log(LogLevel::Trace,

                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_LAUNCH(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnLaunchPacket, client, launch);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_LAUNCH(client, launch); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnLaunchPacketAfter, client, launch);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(ClientId client, bool response, uint shipId)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(\n\tClientId client = {}\n\tbool response = {}\n\tuint shipId = {}\n)",
                    client,
                    response,
                    shipId));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnRequestCreateShipResponsePacket, client, response, shipId);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(client, response, shipId); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnRequestCreateShipResponsePacketAfter, client, response, shipId);

    return retVal;
}

void IClientImpl::unknown_63(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_63(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_63(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(ClientId client, uint objId, DamageList& dmgList)
{
    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_DAMAGEOBJECT(client, objId, dmgList); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_ITEMTRACTORED(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_USE_ITEM(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_USE_ITEM(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnUseItemPacket, client, _genArg1);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_USE_ITEM(client, _genArg1); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnUseItemPacketAfter, client, _genArg1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(ClientId client, FLPACKET_SETREPUTATION& rep)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetReputationPacket, client, rep);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETREPUTATION(client, rep); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnSetReputationPacketAfter, client, rep);

    return retVal;
}

void IClientImpl::unknown_68(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_68(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_68(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SENDCOMM(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6,
                                                uint _genArg7, uint _genArg8, uint _genArg9, uint _genArg10, uint _genArg11, uint _genArg12, uint _genArg13,
                                                uint _genArg14, uint _genArg15, uint _genArg16, uint _genArg17, uint _genArg18, uint _genArg19, uint _genArg20,
                                                uint _genArg21, uint _genArg22)
{
    bool retVal;
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_SENDCOMM(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = "
                                 L"{}\n\tuint _genArg3 = {}\n\tuint _genArg4 = {}\n\tuint _genArg5 = {}\n\tuint _genArg6 = {}\n\tuint _genArg7 "
                                 L"= {}\n\tuint _genArg8 = {}\n\tuint _genArg9 = {}\n\tuint _genArg10 = {}\n\tuint _genArg11 = {}\n\tuint "
                                 L"_genArg12 = {}\n\tuint _genArg13 = {}\n\tuint _genArg14 = {}\n\tuint _genArg15 = {}\n\tuint _genArg16 = "
                                 L"{}\n\tuint _genArg17 = {}\n\tuint _genArg18 = {}\n\tuint _genArg19 = {}\n\tuint _genArg20 = {}\n\tuint "
                                 L"_genArg21 = {}\n\tuint _genArg22 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2,
                                 _genArg3,
                                 _genArg4,
                                 _genArg5,
                                 _genArg6,
                                 _genArg7,
                                 _genArg8,
                                 _genArg9,
                                 _genArg10,
                                 _genArg11,
                                 _genArg12,
                                 _genArg13,
                                 _genArg14,
                                 _genArg15,
                                 _genArg16,
                                 _genArg17,
                                 _genArg18,
                                 _genArg19,
                                 _genArg20,
                                 _genArg21,
                                 _genArg22));
    CALL_CLIENT_PREAMBLE
    {
        retVal = Send_FLPACKET_SERVER_SENDCOMM(client,
                                               _genArg1,
                                               _genArg2,
                                               _genArg3,
                                               _genArg4,
                                               _genArg5,
                                               _genArg6,
                                               _genArg7,
                                               _genArg8,
                                               _genArg9,
                                               _genArg10,
                                               _genArg11,
                                               _genArg12,
                                               _genArg13,
                                               _genArg14,
                                               _genArg15,
                                               _genArg16,
                                               _genArg17,
                                               _genArg18,
                                               _genArg19,
                                               _genArg20,
                                               _genArg21,
                                               _genArg22);
    }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_70(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_70(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_70(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetMissionMessagePacket, client, _genArg1);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(client, _genArg1); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnSetMissionMessagePacketAfter, client, _genArg1);

    return retVal;
}

void IClientImpl::unknown_72(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_72(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_72(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(ClientId client, uint _genArg1)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetMissionObjectivesPacket, client, _genArg1);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(client, _genArg1); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnSetMissionObjectivesPacketAfter, client, _genArg1);

    return retVal;
}

void IClientImpl::unknown_74(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_74(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_74(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_75(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_75(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_75(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_MARKOBJ(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_SERVER_MARKOBJ(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MARKOBJ(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::unknown_77(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_77(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_77(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETCASH(ClientId client, uint cash)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETCASH(\n\tClientId client = {}\n\tuint cash = {}\n)", client, cash));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetCashPacket, client, cash);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SETCASH(client, cash); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnSetCashPacketAfter, client, cash);

    return retVal;
}

void IClientImpl::unknown_79(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_79(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_79(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_80(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_80(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_80(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_81(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_81(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_81(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_82(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_82(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_82(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_83(ClientId client, char* _genArg1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_83(\n\tClientId client = {}\n\tchar* _genArg1 = {}\n)", client, StringUtils::stows(_genArg1)));

    CALL_CLIENT_PREAMBLE { unknown_83(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(uint& client, uint shipId, uint flag, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(\n\tClientId client = {}\n\tuint shipId = {}\n\tuint flag = {}\n\tuint _genArg1 "
                    L"= {}\n\tuint _genArg2 = {}\n)",
                    client,
                    shipId,
                    flag,
                    _genArg1,
                    _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_REQUEST_RETURNED(client, shipId, flag, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::unknown_85(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_85(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_85(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_86(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_86(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n\tuint _genArg3 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2,
                                 _genArg3));

    CALL_CLIENT_PREAMBLE { unknown_86(client, _genArg1, _genArg2, _genArg3); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(SObjectCargoUpdate& cargoUpdate, uint dunno1, uint dunno2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(\n\tSObjectCargoUpdate client = {}\n\tuint dunno1 = {}\n\tuint dunno2 = {}\n)",
                    cargoUpdate.client,
                    dunno1,
                    dunno2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(cargoUpdate, dunno1, dunno2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_BURNFUSE(ClientId client, FLPACKET_BURNFUSE& burnFuse)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_BURNFUSE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnBurnFusePacket, client, burnFuse);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_BURNFUSE(client, burnFuse); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnBurnFusePacketAfter, client, burnFuse);

    return retVal;
}

void IClientImpl::unknown_89(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_89(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_89(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_90(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_90(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_90(client); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_91(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_91(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_91(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(ClientId client, uint _genArg1, int _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_SET_WEAPON_GROUP(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(ClientId client, uint objHash, int state)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(\n\tClientId client = {}\n\tuint objHash = {}\n\tint state = {}\n)", client, objHash, state));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_SET_VISITED_STATE(client, objHash, state); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(ClientId client, uint objHash, int _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(\n\tClientId client = {}\n\tobjHash = {}\n\tint _genArg2 = {}\n)",
                                 client,
                                 objHash,
                                 _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_REQUEST_BEST_PATH(client, objHash, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(ClientId client, uint _genArg1, int _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tint _genArg2 = {}\n)",
                    client,
                    _genArg1,
                    _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::unknown_96(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_96(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n\tuint _genArg3 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2,
                                 _genArg3));

    CALL_CLIENT_PREAMBLE { unknown_96(client, _genArg1, _genArg2, _genArg3); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(ClientId client, uint _genArg1, int _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tint _genArg2 = {}\n)",
                    client,
                    _genArg1,
                    _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(ClientId client, uint _genArg1, int _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_SET_MISSION_LOG(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(ClientId client, uint _genArg1, int _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tint _genArg2 = {}\n)",
                    client,
                    _genArg1,
                    _genArg2));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_SET_INTERFACE_STATE(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::unknown_100(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_100(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2));

    CALL_CLIENT_PREAMBLE { unknown_100(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_101(uint& client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_101(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_101(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_102(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_102(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_102(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_103(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_103(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_103(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_104(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_104(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2));

    CALL_CLIENT_PREAMBLE { unknown_104(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_105(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_105(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2));

    CALL_CLIENT_PREAMBLE { unknown_105(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_106(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_106(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2));

    CALL_CLIENT_PREAMBLE { unknown_106(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_107(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_107(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)", client, _genArg1, _genArg2));

    CALL_CLIENT_PREAMBLE { unknown_107(client, _genArg1, _genArg2); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_COMMON_PLAYER_TRADE(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::unknown_109(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_109(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_109(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnScanNotifyPacket, client, _genArg1, _genArg2);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_SCANNOTIFY(client, _genArg1, _genArg2); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnScanNotifyPacketAfter, client, _genArg1, _genArg2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(ClientId client, wchar_t* characterName, uint _genArg2, char _genArg3)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(\n\tClientId client = {}\n\twchar_t* characterName = \n\tuint _genArg2 = {}\n)",
                                 client,
                                 std::wstring(characterName),
                                 _genArg2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnPlayerListPacket, client, characterName, _genArg2, _genArg3);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_PLAYERLIST(client, characterName, _genArg2, _genArg3); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnPlayerListPacketAfter, client, characterName, _genArg2, _genArg3);

    return retVal;
}

void IClientImpl::unknown_112(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_112(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_112(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnPlayerList2Packet, client);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_PLAYERLIST_2(client); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnPlayerList2PacketAfter, client);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate6Packet, client, _genArg1, _genArg2);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_6(client, _genArg1, _genArg2); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate6PacketAfter, client, _genArg1, _genArg2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate7Packet, client, _genArg1, _genArg2);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_7(client, _genArg1, _genArg2); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate7PacketAfter, client, _genArg1, _genArg2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(ClientId client, FLPACKET_UNKNOWN& _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdatePacket, client, _genArg1);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE(client, _genArg1); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdatePacketAfter, client, _genArg1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate2Packet, client, _genArg1, _genArg2);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_2(client, _genArg1, _genArg2); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate2PacketAfter, client, _genArg1, _genArg2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(ClientId client, uint targetId, uint rank)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(\n\tClientId client = {}\n\tuint targetId = {}\n\tuint rank = {}\n)", client, targetId, rank));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate3Packet, client, targetId, rank);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_3(client, targetId, rank); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate3PacketAfter, client, targetId, rank);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint "
                                 L"_genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate4Packet, client, _genArg1, _genArg2);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_4(client, _genArg1, _genArg2); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate4PacketAfter, client, _genArg1, _genArg2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(ClientId client, uint _genArg1, uint _genArg2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate5Packet, client, _genArg1, _genArg2);

    if (!skip)
    {
        CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_5(client, _genArg1, _genArg2); }
        CALL_CLIENT_POSTAMBLE;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate5PacketAfter, client, _genArg1, _genArg2);

    return retVal;
}

void IClientImpl::unknown_121(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_121(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_121(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

bool IClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(ClientId client, uint shipId, Vector& formationOffset)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(\n\tClientId client = {}\n\tuint shipId = {}\n)", client, shipId));

    bool retVal;
    CALL_CLIENT_PREAMBLE { retVal = Send_FLPACKET_SERVER_FORMATION_UPDATE(client, shipId, formationOffset); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

void IClientImpl::unknown_123(ClientId client, uint _genArg1, uint _genArg2, uint _genArg3, uint _genArg4, uint _genArg5, uint _genArg6)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_123(\n\tClientId client = {}\n\tuint _genArg1 = {}\n\tuint _genArg2 = {}\n\tuint "
                                 L"_genArg3 = {}\n\tuint _genArg4 = {}\n\tuint _genArg5 = {}\n\tuint _genArg6 = {}\n)",
                                 client,
                                 _genArg1,
                                 _genArg2,
                                 _genArg3,
                                 _genArg4,
                                 _genArg5,
                                 _genArg6));

    CALL_CLIENT_PREAMBLE { unknown_123(client, _genArg1, _genArg2, _genArg3, _genArg4, _genArg5, _genArg6); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_124(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_124(\n\tClientId client = {}\n)", client));

    CALL_CLIENT_PREAMBLE { unknown_124(client); }
    CALL_CLIENT_POSTAMBLE;
}

void IClientImpl::unknown_125(ClientId client, uint _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_125(\n\tClientId client = {}\n\tuint _genArg1 = {}\n)", client, _genArg1));

    CALL_CLIENT_PREAMBLE { unknown_125(client, _genArg1); }
    CALL_CLIENT_POSTAMBLE;
}

int IClientImpl::unknown_126(char* _genArg1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_126(\n\tchar* _genArg1 = {}\n)", StringUtils::stows(_genArg1)));

    int retVal;
    CALL_CLIENT_PREAMBLE { retVal = unknown_126(_genArg1); }
    CALL_CLIENT_POSTAMBLE;

    return retVal;
}

bool IClientImpl::DispatchMsgs()
{
    cdpServer->DispatchMsgs(); // calls IServerImpl functions, which also call
    // IClientImpl functions
    return true;
}
