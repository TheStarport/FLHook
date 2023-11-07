#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"

bool IClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(ClientId client, XFireWeaponInfo& fwi)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnFireWeaponPacket, client, fwi);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_COMMON_FIREWEAPON(client, fwi); }
        CallClientPostamble;
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
        CallClientPreamble { retVal = Send_FLPACKET_COMMON_ACTIVATEEQUIP(client, aq); }
        CallClientPostamble;
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
        CallClientPreamble { retVal = Send_FLPACKET_COMMON_ACTIVATECRUISE(client, aq); }
        CallClientPostamble;
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
        CallClientPreamble { retVal = Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(client, aq); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnActivateThrusterPacketAfter, client, aq);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SETTARGET(ClientId client, XSetTarget& st)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_SETTARGET(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_SETTARGET(client, st); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_6(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_6(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_6(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(ClientId client, XGoTradelane& tl)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_GOTRADELANE(client, tl); }
    CallClientPostamble;

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
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_STOPTRADELANE(client, shipId, archTradelane1, archTradelane2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(ClientId client, XJettisonCargo& jc)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_JETTISONCARGO(client, jc); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::SendPacket(ClientId client, void* unk1)
{
    bool retVal;
    CallClientPreamble { retVal = SendPacket(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Startup(uint unk1, uint unk2)
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
    CallClientPreamble { retVal = Startup(unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::nullsub(uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::nullsub(\n\tuint unk1 = {}\n)", unk1));

    CallClientPreamble { nullsub(unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_LOGINRESPONSE(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_CHARACTERINFO(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_CHARSELECTVERIFIED(client, unk1); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::Shutdown()
{
    CallClientPreamble { Shutdown(); }
    CallClientPostamble;
}

bool IClientImpl::CDPClientProxyDisconnect(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::CDPClientProxyDisconnect(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = CDPClientProxyDisconnect(client); }
    CallClientPostamble;

    return retVal;
}

uint IClientImpl::CDPClientProxyGetSendQSize(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::CDPClientProxyGetSendQSize(\n\tClientId client = {}\n)", client));

    uint retVal;
    CallClientPreamble { retVal = CDPClientProxyGetSendQSize(client); }
    CallClientPostamble;

    return retVal;
}

uint IClientImpl::CDPClientProxyGetSendQBytes(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::CDPClientProxyGetSendQBytes(\n\tClientId client = {}\n)", client));

    uint retVal;
    CallClientPreamble { retVal = CDPClientProxyGetSendQBytes(client); }
    CallClientPostamble;

    return retVal;
}

double IClientImpl::CDPClientProxyGetLinkSaturation(ClientId client)
{
    double retVal = 0.0;
    CallClientPreamble { retVal = CDPClientProxyGetLinkSaturation(client); }
    CallClientPostamble;
    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(ClientId client, uint shipArch)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(\n\tClientId client = {}\n\tuint shipArch = {}\n)", client, shipArch));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetShipArchPacket, client, shipArch);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETSHIPARCH(client, shipArch); }
        CallClientPostamble;
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
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETHULLSTATUS(client, status); }
        CallClientPostamble;
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
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(client, collisionGroupList); }
        CallClientPostamble;
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
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETEQUIPMENT(client, equipmentVector); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetEquipmentPacketAfter, client, equipmentVector);

    return retVal;
}

void IClientImpl::unknown_26(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_26(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_26(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETADDITEM(ClientId client, FLPACKET_UNKNOWN& unk1, FLPACKET_UNKNOWN& unk2)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETADDITEM(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetAddItemPacket, client, unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETADDITEM(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetAddItemPacketAfter, client, unk1, unk2);

    return retVal;
}

void IClientImpl::unknown_28(ClientId client, uint unk1, uint unk2, uint unk3)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_28(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n\tuint unk3 = {}\n)", client, unk1, unk2, unk3));

    CallClientPreamble { unknown_28(client, unk1, unk2, unk3); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetStartRoomPacket, client, unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETSTARTROOM(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetStartRoomPacketAfter, client, unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFDESTROYCHARACTER(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFUPDATECHAR(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(\n\tClientId client = {}\n\tuint unk1 = "
                                 L"{}\n\tuint unk2 = {}\n)",
                                 client,
                                 unk1,
                                 unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(ClientId client, uint unk1)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(client, unk1); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_36(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_36(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_36(client, unk1, unk2); }
    CallClientPostamble;
}

void IClientImpl::unknown_37(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_37(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_37(client, unk1, unk2); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace,

                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(\n\tClientId client = {}\n\tuint unk1 = "
                                 L"{}\n\tuint unk2 = {}\n)",
                                 client,
                                 unk1,
                                 unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(\n\tClientId client = {}\n\tuint unk1 = "
                                 L"{}\n)",
                                 client,
                                 unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(\n\tClientId client = {}\n\tuint unk1 = "
                                 L"{}\n\tuint unk2 = {}\n)",
                                 client,
                                 unk1,
                                 unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(ClientId client, uint reason)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(\n\tClientId client = {}\n\tuint reason = {}\n)", client, reason));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(client, reason); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_44(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_44(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_44(client, unk1, unk2); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(\n\tClientId client = {}\n\tuint unk1 = "
                                 L"{}\n\tuint unk2 = {}\n)",
                                 client,
                                 unk1,
                                 unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(\n\tClientId client = {}\n\tuint unk1 = "
                                 L"{}\n)",
                                 client,
                                 unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(ClientId client, FLPACKET_CREATESOLAR& solar)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateSolarPacket, client, solar);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATESOLAR(client, solar); }
        CallClientPostamble;
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
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATESHIP(client, ship); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateShipPacketAfter, client, ship);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATELOOT(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATELOOT(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateLootPacket, client, unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATELOOT(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateLootPacketAfter, client, unk1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATEMINE(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATEMINE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateMinePacket, client, unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATEMINE(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateMinePacketAfter, client, unk1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(uint& client, FLPACKET_CREATEGUIDED& guided)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateGuidedPacket, client, guided);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATEGUIDED(client, guided); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateGuidedPacketAfter, client, guided);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateCounterPacket, client, unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATECOUNTER(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateCounterPacketAfter, client, unk1);

    return retVal;
}

void IClientImpl::unknown_53(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_53(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_53(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_54(ClientId client, uint unk1, uint unk2, uint unk3)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_54(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n\tuint unk3 = {}\n)", client, unk1, unk2, unk3));

    CallClientPreamble { unknown_54(client, unk1, unk2, unk3); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_COMMON_UPDATEOBJECT(ClientId client, SSPObjUpdateInfo& update)
{
    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnUpdateObjectPacket, client, update);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_COMMON_UPDATEOBJECT(client, update); }
        CallClientPostamble;
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
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_DESTROYOBJECT(client, destroy); }
        CallClientPostamble;
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
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_ACTIVATEOBJECT(client, aq); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnActivateObjectPacketAfter, client, aq);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(ClientId client, FLPACKET_SYSTEM_SWITCH_OUT& systemSwitchOut)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(client, systemSwitchOut); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(ClientId client, FLPACKET_SYSTEM_SWITCH_IN& systemSwitchIn)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(client, systemSwitchIn); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_LAND(ClientId client, FLPACKET_LAND& land)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_LAND(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_LAND(client, land); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_LAUNCH(ClientId client, FLPACKET_LAUNCH& launch)
{
    Logger::i()->Log(LogLevel::Trace,

                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_LAUNCH(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnLaunchPacket, client, launch);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_LAUNCH(client, launch); }
        CallClientPostamble;
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
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(client, response, shipId); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnRequestCreateShipResponsePacketAfter, client, response, shipId);

    return retVal;
}

void IClientImpl::unknown_63(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_63(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_63(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(ClientId client, uint objId, DamageList& dmgList)
{
    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_DAMAGEOBJECT(client, objId, dmgList); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_ITEMTRACTORED(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_USE_ITEM(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_USE_ITEM(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnUseItemPacket, client, unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_USE_ITEM(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnUseItemPacketAfter, client, unk1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(ClientId client, FLPACKET_SETREPUTATION& rep)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetReputationPacket, client, rep);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETREPUTATION(client, rep); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetReputationPacketAfter, client, rep);

    return retVal;
}

void IClientImpl::unknown_68(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_68(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_68(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SENDCOMM(ClientId client, uint unk1, uint unk2, uint unk3, uint unk4, uint unk5, uint unk6, uint unk7, uint unk8,
                                                uint unk9, uint unk10, uint unk11, uint unk12, uint unk13, uint unk14, uint unk15, uint unk16, uint unk17,
                                                uint unk18, uint unk19, uint unk20, uint unk21, uint unk22)
{
    bool retVal;
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_SENDCOMM(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = "
                                 L"{}\n\tuint unk3 = {}\n\tuint unk4 = {}\n\tuint unk5 = {}\n\tuint unk6 = {}\n\tuint unk7 "
                                 L"= {}\n\tuint unk8 = {}\n\tuint unk9 = {}\n\tuint unk10 = {}\n\tuint unk11 = {}\n\tuint "
                                 L"unk12 = {}\n\tuint unk13 = {}\n\tuint unk14 = {}\n\tuint unk15 = {}\n\tuint unk16 = "
                                 L"{}\n\tuint unk17 = {}\n\tuint unk18 = {}\n\tuint unk19 = {}\n\tuint unk20 = {}\n\tuint "
                                 L"unk21 = {}\n\tuint unk22 = {}\n)",
                                 client,
                                 unk1,
                                 unk2,
                                 unk3,
                                 unk4,
                                 unk5,
                                 unk6,
                                 unk7,
                                 unk8,
                                 unk9,
                                 unk10,
                                 unk11,
                                 unk12,
                                 unk13,
                                 unk14,
                                 unk15,
                                 unk16,
                                 unk17,
                                 unk18,
                                 unk19,
                                 unk20,
                                 unk21,
                                 unk22));
    CallClientPreamble
    {
        retVal = Send_FLPACKET_SERVER_SENDCOMM(client,
                                               unk1,
                                               unk2,
                                               unk3,
                                               unk4,
                                               unk5,
                                               unk6,
                                               unk7,
                                               unk8,
                                               unk9,
                                               unk10,
                                               unk11,
                                               unk12,
                                               unk13,
                                               unk14,
                                               unk15,
                                               unk16,
                                               unk17,
                                               unk18,
                                               unk19,
                                               unk20,
                                               unk21,
                                               unk22);
    }
    CallClientPostamble;
}

void IClientImpl::unknown_70(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_70(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_70(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetMissionMessagePacket, client, unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetMissionMessagePacketAfter, client, unk1);

    return retVal;
}

void IClientImpl::unknown_72(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_72(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_72(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetMissionObjectivesPacket, client, unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetMissionObjectivesPacketAfter, client, unk1);

    return retVal;
}

void IClientImpl::unknown_74(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_74(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_74(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_75(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_75(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_75(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_MARKOBJ(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_MARKOBJ(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_MARKOBJ(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_77(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_77(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_77(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETCASH(ClientId client, uint cash)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETCASH(\n\tClientId client = {}\n\tuint cash = {}\n)", client, cash));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetCashPacket, client, cash);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETCASH(client, cash); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetCashPacketAfter, client, cash);

    return retVal;
}

void IClientImpl::unknown_79(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_79(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_79(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_80(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_80(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_80(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_81(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_81(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_81(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_82(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_82(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_82(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_83(ClientId client, char* unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_83(\n\tClientId client = {}\n\tchar* unk1 = {}\n)", client, StringUtils::stows(unk1)));

    CallClientPreamble { unknown_83(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(uint& client, uint shipId, uint flag, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(\n\tClientId client = {}\n\tuint shipId = {}\n\tuint flag = {}\n\tuint unk1 "
                    L"= {}\n\tuint unk2 = {}\n)",
                    client,
                    shipId,
                    flag,
                    unk1,
                    unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_REQUEST_RETURNED(client, shipId, flag, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_85(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_85(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_85(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_86(ClientId client, uint unk1, uint unk2, uint unk3)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_86(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n\tuint unk3 = {}\n)", client, unk1, unk2, unk3));

    CallClientPreamble { unknown_86(client, unk1, unk2, unk3); }
    CallClientPostamble;
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
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(cargoUpdate, dunno1, dunno2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_BURNFUSE(ClientId client, FLPACKET_BURNFUSE& burnFuse)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_BURNFUSE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnBurnFusePacket, client, burnFuse);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_BURNFUSE(client, burnFuse); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnBurnFusePacketAfter, client, burnFuse);

    return retVal;
}

void IClientImpl::unknown_89(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_89(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_89(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_90(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_90(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_90(client); }
    CallClientPostamble;
}

void IClientImpl::unknown_91(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_91(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_91(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(ClientId client, uint unk1, int unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(\n\tClientId client = {}\n\tuint unk1 = {}\n\tint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_SET_WEAPON_GROUP(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(ClientId client, uint objHash, int state)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(\n\tClientId client = {}\n\tuint objHash = {}\n\tint state = {}\n)", client, objHash, state));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_SET_VISITED_STATE(client, objHash, state); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(ClientId client, uint objHash, int unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(\n\tClientId client = {}\n\tobjHash = {}\n\tint unk2 = {}\n)", client, objHash, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_REQUEST_BEST_PATH(client, objHash, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(ClientId client, uint unk1, int unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(\n\tClientId client = {}\n\tuint unk1 = {}\n\tint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_96(ClientId client, uint unk1, uint unk2, uint unk3)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_96(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n\tuint unk3 = {}\n)", client, unk1, unk2, unk3));

    CallClientPreamble { unknown_96(client, unk1, unk2, unk3); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(ClientId client, uint unk1, int unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(\n\tClientId client = {}\n\tuint unk1 = {}\n\tint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(ClientId client, uint unk1, int unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(\n\tClientId client = {}\n\tuint unk1 = {}\n\tint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_SET_MISSION_LOG(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(ClientId client, uint unk1, int unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(\n\tClientId client = {}\n\tuint unk1 = {}\n\tint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_SET_INTERFACE_STATE(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_100(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_100(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_100(client, unk1, unk2); }
    CallClientPostamble;
}

void IClientImpl::unknown_101(uint& client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_101(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_101(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_102(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_102(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_102(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_103(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_103(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_103(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_104(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_104(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_104(client, unk1, unk2); }
    CallClientPostamble;
}

void IClientImpl::unknown_105(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_105(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_105(client, unk1, unk2); }
    CallClientPostamble;
}

void IClientImpl::unknown_106(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_106(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_106(client, unk1, unk2); }
    CallClientPostamble;
}

void IClientImpl::unknown_107(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_107(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_107(client, unk1, unk2); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_PLAYER_TRADE(client, unk1); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_109(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_109(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_109(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnScanNotifyPacket, client, unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SCANNOTIFY(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnScanNotifyPacketAfter, client, unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(ClientId client, wchar_t* characterName, uint unk2, char unk3)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(\n\tClientId client = {}\n\twchar_t* characterName = \n\tuint unk2 = {}\n)",
                                 client,
                                 std::wstring(characterName),
                                 unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnPlayerListPacket, client, characterName, unk2, unk3);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_PLAYERLIST(client, characterName, unk2, unk3); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnPlayerListPacketAfter, client, characterName, unk2, unk3);

    return retVal;
}

void IClientImpl::unknown_112(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_112(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_112(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnPlayerList2Packet, client);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_PLAYERLIST_2(client); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnPlayerList2PacketAfter, client);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate6Packet, client, unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_6(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate6PacketAfter, client, unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate7Packet, client, unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_7(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate7PacketAfter, client, unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(ClientId client, FLPACKET_UNKNOWN& unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdatePacket, client, unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdatePacketAfter, client, unk1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate2Packet, client, unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_2(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate2PacketAfter, client, unk1, unk2);

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
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_3(client, targetId, rank); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate3PacketAfter, client, targetId, rank);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint "
                                 L"unk2 = {}\n)",
                                 client,
                                 unk1,
                                 unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate4Packet, client, unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_4(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate4PacketAfter, client, unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(ClientId client, uint unk1, uint unk2)
{
    Logger::i()->Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate5Packet, client, unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_5(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate5PacketAfter, client, unk1, unk2);

    return retVal;
}

void IClientImpl::unknown_121(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_121(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_121(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(ClientId client, uint shipId, Vector& formationOffset)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(\n\tClientId client = {}\n\tuint shipId = {}\n)", client, shipId));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_FORMATION_UPDATE(client, shipId, formationOffset); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_123(ClientId client, uint unk1, uint unk2, uint unk3, uint unk4, uint unk5, uint unk6)
{
    Logger::i()->Log(LogLevel::Trace,
                     std::format(L"IClientImpl::unknown_123(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n\tuint "
                                 L"unk3 = {}\n\tuint unk4 = {}\n\tuint unk5 = {}\n\tuint unk6 = {}\n)",
                                 client,
                                 unk1,
                                 unk2,
                                 unk3,
                                 unk4,
                                 unk5,
                                 unk6));

    CallClientPreamble { unknown_123(client, unk1, unk2, unk3, unk4, unk5, unk6); }
    CallClientPostamble;
}

void IClientImpl::unknown_124(ClientId client)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_124(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_124(client); }
    CallClientPostamble;
}

void IClientImpl::unknown_125(ClientId client, uint unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_125(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_125(client, unk1); }
    CallClientPostamble;
}

int IClientImpl::unknown_126(char* unk1)
{
    Logger::i()->Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_126(\n\tchar* unk1 = {}\n)", StringUtils::stows(unk1)));

    int retVal;
    CallClientPreamble { retVal = unknown_126(unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::DispatchMsgs()
{
    cdpServer->DispatchMsgs(); // calls IServerImpl functions, which also call
    // IClientImpl functions
    return true;
}
