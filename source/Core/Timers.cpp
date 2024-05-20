#include "PCH.hpp"

#include <Psapi.h>

#include "API/FLHook/ClientList.hpp"
#include "Core/FLHook.hpp"
#include "Core/MessageHandler.hpp"
#include "Defs/ServerStats.hpp"

std::shared_ptr<Timer> Timer::Add(std::function<void()> function, void* funcAddrRaw, uint interval)
{
    auto funcAddr = reinterpret_cast<DWORD>(funcAddrRaw);
    if (std::ranges::any_of(timers, [funcAddr](const std::shared_ptr<Timer>& existing) { return funcAddr == existing->funcAddr; }))
    {
        return nullptr;
    }

    auto ptr = std::make_shared<Timer>(function, funcAddr, interval);
    timers.emplace_back(ptr);

    return ptr;
}

void Timer::Remove(const DWORD funcAddr)
{
    const auto timer = std::ranges::find_if(timers, [funcAddr](const std::shared_ptr<Timer>& existing) { return funcAddr == existing->funcAddr; });
    if (timer == timers.end())
    {
        return;
    }

    timers.erase(timer);
}

inline void Timer::AddOneShot(std::function<void()> function, uint interval)
{
    oneShotTimers.emplace_back(function, 0, interval, TimeUtils::UnixTime<std::chrono::milliseconds>());
}

void FLHook::PublishServerStats()
{
    ServerStats stats;

    stats.npcsEnabled = instance->disableNpcs;
    stats.serverLoad = instance->serverLoadInMs;

    PROCESS_MEMORY_COUNTERS memCounter;
    GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof memCounter);
    stats.memoryUsage = memCounter.WorkingSetSize;

    for (auto& client : Clients())
    {
        if (client.characterName.empty())
        {
            continue;
        }

        auto system = client.id.GetSystemId().Unwrap();

        ServerStats::Player player;
        player.clientId = client.id.GetValue();
        player.playerName = client.characterName;
        player.systemName = system.GetValue() ? system.GetName().Unwrap() : L"";
        player.systemNick = system.GetValue() ? system.GetNickName().Unwrap() : L"";
        player.ipAddress = client.id.GetPlayerIp().Unwrap();

        stats.players.emplace_back(player);
    }

    const auto bson = rfl::bson::write(stats);
    MessageHandler::i()->Publish(std::string_view(bson.data(), bson.size()), std::wstring(MessageHandler::QueueToStr(MessageHandler::Queue::ServerStats)), L"");
}

void FLHook::TimerCheckKick()
{
    TryHook
    {
        const auto time = TimeUtils::UnixTime<std::chrono::seconds>();
        for (auto& client : Clients())
        {
            if (client.kickTime)
            {
                if (time >= client.kickTime)
                {
                    (void)client.id.Kick();
                    client.kickTime = 0;
                }

                continue;
            }

            const auto& config = FLHook::GetConfig();
            if (config.autoKicks.antiBaseIdle)
            {
                // anti base-idle check
                if (client.baseEnterTime && time - client.baseEnterTime >= config.autoKicks.antiBaseIdle)
                {
                    (void)client.id.Kick(L"Base idling", 10);
                    client.baseEnterTime = 0;
                }
            }

            if (config.autoKicks.antiCharMenuIdle)
            {
                // anti charmenu-idle check
                if (!client.characterName.empty())
                {
                    if (!client.charMenuEnterTime)
                    {
                        client.charMenuEnterTime = static_cast<uint>(time);
                    }
                    else if (time - client.charMenuEnterTime >= config.autoKicks.antiCharMenuIdle)
                    {
                        (void)client.id.Kick();
                        client.charMenuEnterTime = 0;
                    }
                }
                else
                {
                    client.charMenuEnterTime = 0;
                }
            }
        }
    }
    CatchHook({})
}

void FLHook::TimerNpcAndF1Check()
{
    // ReSharper disable once CppRedundantEmptyStatement
    ;
    TryHook
    {
        auto time = TimeUtils::UnixTime<std::chrono::milliseconds>();
        for (auto& client : Clients())
        {
            if (client.f1Time && time >= client.f1Time)
            {
                // f1
                Server.CharacterInfoReq(client.id.GetValue(), false);
                client.f1Time = 0;
            }
            else if (client.timeDisconnect && time >= client.timeDisconnect)
            {
                ulong dataArray[64] = { 0 };
                dataArray[26] = client.id.GetValue();
                auto rcDisconnect = static_cast<DWORD>(AddressList::RcDisconnect);
                __asm {
						pushad
						lea ecx, dataArray
						mov eax, [remoteClient]
						add eax, rcDisconnect
						call eax // disconnect
						popad
                }

                client.timeDisconnect = 0;
                continue;
            }
        }


        /*if (const auto& config = FLHook::GetConfig(); config.general.disableNPCSpawns && instance->serverLoadInMs >= config.general.disableNPCSpawns)
        {
            // TODO: NPC SPAWN TIME!!
            // Hk::Admin::ChangeNPCSpawn(true); // serverload too high, disable npcs
        }
        else
        {
            // Hk::Admin::ChangeNPCSpawn(false);
        }*/
    }
    CatchHook({})
}
