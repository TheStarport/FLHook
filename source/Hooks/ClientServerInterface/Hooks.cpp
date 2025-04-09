#include "PCH.hpp"

#include <croncpp.h>

#include "API/FLHook/AccountManager.hpp"
#include "API/FLHook/HttpServer.hpp"
#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/ExceptionHandler.hpp"
#include "Core/FLHook.hpp"
#include "Core/IpResolver.hpp"

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

    auto currentTime = TimeUtils::UnixTime<std::chrono::milliseconds>();
    for (const auto& timer : Timer::timers)
    {
        // This one isn't actually in seconds, but the plugins should be
        if (currentTime - timer->lastTime >= timer->interval)
        {
            timer->lastTime = currentTime;
            timer->func();
        }
    }

    auto oneShot = Timer::oneShotTimers.begin();
    while (oneShot != Timer::oneShotTimers.end())
    {
        if (auto timer = *oneShot; currentTime - timer->lastTime >= timer->interval)
        {
            timer->func();
            if (timer->callback.has_value())
            {
                timer->callback.value()(timer);
            }
            oneShot = Timer::oneShotTimers.erase(oneShot);
            continue;
        }

        ++oneShot;
    }

    currentTime = TimeUtils::UnixTime<std::chrono::seconds>();
    for (const auto& timer : Timer::cronTimers)
    {
        if (currentTime <= timer->cron->nextInterval)
        {
            continue;
        }

        timer->func();
        timer->lastTime = currentTime;

        std::tm tm{};
        const time_t time = timer->lastTime;
        cron::utils::time_to_tm(&time, &tm);
        auto nextTm = cron_next(cron::make_cron(timer->cron->cronExpression), tm);
        timer->cron->nextInterval = cron::utils::tm_to_time(nextTm);
    }

    const auto hook = FLHook::instance;
    char* data;
    memcpy(&data, static_cast<char*>(dataPtr) + 0x40, 4);
    memcpy(&hook->serverLoadInMs, data + 0x204, 4);
    memcpy(&hook->playerCount, data + 0x208, 4);
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
}

void IServerImplHook::StartupInnerAfter(SStartupInfo& si)
{
    // Patch to set maximum number of players to connect. This is normally less than MaxClientId
    const auto address = FLHook::Offset(FLHook::BinaryType::Server, AddressList::PlayerDbMaxPlayers);
    MemUtils::WriteProcMem(address, &si.maxPlayers, sizeof maxPlayers);

    // Clean up any mail to chars that no longer exist
    // TODO: Mail Rework MailManager::i()->CleanUpOldMail();

    FLHook::instance->accountManager->LoadNewPlayerFLInfo();

    INFO(L"FLHook Ready");

    FLHook::instance->flhookReady = true;

    // Manually iterate over plugins, if any return false unload plugin
    auto pluginManager = PluginManager::i();
    for (auto plugin = pluginManager->begin(); plugin != pluginManager->end();)
    {
        if (auto p = plugin->get(); !p->OnLoadSettings())
        {

            INFO(L"{0} LoadSettings failed. Unloading plugin", { L"pluginName", std::wstring(p->GetName()) });
            auto pCopy = p->dll;
            plugin->reset();
            plugin = pluginManager->plugins.erase(plugin);
            FreeLibrary(pCopy);
        }
        else
        {
            ++plugin;
        }
    }

    if (FLHook::instance->httpServer)
    {
        FLHook::instance->httpServer->RegisterRoutes();
    }
}

int __stdcall IServerImplHook::Update()
{
    auto [retVal, skip] = CallPlugins<int>(&Plugin::OnServerUpdate);

    UpdateInner();

    if (!skip)
    {

        if (FLHook::instance && FLHook::instance->httpServer)
        {
            // Lock the http server
            auto lock = std::scoped_lock{ *FLHook::instance->httpServer };
            CallServerPreamble { retVal = Server.Update(); }
            CallServerPostamble(true, int());
        }
        else
        {
            CallServerPreamble { retVal = Server.Update(); }
            CallServerPostamble(true, int());
        }
    }

    CallPlugins(&Plugin::OnServerUpdateAfter);

    return retVal;
}

void __stdcall IServerImplHook::Shutdown()
{
    TRACE(L"Shutdown");

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
