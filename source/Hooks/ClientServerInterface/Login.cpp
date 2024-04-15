#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"
#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/TaskScheduler.hpp"
#include "Core/ClientServerInterface.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "API/Utils/TempBan.hpp"
#include "Core/IpResolver.hpp"

void IServerImplHook::LoginInnerAfter(const SLoginInfo& li, ClientId client)
{
    TryHook
    {
        if (!client)
        {
            return; // DisconnectDelay bug
        }

        // Kick the player if the account Id doesn't exist. This is caused by a duplicate log on.
        auto acc = client.GetAccount().Unwrap();
        if (acc)
        {
            acc.Logout();
            return;
        }

        // check for ip ban
        const auto ip = client.GetPlayerIp().Unwrap();

        for (const auto& ban : FLHook::GetConfig().bans.banWildcardsAndIPs)
        {
            if (Wildcard::Fit(StringUtils::wstos(ban).c_str(), StringUtils::wstos(ip).c_str()))
            {
                // AddKickLog(client, std::format(L"IP/hostname ban({} matches {})", ip.c_str(), ban.c_str()));
                if (FLHook::GetConfig().bans.banAccountOnMatch)
                {
                    client.GetAccount().Handle().Ban();
                }
                client.Kick();
            }
        }

        // resolve

        {
            const IpResolver::ResolvedIp resolved = { client.GetValue(), client.GetData().connects, ip };
            std::scoped_lock lock(IpResolver::mutex);
            IpResolver::resolveIPs.push_back(resolved);
        }

        // TODO: Move almost all loading and character state functions to a global class for proper management,
        // bonus points for proper threading support / accessors @Nen
        FLHook::instance->LoadUserSettings(client);

        // AddConnectLog(client, ip));
    }
    CatchHook({
        CAccount* acc = Players.FindAccountFromClientID(client.GetValue());
        if (acc)
        {
            acc->ForceLogout();
        }
    });
}

void IServerImplHook::DelayedLogin(const SLoginInfo& li, ClientId client)
{
    CallServerPreamble { Server.Login(li, client.GetValue()); }
    CallServerPostamble(true, );

    LoginInnerAfter(li, client);

    auto acc = client.GetAccount().Unwrap();
    if (FLHook::GetTempBanManager().CheckIfTempBanned(acc))
    {
        client.Kick();
        return;
    }

    CallPlugins(&Plugin::OnLoginAfter, client, li);
}

void __stdcall IServerImplHook::Login(const SLoginInfo& li, ClientId client)
{
    Logger::Log(LogLevel::Trace, std::format(L"Login(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnLogin, client, li); !skip)
    {
        TaskScheduler::ScheduleWithCallback(std::bind(AccountManager::Login, li.account, client),
            std::bind(&IServerImplHook::DelayedLogin, li, client));
    }
}
