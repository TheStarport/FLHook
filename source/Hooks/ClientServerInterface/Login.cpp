#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"
#include "API/FLHook/InfocardManager.hpp"
#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/TaskScheduler.hpp"
#include "Core/ClientServerInterface.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/IpResolver.hpp"

void IServerImplHook::LoginInnerAfter(const SLoginInfo& li, ClientId client)
{
    TryHook
    {
        if (!client)
        {
            return; // DisconnectDelay bug
        }

        auto& clientData = client.GetData();

        if (!clientData.account)
        {
            // This can happen if a player tries to log into an account which
            // is already in use - we need to protect ourselves before
            // following the clientData.account pointer below
            return;
        }

        // Copy over admin information, if applicable
        auto& credentials = FLHook::instance->credentialsMap[client] = {};
        if (const auto& roles = clientData.account->gameRoles; roles.has_value())
        {
            for (auto& role : roles.value())
            {

                auto result = credentials.insert(StringUtils::stows(role));
                if (!result.second)
                {
                    WARN("Duplicate role found on character account {{character}} - {{role}}",
                         { "character", StringUtils::stows(clientData.account->_id) },
                         { "role", role });
                }
            }
        }

        for (auto& next : FLHook::Clients())
        {
            if (next.id != client && clientData.account == next.account)
            {
                // Kick due to duplicate login
                (void)client.Kick();
                return;
            }
        }

        // check for ip ban
        const auto ip = client.GetPlayerIp().Unwrap();

        for (const auto& ban : FLHook::GetConfig()->bans.banWildcardsAndIPs)
        {
            if (wildcards::match(ip, ban))
            {
                // AddKickLog(client, std::format(L"IP/hostname ban({} matches {})", ip.c_str(), ban.c_str()));
                if (FLHook::GetConfig()->bans.banAccountOnMatch)
                {
                    client.GetAccount().Handle().Ban();
                }
                client.Kick();
            }
        }

        const IpResolver::ResolvedIp resolved = { client.GetValue(), client.GetData().connects, ip };
        std::scoped_lock lock(IpResolver::mutex);
        IpResolver::resolveIPs.push_back(resolved);

        for (auto& el : clientData.account->accountData)
        {
            switch (Hash(el.key().data()))
            {
                case Hash("dieMsg"):
                    {
                        clientData.dieMsg = magic_enum::enum_cast<DieMsgType>(StringUtils::stows(el.get_string().value)).value_or(DieMsgType::All);
                        break;
                    }
                default: break;
            }
        }

        // TODO: AddConnectLog(client, ip));
    }
    CatchHook({
        CAccount* acc = Players.FindAccountFromClientID(client.GetValue());
        if (acc)
        {
            acc->ForceLogout();
        }
    });
}

void IServerImplHook::DelayedLogin(SLoginInfo li, ClientId client)
{
    CallServerPreamble { Server.Login(li, client.GetValue()); }
    CallServerPostamble(true, );

    LoginInnerAfter(li, client);

    CallPlugins(&Plugin::OnLoginAfter, client, li);

    FLHook::GetInfocardManager()->SendAllOverrides(client);
}

void __stdcall IServerImplHook::Login(const SLoginInfo& li, ClientId client)
{
    static bool firstLogin = true;
    if (firstLogin)
    {
        // Only call StartupAfter on first login as the only time we can guarantee everything is fully initalised
        CallPlugins(&Plugin::OnServerStartupAfter);
        firstLogin = false;
    }

    TRACE("{{client}}", { "client", client });

    if (const auto skip = CallPlugins(&Plugin::OnLogin, client, li); !skip)
    {
        FLHook::GetTaskScheduler()->ScheduleTask(AccountManager::Login, li, client);
    }
}
