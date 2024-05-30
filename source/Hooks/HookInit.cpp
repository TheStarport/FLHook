#include "API/InternalApi.hpp"
#include "PCH.hpp"

#include "Core/FLHook.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Logger.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/IEngineHook.hpp"
#include "Core/IpResolver.hpp"
#include "Core/VTables.hpp"

#include "Core/FLPatch.hpp"
#include "API/FLHook/AccountManager.hpp"

std::vector<FLPatch> patches;

inline static char repFreeFixOld[5];

void FLHook::ClearClientInfo(ClientId client)
{
    CallPlugins(&Plugin::OnClearClientInfo, client);

    auto& info = client.GetData();

    info.characterName = L"";

    info.dieMsg = DieMsgType::All;
    info.ship = ShipId();
    info.shipOld = ShipId();
    info.spawnTime = 0;
    info.moneyFix.clear();
    info.tradePartner = ClientId();
    info.charMenuEnterTime = 0;
    info.cruiseActivated = false;
    info.kickTime = 0;
    info.lastExitedBaseId = 0;
    info.disconnected = false;
    info.f1Time = 0;
    info.timeDisconnect = 0;
    info.baseId = BaseId();
    info.baseEnterTime = 0;

    info.dmgLast = {};
    info.dieMsgSize = ChatSize::Default;
    info.chatSize = ChatSize::Default;
    info.chatStyle = ChatStyle::Default;

    info.ignoreInfoList.clear();
    info.hostname = L"";
    info.engineKilled = false;
    info.thrusterActivated = false;
    info.inTradelane = false;
    info.groupId = 0;

    info.spawnProtected = false;

    // Reset the dmg list if this client was the inflictor
    for (auto& i : Clients())
    {
        if (i.dmgLast.inflictorPlayerId == client.GetValue())
        {
            i.dmgLast = {};
        }
    }

    AccountManager::ClearClientInfo(client);
    info.account = nullptr;
}

void FLHook::LoadUserSettings(ClientId client)
{
    // TODO: Load from DB
}

