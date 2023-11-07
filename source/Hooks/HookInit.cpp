#include "PCH.hpp"

#include "Core/FLHook.hpp"

#include "API/API.hpp"
#include "Core/IEngineHook.hpp"
#include "Core/IpResolver.hpp"

#include <API/Utils/IniUtils.hpp>

FLHook::PatchInfo FLHook::exePatch = { "flserver.exe",
                                       0x0400000,
                                       { {
                                           { 0x041B094, &IEngineHook::UpdateTime, 4, nullptr, false },
                                           { 0x041BAB0, &IEngineHook::ElapseTime, 4, nullptr, false },
                                       } } };

FLHook::PatchInfo FLHook::contentPatch = { "content.dll",
                                           0x6EA0000,
                                           { {
                                               { 0x6FB358C, &IEngineHook::DockCall, 4, nullptr, false },
                                           } } };

FLHook::PatchInfo FLHook::commonPatch = { "common.dll",
                                          0x6260000,
                                          { {

                                              { 0x0639C138, &IEngineHook::NakedCShipInit, 4, &IEngineHook::oldInitCShip, false },
                                              { 0x0639C064, &IEngineHook::NakedCShipDestroy, 4, &IEngineHook::oldDestroyCShip, false },
                                          } } };

FLHook::PatchInfo FLHook::serverPatch = { "server.dll",
                                          0x6CE0000,
                                          { {
                                              { 0x6D67274, &IEngineHook::NakedShipDestroyed, 4, &oldShipDestroyed, false },
                                              { 0x6D641EC, &IEngineHook::NakedAddDamageEntry, 4, nullptr, false },
                                              { 0x6D67320, &IEngineHook::NakedGuidedHit, 4, &oldGuidedHit, false },
                                              { 0x6D65448, &IEngineHook::NakedGuidedHit, 4, nullptr, false },
                                              { 0x6D67670, &IEngineHook::NakedGuidedHit, 4, nullptr, false },
                                              { 0x6D653F4, &IEngineHook::NakedDamageHit, 4, &oldDamageHit, false },
                                              { 0x6D672CC, &IEngineHook::NakedDamageHit, 4, nullptr, false },
                                              { 0x6D6761C, &IEngineHook::NakedDamageHit, 4, nullptr, false },
                                              { 0x6D65458, &IEngineHook::NakedDamageHit2, 4, &oldDamageHit2, false },
                                              { 0x6D67330, &IEngineHook::NakedDamageHit2, 4, nullptr, false },
                                              { 0x6D67680, &IEngineHook::NakedDamageHit2, 4, nullptr, false },
                                              { 0x6D67668, &IEngineHook::NakedNonGunWeaponHitsBase, 4, &oldNonGunWeaponHitsBase, false },
                                              { 0x6D6420C, &IEngineHook::NakedLaunchPosition, 4, &IEngineHook::oldLaunchPosition, false },
                                              { 0x6D648E0, &IEngineHook::FreeReputationVibe, 4, nullptr, false },
                                          } } };

FLHook::PatchInfo FLHook::remoteClientPatch = { "remoteclient.dll",
                                                0x6B30000,
                                                { {
                                                    { 0x6B6BB80, &IServerImplHook::SendChat, 4, &rcSendChatMsg, false },
                                                } } };

FLHook::PatchInfo FLHook::dalibPatch = { "dalib.dll",
                                         0x65C0000,
                                         { {
                                             { 0x65C4BEC, &IEngineHook::NakedDisconnectPacketSent, 4, &oldDisconnectPacketSent, false },
                                         } } };

