#include "PCH.hpp"

#include "API/FLHook/MailManager.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/FLHook.hpp"
#include "Global.hpp"

void IServerImplHook::ServerReady()
{
    const DWORD address = FLHook::Offset(FLHook::BinaryType::Exe, AddressList::DataPtr);
    MemUtils::ReadProcMem(address, &dataPtr, 4);
}

void IServerImplHook::UpdateInner()
{
    static bool firstTime = true;
    if (firstTime)
    {
        ServerReady();
        FLHook::instance->OnServerStart();
        firstTime = false;
    }

    const auto currentTime = TimeUtils::UnixMilliseconds();
    for (const auto& timer : Timer::timers)
    {
        // This one isn't actually in seconds, but the plugins should be
        if (currentTime - timer->lastTime >= timer->intervalInSeconds)
        {
            timer->lastTime = currentTime;
            timer->func();
        }
    }

    for (const auto& plugin : *PluginManager::i())
    {
        auto timers = plugin->GetTimers();
        if (timers.empty())
        {
            continue;
        }

        for (const auto& timer : timers)
        {
            if (currentTime - timer->lastTime >= timer->intervalInSeconds * 1000)
            {
                timer->lastTime = currentTime;
                timer->func();
            }
        }
    }

    const auto globals = CoreGlobals::i();
    char* data;
    memcpy(&data, dataPtr + 0x40, 4);
    memcpy(&globals->serverLoadInMs, data + 0x204, 4);
    memcpy(&globals->playerCount, data + 0x208, 4);
}

void IServerImplHook::StartupInner(SStartupInfo& si)
{
    FLHook::Startup();

    // Startup the server with this number of players.
    const auto address = FLHook::Offset(FLHook::BinaryType::Server, AddressList::PlayerDbMaxPlayersPatch);
    std::array<byte, 1> nop = { 0x90 };
    std::array<byte, 1> movEcx = { 0xB9 };
    MemUtils::WriteProcMem(address, movEcx.data(), 1);
    MemUtils::WriteProcMem(address + 1, &maxPlayers, sizeof maxPlayers);
    MemUtils::WriteProcMem(address + 5, nop.data(), 1);

    FLHook::instance->startupCache = std::make_unique<StartupCache>();
}

void IServerImplHook::StartupInnerAfter(SStartupInfo& si)
{
    // Patch to set maximum number of players to connect. This is normally less than MaxClientId
    const auto address = FLHook::Offset(FLHook::BinaryType::Server, AddressList::PlayerDbMaxPlayers);
    MemUtils::WriteProcMem(address, &si.maxPlayers, sizeof maxPlayers);

    // read base market data from ini
    LoadBaseMarket();

    // Clean up any mail to chars that no longer exist
    MailManager::i()->CleanUpOldMail();

    FLHook::instance->startupCache->Done();

    Logger::i()->Log(LogLevel::Info, L"FLHook Ready");

    CoreGlobals::i()->flhookReady = true;
}

int __stdcall IServerImplHook::Update()
{
    auto [retVal, skip] = CallPlugins<int>(&Plugin::OnServerUpdate);

    UpdateInner();

    if (!skip)
    {
        CallServerPreamble { retVal = Server.Update(); }
        CallServerPostamble(true, int());
    }

    CallPlugins(&Plugin::OnServerUpdateAfter);

    return retVal;
}

void __stdcall IServerImplHook::Shutdown()
{
    Logger::i()->Log(LogLevel::Trace, L"Shutdown()");

    if (const auto skip = CallPlugins(&Plugin::OnServerShutdown); !skip)
    {
        CallServerPreamble { Server.Shutdown(); }
        CallServerPostamble(true, );
    }

    FLHook::Shutdown();
}

bool __stdcall IServerImplHook::Startup(SStartupInfo& si)
{
    StartupInner(si);

    auto [retVal, skip] = CallPlugins<bool>(&Plugin::OnServerStartup, si);

    if (!skip)
    {
        CallServerPreamble { retVal = Server.Startup(si); }
        CallServerPostamble(true, bool());
    }
    StartupInnerAfter(si);

    CallPlugins(&Plugin::OnServerStartupAfter, si);

    return retVal;
}