void FLHook::InitHookExports()
{
    FLHook::contentDll = GetModuleHandle(L"content.dll");
    FLHook::remoteClient = GetModuleHandle(L"remoteclient.dll");

    // Patch out actual mPlayerData execution code
    AccountManager::InitContentDLLDetours();

    getShipInspect = reinterpret_cast<GetShipInspectT>(Offset(BinaryType::Server, AddressList::GetInspect));
    crcAntiCheat = reinterpret_cast<CRCAntiCheatT>(Offset(BinaryType::Server, AddressList::CrcAntiCheat));
    IEngineHook::oldLoadReputationFromCharacterFile = reinterpret_cast<FARPROC>(Offset(BinaryType::Server, AddressList::SaveFileHouseEntrySaveAndLoadPatch) + 7);

    IEngineHook::loadReputationFromCharacterFileAssembly = new IEngineHook::LoadReputationFromCharacterFileAssembly;
    IEngineHook::disconnectPacketSentAssembly = new IEngineHook::DisconnectPacketSentAssembly;

    patches.emplace_back(FLPatch("flserver.exe", {
        {0x1B094, &IEngineHook::UpdateTime},
        {0x1BAB0, &IEngineHook::ElapseTime},
    }));

    patches.emplace_back(FLPatch("content.dll", {
        { 0x11358C, &IEngineHook::DockCall }
    }));

    patches.emplace_back(FLPatch("remoteclient.dll", {
        { 0x3BB80, &IServerImplHook::SendChat, &rcSendChatMsg }
    }));

    patches.emplace_back(FLPatch("dalib.dll", {
        { 0x4BEC, PVOID(IEngineHook::disconnectPacketSentAssembly->getCode()), &IEngineHook::oldDisconnectPacketSent }
    }));

    patches.emplace_back(FLPatch("server.dll", {
        { 0x8420C, PVOID(IEngineHook::launchPositionAssembly.getCode()), &IEngineHook::oldLaunchPosition },
        { 0x848E0, &IEngineHook::FreeReputationVibe },
    }));

    for (auto& patch : patches)
    {
        patch.Apply();
    }

    const DWORD saveSpCheck = FLHook::Offset(FLHook::BinaryType::Server, AddressList::SaveCharacter);
    MemUtils::NopAddress(saveSpCheck, 2);

    IpResolver::resolveThread = std::thread(IpResolver::ThreadResolver);

    // install IServerImpl callbacks in remoteclient.dll
    auto* serverPointer = reinterpret_cast<char*>(&Server);
    memcpy(&serverPointer, serverPointer, 4);
    for (auto& [proc, remoteAddress, oldProc] : IServerImplHook::entries)
    {
        const auto address = reinterpret_cast<DWORD>(serverPointer + remoteAddress);
        MemUtils::ReadProcMem(address, &oldProc, 4);
        MemUtils::WriteProcMem(address, &proc, 4);
    }

    #define VTablePtr(x) static_cast<unsigned short>(x)

    // Common.dll
    const void* ptr = &IEngineHook::CShipInit;
    IEngineHook::cShipVTable.Hook(VTablePtr(CShipVTable::InitCShip), &ptr);
    ptr = &IEngineHook::ShipDestroy;
    IEngineHook::iShipVTable.Hook(VTablePtr(IShipInspectVTable::ObjectDestroyed), &ptr);

    ptr = &IEngineHook::CLootInit;
    IEngineHook::cLootVTable.Hook(VTablePtr(CLootVTable::InitCLoot), &ptr);
    ptr = &IEngineHook::LootDestroy;
    IEngineHook::iLootVTable.Hook(VTablePtr(ILootInspectVTable::ObjectDestroyed), &ptr);

    ptr = &IEngineHook::CSolarInit;
    IEngineHook::cSolarVtable.Hook(VTablePtr(CSolarVTable::InitializeInstance), &ptr);
    ptr = &IEngineHook::SolarDestroy;
    IEngineHook::iSolarVTable.Hook(VTablePtr(ISolarInspectVTable::ObjectDestroyed), &ptr);

    ptr = &IEngineHook::ShipHullDamage;
    IEngineHook::iShipVTable.Hook(VTablePtr(IShipInspectVTable::DamageHull), &ptr);
    ptr = &IEngineHook::SolarHullDamage;
    IEngineHook::iSolarVTable.Hook(VTablePtr(ISolarInspectVTable::DamageHull), &ptr);

    ptr = &IEngineHook::ShipExplosionHit;
    IEngineHook::iShipVTable.Hook(VTablePtr(IShipInspectVTable::ProcessExplosionDamage), &ptr);
    // Server.dll

    #undef VtablePtr

    // Patch when ships are destroyed to move the damage list pointer into edx, which we read with a __fastcall
    DWORD address = Offset(BinaryType::Server, AddressList::ShipDestroyedInvoke);
    const std::array<byte, 30> shipDestroyedDamageList = { 0x90, 0x90, 0x8A, 0x88, 0x5C, 0x01, 0x00, 0x00, 0x84, 0xC9, 0x75, 0x10, 0x8B, 0x06, 0xFF, 0x73,
        0x14, 0x89, 0xDA, 0x6A, 0x01, 0x89, 0xF1, 0xFF, 0x90, 0x58, 0x01, 0x00, 0x00, 0x90 };
    MemUtils::WriteProcMem(address, shipDestroyedDamageList.data(), shipDestroyedDamageList.size());

    // DetourSendComm();

    // patch rep array free
    address = Offset(BinaryType::Server, AddressList::RepArrayFree);
    MemUtils::ReadProcMem(address, repFreeFixOld, 5);
    MemUtils::NopAddress(address, 5);

    // patch flserver so it can better handle faulty house entries in char files

    // divert call to house load/save func
    address = Offset(BinaryType::Server, AddressList::SaveFileHouseEntrySaveAndLoad);
    constexpr std::array<byte, 1> divertJump = { 0x6F };

    MemUtils::WriteProcMem(address, divertJump.data(), 1);

    // install hook at new address
    address = Offset(BinaryType::Server, AddressList::SaveFileHouseEntrySaveAndLoadPatch);

    constexpr std::array<byte, 1> movEax = { 0xB8 };
    constexpr std::array<byte, 2> jumpEax = { 0xFF, 0xE0 };

    const auto loadRepFromCharFile = reinterpret_cast<const void*>(IEngineHook::loadReputationFromCharacterFileAssembly->getCode());

    MemUtils::WriteProcMem(address, movEax.data(), 1);
    MemUtils::WriteProcMem(address + 1, &loadRepFromCharFile, 4);
    MemUtils::WriteProcMem(address + 5, jumpEax.data(), 2);

    // get CDPServer
    address = Offset(BinaryType::DaLib, AddressList::CdpServer);
    MemUtils::ReadProcMem(address, &cdpServer, 4);

    // charfile encyption(doesn't get disabled when unloading FLHook)
    if (FLHook::GetConfig().general.disableCharfileEncryption)
    {
        constexpr std::array<byte, 2> buffer = { 0x14, 0xB3 };
        address = Offset(BinaryType::Server, AddressList::CharFileEncryption);
        MemUtils::WriteProcMem(address, buffer.data(), 2);
        address = Offset(BinaryType::Server, AddressList::CharFileEncryption2);
        MemUtils::WriteProcMem(address, buffer.data(), 2);
    }

    // maximum group size
    if (FLHook::GetConfig().general.maxGroupSize > 0)
    {
        const auto newGroupSize = static_cast<std::byte>(FLHook::GetConfig().general.maxGroupSize & 0xFF);
        address = Offset(BinaryType::Server, AddressList::MaxGroupSize);
        MemUtils::WriteProcMem(address, &newGroupSize, 1);
        address = Offset(BinaryType::Server, AddressList::MaxGroupSize2);
        MemUtils::WriteProcMem(address, &newGroupSize, 1);
    }

    // get client proxy array, used to retrieve player pings/ips
    address = Offset(BinaryType::RemoteClient, AddressList::CpList);
    char* temp;
    MemUtils::ReadProcMem(address, &temp, 4);
    temp += 0x10;
    memcpy(&clientProxyArray, &temp, 4);

    // init variables
    char dataPath[MAX_PATH];
    GetUserDataPath(dataPath);
    accPath = StringUtils::stows(std::format(R"({}\Accts\MultiPlayer\)", std::string(dataPath)));

    // clear ClientInfo

    for (auto& client : Clients())
    {
        client.connects = 0; // only set to 0 on start
        ClearClientInfo(client.id);
    }

    const std::array<byte, 22> refireBytes = { 0x75, 0x0B, 0xC7, 0x84, 0x8C, 0x9C, 00, 00, 00, 00, 00, 00, 00, 0x41, 0x83, 0xC2, 0x04, 0x39, 0xC1, 0x7C, 0xE9, 0xEB };
    MemUtils::WriteProcMem(Offset(BinaryType::Server, AddressList::SolarRefireRateBug), refireBytes.data(), 22);

    // Enable undocking announcer regardless of distance
    constexpr std::array<byte, 1> undockAnnouncerBytes = { 0xEB };
    MemUtils::WriteProcMem(Offset(BinaryType::Server, AddressList::FixNpcAnnouncer), undockAnnouncerBytes.data(), 1);

    // Remove default death messages
    constexpr std::array<byte, 1> removeDeathMessages = { 0xEB };
    MemUtils::WriteProcMem(Offset(BinaryType::Server, AddressList::RemoveDefaultDeathMessages), removeDeathMessages.data(), 1);
}

