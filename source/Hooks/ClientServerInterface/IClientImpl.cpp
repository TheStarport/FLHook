#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"

bool IClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(uint client, XFireWeaponInfo& fwi)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnFireWeaponPacket, ClientId(client), fwi);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_COMMON_FIREWEAPON(client, fwi); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnFireWeaponPacketAfter, ClientId(client), fwi);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(uint client, XActivateEquip& aq)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnActivateEquipPacket, ClientId(client), aq);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_COMMON_ACTIVATEEQUIP(client, aq); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnActivateEquipPacketAfter, ClientId(client), aq);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(uint client, XActivateCruise& aq)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnActivateCruisePacket, ClientId(client), aq);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_COMMON_ACTIVATECRUISE(client, aq); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnActivateCruisePacketAfter, ClientId(client), aq);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(uint client, XActivateThrusters& aq)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnActivateThrusterPacket, ClientId(client), aq);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(client, aq); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnActivateThrusterPacketAfter, ClientId(client), aq);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SETTARGET(uint client, XSetTarget& st)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_SETTARGET(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_SETTARGET(client, st); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_6(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_6(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_6(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(uint client, XGoTradelane& tl)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_GOTRADELANE(client, tl); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(uint client, uint shipId, uint archTradelane1, uint archTradelane2)
{
    Logger::Log(LogLevel::Trace,
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

bool IClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(uint client, XJettisonCargo& jc)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_JETTISONCARGO(client, jc); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::SendPacket(uint client, void* unk1)
{
    bool retVal;
    CallClientPreamble { retVal = SendPacket(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Startup(uint unk1, uint unk2)
{
    // TODO: Rewrite this base loading code
    auto bases = FLHook::instance->allBases;
    bases.clear();
    Universe::IBase* base = Universe::GetFirstBase();
    while (base)
    {
        BaseInfo bi;
        bi.destroyed = false;
        bi.objectId = base->spaceObjId;
        auto name = "";

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
        bases.emplace_back(bi);

        base = Universe::GetNextBase();
    }

    Universe::ISystem* system = Universe::GetFirstSystem();
    while (system)
    {
        if (!std::string(system->file).empty())
        {
            pub::System::LoadSystem(system->id);
        }
        else
        {
            Logger::Log(LogLevel::Warn, std::format(L"System {} could not be loaded!", system->nickname));
        }
        system = Universe::GetNextSystem();
    }

    bool retVal;
    CallClientPreamble { retVal = Startup(unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::nullsub(uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::nullsub(\n\tuint unk1 = {}\n)", unk1));

    CallClientPreamble { nullsub(unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_LOGINRESPONSE(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_CHARACTERINFO(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(\n\tClientId client = {}\n)", client));

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

bool IClientImpl::CDPClientProxyDisconnect(uint client)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::CDPClientProxyDisconnect(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = CDPClientProxyDisconnect(client); }
    CallClientPostamble;

    return retVal;
}

uint IClientImpl::CDPClientProxyGetSendQSize(uint client)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::CDPClientProxyGetSendQSize(\n\tClientId client = {}\n)", client));

    uint retVal;
    CallClientPreamble { retVal = CDPClientProxyGetSendQSize(client); }
    CallClientPostamble;

    return retVal;
}

uint IClientImpl::CDPClientProxyGetSendQBytes(uint client)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::CDPClientProxyGetSendQBytes(\n\tClientId client = {}\n)", client));

    uint retVal;
    CallClientPreamble { retVal = CDPClientProxyGetSendQBytes(client); }
    CallClientPostamble;

    return retVal;
}

double IClientImpl::CDPClientProxyGetLinkSaturation(uint client)
{
    double retVal = 0.0;
    CallClientPreamble { retVal = CDPClientProxyGetLinkSaturation(client); }
    CallClientPostamble;
    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(uint client, uint shipArch)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(\n\tClientId client = {}\n\tuint shipArch = {}\n)", client, shipArch));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetShipArchPacket, ClientId(client), shipArch);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETSHIPARCH(client, shipArch); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetShipArchPacketAfter, ClientId(client), shipArch);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(uint client, float status)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETHULATUS(\n\tClientId client = {}\n\tfloat status = {}\n)", client, status));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetHullStatusPacket, ClientId(client), status);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETHULLSTATUS(client, status); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetHullStatusPacketAfter, ClientId(client), status);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(uint client, st6::list<XCollision>& collisionGroupList)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetCollisionGroupsPacket, ClientId(client), collisionGroupList);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(client, collisionGroupList); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetCollisionGroupsPacketAfter, ClientId(client), collisionGroupList);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(uint client, st6::vector<EquipDesc>& equipmentVector)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetEquipmentPacket, ClientId(client), equipmentVector);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETEQUIPMENT(client, equipmentVector); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetEquipmentPacketAfter, ClientId(client), equipmentVector);

    return retVal;
}

void IClientImpl::unknown_26(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_26(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_26(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETADDITEM(uint client, FLPACKET_UNKNOWN* unk1, FLPACKET_UNKNOWN* unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETADDITEM(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetAddItemPacket, ClientId(client), unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETADDITEM(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetAddItemPacketAfter, ClientId(client), unk1, unk2);

    return retVal;
}

void IClientImpl::unknown_28(uint client, uint unk1, uint unk2, uint unk3)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_28(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n\tuint unk3 = {}\n)", client, unk1, unk2, unk3));

    CallClientPreamble { unknown_28(client, unk1, unk2, unk3); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(uint client, uint unk1, uint unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetStartRoomPacket, ClientId(client), unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETSTARTROOM(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetStartRoomPacketAfter, ClientId(client), unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(uint client, uint unk1, uint unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFDESTROYCHARACTER(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(uint client, uint unk1, uint unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFUPDATECHAR(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(uint client, uint unk1, uint unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace,
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

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(client, unk1); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_36(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_36(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_36(client, unk1, unk2); }
    CallClientPostamble;
}

void IClientImpl::unknown_37(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_37(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_37(client, unk1, unk2); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace,

                std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace,
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

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(uint client, uint unk1, uint unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(\n\tClientId client = {}\n\tuint unk1 = "
                            L"{}\n)",
                            client,
                            unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace,
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

bool IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(uint client, uint reason)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(\n\tClientId client = {}\n\tuint reason = {}\n)", client, reason));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(client, reason); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_44(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_44(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_44(client, unk1, unk2); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace,
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

bool IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(\n\tClientId client = {}\n\tuint unk1 = "
                            L"{}\n)",
                            client,
                            unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(uint client, FLPACKET_CREATESOLAR& solar)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateSolarPacket, ClientId(client), solar);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATESOLAR(client, solar); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateSolarPacketAfter, ClientId(client), solar);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATESHIP(uint client, FLPACKET_CREATESHIP& ship)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATESHIP(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateShipPacket, ClientId(client), ship);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATESHIP(client, ship); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateShipPacketAfter, ClientId(client), ship);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATELOOT(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATELOOT(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateLootPacket, ClientId(client), unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATELOOT(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateLootPacketAfter, ClientId(client), unk1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATEMINE(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATEMINE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateMinePacket, ClientId(client), unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATEMINE(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateMinePacketAfter, ClientId(client), unk1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(uint& client, FLPACKET_CREATEGUIDED& guided)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateGuidedPacket, ClientId(client), guided);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATEGUIDED(client, guided); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateGuidedPacketAfter, ClientId(client), guided);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnCreateCounterPacket, ClientId(client), unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_CREATECOUNTER(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnCreateCounterPacketAfter, ClientId(client), unk1);

    return retVal;
}

void IClientImpl::unknown_53(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_53(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_53(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_54(uint client, uint unk1, uint unk2, uint unk3)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_54(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n\tuint unk3 = {}\n)", client, unk1, unk2, unk3));

    CallClientPreamble { unknown_54(client, unk1, unk2, unk3); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_COMMON_UPDATEOBJECT(uint client, SSPObjUpdateInfo& update)
{
    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnUpdateObjectPacket, ClientId(client), update);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_COMMON_UPDATEOBJECT(client, update); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnUpdateObjectPacketAfter, ClientId(client), update);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(uint client, FLPACKET_DESTROYOBJECT& destroy)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnDestroyObjectPacket, ClientId(client), destroy);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_DESTROYOBJECT(client, destroy); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnDestroyObjectPacketAfter, ClientId(client), destroy);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(uint client, XActivateEquip& aq)
{
    Logger::Log(LogLevel::Trace,

                std::format(L"IClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnActivateObjectPacket, ClientId(client), aq);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_ACTIVATEOBJECT(client, aq); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnActivateObjectPacketAfter, ClientId(client), aq);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(uint client, FLPACKET_SYSTEM_SWITCH_OUT& systemSwitchOut)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(client, systemSwitchOut); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(uint client, FLPACKET_SYSTEM_SWITCH_IN& systemSwitchIn)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(client, systemSwitchIn); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_LAND(uint client, FLPACKET_LAND& land)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_LAND(\n\tClientId client = {}\n)", client));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_LAND(client, land); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_LAUNCH(uint client, FLPACKET_LAUNCH& launch)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_LAUNCH(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnLaunchPacket, ClientId(client), launch);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_LAUNCH(client, launch); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnLaunchPacketAfter, ClientId(client), launch);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(uint client, bool response, uint shipId)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(\n\tClientId client = {}\n\tbool response = {}\n\tuint shipId = {}\n)",
                            client,
                            response,
                            shipId));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnRequestCreateShipResponsePacket, ClientId(client), response, ShipId(shipId));

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(client, response, shipId); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnRequestCreateShipResponsePacketAfter, ClientId(client), response, ShipId(shipId));

    return retVal;
}

void IClientImpl::unknown_63(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_63(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_63(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(uint client, uint objId, DamageList& dmgList)
{
    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_DAMAGEOBJECT(client, objId, dmgList); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_ITEMTRACTORED(client, unk1); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_USE_ITEM(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_USE_ITEM(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnUseItemPacket, ClientId(client), unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_USE_ITEM(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnUseItemPacketAfter, ClientId(client), unk1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(uint client, FLPACKET_SETREPUTATION& rep)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetReputationPacket, ClientId(client), rep);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETREPUTATION(client, rep); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetReputationPacketAfter, ClientId(client), rep);

    return retVal;
}

void IClientImpl::unknown_68(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_68(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_68(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SENDCOMM(uint client, uint unk1, uint unk2, uint unk3, uint unk4, uint unk5, uint unk6, uint unk7, uint unk8, uint unk9,
                                                uint unk10, uint unk11, uint unk12, uint unk13, uint unk14, uint unk15, uint unk16, uint unk17, uint unk18,
                                                uint unk19, uint unk20, uint unk21, uint unk22)
{
    bool retVal;
    Logger::Log(LogLevel::Trace,
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

void IClientImpl::unknown_70(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_70(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_70(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetMissionMessagePacket, ClientId(client), unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetMissionMessagePacketAfter, ClientId(client), unk1);

    return retVal;
}

void IClientImpl::unknown_72(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_72(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_72(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetMissionObjectivesPacket, ClientId(client), unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetMissionObjectivesPacketAfter, ClientId(client), unk1);

    return retVal;
}

void IClientImpl::unknown_74(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_74(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_74(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_75(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_75(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_75(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_MARKOBJ(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_MARKOBJ(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_MARKOBJ(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_77(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_77(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_77(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SETCASH(uint client, uint cash)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_SETCASH(\n\tClientId client = {}\n\tuint cash = {}\n)", client, cash));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnSetCashPacket, ClientId(client), cash);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SETCASH(client, cash); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnSetCashPacketAfter, ClientId(client), cash);

    return retVal;
}

void IClientImpl::unknown_79(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_79(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_79(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_80(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_80(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_80(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_81(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_81(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_81(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_82(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_82(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_82(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_83(uint client, char* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_83(\n\tClientId client = {}\n\tchar* unk1 = {}\n)", client, StringUtils::stows(unk1)));

    CallClientPreamble { unknown_83(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(uint& client, uint shipId, uint flag, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace,
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

void IClientImpl::unknown_85(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_85(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_85(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_86(uint client, uint unk1, uint unk2, uint unk3)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_86(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n\tuint unk3 = {}\n)", client, unk1, unk2, unk3));

    CallClientPreamble { unknown_86(client, unk1, unk2, unk3); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(SObjectCargoUpdate& cargoUpdate, uint dunno1, uint dunno2)
{
    Logger::Log(
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

bool IClientImpl::Send_FLPACKET_SERVER_BURNFUSE(uint client, FLPACKET_BURNFUSE& burnFuse)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_BURNFUSE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnBurnFusePacket, ClientId(client), burnFuse);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_BURNFUSE(client, burnFuse); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnBurnFusePacketAfter, ClientId(client), burnFuse);

    return retVal;
}

void IClientImpl::unknown_89(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_89(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_89(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_90(uint client)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_90(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_90(client); }
    CallClientPostamble;
}

void IClientImpl::unknown_91(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_91(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_91(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(uint client, uint unk1, int unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(\n\tClientId client = {}\n\tuint unk1 = {}\n\tint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_SET_WEAPON_GROUP(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(uint client, uint objHash, int state)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(\n\tClientId client = {}\n\tuint objHash = {}\n\tint state = {}\n)", client, objHash, state));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_SET_VISITED_STATE(client, objHash, state); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(uint client, uint objHash, int unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(\n\tClientId client = {}\n\tobjHash = {}\n\tint unk2 = {}\n)", client, objHash, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_REQUEST_BEST_PATH(client, objHash, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(uint client, uint unk1, int unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(\n\tClientId client = {}\n\tuint unk1 = {}\n\tint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_96(uint client, uint unk1, uint unk2, uint unk3)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::unknown_96(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n\tuint unk3 = {}\n)", client, unk1, unk2, unk3));

    CallClientPreamble { unknown_96(client, unk1, unk2, unk3); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(uint client, uint unk1, int unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(\n\tClientId client = {}\n\tuint unk1 = {}\n\tint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(uint client, uint unk1, int unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(\n\tClientId client = {}\n\tuint unk1 = {}\n\tint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_SET_MISSION_LOG(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

bool IClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(uint client, uint unk1, int unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(\n\tClientId client = {}\n\tuint unk1 = {}\n\tint unk2 = {}\n)", client, unk1, unk2));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_SET_INTERFACE_STATE(client, unk1, unk2); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_100(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_100(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_100(client, unk1, unk2); }
    CallClientPostamble;
}

void IClientImpl::unknown_101(uint& client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_101(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_101(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_102(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_102(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_102(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_103(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_103(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_103(client, unk1); }
    CallClientPostamble;
}

void IClientImpl::unknown_104(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_104(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_104(client, unk1, unk2); }
    CallClientPostamble;
}

void IClientImpl::unknown_105(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_105(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_105(client, unk1, unk2); }
    CallClientPostamble;
}

void IClientImpl::unknown_106(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_106(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_106(client, unk1, unk2); }
    CallClientPostamble;
}

void IClientImpl::unknown_107(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_107(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    CallClientPreamble { unknown_107(client, unk1, unk2); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_COMMON_PLAYER_TRADE(client, unk1); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_109(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_109(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_109(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(uint client, uint unk1, uint unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnScanNotifyPacket, ClientId(client), unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_SCANNOTIFY(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnScanNotifyPacketAfter, ClientId(client), unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(uint client, wchar_t* characterName, uint unk2, char unk3)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(\n\tClientId client = {}\n\twchar_t* characterName = \n\tuint unk2 = {}\n)",
                            client,
                            std::wstring(characterName),
                            unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnPlayerListPacket, ClientId(client), characterName, unk2, unk3);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_PLAYERLIST(client, characterName, unk2, unk3); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnPlayerListPacketAfter, ClientId(client), characterName, unk2, unk3);

    return retVal;
}

void IClientImpl::unknown_112(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_112(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_112(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(uint client)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnPlayerList2Packet, ClientId(client));

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_PLAYERLIST_2(client); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnPlayerList2PacketAfter, ClientId(client));

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(uint client, uint unk1, uint unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate6Packet, ClientId(client), unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_6(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate6PacketAfter, ClientId(client), unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(uint client, uint unk1, uint unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate7Packet, ClientId(client), unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_7(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate7PacketAfter, ClientId(client), unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(uint client, FLPACKET_UNKNOWN* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(\n\tClientId client = {}\n)", client));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdatePacket, ClientId(client), unk1);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE(client, unk1); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdatePacketAfter, ClientId(client), unk1);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(uint client, uint unk1, uint unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate2Packet, ClientId(client), unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_2(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate2PacketAfter, ClientId(client), unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(uint client, uint targetId, uint rank)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(
            L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(\n\tClientId client = {}\n\tuint targetId = {}\n\tuint rank = {}\n)", client, targetId, rank));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate3Packet, ClientId(client), targetId, rank);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_3(client, targetId, rank); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate3PacketAfter, ClientId(client), targetId, rank);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(uint client, uint unk1, uint unk2)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint "
                            L"unk2 = {}\n)",
                            client,
                            unk1,
                            unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate4Packet, ClientId(client), unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_4(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate4PacketAfter, ClientId(client), unk1, unk2);

    return retVal;
}

bool IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(uint client, uint unk1, uint unk2)
{
    Logger::Log(
        LogLevel::Trace,
        std::format(L"IClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(\n\tClientId client = {}\n\tuint unk1 = {}\n\tuint unk2 = {}\n)", client, unk1, unk2));

    auto [retVal, skip] = CallPlugins<bool>(&PacketInterface::OnMiscObjectUpdate5Packet, ClientId(client), unk1, unk2);

    if (!skip)
    {
        CallClientPreamble { retVal = Send_FLPACKET_SERVER_MISCOBJUPDATE_5(client, unk1, unk2); }
        CallClientPostamble;
    }

    CallPlugins(&PacketInterface::OnMiscObjectUpdate5PacketAfter, ClientId(client), unk1, unk2);

    return retVal;
}

void IClientImpl::unknown_121(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_121(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_121(client, unk1); }
    CallClientPostamble;
}

bool IClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(uint client, uint shipId, Vector& formationOffset)
{
    Logger::Log(LogLevel::Trace,
                std::format(L"IClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(\n\tClientId client = {}\n\tuint shipId = {}\n)", client, shipId));

    bool retVal;
    CallClientPreamble { retVal = Send_FLPACKET_SERVER_FORMATION_UPDATE(client, shipId, formationOffset); }
    CallClientPostamble;

    return retVal;
}

void IClientImpl::unknown_123(uint client, uint unk1, uint unk2, uint unk3, uint unk4, uint unk5, uint unk6)
{
    Logger::Log(LogLevel::Trace,
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

void IClientImpl::unknown_124(uint client)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_124(\n\tClientId client = {}\n)", client));

    CallClientPreamble { unknown_124(client); }
    CallClientPostamble;
}

void IClientImpl::unknown_125(uint client, uint unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_125(\n\tClientId client = {}\n\tuint unk1 = {}\n)", client, unk1));

    CallClientPreamble { unknown_125(client, unk1); }
    CallClientPostamble;
}

int IClientImpl::unknown_126(char* unk1)
{
    Logger::Log(LogLevel::Trace, std::format(L"IClientImpl::unknown_126(\n\tchar* unk1 = {}\n)", StringUtils::stows(unk1)));

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
