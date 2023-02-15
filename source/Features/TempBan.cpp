#include "Features/TempBan.hpp"
#include "Global.hpp"

void TempBanManager::ClearFinishedTempBans()
{
	auto timeNow = timeInMS();
	auto it = tempBanList.begin();
	while (it != tempBanList.end())
	{
		if ((*it).banEnd < timeNow)
		{
			it = tempBanList.erase(it);
		}
		else
		{
			it++;
		}
	}
}


void TempBanManager::AddTempBan(ClientId client, uint durationInMin, const std::wstring& banReason)
{
	if (!FLHookConfig::i()->general.tempBansEnabled)
		return;

	auto account = Hk::Client::GetAccountByClientID(client);
    const auto accId = Hk::Client::GetAccountID(account);

	TempBanInfo banInfo;
	banInfo.banStart = timeInMS();
	banInfo.banEnd = banInfo.banStart + (durationInMin * 1000 * 60);
	banInfo.accountId = accId.value();

	if (!banReason.empty())
	{
		Hk::Player::KickReason(client, banReason);
	}
	else
	{
		Hk::Player::Kick(client);
	}
	
	tempBanList.push_back(banInfo);
}

void TempBanManager::AddTempBan(ClientId client, uint durationInMin)
{
	AddTempBan(client, durationInMin, L"");
}

bool TempBanManager::CheckIfTempBanned(ClientId client)
{
	if (!FLHookConfig::i()->general.tempBansEnabled)
		return false;

	CAccount* acc = Players.FindAccountFromClientID(client);
	const auto id = Hk::Client::GetAccountID(acc);

	return std::ranges::any_of(tempBanList, [&id](TempBanInfo const& ban) { return ban.accountId == id.value(); });
}