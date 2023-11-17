#include "PCH.hpp"

#include "API/Utils/TempBan.hpp"
#include "Defs/FLHookConfig.hpp"

void TempBanManager::ClearFinishedTempBans()
{
    const auto timeNow = TimeUtils::UnixTime<std::chrono::milliseconds>();
    auto it = tempBanList.begin();
    while (it != tempBanList.end())
    {
        if (it->banEnd < timeNow)
        {
            AccountId(it->accountId).UnBan();
            it = tempBanList.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void TempBanManager::AddTempBan(const AccountId account, const uint durationInDays)
{
    if (!FLHookConfig::i()->general.tempBansEnabled)
    {
        return;
    }

    TempBanInfo banInfo;
    banInfo.banStart = TimeUtils::UnixTime<std::chrono::milliseconds>();
    banInfo.banEnd = banInfo.banStart + static_cast<uint64>(durationInDays) * 1000 * 60 * 60 * 24; // 1000ms = 1s, 60s = 1 min, 60 min = 1 hour, 24 hour = 1d
    banInfo.accountId = account.GetValue()->accId;

    tempBanList.push_back(banInfo);
}

bool TempBanManager::CheckIfTempBanned(AccountId account) const
{
    if (!FLHookConfig::i()->general.tempBansEnabled)
    {
        return false;
    }

    return std::ranges::any_of(tempBanList, [&account](const TempBanInfo& ban) { return ban.accountId == account.GetValue()->accId; });
}
