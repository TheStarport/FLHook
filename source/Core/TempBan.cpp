#include "PCH.hpp"

#include "Core/TempBan.hpp"
#include "Defs/FLHookConfig.hpp"

void TempBanManager::ClearFinishedTempBans()
{
    const auto timeNow = TimeUtils::UnixTime<std::chrono::milliseconds>();
    auto it = tempBanList.begin();
    while (it != tempBanList.end())
    {
        if ((*it).banEnd < timeNow)
        {
            it = tempBanList.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void TempBanManager::AddTempBan(ClientId client, uint durationInMin, const std::wstring& banReason)
{
    if (!FLHookConfig::i()->general.tempBansEnabled)
    {
        return;
    }

    auto acc = client.GetAccount().Unwrap();
    const auto accId = Hk::Client::GetAccountID(account).Unwrap();

    TempBanInfo banInfo;
    banInfo.banStart = TimeUtils::UnixTime<std::chrono::milliseconds>();
    banInfo.banEnd = banInfo.banStart + durationInMin * 1000 * 60;
    banInfo.accountId = accId;

    if (!banReason.empty())
    {
        client.Kick(banReason, 10);
    }
    else
    {
        client.Kick();
    }

    tempBanList.push_back(banInfo);
}

void TempBanManager::AddTempBan(ClientId client, uint durationInMin) { AddTempBan(client, durationInMin, L""); }

bool TempBanManager::CheckIfTempBanned(ClientId client)
{
    if (!FLHookConfig::i()->general.tempBansEnabled)
    {
        return false;
    }

    CAccount* acc = client.GetAccount().Unwrap();
    const auto id = Hk::Client::GetAccountID(acc).Unwrap();

    return std::ranges::any_of(tempBanList, [&id](const TempBanInfo& ban) { return ban.accountId == id; });
}