void FLHook::PatchClientImpl()
{
    // install IClientImpl callback
    fakeClientImpl = new IClientImpl;
    hookClientImpl = &Client;

    memcpy(&oldClientImpl, &Client, 4);
    memcpy(&Client, fakeClientImpl, 4);
}

void FLHook::UnloadHookExports()
{
    // uninstall IServerImpl callbacks in remoteclient.dll
    if (auto serverAddr = reinterpret_cast<char*>(&Server))
    {
        memcpy(&serverAddr, serverAddr, 4);
        for (auto& [proc, remoteAddress, oldProc] : IServerImplHook::entries)
        {
            const auto address = reinterpret_cast<DWORD>(serverAddr + remoteAddress);
            MemUtils::WriteProcMem(address, &oldProc, 4);
        }
    }

    InternalApi::ToggleNpcSpawns(true);

    for (auto& patch : patches)
    {
        patch.Revert();
    }

    delete IEngineHook::disconnectPacketSentAssembly;
    delete IEngineHook::loadReputationFromCharacterFileAssembly;

    // UnDetourSendComm();

    // unpatch rep array free
    DWORD address = Offset(BinaryType::Server, AddressList::RepArrayFree);
    MemUtils::WriteProcMem(address, repFreeFixOld, 5);

    // undivert call to house load/save func
    address = Offset(BinaryType::Server, AddressList::SaveFileHouseEntrySaveAndLoad);
    constexpr std::array<byte, 1> divertJump = { 0x76 };
    MemUtils::WriteProcMem(address, divertJump.data(), 1);

    // Undo refire bug
    const std::array<byte, 22> refireBytes = {
        0x74, 0x0A, 0x41, 0x83, 0xC2, 0x04, 0x3B, 0xC8, 0x7C, 0xF4, 0xEB, 0x0B, 0xC7, 0x84, 0x8C, 0x9C, 0, 0, 0, 0, 0, 0,
    };
    MemUtils::WriteProcMem(Offset(BinaryType::Server, AddressList::SolarRefireRateBug), refireBytes.data(), 22);

    // undocking announcer regardless of distance
    constexpr std::array<byte, 1> undockAnnouncerBytes = { 0x74 };
    MemUtils::WriteProcMem(Offset(BinaryType::Server, AddressList::FixNpcAnnouncer), undockAnnouncerBytes.data(), 1);

    // plugins
    PluginManager::i()->UnloadAll();
}
