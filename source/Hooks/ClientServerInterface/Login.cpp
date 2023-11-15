#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "Core/ClientServerInterface.hpp"

#include "API/Utils/PerfTimer.hpp"
#include "Core/IpResolver.hpp"
#include "Core/Logger.hpp"
#include "Core/TempBan.hpp"

bool LoginInnerBefore(const SLoginInfo& li, ClientId client)
{
    // The startup cache disables reading of the banned file. Check this manually on login and boot the player if they are banned.

    if (auto acc = client.GetAccount().Unwrap())
    {
        const auto dir = acc.GetDirectoryName();

        char DataPath[MAX_PATH];
        GetUserDataPath(DataPath);

        const std::wstring path = std::format(L"{}\\Accts\\MultiPlayer\\{}\\banned", StringUtils::stows(std::string(DataPath)), dir);

        if (std::filesystem::exists(path))
        {
            acc.Ban();
            acc.Logout();
            return false;
        }
    }

    return true;
}

bool IServerImplHook::LoginInnerAfter(const SLoginInfo& li, ClientId client)
{
    TryHook
    {
        if (!client)
        {
            return false; // DisconnectDelay bug
        }

        // Kick the player if the account Id doesn't exist. This is caused by a duplicate log on.
        auto acc = client.GetAccount().Unwrap();
        if (acc)
        {
            acc.Logout();
            return false;
        }

        // check for ip ban
        const auto ip = client.GetPlayerIp().Unwrap();

        for (const auto& ban : FLHookConfig::i()->bans.banWildcardsAndIPs)
        {
            if (Wildcard::Fit(StringUtils::wstos(ban).c_str(), StringUtils::wstos(ip).c_str()))
            {
                // AddKickLog(client, std::format(L"IP/hostname ban({} matches {})", ip.c_str(), ban.c_str()));
                if (FLHookConfig::i()->bans.banAccountOnMatch)
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
        return false;
    });

    return true;
}

void __stdcall IServerImplHook::Login(const SLoginInfo& li, ClientId client)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"Login(\n\tClientId client = {}\n)", client));

    if (const auto skip = CallPlugins(&Plugin::OnLogin, client, li); !skip && LoginInnerBefore(li, client))
    {
        CallServerPreamble { Server.Login(li, client.GetValue()); }
        CallServerPostamble(true, );
    }

    LoginInnerAfter(li, client);

    auto acc = client.GetAccount().Unwrap();
    if (FLHook::GetTempBanManager().CheckIfTempBanned(acc))
    {
        client.Kick();
        return;
    }

    CallPlugins(&Plugin::OnLoginAfter, client, li);
}
