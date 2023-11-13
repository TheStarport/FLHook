#include "PCH.hpp"

#include "Core/ClientServerInterface.hpp"

#include "API/API.hpp"

bool LoginInnerBefore(const SLoginInfo& li, ClientId client)
{
    // The startup cache disables reading of the banned file. Check this manually on login and boot the player if they are banned.

    if (CAccount* acc = Players.FindAccountFromClientID(client))
    {
        const std::wstring dir = Hk::Client::GetAccountDirName(acc);

        char DataPath[MAX_PATH];
        GetUserDataPath(DataPath);

        const std::wstring path = std::format(L"{}\\Accts\\MultiPlayer\\{}\\banned", StringUtils::stows(std::string(DataPath)), dir);

        if (std::filesystem::exists(path))
        {
            // Ban the player
            st6::wstring fr(reinterpret_cast<ushort*>(acc->accId));
            Players.BanAccount(fr, true);

            // Kick them
            acc->ForceLogout();
            return false;
        }
    }

    return true;
}

bool LoginInnerAfter(const SLoginInfo& li, ClientId client)
{
    TryHook
    {
        if (client > MaxClientId)
        {
            return false; // DisconnectDelay bug
        }

        if (!Hk::Client::IsValidClientID(client))
        {
            return false; // player was kicked
        }

        // Kick the player if the account Id doesn't exist. This is caused
        // by a duplicate log on.
        CAccount* acc = Players.FindAccountFromClientID(client);
        if (acc && !acc->accId)
        {
            acc->ForceLogout();
            return false;
        }

        // check for ip ban
        const auto ip = Hk::Admin::GetPlayerIP(client);

        for (const auto& ban : FLHookConfig::i()->bans.banWildcardsAndIPs)
        {
            if (Wildcard::Fit(StringUtils::wstos(ban).c_str(), StringUtils::wstos(ip).c_str()))
            {
                // AddKickLog(client, std::format(L"IP/hostname ban({} matches {})", ip.c_str(), ban.c_str()));
                if (FLHookConfig::i()->bans.banAccountOnMatch)
                {
                    Hk::Player::Ban(client, true);
                }
                Hk::Player::Kick(client);
            }
        }

        // resolve
        const RESOLVE_IP rip = { client, ClientInfo::At(client).connects, ip };

        EnterCriticalSection(&csIPResolve);
        resolveIPs.push_back(rip);
        LeaveCriticalSection(&csIPResolve);

        // TODO: Move almost all loading and character state functions to a global class for proper management,
        // bonus points for proper threading support / accessors @Nen
        LoadUserSettings(client);

        // AddConnectLog(client, ip));
    }
    CatchHook({
        CAccount* acc = Players.FindAccountFromClientID(client);
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
        CallServerPreamble { Server.Login(li, client); }
        CallServerPostamble(true, );
    }

    LoginInnerAfter(li, client);

    if (TempBanManager::i()->CheckIfTempBanned(client))
    {
        Hk::Player::Kick(client);
        return;
    }

    CallPlugins(&Plugin::OnLoginAfter, client, li);
}
