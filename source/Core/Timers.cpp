#include "API/InternalApi.hpp"
#include "PCH.hpp"

#include <Psapi.h>

#include "Core/FLHook.hpp"

#include "API/FLHook/ClientList.hpp"

#define CRONCPP_IS_CPP17
#include "croncpp.h"

std::shared_ptr<Timer> Timer::AddCron(const std::function<void()>& function, const std::wstring_view cronExpression)
{
    auto ptr = std::make_shared<Timer>();
    ptr->func = function;
    ptr->lastTime = TimeUtils::UnixTime<std::chrono::seconds>();

    const auto expression = StringUtils::wstos(cronExpression);
    const auto cronexpr = ::cron::make_cron(expression);

    std::tm tm{};
    const time_t time = ptr->lastTime;
    ::cron::utils::time_to_tm(&time, &tm);
    auto next = cron_next(cronexpr, tm);

    CronTimer timer = { cron::utils::tm_to_time(next), expression };

    ptr->cron = timer;
    cronTimers.emplace(ptr);

    return ptr;
}

std::shared_ptr<Timer> Timer::Add(const std::function<void()>& function, const uint intervalInMs)
{
    //TODO: Use chrono:: instead of uint
    auto ptr = std::make_shared<Timer>();
    ptr->func = function;
    ptr->interval = intervalInMs;
    timers.emplace(ptr);

    return ptr;
}

void Timer::Remove(const std::shared_ptr<Timer>& timer)
{
    timers.erase(timer);
    oneShotTimers.erase(timer);
    cronTimers.erase(timer);
}

std::shared_ptr<Timer> Timer::AddOneShot(const std::function<void()>& function, const uint intervalInMs,
                                         const std::optional<std::function<void(std::shared_ptr<Timer>)>>& callback)
{
    std::shared_ptr<Timer> timer = std::make_shared<Timer>();
    timer->func = function;
    timer->interval = intervalInMs;
    timer->lastTime = TimeUtils::UnixTime<std::chrono::milliseconds>();
    timer->callback = callback;
    oneShotTimers.emplace(timer);

    return timer;
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

            const auto config = FLHook::GetConfig();
            if (config->autoKicks.antiBaseIdle)
            {
                // anti base-idle check
                if (client.baseEnterTime && time - client.baseEnterTime >= config->autoKicks.antiBaseIdle)
                {
                    (void)client.id.Kick(L"Base idling", 10);
                    client.baseEnterTime = 0;
                }
            }

            if (config->autoKicks.antiCharMenuIdle)
            {
                // anti charmenu-idle check
                if (!client.characterName.empty())
                {
                    if (!client.charMenuEnterTime)
                    {
                        client.charMenuEnterTime = static_cast<uint>(time);
                    }
                    else if (time - client.charMenuEnterTime >= config->autoKicks.antiCharMenuIdle)
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

void FLHook::OneSecondTimer()
{
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
                // ReSharper disable once CppDFAUnusedValue
                // ReSharper disable once CppDFAUnreadVariable
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

        if (const auto config = GetConfig();
            config->npc.disableNPCSpawns < 0 || (config->npc.disableNPCSpawns && instance->serverLoadInMs >= config->npc.disableNPCSpawns))
        {
            InternalApi::ToggleNpcSpawns(false);
        }
        else
        {
            InternalApi::ToggleNpcSpawns(true);
        }
    }
    CatchHook({})
}
