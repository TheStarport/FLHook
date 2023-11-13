#include "PCH.hpp"

#include <Psapi.h>

#include "API/API.hpp"
#include "Core/FLHook.hpp"
#include "Core/MessageHandler.hpp"
#include "Core/TempBan.hpp"
#include "Defs/ServerStats.hpp"

std::vector<std::shared_ptr<Timer>> Timer::timers;

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

void FLHook::PublishServerStats()
{
    ServerStats stats;

    stats.npcsEnabled = instance->disableNpcs;
    stats.serverLoad = instance->serverLoadInMs;

    PROCESS_MEMORY_COUNTERS memCounter;
    GetProcessMemoryInfo(GetCurrentProcess(), &memCounter, sizeof memCounter);
    stats.memoryUsage = memCounter.WorkingSetSize;

    PlayerData* pd = nullptr;
    for (pd = Players.traverse_active(pd); pd; pd = Players.traverse_active(pd))
    {
        const auto info = Hk::Admin::GetPlayerInfo(pd->onlineId, false).Raw();
        if (info.has_error())
        {
            continue;
        }

        ServerStats::Player player;
        player.clientId = pd->onlineId;
        player.playerName = info.value().character;
        player.systemName = info.value().systemName;
        player.systemNick = StringUtils::stows(Universe::get_system(info.value().system)->nickname);
        player.ipAddress = info.value().IP;

        stats.players.emplace_back(player);
    }

    const auto json = Serializer::ObjToJson(stats);
    MessageHandler::i()->Publish(StringUtils::stows(json.dump()), std::wstring(MessageHandler::QueueToStr(MessageHandler::Queue::ServerStats)), L"");
}

void FLHook::TimerTempBanCheck()
{
    if (const auto* config = FLHookConfig::c(); config->general.tempBansEnabled)
    {
        TempBanManager::i()->ClearFinishedTempBans();
    }
}

void FLHook::TimerCheckKick()
{
    TryHook
    {
        // for all players
        PlayerData* playerData = nullptr;
        while ((playerData = Players.traverse_active(playerData)))
        {
            ClientId client = playerData->onlineId;
            if (client < 1 || client > MaxClientId)
            {
                continue;
            }

            if (ClientInfo::At(client).tmKickTime)
            {
                if (TimeUtils::UnixMilliseconds() >= ClientInfo::At(client).tmKickTime)
                {
                    Hk::Player::Kick(client); // kick time expired
                    ClientInfo::At(client).tmKickTime = 0;
                }
                continue; // player will be kicked anyway
            }
            const auto* config = FLHookConfig::c();
            if (config->general.antiBaseIdle)
            {
                // anti base-idle check
                uint baseId;
                pub::Player::GetBase(client, baseId);
                if (baseId && ClientInfo::At(client).baseEnterTime && time(nullptr) - ClientInfo::At(client).baseEnterTime >= config->general.antiBaseIdle)
                {
                    // AddKickLog(client, "base idling");
                    Hk::Player::MsgAndKick(client, L"Base idling", 10);
                    ClientInfo::At(client).baseEnterTime = 0;
                }
            }

            if (config->general.antiCharMenuIdle)
            {
                // anti charmenu-idle check
                if (Hk::Client::IsInCharSelectMenu(client))
                {
                    if (!ClientInfo::At(client).charMenuEnterTime)
                    {
                        ClientInfo::At(client).charMenuEnterTime = static_cast<uint>(time(nullptr));
                    }
                    else if (time(nullptr) - ClientInfo::At(client).charMenuEnterTime >= config->general.antiCharMenuIdle)
                    {
                        // AddKickLog(client, "Charmenu idling");
                        Hk::Player::Kick(client);
                        ClientInfo::At(client).charMenuEnterTime = 0;
                    }
                }
                else
                {
                    ClientInfo::At(client).charMenuEnterTime = 0;
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
        PlayerData* playerData = nullptr;
        while ((playerData = Players.traverse_active(playerData)))
        {
            ClientId client = playerData->onlineId;
            if (client < 1 || client > MaxClientId)
            {
                continue;
            }

            if (ClientInfo::At(client).tmF1Time && TimeUtils::UnixMilliseconds() >= ClientInfo::At(client).tmF1Time)
            {
                // f1
                Server.CharacterInfoReq(client, false);
                ClientInfo::At(client).tmF1Time = 0;
            }
            else if (ClientInfo::At(client).tmF1TimeDisconnect && TimeUtils::UnixMilliseconds() >= ClientInfo::At(client).tmF1TimeDisconnect)
            {
                ulong dataArray[64] = { 0 };
                dataArray[26] = client;

                __asm {
						pushad
						lea ecx, dataArray
						mov eax, [remoteClient]
						add eax, ADDR_RC_DISCONNECT
						call eax // disconnect
						popad
                }

                ClientInfo::At(client).tmF1TimeDisconnect = 0;
                continue;
            }
        }

        const auto* config = FLHookConfig::c();
        if (config->general.disableNPCSpawns && CoreGlobals::c()->serverLoadInMs >= config->general.disableNPCSpawns)
        {
            Hk::Admin::ChangeNPCSpawn(true); // serverload too high, disable npcs
        }
        else
        {
            Hk::Admin::ChangeNPCSpawn(false);
        }
    }
    CatchHook({})
}
