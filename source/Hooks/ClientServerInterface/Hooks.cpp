#include "PCH.hpp"

#include "Global.hpp"
#include "API/FLHook/MailManager.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    void Shutdown__InnerAfter() { FLHookShutdown(); }

    void Update__Inner()
    {
        static bool firstTime = true;
        if (firstTime)
        {
            FLHookInit();
            firstTime = false;
        }

        const auto currentTime = TimeUtils::UnixMilliseconds();
        for (auto& timer : Timer::timers)
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
        memcpy(&data, g_FLServerDataPtr + 0x40, 4);
        memcpy(&globals->serverLoadInMs, data + 0x204, 4);
        memcpy(&globals->playerCount, data + 0x208, 4);
    }
    void Startup__Inner(const SStartupInfo& si)
    {
        FLHookInit_Pre();

        // Startup the server with this number of players.
        char* address = reinterpret_cast<char*>(server) + ADDR_SRV_PLAYERDBMAXPLAYERSPATCH;
        const char nop[] = { '\x90' };
        const char movECX[] = { '\xB9' };
        MemUtils::WriteProcMem(address, movECX, sizeof movECX);
        MemUtils::WriteProcMem(address + 1, &g_MaxPlayers, sizeof g_MaxPlayers);
        MemUtils::WriteProcMem(address + 5, nop, sizeof nop);

        StartupCache::Init();
    }

    void Startup__InnerAfter(const SStartupInfo& si)
    {
        // Patch to set maximum number of players to connect. This is normally
        // less than MaxClientId
        char* address = reinterpret_cast<char*>(server) + ADDR_SRV_PLAYERDBMAXPLAYERS;
        MemUtils::WriteProcMem(address, &si.maxPlayers, sizeof g_MaxPlayers);

        // read base market data from ini
        LoadBaseMarket();

        // Clean up any mail to chars that no longer exist
        MailManager::i()->CleanUpOldMail();

        StartupCache::Done();

        Logger::i()->Log(LogLevel::Info, L"FLHook Ready");

        CoreGlobals::i()->flhookReady = true;
    }

    int __stdcall Update()
    {
        auto [retVal, skip] = CallPluginsBefore<int>(HookedCall::IServerImpl__Update);

        Update__Inner();

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { retVal = Server.Update(); }
            CALL_SERVER_POSTAMBLE(true, int());
        }

        CallPluginsAfter(HookedCall::IServerImpl__Update);

        return retVal;
    }

    void __stdcall Shutdown()
    {
        Logger::i()->Log(LogLevel::Trace, L"Shutdown()");

        if (const auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__Shutdown); !skip)
        {
            CALL_SERVER_PREAMBLE { Server.Shutdown(); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        Shutdown__InnerAfter();
    }

    bool __stdcall Startup(const SStartupInfo& si)
    {
        Startup__Inner(si);

        auto [retVal, skip] = CallPluginsBefore<bool>(HookedCall::IServerImpl__Startup, si);

        if (!skip)
        {
            CALL_SERVER_PREAMBLE { retVal = Server.Startup(si); }
            CALL_SERVER_POSTAMBLE(true, bool());
        }
        Startup__InnerAfter(si);

        CallPluginsAfter(HookedCall::IServerImpl__Startup, si);

        return retVal;
    }
} // namespace IServerImplHook