bool FLHook::ApplyPatch(PatchInfo& pi)
{
    const HMODULE hMod = GetModuleHandleA(pi.BinName);
    if (!hMod)
    {
        return false;
    }

    for (auto& entry : pi.entries)
    {
        char* address = reinterpret_cast<char*>(hMod) + (entry.address - pi.baseAddress);
        if (!entry.oldValue)
        {
            entry.oldValue = new char[entry.size];
            entry.allocated = true;
        }
        else
        {
            entry.allocated = false;
        }

        MemUtils::ReadProcMem(address, entry.oldValue, entry.size);
        MemUtils::WriteProcMem(address, &entry.newValue, entry.size);
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool FLHook::RevertPatch(PatchInfo& pi)
{
    const HMODULE hMod = GetModuleHandleA(pi.BinName);
    if (!hMod)
    {
        return false;
    }

    for (const auto& entry : pi.entries)
    {
        char* address = (char*)hMod + (entry.address - pi.baseAddress);
        MemUtils::WriteProcMem(address, entry.oldValue, entry.size);
        if (entry.allocated)
        {
            // ReSharper disable once CppDeletingVoidPointer
            delete[] entry.oldValue;
        }
    }

    return true;
}

inline static char repFreeFixOld[5];

void FLHook::ClearClientInfo(ClientId client)
{
    auto* info = &ClientInfo::At(client);

    info->characterName = L"";
    info->characterFile = L"";

    info->dieMsg = DiemsgAll;
    info->ship = 0;
    info->shipOld = 0;
    info->spawnTime = 0;
    info->moneyFix.clear();
    info->tradePartner = 0;
    info->baseEnterTime = 0;
    info->charMenuEnterTime = 0;
    info->cruiseActivated = false;
    info->kickTime = 0;
    info->lastExitedBaseId = 0;
    info->disconnected = false;
    info->characterName = L"";
    info->f1Time = 0;
    info->timeDisconnect = 0;

    info->dmgLast = {};
    info->dieMsgSize = CS_DEFAULT;
    info->chatSize = CS_DEFAULT;
    info->chatStyle = CST_DEFAULT;

    info->ignoreInfoList.clear();
    info->killsInARow = 0;
    info->hostname = L"";
    info->engineKilled = false;
    info->thrusterActivated = false;
    info->tradelane = false;
    info->groupId = 0;

    info->spawnProtected = false;

    // Reset the dmg list if this client was the inflictor
    for (auto& i : ClientInfo::clients)
    {
        if (i.dmgLast.inflictorPlayerId == client)
        {
            i.dmgLast = {};
        }
    }

    Hk::IniUtils::CharacterClearClientInfo(client);

    CallPlugins(&Plugin::OnClearClientInfo, client);
}

void FLHook::LoadUserSettings(ClientId client)
{
    auto* info = &ClientInfo::At(client);

    const CAccount* acc = Players.FindAccountFromClientID(client);
    const std::wstring dir = Hk::Client::GetAccountDirName(acc);
    const std::wstring userFile = std::format(L"{}{}\\accData.json", accPath, dir);

    info->accountData = nlohmann::json::object();

    try
    {
        if (std::wifstream ifs(userFile); !ifs.fail())
        {
            std::wstring fileData((std::istreambuf_iterator(ifs)), (std::istreambuf_iterator<wchar_t>()));
            info->accountData = nlohmann::json::parse(fileData);
        }
    }
    catch (nlohmann::json::exception& ex)
    {
        // TODO: Log to a special error file
        Logger::i()->Log(LogLevel::Err,
                         std::format(L"Error while loading account data from account file ({}): {}", userFile, StringUtils::stows(std::string(ex.what()))));
    }

    auto settings = info->accountData.value("settings", nlohmann::json::object());

    info->dieMsg = settings.value("dieMsg", DiemsgAll);
    info->dieMsgSize = settings.value("dieMsgSize", CS_DEFAULT);
    info->chatSize = settings.value("chatSize", CS_DEFAULT);
    info->chatStyle = settings.value("chatStyle", CST_DEFAULT);

    // read ignorelist
    info->ignoreInfoList.clear();

    for (const auto ignoreList = settings.value("ignoreList", nlohmann::json::object()); const auto& [key, value] : ignoreList.items())
    {
        try
        {
            IgnoreInfo ii;
            ii.character = StringUtils::stows(key);
            ii.flags = value.get<std::wstring>();
            info->ignoreInfoList.emplace_back(ii);
        }
        catch (...)
        {
            Logger::i()->Log(LogLevel::Err, std::format(L"Error while loading ignore list from account file: {}", userFile));
        }
    }

    // Don't know if this is a reference or copy - write again to be safe
    info->accountData["settings"] = settings;
}

void FLHook::InitHookExports()
{
    IpResolver::resolveThread = std::thread(IpResolver::ThreadResolver);

    getShipInspect = reinterpret_cast<GetShipInspect>(Offset(BinaryType::Server, AddressList::GetInspect));

    // install IServerImpl callbacks in remoteclient.dll
    auto* serverPointer = reinterpret_cast<char*>(&Server);
    memcpy(&serverPointer, serverPointer, 4);
    for (auto& [proc, remoteAddress, oldProc] : IServerImplHook::entries)
    {
        char* address = serverPointer + remoteAddress;
        MemUtils::ReadProcMem(address, &oldProc, 4);
        MemUtils::WriteProcMem(address, &proc, 4);
    }

    // patch it
    ApplyPatch(exePatch);
    ApplyPatch(contentPatch);
    ApplyPatch(commonPatch);
    ApplyPatch(serverPatch);
    ApplyPatch(remoteClientPatch);
    ApplyPatch(dalibPatch);

    DetourSendComm();

    // patch rep array free
    DWORD address = Offset(BinaryType::Server, AddressList::RepArrayFree);
    MemUtils::ReadProcMem(address, repFreeFixOld, 5);
    MemUtils::NopAddress(address, 5);

    // patch flserver so it can better handle faulty house entries in char files

    // divert call to house load/save func
    address = Offset(BinaryType::Server, AddressList::SaveFileHouseEntrySaveAndLoad);
    std::array<byte, 1> divertJump = { 0x6F };

    MemUtils::WriteProcMem(address, divertJump.data(), 1);

    // install hook at new address
    address = Offset(BinaryType::Server, AddressList::SaveFileHouseEntrySaveAndLoadPatch);

    std::array<byte, 1> movEax = { 0xB8 };
    std::array<byte, 2> jumpEax = { 0xFF, 0xE0 };

    auto loadRepFromCharFile = reinterpret_cast<FARPROC>(IEngineHook::NakedLoadReputationFromCharacterFile);

    MemUtils::WriteProcMem(address, movEax.data(), 1);
    MemUtils::WriteProcMem(address + 1, &loadRepFromCharFile, 4);
    MemUtils::WriteProcMem(address + 5, jumpEax.data(), 2);

    IEngineHook::oldLoadReputationFromCharacterFile = reinterpret_cast<FARPROC>(
        Offset(BinaryType::Server, static_cast<AddressList>(static_cast<DWORD>(AddressList::SaveFileHouseEntrySaveAndLoadPatch) + 1)));

    // crc anti-cheat
    crcAntiCheat = reinterpret_cast<CRCAntiCheat>(Offset(BinaryType::Server, AddressList::CrcAntiCheat));

    // get CDPServer
    address = Offset(BinaryType::DaLib, AddressList::CdpServer);
    MemUtils::ReadProcMem(address, &cdpServer, 4);

    // anti-deathmsg
    if (FLHookConfig::i()->chatConfig.dieMsg)
    {
        // disables the "old" "A Player has died: ..." chatConfig
        std::array<byte, 1> jmp = { 0xEB };
        address = Offset(BinaryType::Server, AddressList::AntiDieMessage);
        MemUtils::WriteProcMem(address, jmp.data(), 1);
    }

    // charfile encyption(doesn't get disabled when unloading FLHook)
    if (FLHookConfig::i()->general.disableCharfileEncryption)
    {
        std::array<byte, 2> buffer = { 0x14, 0xB3 };
        address = Offset(BinaryType::Server, AddressList::CharFileEncryption);
        MemUtils::WriteProcMem(address, buffer.data(), 2);
        address = Offset(BinaryType::Server, AddressList::CharFileEncryption2);
        MemUtils::WriteProcMem(address, buffer.data(), 2);
    }

    // maximum group size
    if (FLHookConfig::i()->general.maxGroupSize > 0)
    {
        auto newGroupSize = static_cast<std::byte>(FLHookConfig::i()->general.maxGroupSize & 0xFF);
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

    // Load DLLs for strings
    Hk::Chat::LoadStringDLLs();

    // clear ClientInfo
    for (uint i = 0; i < ClientInfo::clients.size(); i++)
    {
        auto& client = ClientInfo::At(i);
        client.client = i;   // Set every client id struct to know of its own id
        client.connects = 0; // only set to 0 on start
        ClearClientInfo(i);
    }

    std::array<byte, 22> refireBytes = { 0x75, 0x0B, 0xC7, 0x84, 0x8C, 0x9C, 00, 00, 00, 00, 00, 00, 00, 0x41, 0x83, 0xC2, 0x04, 0x39, 0xC1, 0x7C, 0xE9, 0xEB };
    MemUtils::WriteProcMem(Offset(BinaryType::Server, AddressList::SolarRefireRateBug), refireBytes.data(), 22);

    // Enable undocking announcer regardless of distance
    std::array<byte, 1> undockAnnouncerBytes = { 0xEB };
    MemUtils::WriteProcMem(Offset(BinaryType::Server, AddressList::FixNpcAnnouncer), undockAnnouncerBytes.data(), 1);
}

void FLHook::PatchClientImpl()
{
    // install IClientImpl callback
    fakeClientImpl = new IClientImpl;
    hookClientImpl = &Client;

    memcpy(&oldClientImpl, &Client, 4);
    MemUtils::WriteProcMem(&Client, fakeClientImpl, 4);
}

void FLHook::UnloadHookExports()
{

    // uninstall IServerImpl callbacks in remoteclient.dll
    if (auto serverAddr = reinterpret_cast<char*>(&Server))
    {
        memcpy(&serverAddr, serverAddr, 4);
        for (auto& [proc, remoteAddress, oldProc] : IServerImplHook::entries)
        {
            const auto address = serverAddr + remoteAddress;
            MemUtils::WriteProcMem(address, &oldProc, 4);
        }
    }

    // reset npc spawn setting
    Hk::Admin::ChangeNPCSpawn(false);

    // restore other hooks
    RevertPatch(exePatch);
    RevertPatch(contentPatch);
    RevertPatch(commonPatch);
    RevertPatch(serverPatch);
    RevertPatch(remoteClientPatch);
    RevertPatch(dalibPatch);

    UnDetourSendComm();

    // unpatch rep array free
    DWORD address = Offset(BinaryType::Server, AddressList::RepArrayFree);
    MemUtils::WriteProcMem(address, repFreeFixOld, 5);

    // undivert call to house load/save func
    address = Offset(BinaryType::Server, AddressList::SaveFileHouseEntrySaveAndLoad);
    std::array<byte, 1> divertJump = { 0x76 };
    MemUtils::WriteProcMem(address, divertJump.data(), 1);

    // anti-death-msg
    std::array<byte, 1> old = { 0x74 };
    address = Offset(BinaryType::Server, AddressList::AntiDieMessage);
    MemUtils::WriteProcMem(address, old.data(), 1);

    // Undo refire bug
    std::array<byte, 22> refireBytes = {
        0x74, 0x0A, 0x41, 0x83, 0xC2, 0x04, 0x3B, 0xC8, 0x7C, 0xF4, 0xEB, 0x0B, 0xC7, 0x84, 0x8C, 0x9C, 0, 0, 0, 0, 0, 0,
    };
    MemUtils::WriteProcMem(Offset(BinaryType::Server, AddressList::SolarRefireRateBug), refireBytes.data(), 22);

    // undocking announcer regardless of distance
    std::array<byte, 1> undockAnnouncerBytes = { 0x74 };
    MemUtils::WriteProcMem(Offset(BinaryType::Server, AddressList::FixNpcAnnouncer), undockAnnouncerBytes.data(), 1);

    // plugins
    PluginManager::i()->UnloadAll();
}

void ClientInfo::SaveAccountData() const
{
    const CAccount* acc = Players.FindAccountFromClientID(client);
    const std::wstring dir = Hk::Client::GetAccountDirName(acc);
    const std::wstring userFile = std::format(L"{}{}\\accData.json", FLHook::GetAccountPath(), dir);

    const auto content = accountData.dump(4);
    std::ofstream of(userFile);
    of.write(content.c_str(), content.size());
}
